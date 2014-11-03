#include <stdio.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <fcntl.h>

#define MAX_MSG 10
#define MSG_SIZE 15

int main() {
	struct mq_attr attr;
	attr.mq_maxmsg = MAX_MSG;
	attr.mq_msgsize = MSG_SIZE;
	attr.mq_flags = 0;
	mqd_t request = mq_open ("/GPUSAM_request", O_WRONLY | O_CREAT, 0664, &attr);
	mqd_t finish = mq_open ("/GPUSAM_finish", O_WRONLY | O_CREAT, 0664, &attr);
	
	char buf[MSG_SIZE];
	int pid = getpid();
	printf("%d\n", pid);
	sprintf(buf, "%c %d %d %d", 'g', 1, pid, 1);
	printf("%s\n", buf);
	mq_send (request, buf, MSG_SIZE, 1);
	mq_send (request, buf, MSG_SIZE, 1);
	usleep (1000);
	mq_send (finish, buf, MSG_SIZE, 1);
	mq_send (finish, buf, MSG_SIZE, 1);

	return 0;
}
