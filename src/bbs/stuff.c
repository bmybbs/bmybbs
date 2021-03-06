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
	Copyright (C) 1999 KCN,Zhou lin,kcn@cic.tsinghua.edu.cn

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
#include "smth_screen.h"
#include "main.h"
#include "io.h"
#include "bbs_global_vars.h"
#include "bbs-internal.h"

#ifdef CAN_EXEC
char tempfile[MAXPATHLEN];
#endif

extern void io_output(const char *s, int len);

int dashf(const char *fname) {
	struct stat st;

	return (stat(fname, &st) == 0 && S_ISREG(st.st_mode));
}

int dashd(const char *fname) {
	struct stat st;

	return (stat(fname, &st) == 0 && S_ISDIR(st.st_mode));
}

int dashl(const char *fname) {
	struct stat st;

	return (lstat(fname, &st) == 0 && S_ISLNK(st.st_mode));
}

int pressanykey() {
	extern int showansi;

	showansi = 1;
	move(t_lines - 1, 0);
	clrtoeol();
	//"\033[5;1;33m按任何键继续...\033[m"
	prints("\033[m                                \033[5;1;33m\xB0\xB4\xC8\xCE\xBA\xCE\xBC\xFC\xBC\xCC\xD0\xF8...\033[m");
	egetch();
	move(t_lines - 1, 0);
	clrtoeol();
	return 0;
}

int pressreturn() {
	extern int showansi;
	char buf[3];

	showansi = 1;
	move(t_lines - 1, 0);
	clrtoeol();
		//"\033[1;33m请按 ◆\033[5;36mEnter\033[m\033[1;33m◆ 继续\033[m",
	getdata(t_lines - 1, 0,
		"                              \033[1;33m\xC7\xEB\xB0\xB4 \xA1\xF4\033[5;36mEnter\033[m\033[1;33m\xA1\xF4 \xBC\xCC\xD0\xF8\033[m",
		buf, 2, NOECHO, YEA);
	move(t_lines - 1, 0);
	clrtoeol();
	return 0;
}

int askyn(char *str, int defa, int gobottom) {
	int x, y;
	char realstr[280];
	char ans[3];

	snprintf(realstr, sizeof (realstr), "%s (Y/N)? [%c]: ", str, (defa) ? 'Y' : 'N');
	if (gobottom)
		move(t_lines - 1, 0);
	getyx(&x, &y);
	clrtoeol();
	getdata(x, y, realstr, ans, 2, DOECHO, YEA);
	if (ans[0] != 'Y' && ans[0] != 'y' && ans[0] != 'N' && ans[0] != 'n') {
		return defa;
	} else if (ans[0] == 'Y' || ans[0] == 'y')
		return 1;
	else
		return 0;
}

void printdash(char *mesg) {
	char buf[80], *ptr;
	int len;

	memset(buf, '=', 79);
	buf[79] = '\0';
	if (mesg != NULL) {
		len = strlen(mesg);
		if (len > 76)
			len = 76;
		ptr = &buf[40 - len / 2];
		ptr[-1] = ' ';
		ptr[len] = ' ';
		strncpy(ptr, mesg, len);
	}
	prints("%s\n", buf);
}

void bell() {
	char sound;

	sound = Ctrl('G');
	io_output(&sound, 1);
}

/* rrr - Snagged from pbbs 1.8 */

#define LOOKFIRST  (0)
#define LOOKLAST   (1)
#define QUOTEMODE  (2)
#define MAXCOMSZ (1024)
#define MAXARGS (40)
#define MAXENVS (20)
#define BINDIR "/bin/"

char *bbsenv[MAXENVS];
int numbbsenvs = 0;

int deltree(char *dst) {
	char rpath[PATH_MAX + 1 + 10], buf[PATH_MAX + 1];
	int i = 0, j = 0, isdir = 0, fd;
	static char *const (disks[]) = {
		MY_BBS_HOME "/0Announce/",
		MY_BBS_HOME "/boards/",
		MY_BBS_HOME "/mail/", MY_BBS_HOME "/", NULL
	};

	if (dashl(dst)) {
		unlink(dst);
		return 1;
	}
	if (realpath(dst, rpath) == NULL)
		return 0;
	j = strlen(rpath);
	if (rpath[j - 1] == '/')
		rpath[j - 1] = 0;
	if (strncmp(rpath, MY_BBS_HOME "/", sizeof (MY_BBS_HOME)))
		return 0;
	strcpy(buf, rpath);
	if (dashd(dst))
		isdir = 1;
	for (i = 0; disks[i]; i++) {
		j = strlen(disks[i]);
		if (!strncmp(rpath, disks[i], j))
			break;
	}
	memmove(rpath + j + 6, rpath + j, sizeof (rpath) - j - 6);
	memcpy(rpath + j, ".junk/", 6);
	j += 6;
	ytht_normalize(rpath + j);
	rpath[PATH_MAX - 10] = 0;
	j = strlen(rpath);
	i = 0;
	if (isdir) {
		while (mkdir(rpath, 0770) != 0 && i < 1000) {
			sprintf(rpath + j, "+%d", i);
			i++;
		}
		if (i == 1000)
			return 0;
	} else {
		while (((fd = open(rpath, O_CREAT | O_EXCL | O_WRONLY, 0660)) < 0) && i < 1000) {
			sprintf(rpath + j, "+%d", i);
			i++;
		}
		if (i == 1000) {
			if (fd >= 0) {
				close(fd);
			}
			return 0;
		}
		close(fd);
	}
	rename(buf, rpath);
	return 1;
}

#if 0
static int
bbssetenv(env, val)
char *env, *val;
{
	int i, len;

	if (numbbsenvs == 0)
		bbsenv[0] = NULL;
	len = strlen(env);
	for (i = 0; bbsenv[i]; i++)
		if (!strncasecmp(env, bbsenv[i], len))
			break;
	if (i >= MAXENVS)
		return -1;
	if (bbsenv[i])
		free(bbsenv[i]);
	else
		bbsenv[++numbbsenvs] = NULL;
	bbsenv[i] = malloc(strlen(env) + strlen(val) + 2);
	strcpy(bbsenv[i], env);
	strcat(bbsenv[i], "=");
	strcat(bbsenv[i], val);
	return 0;
}
#endif

