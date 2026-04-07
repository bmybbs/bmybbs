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
#include "bmy/search.h"
#include "bmy/convcode.h"


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


static void sure_markdel(int ent, struct fileheader *fileinfo, char *direct)
{
	change_dir(direct, fileinfo, (void *) DIR_do_markdel, ent, digestmode, 0);
	return;
}

static void minus_markdel(int ent, struct fileheader *fileinfo, char *direct)
{
	change_dir(direct, fileinfo, (void *) DIR_do_mark_minus_del, ent, digestmode, 0);
	return;
}

static void ann_mark(int ent, struct fileheader *fileinfo, char *direct)
{
	change_dir(direct, fileinfo, (void *) DIR_do_spec, ent, digestmode, 0);
	return;
}

static struct fileheader *select_cur;
static void power_dir(int ent, struct fileheader *fileinfo, char *direct) {
	(void) ent;
	(void) direct;
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
// 由 PyLucene 改为 Lucene IronBlood@bmy
int full_search_action(char *whattosearch)
{
	digestmode = 3;
	setbdir(currdirect, currboard, digestmode);
	unlink(currdirect);
	sprintf(genbuf, "%s full_search %s %s",currentuser.userid, currboard, whattosearch);
	newtrace(genbuf);

	size_t search_size, i;
	struct fileheader_utf *articles = bmy_search_board_gbk(currboard, whattosearch, &search_size);

	if (articles == NULL)
		return PARTUPDATE;

	struct fileheader fh;
	int nr = 0;
	for (i = 0; i < search_size; i++) {
		memset(&fh, 0, sizeof(struct fileheader));
		fh.filetime = articles[i].filetime;
		fh.edittime = articles[i].filetime;
		fh.thread = nr++;
		ytht_strsncpy(fh.owner, articles[i].owner, sizeof(fh.owner));
		u2g(articles[i].title, strlen(articles[i].title), fh.title, sizeof(fh.title));
		append_record(currdirect, &fh, sizeof(struct fileheader));
	}

	free(articles);

	limit_cpu();
	return NEWDIRECT;
}

const ExtStru extstru[] = {
	// 标题
	// 含
	{titlehas, Pair("\xB1\xEA\xCC\xE2", "\xBA\xAC")},
	// 作者
	// 是
	{idis, Pair("\xD7\xF7\xD5\xDF", "\xCA\xC7")},
	// 灌水字数
	// 少于
	{counttextlt, Pair("\xB9\xE0\xCB\xAE\xD7\xD6\xCA\xFD", "\xC9\xD9\xD3\xDA")},
	// 标记
	// 含
	{checkmark, Pair("\xB1\xEA\xBC\xC7", "\xBA\xAC")},
	// 内容
	// 含
	{checktext, Pair("\xC4\xDA\xC8\xDD", "\xBA\xAC")},
	// 属性
	// 是
	{checkattr, Pair("\xCA\xF4\xD0\xD4", "\xCA\xC7")},
	// 推荐星级
	// 高于
	{checkstar, Pair("\xCD\xC6\xBC\xF6\xD0\xC7\xBC\xB6", "\xB8\xDF\xD3\xDA")},
	// 推荐人数
	// 高于
	{checkevas, Pair("\xCD\xC6\xBC\xF6\xC8\xCB\xCA\xFD", "\xB8\xDF\xD3\xDA")},
	{NULL, NULL, NULL}
};

int
power_action(char *filename, unsigned int id1, int id2, char *select, int action)
{
	power_dofunc function;
	int shoot;
	int ret;
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
		// 无法执行超级操作:%d,请联系系统维护.\n
		prints("\xCE\xDE\xB7\xA8\xD6\xB4\xD0\xD0\xB3\xAC\xBC\xB6\xB2\xD9\xD7\xF7" ":%d," "\xC7\xEB\xC1\xAA\xCF\xB5\xCF\xB5\xCD\xB3\xCE\xAC\xBB\xA4" ".\n", ret);
		pressreturn();
		if (action == 9) {
			digestmode = NA;
			setbdir(currdirect, currboard, digestmode);
		}
		return FULLUPDATE;
	} else if (ret > 0) {
		// 限制条件语法错误,从第%d个字符起我就觉得不对劲,一个汉字算两个字符啊!
		prints("\xCF\xDE\xD6\xC6\xCC\xF5\xBC\xFE\xD3\xEF\xB7\xA8\xB4\xED\xCE\xF3" "," "\xB4\xD3\xB5\xDA" "%d" "\xB8\xF6\xD7\xD6\xB7\xFB\xC6\xF0\xCE\xD2\xBE\xCD\xBE\xF5\xB5\xC3\xB2\xBB\xB6\xD4\xBE\xA2" "," "\xD2\xBB\xB8\xF6\xBA\xBA\xD7\xD6\xCB\xE3\xC1\xBD\xB8\xF6\xD7\xD6\xB7\xFB\xB0\xA1" "!", ret);
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

	// 操作完成,有%d篇文章满足条件\n
	prints("\xB2\xD9\xD7\xF7\xCD\xEA\xB3\xC9" "," "\xD3\xD0" "%d" "\xC6\xAA\xCE\xC4\xD5\xC2\xC2\xFA\xD7\xE3\xCC\xF5\xBC\xFE" "\n", shoot);
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
	// 转信
	if (!strcasecmp(buf, "\xD7\xAA\xD0\xC5")) {
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
				// 的大作中提到: 】
				&& !strstr(buf, "\xB5\xC4\xB4\xF3\xD7\xF7\xD6\xD0\xCC\xE1\xB5\xBD" ": " "\xA1\xBF")) {
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
	// 未读
	if (!strcmp(buf, "\xCE\xB4\xB6\xC1")) {
		if (UNREAD(select_cur, &brc))
			return 1;
		else
			return 0;
	// 原作
	} else if (!strcmp(buf, "\xD4\xAD\xD7\xF7")) {
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

int power_select(int ent, struct fileheader *fileinfo, char *direct) {
	(void) ent;
	(void) fileinfo;
	char num[8];
	static char select[STRLEN];
	unsigned int inum1, inum2, answer;
	char dir[STRLEN];
	if (uinfo.mode != READING || digestmode != NA)
		return DONOTHING;
	snprintf(dir, STRLEN, "%s", direct);
	clear();
	//                   超强文章选择\n\n
	prints("                  " "\xB3\xAC\xC7\xBF\xCE\xC4\xD5\xC2\xD1\xA1\xD4\xF1" "\n\n");
	// 请选择操作范围\n
	prints("\xC7\xEB\xD1\xA1\xD4\xF1\xB2\xD9\xD7\xF7\xB7\xB6\xCE\xA7" "\n");
	// 首篇文章编号:
	getdata(3, 0, "\xCA\xD7\xC6\xAA\xCE\xC4\xD5\xC2\xB1\xE0\xBA\xC5" ": ", num, 6, DOECHO, YEA);
	inum1 = atoi(num);
	if (inum1 <= 0) {
		// 错误编号\n
		prints("\xB4\xED\xCE\xF3\xB1\xE0\xBA\xC5" "\n");
		pressreturn();
		return FULLUPDATE;
	}
	// 末篇文章编号:
	getdata(4, 0, "\xC4\xA9\xC6\xAA\xCE\xC4\xD5\xC2\xB1\xE0\xBA\xC5" ": ", num, 6, DOECHO, YEA);
	inum2 = atoi(num);
	if (inum2 - inum1 <= 1) {
		// 错误编号\n
		prints("\xB4\xED\xCE\xF3\xB1\xE0\xBA\xC5" "\n");
		pressreturn();
		return FULLUPDATE;
	}
	move(6, 0);
	// 例子:\n
	prints("\xC0\xFD\xD7\xD3" ":\n"
			//    例子一: 找sysop的所有文章\n
			"   " "\xC0\xFD\xD7\xD3\xD2\xBB" ": " "\xD5\xD2" "sysop" "\xB5\xC4\xCB\xF9\xD3\xD0\xCE\xC4\xD5\xC2" "\n"
			//    请输入限制条件: 作者是 sysop(查找转信文章请输入 作者是 转信)\n
			"   " "\xC7\xEB\xCA\xE4\xC8\xEB\xCF\xDE\xD6\xC6\xCC\xF5\xBC\xFE" ": " "\xD7\xF7\xD5\xDF\xCA\xC7" " sysop(" "\xB2\xE9\xD5\xD2\xD7\xAA\xD0\xC5\xCE\xC4\xD5\xC2\xC7\xEB\xCA\xE4\xC8\xEB" " " "\xD7\xF7\xD5\xDF\xCA\xC7" " " "\xD7\xAA\xD0\xC5" ")\n"
			//    例子二: 找sysop写的灌水文章\n
			"   " "\xC0\xFD\xD7\xD3\xB6\xFE" ": " "\xD5\xD2" "sysop" "\xD0\xB4\xB5\xC4\xB9\xE0\xCB\xAE\xCE\xC4\xD5\xC2" "\n"
			//    请输入限制条件: 作者是 sysop 且 灌水字数少于 40\n
			"   " "\xC7\xEB\xCA\xE4\xC8\xEB\xCF\xDE\xD6\xC6\xCC\xF5\xBC\xFE" ": " "\xD7\xF7\xD5\xDF\xCA\xC7" " sysop " "\xC7\xD2" " " "\xB9\xE0\xCB\xAE\xD7\xD6\xCA\xFD\xC9\xD9\xD3\xDA" " 40\n"
			//    例子三: 找sysop写的灌水文章,而且被标记为m的\n
			"   " "\xC0\xFD\xD7\xD3\xC8\xFD" ": " "\xD5\xD2" "sysop" "\xD0\xB4\xB5\xC4\xB9\xE0\xCB\xAE\xCE\xC4\xD5\xC2" "," "\xB6\xF8\xC7\xD2\xB1\xBB\xB1\xEA\xBC\xC7\xCE\xAA" "m" "\xB5\xC4" "\n"
			//    请输入限制条件: 作者是 sysop 且 灌水字数少于 40 且 标记含 m\n
			"   " "\xC7\xEB\xCA\xE4\xC8\xEB\xCF\xDE\xD6\xC6\xCC\xF5\xBC\xFE" ": " "\xD7\xF7\xD5\xDF\xCA\xC7" " sysop " "\xC7\xD2" " " "\xB9\xE0\xCB\xAE\xD7\xD6\xCA\xFD\xC9\xD9\xD3\xDA" " 40 " "\xC7\xD2" " " "\xB1\xEA\xBC\xC7\xBA\xAC" " m\n"
			//    例子四: 找所有标题包含兵马俑或者bmy的文章\n
			"   " "\xC0\xFD\xD7\xD3\xCB\xC4" ": " "\xD5\xD2\xCB\xF9\xD3\xD0\xB1\xEA\xCC\xE2\xB0\xFC\xBA\xAC\xB1\xF8\xC2\xED\xD9\xB8\xBB\xF2\xD5\xDF" "bmy" "\xB5\xC4\xCE\xC4\xD5\xC2" "\n"
			//    请输入限制条件: 标题含 兵马俑 或 标题含 bmy\n
			"   " "\xC7\xEB\xCA\xE4\xC8\xEB\xCF\xDE\xD6\xC6\xCC\xF5\xBC\xFE" ": " "\xB1\xEA\xCC\xE2\xBA\xAC" " " "\xB1\xF8\xC2\xED\xD9\xB8" " " "\xBB\xF2" " " "\xB1\xEA\xCC\xE2\xBA\xAC" " bmy\n"
			//    例子五: 找所有不是sysop也不是XJTU-XANET发表的文章\n
			"   " "\xC0\xFD\xD7\xD3\xCE\xE5" ": " "\xD5\xD2\xCB\xF9\xD3\xD0\xB2\xBB\xCA\xC7" "sysop" "\xD2\xB2\xB2\xBB\xCA\xC7" "XJTU-XANET" "\xB7\xA2\xB1\xED\xB5\xC4\xCE\xC4\xD5\xC2" "\n"
			//    请输入限制条件: 非 (作者是 sysop 或 作者是 XJTU-XANET)\n
			"   " "\xC7\xEB\xCA\xE4\xC8\xEB\xCF\xDE\xD6\xC6\xCC\xF5\xBC\xFE" ": " "\xB7\xC7" " (" "\xD7\xF7\xD5\xDF\xCA\xC7" " sysop " "\xBB\xF2" " " "\xD7\xF7\xD5\xDF\xCA\xC7" " XJTU-XANET)\n"
			//    (或者)请输入限制条件: 作者不是 sysop 且 作者不是 XJTU-XANET\n
			"   (" "\xBB\xF2\xD5\xDF" ")" "\xC7\xEB\xCA\xE4\xC8\xEB\xCF\xDE\xD6\xC6\xCC\xF5\xBC\xFE" ": " "\xD7\xF7\xD5\xDF\xB2\xBB\xCA\xC7" " sysop " "\xC7\xD2" " " "\xD7\xF7\xD5\xDF\xB2\xBB\xCA\xC7" " XJTU-XANET\n"
			//    例子六: 找所有有附件的文章\n
			"   " "\xC0\xFD\xD7\xD3\xC1\xF9" ": " "\xD5\xD2\xCB\xF9\xD3\xD0\xD3\xD0\xB8\xBD\xBC\xFE\xB5\xC4\xCE\xC4\xD5\xC2" "\n"
			//    请输入限制条件: 标记含 @\n
			"   " "\xC7\xEB\xCA\xE4\xC8\xEB\xCF\xDE\xD6\xC6\xCC\xF5\xBC\xFE" ": " "\xB1\xEA\xBC\xC7\xBA\xAC" " @\n"
			//    例子七: 找所有推荐星级在3星以上的文章\n
			"   " "\xC0\xFD\xD7\xD3\xC6\xDF" ": " "\xD5\xD2\xCB\xF9\xD3\xD0\xCD\xC6\xBC\xF6\xD0\xC7\xBC\xB6\xD4\xDA" "3" "\xD0\xC7\xD2\xD4\xC9\xCF\xB5\xC4\xCE\xC4\xD5\xC2" "\n"
			//    请输入限制条件: 推荐星级高于 3\n
			"   " "\xC7\xEB\xCA\xE4\xC8\xEB\xCF\xDE\xD6\xC6\xCC\xF5\xBC\xFE" ": " "\xCD\xC6\xBC\xF6\xD0\xC7\xBC\xB6\xB8\xDF\xD3\xDA" " 3\n"
			//    例子八: 找所有推荐人数在10人以上的文章\n
			"   " "\xC0\xFD\xD7\xD3\xB0\xCB" ": " "\xD5\xD2\xCB\xF9\xD3\xD0\xCD\xC6\xBC\xF6\xC8\xCB\xCA\xFD\xD4\xDA" "10" "\xC8\xCB\xD2\xD4\xC9\xCF\xB5\xC4\xCE\xC4\xD5\xC2" "\n"
			//    请输入限制条件: 推荐人数高于 10\n
			"   " "\xC7\xEB\xCA\xE4\xC8\xEB\xCF\xDE\xD6\xC6\xCC\xF5\xBC\xFE" ": " "\xCD\xC6\xBC\xF6\xC8\xCB\xCA\xFD\xB8\xDF\xD3\xDA" " 10\n");

	// 请输入限制条件:
	getdata(5, 0, "\xC7\xEB\xCA\xE4\xC8\xEB\xCF\xDE\xD6\xC6\xCC\xF5\xBC\xFE" ": ", select, 60, DOECHO, NA);
	clrtobot();
	if (IScurrBM)
		getdata(6, 0,
			// 请输入你希望的操作: 0)取消 1)标记删除 2)标记减文章数删除 3)标记精华 9)阅读:
			"\xC7\xEB\xCA\xE4\xC8\xEB\xC4\xE3\xCF\xA3\xCD\xFB\xB5\xC4\xB2\xD9\xD7\xF7" ": 0)" "\xC8\xA1\xCF\xFB" " 1)" "\xB1\xEA\xBC\xC7\xC9\xBE\xB3\xFD" " 2)" "\xB1\xEA\xBC\xC7\xBC\xF5\xCE\xC4\xD5\xC2\xCA\xFD\xC9\xBE\xB3\xFD" " 3)" "\xB1\xEA\xBC\xC7\xBE\xAB\xBB\xAA" " 9)" "\xD4\xC4\xB6\xC1" ":",
			num, 2, DOECHO, YEA);
	else
		getdata(6, 0,
			// 请输入你希望的操作: 0)取消 9)阅读:
			"\xC7\xEB\xCA\xE4\xC8\xEB\xC4\xE3\xCF\xA3\xCD\xFB\xB5\xC4\xB2\xD9\xD7\xF7" ": 0)" "\xC8\xA1\xCF\xFB" " 9)" "\xD4\xC4\xB6\xC1" ":",
			num, 2, DOECHO, YEA);
	answer = atoi(num);
	if (answer > 0 && answer < 9 && !IScurrBM) {
		// 您选择的功能不能使用\n
		prints("\xC4\xFA\xD1\xA1\xD4\xF1\xB5\xC4\xB9\xA6\xC4\xDC\xB2\xBB\xC4\xDC\xCA\xB9\xD3\xC3" "\n");
		pressreturn();
		return FULLUPDATE;
	}
	return power_action(dir, inum1, inum2, select, answer);
}
