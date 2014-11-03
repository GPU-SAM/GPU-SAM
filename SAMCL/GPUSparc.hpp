#include <CL/opencl.h>
#include <iostream>
#include <map>
#include <mqueue.h>

#include "CLAPI.h"

#define GPUSparcLog(args...) logFileGPUSparc=fopen("./GPUSparcLog.txt","a+");\
			   		 	 fprintf(logFileGPUSparc,args); \
						 fclose (logFileGPUSparc)

#define ELAPSEDTIME(x,y) (y.tv_sec-x.tv_sec)+(y.tv_usec-x.tv_usec)/1000000.0

#define MAX_MSG 256
#define MSG_SIZE 15

using namespace std;


struct meminfo_GPUSparc
{
	cl_mem 			*mem;
	cl_mem_flags 	flags;
	size_t 			size;
	size_t			element_size;
	size_t			region[3];
	void 			*shadow_buffer;
	bool 			merged;
	bool			synched;
	cl_map_flags	map_flags;
	void *			mapped_ptr;
	size_t 			mapped_region[3];
	size_t			mapped_origin[3];
	int 			cohered_gpu;
};

struct kernelinfo_GPUSparc
{
	cl_kernel				*kernel;
	map <cl_uint, cl_mem>	args;
	cl_uint 				arg_num;
};

extern FILE    *logFileGPUSparc;
extern bool	multiGPUmode;
extern bool migration;
extern int pid;
extern int prio;
extern int gpuid;

extern cl_uint nGPU_GPUSparc;
extern cl_uint nCPU_GPUSparc;

extern mqd_t GPUSAMrequest;
extern mqd_t GPUSAMfinish;

extern char  thisone[15];
extern struct mq_attr attr;

extern cl_uint nComputeUnit;

extern cl_platform_id 	platform_GPUSparc;

extern cl_device_id		*dev_GPUSparc;
extern cl_context		*cont_GPUSparc;
extern cl_command_queue	*queue_GPUSparc;

extern map <cl_mem, struct meminfo_GPUSparc> 		memmap_GPUSparc;
extern map <cl_sampler, cl_sampler *>				samplermap_GPUSparc;
extern map <cl_program, cl_program *>				programmap_GPUSparc;
extern map	<cl_kernel, struct kernelinfo_GPUSparc>	kernelmap_GPUSparc;
extern map <cl_event, cl_event *>					eventmap_GPUSparc;

/* GPUSparc Buffer Manager */
extern cl_int bufferMerger_GPUSparc			(struct meminfo_GPUSparc *, cl_event *);
extern cl_int bufferSynchronizer_GPUSparc	(struct meminfo_GPUSparc *, size_t *, size_t *, cl_event *);
extern void analyzeImageFormat_GPUSparc (const cl_image_format *, size_t *, size_t *);

extern cl_int clEnqueueWriteBufferGPUSparc (struct meminfo_GPUSparc *, size_t, size_t, const void *, cl_uint, const cl_event *, cl_event *);
extern cl_int clEnqueueWriteBufferRectGPUSparc (struct meminfo_GPUSparc *, const size_t *, const size_t *, const size_t *, size_t, size_t, size_t, size_t, const void *, cl_uint, const cl_event *, cl_event *);
extern cl_int clEnqueueWriteImageGPUSparc (struct meminfo_GPUSparc *, const size_t *, const size_t *, size_t, size_t,  const void *, cl_uint, const cl_event *, cl_event *);




/* GPUSparc Decomposer */
extern cl_int decomposer_GPUSparc 			(cl_kernel, cl_uint, const size_t *, const size_t *, const size_t *, cl_uint, const cl_event *, cl_event *);

extern FILE *kernelParser_GPUSparc (const char *);

/* event */
extern cl_int waitEventGPUSparc (cl_uint, const cl_event *);

extern void sendRequest(char, int, int);
extern int waitRequest (char, int);
extern void sendFinish (char, int, int);
