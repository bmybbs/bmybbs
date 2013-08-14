#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "ythtlib.h"
#include "ythtbbs.h"
#include "bbs.h"

#define PersonalPATH MY_BBS_HOME"/0Announce/groups/GROUP_0/PersonalCorpus"

time_t nowtime;

struct Report {
	char name[14];
	int day, week, week2, nfile;
} report;

int
anc_readtitle(FILE * fp, char *title, int size)
{
	char buf[512];
	while (fgets(buf, sizeof (buf), fp)) {
		if (!strncmp(buf, "# Title=", 8)) {
			strsncpy(title, buf + 8, size);
			return 0;
		}
	}
	return -1;
}

int
anc_readitem(FILE * fp, char *path, int sizepath, char *name, int sizename)
{
	char buf[512];
	int hasname = 0;
	while (fgets(buf, sizeof (buf), fp)) {
		if (!strncmp(buf, "Name=", 5)) {
			strsncpy(name, buf + 5, sizename);
			hasname = 1;
		}
		if (!hasname)
			continue;
		if (strncmp(buf, "Path=~", 6))
			continue;
		strsncpy(path, strtrim(buf + 6), sizepath);
		hasname = 2;
		break;
	}
	if (hasname != 2)
		return -1;
	return 0;
}

int
anc_hidetitle(char *title)
{
	if (strstr(title, "(BM: SYSOPS)") != NULL
	    || strstr(title, "(BM: SECRET)") != NULL
	    || strstr(title, "(BM: BMS)") != NULL)
		return 1;
	if (strstr(title, "<HIDE>") != NULL)
		return 1;
	return 0;
}

int
howmanynew(char *path, struct Report *rp)
{
	FILE *fp;
	char filen[800], str[500];
	struct stat sbuf;
	int i, ishideguest = 0;
	sprintf(filen, "%s/.Names", path);
	fp = fopen(filen, "r");
	if (fp == NULL) {
		return -1;
	}
	while (fgets(str, 500, fp) != NULL) {
		if (!strncmp(str, "Name=", 5)) {
			if (!strncmp(str, "Name=<HIDE>", 11) ||
			    !strncmp(str, "Name=<GUESTBOOK>", 16) ||
			    !strncmp(str, "Name=【精华区索引】", 19) ||
			    !strncmp(str, "Name=【精华区文章索引】", 23) ||
			    !strncmp(str, "Name=【精华区更新索引】", 23))
				ishideguest = 1;
			else
				ishideguest = 0;
			continue;
		}
		if (ishideguest)
			continue;
		if (strncmp(str, "Path=~", 6))
			continue;
		for (i = strlen(str); i >= strlen("Path=~"); i--) {
			if (strchr("\n\t\r /.", str[i]) == NULL)
				break;
			str[i] = '\0';
		}
		if (strlen(str) == strlen("Path=~"))
			continue;
		sprintf(filen, "%.300s%.200s", path, str + strlen("Path=~"));
		if (lstat(filen, &sbuf) < 0)
			continue;
		if (S_ISDIR(sbuf.st_mode))
			howmanynew(filen, rp);
		else if (S_ISREG(sbuf.st_mode)) {
			rp->nfile++;
			if (nowtime - sbuf.st_mtime < 24 * 3600)
				rp->day++;
			if (nowtime - sbuf.st_mtime < 24 * 3600 * 7)
				rp->week++;
			if (nowtime - sbuf.st_mtime < 24 * 3600 * 14)
				rp->week2++;
		} else
			continue;
	}
	fclose(fp);
	return 0;
}

main()
{
	char ch, path[500], fn[500], name[500];
	int tday = 0, tweek = 0, tweek2 = 0, tnfile = 0;
	FILE *fp;

	nowtime = time(NULL);
	printf("\n\t\t    个人文集更新统计\n\t\t%s\n", ctime(&nowtime));
	printf("个人文集    今日更新  一周内更新  两周内更新  总文章数\n");
	for (ch = 'A'; ch <= 'Z'; ch++) {
		sprintf(fn, "%s/%c/.Names", PersonalPATH, ch);
		fp = fopen(fn, "r");
		if (!fp)
			continue;
		if (anc_readtitle(fp, name, sizeof (name)) < 0) {
			fclose(fp);
			continue;
		}
		while (anc_readitem(fp, fn, sizeof (fn), name, sizeof (name)) >=
		       0) {
			if (anc_hidetitle(name))
				continue;
			if (!strstr(name, " _P"))
				continue;
			snprintf(path, sizeof (path), "%s/%c/%s", PersonalPATH,
				 ch, fn);
			sprintf(report.name, "%.12s", fn + 1);
			report.nfile = 0;
			report.day = 0;
			report.week = 0;
			report.week2 = 0;
			if (howmanynew(path, &report) < 0)
				continue;
			if (report.nfile == 0)
				continue;
			printf("%-12.12s%8d%12d%12d%10d\n", report.name,
			       report.day, report.week, report.week2,
			       report.nfile);
			tday += report.day;
			tweek += report.week;
			tweek2 += report.week2;
			tnfile += report.nfile;
		}
		fclose(fp);
	}
	printf("合    计    %8d%12d%12d%10d\n", tday, tweek, tweek2, tnfile);
}
