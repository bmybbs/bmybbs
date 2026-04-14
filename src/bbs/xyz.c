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
#include <sys/mman.h>
#include "bbs.h"
#include "bbs_global_vars.h"
#include "smth_screen.h"
#include "maintain.h"
#include "bcache.h"
#include "stuff.h"
#include "io.h"
#include "edit.h"
#include "sendmsg.h"
#include "talk.h"
#include "namecomplete.h"
#include "mail.h"
#include "more.h"
#include "bbsinc.h"
#include "convcode.h"
#include "userinfo.h"
#include "main.h"
#include "bbs-internal.h"

#ifdef SSHBBS
extern int ssh_write(int fd, const void *buf, size_t count);
extern int ssh_read(int fd, void *buf, size_t count);
#endif

pid_t childpid;
static int loadkeys(struct one_key *key, char *name);
static int savekeys(struct one_key *key, char *name);
static void keyprint(char *buf, int key);
static int showkeyinfo(struct one_key *akey, int i);
static unsigned int setkeys(struct one_key *key);
static void myexec_cmd(int umode, int pager, const char *cmdfile, const char *param);
static void datapipefd(int fds, int fdn);
static int sendGoodWish(char *userid);
static void childreturn(int i);
static void escape_filename(char *fn);
static void bbs_zsendfile(char *filename);
static void get_load(double load[]);

static const char * const g_permstrings[] = {
	"\xBB\xF9\xB1\xBE\xC8\xA8\xC1\xA6",									/* PERM_BASIC     基本权力 */
	"\xBD\xF8\xC8\xEB\xC1\xC4\xCC\xEC\xCA\xD2",							/* PERM_CHAT      进入聊天室 */
	"\xBA\xF4\xBD\xD0\xCB\xFB\xC8\xCB\xC1\xC4\xCC\xEC",					/* PERM_PAGE      呼叫他人聊天 */
	"\xB7\xA2\xB1\xED\xCE\xC4\xD5\xC2",									/* PERM_POST      发表文章 */
	"\xCA\xB9\xD3\xC3\xD5\xDF\xD7\xCA\xC1\xCF\xD5\xFD\xC8\xB7",			/* PERM_LOGINOK   使用者资料正确 */
	"\xBD\xFB\xD6\xB9\xCA\xB9\xD3\xC3\xC7\xA9\xC3\xFB\xB5\xB5",			/* PERM_DENYSIG   禁止使用签名档 */
	"\xD2\xFE\xC9\xED\xCA\xF5",											/* PERM_CLOAK     隐身术 */
	"\xBF\xB4\xB4\xA9\xD2\xFE\xC9\xED\xCA\xF5",							/* PERM_SEECLOAK  看穿隐身术 */
	"\xD5\xCA\xBA\xC5\xD3\xC0\xBE\xC3\xB1\xA3\xC1\xF4",					/* PERM_XEMPT     帐号永久保留 */
	"\xB1\xE0\xBC\xAD\xBD\xF8\xD5\xBE\xBB\xAD\xC3\xE6",					/* PERM_WELCOME   编辑进站画面 */
	"\xB0\xE5\xD6\xF7",													/* PERM_BOARDS    板主 */
	"\xD5\xCA\xBA\xC5\xB9\xDC\xC0\xED\xD4\xB1",							/* PERM_ACCOUNTS  帐号管理员 */
	"\xB1\xBE\xD5\xBE\xD6\xD9\xB2\xC3",									/* PERM_ARBITRATE 本站仲裁 */
	"\xCD\xB6\xC6\xB1\xB9\xDC\xC0\xED\xD4\xB1",							/* PERM_OVOTE     投票管理员 */
	"\xCF\xB5\xCD\xB3\xCE\xAC\xBB\xA4\xB9\xDC\xC0\xED\xD4\xB1",			/* PERM_SYSOP     系统维护管理员 */
	"Read/Post \xCF\xDE\xD6\xC6",										/* PERM_POSTMASK  Read/Post 限制 */
	"\xBE\xAB\xBB\xAA\xC7\xF8\xD7\xDC\xB9\xDC",							/* PERM_ANNOUNCE  精华区总管 */
	"\xCC\xD6\xC2\xDB\xC7\xF8\xD7\xDC\xB9\xDC",							/* PERM_OBOARDS   讨论区总管 */
	"\xBB\xEE\xB6\xAF\xBF\xB4\xB0\xE6\xD7\xDC\xB9\xDC",					/* PERM_ACBOARD   活动看版总管 */
	"\xB2\xBB\xC4\xDC ZAP(\xCC\xD6\xC2\xDB\xC7\xF8\xD7\xA8\xD3\xC3)",	/* PERM_NOZAP     不能ZAP(讨论区专用) */
	"\xC7\xBF\xD6\xC6\xBA\xF4\xBD\xD0",									/* PERM_FORCEPAGE 强制呼叫 */
	"\xD1\xD3\xB3\xA4\xB7\xA2\xB4\xF4\xCA\xB1\xBC\xE4",					/* PERM_EXT_IDLE  延长发呆时间 */
	"\xB4\xF3\xD0\xC5\xCF\xE4",											/* PERM_SPECIAL1  大信箱 */
	"\xCC\xD8\xCA\xE2\xC8\xA8\xCF\xDE 2",								/* PERM_SPECIAL2  特殊权限 2 */
	"\xCC\xD8\xCA\xE2\xC8\xA8\xCF\xDE 3",								/* PERM_SPECIAL3  特殊权限 3 */
	"\xC7\xF8\xB3\xA4",													/* PERM_SPECIAL4  区长 */
	"\xB1\xBE\xD5\xBE\xBC\xE0\xB2\xEC\xD7\xE9",							/* PERM_SPECIAL5  本站监察组 */
	"\xB1\xBE\xD5\xBE\xC1\xA2\xB7\xA8\xBB\xE1",							/* PERM_SPECIAL6  本站立法会 */
	"\xCC\xD8\xCA\xE2\xC8\xA8\xCF\xDE 7",								/* PERM_SPECIAL7  特殊权限 7 */
	"\xB8\xF6\xC8\xCB\xCE\xC4\xBC\xAF",									/* PERM_SPECIAL8  个人文集 */
	"\xBD\xFB\xD6\xB9\xB7\xA2\xD0\xC5\xC8\xA8",							/* PERM_DENYMAIL  禁止发信权 */
};

static const char *const g_user_definestr[NUMDEFINES] = {
	/* DEF_FRIENDCALL         呼叫器关闭时可让好友呼叫 */
	"\xBA\xF4\xBD\xD0\xC6\xF7\xB9\xD8\xB1\xD5\xCA\xB1\xBF\xC9\xC8\xC3\xBA\xC3\xD3\xD1\xBA\xF4\xBD\xD0",
	/* DEF_ALLMSG             接受所有人的讯息 */
	"\xBD\xD3\xCA\xDC\xCB\xF9\xD3\xD0\xC8\xCB\xB5\xC4\xD1\xB6\xCF\xA2",
	/* DEF_FRIENDMSG          接受好友的讯息 */
	"\xBD\xD3\xCA\xDC\xBA\xC3\xD3\xD1\xB5\xC4\xD1\xB6\xCF\xA2",
	/* DEF_SOUNDMSG           收到讯息发出声音 */
	"\xCA\xD5\xB5\xBD\xD1\xB6\xCF\xA2\xB7\xA2\xB3\xF6\xC9\xF9\xD2\xF4",
	/* DEF_COLOR              使用彩色 */
	"\xCA\xB9\xD3\xC3\xB2\xCA\xC9\xAB",
	/* DEF_ACBOARD            显示活动看版 */
	"\xCF\xD4\xCA\xBE\xBB\xEE\xB6\xAF\xBF\xB4\xB0\xE6",
	/* DEF_ENDLINE            显示选单的讯息栏 */
	"\xCF\xD4\xCA\xBE\xD1\xA1\xB5\xA5\xB5\xC4\xD1\xB6\xCF\xA2\xC0\xB8",
	/* DEF_EDITMSG            编辑时显示状态栏 */
	"\xB1\xE0\xBC\xAD\xCA\xB1\xCF\xD4\xCA\xBE\xD7\xB4\xCC\xAC\xC0\xB8",
	/* DEF_NOTMSGFRIEND       讯息栏采用一般/精简模式 */
	"\xD1\xB6\xCF\xA2\xC0\xB8\xB2\xC9\xD3\xC3\xD2\xBB\xB0\xE3/\xBE\xAB\xBC\xF2\xC4\xA3\xCA\xBD",
	/* DEF_NORMALSCR          选单采用一般/精简模式 */
	"\xD1\xA1\xB5\xA5\xB2\xC9\xD3\xC3\xD2\xBB\xB0\xE3/\xBE\xAB\xBC\xF2\xC4\xA3\xCA\xBD",
	/* DEF_NEWPOST            分类讨论区以 New 显示 */
	"\xB7\xD6\xC0\xE0\xCC\xD6\xC2\xDB\xC7\xF8\xD2\xD4 New \xCF\xD4\xCA\xBE",
	/* DEF_CIRCLE             阅读文章是否使用绕卷选择 */
	"\xD4\xC4\xB6\xC1\xCE\xC4\xD5\xC2\xCA\xC7\xB7\xF1\xCA\xB9\xD3\xC3\xC8\xC6\xBE\xED\xD1\xA1\xD4\xF1",
	/* DEF_FIRSTNEW           阅读文章游标停于第一篇未读 */
	"\xD4\xC4\xB6\xC1\xCE\xC4\xD5\xC2\xD3\xCE\xB1\xEA\xCD\xA3\xD3\xDA\xB5\xDA\xD2\xBB\xC6\xAA\xCE\xB4\xB6\xC1",
	/* DEF_LOGFRIEND          进站时显示好友名单 */
	"\xBD\xF8\xD5\xBE\xCA\xB1\xCF\xD4\xCA\xBE\xBA\xC3\xD3\xD1\xC3\xFB\xB5\xA5",
	/* DEF_INNOTE             进站时显示备忘录 */
	"\xBD\xF8\xD5\xBE\xCA\xB1\xCF\xD4\xCA\xBE\xB1\xB8\xCD\xFC\xC2\xBC",
	/* DEF_OUTNOTE            离站时显示备忘录 */
	"\xC0\xEB\xD5\xBE\xCA\xB1\xCF\xD4\xCA\xBE\xB1\xB8\xCD\xFC\xC2\xBC",
	/* DEF_MAILMSG            离站时询问寄回所有讯息 */
	"\xC0\xEB\xD5\xBE\xCA\xB1\xD1\xAF\xCE\xCA\xBC\xC4\xBB\xD8\xCB\xF9\xD3\xD0\xD1\xB6\xCF\xA2",
	/* DEF_LOGOUT             使用自己的离站画面 */
	"\xCA\xB9\xD3\xC3\xD7\xD4\xBC\xBA\xB5\xC4\xC0\xEB\xD5\xBE\xBB\xAD\xC3\xE6",
	/* DEF_SEEWELC1           我是这个组织的成员 */
	"\xCE\xD2\xCA\xC7\xD5\xE2\xB8\xF6\xD7\xE9\xD6\xAF\xB5\xC4\xB3\xC9\xD4\xB1",
	/* DEF_LOGINFROM          好友上站通知 */
	"\xBA\xC3\xD3\xD1\xC9\xCF\xD5\xBE\xCD\xA8\xD6\xAA",
	/* DEF_NOTEPAD            观看留言版 */
	"\xB9\xDB\xBF\xB4\xC1\xF4\xD1\xD4\xB0\xE6",
	/* DEF_NOLOGINSEND        不要送出上站通知给好友 */
	"\xB2\xBB\xD2\xAA\xCB\xCD\xB3\xF6\xC9\xCF\xD5\xBE\xCD\xA8\xD6\xAA\xB8\xF8\xBA\xC3\xD3\xD1",
	/* DEF_THESIS             主题式看版 */
	"\xD6\xF7\xCC\xE2\xCA\xBD\xBF\xB4\xB0\xE6",
	/* DEF_MSGGETKEY          收到讯息等候回应或清除 */
	"\xCA\xD5\xB5\xBD\xD1\xB6\xCF\xA2\xB5\xC8\xBA\xF2\xBB\xD8\xD3\xA6\xBB\xF2\xC7\xE5\xB3\xFD",
	/* DEF_DELDBLCHAR         汉字整字处理 */
	"\xBA\xBA\xD7\xD6\xD5\xFB\xD7\xD6\xB4\xA6\xC0\xED",
	/* DEF_USEGB KCN 99.09.03 使用GB码阅读 */
	"\xCA\xB9\xD3\xC3\x47\x42\xC2\xEB\xD4\xC4\xB6\xC1",
	/* DEF_ANIENDLINE         使用动态底线 */
	"\xCA\xB9\xD3\xC3\xB6\xAF\xCC\xAC\xB5\xD7\xCF\xDF",
	/* DEF_INTOANN            初次访问版面提示进入精华区 */
	"\xB3\xF5\xB4\xCE\xB7\xC3\xCE\xCA\xB0\xE6\xC3\xE6\xCC\xE1\xCA\xBE\xBD\xF8\xC8\xEB\xBE\xAB\xBB\xAA\xC7\xF8",
	/* DEF_POSTNOMSG          发表文章时暂时屏蔽MSG */
	"\xB7\xA2\xB1\xED\xCE\xC4\xD5\xC2\xCA\xB1\xD4\xDD\xCA\xB1\xC6\xC1\xB1\xCEMSG",
	/* DEF_SEESTATINLOG       进站时观看统计信息 */
	"\xBD\xF8\xD5\xBE\xCA\xB1\xB9\xDB\xBF\xB4\xCD\xB3\xBC\xC6\xD0\xC5\xCF\xA2",
	/* DEF_FILTERXXX          过滤可能令人反感信息 */
	"\xB9\xFD\xC2\xCB\xBF\xC9\xC4\xDC\xC1\xEE\xC8\xCB\xB7\xB4\xB8\xD0\xD0\xC5\xCF\xA2",
	/* DEF_INTERNETMAIL       收取站外信件 */
	//"\xCA\xD5\xC8\xA1\xD5\xBE\xCD\xE2\xD0\xC5\xBC\xFE",
	/* DEF_NEWSTOP10          进站时观看全国十大排行榜 */
	"\xBD\xF8\xD5\xBE\xCA\xB1\xB9\xDB\xBF\xB4\xC8\xAB\xB9\xFA\xCA\xAE\xB4\xF3\xC5\xC5\xD0\xD0\xB0\xF1"
};

static int
loadkeys(struct one_key *key, char *name)
{
	int i;
	FILE *fp;
	fp = fopen(name, "r");
	if (fp == NULL)
		return 0;
	i = 0;
	while (key[i].fptr != NULL) {
		fread(&(key[i].key), sizeof (int), 1, fp);
		i++;
	}
	fclose(fp);
	return 1;
}

static int
savekeys(struct one_key *key, char *name)
{
	int i;
	FILE *fp;
	fp = fopen(name, "w");
	if (fp == NULL)
		return 0;
	i = 0;
	while (key[i].fptr != NULL) {
		fwrite(&(key[i].key), sizeof (int), 1, fp);
		i++;
	}
	fclose(fp);
	return 1;
}

void
loaduserkeys()
{
	char tempname[STRLEN];
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "readkey");
	loadkeys(read_comms, tempname);
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "mailkey");
	loadkeys(mail_comms, tempname);
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "friendkey");
	loadkeys(friend_list, tempname);
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "rejectkey");
	loadkeys(reject_list, tempname);
}

int modify_user_mode(int mode) {
	if (uinfo.mode == mode)
		return 0;
	uinfo.mode = mode;
	update_ulist(&uinfo, utmpent);
	return 0;
}

int
showperminfo(unsigned int pbits, int i, int use_define)
{
	char buf[STRLEN];

	sprintf(buf, "%c. %-30s %3s", 'A' + i,
		(use_define) ? g_user_definestr[i] : g_permstrings[i],
		((pbits >> i) & 1 ? "ON" : "OFF"));
	move(i + 6 - ((i > 15) ? 16 : 0), 0 + ((i > 15) ? 40 : 0));
	prints(buf);
	return YEA;
}

static void
keyprint(char *buf, int key)
{
	if (isprint(key))
		sprintf(buf, "%c", key);
	else {
		switch (key) {
		case KEY_TAB:
			strcpy(buf, "TAB");
			break;
		case KEY_ESC:
			strcpy(buf, "ESC");
			break;
		case KEY_UP:
			strcpy(buf, "UP");
			break;
		case KEY_DOWN:
			strcpy(buf, "DOWN");
			break;
		case KEY_RIGHT:
			strcpy(buf, "RIGHT");
			break;
		case KEY_LEFT:
			strcpy(buf, "LEFT");
			break;
		case KEY_HOME:
			strcpy(buf, "HOME");
			break;
		case KEY_INS:
			strcpy(buf, "INS");
			break;
		case KEY_DEL:
			strcpy(buf, "DEL");
			break;
		case KEY_END:
			strcpy(buf, "END");
			break;
		case KEY_PGUP:
			strcpy(buf, "PGUP");
			break;
		case KEY_PGDN:
			strcpy(buf, "PGDN");
			break;
		default:
			if (isprint(key | 0140))
				sprintf(buf, "Ctrl+%c", key | 0140);
			else
				sprintf(buf, "%x", key);
		}
	}
}

static int
showkeyinfo(struct one_key *akey, int i)
{
	char buf[STRLEN];
	char buf2[15];
	keyprint(buf2, (akey + i)->key);
	snprintf(buf, sizeof buf, "%c. %-26s %6s", '0' + i, (akey + i)->func, buf2);
	move(i + 2 - ((i > 19) ? 20 : 0), 0 + ((i > 19) ? 40 : 0));
	prints(buf);
	return YEA;
}

static unsigned int
setkeys(struct one_key *key)
{
	int i, j, done = NA;
	char choice[3];
	// 请按下你要的代码来设定键盘，按 Enter 结束.\n
	prints("\xC7\xEB\xB0\xB4\xCF\xC2\xC4\xE3\xD2\xAA\xB5\xC4\xB4\xFA\xC2\xEB\xC0\xB4\xC9\xE8\xB6\xA8\xBC\xFC\xC5\xCC\xA3\xAC\xB0\xB4 Enter \xBD\xE1\xCA\xF8.\n");
	i = 0;
	while (key[i].fptr != NULL && i < 40) {
		showkeyinfo(key, i);
		i++;
	}
	while (!done) {
		// 选择(ENTER 结束):
		getdata(t_lines - 1, 0, "\xD1\xA1\xD4\xF1(ENTER \xBD\xE1\xCA\xF8): ", choice, 2, DOECHO,
			YEA);
		*choice = toupper(*choice);
		if (*choice == '\n' || *choice == '\0')
			done = YEA;
		else if (*choice < '0' || *choice > '0' + i - 1)
			bell();
		else {
			j = *choice - '0';
			move(t_lines - 1, 0);
			// 请定义[\033[35m%s\033[m]的功能键:
			prints("\xC7\xEB\xB6\xA8\xD2\xE5[\033[35m%s\033[m]\xB5\xC4\xB9\xA6\xC4\xDC\xBC\xFC:", key[j].func);
			key[j].key = igetkey();
			showkeyinfo(key, j);
			/* d pbits ^= (1 << i);
				if((*showfunc)( pbits, i ,YEA)==NA)
				{
				pbits ^= (1 << i);
				} */
		}
	}
	pressreturn();
	return 0;
}

int x_copykeys(const char *s) {
	(void) s;
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	char ans[3];
	clear();
	move(0, 0);
	clrtoeol();

	// 请选择你要恢复的默认键位,按 Enter 结束.\n
	prints("\xC7\xEB\xD1\xA1\xD4\xF1\xC4\xE3\xD2\xAA\xBB\xD6\xB8\xB4\xB5\xC4\xC4\xAC\xC8\xCF\xBC\xFC\xCE\xBB,\xB0\xB4 Enter \xBD\xE1\xCA\xF8.\n");
	// [0]离开 [1]版面1 [2]版面2 [3]邮箱 [4]好友 [5]坏人 [6]全部 (默认[0]):
	getdata(1, 0, "[0]\xC0\xEB\xBF\xAA [1]\xB0\xE6\xC3\xE6""1 [2]\xB0\xE6\xC3\xE6""2 [3]\xD3\xCA\xCF\xE4 [4]\xBA\xC3\xD3\xD1 [5]\xBB\xB5\xC8\xCB [6]\xC8\xAB\xB2\xBF (\xC4\xAC\xC8\xCF[0]):", ans, 2, DOECHO, YEA);

	if (ans[0] == '0' || ans[0] == '\n' || ans[0] == '\0')
			return 0;

	if (ans[0] == '3' || ans[0] == '6')
	{
		int i = 0;
		while (mail_default_comms[i].fptr != NULL) {
			mail_comms[i].key= mail_default_comms[i].key;
			i++;
		}
		sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "mailkey");
		savekeys(&mail_comms[0], tempname);

	}

	// 设置完毕
	prints("\xC9\xE8\xD6\xC3\xCD\xEA\xB1\xCF");
	pressreturn();
	return 0;
}

/*
int
x_copykeys()
{
	int i, toc = 0;
	char choice[3];
	FILE *fp;
	char buf[STRLEN];
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	clear();
	move(0, 0);
	clrtoeol();
	// 请按下你要的代码来设定键盘类型,按 Enter 结束.\n
	prints("\xC7\xEB\xB0\xB4\xCF\xC2\xC4\xE3\xD2\xAA\xB5\xC4\xB4\xFA\xC2\xEB\xC0\xB4\xC9\xE8\xB6\xA8\xBC\xFC\xC5\xCC\xC0\xE0\xD0\xCD,\xB0\xB4 Enter \xBD\xE1\xCA\xF8.\n");
	fp = fopen(MY_BBS_HOME "/etc/keys", "r");
	if (fp == NULL)
		return -1;
	i = 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		sprintf(tempname, "%c. %-26s", 'A' + i, buf);
		move(i + 2 - ((i > 19) ? 20 : 0), 0 + ((i > 19) ? 40 : 0));
		prints(tempname);
		i++;
	}
	// 选择(ENTER 结束):
	getdata(t_lines - 1, 0, "\xD1\xA1\xD4\xF1(ENTER \xBD\xE1\xCA\xF8): ", choice, 2, DOECHO, YEA);
	*choice = toupper(*choice);
	if (*choice == '\n' || *choice == '\0')
		return 0;
	else if (*choice < 'A' || *choice > 'A' + i - 1)
		bell();
	else {
		toc = 0;
		i = *choice - 'A';
		if (currentuser.userlevel & PERM_SYSOP) {
			getdata(t_lines - 1, 0,
				// 是否把当前你自己的键盘定义设置为系统定义(Y/N):N
				"\xCA\xC7\xB7\xF1\xB0\xD1\xB5\xB1\xC7\xB0\xC4\xE3\xD7\xD4\xBC\xBA\xB5\xC4\xBC\xFC\xC5\xCC\xB6\xA8\xD2\xE5\xC9\xE8\xD6\xC3\xCE\xAA\xCF\xB5\xCD\xB3\xB6\xA8\xD2\xE5(Y/N):N ",
				choice, 2, DOECHO, YEA);
			if (*choice == 'Y')
				toc = 1;
		}
		if (toc) {
			sprintf(buf, MY_BBS_HOME "/etc/readkey.%d", i);
			savekeys(&read_comms[0], buf);
			sprintf(buf, MY_BBS_HOME "/etc/mailkey.%d", i);
			savekeys(&mail_comms[0], buf);
			sprintf(buf, MY_BBS_HOME "/etc/friendkey.%d", i);
			savekeys(&friend_list[0], buf);
			sprintf(buf, MY_BBS_HOME "/etc/rejectkey.%d", i);
			savekeys(&reject_list[0], buf);
		} else {
			sprintf(buf, MY_BBS_HOME "/etc/readkey.%d", i);
			loadkeys(&read_comms[0], buf);
			setuserfile(buf, "readkey");
			savekeys(&read_comms[0], buf);
			sprintf(buf, MY_BBS_HOME "/etc/mailkey.%d", i);
			loadkeys(&mail_comms[0], buf);
			setuserfile(buf, "mailkey");
			savekeys(&mail_comms[0], buf);
			sprintf(buf, MY_BBS_HOME "/etc/friendkey.%d", i);
			loadkeys(&friend_list[0], buf);
			setuserfile(buf, "friendkey");
			savekeys(&friend_list[0], buf);
			sprintf(buf, MY_BBS_HOME "/etc/rejectkey.%d", i);
			loadkeys(&reject_list[0], buf);
			setuserfile(buf, "rejectkey");
			savekeys(&reject_list[0], buf);
		}
		move(t_lines - 1, 0);
		// 设置完毕
		prints("\xC9\xE8\xD6\xC3\xCD\xEA\xB1\xCF");
	}
	pressreturn();
	return 0;
}
*/

int x_setkeys(const char *s) {
	(void) s;
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	clear();
	move(0, 0);
	clrtoeol();
	setkeys(&read_comms[0]);
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "readkey");
	savekeys(&read_comms[0], tempname);
	return 0;
}

int x_setkeys2(const char *s) {
	(void) s;
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	clear();
	move(0, 0);
	clrtoeol();
	setkeys(&read_comms[40]);
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "readkey");
	savekeys(&read_comms[0], tempname);
	return 0;
}

int x_setkeys3(const char *s) {
	(void) s;
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	clear();
	move(0, 0);
	clrtoeol();
	setkeys(&mail_comms[0]);
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "mailkey");
	savekeys(&mail_comms[0], tempname);
	return 0;
}

int x_setkeys4(const char *s) {
	(void) s;
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	clear();
	move(0, 0);
	clrtoeol();
	setkeys(&friend_list[0]);
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "friendkey");
	savekeys(&friend_list[0], tempname);
	return 0;
}

int x_setkeys5(const char *s) {
	(void) s;
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	clear();
	move(0, 0);
	clrtoeol();
	setkeys(&reject_list[0]);
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "rejectdkey");
	savekeys(&reject_list[0], tempname);
	return 0;
}

unsigned int
setperms(unsigned int pbits, char *prompt, int numbers, int (*showfunc) (unsigned int, int, int), int param)
{
	int lastperm = numbers - 1;
	int i, done = NA;
	char choice[3];

	move(4, 0);
	// 请按下你要的代码来设定%s，按 Enter 结束.\n
	prints("\xC7\xEB\xB0\xB4\xCF\xC2\xC4\xE3\xD2\xAA\xB5\xC4\xB4\xFA\xC2\xEB\xC0\xB4\xC9\xE8\xB6\xA8%s\xA3\xAC\xB0\xB4 Enter \xBD\xE1\xCA\xF8.\n", prompt);
	move(6, 0);
	clrtobot();
/*    pbits &= (1 << numbers) - 1;*/
	for (i = 0; i <= lastperm; i++) {
		(*showfunc) (pbits, i, param);
	}
	while (!done) {
		// 选择(ENTER 结束):
		getdata(t_lines - 1, 0, "\xD1\xA1\xD4\xF1(ENTER \xBD\xE1\xCA\xF8): ", choice, 2, DOECHO,
			YEA);
		*choice = toupper(*choice);
		if (*choice == '\n' || *choice == '\0')
			done = YEA;
		else if (*choice < 'A' || *choice > 'A' + lastperm)
			bell();
		else {
			i = *choice - 'A';
			pbits ^= (1 << i);
			if ((*showfunc) (pbits, i, param) == NA) {
				pbits ^= (1 << i);
			}
		}
	}
	return (pbits);
}

int x_level(const char *s) {
	(void) s;
	int id, oldlevel;
	unsigned int newlevel;
	char content[2048];

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();
	move(0, 0);
	prints("Change User Priority\n");
	clrtoeol();
	move(1, 0);
	usercomplete("Enter userid to be changed: ", genbuf);
	if (genbuf[0] == '\0') {
		clear();
		return 0;
	}
	if (!(id = getuser(genbuf))) {
		move(3, 0);
		prints("Invalid User Id");
		clrtoeol();
		pressreturn();
		clear();
		return -1;
	}
	move(1, 0);
	clrtobot();
	move(2, 0);
	prints("Set the desired permissions for user '%s'\n", genbuf);
	// 权限
	newlevel = setperms(lookupuser.userlevel, "\xC8\xA8\xCF\xDE", NUMPERMS, showperminfo, 0);
	move(2, 0);
	if (newlevel == lookupuser.userlevel)
		prints("User '%s' level NOT changed\n", lookupuser.userid);
	else {
		oldlevel = lookupuser.userlevel;
		lookupuser.userlevel = newlevel;
		{
			char secu[STRLEN];
			// 修改 %s 的权限
			sprintf(secu, "\xD0\xDE\xB8\xC4 %s \xB5\xC4\xC8\xA8\xCF\xDE", lookupuser.userid);
			permtostr(oldlevel, genbuf);
			// 修改前的权限：%s\n修改后的权限：
			sprintf(content, "\xD0\xDE\xB8\xC4\xC7\xB0\xB5\xC4\xC8\xA8\xCF\xDE\xA3\xBA%s\n\xD0\xDE\xB8\xC4\xBA\xF3\xB5\xC4\xC8\xA8\xCF\xDE\xA3\xBA",
				genbuf);
			permtostr(lookupuser.userlevel, genbuf);
			strcat(content, genbuf);
			securityreport(secu, content);
		}
		substitute_record(PASSFILE, &lookupuser, sizeof (struct userec), id);
		prints("User '%s' level changed\n", lookupuser.userid);
	}
	pressreturn();
	clear();
	return 0;
}

int x_userdefine(const char *s) {
	(void) s;
	int id;
	unsigned int newlevel;
	extern int nettyNN;

	modify_user_mode(USERDEF);
	if (!(id = getuser(currentuser.userid))) {
		move(3, 0);
		// 错误的使用者 ID...
		prints("\xB4\xED\xCE\xF3\xB5\xC4\xCA\xB9\xD3\xC3\xD5\xDF ID...");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	move(1, 0);
	clrtobot();
	move(2, 0);
	// 参数
	newlevel = setperms(lookupuser.userdefine, "\xB2\xCE\xCA\xFD", NUMDEFINES, showperminfo, 1);
	move(2, 0);
	if (newlevel == lookupuser.userdefine)
		// 参数没有修改...\n
		prints("\xB2\xCE\xCA\xFD\xC3\xBB\xD3\xD0\xD0\xDE\xB8\xC4...\n");
	else {
		lookupuser.userdefine = newlevel;
		currentuser.userdefine = newlevel;
		if ((!g_convcode && !(newlevel & DEF_USEGB)) || (g_convcode && (newlevel & DEF_USEGB)))
			switch_code();
		substitute_record(PASSFILE, &lookupuser, sizeof (struct userec), id);
		uinfo.pager |= FRIEND_PAGER;
		if (!(uinfo.pager & ALL_PAGER)) {
			if (!DEFINE(DEF_FRIENDCALL, currentuser))
				uinfo.pager &= ~FRIEND_PAGER;
		}
		uinfo.pager &= ~ALLMSG_PAGER;
		uinfo.pager &= ~FRIENDMSG_PAGER;
		if (DEFINE(DEF_DELDBLCHAR, currentuser))
			enabledbchar = 1;
		else
			enabledbchar = 0;
		if (DEFINE(DEF_FRIENDMSG, currentuser)) {
			uinfo.pager |= FRIENDMSG_PAGER;
		}
		if (DEFINE(DEF_ALLMSG, currentuser)) {
			uinfo.pager |= ALLMSG_PAGER;
			uinfo.pager |= FRIENDMSG_PAGER;
		}
		update_utmp();
		if (DEFINE(DEF_ACBOARD, currentuser))
			nettyNN = NNread_init();
		// 新的参数设定完成...\n\n
		prints("\xD0\xC2\xB5\xC4\xB2\xCE\xCA\xFD\xC9\xE8\xB6\xA8\xCD\xEA\xB3\xC9...\n\n");
	}
	iscolor = (DEFINE(DEF_COLOR, currentuser)) ? 1 : 0;
	pressreturn();
	clear();
	return 0;
}

int x_cloak(const char *s) {
	(void) s;
	modify_user_mode(GMENU);
	uinfo.invisible = (uinfo.invisible) ? NA : YEA;
	update_utmp();
	if ((currentuser.userlevel & PERM_BOARDS))
		setbmstatus(1);
	if (!uinfo.in_chat) {
		move(1, 0);
		clrtoeol();
		if(uinfo.invisible)				//add by mintbaggio@BMY for normal cloak
			currentuser./*pseudo_*/lastlogout = time(NULL);
		else
			currentuser./*pseudo_*/lastlogout = 0;
		substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
		// 隐身术 (cloak) 已经%s了!
		// 启动
		// 停止
		prints("\xD2\xFE\xC9\xED\xCA\xF5 (cloak) \xD2\xD1\xBE\xAD%s\xC1\xCB!", (uinfo.invisible) ? "\xC6\xF4\xB6\xAF" : "\xCD\xA3\xD6\xB9");
		pressreturn();
	}
	return 0;
}

int x_edits(const char *s) {
	(void) s;
	int aborted;
	char ans[7], buf[STRLEN];
	int ch, num, confirm;
	extern int WishNum;
	static char *const e_file[] = {
		"plans", "signatures", "notes", "logout", "GoodWish", "bansite",
		NULL
	};
	static char *const explain_file[] = {
		// 个人说明档
		"\xB8\xF6\xC8\xCB\xCB\xB5\xC3\xF7\xB5\xB5",
		// 签名档
		"\xC7\xA9\xC3\xFB\xB5\xB5",
		// 自己的备忘录
		"\xD7\xD4\xBC\xBA\xB5\xC4\xB1\xB8\xCD\xFC\xC2\xBC",
		// 离站的画面
		"\xC0\xEB\xD5\xBE\xB5\xC4\xBB\xAD\xC3\xE6",
		// 底部流动信息
		"\xB5\xD7\xB2\xBF\xC1\xF7\xB6\xAF\xD0\xC5\xCF\xA2",
		// 禁止本ID登录的IP(每个IP占一行)
		"\xBD\xFB\xD6\xB9\xB1\xBEID\xB5\xC7\xC2\xBC\xB5\xC4IP(\xC3\xBF\xB8\xF6IP\xD5\xBC\xD2\xBB\xD0\xD0)",
		NULL
	};

	modify_user_mode(GMENU);
	clear();
	move(1, 0);
	// 编修个人档案\n\n
	prints("\xB1\xE0\xD0\xDE\xB8\xF6\xC8\xCB\xB5\xB5\xB0\xB8\n\n");
	for (num = 0; e_file[num] != NULL && explain_file[num] != NULL; num++) {
		prints("[\033[1;32m%d\033[m] %s\n", num + 1, explain_file[num]);
	}
	// [\033[1;32m%d\033[m] 都不想改\n
	prints("[\033[1;32m%d\033[m] \xB6\xBC\xB2\xBB\xCF\xEB\xB8\xC4\n", num + 1);

	// 你要编修哪一项个人档案:
	getdata(num + 5, 0, "\xC4\xE3\xD2\xAA\xB1\xE0\xD0\xDE\xC4\xC4\xD2\xBB\xCF\xEE\xB8\xF6\xC8\xCB\xB5\xB5\xB0\xB8: ", ans, 2, DOECHO, YEA);
	if (ans[0] - '0' <= 0 || ans[0] - '0' > num || ans[0] == '\n' || ans[0] == '\0')
		return 0;

	ch = ans[0] - '0' - 1;
	sethomefile_s(genbuf, sizeof(genbuf), currentuser.userid, e_file[ch]);
	move(3, 0);
	clrtobot();
	// (E)编辑 (D)删除 %s? [E]:
	sprintf(buf, "(E)\xB1\xE0\xBC\xAD (D)\xC9\xBE\xB3\xFD %s? [E]: ", explain_file[ch]);
	getdata(3, 0, buf, ans, 2, DOECHO, YEA);
	if (ans[0] == 'D' || ans[0] == 'd') {
		// 你确定要删除这个档案
		confirm = askyn("\xC4\xE3\xC8\xB7\xB6\xA8\xD2\xAA\xC9\xBE\xB3\xFD\xD5\xE2\xB8\xF6\xB5\xB5\xB0\xB8", NA, NA);
		if (confirm != 1) {
			move(5, 0);
			// 取消删除行动\n
			prints("\xC8\xA1\xCF\xFB\xC9\xBE\xB3\xFD\xD0\xD0\xB6\xAF\n");
			pressreturn();
			clear();
			return 0;
		}
		unlink(genbuf);
		move(5, 0);
		// %s 已删除\n
		prints("%s \xD2\xD1\xC9\xBE\xB3\xFD\n", explain_file[ch]);
		pressreturn();
		if (ch == 4)
			WishNum = 9999;
		clear();
		return 0;
	}
	modify_user_mode(EDITUFILE);
	aborted = vedit(genbuf, NA, YEA);
	clear();
	if (!aborted) {
		// %s 更新过\n
		prints("%s \xB8\xFC\xD0\xC2\xB9\xFD\n", explain_file[ch]);
		sprintf(buf, "edit %s", explain_file[ch]);
		if (!strcmp(e_file[ch], "signatures")) {
			set_numofsig();
			// 系统重新设定以及读入你的签名档...
			prints("\xCF\xB5\xCD\xB3\xD6\xD8\xD0\xC2\xC9\xE8\xB6\xA8\xD2\xD4\xBC\xB0\xB6\xC1\xC8\xEB\xC4\xE3\xB5\xC4\xC7\xA9\xC3\xFB\xB5\xB5...");
		}
	} else
		// %s 取消修改\n
		prints("%s \xC8\xA1\xCF\xFB\xD0\xDE\xB8\xC4\n", explain_file[ch]);
	pressreturn();
	if (ch == 4)
		WishNum = 9999;
	return 0;
}

int a_edits(const char *s) {
	(void) s;
	int aborted;
	char ans[7], buf[STRLEN], buf2[STRLEN];
	int ch, num, confirm;
	/*modified by pzhg 070412 for voteing for valid ids in spacific boards*/
	static char *const e_file[] = {
		"../Welcome", "../Welcome2", "issue", "logout", "movie",
		"endline", "../vote/notes",
		"menu.ini", "badname0", "badname", "../.bad_email",
		"../.bansite",
		"../.blockmail",
		"autopost", "junkboards", "sysops", "prisonor", "untrust",
		"bbsnetA.ini", "bbsnet.ini", "filtertitle",
		"../ftphome/ftp_adm", "badwords", "sbadwords", "pbadwords",
		"../inndlog/newsfeeds.bbs", "spec_site", "secmlist","special","life",
		"commendlist", "manager_team","./pop_register/mail.xjtu.edu.cn","./pop_register/stu.xjtu.edu.cn",
		"top10forbid","voteidboards","newboard","recommboard","guestbanip",NULL
	};
	static char *const explain_file[] = {
		// 特殊进站公布栏
		"\xCC\xD8\xCA\xE2\xBD\xF8\xD5\xBE\xB9\xAB\xB2\xBC\xC0\xB8",
		// 进站画面
		"\xBD\xF8\xD5\xBE\xBB\xAD\xC3\xE6",
		// 进站欢迎档
		"\xBD\xF8\xD5\xBE\xBB\xB6\xD3\xAD\xB5\xB5",
		// 离站画面
		"\xC0\xEB\xD5\xBE\xBB\xAD\xC3\xE6",
		// 活动看版
		"\xBB\xEE\xB6\xAF\xBF\xB4\xB0\xE6",
		// 屏幕底线
		"\xC6\xC1\xC4\xBB\xB5\xD7\xCF\xDF",
		// 公用备忘录
		"\xB9\xAB\xD3\xC3\xB1\xB8\xCD\xFC\xC2\xBC",
		"menu.ini",
		// 不可注册的 ID
		"\xB2\xBB\xBF\xC9\xD7\xA2\xB2\xE1\xB5\xC4 ID",
		// ID 中不能包含的字串
		"ID \xD6\xD0\xB2\xBB\xC4\xDC\xB0\xFC\xBA\xAC\xB5\xC4\xD7\xD6\xB4\xAE",
		// 不可确认之E-Mail
		("\xB2\xBB\xBF\xC9\xC8\xB7\xC8\xCF\xD6\xAE""E-Mail"),
		// 不可上站之位址
		"\xB2\xBB\xBF\xC9\xC9\xCF\xD5\xBE\xD6\xAE\xCE\xBB\xD6\xB7",
		// 拒收E-mail黑名单
		("\xBE\xDC\xCA\xD5""E-mail\xBA\xDA\xC3\xFB\xB5\xA5"),
		// 每日自动送信档
		"\xC3\xBF\xC8\xD5\xD7\xD4\xB6\xAF\xCB\xCD\xD0\xC5\xB5\xB5",
		// 不算POST数的版
		"\xB2\xBB\xCB\xE3POST\xCA\xFD\xB5\xC4\xB0\xE6",
		// 管理者名单
		"\xB9\xDC\xC0\xED\xD5\xDF\xC3\xFB\xB5\xA5",
		// 服刑人员名单
		"\xB7\xFE\xD0\xCC\xC8\xCB\xD4\xB1\xC3\xFB\xB5\xA5",
		// 不信任IP列表
		"\xB2\xBB\xD0\xC5\xC8\xCEIP\xC1\xD0\xB1\xED",
		// 本地网络连线
		"\xB1\xBE\xB5\xD8\xCD\xF8\xC2\xE7\xC1\xAC\xCF\xDF",
		// 网络连线
		"\xCD\xF8\xC2\xE7\xC1\xAC\xCF\xDF",
		// 底线需要过滤的标题
		"\xB5\xD7\xCF\xDF\xD0\xE8\xD2\xAA\xB9\xFD\xC2\xCB\xB5\xC4\xB1\xEA\xCC\xE2",
		// FTP管理员名单
		"FTP\xB9\xDC\xC0\xED\xD4\xB1\xC3\xFB\xB5\xA5",
		// 过滤词汇
		"\xB9\xFD\xC2\xCB\xB4\xCA\xBB\xE3",
		// 精简过滤词汇
		"\xBE\xAB\xBC\xF2\xB9\xFD\xC2\xCB\xB4\xCA\xBB\xE3",
		// 报警词汇
		"\xB1\xA8\xBE\xAF\xB4\xCA\xBB\xE3",
		// 转信版和新闻组对应
		"\xD7\xAA\xD0\xC5\xB0\xE6\xBA\xCD\xD0\xC2\xCE\xC5\xD7\xE9\xB6\xD4\xD3\xA6",
		// 穿梭IP限制次数
		"\xB4\xA9\xCB\xF3IP\xCF\xDE\xD6\xC6\xB4\xCE\xCA\xFD",
		// 主管站长
		"\xD6\xF7\xB9\xDC\xD5\xBE\xB3\xA4",
		// id标识
		"id\xB1\xEA\xCA\xB6",
		// 生命力设定
		"\xC9\xFA\xC3\xFC\xC1\xA6\xC9\xE8\xB6\xA8",
		// 推荐文章
		"\xCD\xC6\xBC\xF6\xCE\xC4\xD5\xC2",
		// 管理团队名单
		"\xB9\xDC\xC0\xED\xCD\xC5\xB6\xD3\xC3\xFB\xB5\xA5",
		// 信箱注册mail.xjtu.edu.cn
		"\xD0\xC5\xCF\xE4\xD7\xA2\xB2\xE1mail.xjtu.edu.cn",
		// 信箱注册stu.xjtu.edu.cn
		"\xD0\xC5\xCF\xE4\xD7\xA2\xB2\xE1stu.xjtu.edu.cn",
		// 不允许上10大版面
		("\xB2\xBB\xD4\xCA\xD0\xED\xC9\xCF""10\xB4\xF3\xB0\xE6\xC3\xE6"),
		// 限制投票权版面
		"\xCF\xDE\xD6\xC6\xCD\xB6\xC6\xB1\xC8\xA8\xB0\xE6\xC3\xE6",
		// 新开版面
		"\xD0\xC2\xBF\xAA\xB0\xE6\xC3\xE6",
		// 推荐版面
		"\xCD\xC6\xBC\xF6\xB0\xE6\xC3\xE6",
		// 禁止guest发文的ip
		"\xBD\xFB\xD6\xB9guest\xB7\xA2\xCE\xC4\xB5\xC4ip",
		NULL
	};//modify by wjbta@bmy  增加id标识的显示, modify by mintbaggio 040406 for front page commend

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return 0;
	}
	clear();
	move(0, 0);
	// 编修系统档案\n\n
	prints("\xB1\xE0\xD0\xDE\xCF\xB5\xCD\xB3\xB5\xB5\xB0\xB8\n\n");
	for (num = 0;
			HAS_PERM(PERM_SYSOP, currentuser) ? e_file[num] != NULL
			&& explain_file[num] != NULL : strcasecmp(explain_file[num], "menu.ini") != 0;
			num++) {
		if (num >= 20)
			move(num - 20 + 2, 39);
		prints("[%2d] %s\n", num + 1, explain_file[num]);
	}
	if (num >= 20)
		move(num - 20 + 2, 39);
	// [%2d] 都不想改\n
	prints("[%2d] \xB6\xBC\xB2\xBB\xCF\xEB\xB8\xC4\n", num + 1);

	// 你要编修哪一项系统档案:
	getdata(t_lines - 1, 0, "\xC4\xE3\xD2\xAA\xB1\xE0\xD0\xDE\xC4\xC4\xD2\xBB\xCF\xEE\xCF\xB5\xCD\xB3\xB5\xB5\xB0\xB8: ", ans, 3, DOECHO,
		YEA);
	ch = atoi(ans);
	if (!isdigit(ans[0]) || ch <= 0 || ch > num || ans[0] == '\n' || ans[0] == '\0')
		return 0;
	ch -= 1;
	sprintf(buf2, "etc/%s", e_file[ch]);
	move(3, 0);
	clrtobot();
	// (E)编辑 (D)删除 %s? [E]:
	sprintf(buf, "(E)\xB1\xE0\xBC\xAD (D)\xC9\xBE\xB3\xFD %s? [E]: ", explain_file[ch]);
	getdata(3, 0, buf, ans, 2, DOECHO, YEA);
	if (ans[0] == 'D' || ans[0] == 'd') {
		// 你确定要删除这个系统档
		confirm = askyn("\xC4\xE3\xC8\xB7\xB6\xA8\xD2\xAA\xC9\xBE\xB3\xFD\xD5\xE2\xB8\xF6\xCF\xB5\xCD\xB3\xB5\xB5", NA, NA);
		if (confirm != 1) {
			move(5, 0);
			// 取消删除行动\n
			prints("\xC8\xA1\xCF\xFB\xC9\xBE\xB3\xFD\xD0\xD0\xB6\xAF\n");
			pressreturn();
			clear();
			return 0;
		}
		{
			char secu[STRLEN];
			// 删除系统档案：%s
			sprintf(secu, "\xC9\xBE\xB3\xFD\xCF\xB5\xCD\xB3\xB5\xB5\xB0\xB8\xA3\xBA%s", explain_file[ch]);
			securityreport(secu, secu);
		}
		unlink(buf2);
		move(5, 0);
		// %s 已删除\n
		prints("%s \xD2\xD1\xC9\xBE\xB3\xFD\n", explain_file[ch]);
		pressreturn();
		clear();
		return 0;
	}
	modify_user_mode(EDITSFILE);
	aborted = vedit(buf2, NA, YEA);
	clear();
	if (aborted != -1) {
		// %s 更新过
		prints("%s \xB8\xFC\xD0\xC2\xB9\xFD", explain_file[ch]);
		{
			char secu[STRLEN];
			// 修改系统档案：%s
			sprintf(secu, "\xD0\xDE\xB8\xC4\xCF\xB5\xCD\xB3\xB5\xB5\xB0\xB8\xA3\xBA%s", explain_file[ch]);
			securityreport(secu, secu);
		}

		if (!strcmp(e_file[ch], "../Welcome")) {
			unlink("Welcome.rec");
			// \nWelcome 记录档更新
			prints("\nWelcome \xBC\xC7\xC2\xBC\xB5\xB5\xB8\xFC\xD0\xC2");
		}
	}
	pressreturn();
	return 0;
}

//added by pzhg for SysFiles2
int a_edits2(const char *s) {
	(void) s;
	int aborted;
	char ans[7], buf[STRLEN], buf2[STRLEN];
	int ch, num, confirm;
	static char *const e_file[] = {
		"birthday","sysboards","adpost","ad_banner","ad_left", "secorder",
		NULL
	};
	static char *const explain_file[] = {
		// 生日欢迎画面
		"\xC9\xFA\xC8\xD5\xBB\xB6\xD3\xAD\xBB\xAD\xC3\xE6",
		// 站务管理版面
		"\xD5\xBE\xCE\xF1\xB9\xDC\xC0\xED\xB0\xE6\xC3\xE6",
		// 滚动广告
		"\xB9\xF6\xB6\xAF\xB9\xE3\xB8\xE6",
		"Banner",
		"Left Ads",
		// WWW讨论区顺序
		"WWW\xCC\xD6\xC2\xDB\xC7\xF8\xCB\xB3\xD0\xF2",
		NULL
	};

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return 0;
	}
	clear();
	move(0, 0);
	// 编修系统档案2\n\n
	prints("\xB1\xE0\xD0\xDE\xCF\xB5\xCD\xB3\xB5\xB5\xB0\xB8""2\n\n");
	for (num = 0;
			HAS_PERM(PERM_SYSOP, currentuser) ? e_file[num] != NULL
			&& explain_file[num] != NULL : strcasecmp(explain_file[num], "menu.ini") != 0;
			num++) {
		if (num >= 20)
			move(num - 20 + 2, 39);
		prints("[%2d] %s\n", num + 1, explain_file[num]);
	}
	if (num >= 20)
		move(num - 20 + 2, 39);
	// [%2d] 都不想改\n
	prints("[%2d] \xB6\xBC\xB2\xBB\xCF\xEB\xB8\xC4\n", num + 1);

	// 你要编修哪一项系统档案:
	getdata(t_lines - 1, 0, "\xC4\xE3\xD2\xAA\xB1\xE0\xD0\xDE\xC4\xC4\xD2\xBB\xCF\xEE\xCF\xB5\xCD\xB3\xB5\xB5\xB0\xB8: ", ans, 3, DOECHO,
		YEA);
	ch = atoi(ans);
	if (!isdigit(ans[0]) || ch <= 0 || ch > num || ans[0] == '\n' || ans[0] == '\0')
		return 0;
	ch = ch % 6; // items of e_file
	sprintf(buf2, "etc/%s", e_file[ch]);
	move(3, 0);
	clrtobot();
	// (E)编辑 (D)删除 %s? [E]:
	sprintf(buf, "(E)\xB1\xE0\xBC\xAD (D)\xC9\xBE\xB3\xFD %s? [E]: ", explain_file[ch]);
	getdata(3, 0, buf, ans, 2, DOECHO, YEA);
	if (ans[0] == 'D' || ans[0] == 'd') {
		// 你确定要删除这个系统档
		confirm = askyn("\xC4\xE3\xC8\xB7\xB6\xA8\xD2\xAA\xC9\xBE\xB3\xFD\xD5\xE2\xB8\xF6\xCF\xB5\xCD\xB3\xB5\xB5", NA, NA);
		if (confirm != 1) {
			move(5, 0);
			// 取消删除行动\n
			prints("\xC8\xA1\xCF\xFB\xC9\xBE\xB3\xFD\xD0\xD0\xB6\xAF\n");
			pressreturn();
			clear();
			return 0;
		}
		{
			char secu[STRLEN];
			// 删除系统档案：%s
			sprintf(secu, "\xC9\xBE\xB3\xFD\xCF\xB5\xCD\xB3\xB5\xB5\xB0\xB8\xA3\xBA%s", explain_file[ch]);
			securityreport(secu, secu);
		}
		unlink(buf2);
		move(5, 0);
		// %s 已删除\n
		prints("%s \xD2\xD1\xC9\xBE\xB3\xFD\n", explain_file[ch]);
		pressreturn();
		clear();
		return 0;
	}
	modify_user_mode(EDITSFILE);
	aborted = vedit(buf2, NA, YEA);
	clear();
	if (aborted != -1) {
		// %s 更新过
		prints("%s \xB8\xFC\xD0\xC2\xB9\xFD", explain_file[ch]);
		{
			char secu[STRLEN];
			// 修改系统档案：%s
			sprintf(secu, "\xD0\xDE\xB8\xC4\xCF\xB5\xCD\xB3\xB5\xB5\xB0\xB8\xA3\xBA%s", explain_file[ch]);
			securityreport(secu, secu);
		}
	}
	pressreturn();
	return 0;
}

int x_lockscreen(const char *s) {
	(void) s;
	char buf[PASSLEN + 1];
	time_t now;
	modify_user_mode(LOCKSCREEN);
	block_msg();
	move(9, 0);
	clrtobot();
	update_endline();
	buf[0] = '\0';
	char ubuf[3];
	char user_self[16];
	now = time(0);
	move(9, 0);
	prints(
			"\n\033[1;37m       _       _____   ___     _   _   ___     ___       __"
			"\n      ( )     (  _  ) (  _`\\  ( ) ( ) (  _`\\  (  _`\\    |  |"
			"\n      | |     | ( ) | | ( (_) | |/'/' | (_(_) | | ) |   |  |"
			"\n      | |  _  | | | | | |  _  | , <   |  _)_  | | | )   |  |"
			"\n      | |_( ) | (_) | | (_( ) | |\\`\\  | (_( ) | |_) |   |==|"
			"\n      (____/' (_____) (____/' (_) (_) (____/' (____/'   |__|\033[m\n");
	move(17,0);
	// 锁屏理由? (1)吃饭去了 (2)和MM聊天 (3)别来烦我 (4)没理由 (5)自定义:
	getdata(17,0,"\xCB\xF8\xC6\xC1\xC0\xED\xD3\xC9? (1)\xB3\xD4\xB7\xB9\xC8\xA5\xC1\xCB (2)\xBA\xCDMM\xC1\xC4\xCC\xEC (3)\xB1\xF0\xC0\xB4\xB7\xB3\xCE\xD2 (4)\xC3\xBB\xC0\xED\xD3\xC9 (5)\xD7\xD4\xB6\xA8\xD2\xE5:",ubuf,3,DOECHO,YEA); //add by landefeng@BMY for 锁屏理由
	switch(ubuf[0]){
		case '1':
			modify_user_mode(USERDF1);break;
		case '2':
			modify_user_mode(USERDF2);break;
		case '3':
			modify_user_mode(USERDF3);break;
		case '5':						//add by leoncom@bmy 自定义锁屏理由
			move(17,0);
			clrtobot();
			update_endline();
			if(HAS_PERM(PERM_SELFLOCK, currentuser)) {
				uinfo.user_state_temp[0]='\0';                  //清除上次记录
				update_ulist(&uinfo,utmpent);
				// 请输入自定义理由:
				getdata(17,0,"\xC7\xEB\xCA\xE4\xC8\xEB\xD7\xD4\xB6\xA8\xD2\xE5\xC0\xED\xD3\xC9:",user_self,9,DOECHO,YEA);
				int i=0,flag=0;
				for(i=0;i<=7;i++){
					if(user_self[i]==' ')
					{
						flag=1;
						break;
					}
				}
				if ((stringfilter(user_self,0) == YTHT_SMTH_FILTER_RESULT_SAFE) && !flag)
				{
					strcpy(uinfo.user_state_temp,user_self);
					update_ulist(&uinfo, utmpent);
					move(17,0);
					clrtobot();
					// 您的当前锁屏理由为:%s
					prints("\xC4\xFA\xB5\xC4\xB5\xB1\xC7\xB0\xCB\xF8\xC6\xC1\xC0\xED\xD3\xC9\xCE\xAA:%s",user_self);
					modify_user_mode(USERDF4);break;
				}
				else
				{
					move(17,0);
					clrtobot();
					// 您输入的自定义理由含有不合适词汇或特殊字符，将以默认方式锁屏
					prints("\xC4\xFA\xCA\xE4\xC8\xEB\xB5\xC4\xD7\xD4\xB6\xA8\xD2\xE5\xC0\xED\xD3\xC9\xBA\xAC\xD3\xD0\xB2\xBB\xBA\xCF\xCA\xCA\xB4\xCA\xBB\xE3\xBB\xF2\xCC\xD8\xCA\xE2\xD7\xD6\xB7\xFB\xA3\xAC\xBD\xAB\xD2\xD4\xC4\xAC\xC8\xCF\xB7\xBD\xCA\xBD\xCB\xF8\xC6\xC1");
					modify_user_mode(LOCKSCREEN);
					break;
				}
			} else {
				move(17,0);
				clrtobot();
				// 你被管理员取消自定义锁屏的权限，将以默认方式锁定
				prints("\xC4\xE3\xB1\xBB\xB9\xDC\xC0\xED\xD4\xB1\xC8\xA1\xCF\xFB\xD7\xD4\xB6\xA8\xD2\xE5\xCB\xF8\xC6\xC1\xB5\xC4\xC8\xA8\xCF\xDE\xA3\xAC\xBD\xAB\xD2\xD4\xC4\xAC\xC8\xCF\xB7\xBD\xCA\xBD\xCB\xF8\xB6\xA8");
				modify_user_mode(LOCKSCREEN);
				break;
			}
		default:
			// 默认锁屏
			prints("\xC4\xAC\xC8\xCF\xCB\xF8\xC6\xC1");
			modify_user_mode(LOCKSCREEN);
	}
	move(18,0);
	clrtobot();
	// \033[1;36m荧幕已在\033[33m %19s\033[36m 时被\033[32m %-12s \033[36m暂时锁住了...\033[m
	prints("\033[1;36m\xD3\xAB\xC4\xBB\xD2\xD1\xD4\xDA\033[33m %19s\033[36m \xCA\xB1\xB1\xBB\033[32m %-12s \033[36m\xD4\xDD\xCA\xB1\xCB\xF8\xD7\xA1\xC1\xCB...\033[m", ctime(&now), currentuser.userid);
	while (*buf == '\0' || !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
		move(19, 0);
		clrtobot();
		update_endline();
		// 请输入你的密码以解锁:
		getdata(19, 0, buf[0] == '\0' ? "\xC7\xEB\xCA\xE4\xC8\xEB\xC4\xE3\xB5\xC4\xC3\xDC\xC2\xEB\xD2\xD4\xBD\xE2\xCB\xF8: " :
			// 你输入的密码有误，请重新输入你的密码以解锁:
			"\xC4\xE3\xCA\xE4\xC8\xEB\xB5\xC4\xC3\xDC\xC2\xEB\xD3\xD0\xCE\xF3\xA3\xAC\xC7\xEB\xD6\xD8\xD0\xC2\xCA\xE4\xC8\xEB\xC4\xE3\xB5\xC4\xC3\xDC\xC2\xEB\xD2\xD4\xBD\xE2\xCB\xF8: ", buf,
			PASSLEN, NOECHO, YEA);
	}
	unblock_msg();
	return 0;
}

int
heavyload(float maxload)
{
	double cpu_load[3];
	if (maxload == 0)
		maxload = 15;
	get_load(cpu_load);
	if (cpu_load[0] > maxload)
		return 1;
	else
		return 0;
}

static void
myexec_cmd(int umode, int pager, const char *cmdfile, const char *param)
{
	char buf[STRLEN * 8], param1[256];
	int save_pager;
	pid_t childpid;
	int p[2];
	param1[0] = 0;
	if (param != NULL) {
		char *avoid = "&;!`'\"|?~<>^()[]{}$\n\r\\", *ptr;
		int n = strlen(avoid);
		ytht_strsncpy(param1, param, sizeof(param1));
		while (n > 0) {
			n--;
			ptr = strchr(param1, avoid[n]);
			if (ptr != NULL)
				*ptr = 0;
		}
	}

	if (!HAS_PERM(PERM_SYSOP, currentuser) && heavyload(0)) {
		clear();
		// 抱歉，目前系统负荷过重，此功能暂时不能执行...
		prints("\xB1\xA7\xC7\xB8\xA3\xAC\xC4\xBF\xC7\xB0\xCF\xB5\xCD\xB3\xB8\xBA\xBA\xC9\xB9\xFD\xD6\xD8\xA3\xAC\xB4\xCB\xB9\xA6\xC4\xDC\xD4\xDD\xCA\xB1\xB2\xBB\xC4\xDC\xD6\xB4\xD0\xD0...");
		pressanykey();
		return;
	}

	if (!dashf(cmdfile)) {
		move(2, 0);
		prints("no %s\n", cmdfile);
		pressreturn();
		return;
	}

	save_pager = uinfo.pager;
	if (pager == NA) {
		uinfo.pager = 0;
	}

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, p) < 0)
		return;

	modify_user_mode(umode);
	refresh();
	signal(SIGALRM, SIG_IGN);
	signal(SIGCHLD, SIG_DFL);
	childpid = fork();
	if (childpid == 0) {
		char pidstr[20];
		sprintf(pidstr, "%d", getppid());
		close(p[0]);
		if (p[1] != 0)
			dup2(p[1], 0);
		dup2(0, 1);
		dup2(0, 2);
		if (param1[0]) {
			snprintf(buf, sizeof (buf),
				"%s exec %s \"%s\" %s %s %d",
				currentuser.userid, cmdfile, param1,
				currentuser.userid, uinfo.from, getppid());
			newtrace(buf);
			execl(cmdfile, cmdfile, param1, currentuser.userid, uinfo.from, pidstr, NULL);
		} else {
			snprintf(buf, sizeof (buf), "%s exec %s %s %s %d",
				currentuser.userid, cmdfile,
				currentuser.userid, uinfo.from, getppid());
			newtrace(buf);
			execl(cmdfile, cmdfile, currentuser.userid, uinfo.from, pidstr, NULL);
		}
		exit(0);
	} else if (childpid > 0) {
		close(p[1]);
		datapipefd(0, p[0]);
		close(p[0]);
		while (wait(NULL) != childpid)
			sleep(1);
	} else {
		close(p[0]);
		close(p[1]);
	}
	uinfo.pager = save_pager;
	signal(SIGCHLD, SIG_IGN);
	return;
}

static void
datapipefd(int fds, int fdn)
{
	fd_set rs;
	int retv, max;
	char buf[1024];

	max = 1 + ((fdn > fds) ? fdn : fds);
	FD_ZERO(&rs);
	while (1) {
		FD_SET(fds, &rs);
		FD_SET(fdn, &rs);
		retv = select(max, &rs, NULL, NULL, NULL);
		if (retv < 0) {
			if (errno != EINTR)
				break;
			continue;
		}
		if (FD_ISSET(fds, &rs)) {
#ifdef SSHBBS
			retv = ssh_read(fds, buf, sizeof (buf));
#else
			retv = read(fds, buf, sizeof (buf));
#endif
			if (retv > 0) {
				time(&now_t);
				uinfo.lasttime = now_t;
				if ((unsigned long) now_t - (unsigned long) old > 60)
					update_utmp();
				write(fdn, buf, retv);
			} else if (retv == 0 || (retv < 0 && errno != EINTR))
				break;
			FD_CLR(fds, &rs);
		}
		if (FD_ISSET(fdn, &rs)) {
			retv = read(fdn, buf, sizeof (buf));
			if (retv > 0) {
#ifdef SSHBBS
				ssh_write(fds, buf, retv);
#else
				write(fds, buf, retv);
#endif
			} else if (retv == 0 || (retv < 0 && errno != EINTR))
				break;
			FD_CLR(fdn, &rs);
		}
	}
}

int sendgoodwish(const char *uid) {
	(void) uid;
	return sendGoodWish(NULL);
}

static int
sendGoodWish(char *userid)
{
	FILE *fp, *mp;
	int tuid, i, count;
	time_t now;
	char buf[5][STRLEN], tmpbuf[STRLEN], filebuf[STRLEN];
	char uid[IDLEN + 1], *timestr, ans[8], uident[13], tmp[3];
	int cnt, n, fmode = NA;
	char wishlists[STRLEN];

	modify_user_mode(GOODWISH);
	clear();
	move(1, 0);
	// \033[0;1;32m留言本\033[m\n您可以在这里给您的朋友送去您的祝福，
	prints("\033[0;1;32m\xC1\xF4\xD1\xD4\xB1\xBE\033[m\n\xC4\xFA\xBF\xC9\xD2\xD4\xD4\xDA\xD5\xE2\xC0\xEF\xB8\xF8\xC4\xFA\xB5\xC4\xC5\xF3\xD3\xD1\xCB\xCD\xC8\xA5\xC4\xFA\xB5\xC4\xD7\xA3\xB8\xA3\xA3\xAC");
	// \n也可以为您给他/她捎上一句悄悄话。
	prints("\n\xD2\xB2\xBF\xC9\xD2\xD4\xCE\xAA\xC4\xFA\xB8\xF8\xCB\xFB/\xCB\xFD\xC9\xD3\xC9\xCF\xD2\xBB\xBE\xE4\xC7\xC4\xC7\xC4\xBB\xB0\xA1\xA3");
	move(6, 0);

	if (userid == NULL) {
		getdata(3, 0,
			// (1)给个人送去祝福 (2)给一群人送去祝福 (0) 取消 [1]
			"(1)\xB8\xF8\xB8\xF6\xC8\xCB\xCB\xCD\xC8\xA5\xD7\xA3\xB8\xA3 (2)\xB8\xF8\xD2\xBB\xC8\xBA\xC8\xCB\xCB\xCD\xC8\xA5\xD7\xA3\xB8\xA3 (0) \xC8\xA1\xCF\xFB [1]",
			ans, 2, DOECHO, YEA);
		if (ans[0] == '0') {
			clear();
			return 0;
		}
		if (ans[0] != '2') {
			// 请输入他的 ID:
			usercomplete("\xC7\xEB\xCA\xE4\xC8\xEB\xCB\xFB\xB5\xC4 ID: ", uid);
			if (uid[0] == '\0') {
				clear();
				return 0;
			}
		} else {
			clear();
			sethomefile_s(wishlists, sizeof(wishlists), currentuser.userid, "wishlist");
			cnt = listfilecontent(wishlists);
			while (1) {
				getdata(0, 0,
					// (A)增加 (D)删除 (I)引入好友 (C)清除目前名单 (E)放弃 (S)送出?[S]：
					"(A)\xD4\xF6\xBC\xD3 (D)\xC9\xBE\xB3\xFD (I)\xD2\xFD\xC8\xEB\xBA\xC3\xD3\xD1 (C)\xC7\xE5\xB3\xFD\xC4\xBF\xC7\xB0\xC3\xFB\xB5\xA5 (E)\xB7\xC5\xC6\xFA (S)\xCB\xCD\xB3\xF6?[S]\xA3\xBA ",
					tmp, 2, DOECHO, YEA);
				if (tmp[0] == '\n' || tmp[0] == '\0' || tmp[0] == 's' || tmp[0] == 'S') {
					clear();
					break;
				}
				if (tmp[0] == 'a' || tmp[0] == 'd' || tmp[0] == 'A' || tmp[0] == 'D') {
					move(1, 0);
					if (tmp[0] == 'a' || tmp[0] == 'A')
						// 请依次输入使用者代号(只按 ENTER 结束输入):
						usercomplete("\xC7\xEB\xD2\xC0\xB4\xCE\xCA\xE4\xC8\xEB\xCA\xB9\xD3\xC3\xD5\xDF\xB4\xFA\xBA\xC5(\xD6\xBB\xB0\xB4 ENTER \xBD\xE1\xCA\xF8\xCA\xE4\xC8\xEB): ", uident);
					else
						// 请依次输入使用者代号(只按 ENTER 结束输入):
						namecomplete("\xC7\xEB\xD2\xC0\xB4\xCE\xCA\xE4\xC8\xEB\xCA\xB9\xD3\xC3\xD5\xDF\xB4\xFA\xBA\xC5(\xD6\xBB\xB0\xB4 ENTER \xBD\xE1\xCA\xF8\xCA\xE4\xC8\xEB): ", uident);
					move(1, 0);
					clrtoeol();
					if (uident[0] == '\0')
						continue;
					if (!getuser(uident)) {
						move(2, 0);
						// 这个使用者代号是错误的.\n
						prints("\xD5\xE2\xB8\xF6\xCA\xB9\xD3\xC3\xD5\xDF\xB4\xFA\xBA\xC5\xCA\xC7\xB4\xED\xCE\xF3\xB5\xC4.\n");
					}
				}
				switch (tmp[0]) {
				case 'A':
				case 'a':
					if (seek_in_file(wishlists, uident)) {
						move(2, 0);
						// 已经列为收祝福人之一 \n
						prints("\xD2\xD1\xBE\xAD\xC1\xD0\xCE\xAA\xCA\xD5\xD7\xA3\xB8\xA3\xC8\xCB\xD6\xAE\xD2\xBB \n");
						break;
					}
					ytht_add_to_file(wishlists, uident);
					cnt++;
					break;
				case 'E':
				case 'e':
				case 'Q':
				case 'q':
					cnt = 0;
					break;
				case 'D':
				case 'd':
					if (seek_in_file(wishlists, uident)) {
						ytht_del_from_file(wishlists, uident, true);
						cnt--;
					}
					break;
				case 'I':
				case 'i':
					n = 0;
					clear();
					for (i = cnt; n < uinfo.fnum; i++) {
						int key;
						move(2, 0);
						ythtbbs_cache_UserTable_getuserid(uinfo.friend[n], uident, sizeof(uident));
						prints("%s\n", uident);
						move(3, 0);
						n++;
						// (A)全部加入 (Y)加入 (N)不加入 (Q)结束? [Y]:
						prints("(A)\xC8\xAB\xB2\xBF\xBC\xD3\xC8\xEB (Y)\xBC\xD3\xC8\xEB (N)\xB2\xBB\xBC\xD3\xC8\xEB (Q)\xBD\xE1\xCA\xF8? [Y]:");
						if (!fmode)
							key = igetkey();
						else
							key = 'Y';
						if (key == 'q' || key == 'Q')
							break;
						if (key == 'A' || key == 'a') {
							fmode = YEA;
							key = 'Y';
						}
						if (key == '\0' || key == '\n' || key == 'y' || key == 'Y') {
							if (!getuser(uident)) {
								move(4, 0);
								// 这个使用者代号是错误的.\n
								prints("\xD5\xE2\xB8\xF6\xCA\xB9\xD3\xC3\xD5\xDF\xB4\xFA\xBA\xC5\xCA\xC7\xB4\xED\xCE\xF3\xB5\xC4.\n");
								i--;
								continue;
							} else if (seek_in_file(wishlists, uident)) {
								i--;
								continue;
							}
							ytht_add_to_file(wishlists, uident);
							cnt++;
						}
					}	//for loop
					fmode = NA;
					clear();
					break;
				case 'C':
				case 'c':
					unlink(wishlists);
					cnt = 0;
					break;
				}
				if (strchr("EeQq", tmp[0]))
					break;
				move(5, 0);
				clrtobot();
				move(3, 0);
				clrtobot();
				listfilecontent(wishlists);
			}
			if (cnt <= 0)
				return 0;
			move(5, 0);
			clrtoeol();
			// \033[m【请输入您的留言】
			prints("\033[m\xA1\xBE\xC7\xEB\xCA\xE4\xC8\xEB\xC4\xFA\xB5\xC4\xC1\xF4\xD1\xD4\xA1\xBF       ");
			move(6, 0);
			tmpbuf[0] = '\0';
			// 您的留言[直接按 ENTER 结束留言，最多 5 句，每句最长 50 字符]:
			prints("\xC4\xFA\xB5\xC4\xC1\xF4\xD1\xD4[\xD6\xB1\xBD\xD3\xB0\xB4 ENTER \xBD\xE1\xCA\xF8\xC1\xF4\xD1\xD4\xA3\xAC\xD7\xEE\xB6\xE0 5 \xBE\xE4\xA3\xAC\xC3\xBF\xBE\xE4\xD7\xEE\xB3\xA4 50 \xD7\xD6\xB7\xFB]:");
			for (count = 0; count < 5; count++) {
				getdata(7 + count, 0, ": ", tmpbuf, 51, DOECHO, YEA);
				if (tmpbuf[0] == '\0')
					break;;
				strcpy(buf[count], tmpbuf);
				tmpbuf[0] = '\0';
			}
			if (count == 0)
				return 0;
			// 你确定要发送这条留言吗
			sprintf(genbuf, "\xC4\xE3\xC8\xB7\xB6\xA8\xD2\xAA\xB7\xA2\xCB\xCD\xD5\xE2\xCC\xF5\xC1\xF4\xD1\xD4\xC2\xF0");
			move(9 + count, 0);
			if (askyn(genbuf, YEA, NA) == NA) {
				clear();
				return 0;
			}
			sethomefile_s(wishlists, sizeof(wishlists), currentuser.userid, "wishlist");
			if ((mp = fopen(wishlists, "r")) == NULL) {
				return -3;
			}
			for (n = 0; n < cnt; n++) {
				memset(uid, 0, sizeof uid);
				if (fgets(filebuf, STRLEN, mp) != NULL) {
					if (strtok(filebuf, " \n\r\t") != NULL)
						ytht_strsncpy(uid, filebuf, sizeof uid);
					else
						continue;
				}
				if (uid[0] == '\0') {
					continue;
				}
				sethomefile_s(genbuf, sizeof(genbuf), uid, "GoodWish");
				if ((fp = fopen(genbuf, "a")) == NULL) {
					// 无法开启该用户的底部流动信息文件，请通知站长...\n
					prints("\xCE\xDE\xB7\xA8\xBF\xAA\xC6\xF4\xB8\xC3\xD3\xC3\xBB\xA7\xB5\xC4\xB5\xD7\xB2\xBF\xC1\xF7\xB6\xAF\xD0\xC5\xCF\xA2\xCE\xC4\xBC\xFE\xA3\xAC\xC7\xEB\xCD\xA8\xD6\xAA\xD5\xBE\xB3\xA4...\n");
					pressanykey();
					fclose(mp);
					return NA;
				}
				now = time(0);
				timestr = ctime(&now) + 11;
				*(timestr + 5) = '\0';
				for (i = 0; i < count; i++) {
					// %s(%s)[%d/%d]：%s\n
					fprintf(fp, "%s(%s)[%d/%d]\xA3\xBA%s\n",
						currentuser.userid, timestr,
						i + 1, count, buf[i]);
				}
				fclose(fp);
				sethomefile_s(genbuf, sizeof(genbuf), uid, "HaveNewWish");
				if ((fp = fopen(genbuf, "w+")) != NULL) {
					fputs("Have New Wish", fp);
					fclose(fp);
				}
				move(11 + count, 0);
				// 已经帮您送出您的留言了。\n
				//prints("\xD2\xD1\xBE\xAD\xB0\xEF\xC4\xFA\xCB\xCD\xB3\xF6\xC4\xFA\xB5\xC4\xC1\xF4\xD1\xD4\xC1\xCB\xA1\xA3\n");
				sprintf(genbuf, "%s sendgoodwish %s",
					currentuser.userid, uid);
				newtrace(genbuf);
			}	//for loop
			fclose(mp);
			return 0;
		}
	} else
		ytht_strsncpy(uid, userid, sizeof uid);
	if (!(tuid = getuser(uid))) {
		move(7, 0);
		// \033[1m您输入的使用者代号( ID )不存在！\033[m\n
		prints("\033[1m\xC4\xFA\xCA\xE4\xC8\xEB\xB5\xC4\xCA\xB9\xD3\xC3\xD5\xDF\xB4\xFA\xBA\xC5( ID )\xB2\xBB\xB4\xE6\xD4\xDA\xA3\xA1\033[m\n");
		pressanykey();
		clear();
		return -1;
	}
	move(5, 0);
	clrtoeol();
	// \033[m【给 \033[1m%s\033[m 留言】
	prints("\033[m\xA1\xBE\xB8\xF8 \033[1m%s\033[m \xC1\xF4\xD1\xD4\xA1\xBF       ", uid);
	move(6, 0);
	tmpbuf[0] = '\0';
	// 您的留言[直接按 ENTER 结束留言，最多 5 句，每句最长 50 字符]:
	prints("\xC4\xFA\xB5\xC4\xC1\xF4\xD1\xD4[\xD6\xB1\xBD\xD3\xB0\xB4 ENTER \xBD\xE1\xCA\xF8\xC1\xF4\xD1\xD4\xA3\xAC\xD7\xEE\xB6\xE0 5 \xBE\xE4\xA3\xAC\xC3\xBF\xBE\xE4\xD7\xEE\xB3\xA4 50 \xD7\xD6\xB7\xFB]:");
	for (count = 0; count < 5; count++) {
		getdata(7 + count, 0, ": ", tmpbuf, 51, DOECHO, YEA);
		if (tmpbuf[0] == '\0')
			break;;
		strcpy(buf[count], tmpbuf);
		tmpbuf[0] = '\0';
	}
	if (count == 0)
		return 0;

	// 你确定要发送这条留言给 \033[1m%s\033[m 吗
	sprintf(genbuf, "\xC4\xE3\xC8\xB7\xB6\xA8\xD2\xAA\xB7\xA2\xCB\xCD\xD5\xE2\xCC\xF5\xC1\xF4\xD1\xD4\xB8\xF8 \033[1m%s\033[m \xC2\xF0", uid);
	move(9 + count, 0);
	if (askyn(genbuf, YEA, NA) == NA) {
		clear();
		return 0;
	}
	sethomefile_s(genbuf, sizeof(genbuf), uid, "GoodWish");
	if ((fp = fopen(genbuf, "a")) == NULL) {
		// 无法开启该用户的底部流动信息文件，请通知站长...\n
		prints("\xCE\xDE\xB7\xA8\xBF\xAA\xC6\xF4\xB8\xC3\xD3\xC3\xBB\xA7\xB5\xC4\xB5\xD7\xB2\xBF\xC1\xF7\xB6\xAF\xD0\xC5\xCF\xA2\xCE\xC4\xBC\xFE\xA3\xAC\xC7\xEB\xCD\xA8\xD6\xAA\xD5\xBE\xB3\xA4...\n");
		pressanykey();
		return NA;
	}
	now = time(0);
	timestr = ctime(&now) + 11;
	*(timestr + 5) = '\0';
	for (i = 0; i < count; i++) {
		// %s(%s)[%d/%d]：%s\n
		fprintf(fp, "%s(%s)[%d/%d]\xA3\xBA%s\n",
			currentuser.userid, timestr, i + 1, count, buf[i]);
	}
	fclose(fp);
	sethomefile_s(genbuf, sizeof(genbuf), uid, "HaveNewWish");
	if ((fp = fopen(genbuf, "w+")) != NULL) {
		fputs("Have New Wish", fp);
		fclose(fp);
	}
	move(11 + count, 0);
	// 已经帮您送出您的留言了。\n
	prints("\xD2\xD1\xBE\xAD\xB0\xEF\xC4\xFA\xCB\xCD\xB3\xF6\xC4\xFA\xB5\xC4\xC1\xF4\xD1\xD4\xC1\xCB\xA1\xA3\n");
	sprintf(genbuf, "%s sendgoodwish %s", currentuser.userid, uid);
	newtrace(genbuf);
	// 把这条祝福发送给其他人么?
	/*sprintf(genbuf,"\xB0\xD1\xD5\xE2\xCC\xF5\xD7\xA3\xB8\xA3\xB7\xA2\xCB\xCD\xB8\xF8\xC6\xE4\xCB\xFB\xC8\xCB\xC3\xB4?");
	if(askyn(genbuf,YEA,NA)==YEA){
		// 请输入他的 ID:
		usercomplete("\xC7\xEB\xCA\xE4\xC8\xEB\xCB\xFB\xB5\xC4 ID: ", uid);
		if (uid[0] == '\0') {
			clear();
			return 0;
		}
		if (!(tuid = getuser(uid))) {
			move(7, 0);
			// \x1b[1m您输入的使用者代号( ID )不存在！\x1b[m\n
			prints("\x1b[1m\xC4\xFA\xCA\xE4\xC8\xEB\xB5\xC4\xCA\xB9\xD3\xC3\xD5\xDF\xB4\xFA\xBA\xC5( ID )\xB2\xBB\xB4\xE6\xD4\xDA\xA3\xA1\x1b[m\n");
			pressanykey();
			clear();
			return -1;
		}
		goto loop;
	} */
	pressanykey();
	clear();
	return 0;
}

static void
childreturn(int i)
{
	(void) i;
	int retv;
	while ((retv = waitpid(-1, NULL, WNOHANG | WUNTRACED)) > 0)
		if (childpid > 0 && retv == childpid)
			childpid = 0;
}

int ent_bnet(const char *cmd) {
	int p[2];
#ifdef SSHBBS
	move(9, 0);
	clrtobot();
	// 您目前是使用 ssh 方式连接 %s.\n
	prints("\xC4\xFA\xC4\xBF\xC7\xB0\xCA\xC7\xCA\xB9\xD3\xC3 ssh \xB7\xBD\xCA\xBD\xC1\xAC\xBD\xD3 %s.\n"
			// ssh 会对网络数据传输进行加密,保护您的密码和其它私人信息.\n
			"ssh \xBB\xE1\xB6\xD4\xCD\xF8\xC2\xE7\xCA\xFD\xBE\xDD\xB4\xAB\xCA\xE4\xBD\xF8\xD0\xD0\xBC\xD3\xC3\xDC,\xB1\xA3\xBB\xA4\xC4\xFA\xB5\xC4\xC3\xDC\xC2\xEB\xBA\xCD\xC6\xE4\xCB\xFC\xCB\xBD\xC8\xCB\xD0\xC5\xCF\xA2.\n"
			// 您必须知道,网络穿梭是通过本站连接到其它bbs,虽然从您的机器\n
			"\xC4\xFA\xB1\xD8\xD0\xEB\xD6\xAA\xB5\xC0,\xCD\xF8\xC2\xE7\xB4\xA9\xCB\xF3\xCA\xC7\xCD\xA8\xB9\xFD\xB1\xBE\xD5\xBE\xC1\xAC\xBD\xD3\xB5\xBD\xC6\xE4\xCB\xFCbbs,\xCB\xE4\xC8\xBB\xB4\xD3\xC4\xFA\xB5\xC4\xBB\xFA\xC6\xF7\n"
			// 到本站间的数据传输是加密的,但是从本站到另外的 BBS 站点间的\n
			"\xB5\xBD\xB1\xBE\xD5\xBE\xBC\xE4\xB5\xC4\xCA\xFD\xBE\xDD\xB4\xAB\xCA\xE4\xCA\xC7\xBC\xD3\xC3\xDC\xB5\xC4,\xB5\xAB\xCA\xC7\xB4\xD3\xB1\xBE\xD5\xBE\xB5\xBD\xC1\xED\xCD\xE2\xB5\xC4 BBS \xD5\xBE\xB5\xE3\xBC\xE4\xB5\xC4\n"
			// 数据传输并没有加密,您的密码和其它私人信息有被第三方窃听的\n可能.\n
			"\xCA\xFD\xBE\xDD\xB4\xAB\xCA\xE4\xB2\xA2\xC3\xBB\xD3\xD0\xBC\xD3\xC3\xDC,\xC4\xFA\xB5\xC4\xC3\xDC\xC2\xEB\xBA\xCD\xC6\xE4\xCB\xFC\xCB\xBD\xC8\xCB\xD0\xC5\xCF\xA2\xD3\xD0\xB1\xBB\xB5\xDA\xC8\xFD\xB7\xBD\xC7\xD4\xCC\xFD\xB5\xC4\n\xBF\xC9\xC4\xDC.\n",
			MY_BBS_NAME);
	// 你确定要使用网络穿梭吗
	if (askyn("\xC4\xE3\xC8\xB7\xB6\xA8\xD2\xAA\xCA\xB9\xD3\xC3\xCD\xF8\xC2\xE7\xB4\xA9\xCB\xF3\xC2\xF0", NA, NA) != 1)
		return -1;
#endif
	signal(SIGALRM, SIG_IGN);
	signal(SIGCHLD, childreturn);
	modify_user_mode(BBSNET);
	do_delay(1);
	refresh();
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, p) < 0)
		return -1;
	if (p[0] <= 2 || p[1] <= 2) {
		int i = p[0] + p[1] + 1;
		dup2(p[0], i);
		dup2(p[1], i + 1);
		close(p[0]);
		close(p[1]);
		p[0] = i;
		p[1] = i + 1;
	}
	inBBSNET = 1;
	childpid = fork();
	if (childpid == 0) {
		close(p[1]);
		if (p[0] != 0)
			dup2(p[0], 0);
		dup2(0, 1);
		dup2(0, 2);
		execl("bin/bbsnet", "bbsnet",
				(cmd[0] == 'A') ? "etc/bbsnetA.ini" : "etc/bbsnet.ini",
				"bin/telnet", currentuser.userid, NULL);
		exit(0);
	} else if (childpid > 0) {
		close(p[0]);
		datapipefd(0, p[1]);
		close(p[1]);
	} else {
		close(p[0]);
		close(p[1]);
	}
	signal(SIGCHLD, SIG_IGN);
	inBBSNET = 0;
	redoscr();
//      r_msg();
	return 0;
}

int x_denylevel(const char *s) {
	(void) s;
	int id;
	char ans[7], content[2048];
	int oldlevel;
	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();
	move(0, 0);
	// 更改使用者基本权限\n
	prints("\xB8\xFC\xB8\xC4\xCA\xB9\xD3\xC3\xD5\xDF\xBB\xF9\xB1\xBE\xC8\xA8\xCF\xDE\n");
	clrtoeol();
	move(1, 0);
	// 输入欲更改的使用者帐号:
	usercomplete("\xCA\xE4\xC8\xEB\xD3\xFB\xB8\xFC\xB8\xC4\xB5\xC4\xCA\xB9\xD3\xC3\xD5\xDF\xD5\xCA\xBA\xC5: ", genbuf);
	if (genbuf[0] == '\0') {
		clear();
		return 0;
	}
	if (!(id = getuser(genbuf))) {
		move(3, 0);
		prints("Invalid User Id");
		clrtoeol();
		pressreturn();
		clear();
		return -1;
	}
	move(1, 0);
	clrtobot();
	move(2, 0);
	// 设定使用者 '%s' 的基本权限 \n\n
	prints("\xC9\xE8\xB6\xA8\xCA\xB9\xD3\xC3\xD5\xDF '%s' \xB5\xC4\xBB\xF9\xB1\xBE\xC8\xA8\xCF\xDE \n\n", genbuf);
	// (1) 封禁发表文章权利       (A) 恢复发表文章权利\n
	prints("(1) \xB7\xE2\xBD\xFB\xB7\xA2\xB1\xED\xCE\xC4\xD5\xC2\xC8\xA8\xC0\xFB       (A) \xBB\xD6\xB8\xB4\xB7\xA2\xB1\xED\xCE\xC4\xD5\xC2\xC8\xA8\xC0\xFB\n");
	// (2) 取消基本上站权利       (B) 恢复基本上站权利\n
	prints("(2) \xC8\xA1\xCF\xFB\xBB\xF9\xB1\xBE\xC9\xCF\xD5\xBE\xC8\xA8\xC0\xFB       (B) \xBB\xD6\xB8\xB4\xBB\xF9\xB1\xBE\xC9\xCF\xD5\xBE\xC8\xA8\xC0\xFB\n");
	// (3) 禁止进入聊天室         (C) 恢复进入聊天室权利\n
	prints("(3) \xBD\xFB\xD6\xB9\xBD\xF8\xC8\xEB\xC1\xC4\xCC\xEC\xCA\xD2         (C) \xBB\xD6\xB8\xB4\xBD\xF8\xC8\xEB\xC1\xC4\xCC\xEC\xCA\xD2\xC8\xA8\xC0\xFB\n");
	// (4) 禁止呼叫他人聊天       (D) 恢复呼叫他人聊天权利\n
	prints("(4) \xBD\xFB\xD6\xB9\xBA\xF4\xBD\xD0\xCB\xFB\xC8\xCB\xC1\xC4\xCC\xEC       (D) \xBB\xD6\xB8\xB4\xBA\xF4\xBD\xD0\xCB\xFB\xC8\xCB\xC1\xC4\xCC\xEC\xC8\xA8\xC0\xFB\n");
	// (5) 整理个人精华区         (E) 不能整理个人精华区\n
	prints("(5) \xD5\xFB\xC0\xED\xB8\xF6\xC8\xCB\xBE\xAB\xBB\xAA\xC7\xF8         (E) \xB2\xBB\xC4\xDC\xD5\xFB\xC0\xED\xB8\xF6\xC8\xCB\xBE\xAB\xBB\xAA\xC7\xF8\n");
	// (6) 禁止发送信件           (F) 恢复发信权利\n
	prints("(6) \xBD\xFB\xD6\xB9\xB7\xA2\xCB\xCD\xD0\xC5\xBC\xFE           (F) \xBB\xD6\xB8\xB4\xB7\xA2\xD0\xC5\xC8\xA8\xC0\xFB\n");
	// (7) 禁止使用签名档         (G) 恢复使用签名档权利\n
	prints("(7) \xBD\xFB\xD6\xB9\xCA\xB9\xD3\xC3\xC7\xA9\xC3\xFB\xB5\xB5         (G) \xBB\xD6\xB8\xB4\xCA\xB9\xD3\xC3\xC7\xA9\xC3\xFB\xB5\xB5\xC8\xA8\xC0\xFB\n");
	// (8) 禁止使用自定义锁屏     (H) 恢复使用自定义锁屏\n
	prints("(8) \xBD\xFB\xD6\xB9\xCA\xB9\xD3\xC3\xD7\xD4\xB6\xA8\xD2\xE5\xCB\xF8\xC6\xC1     (H) \xBB\xD6\xB8\xB4\xCA\xB9\xD3\xC3\xD7\xD4\xB6\xA8\xD2\xE5\xCB\xF8\xC6\xC1\n");
	// 请输入你的处理:
	getdata(13, 0, "\xC7\xEB\xCA\xE4\xC8\xEB\xC4\xE3\xB5\xC4\xB4\xA6\xC0\xED: ", ans, 3, DOECHO, YEA);
	oldlevel = lookupuser.userlevel;
	switch (ans[0]) {
	case '1':
		lookupuser.userlevel &= ~PERM_POST;
		break;
	case 'a':
	case 'A':
		lookupuser.userlevel |= PERM_POST;
		break;
	case '2':
		lookupuser.userlevel &= ~PERM_BASIC;
		break;
	case 'b':
	case 'B':
		lookupuser.userlevel |= PERM_BASIC;
		break;
	case '3':
		lookupuser.userlevel &= ~PERM_CHAT;
		break;
	case 'c':
	case 'C':
		lookupuser.userlevel |= PERM_CHAT;
		break;
	case '4':
		lookupuser.userlevel &= ~PERM_PAGE;
		break;
	case 'd':
	case 'D':
		lookupuser.userlevel |= PERM_PAGE;
		break;
	case '5':
		lookupuser.userlevel |= PERM_SPECIAL8;
		break;
	case 'e':
	case 'E':
		lookupuser.userlevel &= ~PERM_SPECIAL8;
		break;
	case '6':
		lookupuser.userlevel |= PERM_DENYMAIL;
		break;
	case 'f':
	case 'F':
		lookupuser.userlevel &= ~PERM_DENYMAIL;
		break;
	case '7':
		lookupuser.userlevel |= PERM_DENYSIG;
		{
			// 禁止使用签名档的原因：
			getdata(13, 0, "\xBD\xFB\xD6\xB9\xCA\xB9\xD3\xC3\xC7\xA9\xC3\xFB\xB5\xB5\xB5\xC4\xD4\xAD\xD2\xF2\xA3\xBA", genbuf, 40,
				DOECHO, YEA);
			sprintf(content,
				// 您被禁止使用签名档，原因是：\n    %s\n\n
				"\xC4\xFA\xB1\xBB\xBD\xFB\xD6\xB9\xCA\xB9\xD3\xC3\xC7\xA9\xC3\xFB\xB5\xB5\xA3\xAC\xD4\xAD\xD2\xF2\xCA\xC7\xA3\xBA\n    %s\n\n"
				// (如果是因为签名档图片大小超，请参阅 Announce
				"(\xC8\xE7\xB9\xFB\xCA\xC7\xD2\xF2\xCE\xAA\xC7\xA9\xC3\xFB\xB5\xB5\xCD\xBC\xC6\xAC\xB4\xF3\xD0\xA1\xB3\xAC\xA3\xAC\xC7\xEB\xB2\xCE\xD4\xC4 Announce "
				// 版的公告<<关于图片签名档的大小限制>>\n
				"\xB0\xE6\xB5\xC4\xB9\xAB\xB8\xE6<<\xB9\xD8\xD3\xDA\xCD\xBC\xC6\xAC\xC7\xA9\xC3\xFB\xB5\xB5\xB5\xC4\xB4\xF3\xD0\xA1\xCF\xDE\xD6\xC6>>\n"
				"http://ytht.net/Ytht.Net/bbscon?B=Announce&F=M.1047666649.A )",
				genbuf);
			// 您被禁止使用签名档
			mail_buf(content, lookupuser.userid, "\xC4\xFA\xB1\xBB\xBD\xFB\xD6\xB9\xCA\xB9\xD3\xC3\xC7\xA9\xC3\xFB\xB5\xB5");
		}
		break;
	case 'g':
	case 'G':
		lookupuser.userlevel &= ~PERM_DENYSIG;
		break;
	case '8':
		lookupuser.userlevel &= ~PERM_SELFLOCK;
		{
			// 禁止使用自定义锁屏的原因：
			getdata(13, 0, "\xBD\xFB\xD6\xB9\xCA\xB9\xD3\xC3\xD7\xD4\xB6\xA8\xD2\xE5\xCB\xF8\xC6\xC1\xB5\xC4\xD4\xAD\xD2\xF2\xA3\xBA", genbuf, 40,
				DOECHO, YEA);
			sprintf(content,
				// 您被禁止使用自定义锁屏，原因如下:\n\n   %s\n
				"\xC4\xFA\xB1\xBB\xBD\xFB\xD6\xB9\xCA\xB9\xD3\xC3\xD7\xD4\xB6\xA8\xD2\xE5\xCB\xF8\xC6\xC1\xA3\xAC\xD4\xAD\xD2\xF2\xC8\xE7\xCF\xC2:\n\n   %s\n",
				genbuf);
			// 您被禁止使用自定义锁屏
			mail_buf(content, lookupuser.userid, "\xC4\xFA\xB1\xBB\xBD\xFB\xD6\xB9\xCA\xB9\xD3\xC3\xD7\xD4\xB6\xA8\xD2\xE5\xCB\xF8\xC6\xC1");
		}
		break;
	case 'h':
	case 'H':
		lookupuser.userlevel |= PERM_SELFLOCK;
		// 您的自定义锁屏封禁已经被解除，重新登录后可以继续使用
		sprintf(content,"\xC4\xFA\xB5\xC4\xD7\xD4\xB6\xA8\xD2\xE5\xCB\xF8\xC6\xC1\xB7\xE2\xBD\xFB\xD2\xD1\xBE\xAD\xB1\xBB\xBD\xE2\xB3\xFD\xA3\xAC\xD6\xD8\xD0\xC2\xB5\xC7\xC2\xBC\xBA\xF3\xBF\xC9\xD2\xD4\xBC\xCC\xD0\xF8\xCA\xB9\xD3\xC3");
		// 您可以使用自定义锁屏
		mail_buf(content,lookupuser.userid,"\xC4\xFA\xBF\xC9\xD2\xD4\xCA\xB9\xD3\xC3\xD7\xD4\xB6\xA8\xD2\xE5\xCB\xF8\xC6\xC1");
		break;
	default:
		// \n 使用者 '%s' 基本权利没有变更\n
		prints("\n \xCA\xB9\xD3\xC3\xD5\xDF '%s' \xBB\xF9\xB1\xBE\xC8\xA8\xC0\xFB\xC3\xBB\xD3\xD0\xB1\xE4\xB8\xFC\n", lookupuser.userid);
		pressreturn();
		clear();
		return 0;
	}
	{
		char secu[STRLEN];
		// 修改 %s 的基本权限
		sprintf(secu, "\xD0\xDE\xB8\xC4 %s \xB5\xC4\xBB\xF9\xB1\xBE\xC8\xA8\xCF\xDE", lookupuser.userid);
		permtostr(oldlevel, genbuf);
		// 修改前的权限：%s\n修改后的权限：
		sprintf(content, "\xD0\xDE\xB8\xC4\xC7\xB0\xB5\xC4\xC8\xA8\xCF\xDE\xA3\xBA%s\n\xD0\xDE\xB8\xC4\xBA\xF3\xB5\xC4\xC8\xA8\xCF\xDE\xA3\xBA", genbuf);
		permtostr(lookupuser.userlevel, genbuf);
		strcat(content, genbuf);
		securityreport(secu, content);
	}

	substitute_record(PASSFILE, &lookupuser, sizeof (struct userec), id);
	clear();
	return 0;
}

int
s_checkid(const char *s)
{
	(void) s;
	char buf[256];
	char checkuser[20];
	int day, id;
	modify_user_mode(GMENU);
	clear();
	// 调查ID发文情况\n
	stand_title("\xB5\xF7\xB2\xE9ID\xB7\xA2\xCE\xC4\xC7\xE9\xBF\xF6\n");
	clrtoeol();
	move(2, 0);
	// 输入欲调查的使用者帐号:
	usercomplete("\xCA\xE4\xC8\xEB\xD3\xFB\xB5\xF7\xB2\xE9\xB5\xC4\xCA\xB9\xD3\xC3\xD5\xDF\xD5\xCA\xBA\xC5: ", genbuf);
	if (genbuf[0] == '\0') {
	clear();
		return 0;
	}
	ytht_strsncpy(checkuser, genbuf, sizeof checkuser);
	if (!(id = getuser(genbuf))) {
		move(4, 0);
		// 无效的使用者帐号
		prints("\xCE\xDE\xD0\xA7\xB5\xC4\xCA\xB9\xD3\xC3\xD5\xDF\xD5\xCA\xBA\xC5");
		clrtoeol();
		pressreturn();
	clear();
		return 0;
	}
	// 输入天数(0-所有时间):
	getdata(5, 0, "\xCA\xE4\xC8\xEB\xCC\xEC\xCA\xFD(0-\xCB\xF9\xD3\xD0\xCA\xB1\xBC\xE4): ", buf, 7, DOECHO, YEA);
	day = atoi(buf);
	sprintf(buf,
		 "/usr/bin/nice " MY_BBS_HOME "/bin/finddf %d %d %s > " MY_BBS_HOME
		 "/bbstmpfs/tmp/checkid.%s 2>/dev/null", currentuser.userlevel,
		 day, checkuser, currentuser.userid);
	if ((HAS_PERM(PERM_SYSOP, currentuser) && heavyload(2.5))
		|| (!HAS_PERM(PERM_SYSOP, currentuser) && heavyload(1.5))) {
		// 系统负载过重, 无法执行本指令
		prints("\xCF\xB5\xCD\xB3\xB8\xBA\xD4\xD8\xB9\xFD\xD6\xD8, \xCE\xDE\xB7\xA8\xD6\xB4\xD0\xD0\xB1\xBE\xD6\xB8\xC1\xEE");
		pressreturn();
		return 1;
	}
	system(buf);
	sprintf(buf, "%s finddf %s %d", currentuser.userid, checkuser, day);
	newtrace(buf);
	sprintf(buf, MY_BBS_HOME "/bbstmpfs/tmp/checkid.%s",
		 currentuser.userid);
	mail_file(buf, currentuser.userid, "\"System Report\"");
	// 完毕
	prints("\xCD\xEA\xB1\xCF");
	clrtoeol();
	pressreturn();
	clear();
	return 1;
}

char *directfile(char *fpath, char *direct, char *filename) {
	char *t;
	strcpy(fpath, direct);
	if ((t = strrchr(fpath, '/')) == NULL)
		exit(0);
	t++;
	strcpy(t, filename);
	return fpath;
}

static void
escape_filename(char *fn)
{
	static const char invalid[] = {
		'/', '\\', '!', '&', '|', '*', '?', '`', '\'', '\"', ';', '<',
		'>', ':', '~', '(', ')', '[', ']', '{', '}', '$', '\n', '\r'
	};
	size_t i, j;

	for (i = 0; i < strlen(fn); i++)
		for (j = 0; j < sizeof (invalid); j++)
			if (fn[i] == invalid[j])
				fn[i] = '_';
}

int
zsend_file(char *from, char *title)
{
	char name[200], name1[200];
	char path[200];
	FILE *fr, *fw;
	char to[512];
	char buf[512], *fn = NULL;
	char attachfile[512];
	char attach_to_send[1024];
	int  isa, base64;
	size_t len;

	ansimore("etc/zmodem", 0);
	move(14, 0);
	len = file_size(from);

	// 此次传输共 %d bytes, 大约耗时 %d 秒（以 5k/s 计算）
	prints("\xB4\xCB\xB4\xCE\xB4\xAB\xCA\xE4\xB9\xB2 %d bytes, \xB4\xF3\xD4\xBC\xBA\xC4\xCA\xB1 %d \xC3\xEB\xA3\xA8\xD2\xD4 5k/s \xBC\xC6\xCB\xE3\xA3\xA9", len, len / ZMODEM_RATE);
	move(t_lines - 1, 0);
	clrtoeol();
	strcpy(name, "N");

	getdata(t_lines - 1, 0,
		// 您确定要使用Zmodem传输文件么?[y/N]
		"\xC4\xFA\xC8\xB7\xB6\xA8\xD2\xAA\xCA\xB9\xD3\xC3Zmodem\xB4\xAB\xCA\xE4\xCE\xC4\xBC\xFE\xC3\xB4?[y/N]", name, 2, DOECHO, YEA);
	if (toupper(name[0]) != 'Y')
		return FULLUPDATE;
	strncpy(name, title, 76);
	name[80] = '\0';
	escape_filename(name);
	move(t_lines - 2, 0);
	clrtoeol();
	// 请输入文件名，为空则放弃
	prints("\xC7\xEB\xCA\xE4\xC8\xEB\xCE\xC4\xBC\xFE\xC3\xFB\xA3\xAC\xCE\xAA\xBF\xD5\xD4\xF2\xB7\xC5\xC6\xFA");
	move(t_lines - 1, 0);
	clrtoeol();
	getdata(t_lines - 1, 0, "", name, 78, DOECHO, 0);
	if (name[0] == '\0')
		return FULLUPDATE;
	name[78] = '\0';
	escape_filename(name);
	sprintf(name1, "YTHT-%s-", currboard);
	strcat(name1, name);
	strcpy(name, name1);
	strcat(name1, ".TXT");
	snprintf(path, sizeof (path), PATHZMODEM "/%s.%d", currentuser.userid, uinfo.pid);
	mkdir(path, 0770);
	sprintf(to, "%s/%s", path, name1);
	fr = fopen(from, "r");
	if (fr == NULL)
		return FULLUPDATE;
	fw = fopen(to, "w");
	if (fw == NULL) {
		fclose(fr);
		return FULLUPDATE;
	}
	while (fgets(buf, sizeof (buf), fr) != NULL) {
		base64 = isa = 0;
		if (!strncmp(buf, "begin 644", 10)) {
			isa = 1;
			base64 = 1;
			fn = buf + 10;
		} else if (checkbinaryattach(buf, fr, &len)) {
			isa = 1;
			base64 = 0;
			fn = buf + 18;
		}
		if (isa) {
			snprintf(attachfile, sizeof(attachfile), "%s-attach-%s",  name, fn);
			if (getattach(fr, attachfile, path, base64, len, 0)) {
				// 附件%s错误\n
				fprintf(fw, "\xB8\xBD\xBC\xFE%s\xB4\xED\xCE\xF3\n", fn);
			} else {
				sprintf(attach_to_send, "%s/%s", path, attachfile);
				bbs_zsendfile(attach_to_send);
				// 附件%s\n
				fprintf(fw, "\xB8\xBD\xBC\xFE%s\n", fn);
			}
		} else
			fputs(buf, fw);
	}
	fclose(fw);
	fclose(fr);
	bbs_zsendfile(to);
	return FULLUPDATE;
}

static void
bbs_zsendfile(char *filename)
{
	if (!dashf(filename))
		return;
	refresh();
	myexec_cmd(READING, NA, "bin/sz.sh", filename);
	unlink(filename);
}

static void
get_load(double load[])
{
#if defined(LINUX)
	FILE *fp;
	fp = fopen("/proc/loadavg", "r");
	if (!fp)
		load[0] = load[1] = load[2] = 0;
	else {
		float av[3];
		fscanf(fp, "%g %g %g", av, av + 1, av + 2);
		fclose(fp);
		load[0] = av[0];
		load[1] = av[1];
		load[2] = av[2];
	}
#elif defined(BSD44)
	getloadavg(load, 3);
#else
	struct statstime rs;
	rstat("localhost", &rs);
	load[0] = rs.avenrun[0] / (double) (1 << 8);
	load[1] = rs.avenrun[1] / (double) (1 << 8);
	load[2] = rs.avenrun[2] / (double) (1 << 8);
#endif
}
