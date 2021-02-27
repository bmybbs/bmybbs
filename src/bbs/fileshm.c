/*
	Pirate Bulletin Board System
	Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
	Eagles Bulletin Board System
	Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
						Guy Vega, gtvega@seabass.st.usm.edu
						Dominic Tynes, dbtynes@seabass.st.usm.edu
	Firebird Bulletin Board System
	Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
						Peng Piaw Foong, ppfoong@csie.ncu.edu.tw

	Copyright (C) 1999, KCN,Zhou Lin, kcn@cic.tsinghua.edu.cn

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 1, or (at your option)
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "bbs.h"
#include "bcache.h"
#include "smth_screen.h"
#include "stuff.h"
#include "goodbye.h"
#include "bbs_global_vars.h"

#define FILE_BUFSIZE        250    /* max. length of a file in SHM*/
#define FILE_MAXLINE         25    /* max. line of a file in SHM */
#define MAX_WELCOME          15    /* 欢迎画面数 */
#define MAX_GOODBYE          15    /* 离站画面数 */
#define MAX_ISSUE            15    /* 最大进站画面数 */
#define MAX_ENDLINE          15    /* 最大底线叶面数 */

struct FILESHM {
	char line[FILE_MAXLINE][FILE_BUFSIZE];
	int fileline;
	int max;
	time_t update;
};

static struct FILESHM *welcomeshm = NULL;
static struct FILESHM *goodbyeshm = NULL;
static struct FILESHM *issueshm   = NULL;
static struct FILESHM *endlineshm = NULL;

static void show_shmfile(struct FILESHM *fh);

int fill_shmfile(int mode, char *fname, int shmkey) {
	FILE *fffd;
	char *ptr;
	char buf[FILE_BUFSIZE];
	struct stat st;
	time_t ftime, now;
	int lines = 0, nowfn = 0, maxnum;
	struct FILESHM *tmp;

	switch (mode) {
	case 1:
		maxnum = MAX_ISSUE;
		break;
	case 2:
		maxnum = MAX_GOODBYE;
		break;
	case 3:
		maxnum = MAX_WELCOME;
		break;
	case 4:
	case 5:
	default:
		maxnum = MAX_ENDLINE;
		break;
	}
	now = time(0);
	if (stat(fname, &st) < 0) {
		return 0;
	}
	ftime = st.st_mtime;
	tmp = (void *) attach_shm(shmkey, sizeof (struct FILESHM) * maxnum);
	switch (mode) {
	case 1:
		issueshm = tmp;
		break;
	case 2:
		goodbyeshm = tmp;
		break;
	case 3:
		welcomeshm = tmp;
		break;
	case 4:
	case 5:
		endlineshm = tmp;
		break;
	}

	if (abs(now - tmp[0].update) < 86400 && ftime < tmp[0].update) {
		return 1;
	}
	if ((fffd = fopen(fname, "r")) == NULL) {
		return 0;
	}
	while ((fgets(buf, FILE_BUFSIZE, fffd) != NULL) && nowfn < maxnum) {
		if ((mode == 5 && lines >= FILE_MAXLINE) || strstr(buf, "@logout@") || strstr(buf, "@login@")) {
			tmp[nowfn].fileline = lines;
			tmp[nowfn].update = now;
			nowfn++;
			lines = 0;
			if (mode != 5)
				continue;
		}
		if (lines >= FILE_MAXLINE)
			continue;
		ptr = tmp[nowfn].line[lines];
		memcpy(ptr, buf, sizeof (buf));
		lines++;
	}
	fclose(fffd);
	tmp[nowfn].fileline = lines;
	tmp[nowfn].update = now;
	nowfn++;
	tmp[0].max = nowfn;
	return 1;
}

static void show_shmfile(struct FILESHM *fh) {
	int i;
	char buf[FILE_BUFSIZE];

	for (i = 0; i < fh->fileline; i++) {
		strcpy(buf, fh->line[i]);
		showstuff(buf);
	}
}

void show_goodbyeshm() {
	int logouts;

	logouts = goodbyeshm[0].max;
	clear();
	show_shmfile(&goodbyeshm[(currentuser.numlogins % ((logouts <= 1) ? 1 : logouts))]);
}

void show_welcomeshm() {
	int welcomes;

	welcomes = welcomeshm[0].max;
	clear();
	show_shmfile(&welcomeshm [(currentuser.numlogins % ((welcomes <= 1) ? 1 : welcomes))]);
	pressanykey();
}

#if 0
void
show_issue()
{
	int issues = issueshm[0].max;

	show_shmfile(&issueshm[(issues <= 1) ? 0 : ((time(0) / 86400) % (issues))]);
}
#endif

int show_endline() {
	static int i = 0;
	char buf[FILE_BUFSIZE + 10];
	int y, x;
	if (endlineshm == NULL)
		return 0;
	if (endlineshm->fileline <= 0)
		return 0;
	strcpy(buf, endlineshm[i / FILE_MAXLINE].line[i % FILE_MAXLINE]);
	getyx(&y, &x);
	showstuff(buf);
	move(y, x);
	i++;
	if (i / FILE_MAXLINE >= MAX_ENDLINE || i >= FILE_MAXLINE * (endlineshm->max - 1) + endlineshm[i / FILE_MAXLINE].fileline)
		i = 0;
	return 1;
}
