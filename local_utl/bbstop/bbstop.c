/* bbstop.c -- compute the top login/stay/post */

#include <stdio.h>
#define REAL_INFO
#include "bbs.h"

#define NUMCOUNT 100

struct mystat {
	unsigned int key;
	int num;
};

struct userec login_q[NUMCOUNT + 1];
struct userec stay_q[NUMCOUNT + 1];
struct userec post_q[NUMCOUNT + 1];
struct userec perm_q[NUMCOUNT + 1];

struct mystat login_s[MAXUSERS+1];
struct mystat stay_s[MAXUSERS+1];
struct mystat post_s[MAXUSERS+1];
struct mystat age_s[MAXUSERS+1];
int login_c=0, stay_c=0, post_c=0 , age_c=0;
time_t now;

int
login_cmp(struct userec *b, struct userec *a)
{
	return (a->numlogins - b->numlogins);
}

int
post_cmp(struct userec *b, struct userec *a)
{
	return (a->numposts - b->numposts);
}

int
stay_cmp(struct userec *b, struct userec *a)
{
	return (a->stay - b->stay);
}

int
perm_cmp(struct userec *b, struct userec *a)
{
	return (a->numlogins / 3 + a->numposts + a->stay / 3600) -
		(b->numlogins / 3 + b->numposts + b->stay / 3600);
}

void
top_login(FILE * fp)
{
	int i, j, rows = (NUMCOUNT + 1) / 2;
	char buf1[80], buf2[80];

	fprintf(fp, "\n\n%s", "\
\033[1;37m              ===========  \033[1;36m  上站次数排行榜 \033[37m   ============ \n\n\
名次 代号       昵称           次数    名次 代号       昵称            次数 \n\
==== ===============================   ==== ================================\n\033[0m\
");
	for (i = 0; i < rows; i++) {
		snprintf(buf1, sizeof buf1, "[%2d] %-10.10s %-14.14s %3d",
			i + 1, login_q[i].userid, login_q[i].username,
			login_q[i].numlogins);
		j = i + rows;
		snprintf(buf2, sizeof buf2, "[%2d] %-10.10s %-14.14s   %3d",
			j + 1, login_q[j].userid, login_q[j].username,
			login_q[j].numlogins);

		fprintf(fp, "%-39.39s%-39.39s\033[m\n", buf1, buf2);
	}
}

void
top_stay(FILE * fp)
{
	int i, j, rows = (NUMCOUNT + 1) / 2;
	char buf1[80], buf2[80];

	fprintf(fp, "\n\n\n%s", "\
\033[1;37m              ===========   \033[36m 上站总时数排行榜 \033[37m   ============ \n\n\
名次 代号       昵称           总时数  名次 代号       昵称           总时数 \n\
==== ================================  ==== ================================\n\033[0m\
");
	for (i = 0; i < rows; i++) {
		snprintf(buf1, sizeof buf1, "[%2d] %-10.10s %-14.14s%4ld:%2ld",
			i + 1, stay_q[i].userid, stay_q[i].username,
			stay_q[i].stay / 3600, (stay_q[i].stay % 3600) / 60);
		j = i + rows;
		snprintf(buf2, sizeof buf2, "[%2d] %-10.10s %-14.14s%4ld:%2ld",
			j + 1, stay_q[j].userid, stay_q[j].username,
			stay_q[j].stay / 3600, (stay_q[j].stay % 3600) / 60);

		fprintf(fp, "%-39.39s%-39.39s\033[m\n", buf1, buf2);
	}
}

void
top_post(FILE * fp)
{
	int i, j, rows = (NUMCOUNT + 1) / 2;
	char buf1[80], buf2[80];

	fprintf(fp, "\n\n\n%s", "\
\033[1;37m              ===========  \033[36m  讨论次数排行榜 \033[37m   ============ \n\n\
名次 代号       昵称           次数    名次 代号       昵称            次数 \n\
==== ===============================  ===== ================================\n\033[0m\
");
	for (i = 0; i < rows; i++) {
		snprintf(buf1, sizeof buf1, "[%2d] %-10.10s %-14.14s %3d",
			i + 1, post_q[i].userid, post_q[i].username,
			post_q[i].numposts);
		j = i + rows;
		snprintf(buf2, sizeof buf2, "[%2d] %-10.10s %-14.14s   %3d",
			j + 1, post_q[j].userid, post_q[j].username,
			post_q[j].numposts);

		fprintf(fp, "%-39.39s%-39.39s\033[m\n", buf1, buf2);
	}
}

void
top_perm(FILE * fp)
{
	int i, j, rows = (NUMCOUNT + 1) / 2;
	char buf1[80], buf2[80];

	fprintf(fp, "\n\n\n%s", "\
\033[1;37m              ===========    \033[36m总表现积分排行榜\033[37m    ============ \n\
\033[32m                    公式：上站次数/3+文章数+上站几小时\033[37m\n\
名次 代号       昵称            积分   名次 代号       昵称              积分 \n\
==== ===============================   ==== =================================\n\033[0m\
");
	for (i = 0; i < rows; i++) {
		snprintf(buf1, sizeof buf1, "[%2d] %-10.10s %-14.14s %5ld",
			i + 1, perm_q[i].userid, perm_q[i].username,
			(perm_q[i].numlogins / 3) + perm_q[i].numposts +
			(perm_q[i].stay / 3600));
		j = i + rows;
		snprintf(buf2, sizeof buf2, "[%2d] %-10.10s %-14.14s   %5ld",
			j + 1, perm_q[j].userid, perm_q[j].username,
			(perm_q[j].numlogins / 3) + perm_q[j].numposts +
			(perm_q[j].stay / 3600));

		fprintf(fp, "%-39.39s%-39.39s\033[m\n", buf1, buf2);
	}
}

void
insert_data(struct userec *queue, int comp_f(struct userec *, struct userec *), struct userec *aman)
{
	int i, j;
	if (comp_f(&(queue[NUMCOUNT - 1]), aman) <= 0)
		return;
	for (i = NUMCOUNT - 1; i >= 0; i--) {
		j = (*comp_f) (&(queue[i]), aman);
		if (j > 0)
			memcpy(&(queue[i + 1]), &(queue[i]), sizeof (struct userec));
		else
			break;
	}
	memcpy(&(queue[i + 1]), aman, sizeof (struct userec));
}

void
insert_logins(struct userec *aman)
{
	int i;
	for (i=login_c - 1; i>=0; i--) {
		if (aman->numlogins == login_s[i].key) {
			login_s[i].num++;
			return;
		}
	}
	login_s[login_c].key=aman->numlogins;
	login_s[login_c].num=1;
	login_c ++;
}

void
insert_stays(struct userec *aman)
{
	int i;
	for (i=stay_c - 1; i>=0; i--) {
		if (aman->stay / 60 / 60 == stay_s[i].key) {
			stay_s[i].num++;
			return;
		}
	}
	stay_s[stay_c].key=aman->stay / 60 / 60;
	stay_s[stay_c].num=1;
	stay_c ++;
}

void
insert_posts(struct userec *aman)
{
	int i;
	for (i=post_c - 1; i>=0; i--) {
		if (aman->numposts == post_s[i].key) {
			post_s[i].num++;
			return;
		}
	}
	post_s[post_c].key=aman->numposts;
	post_s[post_c].num=1;
	post_c ++;
}

void
insert_ages(struct userec *aman)
{
	int i;
	long key;
	key = (now - aman->firstlogin)/60/60/24;
	for (i=age_c - 1; i>=0; i--) {
		if (key == age_s[i].key) {
			age_s[i].num++;
			return;
		}
	}
	age_s[age_c].key=key;
	age_s[age_c].num=1;
	age_c ++;
}

void
output(char *file, int mode)
{
	FILE *fp;
	char fn[80];
	snprintf(fn, sizeof fn, MY_BBS_HOME "/%s", file);
	fp = fopen(fn, "w");
	if (fp) {
		switch (mode) {
		case 1:
			top_login(fp);
			break;
		case 2:
			top_post(fp);
			break;
		case 3:
			top_stay(fp);
			break;
		case 4:
			top_perm(fp);
			break;
		}
		fclose(fp);
	}
}

int cmpstat(const void *a, const void *b) {
	return (((struct mystat *)b)->key - ((struct mystat *)a)->key);
}

void
output_c(char *file, int mode)
{
	int fd;
	char fn[80];
	int count;
	int i;
	struct mystat *ms;
	switch (mode) {
		case 1:
			count = login_c;
			ms = login_s;
			break;
		case 2:
			count = post_c;
			ms = post_s;
			break;
		case 3:
			count = stay_c;
			ms = stay_s;
			break;
		case 4:
			count = age_c;
			ms = age_s;
			break;
		default:
			ms = NULL;
			count = 0;
	}
	if (ms == NULL)
		return;
	snprintf(fn, sizeof fn, MY_BBS_HOME "/%s", file);
	fd = open(fn, O_WRONLY | O_CREAT, 0660);
	if (fd) {
		qsort(ms, count, sizeof(struct mystat), cmpstat);
		for (i=1; i<count;i++) {
			(ms+i)->num += (ms+i-1)->num;
		}
		write(fd, ms, count*sizeof(struct mystat));
		close(fd);
	}
}
int
main(int argc, char **argv)
{
	(void) argc;
	(void) argv;
	FILE *inf;
	struct userec aman;
	char passwd_file[256];
	snprintf(passwd_file, sizeof passwd_file, MY_BBS_HOME "/" PASSFILE);

	inf = fopen(passwd_file, "rb");

	now = time(NULL);
	if (inf == NULL) {
		printf("Sorry, the data is not ready.\n");
		exit(0);
	}
	bzero(login_q, sizeof (login_q));
	bzero(stay_q, sizeof (stay_q));
	bzero(post_q, sizeof (post_q));
	bzero(perm_q, sizeof (perm_q));

	for (;;) {
		if (fread(&aman, sizeof (aman), 1, inf) <= 0)
			break;
		if (!aman.userid[0])
			continue;
		if (aman.userlevel == 0)
			continue;
		insert_logins(&aman);
		insert_stays(&aman);
		insert_ages(&aman);
		insert_posts(&aman);
		aman.userid[IDLEN + 1] = 0;
		if (strcmp(aman.userid, "guest") == 0)
			continue;
		insert_data(login_q, (void *) login_cmp, &aman);
		insert_data(stay_q, (void *) stay_cmp, &aman);
		insert_data(post_q, (void *) post_cmp, &aman);
		insert_data(perm_q, (void *) perm_cmp, &aman);
	}
	output("/0Announce/bbslist/toplogin", 1);
	output("/0Announce/bbslist/toppost", 2);
	output("/0Announce/bbslist/topstay", 3);
	output("/0Announce/bbslist/topall", 4);
	output_c("/wwwtmp/stat_login", 1);
	output_c("/wwwtmp/stat_post", 2);
	output_c("/wwwtmp/stat_stay", 3);
	output_c("/wwwtmp/stat_age", 4);
}
