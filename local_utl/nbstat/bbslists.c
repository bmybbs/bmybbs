#include "nbstat.h"
#include "bbs.h"
#include "strhash.h"

#define NACSTAT MY_BBS_HOME"/0Announce/bbslist/newacct.today"
#define LOGINSTAT MY_BBS_HOME"/0Announce/bbslist/countusr"
#define USAGE1 MY_BBS_HOME"/0Announce/bbslist/board2"
#define USAGE2 MY_BBS_HOME"/0Announce/groups/GROUP_0/syssecurity/board2"

#define MAX_LINE (15)

diction busage;

char sid[11];

struct bstat {
	char board[20];
	char expname[STRLEN];
	int used;
	int stay;
	int noread;
};

int newaccount[24];
int login[24];
int stay[24];

void
bbslists_newaccount(int day, char *time, char *user, char *other)
{
	char uid[10], fromhost[40];
	char *temp[2] = { uid, fromhost };
	int i;
	int hour;
	i = mystrtok(other, ' ', temp, 2);
	hour = atoi(time);
	if (hour < 0 || hour > 23) {
		errlog("Invalid newaccount time, %s %s", user, other);
		return;
	}
	newaccount[hour]++;

}

void
bbslists_enter(int day, char *time, char *user, char *other)
{
	int hour;
	hour = atoi(time);
	if (hour < 0 || hour > 23) {
		errlog("Invalid login time, %s %s", user, other);
		return;
	}
	login[hour]++;
}

void
bbslists_drop(int day, char *time, char *user, char *other)
{
	int hour;
	hour = atoi(time);
	if (hour < 0 || hour > 23) {
		errlog("Invalid drop time, %s %s", user, other);
		return;
	}
	stay[hour] += atoi(other);
}

void
bbslists_exitbbs(int day, char *time, char *user, char *other)
{
	int hour;
	hour = atoi(time);
	if (hour < 0 || hour > 23) {
		errlog("Invalid exitbbs time, %s %s", user, other);
		return;
	}
	stay[hour] += atoi(other);
}

void
bbslists_use(int day, char *time, char *user, char *other)
{
	struct hword *a;
	struct bstat *data;
	char board[30], staytime[10];
	char *temp[2] = { board, staytime };
	int i;
	i = mystrtok(other, ' ', temp, 2);
	a = finddic(busage, board);
	if (a != NULL) {
		data = a->value;
		data->used++;
		data->stay += atoi(staytime);
	}

}
struct action_f bbslists[] = {
	{"newaccount", bbslists_newaccount},
	{"enter", bbslists_enter},
	{"drop", bbslists_drop},
	{"exitbbs", bbslists_exitbbs},
	{"use", bbslists_use},
	{0}
};

void
draw_account()
{
	int i, j, totaltime = 0, total = 0, max = 0, item, now;
	FILE *fp;
	char *blk[10] = {
		"£ß", "¡õ", "¡õ", "¡õ", "¡õ",
		"¡õ", "¡õ", "¡õ", "¡õ", "¡õ",
	};

	for (i = 0; i < 24; i++) {
		total += login[i];
		totaltime += stay[i];
		if (login[i] > max)
			max = login[i];
	}

	item = max / MAX_LINE + 1;

	if ((fp = fopen(LOGINSTAT, "w")) == NULL) {
		errlog("Cann't open countusr\n");
		exit(-1);
	}

	fprintf(fp,
		"\n[1;36m    ©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´\n");
	for (i = MAX_LINE; i >= 0; i--) {
		if ((i + 1) * item >= 10000)
			fprintf(fp, "[1;37m%-6d[33m", (i + 1) * item);
		else
			fprintf(fp, "[1;37m%4d[36m©¦[33m", (i + 1) * item);
		for (j = 0; j < 24; j++) {
			int shownum;
			shownum = login[j] % 1000;
			if ((item * (i) > login[j])
			    && (item * (i - 1) <= login[j]) && login[j]) {
				fprintf(fp, "[35m%-3.3d[33m", (shownum));
				continue;
			}
			if (login[j] - item * i < item && item * i < login[j])
				fprintf(fp, "%s ",
					blk[((login[j] - item * i) * 10) /
					    item]);
			else if (login[j] - item * i >= item)
				fprintf(fp, "%s ", blk[9]);
			else
				fprintf(fp, "   ");
		}
		fprintf(fp, "[1;36m©¦\n");
	}
	now = time(NULL);
	fprintf(fp,
		"   [37m0[36m©¸¡ª¡ª[37m%s Ã¿Ð¡Ê±µ½·ÃÈË´ÎÍ³¼Æ   [36m¡ª¡ª¡ª[37m%s[36m¡ª¡ª¡ª©¼\n"
		"    [;36m  00 01 02 03 04 05 06 07 08 09 10 11 [1;31m12 13 14 15 16 17 18 19 20 21 22 23\n\n"
		"                 [32m1 [33m¡õ [32m= [37m%-5d [32m×Ü¹²ÉÏÕ¾ÈË´Î£º[37m%-9d[32mÆ½¾ùÊ¹ÓÃÊ±¼ä£º[37m%d[m\n",
		sid, Ctime(now), item, total,
		totaltime / (total ? total : 1) / 60 + 1);
	fclose(fp);
}

void
draw_newacct()
{
	int i, j, total = 0, max = 0, item, now;
	FILE *fp;
	char *blk[10] = {
		"£ß", "¡õ", "¡õ", "¡õ", "¡õ",
		"¡õ", "¡õ", "¡õ", "¡õ", "¡õ",
	};
	for (i = 0; i < 24; i++) {
		total += newaccount[i];
		if (newaccount[i] > max)
			max = newaccount[i];
	}
	item = max / MAX_LINE + 1;

	fp = fopen(NACSTAT, "w");
	if (fp == NULL) {
		errlog("faint,can't open newacct output!");
		exit(-1);
	}
	fprintf(fp,
		"\n[1;36m   ©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´\n");
	for (i = MAX_LINE; i >= 0; i--) {
		fprintf(fp, "[1;37m%3d[36m©¦[31m", (i + 1) * item);
		for (j = 0; j < 24; j++) {
			if ((item * (i) > newaccount[j])
			    && (item * (i - 1) <= newaccount[j])
			    && newaccount[j]) {
				fprintf(fp, "[35m%-3d[31m", (newaccount[j]));
				continue;
			}
			if (newaccount[j] - item * i < item
			    && item * i < newaccount[j])
				fprintf(fp, "%s ",
					blk[((newaccount[j] - item * i) * 10) /
					    item]);
			else if (newaccount[j] - item * i >= item)
				fprintf(fp, "%s ", blk[9]);
			else
				fprintf(fp, "   ");
		}
		fprintf(fp, "[1;36m©¦\n");
	}
	now = time(NULL);
	fprintf(fp,
		"  [37m0[36m©¸¡ª¡ª[37m%s  ±¾ÈÕÐÂÔöÈË¿ÚÍ³¼Æ  [36m¡ª¡ª¡ª¡ª[37m%s[36m¡ª¡ª¡ª©¼\n"
		"   [;36m  00 01 02 03 04 05 06 07 08 09 10 11 [1;32m12 13 14 15 16 17 18 19 20 21 22 23\n\n"
		"                     [33m1 [31m¡õ [33m= [37m%-5d [33m±¾ÈÕÉêÇëÐÂÕÊºÅÈËÊý£º[37m%-9d[m\n",
		sid, Ctime(now), item, total);
	fclose(fp);
}

int static
bstat_cmp(struct bstat *b, struct bstat *a)
{
	if (a->stay != b->stay)
		return (a->stay - b->stay);
	return a->used - b->used;
}

char static *
timetostr(i)
int i;
{
	static char str[30];
	int minute, sec, hour;

	minute = (i / 60);
	hour = minute / 60;
	minute = minute % 60;
	sec = i & 60;
	sprintf(str, "%2d:%2d:%2d", hour, minute, sec);
	return str;
}

void
draw_usage()
{
	int i, buc, count[2];
	struct bstat *data;
	FILE *fp1, *fp2, *fp;

	fp1 = fopen(USAGE1, "w");
	fp2 = fopen(USAGE2, "w");
	if (fp1 == NULL || fp2 == NULL) {
		errlog("faint,can't open usage output!");
		exit(-1);
	}
	fp = fp1;
	for (i = 0; i < 2; i++) {
		fprintf(fp,
			"\033[1;37mÃû´Î %-17.17s %-27.27s %5s %8s %8s\033[m\n",
			"ÌÖÂÛÇøÃû³Æ", "ÖÐÎÄÐðÊö", "ÈË´Î", "ÀÛ¼ÆÊ±¼ä",
			"Æ½¾ùÊ±¼ä");
		fp = fp2;
	}
	buc = getdic(busage, sizeof (struct bstat), (void **) &data);
	if (buc < 0) {
		errlog("Can't malloc usage result!");
		exit(-1);
	}
	qsort(data, buc, sizeof (struct bstat), (void *) bstat_cmp);
	count[0] = count[1] = 0;
	for (i = 0; i < buc; i++) {
		if (data->noread)
			fp = fp2;
		else
			fp = fp1;
		fprintf(fp,
			"\033[1m%4d\033[m %-17.17s %-27.27s %5d %-.8s %8d\n",
			i + 1, data->board, data->expname, data->used,
			timetostr(data->stay),
			(data->used == 0) ? 0 : data->stay / data->used);
		count[0] += data->used;
		count[1] += data->stay;
		data++;
	}
	fp = fp1;
	for (i = 0; i < 2; i++) {
		fprintf(fp,
			"\033[1m%4d\033[m %-17.17s %-27.27s %5d %-.8s %8d\n",
			buc, "Average", "×ÜÆ½¾ù", count[0] / buc,
			timetostr(count[1] / buc),
			(count[0] == 0) ? 0 : count[1] / count[0]);
		fp = fp2;
	}
	fclose(fp1);
	fclose(fp2);
}

void
bbslists_exit()
{
	draw_account();
	draw_newacct();
	draw_usage();
}

void
bbslists_init()
{
	int i;
	struct hword *tmp;
	int numboards;
	numboards = brdshm->number;
	snprintf(sid, sizeof (sid), "%10s", MY_BBS_ID " BBS");
	for (i = 0; i < numboards; i++) {
		if (!bcache[i].header.filename[0])
			continue;
		tmp = malloc(sizeof (struct hword));
		if (tmp == NULL) {
			errlog("Can't malloc in bbslists_init!");
			exit(-1);
		}
		snprintf(tmp->str, 20, "%s", bcache[i].header.filename);
		tmp->value = calloc(1, sizeof (struct bstat));
		if (tmp->value == NULL) {
			errlog("Can't malloc value in board_init!");
			exit(-1);
		}
		strcpy(((struct bstat *) (tmp->value))->board, tmp->str);
		snprintf(((struct bstat *) (tmp->value))->expname,
			 STRLEN, "%s", bcache[i].header.title);
		((struct bstat *) (tmp->value))->noread =
		    boardnoread(&(bcache[i].header));
		insertdic(busage, tmp);
	}
	bzero(newaccount, sizeof (newaccount));
	bzero(login, sizeof (login));
	bzero(stay, sizeof (stay));
	register_stat(bbslists, bbslists_exit);
}
