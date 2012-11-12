#ifdef ENABLE_MYSQL
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "bbs.h"
#include "usesql.h"
//#define DEBUG 1

MYSQL *sql = NULL;

int
connsql()
{
	if (sql)
		return 0;
	sql = mysql_init(NULL);
	mysql_real_connect(sql, "localhost", SQLUSER, SQLPASSWD, SQLDB, 0, NULL,
			   0);
	return 0;
}

void
closesql()
{
	if (sql) {
		mysql_close(sql);
		sql = NULL;
	}
}

struct cmdlist {
	char *str;
	int (*func) (char *, char *);
};

int
qry_oneid(char *qry, char *result)
{
	char *filename, *board, *id;
	char sel[2048];
	int retv = -1, star;
	MYSQL_RES *res;
	MYSQL_ROW est;
	board = strtok(qry, " ");
	if (!board)
		return -1;
	filename = strtok(NULL, " ");
	if (!filename)
		return -1;
	id = strtok(NULL, " ");
	if (!id)
		return -1;
	snprintf(sel, sizeof (sel),
		 "select class from articlevote where filename = '%s' and board ='%s' and id ='%s';",
		 filename, board, id);
	if (mysql_query(sql, sel))
		return -1;
	res = mysql_store_result(sql);
	if (mysql_num_rows(res)) {
		est = mysql_fetch_row(res);
		star = atoi(est[0]);
		sprintf(result, "%d", star);
		retv = 1;
	}
	mysql_free_result(res);
	if (retv != 1)
		sprintf(result, "0");
	return 1;
}

int
qry_avg(char *qry, char *result)
{
	char *filename, *board;
	char sel[2048];
	int retv = 0, count;
	float avg;
	MYSQL_RES *res;
	MYSQL_ROW est;
	board = strtok(qry, " ");
	if (!board)
		return -1;
	filename = strtok(NULL, " ");
	if (!filename)
		return -1;
	sprintf(sel,
		"select AVG(class),COUNT(*) from articlevote where filename = '%s' and board = '%s' ;",
		filename, board);
	if (mysql_query(sql, sel))
		return -1;
	res = mysql_store_result(sql);
	if (mysql_num_rows(res)) {
		est = mysql_fetch_row(res);
		avg = atof(est[0]);
		count = atoi(est[1]) > 255 ? 255 : atoi(est[1]);
		sprintf(result, "%d %f", count, avg);
		retv = 1;
	}
	mysql_free_result(res);
	return retv;
}

int
qry_set(char *qry, char *result)
{
	return 0;
}

int
qry_qset(char *qry, char *result)
{
	char *filename, *board, *id, *ptr;
	char sel[2048];
	int retv = -1, star, oldstar, count, changed = 0;
	float avg;
	MYSQL_RES *res;
	MYSQL_ROW est;
	board = strtok(qry, " ");
	if (!board)
		return -1;
	filename = strtok(NULL, " ");
	if (!filename)
		return -1;
	id = strtok(NULL, " ");
	if (!id)
		return -1;
	ptr = strtok(NULL, " ");
	if (!ptr)
		return -1;
	star = atoi(ptr);
	if (star <= 0)
		return -1;
	snprintf(sel, sizeof (sel),
		 "select class from articlevote where filename = '%s' and board ='%s' and id ='%s';",
		 filename, board, id);
	if (mysql_query(sql, sel))
		return -1;
	res = mysql_store_result(sql);
	if (mysql_num_rows(res)) {
		est = mysql_fetch_row(res);
		oldstar = atoi(est[0]);
		if (star == oldstar) {
			sprintf(result, "%d", oldstar);
			retv = 1;
		} else {
			sprintf(sel,
				"update articlevote set class=%d where filename = '%s' and board ='%s' and id ='%s ';",
				star, filename, board, id);
			mysql_query(sql, sel);
			changed = 1;
		}
	} else {
		oldstar = 0;
		sprintf(sel,
			"insert into articlevote (filename,board,id,class) values ('%s','%s','%s',%d);",
			filename, board, id, star);
		mysql_query(sql, sel);
		changed = 1;
	}
	mysql_free_result(res);
	if (retv == 1)
		return retv;
	if (changed) {
		sprintf(sel,
			"select AVG(class),COUNT(*) from articlevote where filename = '%s' and board = '%s' ;",
			filename, board);
		if (mysql_query(sql, sel))
			return -1;
		res = mysql_store_result(sql);
		count = mysql_num_rows(res);
		if (count) {
			est = mysql_fetch_row(res);
			avg = atof(est[0]);
			count = atoi(est[1]);
			sprintf(result, "%d %d %f", oldstar, count, avg);
			retv = 1;
		}
		mysql_free_result(res);
	}
	return retv;
}

struct cmdlist cmdlist[] = {
	{"ONEID", qry_oneid},
	{"AVG", qry_avg},
	{"SET", qry_set},
	{"QSET", qry_qset},
	{NULL, NULL}
};
void
sigalarm_handler(int i)
{
	closesql();
}

char *
rcvqry(int msqid, int nowait)
{
	static char buf[1024];
	struct mymsgbuf *msgp = (struct mymsgbuf *) buf;
	int retv;
      AGAIN:
	retv =
	    msgrcv(msqid, msgp,
		   sizeof (buf) - sizeof (msgp->mtype) - 1,
		   -(time(NULL) - 10), (nowait ? IPC_NOWAIT : 0) | MSG_NOERROR);
	if (retv >= 0 && msgp->mtype != 1)
		goto AGAIN;
	while (retv > 0 && msgp->mtext[retv - 1] == 0)
		retv--;
	if (retv <= 0)
		return NULL;
	msgp->mtext[retv] = 0;
	return msgp->mtext;
}

int
sendresult(int msqid, long type, int ent, char *result)
{
	static char buf[1024];
	struct mymsgbuf *msgp = (struct mymsgbuf *) buf;
	msgp->mtype = type;
	snprintf(msgp->mtext, sizeof (buf) - sizeof (msgp->mtype), "%d %s",
		 ent, result);
	return msgsnd(msqid, msgp, strlen(msgp->mtext),
		      IPC_NOWAIT | MSG_NOERROR);}

int
main()
{
	char *qry, result[1024], *ptr;
	int n = 0, fd, msqid, i, retv, ent;
	long type;
	umask(027);
	msqid = init_bbsevamsq();
	if (msqid < 0)
		return -1;
#ifndef DEBUG
	if (fork())
		return 0;
	setsid();
	if (fork())
		return 0;
	close(0);
	close(1);
	close(2);
#endif
	fd = open(MY_BBS_HOME "/.bbsevad.lock", O_CREAT | O_RDONLY, 0660);
	if (flock(fd, LOCK_EX | LOCK_NB) < 0)
		return -1;
	while (1) {
		signal(SIGALRM, sigalarm_handler);
		alarm(60);
		qry = rcvqry(msqid, 0);
		alarm(0);
		if (!qry)
			continue;
		if (connsql() < 0)
			exit(0);
		type = atol(qry);
		qry = strchr(qry, ' ');
		if (!qry)
			continue;
		qry++;
		ent = atoi(qry);
		qry = strchr(qry, ' ');
		if (!qry)
			continue;
		qry++;
		ptr = strchr(qry, ' ');
		if (!ptr)
			continue;
		ptr++;
		for (i = 0; cmdlist[i].str; i++) {
			if (!strncmp
			    (qry, cmdlist[i].str, strlen(cmdlist[i].str))) {
				retv = cmdlist[i].func(ptr, result);
				if (retv > 0)
					sendresult(msqid, type, ent, result);
				break;
			}
		}
		n++;
#ifdef DEBUG
		printf("-");
		fflush(stdout);
#endif
		if (n < 300)
			continue;
		closesql();
		n = 0;
	}
}
#else
int
main()
{
	return 0;
}
#endif
