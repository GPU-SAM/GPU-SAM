#include <CL/opencl.h>
#include <iostream>
#include <map>
#include "GPUSparc.hpp"
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

FILE *logFileGPUSparc = NULL;


cl_uint nGPU_GPUSparc = 2;
cl_uint nCPU_GPUSparc = 4;

/* mode */
bool multiGPUmode = false;
bool migration = true;


/* OpenCL */
cl_platform_id 		platform_GPUSparc;
cl_device_id		*dev_GPUSparc;
cl_context			*cont_GPUSparc;
cl_command_queue	*queue_GPUSparc;


/* multi-gpu informations */
map <cl_mem, struct meminfo_GPUSparc> 		memmap_GPUSparc;
map <cl_sampler, cl_sampler *>				samplermap_GPUSparc;
map <cl_program, cl_program *>				programmap_GPUSparc;
map	<cl_kernel, struct kernelinfo_GPUSparc>	kernelmap_GPUSparc;
map <cl_event, cl_event *>					eventmap_GPUSparc;

/* message queues */
mqd_t GPUSAMrequest = 0;
mqd_t GPUSAMfinish = 0;

/* application information */
int pid;
int prio;
int gpuid = -1;
struct mq_attr attr;

/* number of compute unit */
cl_uint nComputeUnit = 8;


void sendRequest (char type, int which, int n) {
	char buf[MSG_SIZE];
	sprintf(buf, "%c %d %d %d", type, which, pid, n);
	mq_send (GPUSAMrequest, buf, MSG_SIZE, prio);
}

int waitRequest (char type, int which) {
	if (which < 0) {
		if (type == 'g') {
			char buf[MSG_SIZE];
			char getgpuid[MSG_SIZE] = {0,};
			unsigned p;
			sprintf(buf, "/%d%c%d", pid, type,which);
			mqd_t grant = mq_open (buf, O_RDONLY | O_CREAT, 0664, &attr);
			mq_receive (grant, getgpuid, MSG_SIZE,&p);
			int g;
			sscanf(getgpuid, "%d", &g);
			mq_close (grant);
			return g;
		}
		else {
			printf("what the\n");
			return -1;
		}
	}
	else {

		char buf[MSG_SIZE];
		char dummy[MSG_SIZE];
		unsigned p;
		if (type == 'g')
			sprintf(buf, "/%d%c%d", pid, type, which);
		else if (type == 'p')
			sprintf(buf, "/%d%c", pid, type);
		else;
		mqd_t grant =	mq_open (buf, O_RDONLY | O_CREAT, 0664, &attr);
		mq_receive (grant, dummy, MSG_SIZE, &p);
		mq_close (grant);
		return which;
	}
}

void sendFinish (char type, int which, int n) {
	char buf[MSG_SIZE];
	sprintf(buf, "%c %d %d %d", type, which, pid, n);
	mq_send (GPUSAMfinish, buf, MSG_SIZE, prio);
}

