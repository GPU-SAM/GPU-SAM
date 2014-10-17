#include <CL/opencl.h>
#include <stdio.h>
#include "CLAPI.h"
#include "GPUSparc.hpp"


#ifdef __cplusplus
extern "C" {
#endif

/* Context APIs */

CL_API_ENTRY cl_context CL_API_CALL
__wrap_clCreateContext
               (const cl_context_properties * properties,
                cl_uint                       num_devices,
                const cl_device_id *          devices,
                void (CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                void *                        user_data,
                cl_int *                      errcode_ret) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i = 0;
	printf("%u \n",nGPU_GPUSparc);
	cont_GPUSparc = (cl_context *)malloc(sizeof(cl_context) * nGPU_GPUSparc);
	for (i = 0; i < nGPU_GPUSparc; i++)
		cont_GPUSparc[i] = __real_clCreateContext (properties, 1, &dev_GPUSparc[i], pfn_notify, user_data, errcode_ret);
	return	cont_GPUSparc[0];
}

CL_API_ENTRY cl_context CL_API_CALL
__wrap_clCreateContextFromType
                       (const cl_context_properties * properties,
                        cl_device_type                device_type,
                        void (CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                        void *                        user_data,
                        cl_int *                      errcode_ret) CL_API_SUFFIX__VERSION_1_0 
{
	if (device_type == CL_DEVICE_TYPE_CPU)
	{
		printf("GPUSparc support for GPUs\n");
		exit(-1);
	}
	cl_context ret = __wrap_clCreateContext (properties, CL_DEVICE_TYPE_GPU, dev_GPUSparc, pfn_notify, user_data, errcode_ret);
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clRetainContext
                 (cl_context context) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	for (i = 0; i < nGPU_GPUSparc; i++)
		ret |= __real_clRetainContext (cont_GPUSparc[i]);
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clReleaseContext
                 (cl_context context) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	for (i = 0; i < nGPU_GPUSparc; i++)
		ret |= __real_clReleaseContext (cont_GPUSparc[i]);
	free (cont_GPUSparc);
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clGetContextInfo
                (cl_context         context, 
                 cl_context_info    param_name, 
                 size_t             param_value_size, 
                 void *             param_value, 
                 size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 
{
	return __real_clGetContextInfo (context, param_name, param_value_size, param_value, param_value_size_ret);
}

#ifdef __cplusplus
}
#endif
