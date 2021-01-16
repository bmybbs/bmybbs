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
#include <stdlib.h>
#include <string.h>
#include "bbs.h"
#include "regular.h"
#include "smth_screen.h"
#include "bbsinc.h"
#include "stuff.h"
#include "io.h"
#include "read.h"
#include "boards.h"
#include "bbs_global_vars.h"


typedef void (*power_dofunc) (int, struct fileheader *, char *);
static int slowtolower(int ch);
static char *slowstrlwr(char *str);
static void sure_markdel(int ent, struct fileheader *fileinfo, char *direct);
static void minus_markdel(int ent, struct fileheader *fileinfo, char *direct);
static void ann_mark(int ent, struct fileheader *fileinfo, char *direct);
static void power_dir(int ent, struct fileheader *fileinfo, char *direct);
static int power_range(char *filename, unsigned int id1, int id2, char *select, power_dofunc function, int *shoot);
static int titlehas(char *buf);
static int idis(char *buf);
static int checkmark(char *buf);
static int counttextlt(char *num);
static int checktext(char *query);
static int checkattr(char *buf);
static int checkstar(char *buf);
static int checkevas(char *buf);
const char *strstr2(const char *s, const char *s2);

static int slowtolower(int ch)
{
	if (ch > 64 && ch < 91)
	{
		return (ch + 32);
	}
	else
	return ch;
}

static char *slowstrlwr(char *str)
{
	size_t i;
	for (i = 0; i < strlen(str); i++)
	{
		str[i] = slowtolower(str[i]);
	}
	return str;
}


static void
sure_markdel(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	change_dir(direct, fileinfo, (void *) DIR_do_markdel, ent, digestmode, 0);
	return;
}

static void
minus_markdel(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	change_dir(direct, fileinfo, (void *) DIR_do_mark_minus_del, ent, digestmode, 0);
	return;
}

static void
ann_mark(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	change_dir(direct, fileinfo, (void *) DIR_do_spec, ent, digestmode, 0);
	return;
}

static struct fileheader *select_cur;
static void power_dir(int ent, struct fileheader *fileinfo, char *direct) {
	append_record(currdirect, fileinfo, sizeof (struct fileheader));
}

static int
power_range(char *filename, unsigned int id1, int id2, char *select, power_dofunc function, int *shoot)
{
	struct fileheader *buf;
	int fd, bufsize, n, ret;
	size_t i;
	struct stat st;
	*shoot = 0;
	if ((fd = open(filename, O_RDONLY)) == -1) {
		return -1;
	}
	fstat(fd, &st);
	if (-1 == id2)
		id2 = st.st_size / sizeof (struct fileheader);
	bufsize = sizeof (struct fileheader) * (id2 + 1 - id1);
	buf = malloc(bufsize);
	if (buf == NULL) {
		close(fd);
		return -4;
	}

	lseek(fd, (id1 - 1) * sizeof (struct fileheader), SEEK_SET);
	n = read(fd, buf, bufsize);
	close(fd);
	for (i = id1; i < id1 + n / sizeof (struct fileheader); i++) {
		select_cur = buf + (i - id1);
		ret = checkf(select);
		if (ret > 0) {
			(*function) (i, select_cur, filename);
			(*shoot)++;
		} else if (ret < 0) {
			free(buf);
			return -ret;
		}
	}
	free(buf);
	return 0;
}

// 全文搜索,interma@bmy
int full_search_action(char *whattosearch)
{
	digestmode = 3;
	setbdir(currdirect, currboard, digestmode);
	unlink(currdirect);
	sprintf(genbuf, "%s full_search %s %s",currentuser.userid, currboard, whattosearch);
	newtrace(genbuf);
	//
	char cmd[256];
	sprintf(cmd, MY_BBS_HOME "/bin/searcher.py %s '%s'", currboard, whattosearch);

	FILE *fp = popen(cmd, "r");
	if (fp == 0)
		return PARTUPDATE;

	char line[256];
	struct fileheader fh;
	bzero(&fh, sizeof(struct fileheader));
	int nr = 0;
	while (fgets(line, 256, fp) != NULL)
	{
		if (line[0] != 'M')
			break;

		char f_buf[16];
		int filetime;
		char t_buf[81];
		char o_buf[16];

		int len = strlen(line);
		ytht_strsncpy(f_buf, line, 15);
		sscanf(f_buf, "M.%ld.A", &(fh.filetime));
		sscanf(f_buf, "M.%ld.A", &(fh.edittime));

		char *p2s = strchr(line+15, ' ');
		int owner_len = p2s-line-15;
		ytht_strsncpy(fh.owner, line + 15, owner_len + 1);

		ytht_strsncpy(fh.title, p2s + 1, len - 2 - owner_len - 14 + 1);
		fh.title[strlen(fh.title)-1] = 0;
		fh.thread = nr++;
		append_record(currdirect, &fh, sizeof(struct fileheader));
	}

	pclose(fp);
	//
	limit_cpu();
	return NEWDIRECT;
}


int
power_action(char *filename, unsigned int id1, int id2, char *select, int action)
{
	ExtStru myextstru[] = {
		{titlehas, Pair("标题", "含")},
		{idis, Pair("作者", "是")},
		{counttextlt, Pair("灌水字数", "少于")},
		{checkmark, Pair("标记", "含")},
		{checktext, Pair("内容", "含")},
		{checkattr, Pair("属性", "是")},
		{checkstar, Pair("推荐星级", "高于")},
		{checkevas, Pair("推荐人数", "高于")},
		{NULL, NULL, NULL}
	};
	power_dofunc function;
	int shoot;
	int ret;
	extstru = myextstru;
	switch (action) {
	case 9:
		digestmode = 3;
		setbdir(currdirect, currboard, digestmode);
		unlink(currdirect);
		function = power_dir;
		break;
	case 1:
		function = sure_markdel;
		break;
	case 2:
		function = minus_markdel;
		break;
	case 3:
		function = ann_mark;
		break;
	case 0:
	default:
		return FULLUPDATE;
	}
	ret = power_range(filename, id1, id2, select, function, &shoot);
	sprintf(genbuf, "%s select %s %d %d",
		currentuser.userid, currboard, id1, id2);
	newtrace(genbuf);
	if (ret < 0) {
		prints("无法执行超级操作:%d,请联系系统维护.\n", ret);
		pressreturn();
		if (action == 9) {
			digestmode = NA;
			setbdir(currdirect, currboard, digestmode);
		}
		return FULLUPDATE;
	} else if (ret > 0) {
		prints("限制条件语法错误,从第%d个字符起我就觉得不对劲,一个汉字算两个字符啊!", ret);
		if (action == 9) {
			digestmode = NA;
			setbdir(currdirect, currboard, digestmode);
		}
		pressreturn();
		return FULLUPDATE;
	}

	limit_cpu();
	if (action == 9)
		return NEWDIRECT;
	fixkeep(filename, (id1 <= 0) ? 1 : id1, (id2 <= 0) ? 1 : id2);

	prints("操作完成,有%d篇文章满足条件\n", shoot);
	pressreturn();
	return FULLUPDATE;
}

const char*
strstr2(const char *s, const char *s2) {
	const char *p;
	size_t len = strlen(s2);
	for(p=s; p[0] != '\0'; p++) {
		if(!strncasecmp(p, s2, len)) return p;
		if(p[0]<0 && p[1]>31) p++;
	}
	return 0;
}

static int
titlehas(char *buf)
{
	char tmptitle[60];
	strcpy(tmptitle, select_cur->title);
	if (strstr2(slowstrlwr(tmptitle), slowstrlwr(buf)))
		return 1;
	else
		return 0;
}

static int
idis(char *buf)
{
	char tmpowner[14];
	strcpy(tmpowner, select_cur->owner);
	if (!strcasecmp(buf, "转信")) {
		if (strstr(slowstrlwr(tmpowner), "."))
			return 1;
		else
			return 0;
	}

	if (strcasecmp(slowstrlwr(tmpowner), slowstrlwr(buf)))

		return 0;
	else
		return 1;
}

static int
checkmark(char *buf)
{
	if (!strcasecmp(buf, "m")) {
		if (select_cur->accessed & FH_MARKED)
			return 1;
		else
			return 0;
	} else if (!strcasecmp(buf, "g")) {
		if (select_cur->accessed & FH_DIGEST)
			return 1;
		else
			return 0;
	} else if (!strcasecmp(buf, "@")) {
		if (select_cur->accessed & FH_ATTACHED)
			return 1;
		else
			return 0;
	} else
		return 0;
}

static int
counttextlt(char *num)
{
	FILE *fp;
	int n, size, size2, i;
	char buf[256];
	n = atoi(num);
	snprintf(buf, sizeof (buf), "boards/%s/%s", currboard, fh2fname(select_cur));
	fp = fopen(buf, "r");
	if (fp == NULL)
		return 1;
	for (i = 0; i < 3; i++)
		if (fgets(buf, sizeof (buf), fp) == NULL)
			break;
	if (i < 3) {
		fclose(fp);
		return 1;
	}
	size = size2 = 0;
	while (fgets(buf, sizeof (buf), fp) != NULL) {
		if (strcmp(buf, "--\n") == 0 || strcmp(buf, "--\r\n") == 0)
			break;
		if (strncmp(buf, ": ", 2)
				&& strncmp(buf, "> ", 2)
				&& !strstr(buf, "的大作中提到: 】")) {
			for (i = 0; buf[i]; i++) {
				if (buf[i] != ' ' && buf[i] != '\t' && buf[i] != '\r' && buf[i] != '\n') {
					if (buf[i] < 0)
						size2++;
					size++;
				}
				if (size - size2 / 2 >= n) {
					fclose(fp);
					return 0;
				}	//别接着记数了, 赶紧退吧
			}
		}
	}
	fclose(fp);
	if (size - size2 / 2 < n)
		return 1;
	else
		return 0;
}

static int
checktext(char *query)
{
	char buf[256];
	snprintf(buf, sizeof (buf), "boards/%s/%s", currboard, fh2fname(select_cur));
	return searchpattern(buf, query);
}

static int
checkattr(char *buf)
{
	if (!strcmp(buf, "未读")) {
		if (UNREAD(select_cur, &brc))
			return 1;
		else
			return 0;
	} else if (!strcmp(buf, "原作")) {
		if (strncmp(select_cur->title, "Re: ", 4))
			return 1;
		else
			return 0;
	} else
		return 0;
}

static int
checkstar(char *buf)
{
	if (select_cur->staravg50 >= atoi(buf) * 50)
		return 1;
	else
		return 0;
}

static int
checkevas(char *buf)
{
	if (select_cur->hasvoted >= atoi(buf))
		return 1;
	else
		return 0;
}

int
power_select(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char num[8];
	static char select[STRLEN];
	unsigned int inum1, inum2, answer;
	char dir[STRLEN];
	if (uinfo.mode != READING || digestmode != NA)
		return DONOTHING;
	snprintf(dir, STRLEN, "%s", direct);
	clear();
	prints("                  超强文章选择\n\n");
	prints("请选择操作范围\n");
	getdata(3, 0, "首篇文章编号: ", num, 6, DOECHO, YEA);
	inum1 = atoi(num);
	if (inum1 <= 0) {
		prints("错误编号\n");
		pressreturn();
		return FULLUPDATE;
	}
	getdata(4, 0, "末篇文章编号: ", num, 6, DOECHO, YEA);
	inum2 = atoi(num);
	if (inum2 - inum1 <= 1) {
		prints("错误编号\n");
		pressreturn();
		return FULLUPDATE;
	}
	move(6, 0);
	prints("例子:\n"
			"   例子一: 找sysop的所有文章\n"
			"   请输入限制条件: 作者是 sysop(查找转信文章请输入 作者是 转信)\n"
			"   例子二: 找sysop写的灌水文章\n"
			"   请输入限制条件: 作者是 sysop 且 灌水字数少于 40\n"
			"   例子三: 找sysop写的灌水文章,而且被标记为m的\n"
			"   请输入限制条件: 作者是 sysop 且 灌水字数少于 40 且 标记含 m\n"
			"   例子四: 找所有标题包含兵马俑或者bmy的文章\n"
			"   请输入限制条件: 标题含 兵马俑 或 标题含 bmy\n"
			"   例子五: 找所有不是sysop也不是XJTU-XANET发表的文章\n"
			"   请输入限制条件: 非 (作者是 sysop 或 作者是 XJTU-XANET)\n"
			"   (或者)请输入限制条件: 作者不是 sysop 且 作者不是 XJTU-XANET\n"
			"   例子六: 找所有有附件的文章\n"
			"   请输入限制条件: 标记含 @\n"
			"   例子七: 找所有推荐星级在3星以上的文章\n"
			"   请输入限制条件: 推荐星级高于 3\n"
			"   例子八: 找所有推荐人数在10人以上的文章\n"
			"   请输入限制条件: 推荐人数高于 10\n");

	getdata(5, 0, "请输入限制条件: ", select, 60, DOECHO, NA);
	clrtobot();
	if (IScurrBM)
		getdata(6, 0,
			"请输入你希望的操作: 0)取消 1)标记删除 2)标记减文章数删除 3)标记精华 9)阅读:",
			num, 2, DOECHO, YEA);
	else
		getdata(6, 0,
			"请输入你希望的操作: 0)取消 9)阅读:",
			num, 2, DOECHO, YEA);
	answer = atoi(num);
	if (answer > 0 && answer < 9 && !IScurrBM) {
		prints("您选择的功能不能使用\n");
		pressreturn();
		return FULLUPDATE;
	}
	return power_action(dir, inum1, inum2, select, answer);
}

