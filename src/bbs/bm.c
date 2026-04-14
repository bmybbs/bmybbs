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
#include "stuff.h"
#include "talk.h"
#include "namecomplete.h"
#include "bcache.h"
#include "bbsinc.h"
#include "more.h"
#include "maintain.h"
#include "mail.h"
#include "bbs_global_vars.h"

#define DENY 1
#define UNDENY 2
#define CHANGEDENY 3

extern int millionairesrec(char *title, char *str, char *owner);
static int addtodeny(char *uident, char *msg, int ischange, int isglobal, int isanony);
static int deldeny(char *uident, int isglobal, int isanony);
static int delclubmember(char *uident);
static int deny_notice(int action, char *user, int isglobal, int isanony, char *msgbuf);

static int addtodeny(char *uident, char *msg, int ischange, int isglobal, int isanony)
{
	char buf[50], strtosave[256];
	char buf2[50];
	int day;
	time_t nowtime;
	char ans[8];
	int seek;

	if (isglobal)
		strcpy(genbuf, "deny_users");
	else if (isanony)
		setbfile(genbuf, sizeof(genbuf), currboard, "deny_anony");
	else
		setbfile(genbuf, sizeof(genbuf), currboard, "deny_users");
	seek = seek_in_file(genbuf, uident);
	if ((ischange && !seek) || (!ischange && seek)) {
		move(2, 0);
		// 输入的ID不对!
		prints_nofmt("\xCA\xE4\xC8\xEB\xB5\xC4" "ID" "\xB2\xBB\xB6\xD4" "!");
		pressreturn();
		return -1;
	}
	buf[0] = 0;
	move(2, 0);
	// 封禁对象：%s
	prints("\xB7\xE2\xBD\xFB\xB6\xD4\xCF\xF3\xA3\xBA" "%s", (isanony) ? "Anonymous" : uident);
	while (strlen(buf) < 4)
		// 输入说明(至少两字):
		getdata(3, 0, "\xCA\xE4\xC8\xEB\xCB\xB5\xC3\xF7" "(" "\xD6\xC1\xC9\xD9\xC1\xBD\xD7\xD6" "): ", buf, 40, DOECHO, YEA);

	do {
		// 输入天数(0-手动解封):
		getdata(4, 0, "\xCA\xE4\xC8\xEB\xCC\xEC\xCA\xFD" "(0-" "\xCA\xD6\xB6\xAF\xBD\xE2\xB7\xE2" "): ", buf2, 4, DOECHO, YEA);
		day = atoi(buf2);
		if (day < 0)
			continue;
		if (!(currentuser.userlevel & PERM_SYSOP) && (!day || day > 20)) {
			move(4, 0);
			// 超过权限,若需要,请联系站长!
			prints_nofmt("\xB3\xAC\xB9\xFD\xC8\xA8\xCF\xDE" "," "\xC8\xF4\xD0\xE8\xD2\xAA" "," "\xC7\xEB\xC1\xAA\xCF\xB5\xD5\xBE\xB3\xA4" "!");
			pressreturn();
		} else
			break;
	} while (1);
	//add by lepton for deny bm's right

	nowtime = time(NULL);
	if (day) {
		struct tm *tmtime;
		//time_t daytime = nowtime + (day + 1) * 24 * 60 * 60;
		time_t undenytime = nowtime + day * 24 * 60 * 60;
		//tmtime = gmtime(&daytime); by bjgyt
		tmtime = gmtime(&undenytime);
		// %-12s %-40s %2d月%2d日解 \x1b[%ldm
		sprintf(strtosave, "%-12s %-40s %2d" "\xD4\xC2" "%2d" "\xC8\xD5\xBD\xE2" " \x1b[%ldm", uident,
			buf, tmtime->tm_mon + 1, tmtime->tm_mday,
			(long int) undenytime);
		//modified by pzhg
		if (currentuser.userlevel & PERM_SPECIAL5) {
			if (isglobal)
				sprintf(msg,
						// 封人原因: %s\n被封天数: %d%s\n解封日期: %d月%d日\n
						"\xB7\xE2\xC8\xCB\xD4\xAD\xD2\xF2" ": %s\n" "\xB1\xBB\xB7\xE2\xCC\xEC\xCA\xFD" ": %d%s\n" "\xBD\xE2\xB7\xE2\xC8\xD5\xC6\xDA" ": %d" "\xD4\xC2" "%d" "\xC8\xD5" "\n"
						// 本站不接受对此封禁的虚拟投诉\n
						"\xB1\xBE\xD5\xBE\xB2\xBB\xBD\xD3\xCA\xDC\xB6\xD4\xB4\xCB\xB7\xE2\xBD\xFB\xB5\xC4\xD0\xE9\xC4\xE2\xCD\xB6\xCB\xDF" "\n", buf, day,
						// (全站)
						"(" "\xC8\xAB\xD5\xBE" ")" , tmtime->tm_mon + 1,
						tmtime->tm_mday);
			else
				sprintf(msg,
						// 封人原因: %s\n被封天数: %d%s\n解封日期: %d月%d日\n
						"\xB7\xE2\xC8\xCB\xD4\xAD\xD2\xF2" ": %s\n" "\xB1\xBB\xB7\xE2\xCC\xEC\xCA\xFD" ": %d%s\n" "\xBD\xE2\xB7\xE2\xC8\xD5\xC6\xDA" ": %d" "\xD4\xC2" "%d" "\xC8\xD5" "\n"
						// 本站不接受对此封禁的虚拟投诉\n
						"\xB1\xBE\xD5\xBE\xB2\xBB\xBD\xD3\xCA\xDC\xB6\xD4\xB4\xCB\xB7\xE2\xBD\xFB\xB5\xC4\xD0\xE9\xC4\xE2\xCD\xB6\xCB\xDF" "\n", buf, day,
						// (全站)
						isglobal ? "(" "\xC8\xAB\xD5\xBE" ")" : "", tmtime->tm_mon + 1,
						tmtime->tm_mday);
		}
		else if(isglobal || day>20)
		sprintf(msg,
				// 封人原因: %s\n被封天数: %d%s\n解封日期: %d月%d日\n
				"\xB7\xE2\xC8\xCB\xD4\xAD\xD2\xF2" ": %s\n" "\xB1\xBB\xB7\xE2\xCC\xEC\xCA\xFD" ": %d%s\n" "\xBD\xE2\xB7\xE2\xC8\xD5\xC6\xDA" ": %d" "\xD4\xC2" "%d" "\xC8\xD5" "\n"
				// 如有异议，可向%s提出，或到 committee 版投诉\n
				"\xC8\xE7\xD3\xD0\xD2\xEC\xD2\xE9\xA3\xAC\xBF\xC9\xCF\xF2" "%s" "\xCC\xE1\xB3\xF6\xA3\xAC\xBB\xF2\xB5\xBD" " committee " "\xB0\xE6\xCD\xB6\xCB\xDF" "\n", buf, day,
				// (全站)
				"(" "\xC8\xAB\xD5\xBE" ")" , tmtime->tm_mon + 1,
				// 站务
				tmtime->tm_mday, "\xD5\xBE\xCE\xF1");
		else {
			if (seek_in_file(MY_BBS_HOME"/etc/sysboards",currboard)) {
				sprintf(msg,
						// 封人原因: %s\n被封天数: %d%s\n解封日期: %d月%d日\n
						"\xB7\xE2\xC8\xCB\xD4\xAD\xD2\xF2" ": %s\n" "\xB1\xBB\xB7\xE2\xCC\xEC\xCA\xFD" ": %d%s\n" "\xBD\xE2\xB7\xE2\xC8\xD5\xC6\xDA" ": %d" "\xD4\xC2" "%d" "\xC8\xD5" "\n"
						// 如有异议，可向%s提出，或到 committee 版投诉\n
						"\xC8\xE7\xD3\xD0\xD2\xEC\xD2\xE9\xA3\xAC\xBF\xC9\xCF\xF2" "%s" "\xCC\xE1\xB3\xF6\xA3\xAC\xBB\xF2\xB5\xBD" " committee " "\xB0\xE6\xCD\xB6\xCB\xDF" "\n", buf, day,
						// (全站)
						isglobal ? "(" "\xC8\xAB\xD5\xBE" ")" : "", tmtime->tm_mon + 1,
						// 站务
						tmtime->tm_mday, "\xD5\xBE\xCE\xF1" );
			}
			else {
				sprintf(msg,
					// 封人原因: %s\n被封天数: %d%s\n解封日期: %d月%d日\n
					"\xB7\xE2\xC8\xCB\xD4\xAD\xD2\xF2" ": %s\n" "\xB1\xBB\xB7\xE2\xCC\xEC\xCA\xFD" ": %d%s\n" "\xBD\xE2\xB7\xE2\xC8\xD5\xC6\xDA" ": %d" "\xD4\xC2" "%d" "\xC8\xD5" "\n"
					// 如有异议，可向%s提出，或到 committee 版投诉\n
					"\xC8\xE7\xD3\xD0\xD2\xEC\xD2\xE9\xA3\xAC\xBF\xC9\xCF\xF2" "%s" "\xCC\xE1\xB3\xF6\xA3\xAC\xBB\xF2\xB5\xBD" " committee " "\xB0\xE6\xCD\xB6\xCB\xDF" "\n", buf, day,	// 原来是去 Appeal
					// (全站)
					isglobal ? "(" "\xC8\xAB\xD5\xBE" ")" : "", tmtime->tm_mon + 1,
					// 站务
					// 版主
					tmtime->tm_mday, isglobal ? "\xD5\xBE\xCE\xF1" : "\xB0\xE6\xD6\xF7");
			}
		}
	} else {
		if (currentuser.userlevel & PERM_SPECIAL5) {
			// %-12s %-35s 手动解封
			sprintf(strtosave, "%-12s %-35s " "\xCA\xD6\xB6\xAF\xBD\xE2\xB7\xE2", uident, buf);
			// 封人原因: %s\n被封天数: 手动解封%s\n
			sprintf(msg, "\xB7\xE2\xC8\xCB\xD4\xAD\xD2\xF2" ": %s\n" "\xB1\xBB\xB7\xE2\xCC\xEC\xCA\xFD" ": " "\xCA\xD6\xB6\xAF\xBD\xE2\xB7\xE2" "%s\n"
				// 本站不接受对此封禁的虚拟投诉\n
				"\xB1\xBE\xD5\xBE\xB2\xBB\xBD\xD3\xCA\xDC\xB6\xD4\xB4\xCB\xB7\xE2\xBD\xFB\xB5\xC4\xD0\xE9\xC4\xE2\xCD\xB6\xCB\xDF" "\n",
				// (全站)
				buf, isglobal ? "(" "\xC8\xAB\xD5\xBE" ")" : "");
		}
		else {
			// %-12s %-35s 手动解封
			sprintf(strtosave, "%-12s %-35s " "\xCA\xD6\xB6\xAF\xBD\xE2\xB7\xE2", uident, buf);
			// 封人原因: %s\n被封天数: 手动解封%s\n
			sprintf(msg, "\xB7\xE2\xC8\xCB\xD4\xAD\xD2\xF2" ": %s\n" "\xB1\xBB\xB7\xE2\xCC\xEC\xCA\xFD" ": " "\xCA\xD6\xB6\xAF\xBD\xE2\xB7\xE2" "%s\n"
				// 如有异议，可向%s提出，或到 committee 版投诉\n
				"\xC8\xE7\xD3\xD0\xD2\xEC\xD2\xE9\xA3\xAC\xBF\xC9\xCF\xF2" "%s" "\xCC\xE1\xB3\xF6\xA3\xAC\xBB\xF2\xB5\xBD" " committee " "\xB0\xE6\xCD\xB6\xCB\xDF" "\n",
				// (全站)
				buf, isglobal ? "(" "\xC8\xAB\xD5\xBE" ")" : "",
				// 站务
				// 版主
				isglobal ? "\xD5\xBE\xCE\xF1" : "\xB0\xE6\xD6\xF7");
		}
	}
	if (ischange)
		// 真的要改变么?[Y/N]:
		getdata(5, 0, "\xD5\xE6\xB5\xC4\xD2\xAA\xB8\xC4\xB1\xE4\xC3\xB4" "?[Y/N]: ", ans, 7, DOECHO, YEA);
	else
		// 真的要封么?[Y/N]:
		getdata(5, 0, "\xD5\xE6\xB5\xC4\xD2\xAA\xB7\xE2\xC3\xB4" "?[Y/N]: ", ans, 7, DOECHO, YEA);
	if ((*ans != 'Y') && (*ans != 'y'))
		return -1;
	if (ischange)
		deldeny(uident, isglobal, 0);
	if (isglobal)
		strcpy(genbuf, "deny_users");
	else if (isanony)
		setbfile(genbuf, sizeof(genbuf), currboard, "deny_anony");
	else
		setbfile(genbuf, sizeof(genbuf), currboard, "deny_users");
	return ytht_add_to_file(genbuf, strtosave);
}

static int deldeny(char *uident, int isglobal, int isanony)
{
	char fn[STRLEN];

	if (isglobal)
		strcpy(fn, "deny_users");
	else if (isanony)
		setbfile(fn, sizeof(fn), currboard, "deny_anony");
	else
		setbfile(fn, sizeof(fn), currboard, "deny_users");
	return ytht_del_from_file(fn, uident, true);
}

int
deny_user()
{
	char uident[STRLEN];
	char ans[8];
	char msgbuf[256];
	int count, isglobal = 0;

	if (!IScurrBM) {
		return DONOTHING;
	}

	if (!strcmp(currboard, "sysop")) {
		getdata(0, 0,
			// 设定哪个无法 Post 的名单？(A) sysop版 (B) 系统 (E)离开 [E]:
			"\xC9\xE8\xB6\xA8\xC4\xC4\xB8\xF6\xCE\xDE\xB7\xA8" " Post " "\xB5\xC4\xC3\xFB\xB5\xA5\xA3\xBF" "(A) sysop" "\xB0\xE6" " (B) " "\xCF\xB5\xCD\xB3" " (E)" "\xC0\xEB\xBF\xAA" " [E]:",
			ans, 7, DOECHO, YEA);
		if (!strchr("AaBb", *ans))
			return FULLUPDATE;
		if (*ans == 'B' || *ans == 'b')
			isglobal = 1;
	}
	if (isglobal)
		strcpy(genbuf, "deny_users");
	else
		setbfile(genbuf, sizeof(genbuf), currboard, "deny_users");
//      ansimore(genbuf, YEA);
	while (1) {
		clear();
		// 设定无法 Post 的名单\n
		prints_nofmt("\xC9\xE8\xB6\xA8\xCE\xDE\xB7\xA8" " Post " "\xB5\xC4\xC3\xFB\xB5\xA5" "\n");
		if (isglobal)
			strcpy(genbuf, "deny_users");
		else
			setbfile(genbuf, sizeof(genbuf), currboard, "deny_users");
		count = listfilecontent(genbuf);
		if (count)
			getdata(1, 0,
				// (A)增加 (D)删除 (C)改变 or (E)离开 [E]:
				"(A)" "\xD4\xF6\xBC\xD3" " (D)" "\xC9\xBE\xB3\xFD" " (C)" "\xB8\xC4\xB1\xE4" " or (E)" "\xC0\xEB\xBF\xAA" " [E]: ", ans,
				7, DOECHO, YEA);
		else
			// (A)增加 or (E)离开 [E]:
			getdata(1, 0, "(A)" "\xD4\xF6\xBC\xD3" " or (E)" "\xC0\xEB\xBF\xAA" " [E]: ", ans, 7,
				DOECHO, YEA);
		if (*ans == 'A' || *ans == 'a') {
			move(1, 0);
			if (isglobal)
				// 增加无法 POST 的使用者:
				usercomplete("\xD4\xF6\xBC\xD3\xCE\xDE\xB7\xA8" " POST " "\xB5\xC4\xCA\xB9\xD3\xC3\xD5\xDF" ": ", uident);
			else {
				int canpost = 0;
				while (!canpost) {
					move(1, 0);
					clrtoeol();
					// 增加无法 POST 的使用者：
					usercomplete("\xD4\xF6\xBC\xD3\xCE\xDE\xB7\xA8" " POST " "\xB5\xC4\xCA\xB9\xD3\xC3\xD5\xDF\xA3\xBA", uident);
					if (*uident == '\0')
						break;
					canpost = posttest(uident, currboard);
				}
			}
			if (*uident != '\0') {
				if (addtodeny(uident, msgbuf, 0, isglobal, 0) == 1) {
					deny_notice(DENY, uident, isglobal, 0, msgbuf);
					sprintf(genbuf, "%s deny %s %s",
						currentuser.userid, currboard,
						uident);
					newtrace(genbuf);

				}
			}
		} else if ((*ans == 'C' || *ans == 'c')) {
			move(1, 0);
			// 改变谁的封禁时间或说明:
			usercomplete("\xB8\xC4\xB1\xE4\xCB\xAD\xB5\xC4\xB7\xE2\xBD\xFB\xCA\xB1\xBC\xE4\xBB\xF2\xCB\xB5\xC3\xF7" ": ", uident);
			if (*uident != '\0') {
				if (addtodeny(uident, msgbuf, 1, isglobal, 0) == 1) {
					deny_notice(CHANGEDENY, uident, isglobal, 0, msgbuf);
				}
			}
		} else if ((*ans == 'D' || *ans == 'd') && count) {
			move(1, 0);
			// 删除无法 POST 的使用者:
			namecomplete("\xC9\xBE\xB3\xFD\xCE\xDE\xB7\xA8" " POST " "\xB5\xC4\xCA\xB9\xD3\xC3\xD5\xDF" ": ", uident);
			move(1, 0);
			clrtoeol();
			if (uident[0] != '\0') {
				if (deldeny(uident, isglobal, 0)) {
					deny_notice(UNDENY, uident, isglobal, 0, msgbuf);
				}
			}
		} else
			break;
	}

	clear();
	return FULLUPDATE;
}

int addclubmember(char *uident, int clubnum)
{
	char genbuf1[80], genbuf2[80];
	int id;
	int i;
	char ans[8];
	int seek;

	if (clubnum == 0) {
		if (!(id = getuser(uident))) {
			move(3, 0);
			prints_nofmt("Invalid User Id");
			clrtoeol();
			pressreturn();
			clear();
			return 0;
		}
		setbfile(genbuf, sizeof(genbuf), currboard, "club_users");
		seek = seek_in_file(genbuf, uident);
		if (seek) {
			move(2, 0);
			// 输入的ID 已经存在!
			prints_nofmt("\xCA\xE4\xC8\xEB\xB5\xC4" "ID " "\xD2\xD1\xBE\xAD\xB4\xE6\xD4\xDA" "!");
			pressreturn();
			return -1;
		}

		// 真的要添加么?[Y/N]:
		getdata(4, 0, "\xD5\xE6\xB5\xC4\xD2\xAA\xCC\xED\xBC\xD3\xC3\xB4" "?[Y/N]: ", ans, 7, DOECHO, YEA);
		if ((*ans != 'Y') && (*ans != 'y'))
			return -1;
		setbfile(genbuf, sizeof(genbuf), currboard, "club_users");
		sethomefile_s(genbuf1, sizeof(genbuf1), uident, "clubrights");
		if ((i = getbnum(currboard)) == 0)
			return DONOTHING;
		sprintf(genbuf2, "%d", ythtbbs_cache_Board_get_board_by_idx(i - 1)->header.clubnum);
		ytht_add_to_file(genbuf1, genbuf2);
		return ytht_add_to_file(genbuf, uident);
	} else {
		setbfile(genbuf, sizeof(genbuf), currboard, "club_users");
		seek = seek_in_file(genbuf, uident);
		if (seek)
			return DONOTHING;
		sethomefile_s(genbuf1, sizeof(genbuf1), uident, "clubrights");
		sprintf(genbuf2, "%d", clubnum);
		ytht_add_to_file(genbuf1, genbuf2);
		return ytht_add_to_file(genbuf, uident);
	}
}

static int delclubmember(char *uident)
{
	char genbuf1[80], genbuf2[80];
	char fn[STRLEN];
	int id;
	int i;
	if (!(id = getuser(uident))) {
		move(3, 0);
		prints_nofmt("Invalid User Id");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	if ((i = getbnum(currboard)) == 0)
		return DONOTHING;
	setbfile(fn, sizeof(fn), currboard, "club_users");
	sethomefile_s(genbuf1, sizeof(genbuf1), uident, "clubrights");
	sprintf(genbuf2, "%d", ythtbbs_cache_Board_get_board_by_idx(i - 1)->header.clubnum);
	ytht_del_from_file(genbuf1, genbuf2, true);
	return ytht_del_from_file(fn, uident, true);
}

int
clubmember(int ent, void *record, char *direct)
{
	(void) ent;
	(void) record;
	(void) direct;

	char uident[IDLEN + 1];
	char ans[8], repbuf[STRLEN * 2], buf[STRLEN], titlebuf[STRLEN];
	int count, i;

	if (!(IScurrBM)) {
		return DONOTHING;
	}
	if ((i = getbnum(currboard)) == 0)
		return DONOTHING;
	if (ythtbbs_cache_Board_get_board_by_idx(i - 1)->header.clubnum == 0)
		return DONOTHING;
	setbfile(genbuf, sizeof(genbuf), currboard, "club_users");
	ansimore(genbuf, YEA);
	while (1) {
		clear();
		// 设定俱乐部名单\n
		prints_nofmt("\xC9\xE8\xB6\xA8\xBE\xE3\xC0\xD6\xB2\xBF\xC3\xFB\xB5\xA5" "\n");
		setbfile(genbuf, sizeof(genbuf), currboard, "club_users");
		count = listfilecontent(genbuf);
		if (count)
			getdata(1, 0,
				// (A)增加 (D)删除or (E)离开or (M)写信给所有成员 [E]:
				"(A)" "\xD4\xF6\xBC\xD3" " (D)" "\xC9\xBE\xB3\xFD" "or (E)" "\xC0\xEB\xBF\xAA" "or (M)" "\xD0\xB4\xD0\xC5\xB8\xF8\xCB\xF9\xD3\xD0\xB3\xC9\xD4\xB1" " [E]: ",
				ans, 7, DOECHO, YEA);
		else
			// (A)增加 or (E)离开 [E]:
			getdata(1, 0, "(A)" "\xD4\xF6\xBC\xD3" " or (E)" "\xC0\xEB\xBF\xAA" " [E]: ", ans, 7,
				DOECHO, YEA);
		if (*ans == 'A' || *ans == 'a') {
			move(1, 0);
			// 增加俱乐部成员:
			usercomplete("\xD4\xF6\xBC\xD3\xBE\xE3\xC0\xD6\xB2\xBF\xB3\xC9\xD4\xB1" ": ", uident);
			if (*uident != '\0') {
				if (addclubmember(uident, 0) == 1) {
					// 加入原因：
					getdata(5, 0, "\xBC\xD3\xC8\xEB\xD4\xAD\xD2\xF2\xA3\xBA", buf, 50, DOECHO, YEA);
					sprintf(titlebuf,
						// %s由%s授予%s俱乐部权利
						"%s" "\xD3\xC9" "%s" "\xCA\xDA\xD3\xE8" "%s" "\xBE\xE3\xC0\xD6\xB2\xBF\xC8\xA8\xC0\xFB",
						uident, currentuser.userid,
						currboard);
					sprintf(repbuf,
						// \n\n原因：
						"%s%s%s", titlebuf, buf[0] ? "\n\n" "\xD4\xAD\xD2\xF2\xA3\xBA":"", buf);
					if(!strcmp(currboard, "Beggar")
							|| !strcmp(currboard, "killer")
							|| !strcmp(currboard, "Rober")
							|| !strcmp(currboard, "Police")) {
						// 成员变更
						millionairesrec(titlebuf, buf, "\xB3\xC9\xD4\xB1\xB1\xE4\xB8\xFC");
					} else {
						securityreport(titlebuf, buf);
					}

					deliverreport(titlebuf, repbuf);
					mail_buf(repbuf, uident, titlebuf);
				}
			}
		} else if ((*ans == 'D' || *ans == 'd') && count) {
			move(1, 0);
			// 删除俱乐部使用者:
			namecomplete("\xC9\xBE\xB3\xFD\xBE\xE3\xC0\xD6\xB2\xBF\xCA\xB9\xD3\xC3\xD5\xDF" ": ", uident);
			move(1, 0);
			clrtoeol();
			if (uident[0] != '\0') {
				// 真的要取消%s的俱乐部权利么？
				sprintf(genbuf, "\xD5\xE6\xB5\xC4\xD2\xAA\xC8\xA1\xCF\xFB" "%s" "\xB5\xC4\xBE\xE3\xC0\xD6\xB2\xBF\xC8\xA8\xC0\xFB\xC3\xB4\xA3\xBF",
					uident);
				if (askyn(genbuf, YEA, NA))
					if (delclubmember(uident)) {
						// 删除原因：
						getdata(5, 0, "\xC9\xBE\xB3\xFD\xD4\xAD\xD2\xF2\xA3\xBA", buf, 50, DOECHO, YEA);
						snprintf(titlebuf, sizeof(titlebuf),
							// %s被%s取消%s俱乐部权利
							"%s" "\xB1\xBB" "%s" "\xC8\xA1\xCF\xFB" "%s" "\xBE\xE3\xC0\xD6\xB2\xBF\xC8\xA8\xC0\xFB",
							uident, currentuser.userid, currboard);
						snprintf(repbuf, sizeof(repbuf),
							// \n\n原因：
							"%s%s%s", titlebuf, buf[0] ? "\n\n" "\xD4\xAD\xD2\xF2\xA3\xBA":"", buf);
						if(!strcmp(currboard, "Beggar")
								|| !strcmp(currboard, "killer")
								|| !strcmp(currboard, "Rober")
								|| !strcmp(currboard, "Police")) {
							// 成员变更
							millionairesrec(titlebuf, buf, "\xB3\xC9\xD4\xB1\xB1\xE4\xB8\xFC");
						} else {
							securityreport(titlebuf, buf);
						}

						deliverreport(titlebuf, repbuf);
						mail_buf(repbuf, uident, titlebuf);
					}
			}
		} else if ((*ans == 'M' || *ans == 'm') && count) {
			club_send();
		} else
			break;
	}
	clear();
	return FULLUPDATE;
}

int deny_from_article(int ent, struct fileheader *fileinfo, char *direct)
{
	(void) ent;
	(void) direct;
	char msgbuf[512];
	char user[STRLEN];
	char ans[8];
	int seek, canpost, isanony;
	if (!IScurrBM) {
		return DONOTHING;
	}
	if (!strcmp(fh2owner(fileinfo), "Anonymous")) {	/* 对匿名文章 */
		isanony = 1;
		setbfile(genbuf, sizeof(genbuf), currboard, "deny_anony");
		ytht_strsncpy(user, fh2realauthor(fileinfo), sizeof user);
	} else {
		isanony = 0;
		setbfile(genbuf, sizeof(genbuf), currboard, "deny_users");
		ytht_strsncpy(user, fileinfo->owner, sizeof user);
	}
	seek = seek_in_file(genbuf, user);
	if (seek) {		/* 解封 */
		move(2, 0);
		// 真的要解封么?[Y/N]:
		getdata(4, 0, "\xD5\xE6\xB5\xC4\xD2\xAA\xBD\xE2\xB7\xE2\xC3\xB4" "?[Y/N]: ", ans, 7, DOECHO, YEA);
		if ((*ans != 'Y') && (*ans != 'y'))
			return -1;
		if (deldeny(user, 0, isanony) == 1)
			deny_notice(UNDENY, user, 0, isanony, msgbuf);

	} else {		/* 匿名封禁 */
		canpost = posttest(user, currboard);
		if ((canpost) && (addtodeny(user, msgbuf, 0, 0, isanony) == 1)) {
			deny_notice(DENY, user, 0, isanony, msgbuf);
			if(isanony) {
				char fname[STRLEN];
				strncpy(quote_user, user, STRLEN);
				setbfile(fname, sizeof(fname), currboard, fh2fname(fileinfo));
				postfile(fname, "AnonyLog", fileinfo->title, 0);
			}
		}
	}
	return 0;
}

static int deny_notice(int action, char *user, int isglobal, int isanony, char *msgbuf)
{
	char repbuf[STRLEN];
	char tmpbuf[STRLEN], tmpbuf2[STRLEN];
	int i;
	char repuser[IDLEN + 1];
	if (isanony)
		strcpy(repuser, "Anonymous");
	else
		strcpy(repuser, user);
	switch (action) {
	case DENY:
		if (isglobal) {
			sprintf(repbuf,
				// %s被%s取消在全站的POST权利
				"%s" "\xB1\xBB" "%s" "\xC8\xA1\xCF\xFB\xD4\xDA\xC8\xAB\xD5\xBE\xB5\xC4" "POST" "\xC8\xA8\xC0\xFB",
				user, currentuser.userid);
			securityreport(repbuf, msgbuf);
			deliverreport(repbuf, msgbuf);
			mail_buf(msgbuf, user, repbuf);
		} else if (((currentuser.userlevel & PERM_SYSOP) || (currentuser.userlevel & PERM_OBOARDS)) && (msgbuf[10] == 'W' || msgbuf[10] == 'w')) {
			for (i = 10; msgbuf[i]; i++)
				if (msgbuf[i + 1] == '\n')
					msgbuf[i + 1] = 0;
			strcpy(tmpbuf, msgbuf + 11);
			strcpy(tmpbuf2, msgbuf + i + 1);
			sprintf(repbuf,
				// %s被站务%s暂时限制在%s的POST权利
				"%s" "\xB1\xBB\xD5\xBE\xCE\xF1" "%s" "\xD4\xDD\xCA\xB1\xCF\xDE\xD6\xC6\xD4\xDA" "%s" "\xB5\xC4" "POST" "\xC8\xA8\xC0\xFB",
				repuser, currentuser.userid, currboard);
			sprintf(msgbuf,
				// 站务%s认为%s有%s嫌疑.请本版版主/副\n
				"\xD5\xBE\xCE\xF1" "%s" "\xC8\xCF\xCE\xAA" "%s" "\xD3\xD0" "%s" "\xCF\xD3\xD2\xC9" "." "\xC7\xEB\xB1\xBE\xB0\xE6\xB0\xE6\xD6\xF7" "/" "\xB8\xB1" "\n"
				// 及时对%s的行为按本版管理标准进行确认.\n
				"\xBC\xB0\xCA\xB1\xB6\xD4" "%s" "\xB5\xC4\xD0\xD0\xCE\xAA\xB0\xB4\xB1\xBE\xB0\xE6\xB9\xDC\xC0\xED\xB1\xEA\xD7\xBC\xBD\xF8\xD0\xD0\xC8\xB7\xC8\xCF" ".\n"
				// 恢复POST权或者给出正确封禁期限.\n\n%s
				"\xBB\xD6\xB8\xB4" "POST" "\xC8\xA8\xBB\xF2\xD5\xDF\xB8\xF8\xB3\xF6\xD5\xFD\xC8\xB7\xB7\xE2\xBD\xFB\xC6\xDA\xCF\xDE" ".\n\n%s",
				currentuser.userid, repuser, tmpbuf, repuser,
				tmpbuf2);
			securityreport(repbuf, msgbuf);
			deliverreport(repbuf, msgbuf);
			sprintf(repbuf,
				// %s被站务%s暂时限制在%s的POST权利
				"%s" "\xB1\xBB\xD5\xBE\xCE\xF1" "%s" "\xD4\xDD\xCA\xB1\xCF\xDE\xD6\xC6\xD4\xDA" "%s" "\xB5\xC4" "POST" "\xC8\xA8\xC0\xFB",
				user, currentuser.userid, currboard);
			sprintf(msgbuf,
				// 站务%s认为%s有%s嫌疑.请本版版主/副\n
				"\xD5\xBE\xCE\xF1" "%s" "\xC8\xCF\xCE\xAA" "%s" "\xD3\xD0" "%s" "\xCF\xD3\xD2\xC9" "." "\xC7\xEB\xB1\xBE\xB0\xE6\xB0\xE6\xD6\xF7" "/" "\xB8\xB1" "\n"
				// 及时对%s的行为按本版管理标准进行确认.\n
				"\xBC\xB0\xCA\xB1\xB6\xD4" "%s" "\xB5\xC4\xD0\xD0\xCE\xAA\xB0\xB4\xB1\xBE\xB0\xE6\xB9\xDC\xC0\xED\xB1\xEA\xD7\xBC\xBD\xF8\xD0\xD0\xC8\xB7\xC8\xCF" ".\n"
				// 恢复POST权或者给出正确封禁期限.\n\n%s
				"\xBB\xD6\xB8\xB4" "POST" "\xC8\xA8\xBB\xF2\xD5\xDF\xB8\xF8\xB3\xF6\xD5\xFD\xC8\xB7\xB7\xE2\xBD\xFB\xC6\xDA\xCF\xDE" ".\n\n%s",
				currentuser.userid, user, tmpbuf, user,
				tmpbuf2);
			mail_buf(msgbuf, user, repbuf);
		}
		/*old action */
		else {
			sprintf(repbuf,
				// %s被%s取消在%s的POST权利
				"%s" "\xB1\xBB" "%s" "\xC8\xA1\xCF\xFB\xD4\xDA" "%s" "\xB5\xC4" "POST" "\xC8\xA8\xC0\xFB",
				repuser, currentuser.userid, currboard);
			securityreport(repbuf, msgbuf);
			deliverreport(repbuf, msgbuf);
			sprintf(repbuf,
				// %s被%s取消在%s的POST权利
				"%s" "\xB1\xBB" "%s" "\xC8\xA1\xCF\xFB\xD4\xDA" "%s" "\xB5\xC4" "POST" "\xC8\xA8\xC0\xFB",
				user, currentuser.userid, currboard);
			mail_buf(msgbuf, user, repbuf);
		}
		break;
	case CHANGEDENY:
		sprintf(repbuf,
			// %s改变封%s的时间或说明
			"%s" "\xB8\xC4\xB1\xE4\xB7\xE2" "%s" "\xB5\xC4\xCA\xB1\xBC\xE4\xBB\xF2\xCB\xB5\xC3\xF7", currentuser.userid, user);
		securityreport(repbuf, msgbuf);
		deliverreport(repbuf, msgbuf);
		mail_buf(msgbuf, user, repbuf);
		break;
	case UNDENY:
		sprintf(repbuf,
			// 恢复 %s 在 %s 的POST权
			"\xBB\xD6\xB8\xB4" " %s " "\xD4\xDA" " %s " "\xB5\xC4" "POST" "\xC8\xA8",
			// 全站
			repuser, isglobal ? "\xC8\xAB\xD5\xBE" : currboard);
		snprintf(msgbuf, 256, "%s %s\n"
			// 请理解版务管理工作,谢谢!\n
			"\xC7\xEB\xC0\xED\xBD\xE2\xB0\xE6\xCE\xF1\xB9\xDC\xC0\xED\xB9\xA4\xD7\xF7" "," "\xD0\xBB\xD0\xBB" "!\n", currentuser.userid,
			repbuf);
		securityreport(repbuf, msgbuf);
		deliverreport(repbuf, msgbuf);
		sprintf(repbuf,
			// 恢复 %s 在 %s 的POST权
			"\xBB\xD6\xB8\xB4" " %s " "\xD4\xDA" " %s " "\xB5\xC4" "POST" "\xC8\xA8",
			// 全站
			user, isglobal ? "\xC8\xAB\xD5\xBE" : currboard);
		snprintf(msgbuf, 256, "%s %s\n"
			// 请理解版务管理工作,谢谢!\n
			"\xC7\xEB\xC0\xED\xBD\xE2\xB0\xE6\xCE\xF1\xB9\xDC\xC0\xED\xB9\xA4\xD7\xF7" "," "\xD0\xBB\xD0\xBB" "!\n", currentuser.userid,
			repbuf);
		mail_buf(msgbuf, user, repbuf);
		break;
	}
	return 0;
}

int
mail_buf_slow(char *userid, char *title, char *content, char *sender)
{
	FILE *fp;
	char buf[256], dir[256];
	struct fileheader header;
	time_t t;
	time_t now;
	bzero(&header, sizeof (header));
	fh_setowner(&header, sender, 0);
	sprintf(buf, "mail/%c/%s/", mytoupper(userid[0]), userid);
	if (!file_isdir(buf))
		return -1;
	now = time(NULL);
	t = trycreatefile(buf, "M.%ld.A", now, 100);
	if (t < 0)
		return -1;
	header.filetime = t;
	ytht_strsncpy(header.title, title, sizeof(header.title));
	fp = fopen(buf, "w");
	if (fp == 0)
		return -2;
	fprintf(fp, "%s", content);
	fclose(fp);
	setmailfile_s(dir, sizeof(dir), userid, ".DIR");
	append_record(dir, &header, sizeof (header));
	return 0;
}

