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
#include "xyz.h"
#include "smth_screen.h"
#include "stuff.h"
#include "maintain.h"
#include "io.h"
#include "namecomplete.h"
#include "bcache.h"
#include "talk.h"
#include "main.h"
#include "mail.h"
#include "announce.h"
#include "bbsinc.h"
#include "boards.h"
#include "bbs_global_vars.h"

int offline(const char *s) {
	(void) s;
	char buf[STRLEN];

	modify_user_mode(OFFLINE);
	clear();
	if (HAS_PERM(PERM_SYSOP, currentuser) || HAS_PERM(PERM_BOARDS, currentuser) || HAS_PERM(PERM_ADMINMENU, currentuser) || HAS_PERM(PERM_SEEULEVELS, currentuser)) {
		move(1, 0);
		// \n\n您有重任在身, 不能随便自杀啦!!\n
		prints("\n\n\xC4\xFA\xD3\xD0\xD6\xD8\xC8\xCE\xD4\xDA\xC9\xED, \xB2\xBB\xC4\xDC\xCB\xE6\xB1\xE3\xD7\xD4\xC9\xB1\xC0\xB2!!\n");
		pressreturn();
		clear();
		return 0;
	}
	if (currentuser.stay < 86400) {
		move(1, 0);

		// \n\n对不起, 您还未够资格执行此命令!!\n
		prints("\n\n\xB6\xD4\xB2\xBB\xC6\xF0, \xC4\xFA\xBB\xB9\xCE\xB4\xB9\xBB\xD7\xCA\xB8\xF1\xD6\xB4\xD0\xD0\xB4\xCB\xC3\xFC\xC1\xEE!!\n");
		// 只有上站时间超过24小时的用户才能自杀.\n
		prints("\xD6\xBB\xD3\xD0\xC9\xCF\xD5\xBE\xCA\xB1\xBC\xE4\xB3\xAC\xB9\xFD" "24\xD0\xA1\xCA\xB1\xB5\xC4\xD3\xC3\xBB\xA7\xB2\xC5\xC4\xDC\xD7\xD4\xC9\xB1.\n");
		pressreturn();
		clear();
		return 0;
	}

	// 请输入你的密码:
	getdata(1, 0, "\xC7\xEB\xCA\xE4\xC8\xEB\xC4\xE3\xB5\xC4\xC3\xDC\xC2\xEB: ", buf, PASSLEN, NOECHO, YEA);
	if (*buf == '\0' || !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
		// \n\n很抱歉, 您输入的密码不正确。\n
		prints("\n\n\xBA\xDC\xB1\xA7\xC7\xB8, \xC4\xFA\xCA\xE4\xC8\xEB\xB5\xC4\xC3\xDC\xC2\xEB\xB2\xBB\xD5\xFD\xC8\xB7\xA1\xA3\n");
		pressreturn();
		clear();
		return 0;
	}
	// 请问你叫什么名字?
	getdata(3, 0, "\xC7\xEB\xCE\xCA\xC4\xE3\xBD\xD0\xCA\xB2\xC3\xB4\xC3\xFB\xD7\xD6? ", buf, NAMELEN, DOECHO, YEA);
	if (*buf == '\0' || strcmp(buf, currentuser.realname)) {
		// \n\n很抱歉, 我并不认识你。\n
		prints("\n\n\xBA\xDC\xB1\xA7\xC7\xB8, \xCE\xD2\xB2\xA2\xB2\xBB\xC8\xCF\xCA\xB6\xC4\xE3\xA1\xA3\n");
		pressreturn();
		clear();
		return 0;
	}
	clear();
	move(1, 0);
	// \033[1;5;31m警告\033[0;1;31m 自杀和id死亡释放邮箱没有任何关系\033[m \n
	prints("\033[1;5;31m\xBE\xAF\xB8\xE6\033[0;1;31m \xD7\xD4\xC9\xB1\xBA\xCDid\xCB\xC0\xCD\xF6\xCA\xCD\xB7\xC5\xD3\xCA\xCF\xE4\xC3\xBB\xD3\xD0\xC8\xCE\xBA\xCE\xB9\xD8\xCF\xB5\033[m \n");
	// 自杀仅仅是让您的id无法正常使用，而id的死亡只和生命力有关系\n
	prints("\xD7\xD4\xC9\xB1\xBD\xF6\xBD\xF6\xCA\xC7\xC8\xC3\xC4\xFA\xB5\xC4id\xCE\xDE\xB7\xA8\xD5\xFD\xB3\xA3\xCA\xB9\xD3\xC3\xA3\xAC\xB6\xF8id\xB5\xC4\xCB\xC0\xCD\xF6\xD6\xBB\xBA\xCD\xC9\xFA\xC3\xFC\xC1\xA6\xD3\xD0\xB9\xD8\xCF\xB5\n");
	// 如果有人告诉你自杀可以帮助您让您现有的id死亡并注册新的id，请离开这里\n
	prints("\xC8\xE7\xB9\xFB\xD3\xD0\xC8\xCB\xB8\xE6\xCB\xDF\xC4\xE3\xD7\xD4\xC9\xB1\xBF\xC9\xD2\xD4\xB0\xEF\xD6\xFA\xC4\xFA\xC8\xC3\xC4\xFA\xCF\xD6\xD3\xD0\xB5\xC4id\xCB\xC0\xCD\xF6\xB2\xA2\xD7\xA2\xB2\xE1\xD0\xC2\xB5\xC4id\xA3\xAC\xC7\xEB\xC0\xEB\xBF\xAA\xD5\xE2\xC0\xEF\n");
	// 然后把告诉你的人打一顿\n\n
	prints("\xC8\xBB\xBA\xF3\xB0\xD1\xB8\xE6\xCB\xDF\xC4\xE3\xB5\xC4\xC8\xCB\xB4\xF2\xD2\xBB\xB6\xD9\n\n");
	// \033[1;5;31m警告\033[0;1;31m： 自杀后, 您的灵魂将升入天国或堕入地狱, 愿您安息
	prints("\033[1;5;31m\xBE\xAF\xB8\xE6\033[0;1;31m\xA3\xBA \xD7\xD4\xC9\xB1\xBA\xF3, \xC4\xFA\xB5\xC4\xC1\xE9\xBB\xEA\xBD\xAB\xC9\xFD\xC8\xEB\xCC\xEC\xB9\xFA\xBB\xF2\xB6\xE9\xC8\xEB\xB5\xD8\xD3\xFC, \xD4\xB8\xC4\xFA\xB0\xB2\xCF\xA2");
	// \n\n\n\033[1;32m好难过喔.....\033[m\n\n\n
	prints("\n\n\n\033[1;32m\xBA\xC3\xC4\xD1\xB9\xFD\xE0\xB8.....\033[m\n\n\n");
	// 你确定要离开这个大家庭
	if (askyn("\xC4\xE3\xC8\xB7\xB6\xA8\xD2\xAA\xC0\xEB\xBF\xAA\xD5\xE2\xB8\xF6\xB4\xF3\xBC\xD2\xCD\xA5", NA, NA) == 1) {
		clear();
		set_safe_record();
		currentuser.dietime = currentuser.stay + 6*18 * 24 * 60 * 60;//改自杀恢复为6*18天   六道轮回+18层地狱
		substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
		Q_Goodbye();
		return 0;
	}

	return 0;
}

int online(const char *s) {
	(void) s;
	char buf[STRLEN];
	modify_user_mode(OFFLINE);
	clear();
	if ((currentuser.stay <= currentuser.dietime)) {
		move(1, 0);
		// \n\n死期未满啊!!\n
		prints("\n\n\xCB\xC0\xC6\xDA\xCE\xB4\xC2\xFA\xB0\xA1!!\n");
		// 你要吓死人啊!\n
		prints("\xC4\xE3\xD2\xAA\xCF\xC5\xCB\xC0\xC8\xCB\xB0\xA1!\n");
		// 你的死期还有 %d 分钟
		prints("\xC4\xE3\xB5\xC4\xCB\xC0\xC6\xDA\xBB\xB9\xD3\xD0 %d \xB7\xD6\xD6\xD3", 1 + (currentuser.dietime - currentuser.stay) / 60);
		pressreturn();
		clear();
		return -1;
	}
	// 请输入你的密码:
	getdata(1, 0, "\xC7\xEB\xCA\xE4\xC8\xEB\xC4\xE3\xB5\xC4\xC3\xDC\xC2\xEB: ", buf, PASSLEN, NOECHO, YEA);
	if (*buf == '\0' || !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
		// \n\n很抱歉, 您输入的密码不正确。\n
		prints("\n\n\xBA\xDC\xB1\xA7\xC7\xB8, \xC4\xFA\xCA\xE4\xC8\xEB\xB5\xC4\xC3\xDC\xC2\xEB\xB2\xBB\xD5\xFD\xC8\xB7\xA1\xA3\n");
		pressreturn();
		clear();
		return -2;
	}
	// 请问你叫什么名字?
	getdata(3, 0, "\xC7\xEB\xCE\xCA\xC4\xE3\xBD\xD0\xCA\xB2\xC3\xB4\xC3\xFB\xD7\xD6? ", buf, NAMELEN, DOECHO, YEA);
	if (*buf == '\0' || strcmp(buf, currentuser.realname)) {
		// \n\n很抱歉, 我并不认识你。\n
		prints("\n\n\xBA\xDC\xB1\xA7\xC7\xB8, \xCE\xD2\xB2\xA2\xB2\xBB\xC8\xCF\xCA\xB6\xC4\xE3\xA1\xA3\n");
		pressreturn();
		clear();
		return -3;
	}
	clear();
	move(1, 0);
	// \033[1;5;31m警告\033[0;1;31m： 生存,还是毁灭,是个值得考虑的问题
	prints("\033[1;5;31m\xBE\xAF\xB8\xE6\033[0;1;31m\xA3\xBA \xC9\xFA\xB4\xE6,\xBB\xB9\xCA\xC7\xBB\xD9\xC3\xF0,\xCA\xC7\xB8\xF6\xD6\xB5\xB5\xC3\xBF\xBC\xC2\xC7\xB5\xC4\xCE\xCA\xCC\xE2");
	// \n\n\n\033[1;32m您可要想清楚.....\033[m\n\n\n
	prints("\n\n\n\033[1;32m\xC4\xFA\xBF\xC9\xD2\xAA\xCF\xEB\xC7\xE5\xB3\xFE.....\033[m\n\n\n");
	// 你确定要返世为人了吗?
	if (askyn("\xC4\xE3\xC8\xB7\xB6\xA8\xD2\xAA\xB7\xB5\xCA\xC0\xCE\xAA\xC8\xCB\xC1\xCB\xC2\xF0?", NA, NA) == 1) {
		clear();
		currentuser.dietime = 0;
		substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
		return Q_Goodbye();
	}
	return 0;
}

int
kick_user(const struct user_info *userinfo, int mode)
{
	int id, ind;
	struct user_info uin;
	struct userec kuinfo;
	char kickuser[40];
	char kickreason[STRLEN];
	char titlebuf[STRLEN];
	char contentbuf[STRLEN];
	char repbuf[STRLEN];
	char msgbuf[STRLEN * 2];
	if (uinfo.mode != LUSERS && uinfo.mode != OFFLINE && uinfo.mode != FRIEND) {
		modify_user_mode(ADMIN);
		stand_title("Kick User");
		move(1, 0);
		usercomplete("Enter userid to be kicked: ", kickuser);
		if (*kickuser == '\0') {
			clear();
			return 0;
		}
		if (!(id = getuser(kickuser))) {
			move(3, 0);
			prints("Invalid User Id");
			clrtoeol();
			pressreturn();
			clear();
			return 0;
		}
		move(1, 0);
		prints("Kick User '%s'.", kickuser);
		clrtoeol();
		getdata(2, 0, "(Yes, or No) [N]: ", genbuf, 4, DOECHO, YEA);
		if (genbuf[0] != 'Y' && genbuf[0] != 'y') {	/* if not yes quit */
			move(2, 0);
			prints("Aborting Kick User\n");
			pressreturn();
			clear();
			return 0;
		}
		search_record(PASSFILE, &kuinfo, sizeof (kuinfo), (void *) cmpuids, kickuser);
		ind = search_ulist(&uin, t_cmpuids, id);
	} else {
		uin = *userinfo;
		strcpy(kickuser, uin.userid);
/*
		id = getuser(kickuser);
		search_record(PASSFILE, &kuinfo, sizeof(kuinfo), cmpuids, kickuser);
		ind = search_ulist( &uin, t_cmpuids, id );
*/
		ind = YEA;
	}


	//踢自己时 mode=1
	if(mode!=1)
	{
		clear();
		move(1,0);
		// 踢人原因:
		getdata(2,0,"\xCC\xDF\xC8\xCB\xD4\xAD\xD2\xF2: ", kickreason,STRLEN,DOECHO,YEA);
	} else {
		kickreason[0] = 0;
	}
	if (uin.pid != 1 && (!ind || !uin.active || uin.pid <= 0 || (kill(uin.pid, 0) == -1))) {
		if (uinfo.mode != LUSERS && uinfo.mode != OFFLINE && uinfo.mode != FRIEND) {
			move(3, 0);
			prints("User Has Logged Out");
			clrtoeol();
			pressreturn();
			clear();
		}
		return 0;
	} else if (kill(uin.pid, SIGHUP) < 0) {
		prints("User can't be kicked");
		pressreturn();
		clear();
		return 1;
	}
	sprintf(contentbuf, "%s",kickreason);
	// %s将%s踢出站外
	sprintf(titlebuf,"%s\xBD\xAB%s\xCC\xDF\xB3\xF6\xD5\xBE\xCD\xE2", currentuser.userid, kickuser);
	securityreport(titlebuf,contentbuf);

	// 您被%s强制离开本站
	sprintf(repbuf,"\xC4\xFA\xB1\xBB%s\xC7\xBF\xD6\xC6\xC0\xEB\xBF\xAA\xB1\xBE\xD5\xBE",currentuser.userid);
	// 理由:%s\n
	snprintf(msgbuf, sizeof(msgbuf), "\xC0\xED\xD3\xC9:%s\n",kickreason);
	mail_buf(msgbuf,kickuser, repbuf);
	sprintf(genbuf, "%s kick %s", currentuser.userid, kickuser);
	newtrace(genbuf);
	move(2, 0);
	if (uinfo.mode != LUSERS && uinfo.mode != OFFLINE && uinfo.mode != FRIEND) {
		prints("User has been Kicked\n");
		pressreturn();
		clear();
	}
	return 1;
}

// 用于 comm_list
// 此时 uinfo.mode = 10 (MMENU)，会进入查询逻辑
int kick_user_wrapper(const char *s) {
	(void) s;
	return kick_user(NULL, 0);
}

