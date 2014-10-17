#include <CL/opencl.h>
#include <stdio.h>
#include "CLAPI.h"
#include "GPUSparc.hpp"


#ifdef __cplusplus
extern "C" {
#endif

/* Kernel Object APIs */
CL_API_ENTRY cl_kernel CL_API_CALL
__wrap_clCreateKernel
              (cl_program      program,
               const char *    kernel_name,
               cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	map<cl_program, cl_program *>::iterator piter_GPUSparc;
	piter_GPUSparc = programmap_GPUSparc.find (program);
	if (piter_GPUSparc == programmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find program in clCreateKernel\n");
		return NULL;
	}
	cl_program *programs_GPUSparc = piter_GPUSparc->second;
	cl_kernel *kernels_GPUSparc = (cl_kernel *)malloc(sizeof(cl_kernel) * nGPU_GPUSparc);
	struct kernelinfo_GPUSparc kinfo_GPUSparc;
	kinfo_GPUSparc.kernel = kernels_GPUSparc;
	kinfo_GPUSparc.arg_num = 0;
	for (i = 0; i < nGPU_GPUSparc; i++)
		kernels_GPUSparc[i] = __real_clCreateKernel (programs_GPUSparc[i], kernel_name, errcode_ret);
	kernelmap_GPUSparc.insert (map<cl_kernel, struct kernelinfo_GPUSparc>::value_type(kernels_GPUSparc[0], kinfo_GPUSparc));
	return kernels_GPUSparc[0];
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clCreateKernelsInProgram
                        (cl_program     program,
                         cl_uint        num_kernels,
                         cl_kernel *    kernels,
                         cl_uint *      num_kernels_ret) CL_API_SUFFIX__VERSION_1_0 
{
	if (kernels == NULL) {
		GPUSparcLog ("kernels == NULL in clCreateKernelsInProgram\n");
		return __real_clCreateKernelsInProgram  (program, num_kernels, kernels, num_kernels_ret);
	}
	cl_uint i,j;
	cl_int ret = CL_SUCCESS;
	cl_kernel **tempkernels = (cl_kernel **)malloc(sizeof(cl_kernel *) * (nGPU_GPUSparc-1));
	for (i = 0; i < nGPU_GPUSparc - 1; i++)
		tempkernels[i] = (cl_kernel *)malloc(sizeof(cl_kernel) * num_kernels);
	map<cl_program, cl_program *>::iterator piter_GPUSparc;
	piter_GPUSparc = programmap_GPUSparc.find (program);

	if (piter_GPUSparc == programmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find program in clCreateKernelsInProgram\n");
		return CL_INVALID_PROGRAM;
	}
	
	cl_program *programs_GPUSparc = piter_GPUSparc->second;
	ret |= __real_clCreateKernelsInProgram (programs_GPUSparc[0], num_kernels, kernels, num_kernels_ret);
	for(i = 1; i < nGPU_GPUSparc; i++)
		ret |= __real_clCreateKernelsInProgram (programs_GPUSparc[i], num_kernels, tempkernels[i-1], num_kernels_ret);
	
	for(i = 0; i < num_kernels; i++) {
		cl_kernel *temp = (cl_kernel *)malloc(sizeof(cl_kernel) * nGPU_GPUSparc);
		temp[0] = kernels[i];
		struct kernelinfo_GPUSparc kinfo_GPUSparc;
		kinfo_GPUSparc.kernel = temp;
		for (j = 1; j < nGPU_GPUSparc; j++)
			temp[j] = tempkernels[j-1][i];
		kernelmap_GPUSparc.insert (map<cl_kernel, struct kernelinfo_GPUSparc>::value_type (temp[0], kinfo_GPUSparc));
	}
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clRetainKernel
                 (cl_kernel kernel) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	
	map<cl_kernel, struct kernelinfo_GPUSparc>::iterator kiter_GPUSparc;
	kiter_GPUSparc = kernelmap_GPUSparc.find (kernel);

	if (kiter_GPUSparc == kernelmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find kernel in clRetainKernel\n");
		return __real_clRetainKernel (kernel);
	}

	cl_kernel *kernels_GPUSparc = kiter_GPUSparc->second.kernel;
	for (i = 0; i  < nGPU_GPUSparc; i++)
		ret |= __real_clRetainKernel (kernels_GPUSparc[i]);
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clReleaseKernel
                 (cl_kernel kernel) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	
	map<cl_kernel, struct kernelinfo_GPUSparc>::iterator kiter_GPUSparc;
	kiter_GPUSparc = kernelmap_GPUSparc.find (kernel);

	if (kiter_GPUSparc == kernelmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find kernel in clReleaseKernel\n");
		return __real_clReleaseKernel (kernel);
	}

	cl_kernel *kernels_GPUSparc = kiter_GPUSparc->second.kernel;
	for (i = 0; i  < nGPU_GPUSparc; i++)
		ret |= __real_clReleaseKernel (kernels_GPUSparc[i]);
	free (kernels_GPUSparc);
	kernelmap_GPUSparc.erase (kiter_GPUSparc);
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clSetKernelArg
              (cl_kernel    kernel,
               cl_uint      arg_index,
               size_t       arg_size,
               const void * arg_value) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	map <cl_kernel, struct kernelinfo_GPUSparc>::iterator kiter_GPUSparc;
	kiter_GPUSparc = kernelmap_GPUSparc.find (kernel);
	if (kiter_GPUSparc == kernelmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find kernel in clSetKernelArg\n");
		return __real_clSetKernelArg (kernel, arg_index, arg_size, arg_value);
	}
	struct kernelinfo_GPUSparc *kinfo = &kiter_GPUSparc->second;
	if (kinfo->arg_num < arg_index)
		kinfo->arg_num = arg_index;

	
	cl_kernel *kernels_GPUSparc = kiter_GPUSparc->second.kernel;
	
	if (arg_value != NULL) {
		map<cl_mem, struct meminfo_GPUSparc>::iterator miter_GPUSparc;
		miter_GPUSparc = memmap_GPUSparc.find (*((cl_mem *)arg_value));
		if (miter_GPUSparc == memmap_GPUSparc.end()) {
			for (i = 0; i < nGPU_GPUSparc; i++)
				ret |= __real_clSetKernelArg (kernels_GPUSparc[i], arg_index, arg_size, arg_value);
		}
		else {
			cl_mem *mems_GPUSparc = miter_GPUSparc->second.mem;

			if (!(miter_GPUSparc->second.flags & CL_MEM_READ_ONLY)) {
			
				map<cl_uint, cl_mem>::iterator argiter_GPUSparc;
				argiter_GPUSparc = kiter_GPUSparc->second.args.find (arg_index);
				if (argiter_GPUSparc == kiter_GPUSparc->second.args.end()) {
					kiter_GPUSparc->second.args.insert (map<cl_uint, cl_mem>::value_type (arg_index, mems_GPUSparc[0]));	
				}
				else {
					kiter_GPUSparc->second.args.erase (argiter_GPUSparc);
					kiter_GPUSparc->second.args.insert (map<cl_uint, cl_mem>::value_type (arg_index, mems_GPUSparc[0]));
				}
			}
			for (i = 0; i < nGPU_GPUSparc; i++) 
				ret |= __real_clSetKernelArg (kernels_GPUSparc[i], arg_index, arg_size, (void *)&mems_GPUSparc[i]);

		}
	}
	else {
		for (i = 0; i < nGPU_GPUSparc; i++) 
			ret |= __real_clSetKernelArg (kernels_GPUSparc[i], arg_index, arg_size, arg_value);
	}
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clGetKernelInfo
               (cl_kernel       kernel,
                cl_kernel_info  param_name,
                size_t          param_value_size,
                void *          param_value,
                size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 
{
	return __real_clGetKernelInfo (kernel, param_name, param_value_size, param_value, param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clGetKernelWorkGroupInfo
                        (cl_kernel                  kernel,
                         cl_device_id               device,
                         cl_kernel_work_group_info  param_name,
                         size_t                     param_value_size,
                         void *                     param_value,
                         size_t *                   param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 
{
	return __real_clGetKernelWorkGroupInfo (kernel, device, param_name, param_value_size, param_value, param_value_size_ret);
}

#ifdef __cplusplus
}
#endif
