#include <CL/opencl.h>
#include "CLAPI.h"
#include "GPUSparc.hpp"
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>

#define MAXMSG 10

#ifdef __cplusplus
extern "C" {
#endif

/* platform APIs */

/* Assume that there is one platform available */

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clGetPlatformIDs(cl_uint			num_entries,
						cl_platform_id*	platforms,
						cl_uint*		num_platforms) CL_API_SUFFIX__VERSION_1_0
{
	GPUSparcLog("This is modified library (GPUSparc)\n");
	attr.mq_maxmsg = MAXMSG;
	attr.mq_msgsize = 15;
	attr.mq_flags = 0;
	prio = getpriority (PRIO_PROCESS, 0);
	printf ("priority = %d\n",prio);
	pid = getpid();
	if (cmanager == 0) {
		printf("reset mq\n");
		sprintf(thisone, "/%d", pid);
		printf("%s\n", thisone);
		cmanager = mq_open ("/cmanager", O_WRONLY | O_CREAT, 0664, &attr);
		kmanager = mq_open ("/kmanager", O_WRONLY | O_CREAT, 0664, &attr);
		copymanager = mq_open ("/copy", O_WRONLY | O_CREAT, 0664, &attr);
		copycomplete = mq_open ("/copy_complete", O_WRONLY | O_CREAT, 0664, &attr);

		kernelmanager = mq_open ("/kmanager", O_WRONLY | O_CREAT, 0664, &attr);

	}
	return __real_clGetPlatformIDs(num_entries, platforms, num_platforms);
}

CL_API_ENTRY cl_int CL_API_CALL 
__wrap_clGetPlatformInfo
                 (cl_platform_id   platform, 
                  cl_platform_info param_name,
                  size_t           param_value_size, 
                  void *           param_value,
                  size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 
{
	return __real_clGetPlatformInfo (platform, param_name, param_value_size, param_value, param_value_size_ret);
}


#ifdef __cplusplus
}
#endif
