#include "nbstat.h"
#include "bbs.h"
#include "strhash.h"

#define BMSTAT MY_BBS_HOME"/0Announce/bmstat"
#define BMSTATA MY_BBS_HOME"/0Announce/groups/GROUP_0/syssecurity/bmstata"

diction bmd;

struct bmstat {
	char board[20];
	char class[10];
	char userid[20];
	char noread;
	int inboard;
	int stay;
	int post;
	int digest;
	int mark;
	int range;
	int delete;
	int import;
	int move;
	int same;
	int deny;
	int unmark;
	int undigest;
	int boardscore;
};

void
bm_posted(int day, char *time, char *user, char *other)
{
	struct hword *a;
	struct bmstat *data;
	char buf[STRLEN];
	char board[30], title[128];
	char *tmp[2] = { board, title };
	int i;
	i = ytht_strtok(other, ' ', tmp, 2);
	snprintf(buf, STRLEN - 1, "%s %s", board, user);
	a = finddic(bmd, buf);
	if (a != NULL) {
		data = a->value;
		data->post++;
	}
}

void
bm_use(int day, char *time, char *user, char *other)
{
	struct hword *a;
	struct bmstat *data;
	char buf[STRLEN];
	char board[30], staytime[30];
	char *tmp[2] = { board, staytime };
	int i;
	i = ytht_strtok(other, ' ', tmp, 2);
	snprintf(buf, STRLEN - 1, "%s %s", board, user);
	a = finddic(bmd, buf);
	if (a != NULL) {
		data = a->value;
		data->inboard++;
		data->stay += atoi(staytime);
	}
}

void
bm_import(int day, char *time, char *user, char *other)
{
	struct hword *a;
	struct bmstat *data;
	char buf[STRLEN];
	char board[30], author[20], title[128];
	char *tmp[3] = { board, author, title };
	int i;
	i = ytht_strtok(other, ' ', tmp, 3);
	snprintf(buf, STRLEN - 1, "%s %s", board, user);
	a = finddic(bmd, buf);
	if (a != NULL) {
		data = a->value;
		data->import++;
	}
}

void
bm_move(int day, char *time, char *user, char *other)
{
	struct hword *a;
	struct bmstat *data;
	char buf[STRLEN];
	char board[30], path[512];
	char *tmp[2] = { board, path };
	int i;
	i = ytht_strtok(other, ' ', tmp, 2);
	snprintf(buf, STRLEN - 1, "%s %s", board, user);
	a = finddic(bmd, buf);
	if (a != NULL) {
		data = a->value;
		data->move++;
	}
}

void
bm_undigest(int day, char *time, char *user, char *other)
{
	struct hword *a;
	struct bmstat *data;
	char buf[STRLEN];
	char board[30], author[20], title[128];
	char *tmp[3] = { board, author, title };
	int i;
	i = ytht_strtok(other, ' ', tmp, 3);
	snprintf(buf, STRLEN - 1, "%s %s", board, user);
	a = finddic(bmd, buf);
	if (a != NULL) {
		data = a->value;
		data->undigest++;
	}
}

void
bm_digest(int day, char *time, char *user, char *other)
{
	struct hword *a;
	struct bmstat *data;
	char buf[STRLEN];
	char board[30], author[20], title[128];
	char *tmp[3] = { board, author, title };
	int i;
	i = ytht_strtok(other, ' ', tmp, 3);
	snprintf(buf, STRLEN - 1, "%s %s", board, user);
	a = finddic(bmd, buf);
	if (a != NULL) {
		data = a->value;
		data->digest++;
	}
}

void
bm_mark(int day, char *time, char *user, char *other)
{
	struct hword *a;
	struct bmstat *data;
	char buf[STRLEN];
	char board[30], author[20], title[128];
	char *tmp[3] = { board, author, title };
	int i;
	i = ytht_strtok(other, ' ', tmp, 3);
	snprintf(buf, STRLEN - 1, "%s %s", board, user);
	a = finddic(bmd, buf);
	if (a != NULL) {
		data = a->value;
		data->mark++;
	}
}

void
bm_unmark(int day, char *time, char *user, char *other)
{
	struct hword *a;
	struct bmstat *data;
	char buf[STRLEN];
	char board[30], author[20], title[128];
	char *tmp[3] = { board, author, title };
	int i;
	i = ytht_strtok(other, ' ', tmp, 3);
	snprintf(buf, STRLEN - 1, "%s %s", board, user);
	a = finddic(bmd, buf);
	if (a != NULL) {
		data = a->value;
		data->unmark++;
	}
}

void
bm_range(int day, char *time, char *user, char *other)
{
	struct hword *a;
	struct bmstat *data;
	char buf[STRLEN];
	char board[30], num1[10], num2[10];
	char *tmp[3] = { board, num1, num2 };
	int i;
	i = ytht_strtok(other, ' ', tmp, 3);
	snprintf(buf, STRLEN - 1, "%s %s", board, user);
	a = finddic(bmd, buf);
	if (a != NULL) {
		data = a->value;
		data->range++;
	}
}

void
bm_del(int day, char *time, char *user, char *other)
{
	struct hword *a;
	struct bmstat *data;
	char buf[STRLEN];
	char board[30], author[20], title[128];
	char *tmp[3] = { board, author, title };
	int i;
	i = ytht_strtok(other, ' ', tmp, 3);
	snprintf(buf, STRLEN - 1, "%s %s", board, user);
	a = finddic(bmd, buf);
	if (a != NULL) {
		data = a->value;
		data->delete++;
	}
}

void
bm_deny(int day, char *time, char *user, char *other)
{
	struct hword *a;
	struct bmstat *data;
	char buf[STRLEN];
	char board[30], denyuser[20];
	char *tmp[2] = { board, denyuser };
	int i;
	i = ytht_strtok(other, ' ', tmp, 2);
	snprintf(buf, STRLEN - 1, "%s %s", board, user);
	a = finddic(bmd, buf);
	if (a != NULL) {
		data = a->value;
		data->deny++;
	}
}

void
bm_same(int day, char *time, char *user, char *other)
{
	struct hword *a;
	struct bmstat *data;
	char buf[STRLEN];
	char board[30], title[128];
	char *tmp[2] = { board, title };
	int i;
	i = ytht_strtok(other, ' ', tmp, 2);
	snprintf(buf, STRLEN - 1, "%s %s", board, user);
	a = finddic(bmd, buf);
	if (a != NULL) {
		data = a->value;
		data->same++;
	}
}

struct action_f bm[] = {
	{"post", bm_posted},
	{"use", bm_use},
	{"import", bm_import},
	{"additem", bm_move},
	{"moveitem", bm_move},
	{"paste", bm_move},
	{"undigest", bm_undigest},
	{"digest", bm_digest},
	{"mark", bm_mark},
	{"unmark", bm_unmark},
	{"ranged", bm_range},
	{"del", bm_del},
	{"deny", bm_deny},
	{"sametitle", bm_same},
	{0}
};

int
bm_cmp(const void *bb, const void *aa)
{
	struct bmstat *b = (struct bmstat *) bb;
	struct bmstat *a = (struct bmstat *) aa;
	if (a->stay != b->stay)
		return (a->stay - b->stay);
	if (a->inboard != b->inboard)
		return (a->inboard - b->inboard);
	return a->post - b->post;
}

void
bm_exit()
{
	int i, bmc;
	struct bmstat *data;
	FILE *fp1, *fp2, *fp;

	fp1 = fopen(BMSTAT, "w");
	fp2 = fopen(BMSTATA, "w");
	if (fp1 == NULL || fp2 == NULL) {
		errlog("faint,can't open bmstat output!");
		exit(-1);
	}
	fp = fp1;
	for (i = 0; i < 2; i++) {
		fprintf(fp, "版主工作统计 %s 按停留时间排序.\n", timeperiod);
		fprintf(fp,
			"telnet方式阅读本文 按 / 可以向后搜索字符串,按 ? 可以向前搜索字符串.\n\n");
		fp = fp2;
	}
	bmc = getdic(bmd, sizeof (struct bmstat), (void **) &data);
	if (bmc < 0) {
		errlog("Can't malloc bm result!");
		exit(-1);
	}
	qsort(data, bmc, sizeof (struct bmstat), bm_cmp);
	for (i = 0; i < bmc; i++) {
		if (data->noread)
			fp = fp2;
		else
			fp = fp1;

		fprintf(fp, "%s 版面 %s 版主 %s 停留时间 %d 秒 版面人气 %d\n",
			data->class, data->board, data->userid, data->stay,
			data->boardscore);
		fprintf(fp, "进版 %4d 次 ", data->inboard);
		fprintf(fp, "版内发文 %4d 篇 ", data->post);
		fprintf(fp, "收入文摘 %4d 篇 ", data->digest);
		fprintf(fp, "去掉文摘 %4d 篇\n", data->undigest);
		fprintf(fp, "区段 %4d 次 ", data->range);
		fprintf(fp, "标记文章 %4d 篇 ", data->mark);
		fprintf(fp, "去掉标记 %4d 篇 ", data->unmark);
		fprintf(fp, "删除文章 %4d 篇\n", data->delete);
		fprintf(fp, "封禁 %4d 次 ", data->deny);
		fprintf(fp, "收入精华 %4d 篇 ", data->import);
		fprintf(fp, "整理精华 %4d 次 ", data->move);
		fprintf(fp, "相同主题 %4d 次\n\n", data->same);
		data++;
	}
	fclose(fp1);
	fclose(fp2);
}

void
bm_init()
{
	int i, j, k;
	struct hword *tmp;
	int numboards;

	numboards = brdshm->number;
	for (i = 0; i < numboards; i++) {
		if (bcache[i].header.filename[0]) {
			for (k = 0; k < BMNUM; k++) {
				if (bcache[i].header.bm[k][0] == 0)
					continue;
				tmp = malloc(sizeof (struct hword));
				if (tmp == NULL) {
					errlog("Can't malloc in bm_init!");
					exit(-1);
				}
				if (!strcmp(bcache[i].header.bm[k], "") || !strcmp(bcache[i].header.bm[k], "SYSOP"))
					continue;
				snprintf(tmp->str, STRLEN - 1, "%s %s",
					bcache[i].header.filename,
					bcache[i].header.bm[k]);
				tmp->value = malloc(sizeof (struct bmstat));
				if (tmp->value == NULL) {
					errlog("Can't malloc value in bm_init!");
					exit(-1);
				}
				memset(tmp->value, 0, sizeof (struct bmstat));
				strncpy(((struct bmstat *) (tmp->value))->board, bcache[i].header.filename, 19);
				strncpy(((struct bmstat *) (tmp->value))->class, bcache[i].header.type, 4);
				strncpy(((struct bmstat *) (tmp->value))->userid, bcache[i].header.bm[k], IDLEN);
				((struct bmstat *) (tmp->value))->noread = boardnoread(&(bcache[i].header));
				((struct bmstat *) (tmp->value))->boardscore = bcache[i].score;
				insertdic(bmd, tmp);
			}
		}
	}
	register_stat(bm, bm_exit);
}

