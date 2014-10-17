#include <CL/opencl.h>
#include "CLAPI.h"
#include "GPUSparc.hpp"


#ifdef __cplusplus
extern "C" {
#endif

/* Profiling APIs */
CL_API_ENTRY cl_int CL_API_CALL
__wrap_clGetEventProfilingInfo
                       (cl_event            event,
                        cl_profiling_info   param_name,
                        size_t              param_value_size,
                        void *              param_value,
                        size_t *            param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 
{
	return __real_clGetEventProfilingInfo (event, param_name, param_value_size, param_value, param_value_size_ret);
}

#ifdef __cplusplus
}
#endif
