#include <CL/opencl.h>
#include "CLAPI.h"
#include "GPUSparc.hpp"


#ifdef __cplusplus
extern "C" {
#endif

/* Command Queue APIs */
CL_API_ENTRY cl_command_queue CL_API_CALL
__wrap_clCreateCommandQueue
                    (cl_context                     context, 
                     cl_device_id                   device, 
                     cl_command_queue_properties    properties,
                     cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0 
{
	queue_GPUSparc = (cl_command_queue *)malloc(sizeof(cl_command_queue) * nGPU_GPUSparc);
	cl_uint i;
	for (i = 0; i < nGPU_GPUSparc; i++)
		queue_GPUSparc[i] = __real_clCreateCommandQueue (cont_GPUSparc[i], dev_GPUSparc[i], properties, errcode_ret);
	return queue_GPUSparc[0];
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clRetainCommandQueue
                   (cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_uint ret = CL_SUCCESS;
	for (i = 0; i < nGPU_GPUSparc; i++)
		ret |= __real_clRetainCommandQueue (queue_GPUSparc[i]);
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clReleaseCommandQueue
                   (cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_uint ret = CL_SUCCESS;
	for (i = 0; i < nGPU_GPUSparc; i++)
		ret |= __real_clReleaseCommandQueue (queue_GPUSparc[i]);
	free (queue_GPUSparc);
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clGetCommandQueueInfo
                     (cl_command_queue      command_queue,
                      cl_command_queue_info param_name,
                      size_t                param_value_size,
                      void *                param_value,
                      size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 
{
	return __real_clGetCommandQueueInfo (command_queue, param_name, param_value_size, param_value, param_value_size_ret);
}

#ifdef CL_USE_DEPRECATED_OPENCL_1_0_APIS

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clSetCommandQueueProperty
                         (cl_command_queue              command_queue,
                          cl_command_queue_properties   properties, 
                          cl_bool                       enable,
                          cl_command_queue_properties * old_properties) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	for (i = 0; i < nGPU_GPUSparc; i++)
		ret |= __real_clSetCommandQueueProperty (queue_GPUSparc[i], properties, enable, old_properties);
	return ret;
}

#endif

#ifdef __cplusplus
}
#endif
