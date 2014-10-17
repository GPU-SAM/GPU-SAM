#include <CL/opencl.h>
#include <iostream>
#include <map>
#include "GPUSparc.hpp"
#include <mqueue.h>

using namespace std;

FILE *logFileGPUSparc = NULL;

cl_uint nGPU_GPUSparc = 1;
cl_uint nCPU_GPUSparc = 4;

bool multiGPUmode = true;
bool migration = false;

cl_platform_id 		platform_GPUSparc;

cl_device_id		*dev_GPUSparc;
cl_context			*cont_GPUSparc;
cl_command_queue	*queue_GPUSparc;

map <cl_mem, struct meminfo_GPUSparc> 		memmap_GPUSparc;
map <cl_sampler, cl_sampler *>				samplermap_GPUSparc;
map <cl_program, cl_program *>				programmap_GPUSparc;
map	<cl_kernel, struct kernelinfo_GPUSparc>	kernelmap_GPUSparc;
map <cl_event, cl_event *>					eventmap_GPUSparc;

mqd_t cmanager = 0;
mqd_t kmanager = 0;
mqd_t copymanager = 0;
mqd_t copycomplete = 0;
mqd_t kernelmanager = 0;
mqd_t kernelcomplete = 0;

int pid;
int prio;
int gpuid = -1;
struct mq_attr attr;

cl_uint nComputeUnit = 8;

char thisone[15];
