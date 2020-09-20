#include "ythtlib.h"
#include "nbstat.h"
#include "bbs.h"
#include "strhash.h"
#define BSSTAT MY_BBS_HOME"/0Announce/bbslist/boardscore"
diction bsstat;

struct bscore {
	char board[20];
	char expname[STRLEN];
	diction user;
	diction author;
	int person;
	int post;
	int stay;
	int noread;
	int score;
};

struct sstay {
	int stay;
	int day;
};

struct UTMPFILE *shm_utmp;
void
bs_use(int day, char *time, char *user, char *other)
{
	struct hword *a, *tmp;
	struct bscore *data;
	char board[30], staytime[10];
	char *temp[2] = { board, staytime };
	int i, stay, countstay;
//      int hour = 2;
	i = mystrtok(other, ' ', temp, 2);
	a = finddic(bsstat, board);
	stay = atoi(staytime);
//      if (stay > hour * 3600)
//              stay = hour * 3600;
	if (a != NULL) {
		data = a->value;
		a = finddic(data->user, user);
		if (a == NULL) {
			tmp = malloc(sizeof (struct hword));
			snprintf(tmp->str, STRLEN, "%s", user);
			tmp->value = calloc(1, sizeof (struct sstay));
			((struct sstay *) (tmp->value))->stay = stay;
			((struct sstay *) (tmp->value))->day = day;
			countstay = stay;
			insertdic(data->user, tmp);
			data->person++;
		} else {
			countstay = stay;
			if (((struct sstay *) (a->value))->day != day) {
				((struct sstay *) (a->value))->stay = 0;
				((struct sstay *) (a->value))->day = day;
			}
			stay += ((struct sstay *) (a->value))->stay;
/*			if (stay > hour * 3600) {
				countstay =
				    hour * 3600 -
				    ((struct sstay *) (a->value))->stay;
				stay = hour * 3600;
			}
*/
			((struct sstay *) (a->value))->stay = stay;
		}
		data->stay += countstay;
	}
}

int
bs_cmp(struct bscore *b, struct bscore *a)
{
	return a->score - b->score;
}

void
bs_post(int day, char *time, char *user, char *other)
{
	struct hword *a, *tmp;
	struct bscore *data;
	char board[30], title[128];
	char *temp[2] = { board, title };
	int i;
	i = mystrtok(other, ' ', temp, 2);
	a = finddic(bsstat, board);
	if (a != NULL) {
		data = a->value;
		a = finddic(data->author, user);
		if (a == NULL) {
			tmp = malloc(sizeof (struct hword));
			snprintf(tmp->str, STRLEN, "%s", user);
			insertdic(data->author, tmp);
			data->post++;
		}
	}
}

struct action_f bs[] = {
	{"use", bs_use},
	{"post", bs_post},
	{0}
};

void
bs_exit()
{
	int buc, i, count, numboards;
	int boards;
	struct bscore *data;
	FILE *fp;
	struct hword *a;
	buc = getdic(bsstat, sizeof (struct bscore), (void **) &data);
	if (buc < 0) {
		errlog("Can't malloc bu result!");
		exit(-1);
	}
	numboards = brdshm->number;
	for (i = 0; i < numboards; i++) {
		a = finddic(bsstat, bcache[i].header.filename);
		if (a != NULL) {
			data = a->value;
			data->score = bcache[i].score =
			    data->stay / 1000 + data->person * 2 +
			    data->post * 10;
		}
	}
	fp = fopen(BSSTAT, "w");
	if (fp == NULL) {
		errlog("can't open bsstat output!");
		exit(-1);
	}
	fprintf(fp, "\033[1;37m名次 %-15.15s%-38.38s %s  \033[m\n",
		"讨论区名称", "中文叙述", "人气值");
	buc = getdic(bsstat, sizeof (struct bscore), (void **) &data);
	qsort(data, buc, sizeof (struct bscore), (void *) bs_cmp);
	count = 0;
	boards = 0;
	for (i = 0; i < buc; i++) {
		if (!data->noread) {
			fprintf(fp,
				"\033[1m%4d\033[m %-15.15s%-38.38s %5d\n",
				i + 1, data->board, data->expname, data->score);
			count += data->score;
			boards++;
		}
		data++;
	}
	fprintf(fp, "\033[1m%4d\033[m %-15.15s%-38.38s %5d \n",
		boards, "Average", "平均", count / boards);
	fclose(fp);
	shm_utmp =
	    (struct UTMPFILE *) get_old_shm(UTMP_SHMKEY,
					    sizeof (struct UTMPFILE));
	shm_utmp->ave_score = count / boards;
}

void
bs_init()
{
	int i;
	struct hword *tmp;
	int numboards;
	numboards = brdshm->number;
	for (i = 0; i < numboards; i++) {
		if (!bcache[i].header.filename[0])
			continue;
		tmp = malloc(sizeof (struct hword));
		if (tmp == NULL) {
			errlog("Can't malloc in board_init!");
			exit(-1);
		}
		snprintf(tmp->str, 20, "%s", bcache[i].header.filename);
		tmp->value = calloc(1, sizeof (struct bscore));
		if (tmp->value == NULL) {
			errlog("Can't malloc value in board_init!");
			exit(-1);
		}
		strcpy(((struct bscore *) (tmp->value))->board, tmp->str);
		snprintf(((struct bscore *) (tmp->value))->expname, STRLEN,
			 "[%s] %s", bcache[i].header.type,
			 bcache[i].header.title);
		((struct bscore *) (tmp->value))->noread =
		    boardnoread(&(bcache[i].header));
		insertdic(bsstat, tmp);
	}
	register_stat(bs, bs_exit);
}
