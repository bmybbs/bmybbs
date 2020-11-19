#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "ythtbbs/ythtbbs.h"

int
ythtbbs_announce_checktitle(const char *str)
{
	int indextype = 0;
	if (strstr(str, "\xA1\xBE\xBE\xAB\xBB\xAA\xC7\xF8\xCB\xF7\xD2\xFD\xA1\xBF") == str) // 【精华区索引】
		indextype = 2;
	else if (strstr(str, "\xA1\xBE\xBE\xAB\xBB\xAA\xC7\xF8\xC4\xBF\xC2\xBC\xCB\xF7\xD2\xFD\xA1\xBF") == str) // 【精华区目录索引】
		indextype = 2;
	else if (strstr(str, "\xA1\xBE\xBE\xAB\xBB\xAA\xC7\xF8\xCE\xC4\xD5\xC2\xCB\xF7\xD2\xFD\xA1\xBF") == str) // 【精华区文章索引】
		indextype = 1;
	else if (strstr(str, "\xA1\xBE\xBE\xAB\xBB\xAA\xC7\xF8\xB8\xFC\xD0\xC2\xCB\xF7\xD2\xFD\xA1\xBF") == str) // 【精华区更新索引】
		indextype = 3;
	return indextype;
}

/*
int
do_testtime(int t, char *path0, int mode, int time)	//mode=0 测试所有文件 mode=1 测试 .Names 文件
{
	char names[1024], genbuf[1024], path00[1024];
	FILE *fp;
	int m;
	if (!lfile_isdir(path0))
		return 0;
	if (file_time(path0) >= t - time)
		return 1;
	sprintf(names, "%s/.Names", path0);
	if (file_time(names) >= t - time)
		return 1;
	fp = fopen(names, "r");
	if (!fp)
		return 0;
	while (fgets(genbuf, sizeof (genbuf), fp)) {
		if (!strncmp(genbuf, "Name=", 5)) {
			if (ythtbbs_announce_checktitle(genbuf + 5))
				continue;
			fgets(genbuf, 256, fp);
			if (strncmp("Path=~/", genbuf, 6) != 0)
				continue;
			for (m = 0; m < strlen(genbuf); m++)
				if (genbuf[m] <= 27)
					genbuf[m] = 0;
			if (!strcmp("Path=~/", genbuf))
				continue;
			sprintf(path00, "%s/%s", path0, genbuf + 7);
			for (m = 0; m < strlen(path00); m++)
				if (path00[m] <= 27)
					path00[m] = 0;
			if (checkautofile(genbuf + 7, path00))
				continue;
			if (!file_exist(path00))
				continue;
			if (lfile_isdir(path00) && do_testtime(t, path00, mode, time)) {
				fclose(fp);
				return 1;
			} else if (!mode && file_time(path00) >= t - time) {
				fclose(fp);
				return 1;
			}
		}
	}
	fclose(fp);
	return 0;
}

struct autofile {
	char *fname;
	char *fpath;
	ino_t inode;
	dev_t dev;
};

struct autofile autofile[] = {
	{"ctop", MY_BBS_HOME "/0Announce/groups/GROUP_0/Personal_Corpus/ctop"},
	{"7days", MY_BBS_HOME "/0Announce/groups/GROUP_0/BM_Club/7days"},
	{"bmlist", MY_BBS_HOME "/0Announce/groups/GROUP_0/BM_Club/bmlist"},
	{"boardlist",
	 MY_BBS_HOME "/0Announce/groups/GROUP_0/BM_Club/boardlist"},
	{NULL}
};

int
checkautofile(char *fname, char *fpath)
{
	static int inited = 0;
	int i;
	struct stat *st;
	if (!inited) {
		for (i = 0; autofile[i].fname; i++) {
			st = f_stat(autofile[i].fpath);
			autofile[i].dev = st->st_dev;
			autofile[i].inode = st->st_ino;
		}
		inited = 1;
	}
	for (i = 0; autofile[i].fname; i++) {
		if (strcmp(fname, autofile[i].fname))
			continue;
		st = f_stat(fpath);
		if (autofile[i].dev == st->st_dev &&
		    autofile[i].inode == st->st_ino) return 1;
	}
	return 0;
}
*/