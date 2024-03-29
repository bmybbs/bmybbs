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
#include "ythtbbs/identify.h"
#include "smth_screen.h"
#include "io.h"
#include "stuff.h"
#include "more.h"
#include "main.h"
#include "term.h"
#include "bbsinc.h"
#include "bcache.h"
#include "mail.h"
#include "maintain.h"
#include "bbs_global_vars.h"
#include "bmy/user.h"
/*
#define  EMAIL          0x0001
#define  NICK           0x0002
#define  REALNAME       0x0004
#define  ADDR           0x0008
#define  REALEMAIL      0x0010
#define  BADEMAIL       0x0020
#define  NEWREG         0x0040
*/
char *sysconf_str();
char *genpasswd();

extern char fromhost[60];
extern time_t login_start_time;
extern int g_convcode;

time_t system_time;

static int valid_ident(char *ident);

static int
getnewuserid(struct userec *newuser)
{
	int val, i;
	enum ythtbbs_register_status rc;

	prints("寻找新帐号中, 请稍待片刻...\n\r");
	refresh();
	ythtbbs_user_clean();

	rc = ythtbbs_user_create(newuser, &i, &val);

	switch(rc) {
	case YTHTBBS_REGISTER_FILE_ERROR:
		return -1;
		break;
	case YTHTBBS_REGISTER_FULL:
		if (dashf("etc/user_full")) {
			ansimore("etc/user_full", false);
		} else {
			prints("抱歉, 使用者帐号已经满了, 无法注册新的帐号.\n\r");
		}
		prints("请等待 %d 分钟后再试一次, 祝你好运.\n\r", val);
		refresh();
		exit(1);
		break;
	default:
	case YTHTBBS_REGISTER_CANNOT_SEEK:
		return -1;
		break;
	case YTHTBBS_REGISTER_OK:
		return i;
		break;
	}
}

void
new_register()
{
	struct userec newuser;
	char passbuf[STRLEN];
	int allocid, try;

	memset(&newuser, 0, sizeof (newuser));

	ansimore("etc/register", NA);
	try = 0;
	while (1) {
		if (++try >= 9) {
			prints("\n掰掰，按太多下  <Enter> 了...\n");
			refresh();
			longjmp(byebye, -1);
		}
		move(t_lines - 6, 0);
		prints("帐号名称为您在本站所实际显示的用户名称，不可修改，请慎重选择 ");
		getdata(t_lines - 5, 0,
				"请输入帐号名称 (Enter User ID, \"0\" to abort): ",
				newuser.userid, IDLEN + 1, DOECHO, YEA);
		if (newuser.userid[0] == '0') {
			longjmp(byebye, -1);
		}
		if (id_with_num(newuser.userid)) {
			prints("帐号必须全为英文字母!\n");
		} else if (strlen(newuser.userid) < 2) {
			prints("帐号至少需有两个英文字母!\n");
		} else if ((*newuser.userid == '\0') || is_bad_id(newuser.userid)) {
			prints("抱歉, 您不能使用这个字作为帐号。 请想过另外一个。\n");
		} else if (dosearchuser(newuser.userid)) {
			prints("此帐号已经有人使用\n");
		} else
			break;
	}
	while (1) {
		getdata(t_lines - 4, 0, "请设定您的密码 (Setup Password): ", passbuf, PASSLEN, NOECHO, YEA);
		if (strlen(passbuf) < 4 || !strcmp(passbuf, newuser.userid)) {
			prints("密码太短或与使用者代号相同, 请重新输入\n");
			continue;
		}
		ytht_strsncpy(newuser.passwd, passbuf, PASSLEN);
		getdata(t_lines - 3, 0, "请再输入一次你的密码 (Reconfirm Password): ", passbuf, PASSLEN, NOECHO, YEA);
		if (strncmp(passbuf, newuser.passwd, PASSLEN - 1) != 0) {
			prints("密码输入错误, 请重新输入密码.\n");
			continue;
		}
		passbuf[8] = '\0';
		ytht_strsncpy(newuser.passwd, ytht_crypt_genpasswd(passbuf), sizeof(newuser.passwd));
		break;
	}
	strcpy(newuser.ip, "");
	newuser.userdefine = -1;
	if (!strcmp(newuser.userid, "guest")) {
		newuser.userlevel = 0;
		newuser.userdefine &= ~(DEF_FRIENDCALL | DEF_ALLMSG | DEF_FRIENDMSG);
	} else {
		newuser.userlevel = PERM_BASIC;
		newuser.flags[0] = PAGER_FLAG | BRDSORT_FLAG2;
	}
	newuser.userdefine &= ~(DEF_MAILMSG | DEF_NOLOGINSEND);
	if (g_convcode)
		newuser.userdefine &= ~DEF_USEGB;

	newuser.flags[1] = 0;
	newuser.firstlogin = newuser.lastlogin = time(NULL);
	newuser.lastlogout = 0;
	allocid = getnewuserid(&newuser);
	if (allocid > MAXUSERS || allocid <= 0) {
		prints("No space for new users on the system!\n\r");
		refresh();
		exit(1);
	}
	ythtbbs_cache_UserTable_setuserid(allocid, newuser.userid);
	if (!dosearchuser(newuser.userid)) {
		prints("User failed to create\n");
		refresh();
		exit(1);
	}
	sethomepath_s(genbuf, sizeof(genbuf), newuser.userid);
	mkdir(genbuf, 0775);
	sprintf(genbuf, "%s newaccount %d %s", newuser.userid, allocid, fromhost);
	newtrace(genbuf);
	bmy_user_create(allocid, newuser.userid);
}

int
invalid_email(char *addr)
{
	FILE *fp;
	char temp[STRLEN];

	if ((fp = fopen(".bad_email", "r")) != NULL) {
		while (fgets(temp, STRLEN, fp) != NULL) {
			strtok(temp, "\n");
			if (strstr(addr, temp) != NULL) {
				fclose(fp);
				return 1;
			}
		}
		fclose(fp);
	}
	return 0;
}

static int
invalid_realmail(char *userid, char *email, int msize)
{
	FILE *fn;
	char fname[STRLEN];
	struct stat st;

	//判断是否使用email注册... 不过是不是真的用这个判断的? --ylsdd
	if (sysconf_str("EMAILFILE") == NULL)
		return 0;

	if (strchr(email, '@') && valid_ident(email) && HAS_PERM(PERM_LOGINOK, currentuser))
		return 0;

	sethomefile_s(fname, sizeof(fname), userid, "register");
	if (stat(fname, &st) == 0) {
	}
	sethomefile_s(fname, sizeof(fname), userid, "register");
	if ((fn = fopen(fname, "r")) != NULL) {
		fgets(genbuf, STRLEN, fn);
		fclose(fn);
		strtok(genbuf, "\n");
		if (valid_ident(genbuf) && ((strchr(genbuf, '@') != NULL) || strstr(genbuf, "usernum"))) {
			if (strchr(genbuf, '@') != NULL)
				strncpy(email, genbuf, msize);
			move(21, 0);

			#ifndef POP_CHECK
			prints("恭贺您!! 您已顺利完成本站的使用者注册手续,\n");
			prints("从现在起您将拥有一般使用者的权利与义务...\n");
			#endif

			pressanykey();
			return 0;
		}
	}
	return 1;
}

void
check_register_info()
{
	struct userec *urec = &currentuser;
	char *newregfile;
	int perm;
	FILE *fout;
	char buf[192], buf2[STRLEN];

	clear();
	sprintf(buf, "%s", email_domain());
	if (!(urec->userlevel & PERM_BASIC)) {
		urec->userlevel = 0;
		return;
	}
//20060313修改 by clearboy(王晓钰)
//今天遇到了一个麻烦的问题,telnet登陆后就马上退出了
//大家跟踪了很久，发现问题在sysconf_eval
//进一步的问题，在于/home/bbs/deverrlog这个错误记录文件超过了2錑
//发现sysconf_eval不是必须的，所以删除掉。
	perm = PERM_DEFAULT;// & sysconf_eval("AUTOSET_PERM");


	while ((strlen(urec->username) < 2)) {
		getdata(2, 0, "请输入您的昵称 (Enter nickname): ",
			urec->username, NAMELEN, DOECHO, YEA);
		strcpy(uinfo.username, urec->username);
		update_utmp();
	}
	while ((strlen(urec->realname) < 4)
			|| (strstr(urec->realname, "  "))
			|| (strstr(urec->realname, "　"))) {
		move(3, 0);
		prints("请输入您的真实姓名 (Enter realname):\n");
		getdata(4, 0, "> ", urec->realname, NAMELEN, DOECHO, YEA);
	}
	while ((strlen(urec->address) < 10) || (strstr(urec->address, "   "))) {
		move(5, 0);
		prints("请输入您的通讯地址 (Enter home address)：\n");
		getdata(6, 0, "> ", urec->address, NAMELEN, DOECHO, YEA);
	}
	#ifndef POP_CHECK
	if (strchr(urec->email, '@') == NULL) {
		move(8, 0);
		prints("电子信箱格式为: \033[1;37muserid@your.domain.name\033[m\n");
		prints("请输入电子信箱 (不能提供者按 <Enter>)");
		getdata(10, 0, "> ", urec->email, STRLEN - 10, DOECHO, YEA);
		if (strchr(urec->email, '@') == NULL) {
			sprintf(genbuf, "%s.bbs@%s", urec->userid, buf);
			strsncpy(urec->email, genbuf, sizeof (urec->email));
		}
	}
	#endif
	if (!strcmp(currentuser.userid, "SYSOP")) {
		currentuser.userlevel = ~0;
		set_safe_record();
		substitute_record(PASSFILE, &currentuser, sizeof (struct userec), usernum);
	}
	if (!(currentuser.userlevel & PERM_LOGINOK)) {
		if (!invalid_realmail(urec->userid, urec->realmail, STRLEN - 16))
		{
			#ifndef POP_CHECK /* 防止拣回尸体后，不用输入信箱名。interma@BMY*/
			sethomefile_s(buf, sizeof buf, urec->userid, "sucessreg");
			if (((dashf(buf)) && !sysconf_str("EMAILFILE"))
					|| (sysconf_str("EMAILFILE"))) {
				set_safe_record();
				urec->userlevel |= PERM_DEFAULT;
				substitute_record(PASSFILE, urec, sizeof (struct userec), usernum);
			}
			#endif
		} else {
#ifdef EMAILREG
			if ((!strstr(urec->email, buf)) &&
					(!invalidaddr(urec->email)) &&
					(!invalid_email(urec->email))) {
				move(13, 0);
				prints("您的电子信箱 尚须通过回信验证...  \n");
				prints("    本站将马上寄一封验证信给您,\n");
				prints("    您只要从 %s 回信, 就可以成为本站合格公民.\n\n", urec->email);
				prints("    成为本站合格公民, 就能享有更多的权益喔!\n");
				move(20, 0);
				if (askyn("您要我们现在就寄这一封信吗", YEA, NA) == YEA) {
					randomize();
					code = (time(0) / 2) + (rand() / 10);
					sethomefile_s(genbuf, sizeof genbuf, urec->userid, "mailcheck");
					if ((dp = fopen(genbuf, "w")) == NULL) {
						fclose(dp);
						return;
					}
					fprintf(dp, "%9.9d\n", code);
					fclose(dp);
					sprintf(buf, "/usr/lib/sendmail -f %s.bbs@%s %s ", urec->userid, email_domain(), urec->email);
					fout = popen(buf, "w");
					fin = fopen(sysconf_str("EMAILFILE"), "r");
					/* begin of sending a mail to user to check email-addr */
					if ((fin != NULL) && (fout != NULL)) {
						fprintf(fout, "Reply-To: SYSOP.bbs@%s\n", email_domain());
						fprintf(fout, "From: SYSOP.bbs@%s\n", email_domain());
						fprintf(fout, "To: %s\n", urec->email);
						fprintf(fout, "Subject: @%s@[-%9.9d-]%s mail check.\n", urec->userid, code, MY_BBS_ID);
						fprintf(fout, "X-Forwarded-By: SYSOP \n");
						fprintf(fout, "X-Disclaimer: %s registration mail.\n", MY_BBS_ID);
						fprintf(fout, "\n");
						fprintf(fout, "BBS LOCATION     : %s (%s)\n", email_domain(), MY_BBS_IP);
						fprintf(fout, "YOUR BBS USER ID : %s\n", urec->userid);
						fprintf(fout, "APPLICATION DATE : %s", ctime(&urec->firstlogin));
						fprintf(fout, "LOGIN HOST       : %s\n", fromhost);
						fprintf(fout, "YOUR NICK NAME   : %s\n", urec->username);
						fprintf(fout, "YOUR NAME        : %s\n", urec->realname);
						while (fgets(buf, 255, fin) != NULL) {
							if (buf[0] == '.' && buf[1] == '\n')
								fputs(". \n", fout);
							else
								fputs(buf, fout);
						}
						fprintf(fout, ".\n");
						fclose(fin);
						fclose(fout);
					}	/* end of sending a mail to user to check email-addr */
					getdata(21, 0, "确认信已寄出, 等您回信哦!! 请按 <Enter> : ", ans, 2, DOECHO, YEA);
				}
			} else {
				showansi = 1;
				if (sysconf_str("EMAILFILE") != NULL) {
					prints("\n您所填写的电子邮件地址 【\033[1;33m%s\033[m】\n", urec->email);
					prints("并非合法之 UNIX 帐号，系统不会投递注册信，请到\033[1;32mInfoEdit->Info\033[m中修改...\n");
					pressanykey();
				}
			}
#endif
		}
	}
	if (urec->lastlogin - urec->firstlogin < 3 * 86400) {
		if (urec->numlogins == 1) {

			clear();
			sprintf(buf, "tmp/newcomer.%s", currentuser.userid);
			if ((fout = fopen(buf, "w")) != NULL) {
				fprintf(fout, "大家好,\n\n");
				fprintf(fout, "我是 %s (%s), 来自 %s\n",
					currentuser.userid, urec->username,
					fromhost);
				fprintf(fout, "今天我初来此站报到, 请大家多多指教。\n");
				move(5, 0);
				prints("请作个简短的个人简介, 向本站其他使用者打个招呼\n");
				prints("(最多三行, 写完可直接按 <Enter> 跳离)....");
				getdata(7, 0, ":", buf2, 75, DOECHO, YEA);
				if (buf2[0] != '\0') {
					fprintf(fout, "\n\n自我介绍:\n\n");
					fprintf(fout, "%s\n", buf2);
					getdata(8, 0, ":", buf2, 75, DOECHO, YEA);
					if (buf2[0] != '\0') {
						fprintf(fout, "%s\n", buf2);
						getdata(9, 0, ":", buf2, 75, DOECHO, YEA);
						if (buf2[0] != '\0') {
							fprintf(fout, "%s\n", buf2);
						}
					}
				}
				fclose(fout);
				postfile(buf, "newcomers", "新手上路....", 2);
				unlink(buf);
			}
			pressanykey();
		}
		newregfile = sysconf_str("NEWREGFILE");
		if (!HAS_PERM(PERM_SYSOP, currentuser) && newregfile != NULL) {
			set_safe_record();
			urec->userlevel &= ~(perm);
			substitute_record(PASSFILE, urec, sizeof (struct userec), usernum);
			ansimore(newregfile, YEA);
		}
	}
}

static int
valid_ident(char *ident)
{
	static char *const invalid[] = {
		"unknown@", "root@", "gopher@", "bbs@",
		"guest@", "nobody@", "www@", NULL
	};
	int i;
	if (ident[0] == '@')
		return 0;
	for (i = 0; invalid[i] != NULL; i++)
		if (strstr(ident, invalid[i]) != NULL)
			return 0;
	return 1;
}

