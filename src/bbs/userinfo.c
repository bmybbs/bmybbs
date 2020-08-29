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
 Copyright (C) 1999, Zhou Lin, kcn@cic.tsinghua.edu.cn

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

#ifdef POP_CHECK
// 登陆邮件服务器用的头文件 added by interma@BMY 2005.5.12
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "identify.h"
// 邮件服务器上用户名和密码的长度， added by interma@BMY 2005.5.12
#define USER_LEN 20
#define PASS_LEN 20
#endif

extern time_t login_start_time;
extern char fromhost[60];

static void getfield(int line, char *info, char *desc, char *buf, int len);

#ifdef POP_CHECK
void securityreport(char *str, char *content);

// 令username用户通过验证， added by interma@BMY 2005.5.12
static void register_success(int usernum, char *userid, char *realname,
		char *dept, char *addr, char *phone, char *assoc, char *email)
{
	struct userec uinfo;
	FILE *fout, *fn;
	char buf[STRLEN];
	int n;

	//int id = getuser(userid);
	usernum = getuser(userid);

	setuserfile(genbuf, "mailcheck");
	if ((fn = fopen(genbuf, "w")) == NULL) {
		fclose(fn);
		return;
	}
	fprintf(fn, "usernum: %d\n", usernum);
	fclose(fn);

	memcpy(&uinfo, &lookupuser, sizeof(uinfo));

	strsncpy(uinfo.userid, userid, sizeof(uinfo.userid));
	strsncpy(uinfo.realname, realname, sizeof(uinfo.realname));
	strsncpy(uinfo.address, addr, sizeof(uinfo.address));
	sprintf(genbuf, "%s$%s@%s", dept, phone, userid);
	strsncpy(uinfo.realmail, genbuf, sizeof(uinfo.realmail));

	strsncpy(uinfo.email, email, sizeof(uinfo.email));

	uinfo.userlevel |= PERM_DEFAULT;	// by ylsdd
	substitute_record(PASSFILE, &uinfo, sizeof(struct userec), usernum);

	sethomefile(buf, uinfo.userid, "sucessreg");
	if ((fout = fopen(buf, "w")) != NULL) {
		fprintf(fout, "\n");
		fclose(fout);
	}

	sethomefile(buf, uinfo.userid, "register");

	if ((fout = fopen(buf, "w")) != NULL) {
		fprintf(fout, "%s: %d\n", "usernum", usernum);
		fprintf(fout, "%s: %s\n", "userid", userid);
		fprintf(fout, "%s: %s\n", "realname", realname);
		fprintf(fout, "%s: %s\n", "dept", dept);
		fprintf(fout, "%s: %s\n", "addr", addr);
		fprintf(fout, "%s: %s\n", "phone", phone);
		fprintf(fout, "%s: %s\n", "assoc", assoc);

		n = time(NULL);
		fprintf(fout, "Date: %s", ctime((time_t *) &n));
		fprintf(fout, "Approved: %s\n", userid);
		fclose(fout);
	}

	mail_file("etc/s_fill", uinfo.userid, "恭禧您通过身份验证"); // 这个地方有个瑕疵，就是发信人为本人

	mail_file("etc/s_fill2", uinfo.userid, "欢迎加入" MY_BBS_NAME "大家庭");
	sethomefile(buf, uinfo.userid, "mailcheck");
	unlink(buf);
	sprintf(genbuf, "让 %s 通过身分确认.", uinfo.userid);
	securityreport(genbuf, genbuf);

	return;
}

// username用户验证失败的处理（砍掉这个用户，不过目前暂未使用这个函数）， added by interma@BMY 2005.5.16
static void register_fail(char *userid)
{
	int id;
	strcpy(genbuf, userid);
	id = getuser(genbuf);

	if (lookupuser.userid[0] == '\0' || !strcmp(lookupuser.userid, "SYSOP")) {
		return;
	}

	sprintf(genbuf, "mail/%c/%s", mytoupper(lookupuser.userid[0]), lookupuser.userid);
	deltree(genbuf);
	sprintf(genbuf, "home/%c/%s", mytoupper(lookupuser.userid[0]), lookupuser.userid);
	deltree(genbuf);
	lookupuser.userlevel = 0;
	strcpy(lookupuser.address, "");
	strcpy(lookupuser.username, "");
	strcpy(lookupuser.realname, "");
	strcpy(lookupuser.ip, "");
	strcpy(lookupuser.realmail, "");
	lookupuser.userid[0] = '\0';
	substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
	setuserid(id, lookupuser.userid);
}

char * str_to_upper(char *str)
{
	char *h = str;
	while (*str != '\n' && *str != 0) {
		*str = toupper(*str);
		str++;
	}
	return h;
}

extern char fromhost[60];
#endif
// -------------------------------------------------------------------------------

void permtostr(perm, str)
	int perm;char *str;
{
	int num;
	strcpy(str, "bTCPRp#@XWBA#VS-DOM-F012345678");
	for (num = 0; num < 30; num++)
		if (!(perm & (1 << num)))
			str[num] = '-';
	str[num] = '\0';
}

void disply_userinfo(struct userec *u, int real)
{
	struct stat st;
	long diff;
	size_t num;

	int exp;

	move(real == 1 ? 2 : 3, 0);
	clrtobot();
	prints("您的代号     : %s\n", u->userid);
	prints("您的昵称     : %s\n", u->username);
	prints("真实姓名     : %s\n", u->realname);
	prints("居住住址     : %s\n", u->address);
	prints("电子邮件信箱 : %s\n", u->email);
	if (real) {
		prints("真实 E-mail  : %s\n", u->realmail);
//        if HAS_PERM(PERM_ADMINMENU)
//           prints("Ident 资料   : %s\n", u->ident );
	}
	prints("域名指向     : %s\n", u->ip);
	prints("帐号建立日期 : %s", ctime(&u->firstlogin));
	prints("最近光临日期 : %s", ctime(&u->lastlogin));
	if (real) {
		prints("最近光临机器 : %s\n", u->lasthost);
	}
	prints("上次离站时间 : %s", u->lastlogout ? ctime(&u->lastlogout) : "不详\n");
	prints("上站次数     : %d 次\n", u->numlogins);
	if (real) {
		prints("文章数目     : %d\n", u->numposts);
	}
	exp = countexp(u);
	prints("经验值       : %d(%s)\n", exp, charexp(exp));
	exp = countperf(u);
	prints("表现值       : %d(%s)\n", exp, cperf(exp));
	prints("上站总时数   : %d 小时 %d 分钟\n", u->stay / 3600, (u->stay / 60) % 60);
	sprintf(genbuf, "mail/%c/%s/%s", mytoupper(u->userid[0]), u->userid, DOT_DIR);
	if (stat(genbuf, &st) >= 0)
		num = st.st_size / (sizeof(struct fileheader));
	else
		num = 0;
#ifdef __LP64
	prints("私人信箱     : %ld 封\n", num);
#else
	prints("私人信箱     : %d 封\n", num);
#endif

	if (real) {
		//modified by pzhg 071005
		strcpy(genbuf, "bTCPRp#@XWBA#VS-DOM-F0s23456789H");
		for (num = 0; num < strlen(genbuf); num++)
			if (!(u->userlevel & (1 << num)))
				genbuf[num] = '-';
		//genbuf[num] = '\0';
		//permtostr(u->userlevel, genbuf);
		prints("使用者权限   : %s\n", genbuf);
	} else {
		diff = (time(0) - login_start_time) / 60;
#ifdef __LP64
		prints("停留期间     : %ld 小时 %02ld 分\n", diff / 60, diff % 60);
#else
		prints("停留期间     : %d 小时 %02d 分\n", diff / 60, diff % 60);
#endif
		prints("萤幕大小     : %dx%d\n", t_lines, t_columns);
	}
	prints("\n");
	if (u->userlevel & PERM_LOGINOK) {
		prints("  您的注册程序已经完成, 欢迎加入本站.\n");
	} else if (u->lastlogin - u->firstlogin < 3 * 86400) {
		prints("  新手上路, 请阅读 Announce 讨论区.\n");
	} else {
		prints("  注册尚未成功, 请参考本站进站画面说明.\n");
	}
}

int uinfo_query(struct userec *u, int real, int unum)
{
	struct userec newinfo;
	char ans[3], buf[STRLEN], genbuf[128];
	char src[STRLEN], dst[STRLEN];
	int i, fail = 0, netty_check = 0;
	FILE *fin, *fout, *dp;
	time_t code;

	memcpy(&newinfo, u, sizeof(currentuser));
	getdata(t_lines - 1, 0,
			real ? "请选择 (0)结束 (1)修改资料 (2)设定密码 (3) 改 ID ==> [0]" : "请选择 (0)结束 (1)修改资料 (2)设定密码 (3) 选签名档 ==> [0]",
			ans, 2, DOECHO, YEA);
	clear();

	i = 3;
	move(i++, 0);
	if (ans[0] != '3' || real)
		prints("使用者代号: %s\n", u->userid);

	switch (ans[0]) {
	case '1':
		move(1, 0);
		prints("请逐项修改,直接按 <ENTER> 代表使用 [] 内的资料。\n");

		sprintf(genbuf, "昵称 [%s]: ", u->username);
		getdata(i++, 0, genbuf, buf, NAMELEN, DOECHO, YEA);
		if (buf[0])
			strncpy(newinfo.username, buf, NAMELEN);
		if (!real && buf[0])
			strncpy(uinfo.username, buf, 40);

		sprintf(genbuf, "真实姓名 [%s]: ", u->realname);
		getdata(i++, 0, genbuf, buf, NAMELEN, DOECHO, YEA);
		if (buf[0])
			strncpy(newinfo.realname, buf, NAMELEN);

		sprintf(genbuf, "居住地址 [%s]: ", u->address);
		getdata(i++, 0, genbuf, buf, NAMELEN, DOECHO, YEA);
		if (buf[0])
			strncpy(newinfo.address, buf, NAMELEN);

#ifndef POP_CHECK  // 不让改信箱地址了，因为要绑定
		sprintf(genbuf, "电子信箱 [%s]: ", u->email);
		getdata(i++, 0, genbuf, buf, STRLEN - 10, DOECHO, YEA);
		if (buf[0]) {
			strncpy(newinfo.email, buf, STRLEN);
		}
#endif

		sprintf(genbuf, "域名指向 [%s]: ", u->ip);
		getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
		if (buf[0])
			strncpy(newinfo.ip, buf, 16);

		if (real) {
			sprintf(genbuf, "真实Email[%s]: ", u->realmail);
			getdata(i++, 0, genbuf, buf, STRLEN - 10, DOECHO, YEA);
			if (buf[0])
				strncpy(newinfo.realmail, buf, STRLEN - 16);

			sprintf(genbuf, "上线次数 [%d]: ", u->numlogins);
			getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
			if (atoi(buf) > 0)
				newinfo.numlogins = atoi(buf);

			sprintf(genbuf, "文章数目 [%d]: ", u->numposts);
			getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
			if (atoi(buf) > 0)
				newinfo.numposts = atoi(buf);

			sprintf(genbuf, "上站小时数 [%ld 小时 %ld 分钟]: ",
					(long int) (u->stay / 3600),
					(long int) ((u->stay / 60) % 60));
			getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
			if (atoi(buf) > 0)
				newinfo.stay = atoi(buf) * 3600;
		}

		break;
	case '2':
		if (!real) {
			getdata(i++, 0, "请输入原密码: ", buf, PASSLEN, NOECHO, YEA);
			if (*buf == '\0' || !checkpasswd(u->passwd, buf)) {
				prints("\n\n很抱歉, 您输入的密码不正确。\n");
				fail++;
				break;
			}
		}
		getdata(i++, 0, "请设定新密码: ", buf, PASSLEN, NOECHO, YEA);
		if (buf[0] == '\0') {
			prints("\n\n密码设定取消, 继续使用旧密码\n");
			fail++;
			break;
		}
		strncpy(genbuf, buf, PASSLEN);

		getdata(i++, 0, "请重新输入新密码: ", buf, PASSLEN, NOECHO,
		YEA);
		if (strncmp(buf, genbuf, PASSLEN)) {
			prints("\n\n新密码确认失败, 无法设定新密码。\n");
			fail++;
			break;
		}
		buf[8] = '\0';
		strncpy(newinfo.passwd, genpasswd(buf), PASSLEN);
		break;
	case '3':
		if (!real) {
			sprintf(genbuf, "目前使用签名档 [%d]: ", u->signature);
			getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
			if (atoi(buf) > 0)
				newinfo.signature = atoi(buf);
		} else {
			getdata(i++, 0, "新的使用者代号: ", genbuf, IDLEN + 1, DOECHO, YEA);
			if (*genbuf != '\0') {
				if (getuser(genbuf)) {
					prints("\n错误! 已经有同样 ID 的使用者\n");
					fail++;
				} else if (!goodgbid(genbuf)) {
					prints("\n错误! 不合法的 ID\n");
					fail++;
				} else {
					strncpy(newinfo.userid, genbuf, IDLEN + 2);
				}
			}
		}
		break;
	default:
		clear();
		return 0;
	}
	if (fail != 0) {
		pressreturn();
		clear();
		return 0;
	}
	if (askyn("确定要改变吗", NA, YEA) == YEA) {
		if (real) {
			char secu[STRLEN];
			sprintf(secu, "修改 %s 的基本资料或密码。", u->userid);
			securityreport(secu, secu);
		}
		if (strcmp(u->userid, newinfo.userid)) {

			sprintf(src, "mail/%c/%s", mytoupper(u->userid[0]), u->userid);
			sprintf(dst, "mail/%c/%s", mytoupper(newinfo.userid[0]), newinfo.userid);
			rename(src, dst);
			sethomepath(src, u->userid);
			sethomepath(dst, newinfo.userid);
			rename(src, dst);
			sethomefile(src, u->userid, "register");
			unlink(src);
			sethomefile(src, u->userid, "register.old");
			unlink(src);
			setuserid(unum, newinfo.userid);
		}
		if (!strcmp(u->userid, currentuser.userid)) {
			extern int WishNum;
			strncpy(uinfo.username, newinfo.username, NAMELEN);
			WishNum = 9999;
		}
		/* added by netty to automatically send a mail to new user. */

		if ((netty_check == 1)) {
			sprintf(genbuf, "%s", email_domain());
			if ((sysconf_str("EMAILFILE") != NULL)
					&& (!strstr(newinfo.email, genbuf))
					&& (!invalidaddr(newinfo.email))
					&& (!invalid_email(newinfo.email))) {
				randomize();
				code = (time(0) / 2) + (rand() / 10);
				sethomefile(genbuf, u->userid, "mailcheck");
				if ((dp = fopen(genbuf, "w")) == NULL) {
					fclose(dp);
					return -2;
				}
				fprintf(dp, "%9.9ld\n", (long int) code);
				fclose(dp);
				sprintf(genbuf, "/usr/lib/sendmail -f %s.bbs@%s %s ", u->userid, email_domain(), newinfo.email);
				fout = popen(genbuf, "w");
				fin = fopen(sysconf_str("EMAILFILE"), "r");
				if (fin == NULL || fout == NULL)
					return -1;
				fprintf(fout, "Reply-To: SYSOP.bbs@%s\n", email_domain());
				fprintf(fout, "From: SYSOP.bbs@%s\n", email_domain());
				fprintf(fout, "To: %s\n", newinfo.email);
				fprintf(fout, "Subject: @%s@[-%9.9ld-]%s mail check.\n", u->userid, (long int) code, MY_BBS_ID);
				fprintf(fout, "X-Forwarded-By: SYSOP \n");
				fprintf(fout, "X-Disclaimer: %s registration mail.\n", MY_BBS_ID);
				fprintf(fout, "\n");
				fprintf(fout, "BBS LOCATION     : %s (%s)\n", email_domain(), MY_BBS_IP);
				fprintf(fout, "YOUR BBS USER ID : %s\n", u->userid);
				fprintf(fout, "APPLICATION DATE : %s", ctime(&u->firstlogin));
				fprintf(fout, "LOGIN HOST       : %s\n", fromhost);
				fprintf(fout, "YOUR NICK NAME   : %s\n", u->username);
				fprintf(fout, "YOUR NAME        : %s\n", u->realname);
				while (fgets(genbuf, 255, fin) != NULL) {
					if (genbuf[0] == '.' && genbuf[1] == '\n')
						fputs(". \n", fout);
					else
						fputs(genbuf, fout);
				}
				fprintf(fout, ".\n");
				fclose(fin);
				pclose(fout);
			} else {
				if (sysconf_str("EMAILFILE") != NULL) {
					move(t_lines - 5, 0);
					prints("\n您所填的电子邮件地址 【\033[1;33m%s\033[m】\n", newinfo.email);
					prints("并非合法之 UNIX 帐号，系统不会投递注册信，请把它修正好...\n");
					pressanykey();
				}
			}
		}
		memcpy(u, &newinfo, sizeof(newinfo));
		set_safe_record();
		if (netty_check == 1) {
			newinfo.userlevel &= ~(PERM_LOGINOK | PERM_PAGE);
			sethomefile(src, newinfo.userid, "register");
			sethomefile(dst, newinfo.userid, "register.old");
			rename(src, dst);
		}
		substitute_record(PASSFILE, &newinfo, sizeof(newinfo), unum);
	}
	clear();
	return 0;
}

void x_info()
{
	modify_user_mode(GMENU);
	if (!strcmp("guest", currentuser.userid)) {
		disply_userinfo(&currentuser, 0);
		pressreturn();
		return;
	}
	disply_userinfo(&currentuser, 1);
	uinfo_query(&currentuser, 0, usernum);
}

static void getfield(line, info, desc, buf, len)
	int line, len;char *info, *desc, *buf;
{
	char prompt[STRLEN];

	sprintf(genbuf, "  原先设定: %-46.46s \033[1;32m(%s)\033[m",
			(buf[0] == '\0') ? "(未设定)" : buf, info);
	move(line, 0);
	prints("%s", genbuf);
	sprintf(prompt, "  %s: ", desc);
	getdata(line + 1, 0, prompt, genbuf, len, DOECHO, YEA);
	if (genbuf[0] != '\0') {
		strncpy(buf, genbuf, len);
	}
	move(line, 0);
	clrtoeol();
	prints("  %s: %s\n", desc, buf);
	clrtoeol();
}

#ifdef POP_CHECK
void x_fillform()
{
	char rname[NAMELEN], addr[STRLEN];
	char phone[STRLEN], dept[STRLEN], assoc[STRLEN];
	char ans[5], *mesg, *ptr;
	FILE *fn;
	char user[USER_LEN + 1];
	char email[STRLEN];
	int rc;
	char tempn[3];
	int n = -1;

	struct active_data act_data;
	int index;
	int cap_status;

	modify_user_mode(NEW);
	move(3, 0);
	clrtobot();
	if (!strcmp("guest", currentuser.userid)) {
		prints("抱歉, 请用 new 申请一个新帐号后再填申请表.");
		pressreturn();
		return;
	}
	if (currentuser.userlevel & PERM_LOGINOK) {
		prints("您已经完成本站的使用者注册手续, 欢迎加入本站的行列.");
		pressreturn();
		return;
	}
	if ((fn = fopen("new_register", "r")) != NULL) {
		while (fgets(genbuf, STRLEN, fn) != NULL) {
			if ((ptr = strchr(genbuf, '\n')) != NULL)
				*ptr = '\0';
			if (strncmp(genbuf, "userid: ", 8) == 0
					&& strcmp(genbuf + 8, currentuser.userid) == 0) {
				fclose(fn);
				prints("站长尚未处理您的注册申请单, 请耐心等候.");
				pressreturn();
				return;
			}
		}
		fclose(fn);
	}

	cap_status = check_captcha_status(currentuser.userid);
	if (cap_status == CAPTCHA_OK) {
		char code[6];
		do {
			clear();
			getdata(3, 0, "请输入您的验证码(输入x放弃验证) >>  ", code, 5, DOECHO, YEA);

			if (strcasecmp(code, "x") == 0) {
				return;
			}

			cap_status = verify_captcha_for_user(currentuser.userid, code);
			if (cap_status == CAPTCHA_OK) {
				// 当前邮箱验证成功
				// IronBlood 2020.08.29 备注：
				// 判断路径从 pop_register/%s_privilege 改变为 pop_register/%s，结合系统管理菜单
				// 中的设置项，目前可以为 mail / stu 两个邮箱域设置不受账号数目限制的邮箱
				char path[128];
				char *domain;
				int isprivilege = 0;

				snprintf(email, sizeof(email), "%s", currentuser.email);
				domain = strchr(email, '@');
				if (domain != NULL) {
					*domain = '\0';
					domain++;
					sprintf(path, MY_BBS_HOME "/etc/pop_register/%s", domain);

					if (seek_in_file(path, user)) {
						isprivilege = 1;
					}
				}

				if (isprivilege == 0 && query_record_num(email, MAIL_ACTIVE) >= MAX_USER_PER_RECORD) {
					clear();
					move(3, 0);
					prints("您的信箱已经验证过 %d 个id，无法再用于验证了!\n", MAX_USER_PER_RECORD);
					pressreturn();
					return;
				}

				int response;

				read_active(currentuser.userid, &act_data);
				act_data.status = 1;
				response = write_active(&act_data);

				if (response == WRITE_SUCCESS || response == UPDATE_SUCCESS) {
					clear();
					move(5, 0);
					prints("身份审核成功，您已经可以使用所用功能了！\n");
					strncpy(currentuser.email, email, STRLEN);
					register_success(usernum, currentuser.userid, rname, dept, addr, phone, assoc, email);

					//scroll();
					pressreturn();
					return;
				}
				clear();
				move(3, 0);
				prints("验证失败!");
				pressreturn();
				return;
			} else {
				// 验证码校验失败，重新判断状态
				cap_status = check_captcha_status(currentuser.userid);
				if (cap_status == CAPTCHA_TIMEOUT || cap_status == CAPTCHA_USED) {
					prints("验证码已失效，请重新填写注册单");
					pressanykey();
					break;
				} else {
					prints("验证码错误，请重新输入");
					pressanykey();
				}
			}
		} while (1);
	}

	move(3, 0);
	sprintf(genbuf, "您要填写注册单，加入%s大家庭吗？", MY_BBS_NAME);
	if (askyn(genbuf, YEA, NA) == NA)
		return;
	strncpy(rname, currentuser.realname, NAMELEN);
	strncpy(addr, currentuser.address, STRLEN);
	dept[0] = phone[0] = assoc[0] = '\0';
	while (1) {
		move(3, 0);
		clrtoeol();
		prints("%s 您好, 请据实填写以下的资料:\n", currentuser.userid);
		getfield(6, "请用中文", "真实姓名", rname, NAMELEN);
		getfield(8, "学校系级或公司职称", "学校系级或工作单位", dept, STRLEN);
		getfield(10, "包括寝室或门牌号码", "目前住址或通讯地址", addr, STRLEN);
		getfield(12, "包括可联络时间", "联络电话", phone, STRLEN);
		getfield(14, "校友会或毕业学校", "校 友 会", assoc, STRLEN);
		/* only for 9#        getfield( 14, "介绍人ID和真实姓名",    "介绍人(外来的ID须填:ID/姓名)",   assoc, STRLEN );
		 */
		mesg = "以上资料是否正确, 按 Q 放弃注册 (Y/N/Quit)? [N]: ";
		getdata(t_lines - 1, 0, mesg, ans, 3, DOECHO, YEA);
		if (ans[0] == 'Q' || ans[0] == 'q')
			return;
		if (ans[0] == 'Y' || ans[0] == 'y')
			break;
	}
	strncpy(currentuser.realname, rname, NAMELEN);
	strncpy(currentuser.address, addr, STRLEN);
	memset(&act_data, 0, sizeof(act_data));
	strcpy(act_data.name, rname);
	strcpy(act_data.dept, dept);
	strcpy(act_data.userid, currentuser.userid);
	strcpy(act_data.phone, phone);
	strcpy(act_data.operator, currentuser.userid);
	strcpy(act_data.ip, currentuser.lasthost);
	act_data.status = 0;
	write_active(&act_data);

	// 以下要用户选择是否要通过邮件服务器进行审核， added by interma@BMY 2005.5.12
	do {
		clear();
		move(3, 0);
		prints("下面将进行实名认证。本站目前支持以下域名的电子信箱进行认证. \n");
		prints("每个信箱可以认证 %d 个id.\n\n", MAX_USER_PER_RECORD);
		for (index = 1; index <= DOMAIN_COUNT; ++index) {
			prints("[%d] %s \n", index, MAIL_DOMAINS[index]);
		}
		while (!(n > 0 && n <= DOMAIN_COUNT)) {
			getdata(10, 0, "请选择你的信箱域名序号（新生注册请选择1） >>  ", tempn, 3, DOECHO, YEA);
			sscanf(tempn, "%d", &n);
		}
		getdata(13, 0, "信箱用户名(输入x放弃验证，新生注册请输入用户名test) >>  ", user, USER_LEN, DOECHO, YEA);

		if (strcmp(user, "x") == 0) {
			return;
		}
		if (strcasecmp(user, "test") == 0) {
			clear();
			move(5, 0);
			prints("欢迎您加入交大，来到兵马俑BBS。\n您采用了新生测试信箱注册，目前您是新生用户身份。");
			prints("目前您没有发文、信件、消息等权限。\n\n");
			prints("请在开学取得xjtu.edu.cn信箱后，\n按照上站登录后的提示完成信箱绑定认证操作，成为本站正式用户。");
			pressanykey();
			return;
		}

		move(11, 0);
		clrtobot();
		move(12, 0);
		strcpy(email, str_to_lowercase(user));
		strcat(email, "@");
		strcat(email, MAIL_DOMAINS[n]);
		rc = send_active_mail(currentuser.userid, email);

		if (rc != 1)
			prints("验证码失败，请检查后重新输入.");
	} while (rc != 1);

	strncpy(currentuser.email, email, STRLEN);
	strcpy(act_data.email, email);
	write_active(&act_data);

	move(3, 0);
	sprintf(genbuf, "验证码已发送，需要现在输入吗？");
	if (askyn(genbuf, YEA, NA) == NA)
		return;

	x_fillform();
}
#else
void
x_fillform()
{
	char rname[NAMELEN], addr[STRLEN];
	char phone[STRLEN], dept[STRLEN], assoc[STRLEN];
	char ans[5], *mesg, *ptr;
	FILE *fn;
	time_t now;
	int lockfd;

	modify_user_mode(NEW);
	move(3, 0);
	clrtobot();
	if (!strcmp("guest", currentuser.userid))
	{
		prints("抱歉, 请用 new 申请一个新帐号后再填申请表.");
		pressreturn();
		return;
	}
	if (currentuser.userlevel & PERM_LOGINOK)
	{
		prints("您已经完成本站的使用者注册手续, 欢迎加入本站的行列.");
		pressreturn();
		return;
	}
	if ((fn = fopen("new_register", "r")) != NULL)
	{
		while (fgets(genbuf, STRLEN, fn) != NULL)
		{
			if ((ptr = strchr(genbuf, '\n')) != NULL)
			*ptr = '\0';
			if (strncmp(genbuf, "userid: ", 8) == 0 &&
					strcmp(genbuf + 8, currentuser.userid) == 0)
			{
				fclose(fn);
				prints
				("站长尚未处理您的注册申请单, 请耐心等候.");
				pressreturn();
				return;
			}
		}
		fclose(fn);
	}
	move(3, 0);
	sprintf(genbuf, "您要填写注册单，加入%s大家庭吗？", MY_BBS_NAME);
	if (askyn(genbuf, YEA, NA) == NA)
	return;
	strncpy(rname, currentuser.realname, NAMELEN);
	strncpy(addr, currentuser.address, STRLEN);
	dept[0] = phone[0] = assoc[0] = '\0';
	while (1)
	{
		move(3, 0);
		clrtoeol();
		prints("%s 您好, 请据实填写以下的资料:\n", currentuser.userid);
		getfield(6, "请用中文", "真实姓名", rname, NAMELEN);
		getfield(8, "学校系级或公司职称", "学校系级或工作单位", dept, STRLEN);
		getfield(10, "包括寝室或门牌号码", "目前住址或通讯地址", addr, STRLEN);
		getfield(12, "包括可联络时间", "联络电话", phone, STRLEN);
		getfield(14, "校友会或毕业学校", "校 友 会", assoc, STRLEN);
		/* only for 9#        getfield( 14, "介绍人ID和真实姓名",    "介绍人(外来的ID须填:ID/姓名)",   assoc, STRLEN );
		 */
		mesg = "以上资料是否正确, 按 Q 放弃注册 (Y/N/Quit)? [N]: ";
		getdata(t_lines - 1, 0, mesg, ans, 3, DOECHO, YEA);
		if (ans[0] == 'Q' || ans[0] == 'q')
		return;
		if (ans[0] == 'Y' || ans[0] == 'y')
		break;
	}
	strncpy(currentuser.realname, rname, NAMELEN);
	strncpy(currentuser.address, addr, STRLEN);
	lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX);
	if ((fn = fopen("new_register", "a")) != NULL)
	{
		now = time(NULL);
		fprintf(fn, "usernum: %d, %s", usernum, ctime(&now));
		fprintf(fn, "userid: %s\n", currentuser.userid);
		fprintf(fn, "realname: %s\n", rname);
		fprintf(fn, "dept: %s\n", dept);
		fprintf(fn, "addr: %s\n", addr);
		fprintf(fn, "phone: %s\n", phone);
		fprintf(fn, "assoc: %s\n", assoc);
		fprintf(fn, "----\n");
		fclose(fn);
	}
	close(lockfd);
	setuserfile(genbuf, "mailcheck");
	if ((fn = fopen(genbuf, "w")) == NULL)
	{
		fclose(fn);
		return;
	}
	fprintf(fn, "usernum: %d\n", usernum);
	fclose(fn);
}

#endif
