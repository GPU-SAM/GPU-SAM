#include <CL/opencl.h>
#include "CLAPI.h"
#include "GPUSparc.hpp"


#ifdef __cplusplus
extern "C" {
#endif

/* Flush and Finish APIs */
CL_API_ENTRY cl_int CL_API_CALL
__wrap_clFlush
           (cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 
{	
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	for (i = 0; i < nGPU_GPUSparc; i++)
		ret |= __real_clFlush (queue_GPUSparc[i]);
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clFinish
           (cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	for (i = 0; i < nGPU_GPUSparc; i++)
		ret |= __real_clFinish (queue_GPUSparc[i]);
	return ret;
}


#ifdef __cplusplus
}
#endif
