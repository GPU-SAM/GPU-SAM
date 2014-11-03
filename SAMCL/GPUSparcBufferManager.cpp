#include <CL/opencl.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "GPUSparc.hpp"

#include <sys/time.h>
#include <mqueue.h>
#include <sched.h>
#include <sys/resource.h>

#define CHUNK_SIZE (8*1024*1024)
#define MSG_SIZE 15

#define CEIL(x,y) x%y==0 ? x/y : (int)(x/y)+1

mqd_t ready;

struct tinfo {
	size_t element_size;
	size_t size;
	size_t offset;
	void **buffer;
	void *ptr;
};

char msg[MSG_SIZE];


void *t_merge (void *data);

cl_int bufferMerger_GPUSparc (struct meminfo_GPUSparc *minfo, cl_event *event)
{
	cl_int ret = CL_SUCCESS;
	cl_mem *mems = minfo->mem;
	timeval t1, t2;
	cl_uint i;
	void **buffer = (void **)malloc(sizeof(void *) * nGPU_GPUSparc);
	

	cl_event *tevent = (cl_event *)malloc(sizeof(cl_event) * nGPU_GPUSparc);


	if (multiGPUmode) {
		int n = minfo->size / CHUNK_SIZE;
		if (n == 0) n++;

		sendRequest ('p', 0, n*nGPU_GPUSparc);
		gettimeofday (&t1, NULL);
		int j;

		for(i = 0; i < nGPU_GPUSparc; i++) {
			buffer[i] = malloc(minfo->size);
			size_t remain = minfo->size;
			size_t off = 0;
			for(j = 0; j < n; j++) {
				size_t size = remain < 2*CHUNK_SIZE?remain:CHUNK_SIZE;
				waitRequest('p', 0);
				ret |= __real_clEnqueueReadBuffer (queue_GPUSparc[i], mems[i], CL_TRUE, off, size, (char *)buffer[i]+off, 0, NULL, &tevent[i]);
				remain -= size;
				off += size;
				sendFinish ('p', 0, 0);
			}
		}
		if (event != NULL) {
			*event = tevent[0];
			eventmap_GPUSparc.insert (map<cl_event,cl_event *>::value_type (tevent[0], tevent));
		}
		else free (tevent);

		gettimeofday (&t2, NULL);
		GPUSparcLog ("Read time: %f\n", ELAPSEDTIME(t1,t2));	

		struct tinfo *thrinfo = (struct tinfo *)malloc(sizeof(struct tinfo) * nCPU_GPUSparc);
		pthread_t *p_thread = (pthread_t *)malloc(sizeof(struct tinfo) * nCPU_GPUSparc);
		size_t element_size = minfo->element_size;
		size_t total = (minfo->size)/element_size;
	 	n = nCPU_GPUSparc;
		
		for(i = 0; i < nCPU_GPUSparc; i++) {
			thrinfo[i].element_size = element_size;
			thrinfo[i].size = CEIL(total, n);
			n--;
			total -= thrinfo[i].size;
			if (i == 0)
				thrinfo[i].offset = 0;
			else
				thrinfo[i].offset = thrinfo[i-1].offset + thrinfo[i-1].size;
			thrinfo[i].ptr = minfo->shadow_buffer;
			thrinfo[i].buffer = buffer;
		}
		gettimeofday (&t1, NULL);


		for(i = 0; i < nCPU_GPUSparc; i++)
			pthread_create (&p_thread[i], NULL, t_merge, (void *)&thrinfo[i]);
		for(i = 0; i < nCPU_GPUSparc; i++)
			pthread_join (p_thread[i], NULL);

		gettimeofday (&t2, NULL);
	
		GPUSparcLog ("Merge time: %f\n", ELAPSEDTIME(t1,t2));
	
	
		free (p_thread);
		free (thrinfo);
		for(i = 0; i < nGPU_GPUSparc; i++) free(buffer[i]);
		free(buffer);
		minfo->merged = true;
		minfo->synched = false;
	}
	else {
		int n = minfo->size/CHUNK_SIZE;
		if (n == 0) n++;
		sendRequest ('p', 0, n);
		gettimeofday (&t1, NULL);
		int j;

		size_t off = 0;
		size_t remain = minfo->size;
		for(j = 0; j < n; j++) {
			size_t size = remain < 2*CHUNK_SIZE?remain:CHUNK_SIZE;
			waitRequest('p', 0);
			ret |= __real_clEnqueueReadBuffer (queue_GPUSparc[minfo->cohered_gpu], mems[minfo->cohered_gpu], CL_TRUE, off, size, (char *)minfo->shadow_buffer+off, 0, NULL, event);

			remain -= size;
			off += size;
			sendFinish ('p', 0, 0);		
		}
		gettimeofday (&t2, NULL);
		GPUSparcLog ("Read time: %f\n", ELAPSEDTIME(t1,t2));	
	}
	return ret;
}


cl_int bufferSynchronizer_GPUSparc (struct meminfo_GPUSparc *minfo, size_t *offset, size_t *region, cl_event *event)
{
	cl_int ret = CL_SUCCESS;
	cl_uint i;
	if (offset == NULL) {
		offset = (size_t *)malloc(sizeof(size_t) * 3);
		for(i = 0; i < 3; i++) 	
			offset[i] = 0;
	}
	if (region == NULL) {
		region = (size_t *)malloc(sizeof(size_t) * 3);
		for(i = 0; i < 3; i++) 
			region[i] = minfo->region[i];
	}

	timeval t1, t2;

	gettimeofday (&t1, NULL);
	cl_event *tevent = (cl_event *)malloc(sizeof(cl_event) * nGPU_GPUSparc);
	if (region[0] * region[1] * region[2] * minfo->element_size == minfo->size) {
				
		minfo->synched = true;

		if (multiGPUmode || migration || gpuid < 0) {

			int n = (minfo->size)/CHUNK_SIZE;
			if (n == 0) n++;
			sendRequest ('p', 0, n*nGPU_GPUSparc);
			cl_uint j;
			for(j = 0; j < nGPU_GPUSparc; j++) {
				size_t remain = minfo->size;
				size_t off = 0;
				int i;
				for(i = 0; i < n; i++) {
					size_t size = CHUNK_SIZE*2 > remain? remain : CHUNK_SIZE;
					waitRequest ('p', 0);
					ret |= __real_clEnqueueWriteBuffer (queue_GPUSparc[j], minfo->mem[j], CL_TRUE, off, size, (char *)minfo->shadow_buffer+off, 0, NULL, &tevent[j]);
					off += size;
					remain -= size;
					sendFinish ('p', 0, 0);
				}
			}
		}
		else if (gpuid >= 0){
			unsigned n = (minfo->size)/CHUNK_SIZE;
			if (n == 0) n++;

			sendRequest('p', 0, n);
			size_t remain = minfo->size;
			size_t off = 0;

			for(i = 0; i < n; i++) {
				size_t size = CHUNK_SIZE*2 > remain? remain: CHUNK_SIZE;
				waitRequest ('p', 0);
				ret |= __real_clEnqueueWriteBuffer (queue_GPUSparc[gpuid], minfo->mem[gpuid], CL_TRUE, off, size, (char *)minfo->shadow_buffer+off, 0, NULL, NULL);
				off += size;
				remain -= size;
				sendFinish ('p', 0, 0);
			}
		}
		else {
			printf("?\n?\n");
		}
	}

	else {
	
		size_t row_pitch = minfo->region[0] * minfo->element_size;
		size_t slice_pitch = minfo->region[1] * row_pitch;


		int n = nGPU_GPUSparc;
		sendRequest ('p', 0, n);
		for(i = 0; i < nGPU_GPUSparc; i++) {
			waitRequest ('p', 0);
			ret |= __real_clEnqueueWriteBufferRect (queue_GPUSparc[i], minfo->mem[i], CL_TRUE, offset, offset, region, row_pitch, slice_pitch, row_pitch, slice_pitch, minfo->shadow_buffer, 0, NULL, &tevent[i]);
			sendFinish ('p', 0, 0);
		}
	}
	if (event != NULL) {
		*event = tevent[0];
		eventmap_GPUSparc.insert (map<cl_event,cl_event *>::value_type (tevent[0], tevent));
	}
	else free (tevent);
	gettimeofday (&t2, NULL);
	
	GPUSparcLog ("Synchronizer: %f\n", ELAPSEDTIME(t1,t2));

	return ret;
}

cl_int clEnqueueWriteBufferGPUSparc 
						(struct meminfo_GPUSparc * minfo,
 						 size_t					   offset,
						 size_t					   cb,
						 const void *			   ptr,
						 cl_uint 				   num_events_in_wait_list,
						 const cl_event *		   event_wait_list,
						 cl_event *				   event) 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;

	timeval t1, t2;
	gettimeofday (&t1, NULL);
	cl_event *tevent = (cl_event *)malloc(sizeof(cl_event) * nGPU_GPUSparc);


	if (gpuid >= 0 && !migration && !multiGPUmode) {
		GPUSparcLog ("Single Mode\n");
		unsigned n = cb / CHUNK_SIZE;
		if (n == 0) n++;
		sendRequest ('p', 0, n);
		size_t remain = cb;
		size_t off = offset;
		size_t ptr_off = 0;
		for(i = 0; i < n; i++) {
			size_t size = CHUNK_SIZE*2 > remain? remain : CHUNK_SIZE;
			waitRequest ('p', 0);
			ret |= __real_clEnqueueWriteBuffer (queue_GPUSparc[gpuid], minfo->mem[gpuid], CL_TRUE, off, size, (char *)ptr+ptr_off, 0, NULL, NULL);
			off += size;
			ptr_off += size;
			remain -= size;
			sendFinish ('p', 0, 0);
		}
	}
	else {
		
		unsigned n = cb / CHUNK_SIZE;
		if (n == 0) n++;

		sendRequest ('p', 0, n*nGPU_GPUSparc);
		cl_uint j;
		for(j = 0; j < nGPU_GPUSparc; j++) {
			size_t remain = cb;
			size_t off = offset;
			size_t ptr_off = 0;
			for(i = 0; i < n; i++) {
				size_t size = CHUNK_SIZE*2 > remain? remain : CHUNK_SIZE;
				waitRequest('p', 0);
				ret |= __real_clEnqueueWriteBuffer (queue_GPUSparc[j], minfo->mem[j], CL_TRUE, off, size, (char *)ptr+ptr_off, 0, NULL, &tevent[j]);

				off += size;
				ptr_off += size;
				remain -= size;
				sendFinish ('p', 0, 0);
			}
		}
	}


	if (event != NULL) {
		*event = tevent[0];
		eventmap_GPUSparc.insert (map<cl_event,cl_event *>::value_type (tevent[0], tevent));
	}
	else free (tevent);
	gettimeofday (&t2, NULL);

	GPUSparcLog ("Write: %f\n", ELAPSEDTIME (t1, t2));
	return ret;
}

cl_int clEnqueueWriteBufferRectGPUSparc
						(struct meminfo_GPUSparc * minfo,
						 const size_t *			   buffer_origin,
						 const size_t *			   host_origin,
						 const size_t *			   region,
						 size_t					   buffer_row_pitch,
						 size_t					   buffer_slice_pitch,
						 size_t					   host_row_pitch,
						 size_t					   host_slice_pitch,
						 const void *			   ptr,
						 cl_uint 				   num_events_in_wait_list,
						 const cl_event *		   event_wait_list,
						 cl_event *				   event)
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	
	cl_event *tevent = (cl_event *)malloc(sizeof(cl_event) * nGPU_GPUSparc);
	/* Not implemented yet */
	for (i = 0; i < nGPU_GPUSparc; i++) {
		ret |= __real_clEnqueueWriteBufferRect (queue_GPUSparc[i], minfo->mem[i], CL_TRUE, buffer_origin, host_origin, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, &tevent[i]);
	}

	if (event != NULL) {
		*event = tevent[0];
		eventmap_GPUSparc.insert (map<cl_event,cl_event *>::value_type (tevent[0], tevent));
	}
	else free (tevent);
	return ret;
}

cl_int clEnqueueWriteImageGPUSparc
						(struct meminfo_GPUSparc *minfo,
						 const size_t *			  origin,
						 const size_t *			  region,
						 size_t					  row_pitch,
						 size_t					  slice_pitch,
						 const void *			  ptr,
						 cl_uint				  num_events_in_wait_list,
						 const cl_event *		  event_wait_list,
						 cl_event *				  event)
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;

	/* Not implemented yet */
	cl_event *tevent = (cl_event *)malloc(sizeof(cl_event) * nGPU_GPUSparc);
	for (i = 0; i < nGPU_GPUSparc; i++) {
		ret |= __real_clEnqueueWriteImage (queue_GPUSparc[i], minfo->mem[i], CL_TRUE, origin, region, row_pitch, slice_pitch, ptr, num_events_in_wait_list, event_wait_list, &tevent[i]);
	}
	if (event != NULL) {
		*event = tevent[0];
		eventmap_GPUSparc.insert (map<cl_event,cl_event *>::value_type (tevent[0], tevent));
	}
	else free (tevent);
	return ret;
}




void analyzeImageFormat_GPUSparc 
			(const cl_image_format * image_format,
			 size_t *element_size,
			 size_t *channels) {
	switch (image_format->image_channel_order) {
		case CL_R:
		case CL_Rx:
		case CL_A:
		case CL_INTENSITY:
		case CL_LUMINANCE:
			*channels = 1;
			break;
		case CL_RG:
		case CL_RGx:
		case CL_RA:
			*channels = 2;
			break;
		case CL_RGB:
		case CL_RGBx:
			*channels = 3;
			break;
		case CL_RGBA:
		case CL_ARGB:
		case CL_BGRA:
			*channels = 4;
			break;
		default:
			GPUSparcLog ("Invalid Image Format (channels)\n");
			return;
	}

	switch (image_format->image_channel_data_type) {
		case CL_SNORM_INT8:
		case CL_UNORM_INT8:
		case CL_SIGNED_INT8:
		case CL_UNSIGNED_INT8:
			*element_size = 1;
			break;
		case CL_SNORM_INT16:
		case CL_UNORM_INT16:
		case CL_UNORM_SHORT_565:
		case CL_UNORM_SHORT_555:
		case CL_SIGNED_INT16:
		case CL_UNSIGNED_INT16:
		case CL_HALF_FLOAT:
			*element_size = 2;
			break;
		case CL_UNORM_INT_101010:
		case CL_SIGNED_INT32:
		case CL_UNSIGNED_INT32:
		case CL_FLOAT:
			*element_size = 4;
			break;
		default:
			GPUSparcLog ("Invalid Image Format (data type)\n");
			return;
	}
	*element_size *= *channels;
}

void *t_merge (void *data)
{
	struct tinfo *thrinfo = (struct tinfo *)data;
	size_t j;
	cl_uint i;

	char *host_mem = (char *)thrinfo->ptr;
	for(j = thrinfo->offset; j < thrinfo->offset + thrinfo->size; j+=thrinfo->element_size) {
		for(i = 0; i < nGPU_GPUSparc; i++) {
			char *temp = (char *)thrinfo->buffer[i];
			if (temp[j] != host_mem[j]){
				host_mem[j] = temp[j];
				break;				
			}
		}
	}
	return NULL;
}
