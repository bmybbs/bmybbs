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
#include "smth_screen.h"
#include "io.h"
#include "maintain.h"
#include "stuff.h"
#include "xyz.h"
#include "namecomplete.h"
#include "bcache.h"
#include "userinfo.h"
#include "more.h"
#include "bm.h"
#include "bbsinc.h"
#include "announce.h"
#include "mail.h"
#include "bbs_global_vars.h"
#include "bmy/board.h"

char cexplain[STRLEN];
char lookgrp[30];

static int valid_brdname(char *brd);
static char *chgrp(void);
static int freeclubnum(void);
static int setsecstr(char *buf, int ln);
static void anno_title(char *buf, struct boardheader *bh);

//proto.h中有了
//int release_email(char *userid, char *email); //释放邮箱

int
check_systempasswd(void)
{
	FILE *pass;
	char passbuf[20], prepass[STRLEN];

	clear();
	if ((pass = fopen("etc/.syspasswd", "r")) != NULL) {
		fgets(prepass, STRLEN, pass);
		fclose(pass);
		prepass[strlen(prepass) - 1] = '\0';
		// 请输入系统密码:
		getdata(1, 0, "\xC7\xEB\xCA\xE4\xC8\xEB\xCF\xB5\xCD\xB3\xC3\xDC\xC2\xEB: ", passbuf, 19, NOECHO, YEA);
		if (passbuf[0] == '\0' || passbuf[0] == '\n')
			return NA;
		if (!ytht_crypt_checkpasswd(prepass, passbuf)) {
			move(2, 0);
			// 错误的系统密码...
			prints("\xB4\xED\xCE\xF3\xB5\xC4\xCF\xB5\xCD\xB3\xC3\xDC\xC2\xEB...");
			// 系统密码输入错误...
			// 系统密码输入错误...
			securityreport("\xCF\xB5\xCD\xB3\xC3\xDC\xC2\xEB\xCA\xE4\xC8\xEB\xB4\xED\xCE\xF3...", "\xCF\xB5\xCD\xB3\xC3\xDC\xC2\xEB\xCA\xE4\xC8\xEB\xB4\xED\xCE\xF3...");
			pressanykey();
			return NA;
		}
	}
	return YEA;
}

void deliverreport(char *title, char *str)
{
	FILE *se;
	char fname[STRLEN];
	int savemode;

	savemode = uinfo.mode;
	sprintf(fname, "bbstmpfs/tmp/deliver.%s.%05d", currentuser.userid,
		uinfo.pid);
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "%s", str);
		fclose(se);
		postfile(fname, currboard, title, 1);
		unlink(fname);
		modify_user_mode(savemode);
	}
}

void securityreport(char *str, char *content)
{
	FILE *se;
	char fname[STRLEN];
	int savemode;

	savemode = uinfo.mode;
	//report(str);
	sprintf(fname, "bbstmpfs/tmp/security.%s.%05d", currentuser.userid,
		uinfo.pid);
	if ((se = fopen(fname, "w")) != NULL) {
		// 系统安全记录系统\n原因：\n%s\n
		fprintf(se, "\xCF\xB5\xCD\xB3\xB0\xB2\xC8\xAB\xBC\xC7\xC2\xBC\xCF\xB5\xCD\xB3\n\xD4\xAD\xD2\xF2\xA3\xBA\n%s\n", content);
		// 以下是部分个人资料\n
		fprintf(se, "\xD2\xD4\xCF\xC2\xCA\xC7\xB2\xBF\xB7\xD6\xB8\xF6\xC8\xCB\xD7\xCA\xC1\xCF\n");
		// 最近光临机器: %s
		fprintf(se, "\xD7\xEE\xBD\xFC\xB9\xE2\xC1\xD9\xBB\xFA\xC6\xF7: %s", currentuser.lasthost);
		fclose(se);
		postfile(fname, "syssecurity", str, 2);
		unlink(fname);
		modify_user_mode(savemode);
	}
}

int get_grp(const char *seekstr)
{
	FILE *fp;
	char buf[STRLEN];
	char *namep;

	if ((fp = fopen("0Announce/.Search", "r")) == NULL)
		return 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		namep = strtok(buf, ": \n\r\t");
		if (namep != NULL && strcasecmp(namep, seekstr) == 0) {
			fclose(fp);
			strtok(NULL, "/");
			namep = strtok(NULL, "/");
			if (strlen(namep) < 30) {
				strcpy(lookgrp, namep);
				return 1;
			} else
				return 0;
		}
	}
	fclose(fp);
	return 0;
}

void stand_title(char *title)
{
	clear();
	//standout();
	prints(title);
	//standend();
}

int m_info(const char *s) {
	(void) s;
	struct userec local_uinfo;
	int id;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();
	// 修改使用者代号
	stand_title("\xD0\xDE\xB8\xC4\xCA\xB9\xD3\xC3\xD5\xDF\xB4\xFA\xBA\xC5");
	move(1, 0);
	// 请输入使用者代号:
	usercomplete("\xC7\xEB\xCA\xE4\xC8\xEB\xCA\xB9\xD3\xC3\xD5\xDF\xB4\xFA\xBA\xC5: ", genbuf);
	if (*genbuf == '\0') {
		clear();
		return -1;
	}

	if (!(id = getuser(genbuf))) {
		move(3, 0);
		// 错误的使用者代号
		prints("\xB4\xED\xCE\xF3\xB5\xC4\xCA\xB9\xD3\xC3\xD5\xDF\xB4\xFA\xBA\xC5");
		clrtoeol();
		pressreturn();
		clear();
		return -1;
	}
	memcpy(&local_uinfo, &lookupuser, sizeof (local_uinfo));

	move(1, 0);
	clrtobot();
	disply_userinfo(&local_uinfo, 1);
	uinfo_query(&local_uinfo, 1, id);
	return 0;
}

static int valid_brdname(char *brd)
{
	char ch;

	ch = *brd++;
	if (!isalnum(ch) && ch != '_')
		return 0;
	while ((ch = *brd++) != '\0') {
		if (!isalnum(ch) && ch != '_')
			return 0;
	}
	return 1;
}

static char *
chgrp(void)
{
	int i, ch;
	static char buf[STRLEN];
	char ans[6];

/*下面两个数组因分类变化而修改 by ylsdd*/
#if 0
	static char *const explain[] = {
		// 本站系统
		"\xB1\xBE\xD5\xBE\xCF\xB5\xCD\xB3",
		// 交通大学
		"\xBD\xBB\xCD\xA8\xB4\xF3\xD1\xA7",
		// 开发技术
		"\xBF\xAA\xB7\xA2\xBC\xBC\xCA\xF5",
		// 电脑应用
		"\xB5\xE7\xC4\xD4\xD3\xA6\xD3\xC3",
		// 学术科学
		"\xD1\xA7\xCA\xF5\xBF\xC6\xD1\xA7",
		// 社会科学
		"\xC9\xE7\xBB\xE1\xBF\xC6\xD1\xA7",
		// 文学艺术
		"\xCE\xC4\xD1\xA7\xD2\xD5\xCA\xF5",
		// 知性感性
		"\xD6\xAA\xD0\xD4\xB8\xD0\xD0\xD4",
		// 体育运动
		"\xCC\xE5\xD3\xFD\xD4\xCB\xB6\xAF",
		// 休闲音乐
		"\xD0\xDD\xCF\xD0\xD2\xF4\xC0\xD6",
		// 游戏天地
		"\xD3\xCE\xCF\xB7\xCC\xEC\xB5\xD8",
		// 兄弟院校
		"\xD0\xD6\xB5\xDC\xD4\xBA\xD0\xA3",
		// 新闻信息
		"\xD0\xC2\xCE\xC5\xD0\xC5\xCF\xA2",
		// 乡音乡情
		"\xCF\xE7\xD2\xF4\xCF\xE7\xC7\xE9",
		"TEMP",
		NULL
	};

	static char *const groups[] = {
		"GROUP_0",
		"GROUP_1",
		"GROUP_2",
		"GROUP_3",
		"GROUP_4",
		"GROUP_5",
		"GROUP_6",
		"GROUP_7",
		"GROUP_8",
		"GROUP_9",
		"GROUP_G",
		"GROUP_B",
		"GROUP_N",
		"GROUP_H",
		"GROUP_S",
		NULL
	};
#endif
	clear();
	move(2, 0);
	// 选择精华区的目录\n\n
	prints("\xD1\xA1\xD4\xF1\xBE\xAB\xBB\xAA\xC7\xF8\xB5\xC4\xC4\xBF\xC2\xBC\n\n");
	for (i = 0; i < sectree.nsubsec; i++) {
		prints("\033[1;32m%2d\033[m. %-20s                GROUP_%c\n", i,
				sectree.subsec[i]->title, sectree.subsec[i]->basestr[0]);
	}
	// 请输入你的选择(0~%d):
	sprintf(buf, "\xC7\xEB\xCA\xE4\xC8\xEB\xC4\xE3\xB5\xC4\xD1\xA1\xD4\xF1(0~%d): ", --i);
	while (1) {
		getdata(i + 6, 0, buf, ans, 4, DOECHO, YEA);
		if (!isdigit(ans[0]))
			continue;
		ch = atoi(ans);
		if (ch < 0 || ch > i || ans[0] == '\r' || ans[0] == '\0')
			continue;
		else
			break;
	}
	strcpy(cexplain, sectree.subsec[ch]->title);
	snprintf(buf, sizeof (buf), "GROUP_%c", sectree.subsec[ch]->basestr[0]);
	return buf;
}

static int
freeclubnum(void)
{
	FILE *fp;
	int club[4] = { 0, 0, 0, 0 };
	int i;
	struct boardheader rec;
	if ((fp = fopen(BOARDS, "r")) == NULL) {
		return -1;
	}
	while (!feof(fp)) {
		fread(&rec, sizeof (struct boardheader), 1, fp);
		if (rec.clubnum != 0)
			club[rec.clubnum / 32] |= (1 << (rec.clubnum % 32));
	}
	fclose(fp);
	for (i = 1; i < 32 * 4; i++)
		if ((~club[i / 32]) & (1 << (i % 32))) {
			return i;
		}
	return -1;
}

static int
setsecstr(char *buf, int ln)
{
	const struct sectree *sec;
	int i = 0, ch, len, choose = 0;
	sec = getsectree(buf);
	move(ln, 0);
	clrtobot();
	while (1) {
		// =======当前分区选择: \033[31m%s\033[0;1m %s\033[m =======\n
		prints("=======\xB5\xB1\xC7\xB0\xB7\xD6\xC7\xF8\xD1\xA1\xD4\xF1: \033[31m%s\033[0;1m %s\033[m =======\n", sec->basestr, sec->title);
		if (sec->parent) {
			//  (\033[4;33m#\033[0m) 回上级分区\n
			prints(" (\033[4;33m#\033[0m) \xBB\xD8\xC9\xCF\xBC\xB6\xB7\xD6\xC7\xF8\n");
			//  (\033[4;33m%%\033[0m) 就放在这里\n
			prints(" (\033[4;33m%%\033[0m) \xBE\xCD\xB7\xC5\xD4\xDA\xD5\xE2\xC0\xEF\n");
		}
		//  (\033[4;33m*\033[0m) 保持原来设定(可用回车选定本项)\n
		prints(" (\033[4;33m*\033[0m) \xB1\xA3\xB3\xD6\xD4\xAD\xC0\xB4\xC9\xE8\xB6\xA8(\xBF\xC9\xD3\xC3\xBB\xD8\xB3\xB5\xD1\xA1\xB6\xA8\xB1\xBE\xCF\xEE)\n");
		len = strlen(sec->basestr);
		for (i = 0; i < sec->nsubsec; i++) {
			if (i && !(i % 3))
				prints("\n");
			ch = sec->subsec[i]->basestr[len];
			prints(" (\033[4;33m%c\033[0m) \033[31;1m %s\033[0m", ch, sec->subsec[i]->title);
		}
		// \n请按括号内的字母选择
		prints("\n\xC7\xEB\xB0\xB4\xC0\xA8\xBA\xC5\xC4\xDA\xB5\xC4\xD7\xD6\xC4\xB8\xD1\xA1\xD4\xF1");
		while (1) {
			ch = igetkey();
			if (ch == '\n' || ch == '\r')
				ch = '*';
			if (sec->parent == NULL && (ch == '#' || ch == '%'))
				continue;
			for (i = 0; i < sec->nsubsec; i++) {
				if (sec->subsec[i]->basestr[len] == ch) {
					choose = i;
					break;
				}
			}
			if (ch != '#' && ch != '*' && ch != '%' && i == sec->nsubsec) continue;
			break;
		}
		move(ln, 0);
		clrtobot();
		switch (ch) {
		case '#':
			sec = sec->parent;
			break;
		case '%':
			strcpy(buf, sec->basestr);
			return 0;
		case '*':
			strcpy(buf, "");
			return 0;
		default:
			sec = sec->subsec[choose];
		}
	}
}

int m_newbrd(const char *s) {
	(void) s;
	struct boardheader newboard;
	char ans[4];
	char vbuf[100];
	char *group;
	int bid;
	time_t now;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();
	// 开启新讨论区
	stand_title("\xBF\xAA\xC6\xF4\xD0\xC2\xCC\xD6\xC2\xDB\xC7\xF8");
	memset(&newboard, 0, sizeof (newboard));
	move(2, 0);
	ansimore2("etc/boardref", NA, 1, 11);
	while (1) {
		// 讨论区名称:
		getdata(10, 0, "\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB\xB3\xC6:   ", newboard.filename, 18, DOECHO,
			YEA);
		if (newboard.filename[0] != 0) {
			struct boardheader dh;
			if (new_search_record(BOARDS, &dh, sizeof (dh), (void *) cmpbnames, newboard.filename)) {
				// \n错误! 此讨论区已经存在!!
				prints("\n\xB4\xED\xCE\xF3! \xB4\xCB\xCC\xD6\xC2\xDB\xC7\xF8\xD2\xD1\xBE\xAD\xB4\xE6\xD4\xDA!!");
				pressanykey();
				return -1;
			}
		} else
			return -1;
		if (valid_brdname(newboard.filename))
			break;
		// \n不合法名称!!
		prints("\n\xB2\xBB\xBA\xCF\xB7\xA8\xC3\xFB\xB3\xC6!!");
	}
	// 讨论区中文名:
	getdata(11, 0, "\xCC\xD6\xC2\xDB\xC7\xF8\xD6\xD0\xCE\xC4\xC3\xFB: ", newboard.title,
		sizeof (newboard.title), DOECHO, YEA);
	if (newboard.title[0] == '\0')
		return -1;
	strcpy(vbuf, "vote/");
	strcat(vbuf, newboard.filename);
	setbpath(genbuf, sizeof(genbuf), newboard.filename);
	if (getbnum(newboard.filename) > 0 || mkdir(genbuf, 0777) == -1 || mkdir(vbuf, 0777) == -1) {
		// \n错误的讨论区名称!!\n
		prints("\n\xB4\xED\xCE\xF3\xB5\xC4\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB\xB3\xC6!!\n");
		pressreturn();
		rmdir(vbuf);
		rmdir(genbuf);
		clear();
		return -1;
	}
	move(12, 0);
	// 选择主分区:
	prints("\xD1\xA1\xD4\xF1\xD6\xF7\xB7\xD6\xC7\xF8: ");
	while (1) {
		genbuf[0] = 0;
		setsecstr(genbuf, 13);
		if (genbuf[0] != '\0')
			break;
	}
	move(12, 0);
	// 主分区设定: %s
	prints("\xD6\xF7\xB7\xD6\xC7\xF8\xC9\xE8\xB6\xA8: %s", genbuf);
	newboard.secnumber1 = genbuf[0];
	ytht_strsncpy(newboard.sec1, genbuf, sizeof(newboard.sec1));
	move(12, 30);
	// 选择分区链接:
	prints("\xD1\xA1\xD4\xF1\xB7\xD6\xC7\xF8\xC1\xB4\xBD\xD3: ");
	genbuf[0] = 0;
	setsecstr(genbuf, 13);
	move(12, 30);
	// 分区链接设定: %s
	prints("\xB7\xD6\xC7\xF8\xC1\xB4\xBD\xD3\xC9\xE8\xB6\xA8: %s", genbuf);
	newboard.secnumber2 = genbuf[0];
	ytht_strsncpy(newboard.sec2, genbuf, sizeof(newboard.sec2));
	move(13, 0);
	while (1) {
		// 讨论区分类(4字):
		getdata(13, 0, "\xCC\xD6\xC2\xDB\xC7\xF8\xB7\xD6\xC0\xE0(4\xD7\xD6):", newboard.type,
			sizeof (newboard.type), DOECHO, YEA);
		if (strlen(newboard.type) == 4)
			break;
	}
	move(14, 0);
	if (newboard.secnumber2 == 'C') {
		newboard.flag &= ~ANONY_FLAG;
		newboard.level = 0;
		if ((newboard.clubnum = freeclubnum()) == -1) {
			// 没有空的俱乐部位置了
			prints("\xC3\xBB\xD3\xD0\xBF\xD5\xB5\xC4\xBE\xE3\xC0\xD6\xB2\xBF\xCE\xBB\xD6\xC3\xC1\xCB");
			pressreturn();
			clear();
			return -1;
		}
		sprintf(genbuf, "%d", newboard.clubnum);
		// 是否是开放式俱乐部
		if (askyn("\xCA\xC7\xB7\xF1\xCA\xC7\xBF\xAA\xB7\xC5\xCA\xBD\xBE\xE3\xC0\xD6\xB2\xBF", YEA, NA) == YEA)
			newboard.flag |= CLUBTYPE_FLAG;
		else
			newboard.flag &= ~CLUBTYPE_FLAG;
	} else {
		// 是否限制存取权利
		if (askyn("\xCA\xC7\xB7\xF1\xCF\xDE\xD6\xC6\xB4\xE6\xC8\xA1\xC8\xA8\xC0\xFB", NA, NA) == YEA) {
			// 限制 Read/Post? [R]:
			getdata(15, 0, "\xCF\xDE\xD6\xC6 Read/Post? [R]: ", ans, 2, DOECHO,
				YEA);
			if (*ans == 'P' || *ans == 'p')
				newboard.level = PERM_POSTMASK;
			else
				newboard.level = 0;
			move(1, 0);
			clrtobot();
			move(2, 0);
			// 设定 %s 权利. 讨论区: '%s'\n
			prints("\xC9\xE8\xB6\xA8 %s \xC8\xA8\xC0\xFB. \xCC\xD6\xC2\xDB\xC7\xF8: '%s'\n", (newboard.level & PERM_POSTMASK ? "POST" : "READ"), newboard.filename);
			// 权限
			newboard.level = setperms(newboard.level, "\xC8\xA8\xCF\xDE", NUMPERMS, showperminfo, 0);
			clear();
		} else
			newboard.level = 0;

		move(15, 0);
		// 是否加入匿名版
		if (askyn("\xCA\xC7\xB7\xF1\xBC\xD3\xC8\xEB\xC4\xE4\xC3\xFB\xB0\xE6", NA, NA) == YEA)
			newboard.flag |= ANONY_FLAG;
		else
			newboard.flag &= ~ANONY_FLAG;
	}
	move(16, 0);
	// 是否是转信版面
	if (askyn("\xCA\xC7\xB7\xF1\xCA\xC7\xD7\xAA\xD0\xC5\xB0\xE6\xC3\xE6", NA, NA) == YEA)
		newboard.flag |= INNBBSD_FLAG;
	else
		newboard.flag &= ~INNBBSD_FLAG;

	// 是否是需要进行内容检查的版面
	if (askyn("\xCA\xC7\xB7\xF1\xCA\xC7\xD0\xE8\xD2\xAA\xBD\xF8\xD0\xD0\xC4\xDA\xC8\xDD\xBC\xEC\xB2\xE9\xB5\xC4\xB0\xE6\xC3\xE6", NA, NA) == YEA)
		newboard.flag |= IS1984_FLAG;
	else
		newboard.flag &= ~IS1984_FLAG;
	// 版面内容是否可能和政治相关
	if (askyn("\xB0\xE6\xC3\xE6\xC4\xDA\xC8\xDD\xCA\xC7\xB7\xF1\xBF\xC9\xC4\xDC\xBA\xCD\xD5\xFE\xD6\xCE\xCF\xE0\xB9\xD8", NA, NA) == YEA)
		newboard.flag |= POLITICAL_FLAG;
	else
		newboard.flag &= ~POLITICAL_FLAG;
	now = time(NULL);
	newboard.board_mtime = now;
	newboard.board_ctime = now;

	if ((bid = getbnum("")) > 0) {
		substitute_record(BOARDS, &newboard, sizeof (newboard), bid);
	} else if (append_record(BOARDS, &newboard, sizeof (newboard)) == -1) {
		pressreturn();
		clear();
		return -1;
	}

	ythtbbs_cache_Board_resolve();
	if (!bmy_board_is_system_board(newboard.filename)) {
		int boardnum = ythtbbs_cache_Board_get_idx_by_name(newboard.filename) + 1;
		if (boardnum > 0) {
			bmy_board_create(boardnum, newboard.filename, newboard.title, newboard.sec1);
		}
	}

	group = chgrp();
	sprintf(vbuf, "%-38.38s", newboard.title);
	if (group != NULL) {
		if (add_grp(group, cexplain, newboard.filename, vbuf) == -1)
			// \n成立精华区失败....\n
			prints("\n\xB3\xC9\xC1\xA2\xBE\xAB\xBB\xAA\xC7\xF8\xCA\xA7\xB0\xDC....\n");
		else
			// 已经置入精华区...\n
			prints("\xD2\xD1\xBE\xAD\xD6\xC3\xC8\xEB\xBE\xAB\xBB\xAA\xC7\xF8...\n");
	}

	// \n新讨论区成立\n
	prints("\n\xD0\xC2\xCC\xD6\xC2\xDB\xC7\xF8\xB3\xC9\xC1\xA2\n");
	{
		char secu[STRLEN];
		// 成立新版：%s
		sprintf(secu, "\xB3\xC9\xC1\xA2\xD0\xC2\xB0\xE6\xA3\xBA%s", newboard.filename);
		securityreport(secu, secu);
	}
	pressreturn();
	clear();
	return 0;
}

static void anno_title(char *buf, struct boardheader *bh)
{
	char bm[IDLEN * 4 + 4];	//放四个版务
	sprintf(buf, "%-38.38s", bh->title);
	if (bh->bm[0][0] == 0)
		return;
	else {
		strcat(buf, "(BM:");
		bm2str(bm, bh);
		strcat(buf, bm);
	}
	strcat(buf, ")");
	return;
}

int m_editbrd(const char *s) {
	(void) s;
	char bname[STRLEN], buf[STRLEN], oldtitle[STRLEN], vbuf[256], *group;
	char oldpath[STRLEN], newpath[STRLEN], tmp_grp[30];
	int pos, noidboard, a_mv, isclub, innboard, isopenclub, is1984;
	int political;
	struct boardheader fh, newfh;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();
	// 修改讨论区资讯
	stand_title("\xD0\xDE\xB8\xC4\xCC\xD6\xC2\xDB\xC7\xF8\xD7\xCA\xD1\xB6");
	move(1, 0);
	make_blist_full();
	// 输入讨论区名称:
	namecomplete("\xCA\xE4\xC8\xEB\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB\xB3\xC6: ", bname);
	if (*bname == '\0') {
		move(2, 0);
		// 错误的讨论区名称
		prints("\xB4\xED\xCE\xF3\xB5\xC4\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB\xB3\xC6");
		pressreturn();
		clear();
		return -1;
	}
	pos = new_search_record(BOARDS, &fh, sizeof (fh), (void *) cmpbnames, bname);
	if (!pos) {
		move(2, 0);
		// 错误的讨论区名称
		prints("\xB4\xED\xCE\xF3\xB5\xC4\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB\xB3\xC6");
		pressreturn();
		clear();
		return -1;
	}
	noidboard = fh.flag & ANONY_FLAG;
	isclub = (fh.clubnum > 0);
	innboard = (fh.flag & INNBBSD_FLAG) ? YEA : NA;
	isopenclub = fh.flag & CLUBTYPE_FLAG;
	is1984 = fh.flag & IS1984_FLAG;
	political = fh.flag & POLITICAL_FLAG;
	move(2, 0);
	memcpy(&newfh, &fh, sizeof (newfh));
	// 讨论区名称:   %s
	prints("\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB\xB3\xC6:   %s", fh.filename);
	move(2, 40);
	// 讨论区说明:   %s\n
	prints("\xCC\xD6\xC2\xDB\xC7\xF8\xCB\xB5\xC3\xF7:   %s\n", fh.title);
	// 匿名讨论区:   %s  俱乐部版面：  %s  转信讨论区：  %s\n
	prints("\xC4\xE4\xC3\xFB\xCC\xD6\xC2\xDB\xC7\xF8:   %s  \xBE\xE3\xC0\xD6\xB2\xBF\xB0\xE6\xC3\xE6\xA3\xBA  %s  \xD7\xAA\xD0\xC5\xCC\xD6\xC2\xDB\xC7\xF8\xA3\xBA  %s\n",
			(noidboard) ? "Yes" : "No", (isclub) ? "Yes" : "No",
			(innboard) ? "Yes" : "No");
	strcpy(oldtitle, fh.title);
	// 限制 %s 权利: %s
	prints("\xCF\xDE\xD6\xC6 %s \xC8\xA8\xC0\xFB: %s",
			(fh.level & PERM_POSTMASK) ? "POST" :
			(fh.level & PERM_NOZAP) ? "ZAP" : "READ",
			// 不设限
			// 有设限
			(fh.level & ~PERM_POSTMASK) == 0 ? "\xB2\xBB\xC9\xE8\xCF\xDE" : "\xD3\xD0\xC9\xE8\xCF\xDE");
	//  %s进行人工文章审查
	// 要
	// 不
	prints(" %s\xBD\xF8\xD0\xD0\xC8\xCB\xB9\xA4\xCE\xC4\xD5\xC2\xC9\xF3\xB2\xE9", is1984 ? "\xD2\xAA" : "\xB2\xBB");
	if (political)
		//  内容可能和政治相关
		prints(" \xC4\xDA\xC8\xDD\xBF\xC9\xC4\xDC\xBA\xCD\xD5\xFE\xD6\xCE\xCF\xE0\xB9\xD8");
	move(5, 0);
	// 是否更改以上资讯
	if (askyn("\xCA\xC7\xB7\xF1\xB8\xFC\xB8\xC4\xD2\xD4\xC9\xCF\xD7\xCA\xD1\xB6", NA, NA) == YEA) {
		move(6, 0);
		// 直接按 <Return> 不修改此栏资讯...
		prints("\xD6\xB1\xBD\xD3\xB0\xB4 <Return> \xB2\xBB\xD0\xDE\xB8\xC4\xB4\xCB\xC0\xB8\xD7\xCA\xD1\xB6...");
enterbname:
		// 新讨论区名称:
		getdata(7, 0, "\xD0\xC2\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB\xB3\xC6: ", genbuf, 18, DOECHO, YEA);
		if (genbuf[0] != 0) {
			struct boardheader dh;
			if (new_search_record(BOARDS, &dh, sizeof (dh), (void *) cmpbnames, genbuf)) {
				move(2, 0);
				// 错误! 此讨论区已经存在!!
				prints("\xB4\xED\xCE\xF3! \xB4\xCB\xCC\xD6\xC2\xDB\xC7\xF8\xD2\xD1\xBE\xAD\xB4\xE6\xD4\xDA!!");
				move(7, 0);
				clrtoeol();
				goto enterbname;
			}
			if (valid_brdname(genbuf)) {
				ytht_strsncpy(newfh.filename, genbuf, sizeof(newfh.filename));
				strcpy(bname, genbuf);
			} else {
				move(2, 0);
				// 不合法的讨论区名称!
				prints("\xB2\xBB\xBA\xCF\xB7\xA8\xB5\xC4\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB\xB3\xC6!");
				move(7, 0);
				clrtoeol();
				goto enterbname;
			}
		}
		// 新讨论区中文名:
		getdata(8, 0, "\xD0\xC2\xCC\xD6\xC2\xDB\xC7\xF8\xD6\xD0\xCE\xC4\xC3\xFB: ", genbuf, 24, DOECHO, YEA);
		if (genbuf[0] != 0)
			ytht_strsncpy(newfh.title, genbuf, sizeof(newfh.title));
		ansimore2("etc/boardref", NA, 9, 7);
		strcpy(genbuf, newfh.sec1);
		move(16, 0);
		// 选择新分区: %s
		prints("\xD1\xA1\xD4\xF1\xD0\xC2\xB7\xD6\xC7\xF8: %s", genbuf);
		setsecstr(genbuf, 17);
		if (genbuf[0] != 0) {
			newfh.secnumber1 = genbuf[0];
			ytht_strsncpy(newfh.sec1, genbuf, sizeof(newfh.sec1));
		}
		move(16, 0);
		// 新分区设定: %s
		prints("\xD0\xC2\xB7\xD6\xC7\xF8\xC9\xE8\xB6\xA8: %s", genbuf);
		move(16, 40);
		strcpy(genbuf, newfh.sec2);
		// 选择新分区链接: %s
		prints("\xD1\xA1\xD4\xF1\xD0\xC2\xB7\xD6\xC7\xF8\xC1\xB4\xBD\xD3: %s", genbuf);
		setsecstr(genbuf, 17);
		newfh.secnumber2 = genbuf[0];
		ytht_strsncpy(newfh.sec2, genbuf, sizeof(newfh.sec2));
		move(16, 40);
		// 新分区链接设定: %s
		prints("\xD0\xC2\xB7\xD6\xC7\xF8\xC1\xB4\xBD\xD3\xC9\xE8\xB6\xA8: %s", genbuf);
		// 新讨论区分类(4字):
		getdata(17, 0, "\xD0\xC2\xCC\xD6\xC2\xDB\xC7\xF8\xB7\xD6\xC0\xE0(4\xD7\xD6): ", genbuf, 5, DOECHO, YEA);
		if (genbuf[0] != 0)
			ytht_strsncpy(newfh.type, genbuf, sizeof(newfh.type));
		move(18, 0);
		// 是否是转信版面
		if (askyn("\xCA\xC7\xB7\xF1\xCA\xC7\xD7\xAA\xD0\xC5\xB0\xE6\xC3\xE6", innboard, NA) == YEA)
			newfh.flag |= INNBBSD_FLAG;
		else
			newfh.flag &= ~INNBBSD_FLAG;
		move(18, 28);
		// 是否是需要进行内容检查的版面
		if (askyn("\xCA\xC7\xB7\xF1\xCA\xC7\xD0\xE8\xD2\xAA\xBD\xF8\xD0\xD0\xC4\xDA\xC8\xDD\xBC\xEC\xB2\xE9\xB5\xC4\xB0\xE6\xC3\xE6", is1984, NA) == YEA)
			newfh.flag |= IS1984_FLAG;
		else
			newfh.flag &= ~IS1984_FLAG;
		// 版面内容是否可能和政治相关
		if (askyn("\xB0\xE6\xC3\xE6\xC4\xDA\xC8\xDD\xCA\xC7\xB7\xF1\xBF\xC9\xC4\xDC\xBA\xCD\xD5\xFE\xD6\xCE\xCF\xE0\xB9\xD8", political, NA) == YEA)
			newfh.flag |= POLITICAL_FLAG;
		else
			newfh.flag &= ~POLITICAL_FLAG;

		genbuf[0] = 0;
		move(19, 0);
		// 是否移动精华区的位置
		if (askyn("\xCA\xC7\xB7\xF1\xD2\xC6\xB6\xAF\xBE\xAB\xBB\xAA\xC7\xF8\xB5\xC4\xCE\xBB\xD6\xC3", NA, NA) == YEA)
			a_mv = 2;
		else
			a_mv = 0;
		move(20, 0);
		if (newfh.secnumber2 == 'C')	//是俱乐部版面
		{
			newfh.flag &= ~ANONY_FLAG;
			newfh.level = 0;
			if (fh.clubnum)
				newfh.clubnum = fh.clubnum;
			else
				newfh.clubnum = freeclubnum();
			// 是否是开放式俱乐部
			if (askyn("\xCA\xC7\xB7\xF1\xCA\xC7\xBF\xAA\xB7\xC5\xCA\xBD\xBE\xE3\xC0\xD6\xB2\xBF", isopenclub, NA) == YEA)
				newfh.flag |= CLUBTYPE_FLAG;
			else
				newfh.flag &= ~CLUBTYPE_FLAG;
			// 确定要更改吗? (Y/N) [N]:
			getdata(21, 0, "\xC8\xB7\xB6\xA8\xD2\xAA\xB8\xFC\xB8\xC4\xC2\xF0? (Y/N) [N]: ", genbuf, 4,
				DOECHO, YEA);
		} else {
			newfh.clubnum = 0;
			// 匿名版 (Y/N)? [%c]:
			sprintf(buf, "\xC4\xE4\xC3\xFB\xB0\xE6 (Y/N)? [%c]: ",
				(noidboard) ? 'Y' : 'N');
			getdata(20, 0, buf, genbuf, 4, DOECHO, YEA);
			if (*genbuf == 'y' || *genbuf == 'Y' || *genbuf == 'N' || *genbuf == 'n') {
				if (*genbuf == 'y' || *genbuf == 'Y')
					newfh.flag |= ANONY_FLAG;
				else
					newfh.flag &= ~ANONY_FLAG;
			}
			// 是否更改存取权限
			if (askyn("\xCA\xC7\xB7\xF1\xB8\xFC\xB8\xC4\xB4\xE6\xC8\xA1\xC8\xA8\xCF\xDE", NA, NA) == YEA) {
				char ans[4];
				sprintf(genbuf,
					// 限制 (R)阅读 或 (P)张贴 文章 [%c]:
					"\xCF\xDE\xD6\xC6 (R)\xD4\xC4\xB6\xC1 \xBB\xF2 (P)\xD5\xC5\xCC\xF9 \xCE\xC4\xD5\xC2 [%c]: ",
					(newfh.level & PERM_POSTMASK ? 'P' : 'R'));
				getdata(21, 0, genbuf, ans, 2, DOECHO, YEA);
				if ((newfh.level & PERM_POSTMASK) && (*ans == 'R' || *ans == 'r'))
					newfh.level &= ~PERM_POSTMASK;
				else if (!(newfh.level & PERM_POSTMASK) && (*ans == 'P' || *ans == 'p'))
					newfh.level |= PERM_POSTMASK;
				clear();
				move(2, 0);
				// 设定 %s '%s' 讨论区的权限\n
				prints("\xC9\xE8\xB6\xA8 %s '%s' \xCC\xD6\xC2\xDB\xC7\xF8\xB5\xC4\xC8\xA8\xCF\xDE\n",
						// 张贴
						newfh.level & PERM_POSTMASK ? "\xD5\xC5\xCC\xF9" :
						// 阅读
						"\xD4\xC4\xB6\xC1", newfh.filename);
				// 权限
				newfh.level = setperms(newfh.level, "\xC8\xA8\xCF\xDE", NUMPERMS, showperminfo, 0);
				clear();
				// 确定要更改吗? (Y/N) [N]:
				getdata(0, 0, "\xC8\xB7\xB6\xA8\xD2\xAA\xB8\xFC\xB8\xC4\xC2\xF0? (Y/N) [N]: ", genbuf, 4, DOECHO, YEA);
			} else {
				// 确定要更改吗? (Y/N) [N]:
				getdata(22, 0, "\xC8\xB7\xB6\xA8\xD2\xAA\xB8\xFC\xB8\xC4\xC2\xF0? (Y/N) [N]: ", genbuf, 4, DOECHO, YEA);
			}
		}
		clear();
		if (*genbuf == 'Y' || *genbuf == 'y') {
			{
				char secu[STRLEN];
				// 修改讨论区：%s(%s)
				sprintf(secu, "\xD0\xDE\xB8\xC4\xCC\xD6\xC2\xDB\xC7\xF8\xA3\xBA%s(%s)", fh.filename,
					newfh.filename);
				securityreport(secu, secu);
			}
			newfh.board_mtime = time(NULL);
			if (strcmp(fh.filename, newfh.filename)) {
				char local_old[256], tar[256];
				a_mv = 1;
				setbpath(local_old, sizeof(local_old), fh.filename);
				setbpath(tar, sizeof(tar), newfh.filename);
				rename(local_old, tar);
				sprintf(local_old, "vote/%s", fh.filename);
				sprintf(tar, "vote/%s", newfh.filename);
				rename(local_old, tar);
				if (seek_in_file("etc/junkboards", fh.filename)) {
					ytht_del_from_file("etc/junkboards", fh.filename, true);
					ytht_add_to_file("etc/junkboards", newfh.filename);
				}
			}
			get_grp(fh.filename);
			anno_title(vbuf, &newfh);
			edit_grp(fh.filename, lookgrp, oldtitle, vbuf);
			if (a_mv >= 1) {
				if ((group = chgrp()) != NULL) {
					get_grp(fh.filename);
					strcpy(tmp_grp, lookgrp);
					if (strcmp(tmp_grp, group) || a_mv != 2) {
						ytht_del_from_file("0Announce/.Search", fh.filename, true);
						if (add_grp(group, cexplain, newfh.filename, vbuf) == -1)
							// \n成立精华区失败....\n
							prints("\n\xB3\xC9\xC1\xA2\xBE\xAB\xBB\xAA\xC7\xF8\xCA\xA7\xB0\xDC....\n");
						else
							// 已经置入精华区...\n
							prints("\xD2\xD1\xBE\xAD\xD6\xC3\xC8\xEB\xBE\xAB\xBB\xAA\xC7\xF8...\n");
						sprintf(newpath, "0Announce/groups/%s/%s", group, newfh.filename);
						sprintf(oldpath, "0Announce/groups/%s/%s", tmp_grp, fh.filename);
						if (dashd(oldpath)) {
							deltree(newpath);
						}
						rename(oldpath, newpath);
						del_grp(tmp_grp, fh.filename, fh.title);
					}
				}
			}
			substitute_record(BOARDS, &newfh, sizeof (newfh), pos);
			ythtbbs_cache_Board_resolve();
			if (bmy_board_is_system_board(newfh.filename) && bmy_board_is_system_board(fh.filename)) {
				// 新旧名称都属于系统版面，忽略不处理
			} else if (bmy_board_is_system_board(fh.filename)) {
				// 否则，如果原先属于系统版面，则添加，暂不导入版面数据
				bmy_board_create(pos, newfh.filename, newfh.title, newfh.sec1);
			} else if (bmy_board_is_system_board(newfh.filename)) {
				// 再或者，现在属于系统版面了，则移除原记录
				bmy_board_delete(pos, fh.filename);
			} else {
				// 最后，对于普通情况，判断是否重命名
				if (strcmp(newfh.filename, fh.filename) != 0 || strcmp(newfh.title, fh.title) != 0 || strcmp(newfh.sec1, fh.sec1) != 0) {
					bmy_board_rename(pos, newfh.filename, newfh.title, newfh.sec1);
				}
			}
		}
	}
	clear();
	return 0;
}

extern int delmsgs[];
extern int delcnt;

int m_register(const char *s) {
	(void) s;
	FILE *fn;
	char ans[3];
	int x;
	char uident[STRLEN];

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();

	// 设定使用者注册资料(请使用新的实名认证管理选单)
	stand_title("\xC9\xE8\xB6\xA8\xCA\xB9\xD3\xC3\xD5\xDF\xD7\xA2\xB2\xE1\xD7\xCA\xC1\xCF(\xC7\xEB\xCA\xB9\xD3\xC3\xD0\xC2\xB5\xC4\xCA\xB5\xC3\xFB\xC8\xCF\xD6\xA4\xB9\xDC\xC0\xED\xD1\xA1\xB5\xA5)");
	for (;;) {
		getdata(1, 0,
			// [0]离开 [1]邮箱绑定操作 [2]查询使用者注册资料 (默认[2]):
			"[0]\xC0\xEB\xBF\xAA [1]\xD3\xCA\xCF\xE4\xB0\xF3\xB6\xA8\xB2\xD9\xD7\xF7 [2]\xB2\xE9\xD1\xAF\xCA\xB9\xD3\xC3\xD5\xDF\xD7\xA2\xB2\xE1\xD7\xCA\xC1\xCF (\xC4\xAC\xC8\xCF[2]):",
			ans, 2, DOECHO, YEA);
		if (ans[0] == '0')
			return 0;
		if (ans[0] == '\n' || ans[0] == '\0')
			ans[0] = '2';

		if (ans[0] == '1' || ans[0] == '2')
			break;

	}
	if (ans[0] == '1') {
		clear();
		move(3, 0);
		// 此功能已经废弃。请使用新的实名认证管理选单!
		prints("\xB4\xCB\xB9\xA6\xC4\xDC\xD2\xD1\xBE\xAD\xB7\xCF\xC6\xFA\xA1\xA3\xC7\xEB\xCA\xB9\xD3\xC3\xD0\xC2\xB5\xC4\xCA\xB5\xC3\xFB\xC8\xCF\xD6\xA4\xB9\xDC\xC0\xED\xD1\xA1\xB5\xA5!");
		pressreturn();
	} else {
		move(1, 0);
		// 请输入要查询的代号:
		usercomplete("\xC7\xEB\xCA\xE4\xC8\xEB\xD2\xAA\xB2\xE9\xD1\xAF\xB5\xC4\xB4\xFA\xBA\xC5: ", uident);
		if (uident[0] != '\0') {
			if (!getuser(uident)) {
				move(2, 0);
				// 错误的使用者代号...
				prints("\xB4\xED\xCE\xF3\xB5\xC4\xCA\xB9\xD3\xC3\xD5\xDF\xB4\xFA\xBA\xC5...");
			} else {
				sprintf(genbuf, "home/%c/%s/register",
					mytoupper(lookupuser.userid[0]),
					lookupuser.userid);
				if ((fn = fopen(genbuf, "r")) != NULL) {
					// \n注册资料如下:\n\n
					prints("\n\xD7\xA2\xB2\xE1\xD7\xCA\xC1\xCF\xC8\xE7\xCF\xC2:\n\n");
					for (x = 1; x <= 15; x++) {
						if (fgets(genbuf, STRLEN, fn))
							prints("%s", genbuf);
						else
							break;
					}
					fclose(fn);
				} else
					// \n\n找不到他/她的注册资料!!\n
					prints("\n\n\xD5\xD2\xB2\xBB\xB5\xBD\xCB\xFB/\xCB\xFD\xB5\xC4\xD7\xA2\xB2\xE1\xD7\xCA\xC1\xCF!!\n");
			}
		}
		pressanykey();
	}
	clear();
	return 0;
}

int m_ordainBM(const char *s) {
	(void) s;
	return do_ordainBM(NULL, NULL);
}

int
do_ordainBM(const char *userid, const char *abname)
{
	int id, pos, oldbm = 0, i, bigbm, bmpos, minpos, maxpos;
	struct boardheader fh;
	char bname[24] /* boardheader.filename */, tmp[STRLEN], buf[5][STRLEN];
	char content[1024], title[STRLEN];
	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();
	// 任命版主\n
	stand_title("\xC8\xCE\xC3\xFC\xB0\xE6\xD6\xF7\n");
	clrtoeol();
	move(2, 0);
	if (userid)
		ytht_strsncpy(genbuf, userid, sizeof(genbuf));
	else
		// 输入欲任命的使用者帐号:
		usercomplete("\xCA\xE4\xC8\xEB\xD3\xFB\xC8\xCE\xC3\xFC\xB5\xC4\xCA\xB9\xD3\xC3\xD5\xDF\xD5\xCA\xBA\xC5: ", genbuf);
	if (genbuf[0] == '\0') {
		clear();
		return 0;
	}
	if (!(id = getuser(genbuf))) {
		move(4, 0);
		// 无效的使用者帐号
		prints("\xCE\xDE\xD0\xA7\xB5\xC4\xCA\xB9\xD3\xC3\xD5\xDF\xD5\xCA\xBA\xC5");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	if (abname)
		ytht_strsncpy(bname, abname, sizeof(bname));
	else {
		make_blist_full();
		// 输入该使用者将管理的讨论区名称:
		namecomplete("\xCA\xE4\xC8\xEB\xB8\xC3\xCA\xB9\xD3\xC3\xD5\xDF\xBD\xAB\xB9\xDC\xC0\xED\xB5\xC4\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB\xB3\xC6: ", bname);
	}
	if (*bname == '\0') {
		move(5, 0);
		// 错误的讨论区名称
		prints("\xB4\xED\xCE\xF3\xB5\xC4\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB\xB3\xC6");
		pressreturn();
		clear();
		return -1;
	}
	pos = new_search_record(BOARDS, &fh, sizeof (fh), (void *) cmpbnames, bname);
	if (!pos) {
		move(5, 0);
		// 错误的讨论区名称
		prints("\xB4\xED\xCE\xF3\xB5\xC4\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB\xB3\xC6");
		pressreturn();
		clear();
		return -1;
	}
	oldbm = getbmnum(lookupuser.userid);
	if (oldbm >= 3 && strcmp(lookupuser.userid, "SYSOP") && normal_board(bname)) {
		move(5, 0);
		//  %s 已经是%d个版的版主了
		prints(" %s \xD2\xD1\xBE\xAD\xCA\xC7%d\xB8\xF6\xB0\xE6\xB5\xC4\xB0\xE6\xD6\xF7\xC1\xCB", lookupuser.userid, oldbm);
		// \n一定要任命么?
		if (askyn("\n\xD2\xBB\xB6\xA8\xD2\xAA\xC8\xCE\xC3\xFC\xC3\xB4? ", NA, NA) == NA){
			pressanykey();
			clear();
			return -1;
		}
	}
	// 任命为大版主么? (可以整理精华区)
	if (askyn("\xC8\xCE\xC3\xFC\xCE\xAA\xB4\xF3\xB0\xE6\xD6\xF7\xC3\xB4? (\xBF\xC9\xD2\xD4\xD5\xFB\xC0\xED\xBE\xAB\xBB\xAA\xC7\xF8)", YEA, NA) == YEA) {
		bigbm = 1;
		minpos = 0;
		maxpos = 3;
	} else {
		bigbm = 0;
		minpos = 4;
		maxpos = BMNUM - 1;
	}
	bmpos = -1;
	for (i = 0; i < BMNUM; i++) {
		if (fh.bm[i][0] == 0 && (i >= minpos) && (i <= maxpos) && (bmpos == -1)) {
			bmpos = i;
		}
		if (!strncmp(fh.bm[i], lookupuser.userid, IDLEN)) {
			//  %s 已经是该版版主
			prints(" %s \xD2\xD1\xBE\xAD\xCA\xC7\xB8\xC3\xB0\xE6\xB0\xE6\xD6\xF7", lookupuser.userid);
			pressanykey();
			clear();
			return -1;
		}
	}
	if (bmpos == -1) {
		//  %s 没有空余版主位置
		prints(" %s \xC3\xBB\xD3\xD0\xBF\xD5\xD3\xE0\xB0\xE6\xD6\xF7\xCE\xBB\xD6\xC3", bname);
		pressanykey();
		clear();
		return -1;
	}
	// \n你将任命 %s 为 %s 版版主.\n
	prints("\n\xC4\xE3\xBD\xAB\xC8\xCE\xC3\xFC %s \xCE\xAA %s \xB0\xE6\xB0\xE6\xD6\xF7.\n", lookupuser.userid, bname);
	// 你确定要任命吗?
	if (askyn("\xC4\xE3\xC8\xB7\xB6\xA8\xD2\xAA\xC8\xCE\xC3\xFC\xC2\xF0?", YEA, NA) == NA) {
		// 取消任命版主
		prints("\xC8\xA1\xCF\xFB\xC8\xCE\xC3\xFC\xB0\xE6\xD6\xF7");
		pressanykey();
		clear();
		return -1;
	}
	for (i = 0; i < 5; i++)
		buf[i][0] = '\0';
	move(12, 0);
	// 请输入任命附言(最多五行，按 Enter 结束)
	prints("\xC7\xEB\xCA\xE4\xC8\xEB\xC8\xCE\xC3\xFC\xB8\xBD\xD1\xD4(\xD7\xEE\xB6\xE0\xCE\xE5\xD0\xD0\xA3\xAC\xB0\xB4 Enter \xBD\xE1\xCA\xF8)");
	for (i = 0; i < 5; i++) {
		getdata(i + 13, 0, ": ", buf[i], STRLEN - 5, DOECHO, YEA);
		if (buf[i][0] == '\0')
			break;
	}

	if (!oldbm) {
		char secu[STRLEN];
		lookupuser.userlevel |= PERM_BOARDS;
		substitute_record(PASSFILE, &lookupuser, sizeof (struct userec), id);
		// 版主任命, 给予 %s 的版主权限
		sprintf(secu, "\xB0\xE6\xD6\xF7\xC8\xCE\xC3\xFC, \xB8\xF8\xD3\xE8 %s \xB5\xC4\xB0\xE6\xD6\xF7\xC8\xA8\xCF\xDE", lookupuser.userid);
		securityreport(secu, secu);
		move(19, 0);
		// 版务操作手册
		mail_file("etc/bmhelp", lookupuser.userid, "\xB0\xE6\xCE\xF1\xB2\xD9\xD7\xF7\xCA\xD6\xB2\xE1");
		// 过刊使用说明
		mail_file("etc/backnumbers", lookupuser.userid, "\xB9\xFD\xBF\xAF\xCA\xB9\xD3\xC3\xCB\xB5\xC3\xF7");
		prints(secu);
	}
	strncpy(fh.bm[bmpos], lookupuser.userid, IDLEN);
	fh.hiretime[bmpos] = time(NULL);
	if (bigbm) {
		anno_title(tmp, &fh);
		get_grp(fh.filename);
		edit_grp(fh.filename, lookgrp, fh.title, tmp);
	}
	substitute_record(BOARDS, &fh, sizeof (fh), pos);
	if (fh.clubnum) {
		char tmpb[30];
		ytht_strsncpy(tmpb, currboard, 30);
		ytht_strsncpy(currboard, fh.filename, 30);
		addclubmember(lookupuser.userid, fh.clubnum);
		ytht_strsncpy(currboard, tmpb, 30);
	}
	ythtbbs_cache_Board_resolve();
	// 任命 %s 为 %s 讨论区版主
	sprintf(genbuf, "\xC8\xCE\xC3\xFC %s \xCE\xAA %s \xCC\xD6\xC2\xDB\xC7\xF8\xB0\xE6\xD6\xF7", lookupuser.userid, fh.filename);
	securityreport(genbuf, genbuf);
	move(19, 0);
	prints("%s", genbuf);
	// [公告]任命%s 版版主 %s
	sprintf(title, "[\xB9\xAB\xB8\xE6]\xC8\xCE\xC3\xFC%s \xB0\xE6\xB0\xE6\xD6\xF7 %s ", bname, lookupuser.userid);
	if(strcmp(bname,"BM_exam")&&strcmp(bname,"BM_examII")&&strcmp(bname,"BM_examIII"))
		sprintf(content,
			// \n\t\t    【 版务任命公告 】\n\n\n
			// \t  %s 网友：\n\n
			"\n\t\t    \xA1\xBE \xB0\xE6\xCE\xF1\xC8\xCE\xC3\xFC\xB9\xAB\xB8\xE6 \xA1\xBF\n\n\n" "\t  %s \xCD\xF8\xD3\xD1\xA3\xBA\n\n"
			// \t      经本站站务组审批、纪律委员会考核通过，\n\t  现正式任命你为 %s 版版务。\n\n
			"\t      \xBE\xAD\xB1\xBE\xD5\xBE\xD5\xBE\xCE\xF1\xD7\xE9\xC9\xF3\xC5\xFA\xA1\xA2\xBC\xCD\xC2\xC9\xCE\xAF\xD4\xB1\xBB\xE1\xBF\xBC\xBA\xCB\xCD\xA8\xB9\xFD\xA3\xAC\n\t  \xCF\xD6\xD5\xFD\xCA\xBD\xC8\xCE\xC3\xFC\xC4\xE3\xCE\xAA %s \xB0\xE6\xB0\xE6\xCE\xF1\xA1\xA3\n\n"
			// \t      请在 3 天之内在 BM_home 版面报到。\n
			"\t      \xC7\xEB\xD4\xDA 3 \xCC\xEC\xD6\xAE\xC4\xDA\xD4\xDA BM_home \xB0\xE6\xC3\xE6\xB1\xA8\xB5\xBD\xA1\xA3\n",
			lookupuser.userid,
			bname);
	else
		sprintf(content,
			// \n\t\t    【 实习版务任命公告 】\n\n\n
			// \t  %s 网友：\n\n
			"\n\t\t    \xA1\xBE \xCA\xB5\xCF\xB0\xB0\xE6\xCE\xF1\xC8\xCE\xC3\xFC\xB9\xAB\xB8\xE6 \xA1\xBF\n\n\n" "\t  %s \xCD\xF8\xD3\xD1\xA3\xBA\n\n"
			// \t      经本站站务组审批、 现任命你为 %s 版实习版务。\n\n
			"\t      \xBE\xAD\xB1\xBE\xD5\xBE\xD5\xBE\xCE\xF1\xD7\xE9\xC9\xF3\xC5\xFA\xA1\xA2 \xCF\xD6\xC8\xCE\xC3\xFC\xC4\xE3\xCE\xAA %s \xB0\xE6\xCA\xB5\xCF\xB0\xB0\xE6\xCE\xF1\xA1\xA3\n\n"
			// \t      请于三天的培训期内熟悉版务考试相关知识，\n\n\t      及时联系纪委参加考试。\n
			"\t      \xC7\xEB\xD3\xDA\xC8\xFD\xCC\xEC\xB5\xC4\xC5\xE0\xD1\xB5\xC6\xDA\xC4\xDA\xCA\xEC\xCF\xA4\xB0\xE6\xCE\xF1\xBF\xBC\xCA\xD4\xCF\xE0\xB9\xD8\xD6\xAA\xCA\xB6\xA3\xAC\n\n\t      \xBC\xB0\xCA\xB1\xC1\xAA\xCF\xB5\xBC\xCD\xCE\xAF\xB2\xCE\xBC\xD3\xBF\xBC\xCA\xD4\xA1\xA3\n",
			lookupuser.userid,
			bname);
	for (i = 0; i < 5; i++) {
		if (buf[i][0] == '\0')
			break;
		if (i == 0)
			// \n\n任命附言：\n
			strcat(content, "\n\n\xC8\xCE\xC3\xFC\xB8\xBD\xD1\xD4\xA3\xBA\n");
		strcat(content, buf[i]);
		strcat(content, "\n");
	}
	strcpy(currboard, bname);
	deliverreport(title, content);
	if (normal_board(bname)) {
		strcpy(currboard, "Board");
		deliverreport(title, content);
	}
	pressanykey();
	return 0;
}

int m_retireBM(const char *s) {
	(void) s;
	return do_retireBM(NULL, NULL);
}

int
do_retireBM(const char *userid, const char *abname)
{
	int id, pos, bmpos, right = 0, oldbm = 0, i;
	int bm = 1;
	struct boardheader fh;
	char buf[5][STRLEN];
	char bname[24] /* boardheader.filename */;
	char content[1024], title[STRLEN];
	char tmp[STRLEN];
	modify_user_mode(ADMIN);
	if (!check_systempasswd())
		return -1;

	clear();
	// 版主离职\n
	stand_title("\xB0\xE6\xD6\xF7\xC0\xEB\xD6\xB0\n");
	clrtoeol();
	if (userid)
		ytht_strsncpy(genbuf, userid, sizeof(genbuf));
	else
		// 输入欲离任的使用者帐号:
		usercomplete("\xCA\xE4\xC8\xEB\xD3\xFB\xC0\xEB\xC8\xCE\xB5\xC4\xCA\xB9\xD3\xC3\xD5\xDF\xD5\xCA\xBA\xC5: ", genbuf);
	if (genbuf[0] == '\0') {
		clear();
		return 0;
	}
	if (!(id = getuser(genbuf))) {
		move(4, 0);
		// 无效的使用者帐号
		prints("\xCE\xDE\xD0\xA7\xB5\xC4\xCA\xB9\xD3\xC3\xD5\xDF\xD5\xCA\xBA\xC5");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	if (abname)
		ytht_strsncpy(bname, abname, sizeof(bname));
	else {
		make_blist_full();
		// 输入该使用者将管理的讨论区名称:
		namecomplete("\xCA\xE4\xC8\xEB\xB8\xC3\xCA\xB9\xD3\xC3\xD5\xDF\xBD\xAB\xB9\xDC\xC0\xED\xB5\xC4\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB\xB3\xC6: ", bname);
	}
	if (*bname == '\0') {
		move(5, 0);
		// 错误的讨论区名称
		prints("\xB4\xED\xCE\xF3\xB5\xC4\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB\xB3\xC6");
		pressreturn();
		clear();
		return -1;
	}
	pos = new_search_record(BOARDS, &fh, sizeof (fh), (void *) cmpbnames, bname);
	if (!pos) {
		move(5, 0);
		// 错误的讨论区名称
		prints("\xB4\xED\xCE\xF3\xB5\xC4\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB\xB3\xC6");
		pressreturn();
		clear();
		return -1;
	}
	bmpos = -1;
	for (i = 0; i < BMNUM; i++) {
		if (!strcasecmp(fh.bm[i], lookupuser.userid)) {
			bmpos = i;
			if (i < 4)
				bm = 1;
			else
				bm = 0;
		}
	}

	oldbm = getbmnum(lookupuser.userid);
	if (bmpos == -1) {
		move(5, 0);
		//  版主名单中没有%s，如有错误，请通知系统维护。
		prints(" \xB0\xE6\xD6\xF7\xC3\xFB\xB5\xA5\xD6\xD0\xC3\xBB\xD3\xD0%s\xA3\xAC\xC8\xE7\xD3\xD0\xB4\xED\xCE\xF3\xA3\xAC\xC7\xEB\xCD\xA8\xD6\xAA\xCF\xB5\xCD\xB3\xCE\xAC\xBB\xA4\xA1\xA3", lookupuser.userid);
		pressanykey();
		clear();
		return -1;
	}
	// \n你将取消 %s 的 %s 版%s版职务.\n
	// 大
	prints("\n\xC4\xE3\xBD\xAB\xC8\xA1\xCF\xFB %s \xB5\xC4 %s \xB0\xE6%s\xB0\xE6\xD6\xB0\xCE\xF1.\n", lookupuser.userid, bname, bm ? "\xB4\xF3" : "");
	// 你确定要取消他的该版版主职务吗?
	if (askyn("\xC4\xE3\xC8\xB7\xB6\xA8\xD2\xAA\xC8\xA1\xCF\xFB\xCB\xFB\xB5\xC4\xB8\xC3\xB0\xE6\xB0\xE6\xD6\xF7\xD6\xB0\xCE\xF1\xC2\xF0?", YEA, NA) == NA) {
		// \n呵呵，你改变心意了？ %s 继续留任 %s 版版主职务！
		prints("\n\xBA\xC7\xBA\xC7\xA3\xAC\xC4\xE3\xB8\xC4\xB1\xE4\xD0\xC4\xD2\xE2\xC1\xCB\xA3\xBF %s \xBC\xCC\xD0\xF8\xC1\xF4\xC8\xCE %s \xB0\xE6\xB0\xE6\xD6\xF7\xD6\xB0\xCE\xF1\xA3\xA1", lookupuser.userid, bname);
		pressanykey();
		clear();
		return -1;
	}
	anno_title(title, &fh);
	fh.bm[bmpos][0] = 0;	//先清理掉, 免的有问题
	fh.hiretime[bmpos] = 0;
	for (i = bmpos; i < (bm ? 4 : BMNUM); i++) {
		if (i == (bm ? 3 : BMNUM - 1)) {	//最后一个BM
			fh.bm[i][0] = 0;
			fh.hiretime[i] = 0;
		} else {
			strcpy(fh.bm[i], fh.bm[i + 1]);
			fh.bm[i][strlen(fh.bm[i + 1])] = 0;
			fh.hiretime[i] = fh.hiretime[i + 1];
		}
	}
	if (bm) {
		anno_title(tmp, &fh);
		get_grp(fh.filename);
		edit_grp(fh.filename, lookgrp, title, tmp);
	}
	substitute_record(BOARDS, &fh, sizeof (fh), pos);
	ythtbbs_cache_Board_resolve();
	// 取消 %s 的 %s 讨论区版主职务
	sprintf(genbuf, "\xC8\xA1\xCF\xFB %s \xB5\xC4 %s \xCC\xD6\xC2\xDB\xC7\xF8\xB0\xE6\xD6\xF7\xD6\xB0\xCE\xF1", lookupuser.userid, fh.filename);
	securityreport(genbuf, genbuf);
	move(8, 0);
	prints("%s", genbuf);
	if (oldbm == 1 || oldbm == 0) {
		char secu[STRLEN];
		if (!(lookupuser.userlevel & PERM_OBOARDS) && !(lookupuser.userlevel & PERM_SYSOP)) {
			lookupuser.userlevel &= ~PERM_BOARDS;
			substitute_record(PASSFILE, &lookupuser, sizeof (struct userec), id);
			// 版主卸职, 取消 %s 的版主权限
			sprintf(secu, "\xB0\xE6\xD6\xF7\xD0\xB6\xD6\xB0, \xC8\xA1\xCF\xFB %s \xB5\xC4\xB0\xE6\xD6\xF7\xC8\xA8\xCF\xDE", lookupuser.userid);
			securityreport(secu, secu);
			move(9, 0);
			prints(secu);
		}
	}
	prints("\n\n");
	// 需要在相关版面发送通告吗?
	if (askyn("\xD0\xE8\xD2\xAA\xD4\xDA\xCF\xE0\xB9\xD8\xB0\xE6\xC3\xE6\xB7\xA2\xCB\xCD\xCD\xA8\xB8\xE6\xC2\xF0?", YEA, NA) == NA) {
		pressanykey();
		return 0;
	}
	prints("\n");
	// 正常离任请按 Enter 键确认，撤职惩罚按 N 键
	if (askyn("\xD5\xFD\xB3\xA3\xC0\xEB\xC8\xCE\xC7\xEB\xB0\xB4 Enter \xBC\xFC\xC8\xB7\xC8\xCF\xA3\xAC\xB3\xB7\xD6\xB0\xB3\xCD\xB7\xA3\xB0\xB4 N \xBC\xFC", YEA, NA) == YEA)
		right = 1;
	else
		right = 0;
	if (right)
		// [公告]%s 版%s %s 离任
		sprintf(title, "[\xB9\xAB\xB8\xE6]%s \xB0\xE6%s %s \xC0\xEB\xC8\xCE", bname,
			// 大版主
			// 版主
			bm ? "\xB4\xF3\xB0\xE6\xD6\xF7" : "\xB0\xE6\xD6\xF7", lookupuser.userid);
	else
		// [公告]撤除 %s 版%s %s
		sprintf(title, "[\xB9\xAB\xB8\xE6]\xB3\xB7\xB3\xFD %s \xB0\xE6%s %s ", bname,
			// 大版主
			// 版主
			bm ? "\xB4\xF3\xB0\xE6\xD6\xF7" : "\xB0\xE6\xD6\xF7", lookupuser.userid);
	strcpy(currboard, bname);
	if (right) {
		// \n\t\t\t【 公告 】\n\n
		sprintf(content, "\n\t\t\t\xA1\xBE \xB9\xAB\xB8\xE6 \xA1\xBF\n\n"
			// \t经站务组讨论：\n
			"\t\xBE\xAD\xD5\xBE\xCE\xF1\xD7\xE9\xCC\xD6\xC2\xDB\xA3\xBA\n"
			// \t同意 %s 辞去 %s 版的%s职务。\n
			"\t\xCD\xAC\xD2\xE2 %s \xB4\xC7\xC8\xA5 %s \xB0\xE6\xB5\xC4%s\xD6\xB0\xCE\xF1\xA1\xA3\n"
			// \t在此，对他曾经在 %s 版的辛苦劳作表示感谢。\n\n
			"\t\xD4\xDA\xB4\xCB\xA3\xAC\xB6\xD4\xCB\xFB\xD4\xF8\xBE\xAD\xD4\xDA %s \xB0\xE6\xB5\xC4\xD0\xC1\xBF\xE0\xC0\xCD\xD7\xF7\xB1\xED\xCA\xBE\xB8\xD0\xD0\xBB\xA1\xA3\n\n"
			// \t希望今后也能支持本版的工作.
			"\t\xCF\xA3\xCD\xFB\xBD\xF1\xBA\xF3\xD2\xB2\xC4\xDC\xD6\xA7\xB3\xD6\xB1\xBE\xB0\xE6\xB5\xC4\xB9\xA4\xD7\xF7.",
			// 大版主
			// 版主
			lookupuser.userid, bname, bm ? "\xB4\xF3\xB0\xE6\xD6\xF7" : "\xB0\xE6\xD6\xF7",
			bname);
	} else {
		// \n\t\t\t【撤职公告】\n\n
		sprintf(content, "\n\t\t\t\xA1\xBE\xB3\xB7\xD6\xB0\xB9\xAB\xB8\xE6\xA1\xBF\n\n"
			// \t经站务组讨论决定：\n
			"\t\xBE\xAD\xD5\xBE\xCE\xF1\xD7\xE9\xCC\xD6\xC2\xDB\xBE\xF6\xB6\xA8\xA3\xBA\n"
			// \t撤除 %s 版%s %s 的%s职务。\n
			"\t\xB3\xB7\xB3\xFD %s \xB0\xE6%s %s \xB5\xC4%s\xD6\xB0\xCE\xF1\xA1\xA3\n",
			// 大版主
			// 版主
			bname, bm ? "\xB4\xF3\xB0\xE6\xD6\xF7" : "\xB0\xE6\xD6\xF7", lookupuser.userid,
			// 大版主
			// 版主
			bm ? "\xB4\xF3\xB0\xE6\xD6\xF7" : "\xB0\xE6\xD6\xF7");
	}
	for (i = 0; i < 5; i++)
		buf[i][0] = '\0';
	move(14, 0);
	// 请输入%s附言(最多五行，按 Enter 结束)
	// 版主离任
	// 版主撤职
	prints("\xC7\xEB\xCA\xE4\xC8\xEB%s\xB8\xBD\xD1\xD4(\xD7\xEE\xB6\xE0\xCE\xE5\xD0\xD0\xA3\xAC\xB0\xB4 Enter \xBD\xE1\xCA\xF8)", right ? "\xB0\xE6\xD6\xF7\xC0\xEB\xC8\xCE" : "\xB0\xE6\xD6\xF7\xB3\xB7\xD6\xB0");
	for (i = 0; i < 5; i++) {
		getdata(i + 15, 0, ": ", buf[i], STRLEN - 5, DOECHO, YEA);
		if (buf[i][0] == '\0')
			break;
		if (i == 0)
			// \n\n离任附言：\n
			// \n\n撤职说明：\n
			strcat(content, right ? "\n\n\xC0\xEB\xC8\xCE\xB8\xBD\xD1\xD4\xA3\xBA\n" : "\n\n\xB3\xB7\xD6\xB0\xCB\xB5\xC3\xF7\xA3\xBA\n");
		strcat(content, buf[i]);
		strcat(content, "\n");
	}
	deliverreport(title, content);
	if (normal_board(currboard)) {
		strcpy(currboard, "Board");
		deliverreport(title, content);
	}
	// \n执行完毕！
	prints("\n\xD6\xB4\xD0\xD0\xCD\xEA\xB1\xCF\xA3\xA1");
	pressanykey();
	return 0;
}

static const char *DIGEST_BASE = "0Announce/groups/GROUP_0/PersonalCorpus";

int m_addpersonal(const char *s) {
	(void) s;
	FILE *fn;
	char digestpath[80];
	char personalpath[80], title[STRLEN];
	char firstchar[1];
	int id;
	modify_user_mode(DIGEST);
	if (!check_systempasswd()) {
		return 1;
	}
	clear();
	if (!dashd(DIGEST_BASE)) {		//add by mintbaggio@BMY
		// 请先建立个人文集讨论区：Personal_Corpus, 路径是:%s
		prints("\xC7\xEB\xCF\xC8\xBD\xA8\xC1\xA2\xB8\xF6\xC8\xCB\xCE\xC4\xBC\xAF\xCC\xD6\xC2\xDB\xC7\xF8\xA3\xBAPersonal_Corpus, \xC2\xB7\xBE\xB6\xCA\xC7:%s", DIGEST_BASE);
		pressanykey();
		return 1;
	}
	// 创建个人文集
	stand_title("\xB4\xB4\xBD\xA8\xB8\xF6\xC8\xCB\xCE\xC4\xBC\xAF");
	clrtoeol();
	move(2, 0);
	// 请输入使用者代号:
	usercomplete("\xC7\xEB\xCA\xE4\xC8\xEB\xCA\xB9\xD3\xC3\xD5\xDF\xB4\xFA\xBA\xC5: ", genbuf);
	if (*genbuf == '\0') {
		clear();
		return 1;
	}
	if (!(id = getuser(genbuf))) {
		// 错误的使用者代号
		prints("\xB4\xED\xCE\xF3\xB5\xC4\xCA\xB9\xD3\xC3\xD5\xDF\xB4\xFA\xBA\xC5");
		clrtoeol();
		pressreturn();
		clear();
		return 1;
	}
	if (!isalpha(lookupuser.userid[0])) {
		// 非英文ID，请输入拼音首字母:
		getdata(3, 0, "\xB7\xC7\xD3\xA2\xCE\xC4ID\xA3\xAC\xC7\xEB\xCA\xE4\xC8\xEB\xC6\xB4\xD2\xF4\xCA\xD7\xD7\xD6\xC4\xB8:", firstchar, 2,
			DOECHO, YEA);
	} else
		firstchar[0] = lookupuser.userid[0];
	printf("%c", firstchar[0]);
	snprintf(personalpath, sizeof(personalpath), "%s/%c", DIGEST_BASE, firstchar[0]);	//add by mintbaggio@BMY
	if (!dashd(personalpath)) {		//add by mintbaggio@BMY
		mkdir(personalpath, 0755);
		snprintf(personalpath, sizeof(personalpath), "%s/%c/.Names", DIGEST_BASE, firstchar[0]);
		if ((fn = fopen(personalpath, "w")) == NULL) {
			return -1;
		}
		fprintf(fn, "#\n");
		fprintf(fn, "# Title=%s\n", firstchar);
		fprintf(fn, "#\n");
		fclose(fn);
		linkto(DIGEST_BASE, firstchar, firstchar);
	}
	sprintf(personalpath, "%s/%c/%s", DIGEST_BASE, toupper(firstchar[0]), lookupuser.userid);
	if (dashd(personalpath)) {
		// 该用户的个人文集目录已存在, 按任意键取消..
		prints("\xB8\xC3\xD3\xC3\xBB\xA7\xB5\xC4\xB8\xF6\xC8\xCB\xCE\xC4\xBC\xAF\xC4\xBF\xC2\xBC\xD2\xD1\xB4\xE6\xD4\xDA, \xB0\xB4\xC8\xCE\xD2\xE2\xBC\xFC\xC8\xA1\xCF\xFB..");
		pressanykey();
		return 1;
	}
	if (lookupuser.stay / 60 / 60 < 24) {
		// 该用户上站时间不够,无法申请个人文集, 按任意键取消..
		prints("\xB8\xC3\xD3\xC3\xBB\xA7\xC9\xCF\xD5\xBE\xCA\xB1\xBC\xE4\xB2\xBB\xB9\xBB,\xCE\xDE\xB7\xA8\xC9\xEA\xC7\xEB\xB8\xF6\xC8\xCB\xCE\xC4\xBC\xAF, \xB0\xB4\xC8\xCE\xD2\xE2\xBC\xFC\xC8\xA1\xCF\xFB..");
		pressanykey();
		return 1;
	}
	move(4, 0);
	// 确定要为该用户创建一个个人文集吗?
	if (askyn("\xC8\xB7\xB6\xA8\xD2\xAA\xCE\xAA\xB8\xC3\xD3\xC3\xBB\xA7\xB4\xB4\xBD\xA8\xD2\xBB\xB8\xF6\xB8\xF6\xC8\xCB\xCE\xC4\xBC\xAF\xC2\xF0?", YEA, NA) == NA) {
		// 你选择取消创建. 按任意键取消...
		prints("\xC4\xE3\xD1\xA1\xD4\xF1\xC8\xA1\xCF\xFB\xB4\xB4\xBD\xA8. \xB0\xB4\xC8\xCE\xD2\xE2\xBC\xFC\xC8\xA1\xCF\xFB...");
		pressanykey();
		return 1;
	}
	mkdir(personalpath, 0755);
	chmod(personalpath, 0755);

	move(7, 0);
	// [直接按 ENTER 键, 则标题缺省为: \033[32m%s文集\033[m]
	prints("[\xD6\xB1\xBD\xD3\xB0\xB4 ENTER \xBC\xFC, \xD4\xF2\xB1\xEA\xCC\xE2\xC8\xB1\xCA\xA1\xCE\xAA: \033[32m%s\xCE\xC4\xBC\xAF\033[m]", lookupuser.userid);
	char tmp_buf[40], tmp_title[62];
	// 请输入个人文集之标题:
	getdata(6, 0, "\xC7\xEB\xCA\xE4\xC8\xEB\xB8\xF6\xC8\xCB\xCE\xC4\xBC\xAF\xD6\xAE\xB1\xEA\xCC\xE2: ", tmp_buf, 39, DOECHO, YEA);
	if (tmp_buf[0] == '\0')
		// %s文集
		snprintf(tmp_title, sizeof(tmp_title), "%s\xCE\xC4\xBC\xAF", lookupuser.userid);
	else
		// %s文集——%s
		snprintf(tmp_title, sizeof(tmp_title), "%s\xCE\xC4\xBC\xAF\xA1\xAA\xA1\xAA%s", lookupuser.userid, tmp_buf);
	snprintf(title, sizeof(title), "%-38.38s(BM: %s _Personal)", tmp_title, lookupuser.userid);
	//by bjgyt sprintf(title, "%-38.38s(BM: %s)", title, lookupuser.userid);
	sprintf(digestpath, "%s/%c", DIGEST_BASE, toupper(firstchar[0]));
	linkto(digestpath, lookupuser.userid, title);
	sprintf(personalpath, "%s/%c/%s/.Names", DIGEST_BASE, toupper(firstchar[0]), lookupuser.userid);
	if ((fn = fopen(personalpath, "w")) == NULL) {
		return -1;
	}
	fprintf(fn, "#\n");
	fprintf(fn, "# Title=%s\n", title);
	fprintf(fn, "#\n");
	fclose(fn);
	if (!(lookupuser.userlevel & PERM_SPECIAL8)) {
		char secu[STRLEN];
		lookupuser.userlevel |= PERM_SPECIAL8;
		substitute_record(PASSFILE, &lookupuser, sizeof (struct userec), id);
		// 创建个人文集, 给予 %s 文集管理权限
		sprintf(secu, "\xB4\xB4\xBD\xA8\xB8\xF6\xC8\xCB\xCE\xC4\xBC\xAF, \xB8\xF8\xD3\xE8 %s \xCE\xC4\xBC\xAF\xB9\xDC\xC0\xED\xC8\xA8\xCF\xDE",
			lookupuser.userid);
		securityreport(secu, secu);
		move(10, 0);
		prints(secu);

	}
	// 已经创建个人文集, 请按任意键继续...
	prints("\xD2\xD1\xBE\xAD\xB4\xB4\xBD\xA8\xB8\xF6\xC8\xCB\xCE\xC4\xBC\xAF, \xC7\xEB\xB0\xB4\xC8\xCE\xD2\xE2\xBC\xFC\xBC\xCC\xD0\xF8...");
	pressanykey();
	return 0;
}

