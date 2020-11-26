#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ght_hash_table.h>
#include <signal.h>
#include "bbs.h"

int
init_bbslogmsq()
{
	int msqid;
	struct msqid_ds buf;
	msqid = msgget(BBSLOG_MSQKEY, IPC_CREAT | 0664);
	if (msqid < 0)
		return -1;
	msgctl(msqid, IPC_STAT, &buf);
	buf.msg_qbytes = 1024 * 50;
	msgctl(msqid, IPC_SET, &buf);
	return msqid;
}

char *
rcvlog(int msqid, int nowait)
{
	static char buf[1024];
	struct mymsgbuf *msgp = (struct mymsgbuf *) buf;
	int retv;
	retv = msgrcv(msqid, msgp, sizeof (buf) - sizeof (msgp->mtype) - 2, 0, (nowait ? IPC_NOWAIT : 0) | MSG_NOERROR);
	while (retv > 0 && msgp->mtext[retv - 1] == 0)
		retv--;
	if (retv <= 0)
		return NULL;
	if (msgp->mtext[retv - 1] != '\n') {
		msgp->mtext[retv] = '\n';
		retv++;
	}
	msgp->mtext[retv] = 0;
	return msgp->mtext;
}

char *
getfilename()
{
	static char logf[256];
	time_t dtime;
	struct tm *n;
	time(&dtime);
	n = localtime(&dtime);
	sprintf(logf, MY_BBS_HOME "/newtrace/%d-%02d-%02d.log",
		1900 + n->tm_year, 1 + n->tm_mon, n->tm_mday);
	return logf;
}

struct event {
	struct event *next;
	struct in_addr from;
	time_t t;
} *eventhead = NULL, *eventtail = NULL;

struct bansite {
	struct in_addr from;
	time_t t;
};

struct sitelimit {
	struct in_addr from;
	int limit;
};

#define SPEC_SITE_FILE MY_BBS_HOME "/etc/spec_site"
#define MAX_SPEC_SITE 512
#define DEFAULT_LIMIT 5
struct sitelimit limit[MAX_SPEC_SITE];
int sitelimit_cnt;

ght_hash_table_t *passerrtable = NULL;
ght_hash_table_t *bansitetable = NULL;

int
m_cmp(struct sitelimit *a, struct sitelimit *b)
{
	if (a->from.s_addr > b->from.s_addr)
		return 1;
	else if (a->from.s_addr == b->from.s_addr)
		return 0;
	else
		return -1;
}

void
load_limit_table(void)
{
	static time_t update_time = 0;
	time_t t;
	int i = 0;
	FILE *fp;
	char buf[256], *ptr;
	t = file_time(SPEC_SITE_FILE);
	if (update_time > t)
		return;
	fp = fopen(SPEC_SITE_FILE, "r");
	if (NULL == fp)
		return;
	while (fgets(buf, sizeof (buf), fp) && i < MAX_SPEC_SITE) {
		if ('#' == buf[0])
			continue;
		ptr = strtok(buf, " ");
		if (!ptr)
			continue;
		*ptr = 0;
		inet_aton(buf, &(limit[i].from));
		limit[i].limit = atoi(ptr + 1);
		if (limit[i].limit <= 0)
			limit[i].limit = DEFAULT_LIMIT;
		i++;
	}
	fclose(fp);
	qsort(limit, i, sizeof (struct sitelimit), (void *) m_cmp);
	sitelimit_cnt = i;
	update_time = time(0);
}

int
site_limit(struct in_addr *from)
{
	struct sitelimit *ret;
	struct sitelimit tmp = { *from, 0 };
	load_limit_table();
	if(!sitelimit_cnt)
		return DEFAULT_LIMIT;
	ret = bsearch(&tmp, limit, sitelimit_cnt, sizeof (struct sitelimit), (void *) m_cmp);
	if (!ret)
		return DEFAULT_LIMIT;
	return ret->limit;
}

int
bansiteop(struct in_addr *from)
{
	struct bansite *b;
	ght_iterator_t iterator;
	ght_hash_table_t *tmptable;
	static time_t last = 0;
	time_t now_t = time(NULL);
	FILE *fp;
	if (!bansitetable)
		bansitetable = ght_create(200, NULL, 0);
	if (!bansitetable)
		return -1;
	if (from) {
		if ((b = ght_get(bansitetable, sizeof (*from), from))) {
			b->t = now_t;
			return 0;
		}
		b = malloc(sizeof (*b));
		if (!b)
			return -1;
		b->t = time(NULL);
		b->from = *from;
		if (ght_insert(bansitetable, b, sizeof (*from), from) < 0) {
			free(b);
			return -1;
		}
	}
	if (!from && now_t - last < 100)
		return 0;
	fp = fopen(MY_BBS_HOME "/bbstmpfs/dynamic/bansite", "w");
	if (!fp)
		return -1;
	last = now_t;
	tmptable = bansitetable;
	bansitetable = ght_create(200, NULL, 0);
	if (!bansitetable) {
		bansitetable = tmptable;
		return -1;
	}
	for (b = ght_first(tmptable, &iterator); b; b = ght_next(tmptable, &iterator)) {
		if (b->t < now_t - 6000) {
			free(b);
			continue;
		}
		fprintf(fp, "%s\n", inet_ntoa(b->from));
		if (ght_insert(bansitetable, b, sizeof (b->from), &b->from) < 0) {
			free(b);
		}
	}
	ght_finalize(tmptable);
	fclose(fp);
	return 0;
}

int
filter_passerr(int n, char *arg[])
{
	struct event *e;
	int *count;
	if (n < 4 || strcmp(arg[1], "system") || strcmp(arg[2], "passerr"))
		return 0;
	if (passerrtable == NULL)
		passerrtable = ght_create(10000, NULL, 0);
	if (!passerrtable)
		goto ERROR;
	e = malloc(sizeof (struct event));
	if (!e)
		goto ERROR;
	e->t = time(NULL);
	inet_aton(arg[3], &e->from);
	if ((count = ght_get(passerrtable, sizeof (e->from), &e->from))) {
		(*count)++;
		if (*count > site_limit(&e->from))
			bansiteop(&e->from);
	} else {
		count = malloc(sizeof (int));
		if (!count)
			goto ERROR1;
		*count = 1;
		if (ght_insert(passerrtable, count, sizeof (e->from), &e->from) < 0)
			goto ERROR2;
	}
	//add to list
	if (eventhead == NULL) {
		eventhead = e;
		eventtail = e;
	} else {
		eventtail->next = e;
		eventtail = e;
	}
	e->next = NULL;
	return 1;
ERROR2:
	free(count);
ERROR1:
	free(e);
ERROR:
	return 1;
}

int
expireevent()
{
	struct event *e;
	int *count;
	time_t now_t = time(NULL);
	while (eventhead) {
		e = eventhead;
		if (e->t > now_t - 600)
			break;
		eventhead = e->next;
		count = ght_get(passerrtable, sizeof (e->from), &e->from);
		(*count)--;
		if (!*count) {
			ght_remove(passerrtable, sizeof (e->from), &e->from);
			free(count);
		}
		free(e);
	}
	if (!eventhead)
		eventtail = NULL;
	return 0;
}

int
filterlog(char *str)
{
	char *ptr;
	char buf[512];
	char fields[6][512];
	char *tmp[6] = { fields[0], fields[1], fields[2], fields[3], fields[4],
		fields[5]
	};
	int n, retv;
	strsncpy(buf, str, sizeof (buf));
	if ((ptr = strchr(buf, '\n')))
		*ptr = 0;
	n = mystrtok(buf, ' ', tmp, 6);
	retv = filter_passerr(n, tmp);
	expireevent();
	bansiteop(NULL);
	return retv;
}

int sync_flag = 0;
int fd = -1;
char buf[100*1024];
int n = 0;

static void
set_sync_flag(int signno){
	sync_flag = 1;
}

static void
write_back_all(int signno) {
	if (fd >= 0)
		write(fd, buf, n);
	exit(0);
}

int
main()
{
	char lastch = 0, *str;
	int msqid, i;
	size_t l;

	umask(027);

	msqid = init_bbslogmsq();
	if (msqid < 0)
		return -1;

	if (fork())
		return 0;
	setsid();
	if (fork())
		return 0;

	close(0);
	close(1);
	close(2);

	fd = open(MY_BBS_HOME "/.bbslogd.lock", O_CREAT | O_RDONLY, 0660);
	if (flock(fd, LOCK_EX | LOCK_NB) < 0)
		return -1;

	signal(SIGHUP, set_sync_flag);
	signal(SIGTERM, write_back_all);
	fd = open(getfilename(), O_WRONLY | O_CREAT | O_APPEND, 0660);
	i = 0;
	while (1) {
		while ((str = rcvlog(msqid, 1))) {
			filterlog(str);
			if (str[0] != lastch) {
				if (fd >= 0) {
					write(fd, buf, n);
					n = 0;
					i = 0;
					close(fd);
				}
				fd = open(getfilename(), O_WRONLY | O_CREAT | O_APPEND, 0660);
				lastch = str[0];
			}
			l = strlen(str);
			if (l > sizeof (buf) - n) {
				write(fd, buf, n);
				sync_flag = 0;
				n = 0;
				i = 0;
			}
			memcpy(buf + n, str, l);
			n += l;
		}
		sleep(1);
		i++;
		if (i< 30 && !sync_flag)
			continue;
		write(fd, buf, n);
		n = 0;
		i = 0;
		sync_flag = 0;
	}
}
