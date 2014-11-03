#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <sched.h>
#include <CL/opencl.h>
#include <queue>
#include <pthread.h>

#define MSG_SIZE 15
#define MAX_MSG 256

using namespace std;

int prio = 99;    /* maximum rt-priority */
cl_uint nGPU = 2; /* number of GPU 			 */

mqd_t request; 		/* receiving request   */
mqd_t finish;			/* receiving finished  */


/* Request */
class Request {
	public:
		int pid;
		int priority;
		int numRequest;
		Request(int new_pid, int new_priority, int new_numRequest):pid(new_pid), priority(new_priority), numRequest(new_numRequest){}
		void decrease() {numRequest--;}
};


/* operator of Requests */
bool operator< (const Request& request1, const Request& request2)
{
	return request1.priority > request2.priority;
}

bool operator> (const Request& request1, const Request& request2)
{
	return request1.priority < request2.priority;
}


/* priority queues */
priority_queue<Request, vector<Request>, greater<vector<Request>::value_type> > *pqGPU;
priority_queue<Request, vector<Request>, greater<vector<Request>::value_type> > pqPCI;
priority_queue<Request, vector<Request>, greater<vector<Request>::value_type> > pqGPUALL;
/* information about resource */
bool *runGPU;
bool runPCI;
/* locks */

pthread_mutex_t mutexGPUALL;
pthread_mutex_t *mutexGPU;
pthread_mutex_t mutexPCI;

void *manage_request (void *args)
{
	struct mq_attr attr;
	attr.mq_maxmsg = MAX_MSG;
	attr.mq_msgsize = MSG_SIZE;
	attr.mq_flags = 0;
	while(true) {
		char buf[MSG_SIZE];
		char output[MSG_SIZE];
		unsigned prio;
		/* request received */
		mq_receive (request, buf, MSG_SIZE, &prio);
		char type;
		int which;
		int pid;
		int nRequest;
		sscanf(buf, "%c %d %d %d", &type, &which, &pid, &nRequest);
		//printf("request\t%s\n", buf);
		/* if request is gpu */
		if (type == 'g') {
			/* not decided yet */
			if (which < 0) {
				int i;
				bool done = false;
				for(i = 0; i < nGPU; i++) {
					pthread_mutex_lock (&mutexGPU[i]);									
					if (!runGPU[i] && !done) {
						//printf("GPU%d availble now\n", i);
						char gpuid[MSG_SIZE];
						printf("%d gpu selected!\n", i);
						sprintf(gpuid, "%d", i);
						sprintf(output, "/%d%c%d", pid, type,which);
						//printf("%s\n", output);
						mqd_t grant = mq_open (output, O_WRONLY | O_CREAT, 0664, &attr);
						mq_send (grant, gpuid, MSG_SIZE, prio);
						runGPU[i] = true;
						done = true;
					}

				}
				//printf("xxxx\n");
				if (!done) {
					pthread_mutex_lock (&mutexGPUALL);
					//printf("push push!\n");
					pqGPUALL.push (Request (pid, prio, nRequest));
					pthread_mutex_unlock (&mutexGPUALL);
				}
					

				for(i = 0; i < nGPU; i++) 
					pthread_mutex_unlock (&mutexGPU[i]);
			}
			else {
				pthread_mutex_lock (&mutexGPU[which]);
				/* if the GPU is not available */
				if (runGPU[which]) {
					//printf("GPU%d unavailble now\n", which);
					pqGPU[which].push (Request (pid, prio, nRequest));
				}
				/* if the GPU is available */
				else {
					//printf("GPU%d availble now\n", which);
					sprintf(output, "/%d%c%d", pid, type, which);
					//printf("%s\n", output);
					mqd_t grant = mq_open (output, O_WRONLY | O_CREAT, 0664, &attr);
					mq_send (grant, output, MSG_SIZE, prio);
					runGPU[which] = true;
				}
				pthread_mutex_unlock (&mutexGPU[which]);
			}
		}
		/* if request is pci */
		else if (type == 'p') {
			pthread_mutex_lock (&mutexPCI);
			/* if the PCI is not available */
			if (runPCI) {
				pqPCI.push (Request (pid, prio, nRequest));
				//printf("PCI unavailable now\n");
			}
			/* if the PCI is available */
			else {
				sprintf(output, "/%d%c", pid, type);
				if (nRequest > 1)
					pqPCI.push (Request (pid, prio, nRequest-1));
				mqd_t grant = mq_open (output, O_WRONLY | O_CREAT, 0664, &attr);
				mq_send (grant, output, MSG_SIZE, prio);
				runPCI = true;
				//printf("PCI available now\n");
			}
			pthread_mutex_unlock (&mutexPCI);
		}
		else {
			printf("wront reqeust\n");
		}
	}
	return NULL;
}

void *manage_finish (void *args)
{
	struct mq_attr attr;
	attr.mq_maxmsg = MAX_MSG;
	attr.mq_msgsize = MSG_SIZE;
	attr.mq_flags = 0;
	while(true) {
		char buf[MSG_SIZE];
		char output[MSG_SIZE];
		unsigned prio;
		mq_receive (finish, buf, MSG_SIZE, &prio);
		char type;
		int which;
		int pid;
		int nRequest;
		sscanf(buf, "%c %d %d %d", &type, &which, &pid, &nRequest);
		//printf("finish\t%s\n", buf);
		/* if the resource is GPU */
		if (type == 'g') {
			pthread_mutex_lock (&mutexGPU[which]);
			pthread_mutex_lock (&mutexGPUALL);
			/* if the queue is empty */
			//printf("%d finish\n", which);
			if (pqGPU[which].empty() && pqGPUALL.empty()) {
				//printf("%d empty\n", which);
				runGPU[which] = false;
			}
			/* if the queue is not empty */
			else {
				if (pqGPU[which].empty())
				{
					//printf("%d queue empty\n", which);
				}
				Request r1 = pqGPU[which].empty() ? Request(0,-1,0) : pqGPU[which].top();
				Request r2 = pqGPUALL.empty() ? Request(0,-1,0) : pqGPUALL.top();
				if (r1.priority > r2.priority){
					pqGPU[which].pop();
					if (r1.numRequest <= 1);
					else {
						pqGPU[which].push(Request (r1.pid, r1.priority, r1.numRequest-1));
					}
					//printf("Dequeue:%d %d %d\n", r1.pid, r1.priority, r1.numRequest);
					sprintf(output, "/%d%c%d", r1.pid, type, which);
					mqd_t grant = mq_open (output, O_WRONLY | O_CREAT, 0664, &attr);
					mq_send (grant, output, MSG_SIZE, prio);
				}
				else {
					pqGPUALL.pop();
					if (r2.numRequest <= 1);
					else {
						pqGPUALL.push (Request(r2.pid, r2.priority, r2.numRequest-1));
					}
					//printf("Dequeue:%d %d %d\n", r2.pid, r2.priority, r2.numRequest);
					sprintf(output, "/%d%c%d", r2.pid, type, -1);
					char gpuid[MSG_SIZE];
					sprintf(gpuid, "%d", which);
					printf("%d gpu selected\n",which);
					mqd_t grant = mq_open (output, O_WRONLY | O_CREAT, 0664, &attr);
					mq_send (grant, gpuid, MSG_SIZE, prio);
				}
			}
			pthread_mutex_unlock (&mutexGPUALL);
			pthread_mutex_unlock (&mutexGPU[which]);
		}
		/* if the resource is PCI */
		else if (type == 'p') {
			pthread_mutex_lock (&mutexPCI);
			/* if the queue is empty */
			if (pqPCI.empty()) {
				runPCI = false;
				//printf("Empty queue\n");
			}
			/* if the queue is not empty */
			else {
				Request r = pqPCI.top();
				pqPCI.pop();
				//printf("Dequeue:%d %d %d\n", r.pid, r.priority, r.numRequest);
				if (r.numRequest <= 1);
				else {
					pqPCI.push(Request (r.pid, r.priority, r.numRequest-1));
				}
				sprintf(output, "/%d%c", r.pid, type);
				mqd_t grant = mq_open (output, O_WRONLY | O_CREAT, 0664, &attr);
				mq_send (grant, output, MSG_SIZE, prio);
			}
			pthread_mutex_unlock (&mutexPCI);
		}
		else {
			printf("wrong finish\n");
		}
	}
	return NULL;
}



int main (int argc, char **argv)
{
	int i;
	cl_platform_id platform;
	clGetPlatformIDs (1, &platform, NULL);
	clGetDeviceIDs (platform, CL_DEVICE_TYPE_GPU, 0, NULL, &nGPU);

	/* set this scheduler to maximum rt-priority */
	struct sched_param param;
	param.sched_priority = prio;
	sched_setscheduler (0, SCHED_FIFO, &param);

	/* open the message queues */
	struct mq_attr attr;
	attr.mq_maxmsg = MAX_MSG;
	attr.mq_msgsize = MSG_SIZE;
	attr.mq_flags = 0;
	request = mq_open ("/GPUSAM_request", O_RDONLY | O_CREAT, 0664, &attr);
	finish = mq_open ("/GPUSAM_finish", O_RDONLY | O_CREAT, 0664, &attr);

	/* create priority queues and information about resource*/
	pqGPU = new priority_queue<Request, vector<Request>, greater<vector<Request>::value_type> > [nGPU];
	runGPU = new bool[nGPU];
	for(i = 0; i < nGPU; i++) 
		runGPU[i] = false;
	runPCI = false;
	
	/* create locks */
	mutexGPU = new pthread_mutex_t[nGPU];

	for(i = 0; i < nGPU; i++) {
		if (pthread_mutex_init (&mutexGPU[i], NULL) != 0) {
			printf("mutex init failed!\n");
		}
	}
	if (pthread_mutex_init (&mutexPCI, NULL) != 0) {
		printf("mutex init failed!\n");
	}
	if (pthread_mutex_init (&mutexGPUALL, NULL) != 0) {
		printf("mutex init failed!\n");
	}

	/* create two threads (request, finish) */
	pthread_t tRequest;
	pthread_t tFinish;

	if (pthread_create(&tRequest, NULL, manage_request, NULL) != 0) {
		printf("can't create thread for requests\n");
	}
	if (pthread_create(&tFinish, NULL, manage_finish, NULL) != 0) {
		printf("can't create thread for finishes\n");
	}
	
	pthread_join (tRequest, NULL);
	pthread_join (tFinish, NULL);
	printf("Do not reach here\n");
	return 0;
}
