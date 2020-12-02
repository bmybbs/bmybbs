#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "config.h"
#include "ytht/msg.h"

static int init_newtracelogmsq() {
	int msqid;
	struct msqid_ds buf;
	msqid = msgget(BBSLOG_MSQKEY, IPC_CREAT | 0664);
	if (msqid < 0)
		return -1;
	msgctl(msqid, IPC_STAT, &buf);
	buf.msg_qbytes = 50 * 1024;
	msgctl(msqid, IPC_SET, &buf);
	return msqid;
}

void newtrace(const char *s) {
	static int disable = 0;
	static int msqid = -1;
	time_t dtime;
	char buf[512];
	char timestr[16];
	char *ptr;
	struct tm *n;
	struct mymsgbuf *msg = (struct mymsgbuf *) buf;
	if (disable)
		return;
	time(&dtime);
	n = localtime(&dtime);
	sprintf(timestr, "%02d:%02d:%02d", n->tm_hour, n->tm_min, n->tm_sec);
	snprintf(msg->mtext, sizeof (buf) - sizeof (msg->mtype), "%s %s\n", timestr, s);
	ptr = msg->mtext;
	while ((ptr = strchr(ptr, '\n'))) {
		if (!ptr[1])
			break;
		*ptr = '*';
	}
	msg->mtype = 1;
	if (msqid < 0) {
		msqid = init_newtracelogmsq();
		if (msqid < 0) {
			disable = 1;
			return;
		}
	}
	msgsnd(msqid, msg, strlen(msg->mtext), IPC_NOWAIT | MSG_NOERROR);
	return;
}

