#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include "ythtbbs/ythtbbs.h"

int
init_bbsevamsq()
{
	int msqid;
	struct msqid_ds buf;
	msqid = msgget(BBSEVA_MSQKEY, IPC_CREAT | 0664);
	if (msqid < 0)
		return -1;
	msgctl(msqid, IPC_STAT, &buf);
	if (buf.msg_qbytes != 50 * 1024) {
		buf.msg_qbytes = 50 * 1024;
		msgctl(msqid, IPC_SET, &buf);
	}
	return msqid;
}

static int
bbseva_qry(int ent, char *s, int size, int needreply)
{
	static int disable = 0;
	static int msqid = -1;
	long msgtype;
	int i, retv;
	char buf[512];
	struct mymsgbuf *msg = (struct mymsgbuf *) buf;
	if (disable)
		return -1;
	msg->mtype = 1;
	msgtype = time(NULL);
	snprintf(msg->mtext, sizeof (buf) - sizeof (msg->mtype),
		 "%ld %d %s", msgtype, ent, s);
	if (msqid < 0) {
		msqid = init_bbsevamsq();
		if (msqid < 0) {
			disable = 1;
			return -1;
		}
	}
	if (msgsnd(msqid, msg, strlen(msg->mtext), IPC_NOWAIT | MSG_NOERROR) <
	    0) return -1;
	if (!needreply)
		return 0;
	for (i = 0; i < 8; i++) {
		usleep(300000l);
		retv =
		    msgrcv(msqid, msg, sizeof (buf) - sizeof (msg->mtype) - 1,
			   msgtype, IPC_NOWAIT | MSG_NOERROR);
		if (retv > 0) {
			msg->mtext[retv] = 0;
			if (atoi(msg->mtext) != ent)
				return -1;
			if (!strchr(msg->mtext, ' '))
				return -1;
			ytht_strsncpy(s, strchr(msg->mtext, ' ') + 1, size);
			return 0;
		}
	}
	return -1;
}

int
bbseva_askoneid(int ent, char *board, char *filename, char *id)
{
	char buf[256];
	snprintf(buf, sizeof (buf), "ONEID %s %s %s", board, filename, id);
	if (bbseva_qry(ent, buf, sizeof (buf), 1) < 0)
		return -1;
	return atoi(buf);
}

int
bbseva_askavg(int ent, char *board, char *filename, float *avg)
{
	char buf[256];
	int retv;
	snprintf(buf, sizeof (buf), "AVG %s %s", board, filename);
	if (bbseva_qry(ent, buf, sizeof (buf), 1) < 0)
		return -1;
	if (sscanf(buf, "%d%f", &retv, avg) != 2)
		return -1;
	return retv;
}

int
bbseva_set(int ent, char *board, char *filename, char *id, int star)
{
	char buf[256];
	snprintf(buf, sizeof (buf), "SET %s %s %s %d", board, filename, id,
		 star);
	return bbseva_qry(ent, buf, sizeof (buf), 0);
}

int
bbseva_qset(int ent, char *board, char *filename, char *id, int star,
	    int *oldstar, int *count, float *avg)
{
	char buf[256];
	int retv;
	snprintf(buf, sizeof (buf), "QSET %s %s %s %d", board, filename, id,
		 star);
	if (bbseva_qry(ent, buf, sizeof (buf), 1) < 0)
		return -1;
	retv = sscanf(buf, "%d%d%f", oldstar, count, avg);
	if (retv == 1) {
		if (*oldstar != star)
			return -1;
		return 0;
	}
	if (retv == 3)
		return 0;
	return -1;
}
