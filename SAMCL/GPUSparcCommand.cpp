#include <CL/opencl.h>
#include <stdio.h>
#include <string.h>
#include "CLAPI.h"
#include "GPUSparc.hpp"


#ifdef __cplusplus
extern "C" {
#endif
                     
/* Enqueued Commands APIs */
CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueReadBuffer
                   (cl_command_queue    command_queue,
                    cl_mem              buffer,
                    cl_bool             blocking_read,
                    size_t              offset,
                    size_t              cb, 
                    void *              ptr,
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event) CL_API_SUFFIX__VERSION_1_0 
{
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_GPUSparc;
	miter_GPUSparc = memmap_GPUSparc.find (buffer);

	if (miter_GPUSparc == memmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find buffer in clEnqueueReadBuffer\n");
		return CL_INVALID_MEM_OBJECT;
	}

	cl_int ret = CL_SUCCESS;

	if (!miter_GPUSparc->second.merged) {
		struct meminfo_GPUSparc *minfo = &miter_GPUSparc->second;
		ret |= bufferMerger_GPUSparc (minfo, event);
	}
	memcpy (ptr, (void *)(((size_t)miter_GPUSparc->second.shadow_buffer)+offset), cb);
	return ret;
}
CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueReadBufferRect
                       (cl_command_queue    command_queue,
                        cl_mem              buffer,
                        cl_bool             blocking_read,
                        const size_t *      buffer_origin,
                        const size_t *      host_origin, 
                        const size_t *      region,
                        size_t              buffer_row_pitch,
                        size_t              buffer_slice_pitch,
                        size_t              host_row_pitch,
                        size_t              host_slice_pitch,
                        void *              ptr,
                        cl_uint             num_events_in_wait_list,
                        const cl_event *    event_wait_list,
                        cl_event *          event) CL_API_SUFFIX__VERSION_1_1
{
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_GPUSparc;
	miter_GPUSparc = memmap_GPUSparc.find (buffer);
	if (miter_GPUSparc == memmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find buffer in clEnqueueReadBufferRect\n");
		return CL_INVALID_MEM_OBJECT;
	}

	cl_int ret = CL_SUCCESS;

	if (!miter_GPUSparc->second.merged)	{
		struct meminfo_GPUSparc *minfo = &miter_GPUSparc->second;
		ret |= bufferMerger_GPUSparc (minfo, event);
	}
	
	if (buffer_row_pitch == 0)
		buffer_row_pitch = region[0];
	if (buffer_slice_pitch == 0)
		buffer_slice_pitch = region[1] * buffer_row_pitch;
	if (host_row_pitch == 0)
		host_row_pitch = region[0];
	if (host_slice_pitch == 0)
		host_slice_pitch = region[1] * host_row_pitch;
	
	if (miter_GPUSparc->second.shadow_buffer != NULL) {
		size_t j,k;
		
		size_t sz = buffer_origin[2], dz = host_origin[2];

		const void *src = miter_GPUSparc->second.shadow_buffer;
		const void *dst = ptr;

		for (j = 0; j < region[2]; j++,dz++,sz++) {
			size_t sy = buffer_origin[1], dy = host_origin[1];
			for(k = 0; k < region[1]; k++,dy++,sy++) {
				size_t soff = sz * host_slice_pitch + sy * host_row_pitch + buffer_origin[0];
				size_t doff = dz * buffer_slice_pitch + dy * buffer_row_pitch + host_origin[0];
				memcpy ((char *)dst + doff, (char *)src + soff, region[0]);
			}
		}
	}
	return ret;
}


CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueWriteBuffer
                    (cl_command_queue   command_queue, 
                     cl_mem             buffer, 
                     cl_bool            blocking_write, 
                     size_t             offset, 
                     size_t             cb, 
                     const void *       ptr, 
                     cl_uint            num_events_in_wait_list, 
                     const cl_event *   event_wait_list, 
                     cl_event *         event) CL_API_SUFFIX__VERSION_1_0 
{

	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_GPUSparc;
	miter_GPUSparc = memmap_GPUSparc.find (buffer);
	if (miter_GPUSparc == memmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find buffer in clEnqueueWriteBuffer\n");
		return CL_INVALID_MEM_OBJECT;
	}	
	struct meminfo_GPUSparc *minfo = &miter_GPUSparc->second;
	if (miter_GPUSparc->second.size == cb) {
		
		minfo->merged = true;
		minfo->synched = true;
	}

	if (miter_GPUSparc->second.shadow_buffer != NULL)
		memcpy (miter_GPUSparc->second.shadow_buffer, (void *)(((size_t)ptr)+offset), cb);
	
	struct meminfo_GPUSparc *minfo_GPUSparc = &miter_GPUSparc->second;

	return clEnqueueWriteBufferGPUSparc (minfo_GPUSparc, offset, cb, ptr, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueWriteBufferRect
                       (cl_command_queue    command_queue,
                        cl_mem              buffer,
                        cl_bool             blocking_read,
                        const size_t *      buffer_origin,
                        const size_t *      host_origin, 
                        const size_t *      region,
                        size_t              buffer_row_pitch,
                        size_t              buffer_slice_pitch,
                        size_t              host_row_pitch,
                        size_t              host_slice_pitch,
                        const void *        ptr,
                        cl_uint             num_events_in_wait_list,
                        const cl_event *    event_wait_list,
                        cl_event *          event) CL_API_SUFFIX__VERSION_1_1
{
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_GPUSparc;
	miter_GPUSparc = memmap_GPUSparc.find (buffer);
	if (miter_GPUSparc == memmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find buffer in clEnqueueWriteBufferRect\n");
		return CL_INVALID_MEM_OBJECT;
	}

	struct meminfo_GPUSparc *minfo = &miter_GPUSparc->second;
	if (miter_GPUSparc->second.size == region[2]*region[1]*region[0]) {
		minfo->merged = true;
		minfo->synched = true;
	}

	if (buffer_row_pitch == 0)
		buffer_row_pitch = region[0];
	if (buffer_slice_pitch == 0)
		buffer_slice_pitch = region[1] * buffer_row_pitch;
	if (host_row_pitch == 0)
		host_row_pitch = region[0];
	if (host_slice_pitch == 0)
		host_slice_pitch = region[1] * host_row_pitch;

	if (miter_GPUSparc->second.shadow_buffer != NULL)
	{
		size_t j,k;
		
		size_t sz = host_origin[2], dz = buffer_origin[2];

		const void *src = ptr;
		const void *dst = miter_GPUSparc->second.shadow_buffer;

		for (j = 0; j < region[2]; j++,sz++,dz++) {
			size_t sy = host_origin[1], dy = buffer_origin[1];
			for(k = 0; k < region[1]; k++,sy++,dy++) {
				size_t soff = sz * host_slice_pitch + sy * host_row_pitch + host_origin[0];
				size_t doff = dz * buffer_slice_pitch + dy * buffer_row_pitch + buffer_origin[0];
				memcpy ((char *)dst + doff, (char *)src + soff, region[0]);
			}
		}
	}
	
	struct meminfo_GPUSparc *minfo_GPUSparc = &miter_GPUSparc->second;
	return clEnqueueWriteBufferRectGPUSparc (minfo_GPUSparc, buffer_origin, host_origin, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueCopyBuffer
                   (cl_command_queue    command_queue, 
                    cl_mem              src_buffer,
                    cl_mem              dst_buffer, 
                    size_t              src_offset,
                    size_t              dst_offset,
                    size_t              cb, 
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_src_GPUSparc;
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_dst_GPUSparc;

	miter_src_GPUSparc = memmap_GPUSparc.find (src_buffer);
	miter_dst_GPUSparc = memmap_GPUSparc.find (dst_buffer);

	if (miter_src_GPUSparc == memmap_GPUSparc.end() || miter_dst_GPUSparc == memmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find src_buffer in clEnqueueCopyBuffer\n");
		return CL_INVALID_MEM_OBJECT;
	}
	if (!miter_src_GPUSparc->second.merged) {
		struct meminfo_GPUSparc *minfo_src = &miter_src_GPUSparc->second;
		ret |= bufferMerger_GPUSparc(minfo_src, NULL);
	}
	if (!miter_src_GPUSparc->second.synched) {
		struct meminfo_GPUSparc *minfo_src = &miter_src_GPUSparc->second;
		ret |= bufferSynchronizer_GPUSparc (minfo_src, NULL, NULL, NULL);
	}

	if (cb == miter_dst_GPUSparc->second.size) {

		struct meminfo_GPUSparc *minfo_dst = &miter_dst_GPUSparc->second;
		minfo_dst->merged = true;
		minfo_dst->synched = true;
	}
	const void *src = miter_src_GPUSparc->second.shadow_buffer;
	const void *dst = miter_dst_GPUSparc->second.shadow_buffer;

	if (src != NULL && dst != NULL) {
		memcpy ((char *)dst + dst_offset, (char *)src + src_offset, cb);
	}
	
	cl_event *tevent = (cl_event *)malloc(sizeof(cl_event) * nGPU_GPUSparc);
	for (i = 0; i < nGPU_GPUSparc; i++) {
		ret |= __real_clEnqueueCopyBuffer (queue_GPUSparc[i], miter_src_GPUSparc->second.mem[i], miter_dst_GPUSparc->second.mem[i], src_offset, dst_offset, cb, num_events_in_wait_list, event_wait_list, &tevent[i]);
	}
	if (event != NULL) {
		*event = tevent[0];
		eventmap_GPUSparc.insert (map<cl_event,cl_event *>::value_type (tevent[0], tevent));
	}
	else
		free (tevent);
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueCopyBufferRect
                   (cl_command_queue    command_queue, 
                    cl_mem              src_buffer,
                    cl_mem              dst_buffer, 
                    const size_t *      src_origin,
                    const size_t *      dst_origin,
                    const size_t *      region, 
                    size_t              src_row_pitch,
                    size_t              src_slice_pitch,
                    size_t              dst_row_pitch,
                    size_t              dst_slice_pitch,
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event) CL_API_SUFFIX__VERSION_1_1
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_src_GPUSparc;
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_dst_GPUSparc;

	miter_src_GPUSparc = memmap_GPUSparc.find (src_buffer);
	miter_dst_GPUSparc = memmap_GPUSparc.find (dst_buffer);

	if (miter_src_GPUSparc == memmap_GPUSparc.end() || miter_dst_GPUSparc == memmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find src_buffer in clEnqueueCopyBufferRect\n");
		return CL_INVALID_MEM_OBJECT;
	}
	if (!miter_src_GPUSparc->second.merged) {
		struct meminfo_GPUSparc *minfo_src = &miter_src_GPUSparc->second;
		ret |= bufferMerger_GPUSparc(minfo_src, NULL);
	}
	if (!miter_src_GPUSparc->second.synched) {
		struct meminfo_GPUSparc *minfo_src = &miter_src_GPUSparc->second;
		ret |= bufferSynchronizer_GPUSparc(minfo_src, NULL, NULL, NULL);
	}
	if (region[2]*region[1]*region[0] == miter_dst_GPUSparc->second.size) {

		struct meminfo_GPUSparc *minfo_dst = &miter_dst_GPUSparc->second;
		minfo_dst->merged = true;
		minfo_dst->synched = true;
	}

	const void *src = miter_src_GPUSparc->second.shadow_buffer;
	const void *dst = miter_dst_GPUSparc->second.shadow_buffer;

	if (src != NULL && dst != NULL) {
		size_t sz = src_origin[2], dz = dst_origin[2];
		size_t j, k;
		for (k = 0; k < region[2]; k++,dz++,sz++) {
			size_t sy = src_origin[1], dy = dst_origin[1];
			for (j = 0; j < region[1]; j++,dy++,sy++) {
				size_t soff = sz * src_slice_pitch + sy * src_row_pitch + src_origin[0];
				size_t doff = dz * dst_slice_pitch + dy * dst_row_pitch + dst_origin[0];
				memcpy ((char *)dst + doff, (char *)src + soff, region[0]);
			}
		}
	}
	cl_event *tevent = (cl_event *)malloc(sizeof(cl_event) * nGPU_GPUSparc);
	for (i = 0; i < nGPU_GPUSparc; i++) {
		ret |= __real_clEnqueueCopyBufferRect (queue_GPUSparc[i], miter_src_GPUSparc->second.mem[i], miter_dst_GPUSparc->second.mem[i], src_origin, dst_origin, region, src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch, num_events_in_wait_list, event_wait_list, &tevent[i]);
	}
	if (event != NULL) {
		*event = tevent[0];
		eventmap_GPUSparc.insert (map<cl_event,cl_event *>::value_type (tevent[0], tevent));
	}
	else
		free (tevent);
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueReadImage
                  (cl_command_queue     command_queue,
                   cl_mem               image,
                   cl_bool              blocking_read, 
                   const size_t *       origin,
                   const size_t *       region,
                   size_t               row_pitch,
                   size_t               slice_pitch, 
                   void *               ptr,
                   cl_uint              num_events_in_wait_list,
                   const cl_event *     event_wait_list,
                   cl_event *           event) CL_API_SUFFIX__VERSION_1_0 
{
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_GPUSparc;
	miter_GPUSparc = memmap_GPUSparc.find (image);
	if (miter_GPUSparc == memmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find image in clEnqueueReadImage\n");
		return CL_INVALID_MEM_OBJECT;
	}

	cl_int ret = CL_SUCCESS;

	struct meminfo_GPUSparc *minfo = &miter_GPUSparc->second;

	if (!miter_GPUSparc->second.merged) {
		struct meminfo_GPUSparc *minfo = &miter_GPUSparc->second;
		ret |= bufferMerger_GPUSparc (minfo, event);
	}

	if (row_pitch == 0)
		row_pitch = minfo->region[0] * minfo->element_size;
	if (slice_pitch == 0 && region[2] != 1)
		slice_pitch = row_pitch * minfo->region[1];
	
	if (minfo->shadow_buffer != NULL) {
		const void *src = minfo->shadow_buffer;
		const void *dst = ptr;
		size_t j,k;
		size_t element_size = minfo->element_size;
		size_t sz = origin[2], dz = 0;
		for (k = 0; k < region[2]; k++,sz++,dz++) {
			size_t sy = origin[1], dy = 0;
			for(j = 0; j < region[1]; j++,sy++,dy++) {
				size_t soff = sz * slice_pitch + sy * row_pitch + origin[0];
				size_t doff = dz * slice_pitch + dy * row_pitch;
				memcpy ((char *)dst+doff, (char *)src+soff, region[0] * element_size);
			}
		}
	}

	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueWriteImage
                   (cl_command_queue    command_queue,
                    cl_mem              image,
                    cl_bool             blocking_write, 
                    const size_t *      origin,
                    const size_t *      region,
                    size_t              input_row_pitch,
                    size_t              input_slice_pitch, 
                    const void *        ptr,
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event) CL_API_SUFFIX__VERSION_1_0 
{
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_GPUSparc;
	miter_GPUSparc = memmap_GPUSparc.find (image);
	if (miter_GPUSparc == memmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find image in clEnqueueWriteImage\n");
		return CL_INVALID_MEM_OBJECT;
	}

	struct meminfo_GPUSparc *minfo = &miter_GPUSparc->second;
	
	if (minfo->size == region[2] * region[1] * region[0] * minfo->element_size) {
		minfo->merged = true;
		minfo->synched = true;
	}

	if (input_row_pitch == 0) 
		input_row_pitch = minfo->region[0] * minfo->element_size;
	if (input_slice_pitch == 0 && region[2] != 1) 
		input_slice_pitch = input_row_pitch * minfo->region[1];
	

	if (minfo->shadow_buffer != NULL) {
		const void *src = ptr;
		const void *dst = minfo->shadow_buffer;
		size_t j,k;
		size_t element_size = minfo->element_size;
		size_t dz = origin[2], sz = 0;
		for(k = 0; k < region[2]; k++,dz++,sz++) {
			size_t dy = origin[1], sy = 0;
			for(j = 0; j < region[1]; j++,dy++,sy++) {
				size_t doff = dz * input_slice_pitch + dy * input_row_pitch + origin[0];
				size_t soff = sz * input_slice_pitch + sy * input_row_pitch;
				memcpy ((char *)dst+doff, (char *)src+soff, region[0] * element_size);
			}
		}
	}

	return clEnqueueWriteImageGPUSparc (minfo, origin, region, input_row_pitch, input_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);	
}


CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueCopyImage
                  (cl_command_queue     command_queue,
                   cl_mem               src_image,
                   cl_mem               dst_image, 
                   const size_t *       src_origin,
                   const size_t *       dst_origin,
                   const size_t *       region, 
                   cl_uint              num_events_in_wait_list,
                   const cl_event *     event_wait_list,
                   cl_event *           event) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_src_GPUSparc;
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_dst_GPUSparc;

	miter_src_GPUSparc = memmap_GPUSparc.find (src_image);
	miter_dst_GPUSparc = memmap_GPUSparc.find (dst_image);

	if (miter_src_GPUSparc == memmap_GPUSparc.end() || miter_dst_GPUSparc == memmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find src_buffer in clEnqueueCopyImage\n");
		return CL_INVALID_MEM_OBJECT;
	}
	if (!miter_src_GPUSparc->second.merged) {
		struct meminfo_GPUSparc *minfo_src = &miter_src_GPUSparc->second;
		ret |= bufferMerger_GPUSparc(minfo_src, NULL);
	}
	if (!miter_src_GPUSparc->second.synched) {
		struct meminfo_GPUSparc *minfo_src = &miter_src_GPUSparc->second;
		ret |= bufferSynchronizer_GPUSparc(minfo_src, NULL, NULL, NULL);
	}
	size_t element_size = miter_dst_GPUSparc->second.element_size;
	if (region[2]*region[1]*region[0]*element_size == miter_dst_GPUSparc->second.size) {
		struct meminfo_GPUSparc *minfo_dst = &miter_dst_GPUSparc->second;
		minfo_dst->merged = true;
		minfo_dst->synched = true;
	}

	const void *src = miter_src_GPUSparc->second.shadow_buffer;
	const void *dst = miter_dst_GPUSparc->second.shadow_buffer;

	if (src != NULL && dst != NULL) {
		size_t sz = src_origin[2], dz = dst_origin[2];
		size_t j, k;

		size_t src_row_pitch = miter_src_GPUSparc->second.region[0] * element_size;
		size_t src_slice_pitch = miter_src_GPUSparc->second.region[1] * src_row_pitch;

		size_t dst_row_pitch = miter_dst_GPUSparc->second.region[0] * element_size;
		size_t dst_slice_pitch = miter_dst_GPUSparc->second.region[1] * dst_row_pitch;

		for (k = 0; k < region[2]; k++,sz++,dz++) {
			size_t sy = src_origin[1], dy = dst_origin[1];
			for (j = 0; j < region[1]; j++,sy++,dy++) {
				size_t soff = sz * src_slice_pitch + sy * src_row_pitch + src_origin[0];
				size_t doff = dz * dst_slice_pitch + dy * dst_row_pitch + dst_origin[0];
				memcpy ((char *)dst + doff, (char *)src + soff, region[0] * element_size);
			}
		}
	}
	cl_event *tevent = (cl_event *)malloc(sizeof(cl_event) * nGPU_GPUSparc);

	for (i = 0; i < nGPU_GPUSparc; i++) {
		ret |= __real_clEnqueueCopyImage (queue_GPUSparc[i], miter_src_GPUSparc->second.mem[i], miter_dst_GPUSparc->second.mem[i], src_origin, dst_origin, region, num_events_in_wait_list, event_wait_list, &tevent[i]);
	}
	if (event != NULL) {
		*event = tevent[0];
		eventmap_GPUSparc.insert (map<cl_event,cl_event *>::value_type (tevent[0], tevent));
	}
	else
		free (tevent);
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueCopyImageToBuffer
                          (cl_command_queue command_queue,
                           cl_mem           src_image,
                           cl_mem           dst_buffer, 
                           const size_t *   src_origin,
                           const size_t *   region, 
                           size_t           dst_offset,
                           cl_uint          num_events_in_wait_list,
                           const cl_event * event_wait_list,
                           cl_event *       event) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_src_GPUSparc;
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_dst_GPUSparc;

	miter_src_GPUSparc = memmap_GPUSparc.find (src_image);
	miter_dst_GPUSparc = memmap_GPUSparc.find (dst_buffer);

	if (miter_src_GPUSparc == memmap_GPUSparc.end() || miter_dst_GPUSparc == memmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find src_buffer in clEnqueueCopyImageToBuffer\n");
		return CL_INVALID_MEM_OBJECT;
	}
	if (!miter_src_GPUSparc->second.merged) {
		struct meminfo_GPUSparc *minfo_src = &miter_src_GPUSparc->second;
		ret |= bufferMerger_GPUSparc(minfo_src, NULL);
	}
	if (!miter_src_GPUSparc->second.synched) {
		struct meminfo_GPUSparc *minfo_src = &miter_src_GPUSparc->second;
		ret |= bufferSynchronizer_GPUSparc(minfo_src, NULL, NULL, NULL);
	}
	size_t element_size = miter_src_GPUSparc->second.element_size;
	if (region[2]*region[1]*region[0]*element_size == miter_dst_GPUSparc->second.size) {
		struct meminfo_GPUSparc *minfo_dst = &miter_dst_GPUSparc->second;
		minfo_dst->merged = true;
		minfo_dst->synched = true;
	}

	const void *src = miter_src_GPUSparc->second.shadow_buffer;
	const void *dst = miter_dst_GPUSparc->second.shadow_buffer;

	if (src != NULL && dst != NULL) {
		size_t sz = src_origin[2], dz = 0;
		size_t j, k;

		size_t src_row_pitch = miter_src_GPUSparc->second.region[0] * element_size;
		size_t src_slice_pitch = miter_src_GPUSparc->second.region[1] * src_row_pitch;
		


		for (k = 0; k < region[2]; k++,sz++,dz++) {
			size_t sy = src_origin[1], dy = 0;
			for (j = 0; j < region[1]; j++,sy++,dy++) {
				size_t soff = sz * src_slice_pitch + sy * src_row_pitch + src_origin[0];
				size_t doff = dz * src_slice_pitch + dy * src_row_pitch + dst_offset;
				memcpy ((char *)dst + doff, (char *)src + soff, region[0] * element_size);
			}
		}
	}
	cl_event *tevent = (cl_event *)malloc(sizeof(cl_event) * nGPU_GPUSparc);
	for (i = 0; i < nGPU_GPUSparc; i++) {
		ret |= __real_clEnqueueCopyImageToBuffer (queue_GPUSparc[i], miter_src_GPUSparc->second.mem[i], miter_dst_GPUSparc->second.mem[i], src_origin, region, dst_offset, num_events_in_wait_list, event_wait_list, &tevent[i]);
	}
	if (event != NULL) {
		*event = tevent[0];
		eventmap_GPUSparc.insert (map<cl_event,cl_event *>::value_type (tevent[0], tevent));
	}
	else
		free (tevent);
	return ret;
}


CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueCopyBufferToImage
                          (cl_command_queue command_queue,
                           cl_mem           src_buffer,
                           cl_mem           dst_image, 
                           size_t           src_offset,
                           const size_t *   dst_origin,
                           const size_t *   region, 
                           cl_uint          num_events_in_wait_list,
                           const cl_event * event_wait_list,
                           cl_event *       event) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_src_GPUSparc;
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_dst_GPUSparc;

	miter_src_GPUSparc = memmap_GPUSparc.find (src_buffer);
	miter_dst_GPUSparc = memmap_GPUSparc.find (dst_image);

	if (miter_src_GPUSparc == memmap_GPUSparc.end() || miter_dst_GPUSparc == memmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find src_buffer in clEnqueueCopyBufferToImage\n");
		return CL_INVALID_MEM_OBJECT;
	}
	if (!miter_src_GPUSparc->second.merged) {
		struct meminfo_GPUSparc *minfo_src = &miter_src_GPUSparc->second;
		ret |= bufferMerger_GPUSparc(minfo_src, NULL);
	}
	if (!miter_src_GPUSparc->second.synched) {
		struct meminfo_GPUSparc *minfo_src = &miter_src_GPUSparc->second;
		ret |= bufferSynchronizer_GPUSparc(minfo_src, NULL, NULL, NULL);
	}
	size_t element_size = miter_dst_GPUSparc->second.element_size;
	if (region[2]*region[1]*region[0]*element_size == miter_dst_GPUSparc->second.size) {
		struct meminfo_GPUSparc *minfo_dst = &miter_dst_GPUSparc->second;
		minfo_dst->merged = true;
		minfo_dst->synched = true;
	}

	const void *src = miter_src_GPUSparc->second.shadow_buffer;
	const void *dst = miter_dst_GPUSparc->second.shadow_buffer;

	if (src != NULL && dst != NULL) {
		size_t sz = 0, dz = dst_origin[2];
		size_t j, k;

		size_t dst_row_pitch = miter_dst_GPUSparc->second.region[0] * element_size;
		size_t dst_slice_pitch = miter_dst_GPUSparc->second.region[1] * dst_row_pitch;
		


		for (k = 0; k < region[2]; k++,sz++,dz++) {
			size_t sy = 0, dy = dst_origin[1];
			for (j = 0; j < region[1]; j++,sy++,dy++) {
				size_t soff = sz * dst_slice_pitch + sy * dst_row_pitch + src_offset;
				size_t doff = dz * dst_slice_pitch + dy * dst_row_pitch + dst_origin[0];
				memcpy ((char *)dst + doff, (char *)src + soff, region[0] * element_size);
			}
		}
	}
	cl_event *tevent = (cl_event *)malloc(sizeof(cl_event) * nGPU_GPUSparc);
	for (i = 0; i < nGPU_GPUSparc; i++) {
		ret |= __real_clEnqueueCopyBufferToImage (queue_GPUSparc[i], miter_src_GPUSparc->second.mem[i], miter_dst_GPUSparc->second.mem[i], src_offset, dst_origin, region, num_events_in_wait_list, event_wait_list, &tevent[i]);
	}
	if (event != NULL) {
		*event = tevent[0];
		eventmap_GPUSparc.insert (map<cl_event,cl_event *>::value_type (tevent[0], tevent));
	}
	else
		free (tevent);
	return ret;
}

CL_API_ENTRY void * CL_API_CALL
__wrap_clEnqueueMapBuffer
                  (cl_command_queue command_queue,
                   cl_mem           buffer,
                   cl_bool          blocking_map, 
                   cl_map_flags     map_flags,
                   size_t           offset,
                   size_t           size,
                   cl_uint          num_events_in_wait_list,
                   const cl_event * event_wait_list,
                   cl_event *       event,
                   cl_int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0 
{
	void *ret = NULL;

	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_GPUSparc;
	miter_GPUSparc = memmap_GPUSparc.find (buffer);
	if (miter_GPUSparc == memmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find buffer in clEnqueueMapBuffer\n");
		*errcode_ret = CL_INVALID_MEM_OBJECT;
		return NULL;
	}
	*errcode_ret = CL_SUCCESS;
	struct meminfo_GPUSparc *minfo = &miter_GPUSparc->second;

	if (map_flags | CL_MAP_READ) {
		if (!miter_GPUSparc->second.merged)	{
			struct meminfo_GPUSparc *minfo = &miter_GPUSparc->second;
			*errcode_ret |= bufferMerger_GPUSparc (minfo, event);
		}

		ret = malloc(size);
		memcpy (ret, (char *)miter_GPUSparc->second.shadow_buffer+offset, size);
	}
	if (map_flags | CL_MAP_WRITE) {
		if (size == miter_GPUSparc->second.size) {
			minfo->merged = true;
			minfo->synched = true;
		}
		if (ret == NULL)
			ret = malloc(size);
	}
	minfo->map_flags = map_flags;
	minfo->mapped_ptr = ret;
	minfo->mapped_region[0] = size;
	minfo->mapped_region[1] = 1;
	minfo->mapped_region[2] = 1;
	minfo->mapped_origin[0] = offset;
	minfo->mapped_origin[1] = 0;
	minfo->mapped_origin[2] = 0;
	return ret;

}

CL_API_ENTRY void * CL_API_CALL
__wrap_clEnqueueMapImage
                 (cl_command_queue  command_queue,
                  cl_mem            image, 
                  cl_bool           blocking_map, 
                  cl_map_flags      map_flags, 
                  const size_t *    origin,
                  const size_t *    region,
                  size_t *          image_row_pitch,
                  size_t *          image_slice_pitch,
                  cl_uint           num_events_in_wait_list,
                  const cl_event *  event_wait_list,
                  cl_event *        event,
                  cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0 
{
	void *ret = NULL;
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_GPUSparc;
	miter_GPUSparc = memmap_GPUSparc.find (image);
	if (miter_GPUSparc == memmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find image in clEnqueueMapImage\n");
		*errcode_ret = CL_INVALID_MEM_OBJECT;
		return NULL;
	}

	struct meminfo_GPUSparc *minfo = &miter_GPUSparc->second;
	*errcode_ret = CL_SUCCESS;
	if (map_flags | CL_MAP_READ) {
		if (!miter_GPUSparc->second.merged) {
			struct meminfo_GPUSparc *minfo = &miter_GPUSparc->second;
			*errcode_ret |= bufferMerger_GPUSparc(minfo, event);
		}
		*image_row_pitch = miter_GPUSparc->second.region[0] * miter_GPUSparc->second.element_size;
		*image_slice_pitch = *image_row_pitch * miter_GPUSparc->second.region[1];
		
		size_t element_size = miter_GPUSparc->second.element_size;
		ret = malloc (region[0]*region[1]*region[2]*element_size);
		size_t j,k;
		size_t sz = origin[2], dz = 0;
		size_t dst_row_pitch = region[0] * element_size;
		size_t dst_slice_pitch = dst_row_pitch * region[1];
		const void *src = miter_GPUSparc->second.shadow_buffer;
		for (k = 0; k < region[2]; k++,sz++,dz++) {
			size_t sy = origin[1], dy = 0;
			for(j = 0; j < region[1]; j++,sy++,dy++) {
				size_t soff = sz * (*image_slice_pitch) + sy * (*image_row_pitch) + origin[0];
				size_t doff = dz * dst_slice_pitch + dy * dst_row_pitch;
				memcpy ((char *)ret + doff, (char *)src + soff, region[0] * element_size);
			}
		}		
	}
	if (map_flags | CL_MAP_WRITE) {
		size_t element_size = miter_GPUSparc->second.element_size;
		if (region[2]*region[1]*region[0]*element_size == miter_GPUSparc->second.size) {
			minfo->merged = true;
			minfo->synched = true;
		}
		if (ret != NULL) {
			ret = malloc (region[0] * region[1] * region[2] * element_size);
		}
	}

	minfo->map_flags = map_flags;
	minfo->mapped_ptr = ret;
	minfo->mapped_region[0] = region[0];
	minfo->mapped_region[1] = region[1];
	minfo->mapped_region[2] = region[2];
	minfo->mapped_origin[0] = origin[0];
	minfo->mapped_origin[1] = origin[1];
	minfo->mapped_origin[2] = origin[2];

	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueUnmapMemObject
                       (cl_command_queue command_queue,
                        cl_mem           memobj,
                        void *           mapped_ptr,
                        cl_uint          num_events_in_wait_list,
                        const cl_event * event_wait_list,
                        cl_event *       event) CL_API_SUFFIX__VERSION_1_0 
{
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_GPUSparc;
	miter_GPUSparc = memmap_GPUSparc.find (memobj);
	if (miter_GPUSparc == memmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find memobj in clEnqueueUnmapMemObject\n");
		return CL_INVALID_MEM_OBJECT;
	}
	if (miter_GPUSparc->second.mapped_ptr != mapped_ptr) {
		return CL_INVALID_VALUE;
	}
	struct meminfo_GPUSparc *minfo = &miter_GPUSparc->second;
	cl_int ret = CL_SUCCESS;
	if (miter_GPUSparc->second.map_flags | CL_MAP_WRITE) {

		const void *dst = minfo->shadow_buffer;
		const void *src = mapped_ptr;
		size_t element_size = minfo->element_size;

		if (minfo->mapped_region[2]*minfo->mapped_region[1]*minfo->mapped_region[0]*element_size == minfo->size) {

			minfo->merged = true;
			minfo->synched = true;
		}
		
		size_t src_row_pitch = minfo->mapped_region[0] * element_size;
		size_t src_slice_pitch = src_row_pitch * minfo->mapped_region[1];

		size_t dst_row_pitch = minfo->region[0] * element_size;
		size_t dst_slice_pitch = minfo->region[1] * dst_row_pitch;

		size_t sz = 0, dz = minfo->mapped_origin[2];
		size_t j,k;
		for(k = 0; k < minfo->mapped_region[2]; k++,sz++,dz++) {
			size_t sy = 0, dy = minfo->mapped_origin[1];
			for(j = 0; j < minfo->mapped_region[1]; j++,sy++,dy++) {
				size_t soff = sz * src_slice_pitch + sy * src_row_pitch + minfo->mapped_origin[0];
				size_t doff = dz * dst_slice_pitch + dy * dst_row_pitch;
				memcpy ((char *)dst+doff, (char *)src+soff, minfo->mapped_region[0] * element_size);
			}
		}
		ret |= bufferSynchronizer_GPUSparc (minfo, minfo->mapped_origin, minfo->mapped_region, event);
	}
	minfo->mapped_ptr = NULL;
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueNDRangeKernel
                      (cl_command_queue command_queue,
                       cl_kernel        kernel,
                       cl_uint          work_dim,
                       const size_t *   global_work_offset,
                       const size_t *   global_work_size,
                       const size_t *   local_work_size,
                       cl_uint          num_events_in_wait_list,
                       const cl_event * event_wait_list,
                       cl_event *       event) CL_API_SUFFIX__VERSION_1_0 
{
	cl_int ret = CL_SUCCESS;
	ret |= decomposer_GPUSparc (kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event);
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueTask
             (cl_command_queue  command_queue,
              cl_kernel         kernel,
              cl_uint           num_events_in_wait_list,
              const cl_event *  event_wait_list,
              cl_event *        event) CL_API_SUFFIX__VERSION_1_0 
{

	/* Cannot Decompose This */
	/* Assume that this function is not used */
	GPUSparcLog ("clEnqueueTask Called\n");
	return __real_clEnqueueTask (command_queue, kernel, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueNativeKernel
                     (cl_command_queue  command_queue,
          					  void (*user_func)(void *), 
                      void *            args,
                      size_t            cb_args, 
                      cl_uint           num_mem_objects,
                      const cl_mem *    mem_list,
                      const void **     args_mem_loc,
                      cl_uint           num_events_in_wait_list,
                      const cl_event *  event_wait_list,
                      cl_event *        event) CL_API_SUFFIX__VERSION_1_0 
{	
	/* CPU function */
	/* Assume that this function is not used */
	GPUSparcLog ("clEnqueueNativeKernel Called\n");
	return __real_clEnqueueNativeKernel (command_queue, user_func, args, cb_args, num_mem_objects, mem_list, args_mem_loc, num_events_in_wait_list, event_wait_list, event);
}

#ifdef CL_USE_DEPRECATED_OPENCL_1_0_APIS
CL_API_ENTRY cl_int CL_API_CALL
__wrap_clSetCommandQueueProperty (cl_command_queue    command_queue,
								  cl_command_queue_properties properties,
								  cl_bool enable,
								  cl_command_queue_properties *old_properties)
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	for (i = 0; i < nGPU_GPUSprc; i++) 
		ret |= __real_clSetCommandQueueProperty (queue_GPUSparc[i], properties, enable, old_properties);	
	return ret;
}

#endif 


CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueMarker
               (cl_command_queue    command_queue,
                cl_event *          event) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	cl_event *event_GPUSparc = (cl_event *)malloc(sizeof(cl_event) * nGPU_GPUSparc);
	for (i = 0; i < nGPU_GPUSparc; i++) {
		ret |= __real_clEnqueueMarker (queue_GPUSparc[i], &event_GPUSparc[i]);
	}
	if (event != NULL) {
		*event = event_GPUSparc[0];
		eventmap_GPUSparc.insert (map<cl_event, cl_event*>::value_type(event_GPUSparc[0], event_GPUSparc));
	}
	else free (event_GPUSparc);
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueWaitForEvents
                      (cl_command_queue command_queue,
                       cl_uint          num_events,
                       const cl_event * event_list) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i,j;
	cl_int ret = CL_SUCCESS;

	for(i = 0; i < num_events; i++) {
		map<cl_event, cl_event*>::iterator eiter;
		eiter = eventmap_GPUSparc.find (event_list[i]);
		if (eiter == eventmap_GPUSparc.end()) {
			GPUSparcLog("Cannot find event in clEnqueueWaitForEvent\n");
			continue;
		}
		for(j = 0; j < nGPU_GPUSparc; j++) {
			ret |= __real_clEnqueueWaitForEvents (queue_GPUSparc[j], 1, &eiter->second[j]);
		}
	}
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clEnqueueBarrier
                   (cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	for (i = 0; i < nGPU_GPUSparc; i++) {
		ret |= __real_clEnqueueBarrier (queue_GPUSparc[i]);
	}
	return ret;	
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clUnloadCompiler
                 (void) CL_API_SUFFIX__VERSION_1_0 
{
  return __real_clUnloadCompiler ();
}

#ifdef __cplusplus
}
#endif
