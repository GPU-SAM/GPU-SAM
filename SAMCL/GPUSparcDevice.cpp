#include <CL/opencl.h>
#include <stdio.h>
#include "CLAPI.h"
#include "GPUSparc.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/* Device APIs */
CL_API_ENTRY cl_int CL_API_CALL
__wrap_clGetDeviceIDs
              (cl_platform_id   platform,
               cl_device_type   device_type, 
               cl_uint          num_entries, 
               cl_device_id *   devices, 
               cl_uint *        num_devices) CL_API_SUFFIX__VERSION_1_0 
{
	if (device_type == CL_DEVICE_TYPE_CPU)
	{
		printf("GPUSparc support for GPUs\n");
		exit(-1);
	}
	if (devices != NULL)
	{
		__real_clGetDeviceIDs (platform, CL_DEVICE_TYPE_GPU,0, NULL, &nGPU_GPUSparc);
		dev_GPUSparc = (cl_device_id *)malloc (sizeof(cl_device_id) * nGPU_GPUSparc);
		__real_clGetDeviceIDs (platform, CL_DEVICE_TYPE_GPU, nGPU_GPUSparc, dev_GPUSparc, NULL);
		GPUSparcLog("GPUSparc: nGPU = %u\n", nGPU_GPUSparc);
	}
	cl_int ret = __real_clGetDeviceIDs (platform, CL_DEVICE_TYPE_GPU, 1, devices, num_devices);
	if (num_devices != NULL)
		*num_devices = 1;
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clGetDeviceInfo
               (cl_device_id    device,
                cl_device_info  param_name, 
                size_t          param_value_size, 
                void *          param_value,
                size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
	return __real_clGetDeviceInfo (device, param_name, param_value_size, param_value, param_value_size_ret);
}

#ifdef __cplusplus
}
#endif
