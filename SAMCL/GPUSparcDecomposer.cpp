#include <CL/opencl.h>
#include <stdio.h>
#include "GPUSparc.hpp"
#include <sys/time.h>
#include <string.h>
#include <pthread.h>


#define CEIL(x,y) x % y == 0 ? x/y : (int)(x/y)+1

char msgL[MSG_SIZE];

mqd_t *readyKLaunch;
mqd_t readyLaunch;


struct kernelLauncherData{
	cl_uint index;
	cl_kernel kernel;
	cl_uint   work_dim;
	cl_uint 	max_index;
	int iter;
	int off;
	const size_t *global_work_size;
	const size_t *global_work_offset;
	const size_t *local_work_size;
	size_t size;
	unsigned int *num_groups;
	size_t step;
	int arg_num;
	cl_event *event;
};

void *single_kernel_launcher (void *data);
void *kernel_launcher (void *data)
{
	struct kernelLauncherData *kd = (struct kernelLauncherData *)data;
	size_t *global_work = (size_t *)malloc(sizeof(size_t)*(kd->work_dim));

	cl_uint i;
	for(i = 0; i < kd->work_dim; i++) {
		if (i != kd->max_index)
			global_work[i] = kd->global_work_size[i];
	}

	unsigned int one = 1;
	unsigned int zero = 0;
	
	for(i = 0; i < 3; i++) {
		if (i < kd->work_dim)
			__real_clSetKernelArg (kd->kernel,kd->arg_num+1+i, sizeof(unsigned int), &(kd->global_work_size[i]));
		else
			__real_clSetKernelArg (kd->kernel, kd->arg_num+1+i, sizeof(unsigned int), &one);
	}
	for(i = 0; i < 3; i++) {
		if (i < kd->work_dim) {
			__real_clSetKernelArg (kd->kernel, kd->arg_num+7+i, sizeof(unsigned int), &(kd->num_groups[i]));
		}
		else
			__real_clSetKernelArg (kd->kernel, kd->arg_num+7+i, sizeof(unsigned int), &one);
	}
	cl_uint j;
	size_t remain = kd->size;
	size_t step = kd->step;
	if (kd->local_work_size != NULL)
		step /= kd->local_work_size[kd->max_index];

	unsigned int off = kd->off;
	while (remain > 0) {
		size_t size = remain >= 2*step ? step : remain;
		if (kd->local_work_size != NULL)	global_work[kd->max_index] = size*kd->local_work_size[kd->max_index];
		else global_work[kd->max_index] = size;
		remain -= size;
		for(j = 0; j < 3; j++) {
			if (j == kd->max_index) 
				__real_clSetKernelArg (kd->kernel, kd->arg_num+4+j, sizeof(unsigned int), &off);
			else
				__real_clSetKernelArg (kd->kernel, kd->arg_num+4+j, sizeof(unsigned int), &zero);
		}
		sendRequest('g', kd->index, 1);
		waitRequest('g', kd->index);
		__real_clEnqueueNDRangeKernel (queue_GPUSparc[kd->index], kd->kernel, kd->work_dim, kd->global_work_offset, global_work, kd->local_work_size, 0, NULL, kd->event);
		__real_clFinish (queue_GPUSparc[kd->index]);
		sendFinish('g', kd->index, 1);
		off += size;

	}
	free (global_work);
	return NULL;
}

cl_uint getMaxIndex (cl_uint, const size_t *, const size_t *);
cl_uint getStep (cl_uint, const size_t *, const size_t *, cl_uint);


cl_int decomposer_GPUSparc(cl_kernel 	  	kernel,
						   cl_uint 	      	work_dim, 
						   const size_t * 	global_work_offset, 
						   const size_t * 	global_work_size,
						   const size_t * 	local_work_size,
						   cl_uint 		  	num_events_in_wait_list,
						   const cl_event *	event_wait_list,
						   cl_event *		event)
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;

	map<cl_kernel, struct kernelinfo_GPUSparc>::iterator kiter;
	kiter = kernelmap_GPUSparc.find (kernel);
	if (kiter == kernelmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find kernel in clEnqueueNDRangeKernel\n");
	}
	cl_kernel *kernels = kiter->second.kernel;

	size_t *local_work 	   = (size_t *)malloc(sizeof(size_t) * work_dim);

	unsigned int *num_group = (unsigned int *)malloc(sizeof(unsigned int) * work_dim);

	for(i = 0; i < work_dim; i++) {
		local_work[i] = (local_work_size != NULL)? local_work_size[i] : 1;
		num_group[i] = global_work_size[i]/local_work[i];
	}

	timeval t1, t2;


	cl_uint max_index = getMaxIndex (work_dim,global_work_size, local_work);
	cl_uint step = getStep (work_dim, global_work_size, local_work, max_index);

	map<cl_uint, cl_mem> args = kiter->second.args;
	map<cl_uint, cl_mem>::iterator argiter;
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter;
	/* inter-kernel dependency */
	GPUSparcLog ("Arg Sync\n");
	for (argiter = args.begin(); argiter != args.end(); ++argiter) {
		miter = memmap_GPUSparc.find (argiter->second);
		if (miter == memmap_GPUSparc.end()) {
			GPUSparcLog ("Cannot find args in clEnqueueNDRangeKernel\n");
			continue;
		}
		struct meminfo_GPUSparc *minfo = &(miter->second);
		if (!(miter->second.merged) && multiGPUmode) {
			ret |= bufferMerger_GPUSparc (minfo, NULL);
		}
		if (!(miter->second.synched)) {
			ret |= bufferSynchronizer_GPUSparc (minfo, NULL, NULL, NULL);
		}
	}
	gettimeofday (&t1, NULL);
	cl_event *tevent = (cl_event *)malloc(sizeof(cl_event) * nGPU_GPUSparc);
	
	cl_uint arg_index = kiter->second.arg_num;

	if (!multiGPUmode)
	{	
		if (migration)
			gpuid = -1;
		
		sendRequest('g', gpuid, 1);
		gpuid = waitRequest('g', -1);
		struct kernelLauncherData *kd = (struct kernelLauncherData *)malloc(sizeof(struct kernelLauncherData));
		kd->index = gpuid;
		kd->kernel = kernels[gpuid];
		kd->max_index = max_index;
		kd->work_dim = work_dim;
		kd->global_work_size = global_work_size;
		kd->global_work_offset = global_work_offset;
		kd->local_work_size = local_work_size;
		kd->step = step;
		kd->num_groups = num_group;
		kd->arg_num = arg_index;
		kd->event = NULL;
		kd->size = global_work_size[max_index]/local_work[max_index];
		kd->off = 0;
		single_kernel_launcher (kd);
	}
	else {

	
		struct kernelLauncherData *kd = (struct kernelLauncherData *)malloc(sizeof(struct kernelLauncherData)*nGPU_GPUSparc);
		int total = global_work_size[max_index]/local_work[max_index];
		int nGPU = nGPU_GPUSparc;
		
		for(i = 0; i < nGPU_GPUSparc; i++) {
			kd[i].index = i;
			kd[i].kernel = kernels[i];
			kd[i].work_dim = work_dim;
			kd[i].max_index = max_index;
			kd[i].global_work_size = global_work_size;
			kd[i].global_work_offset = global_work_offset;
			kd[i].local_work_size = local_work_size;
			kd[i].step = step;
			kd[i].num_groups = num_group;
			kd[i].arg_num = arg_index;
			kd[i].event = &tevent[i];
			int amount = CEIL(total,nGPU);
			total -= amount;
			nGPU--;
			kd[i].size = amount;
			if (i == 0)
					kd[i].off = 0;
					else
				kd[i].off = kd[i-1].off + kd[i-1].size;
		}
	
		pthread_t *kthread = (pthread_t *)malloc(sizeof(pthread_t) * nGPU_GPUSparc);
		for(i = 0; i < nGPU_GPUSparc; i++) {
			pthread_create (&kthread[i], NULL, kernel_launcher, &kd[i]);
		}

		for(i = 0; i < nGPU_GPUSparc; i++) {
			pthread_join (kthread[i], NULL);
		}


		if (event != NULL) {
			eventmap_GPUSparc.insert (map<cl_event, cl_event *>::value_type (tevent[0], tevent));
			*event = tevent[0];
	
		}
		else free (tevent);
	}
	gettimeofday (&t2, NULL);
	GPUSparcLog ("kernel time: %f\n", ELAPSEDTIME(t1, t2));
	for (argiter = args.begin(); argiter != args.end(); ++argiter) {
		miter = memmap_GPUSparc.find (argiter->second);
		struct meminfo_GPUSparc *minfo = &miter->second;
		minfo->merged = false;
		if (!multiGPUmode)
			minfo->cohered_gpu = gpuid;
	}
	if (migration)
		gpuid = -1;

	free (num_group);
	free (local_work);
	return ret;
}


cl_uint getMaxIndex (cl_uint 		 work_dim, 
					  const size_t * global_work_size,
					  const size_t * local_work_size)
{
	cl_uint i;
	cl_uint max_index = 0;
	size_t max = 0;
	for (i = 0; i < work_dim; i++)
	{
		if (max < global_work_size[i] / local_work_size[i])
		{
			max_index = i;
			max = global_work_size[i] / local_work_size[i];
		}
	}
	return max_index;
}

cl_uint getStep (cl_uint work_dim,
				 const size_t * global_work_size,
				 const size_t * local_work_size,
				 cl_uint max_index)
{
	if (multiGPUmode)
		return global_work_size[max_index]/nGPU_GPUSparc;
	else
		return global_work_size[max_index];
	/*
	for(i = 0; i < work_dim; i++) {
		if (i != max_index) {
			cu *= global_work_size[i] / local_work_size[i];
		}
	}*/
	/* decide # of work-groups per executino */

	/*GPUSparcLog("%d %d %d\n", global_work_size[max_index], local_work_size[max_index], nGPU_GPUSparc);
	cl_uint step = CEIL (nComputeUnit * 64, cu);
	step *= local_work_size[max_index];
	if (step > global_work_size[max_index]/nGPU_GPUSparc)
		step = global_work_size[max_index]/local_work_size[max_index]/nGPU_GPUSparc*local_work_size[max_index];
	if (step == 0) {
		step = global_work_size[max_index]/local_work_size[max_index]/nGPU_GPUSparc*local_work_size[max_index];

	}
	if (step == 0) {
		step = global_work_size[max_index];
	}

	GPUSparcLog ("step:=%d\n", step);
	return step;*/
}


void *single_kernel_launcher (void *data)
{
	struct kernelLauncherData *kd = (struct kernelLauncherData *)data;

	cl_uint i;
	cl_uint j;

	unsigned int one = 1;
	unsigned int zero = 0;
	
	for(i = 0; i < 3; i++) {
		if (i < kd->work_dim)
			__real_clSetKernelArg (kd->kernel,kd->arg_num+1+i, sizeof(unsigned int), &(kd->global_work_size[i]));
		else
			__real_clSetKernelArg (kd->kernel, kd->arg_num+1+i, sizeof(unsigned int), &one);
	}
	for(i = 0; i < 3; i++) {
		if (i < kd->work_dim) {
			__real_clSetKernelArg (kd->kernel, kd->arg_num+7+i, sizeof(unsigned int), &(kd->num_groups[i]));
		}
		else
			__real_clSetKernelArg (kd->kernel, kd->arg_num+7+i, sizeof(unsigned int), &one);
	}
	for(j = 0; j < 3; j++) {
		if (j == kd->max_index) 
			__real_clSetKernelArg (kd->kernel, kd->arg_num+4+j, sizeof(unsigned int), &zero);
		else
			__real_clSetKernelArg (kd->kernel, kd->arg_num+4+j, sizeof(unsigned int), &zero);
	}
	__real_clEnqueueNDRangeKernel (queue_GPUSparc[gpuid], kd->kernel, kd->work_dim, kd->global_work_offset, kd->global_work_size, kd->local_work_size, 0, NULL, kd->event);
	__real_clFinish (queue_GPUSparc[gpuid]);
	//printf("%d\n", gpuid);
	sendFinish('g', gpuid, 1);
	return NULL;
}
