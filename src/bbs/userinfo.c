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
// µÇÂ½ÓÊ¼ş·şÎñÆ÷ÓÃµÄÍ·ÎÄ¼ş added by interma@BMY 2005.5.12
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
// ÓÊ¼ş·şÎñÆ÷ÉÏÓÃ»§ÃûºÍÃÜÂëµÄ³¤¶È£¬ added by interma@BMY 2005.5.12
#define USER_LEN 20
#define PASS_LEN 20
#endif

extern time_t login_start_time;
extern char fromhost[60];

static void getfield(int line, char *info, char *desc, char *buf, int len);

#ifdef POP_CHECK
// µÇÂ½ÓÊ¼ş·şÎñÆ÷£¬½øĞĞÉí·İÑéÖ¤£¬ added by interma@BMY 2005.5.12
/* ·µ»ØÖµÎª1±íÊ¾ÓĞĞ§£¬0±íÊ¾ÎŞĞ§, -1±íÊ¾ºÍpop·şÎñÆ÷Á¬½Ó³ö´í */
static int test_mail_valid(const char *user, const char *pass,
		const char *popip)
{
	char buffer[512];
	int sockfd;
	struct sockaddr_in server_addr;
	struct hostent *host;

	if (user[0] == ' ' || pass[0] == ' ')
		return 0;

	/* ¿Í»§³ÌĞò¿ªÊ¼½¨Á¢ sockfdÃèÊö·û */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		close(sockfd);
		return -1;
	}
	int i;
	for (i = 0; i < 8; i++)
		server_addr.sin_zero[i] = 0;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(110);
	// 202.117.1.22 == stu.xjtu.edu.cn
	if (inet_aton(popip, &server_addr.sin_addr) == 0) {
		close(sockfd);
		return -1;
	}

	/* ¿Í»§³ÌĞò·¢ÆğÁ¬½ÓÇëÇó */
	if (connect(sockfd, (struct sockaddr *) (&server_addr),
			sizeof(struct sockaddr)) == -1) {
		close(sockfd);
		return -1;
	}

	if (read(sockfd, buffer, 512) == -1) {
		close(sockfd);
		return -1;
	}
	if (buffer[0] == '-')
		return -1;

	sprintf(buffer, "USER %s\r\n", user);
	if (write(sockfd, buffer, strlen(buffer)) == -1) {
		close(sockfd);
		return -1;
	}

	if (read(sockfd, buffer, 512) == -1) {
		close(sockfd);
		return -1;
	}
	if (buffer[0] == '-') {
		close(sockfd);
		return 0;
	}

	sprintf(buffer, "PASS %s\r\n", pass);
	if (write(sockfd, buffer, strlen(buffer)) == -1) {
		close(sockfd);
		return -1;
	}

	if (read(sockfd, buffer, 512) == -1) {
		close(sockfd);
		return -1;
	}
	if (buffer[0] == '-') {
		close(sockfd);
		return 0;
	}

	write(sockfd, "QUIT\r\n", strlen("QUIT\r\n"));
	close(sockfd);
	return 1;
}

void securityreport(char *str, char *content);

// ÁîusernameÓÃ»§Í¨¹ıÑéÖ¤£¬ added by interma@BMY 2005.5.12
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

	mail_file("etc/s_fill", uinfo.userid, "¹§ìûÄúÍ¨¹ıÉí·İÑéÖ¤"); // Õâ¸öµØ·½ÓĞ¸öè¦´Ã£¬¾ÍÊÇ·¢ĞÅÈËÎª±¾ÈË

	mail_file("etc/s_fill2", uinfo.userid, "»¶Ó­¼ÓÈë" MY_BBS_NAME "´ó¼ÒÍ¥");
	sethomefile(buf, uinfo.userid, "mailcheck");
	unlink(buf);
	sprintf(genbuf, "ÈÃ %s Í¨¹ıÉí·ÖÈ·ÈÏ.", uinfo.userid);
	securityreport(genbuf, genbuf);

	return;
}

// usernameÓÃ»§ÑéÖ¤Ê§°ÜµÄ´¦Àí£¨¿³µôÕâ¸öÓÃ»§£¬²»¹ıÄ¿Ç°ÔİÎ´Ê¹ÓÃÕâ¸öº¯Êı£©£¬ added by interma@BMY 2005.5.16
void register_fail(char *userid)
{
	int id;
	strcpy(genbuf, userid);
	id = getuser(genbuf);

	if (lookupuser.userid[0] == '\0' || !strcmp(lookupuser.userid, "SYSOP")) {
		return;
	}

	sprintf(genbuf, "mail/%c/%s", mytoupper(lookupuser.userid[0]),
			lookupuser.userid);
	deltree(genbuf);
	sprintf(genbuf, "home/%c/%s", mytoupper(lookupuser.userid[0]),
			lookupuser.userid);
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
// ¼ÍÂ¼pop·şÎñÆ÷ÉÏµÄÓÃ»§Ãû£¬·ÀÖ¹ÖØ¸´×¢²á¶à¸öid£¬ added by interma@BMY 2005.5.16
/* ·µ»ØÖµÎª0±íÊ¾ÒÑ¼ÍÂ¼£¨Î´´æÔÚ£©£¬1±íÊ¾ÒÑ´æÔÚ */
int write_pop_user(char *user, char *userid, char *pop_name)
{
	FILE *fp;
	char buf[256];
	char path[256];
	int isprivilege = 0;

	char username[USER_LEN + 2];
	sprintf(username, "%s\n", user);

	// Ê×ÏÈ½øĞĞÌØÈ¨ÓÃ»§£¨privilege£©¼ìÑé
	sprintf(path, MY_BBS_HOME "/etc/pop_register/%s_privilege", pop_name);

	fp = fopen(path, "r");
	if (fp != NULL) {
		while (fgets(buf, 256, fp) != NULL) {
			if (strcmp(str_to_upper(username), str_to_upper(buf)) == 0) {
				isprivilege = 1;
				break;
			}
		}

		fclose(fp);
	}

	// ÒÔÏÂ½øĞĞÆÕÍ¨ÓÃ»§¼ìÑé
	sprintf(path, MY_BBS_HOME "/etc/pop_register/%s", pop_name);

	int lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX); // ¼ÓËøÀ´±£Ö¤»¥³â²Ù×÷

	fp = fopen(path, "a+");

	if (fp == NULL) {
		close(lockfd);
		return 0;
	}

	if (isprivilege == 0) {
		fseek(fp, 0, SEEK_SET);
		while (fgets(buf, 256, fp) != NULL) {
			if (strcmp(str_to_upper(username), str_to_upper(buf)) == 0) {
				fclose(fp);
				close(lockfd);
				return 1;
			}
			fgets(buf, 256, fp);
		}
	}

	fseek(fp, 0, SEEK_END);
	fputs(user, fp);

	time_t t;
	time(&t);

	sprintf(buf, "\n%s : %s : %s", userid, fromhost, ctime(&t));
	fputs(buf, fp);

	fclose(fp);
	close(lockfd);
	return 0;
}
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
#ifdef __LP64
	long diff, num;
#else
	int diff, num;
#endif

	int exp;

	move(real == 1 ? 2 : 3, 0);
	clrtobot();
	prints("ÄúµÄ´úºÅ     : %s\n", u->userid);
	prints("ÄúµÄêÇ³Æ     : %s\n", u->username);
	prints("ÕæÊµĞÕÃû     : %s\n", u->realname);
	prints("¾Ó×¡×¡Ö·     : %s\n", u->address);
	prints("µç×ÓÓÊ¼şĞÅÏä : %s\n", u->email);
	if (real) {
		prints("ÕæÊµ E-mail  : %s\n", u->realmail);
//        if HAS_PERM(PERM_ADMINMENU)
//           prints("Ident ×ÊÁÏ   : %s\n", u->ident );
	}
	prints("ÓòÃûÖ¸Ïò     : %s\n", u->ip);
	prints("ÕÊºÅ½¨Á¢ÈÕÆÚ : %s", ctime(&u->firstlogin));
	prints("×î½ü¹âÁÙÈÕÆÚ : %s", ctime(&u->lastlogin));
	if (real) {
		prints("×î½ü¹âÁÙ»úÆ÷ : %s\n", u->lasthost);
	}
	prints("ÉÏ´ÎÀëÕ¾Ê±¼ä : %s", u->lastlogout ? ctime(&u->lastlogout) : "²»Ïê\n");
	prints("ÉÏÕ¾´ÎÊı     : %d ´Î\n", u->numlogins);
	if (real) {
		prints("ÎÄÕÂÊıÄ¿     : %d\n", u->numposts);
	}
	exp = countexp(u);
	prints("¾­ÑéÖµ       : %d(%s)\n", exp, charexp(exp));
	exp = countperf(u);
	prints("±íÏÖÖµ       : %d(%s)\n", exp, cperf(exp));
	prints("ÉÏÕ¾×ÜÊ±Êı   : %d Ğ¡Ê± %d ·ÖÖÓ\n", u->stay / 3600, (u->stay / 60) % 60);
	sprintf(genbuf, "mail/%c/%s/%s", mytoupper(u->userid[0]), u->userid,
			DOT_DIR);
	if (stat(genbuf, &st) >= 0)
		num = st.st_size / (sizeof(struct fileheader));
	else
		num = 0;
#ifdef __LP64
	prints("Ë½ÈËĞÅÏä     : %ld ·â\n", num);
#else
	prints("Ë½ÈËĞÅÏä     : %d ·â\n", num);
#endif

	if (real) {
		//modified by pzhg 071005
		strcpy(genbuf, "bTCPRp#@XWBA#VS-DOM-F0s23456789H");
		for (num = 0; num < strlen(genbuf); num++)
			if (!(u->userlevel & (1 << num)))
				genbuf[num] = '-';
		//genbuf[num] = '\0';
		//permtostr(u->userlevel, genbuf);
		prints("Ê¹ÓÃÕßÈ¨ÏŞ   : %s\n", genbuf);
	} else {
		diff = (time(0) - login_start_time) / 60;
#ifdef __LP64
		prints("Í£ÁôÆÚ¼ä     : %ld Ğ¡Ê± %02ld ·Ö\n", diff / 60,
				diff % 60);
#else
		prints("Í£ÁôÆÚ¼ä     : %d Ğ¡Ê± %02d ·Ö\n", diff / 60, diff % 60);
#endif
		prints("Ó©Ä»´óĞ¡     : %dx%d\n", t_lines, t_columns);
	}
	prints("\n");
	if (u->userlevel & PERM_LOGINOK) {
		prints("  ÄúµÄ×¢²á³ÌĞòÒÑ¾­Íê³É, »¶Ó­¼ÓÈë±¾Õ¾.\n");
	} else if (u->lastlogin - u->firstlogin < 3 * 86400) {
		prints("  ĞÂÊÖÉÏÂ·, ÇëÔÄ¶Á Announce ÌÖÂÛÇø.\n");
	} else {
		prints("  ×¢²áÉĞÎ´³É¹¦, Çë²Î¿¼±¾Õ¾½øÕ¾»­ÃæËµÃ÷.\n");
	}
}

int uinfo_query(u, real, unum)
	struct userec *u;int real, unum;
{
	struct userec newinfo;
	char ans[3], buf[STRLEN], genbuf[128];
	char src[STRLEN], dst[STRLEN];
	int i, fail = 0, netty_check = 0;
	FILE *fin, *fout, *dp;
	time_t code;

	memcpy(&newinfo, u, sizeof(currentuser));
	getdata(t_lines - 1, 0,
			real ? "ÇëÑ¡Ôñ (0)½áÊø (1)ĞŞ¸Ä×ÊÁÏ (2)Éè¶¨ÃÜÂë (3) ¸Ä ID ==> [0]" : "ÇëÑ¡Ôñ (0)½áÊø (1)ĞŞ¸Ä×ÊÁÏ (2)Éè¶¨ÃÜÂë (3) Ñ¡Ç©Ãûµµ ==> [0]",
			ans, 2, DOECHO, YEA);
	clear();

	i = 3;
	move(i++, 0);
	if (ans[0] != '3' || real)
		prints("Ê¹ÓÃÕß´úºÅ: %s\n", u->userid);

	switch (ans[0]) {
	case '1':
		move(1, 0);
		prints("ÇëÖğÏîĞŞ¸Ä,Ö±½Ó°´ <ENTER> ´ú±íÊ¹ÓÃ [] ÄÚµÄ×ÊÁÏ¡£\n");

		sprintf(genbuf, "êÇ³Æ [%s]: ", u->username);
		getdata(i++, 0, genbuf, buf, NAMELEN, DOECHO, YEA);
		if (buf[0])
			strncpy(newinfo.username, buf, NAMELEN);
		if (!real && buf[0])
			strncpy(uinfo.username, buf, 40);

		sprintf(genbuf, "ÕæÊµĞÕÃû [%s]: ", u->realname);
		getdata(i++, 0, genbuf, buf, NAMELEN, DOECHO, YEA);
		if (buf[0])
			strncpy(newinfo.realname, buf, NAMELEN);

		sprintf(genbuf, "¾Ó×¡µØÖ· [%s]: ", u->address);
		getdata(i++, 0, genbuf, buf, NAMELEN, DOECHO, YEA);
		if (buf[0])
			strncpy(newinfo.address, buf, NAMELEN);

#ifndef POP_CHECK  // ²»ÈÃ¸ÄĞÅÏäµØÖ·ÁË£¬ÒòÎªÒª°ó¶¨
		sprintf(genbuf, "µç×ÓĞÅÏä [%s]: ", u->email);
		getdata(i++, 0, genbuf, buf, STRLEN - 10, DOECHO, YEA);
		if (buf[0])
		{
			strncpy(newinfo.email, buf, STRLEN);
		}
#endif

		sprintf(genbuf, "ÓòÃûÖ¸Ïò [%s]: ", u->ip);
		getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
		if (buf[0])
			strncpy(newinfo.ip, buf, 16);

		if (real) {
			sprintf(genbuf, "ÕæÊµEmail[%s]: ", u->realmail);
			getdata(i++, 0, genbuf, buf, STRLEN - 10, DOECHO, YEA);
			if (buf[0])
				strncpy(newinfo.realmail, buf, STRLEN - 16);

			sprintf(genbuf, "ÉÏÏß´ÎÊı [%d]: ", u->numlogins);
			getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
			if (atoi(buf) > 0)
				newinfo.numlogins = atoi(buf);

			sprintf(genbuf, "ÎÄÕÂÊıÄ¿ [%d]: ", u->numposts);
			getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
			if (atoi(buf) > 0)
				newinfo.numposts = atoi(buf);

			sprintf(genbuf, "ÉÏÕ¾Ğ¡Ê±Êı [%ld Ğ¡Ê± %ld ·ÖÖÓ]: ",
					(long int ) (u->stay / 3600),
					(long int ) ((u->stay / 60) % 60));
			getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
			if (atoi(buf) > 0)
				newinfo.stay = atoi(buf) * 3600;

		}

		break;
	case '2':
		if (!real) {
			getdata(i++, 0, "ÇëÊäÈëÔ­ÃÜÂë: ", buf, PASSLEN, NOECHO,
			YEA);
			if (*buf == '\0' || !checkpasswd(u->passwd, buf)) {
				prints("\n\nºÜ±§Ç¸, ÄúÊäÈëµÄÃÜÂë²»ÕıÈ·¡£\n");
				fail++;
				break;
			}
		}
		getdata(i++, 0, "ÇëÉè¶¨ĞÂÃÜÂë: ", buf, PASSLEN, NOECHO, YEA);
		if (buf[0] == '\0') {
			prints("\n\nÃÜÂëÉè¶¨È¡Ïû, ¼ÌĞøÊ¹ÓÃ¾ÉÃÜÂë\n");
			fail++;
			break;
		}
		strncpy(genbuf, buf, PASSLEN);

		getdata(i++, 0, "ÇëÖØĞÂÊäÈëĞÂÃÜÂë: ", buf, PASSLEN, NOECHO,
		YEA);
		if (strncmp(buf, genbuf, PASSLEN)) {
			prints("\n\nĞÂÃÜÂëÈ·ÈÏÊ§°Ü, ÎŞ·¨Éè¶¨ĞÂÃÜÂë¡£\n");
			fail++;
			break;
		}
		buf[8] = '\0';
		strncpy(newinfo.passwd, genpasswd(buf), PASSLEN);
		break;
	case '3':
		if (!real) {
			sprintf(genbuf, "Ä¿Ç°Ê¹ÓÃÇ©Ãûµµ [%d]: ", u->signature);
			getdata(i++, 0, genbuf, buf, 16, DOECHO, YEA);
			if (atoi(buf) > 0)
				newinfo.signature = atoi(buf);
		} else {
			getdata(i++, 0, "ĞÂµÄÊ¹ÓÃÕß´úºÅ: ", genbuf, IDLEN + 1,
			DOECHO, YEA);
			if (*genbuf != '\0') {
				if (getuser(genbuf)) {
					prints("\n´íÎó! ÒÑ¾­ÓĞÍ¬Ñù ID µÄÊ¹ÓÃÕß\n");
					fail++;
				} else if (!goodgbid(genbuf)) {
					prints("\n´íÎó! ²»ºÏ·¨µÄ ID\n");
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
	if (askyn("È·¶¨Òª¸Ä±äÂğ", NA, YEA) == YEA) {
		if (real) {
			char secu[STRLEN];
			sprintf(secu, "ĞŞ¸Ä %s µÄ»ù±¾×ÊÁÏ»òÃÜÂë¡£", u->userid);
			securityreport(secu, secu);
		}
		if (strcmp(u->userid, newinfo.userid)) {

			sprintf(src, "mail/%c/%s", mytoupper(u->userid[0]), u->userid);
			sprintf(dst, "mail/%c/%s", mytoupper(newinfo.userid[0]),
					newinfo.userid);
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
				sprintf(genbuf, "/usr/lib/sendmail -f %s.bbs@%s %s ", u->userid,
						email_domain(), newinfo.email);
				fout = popen(genbuf, "w");
				fin = fopen(sysconf_str("EMAILFILE"), "r");
				if (fin == NULL || fout == NULL)
					return -1;
				fprintf(fout, "Reply-To: SYSOP.bbs@%s\n", email_domain());
				fprintf(fout, "From: SYSOP.bbs@%s\n", email_domain());
				fprintf(fout, "To: %s\n", newinfo.email);
				fprintf(fout, "Subject: @%s@[-%9.9ld-]%s mail check.\n",
						u->userid, (long int) code, MY_BBS_ID);
				fprintf(fout, "X-Forwarded-By: SYSOP \n");
				fprintf(fout, "X-Disclaimer: %s registration mail.\n",
				MY_BBS_ID);
				fprintf(fout, "\n");
				fprintf(fout, "BBS LOCATION     : %s (%s)\n", email_domain(),
				MY_BBS_IP);
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
					prints("\nÄúËùÌîµÄµç×ÓÓÊ¼şµØÖ· ¡¾[1;33m%s[m¡¿\n", newinfo.email);
					prints("²¢·ÇºÏ·¨Ö® UNIX ÕÊºÅ£¬ÏµÍ³²»»áÍ¶µİ×¢²áĞÅ£¬Çë°ÑËüĞŞÕıºÃ...\n");
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

	sprintf(genbuf, "  Ô­ÏÈÉè¶¨: %-46.46s [1;32m(%s)[m",
			(buf[0] == '\0') ? "(Î´Éè¶¨)" : buf, info);
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

//	int lockfd;	// ´Ë´¦¸úËæÏÂ·½×¢ÊÍ´úÂëÔİÊ±×¢ÊÍµô£¬by IronBlood£¬
	struct active_data act_data;
	int index;

	modify_user_mode(NEW);
	move(3, 0);
	clrtobot();
	if (!strcmp("guest", currentuser.userid)) {
		prints("±§Ç¸, ÇëÓÃ new ÉêÇëÒ»¸öĞÂÕÊºÅºóÔÙÌîÉêÇë±í.");
		pressreturn();
		return;
	}
	if (currentuser.userlevel & PERM_LOGINOK) {
		prints("ÄúÒÑ¾­Íê³É±¾Õ¾µÄÊ¹ÓÃÕß×¢²áÊÖĞø, »¶Ó­¼ÓÈë±¾Õ¾µÄĞĞÁĞ.");
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
				prints("Õ¾³¤ÉĞÎ´´¦ÀíÄúµÄ×¢²áÉêÇëµ¥, ÇëÄÍĞÄµÈºò.");
				pressreturn();
				return;
			}
		}
		fclose(fn);
	}
	move(3, 0);
	sprintf(genbuf, "ÄúÒªÌîĞ´×¢²áµ¥£¬¼ÓÈë%s´ó¼ÒÍ¥Âğ£¿", MY_BBS_NAME);
	if (askyn(genbuf, YEA, NA) == NA)
		return;
	strncpy(rname, currentuser.realname, NAMELEN);
	strncpy(addr, currentuser.address, STRLEN);
	dept[0] = phone[0] = assoc[0] = '\0';
	while (1) {
		move(3, 0);
		clrtoeol();
		prints("%s ÄúºÃ, Çë¾İÊµÌîĞ´ÒÔÏÂµÄ×ÊÁÏ:\n", currentuser.userid);
		getfield(6, "ÇëÓÃÖĞÎÄ", "ÕæÊµĞÕÃû", rname, NAMELEN);
		getfield(8, "Ñ§Ğ£Ïµ¼¶»ò¹«Ë¾Ö°³Æ", "Ñ§Ğ£Ïµ¼¶»ò¹¤×÷µ¥Î»", dept,
		STRLEN);
		getfield(10, "°üÀ¨ÇŞÊÒ»òÃÅÅÆºÅÂë", "Ä¿Ç°×¡Ö·»òÍ¨Ñ¶µØÖ·", addr,
		STRLEN);
		getfield(12, "°üÀ¨¿ÉÁªÂçÊ±¼ä", "ÁªÂçµç»°", phone, STRLEN);
		getfield(14, "Ğ£ÓÑ»á»ò±ÏÒµÑ§Ğ£", "Ğ£ ÓÑ »á", assoc, STRLEN);
		/* only for 9#        getfield( 14, "½éÉÜÈËIDºÍÕæÊµĞÕÃû",    "½éÉÜÈË(ÍâÀ´µÄIDĞëÌî:ID/ĞÕÃû)",   assoc, STRLEN );
		 */
		mesg = "ÒÔÉÏ×ÊÁÏÊÇ·ñÕıÈ·, °´ Q ·ÅÆú×¢²á (Y/N/Quit)? [N]: ";
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

	/*
	 lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX);
	 if ((fn = fopen("new_register", "a")) != NULL) {
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
	 */
	/*
	 setuserfile(genbuf, "mailcheck");
	 if ((fn = fopen(genbuf, "w")) == NULL) {
	 fclose(fn);
	 return;
	 }
	 fprintf(fn, "usernum: %d\n", usernum);
	 fclose(fn);
	 */

	// ÒÔÏÂÒªÓÃ»§Ñ¡ÔñÊÇ·ñÒªÍ¨¹ıÓÊ¼ş·şÎñÆ÷½øĞĞÉóºË£¬ added by interma@BMY 2005.5.12
	clear();
	move(3, 0);
	prints("ÏÂÃæ½«½øĞĞÊµÃûÈÏÖ¤¡£±¾Õ¾Ä¿Ç°Ö§³ÖÒÔÏÂÓòÃûµÄµç×ÓĞÅÏä½øĞĞÈÏÖ¤. \n");
	prints("Ã¿¸öĞÅÏä¿ÉÒÔÈÏÖ¤ %d ¸öid.\n\n", MAX_USER_PER_RECORD);
	for (index = 1; index <= DOMAIN_COUNT; ++index) {
		prints("[%d] %s \n", index, MAIL_DOMAINS[index]);
	}
	char tempn[3];
	int n = -1;
	while (!(n > 0 && n <= DOMAIN_COUNT)) {
		getdata(10, 0, "ÇëÑ¡ÔñÄãµÄĞÅÏäÓòÃûĞòºÅ£¨ĞÂÉú×¢²áÇëÑ¡Ôñ1£© >>  ", tempn, 3, DOECHO, YEA);
		sscanf(tempn, "%d", &n);
	}

	char user[USER_LEN + 1];
	char pass[PASS_LEN + 1];

	getdata(13, 0, "ĞÅÏäÓÃ»§Ãû(ÊäÈëx·ÅÆúÑéÖ¤£¬ĞÂÉú×¢²áÇëÊäÈëÓÃ»§Ãûtest£¬ÃÜÂëtest) >>  ", user, USER_LEN,
	DOECHO, YEA);
	getdata(14, 0, "ĞÅÏäÃÜÂë >>  ", pass, PASSLEN, NOECHO, YEA);

	while (test_mail_valid(user, pass, IP_POP[n]) != 1) {
		if (strcmp(user, "x") == 0) {
			return;
		}
		if (strcmp(user, "test") == 0) {
			clear();
			move(5, 0);
			prints("»¶Ó­Äú¼ÓÈë½»´ó£¬À´µ½±øÂíÙ¸BBS¡£\nÄú²ÉÓÃÁËĞÂÉú²âÊÔĞÅÏä×¢²á£¬Ä¿Ç°ÄúÊÇĞÂÉúÓÃ»§Éí·İ¡£");
			prints("Ä¿Ç°ÄúÃ»ÓĞ·¢ÎÄ¡¢ĞÅ¼ş¡¢ÏûÏ¢µÈÈ¨ÏŞ¡£\n\n");
			prints("ÇëÔÚ¿ªÑ§È¡µÃstu.xjtu.edu.cnĞÅÏäºó£¬\n°´ÕÕÉÏÕ¾µÇÂ¼ºóµÄÌáÊ¾Íê³ÉĞÅÏä°ó¶¨ÈÏÖ¤²Ù×÷£¬³ÉÎª±¾Õ¾ÕıÊ½ÓÃ»§¡£");
			pressanykey();
			return;
		}
		move(11, 0);
		clrtobot();
		move(12, 0);
		prints("ÈÏÖ¤Ê§°Ü£¬Çë¼ì²éºóÖØĞÂÊäÈë.");
		getdata(13, 0, "ĞÅÏäÓÃ»§Ãû(ÊäÈëx·ÅÆúÑéÖ¤£¬ĞÂÉú×¢²áÇëÊäÈëÓÃ»§Ãûtest£¬ÃÜÂëtest) >>  ", user,
		USER_LEN, DOECHO, YEA);
		getdata(14, 0, "ĞÅÏäÃÜÂë >>  ", pass, PASSLEN, NOECHO, YEA);
	}

	char email[STRLEN];
	strcpy(email, str_to_lowercase(user));
	strcat(email, "@");
	strcat(email, MAIL_DOMAINS[n]);

	//FILE* fp;
	char path[128];
	sprintf(path, MY_BBS_HOME "/etc/pop_register/%s_privilege",
			MAIL_DOMAINS[n]);
	int isprivilege = 0;

	if (seek_in_file(path, user)) {
		isprivilege = 1;
	}

	if (query_record_num(email, MAIL_ACTIVE) >= MAX_USER_PER_RECORD
			&& isprivilege == 0) {
		clear();
		move(3, 0);
		prints("ÄúµÄĞÅÏäÒÑ¾­ÑéÖ¤¹ı %d ¸öid£¬ÎŞ·¨ÔÙÓÃÓÚÑéÖ¤ÁË!\n", MAX_USER_PER_RECORD);
		pressreturn();
		return;
	}

	int response;

	strcpy(act_data.email, email);
	act_data.status = 1;
	response = write_active(&act_data);

	if (response == WRITE_SUCCESS || response == UPDATE_SUCCESS) {
		clear();
		move(5, 0);
		prints("Éí·İÉóºË³É¹¦£¬ÄúÒÑ¾­¿ÉÒÔÊ¹ÓÃËùÓÃ¹¦ÄÜÁË£¡\n");
		strncpy(currentuser.email, email, STRLEN);
		register_success(usernum, currentuser.userid, rname, dept, addr, phone,
				assoc, email);

		//scroll();
		pressreturn();
		return;
	}
	clear();
	move(3, 0);
	prints("  ÑéÖ¤Ê§°Ü!");
	pressreturn();
	return;

	/*
	 struct stat temp;
	 if (stat(MY_BBS_HOME "/etc/pop_register/pop_list", &temp) == -1)
	 {
	 prints("Ä¿Ç°Ã»ÓĞ¿ÉÒÔĞÅÈÎµÄÓÊ¼ş·şÎñÆ÷ÁĞ±í, Òò´ËÎŞ·¨ÑéÖ¤ÓÃ»§£¡\n");
	 //register_fail(currentuser.userid);
	 scroll();
	 pressreturn();
	 exit(1);
	 }

	 FILE *fp;
	 fp = fopen(MY_BBS_HOME "/etc/pop_register/pop_list", "r");
	 if (fp == NULL)
	 {
	 prints("´ò¿ª¿ÉÒÔĞÅÈÎµÄÓÊ¼ş·şÎñÆ÷ÁĞ±í³ö´í, Òò´ËÎŞ·¨ÑéÖ¤ÓÃ»§£¡\n");
	 //register_fail(currentuser.userid);
	 scroll();
	 pressreturn();
	 exit(1);
	 }

	 char bufpop[256];
	 int numpop = 0;
	 char namepop[10][256]; // ×¢Òâ£º×î¶àĞÅÈÎ10¸öpop·şÎñÆ÷£¬Òª²»¾ÍÒç³öÁË£¡
	 char ippop[10][256];

	 prints("Ä¿Ç°¿ÉÒÔĞÅÈÎµÄÓÊ¼ş·şÎñÆ÷ÁĞ±í: \n");

	 while(fgets(bufpop, 256, fp) != NULL)
	 {
	 if (strcmp(bufpop, "") == 0 || strcmp(bufpop, " ") == 0 || strcmp(bufpop, "\n") == 0)
	 break;
	 strcpy(namepop[numpop], bufpop);
	 fgets(bufpop, 256, fp);
	 strcpy(ippop[numpop], bufpop);

	 //scroll();
	 prints("[%d] %s\n", numpop + 1, namepop[numpop]);
	 numpop ++;
	 }
	 fclose(fp);

	 char tempn[3];
	 int n = -1;

	 while (!(n > 0 && n <= numpop))
	 {
	 getdata(t_lines - 1, 0, "ÇëÑ¡ÔñÄãµÄÓÊ¼ş·şÎñÆ÷: £¨ÊäÈëĞòºÅ£©", tempn, 3, DOECHO, YEA);
	 scroll();
	 sscanf(tempn, "%d", &n);
	 }

	 // ÒÔÏÂ¿ªÊ¼Í¨¹ıÓÊ¼ş·şÎñÆ÷½øĞĞÉóºË

	 scroll();
	 char user[USER_LEN + 1];
	 char pass[PASS_LEN + 1];

	 int i = 0;
	 int result;

	 clear();

	 while (i < 3) // 3ÎªÔÊĞíÖØÊÔµÄ´ÎÊı
	 {
	 getdata(1, 0, "ÇëÊäÈëÓÊ¼ş·şÎñÆ÷ÉÏµÄÓÃ»§Ãû:  ", user, USER_LEN, DOECHO, YEA);
	 scroll();
	 getdata(1, 0, "ÇëÊäÈëÓÊ¼ş·şÎñÆ÷ÉÏµÄÃÜÂë:    ", pass, PASS_LEN, NOECHO, YEA);
	 scroll();
	 scroll();

	 result = test_mail_valid(user, pass, ippop[n - 1]);
	 switch (result)
	 {
	 case 0:
	 prints("Éí·İÉóºËÊ§°Ü£¬ÇëÖØÊÔ                       \n");
	 scroll(); break;
	 case -1:
	 prints("ÓÊ¼ş·şÎñÆ÷Á¬½Ó³ö´í£¬ÇëÖØÊÔ                  \n"); scroll(); break;
	 case 1:
	 // prints("Éí·İÉóºË³É¹¦£¬ÄúÒÑ¾­¿ÉÒÔÊ¹ÓÃËùÓÃ¹¦ÄÜÁË£¡\n");
	 i = 3;
	 break;

	 }
	 i++;
	 } // end of while

	 switch (result)
	 {
	 case -1:
	 case 0:
	 prints("3´ÎÉí·İÉóºË¾ùÊ§°Ü£¬Äú½«Ö»ÄÜÊ¹ÓÃ±¾bbsµÄ×î»ù±¾¹¦ÄÜ£¬Ê®·Ö±§Ç¸\n");

	 //register_fail(currentuser.userid);

	 scroll();
	 pressreturn();
	 return;
	 break;

	 case 1:
	 namepop[n - 1][strlen(namepop[n - 1]) - 1] = 0;
	 if (write_pop_user(user, currentuser.userid, namepop[n - 1]) == 1)
	 {
	 prints("ÄúÒÑ¾­Ê¹ÓÃ¸ÃĞÅÏä×¢²á¹ıIDÁË,Òò´ËÄúÎŞ·¨×¢²áÕâ¸öID,Ê®·Ö±§Ç¸\n");
	 //register_fail(currentuser.userid);

	 scroll();
	 pressreturn();
	 return;
	 }

	 prints("Éí·İÉóºË³É¹¦£¬ÄúÒÑ¾­¿ÉÒÔÊ¹ÓÃËùÓÃ¹¦ÄÜÁË£¡\n");

	 char email[256];
	 sprintf(email, "%s@%s", user, namepop[n - 1]);
	 strncpy(currentuser.email, email, STRLEN);

	 register_success(usernum, currentuser.userid, rname, dept, addr, phone, assoc, email);

	 scroll();
	 pressreturn();
	 break;

	 }
	 */

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
		prints("±§Ç¸, ÇëÓÃ new ÉêÇëÒ»¸öĞÂÕÊºÅºóÔÙÌîÉêÇë±í.");
		pressreturn();
		return;
	}
	if (currentuser.userlevel & PERM_LOGINOK)
	{
		prints("ÄúÒÑ¾­Íê³É±¾Õ¾µÄÊ¹ÓÃÕß×¢²áÊÖĞø, »¶Ó­¼ÓÈë±¾Õ¾µÄĞĞÁĞ.");
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
				("Õ¾³¤ÉĞÎ´´¦ÀíÄúµÄ×¢²áÉêÇëµ¥, ÇëÄÍĞÄµÈºò.");
				pressreturn();
				return;
			}
		}
		fclose(fn);
	}
	move(3, 0);
	sprintf(genbuf, "ÄúÒªÌîĞ´×¢²áµ¥£¬¼ÓÈë%s´ó¼ÒÍ¥Âğ£¿", MY_BBS_NAME);
	if (askyn(genbuf, YEA, NA) == NA)
	return;
	strncpy(rname, currentuser.realname, NAMELEN);
	strncpy(addr, currentuser.address, STRLEN);
	dept[0] = phone[0] = assoc[0] = '\0';
	while (1)
	{
		move(3, 0);
		clrtoeol();
		prints("%s ÄúºÃ, Çë¾İÊµÌîĞ´ÒÔÏÂµÄ×ÊÁÏ:\n", currentuser.userid);
		getfield(6, "ÇëÓÃÖĞÎÄ", "ÕæÊµĞÕÃû", rname, NAMELEN);
		getfield(8, "Ñ§Ğ£Ïµ¼¶»ò¹«Ë¾Ö°³Æ", "Ñ§Ğ£Ïµ¼¶»ò¹¤×÷µ¥Î»", dept,
				STRLEN);
		getfield(10, "°üÀ¨ÇŞÊÒ»òÃÅÅÆºÅÂë", "Ä¿Ç°×¡Ö·»òÍ¨Ñ¶µØÖ·", addr,
				STRLEN);
		getfield(12, "°üÀ¨¿ÉÁªÂçÊ±¼ä", "ÁªÂçµç»°", phone, STRLEN);
		getfield(14, "Ğ£ÓÑ»á»ò±ÏÒµÑ§Ğ£", "Ğ£ ÓÑ »á", assoc, STRLEN);
		/* only for 9#        getfield( 14, "½éÉÜÈËIDºÍÕæÊµĞÕÃû",    "½éÉÜÈË(ÍâÀ´µÄIDĞëÌî:ID/ĞÕÃû)",   assoc, STRLEN );
		 */
		mesg = "ÒÔÉÏ×ÊÁÏÊÇ·ñÕıÈ·, °´ Q ·ÅÆú×¢²á (Y/N/Quit)? [N]: ";
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
