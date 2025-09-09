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
extern char *sysconf_str(char *);
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

	prints("Ѱ�����ʺ���, ���Դ�Ƭ��...\n\r");
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
			prints("��Ǹ, ʹ�����ʺ��Ѿ�����, �޷�ע���µ��ʺ�.\n\r");
		}
		prints("��ȴ� %d ���Ӻ�����һ��, ף�����.\n\r", val);
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
			prints("\n��������̫����  <Enter> ��...\n");
			refresh();
			longjmp(byebye, -1);
		}
		move(t_lines - 6, 0);
		prints("�ʺ�����Ϊ���ڱ�վ��ʵ����ʾ���û����ƣ������޸ģ�������ѡ�� ");
		getdata(t_lines - 5, 0,
				"�������ʺ����� (Enter User ID, \"0\" to abort): ",
				newuser.userid, IDLEN + 1, DOECHO, YEA);
		if (newuser.userid[0] == '0') {
			longjmp(byebye, -1);
		}
		if (id_with_num(newuser.userid)) {
			prints("�ʺű���ȫΪӢ����ĸ!\n");
		} else if (strlen(newuser.userid) < 2) {
			prints("�ʺ�������������Ӣ����ĸ!\n");
		} else if ((*newuser.userid == '\0') || is_bad_id(newuser.userid)) {
			prints("��Ǹ, ������ʹ���������Ϊ�ʺš� ���������һ����\n");
		} else if (dosearchuser(newuser.userid)) {
			prints("���ʺ��Ѿ�����ʹ��\n");
		} else
			break;
	}
	while (1) {
		getdata(t_lines - 4, 0, "���趨�������� (Setup Password): ", passbuf, PASSLEN, NOECHO, YEA);
		if (strlen(passbuf) < 4 || !strcmp(passbuf, newuser.userid)) {
			prints("����̫�̻���ʹ���ߴ�����ͬ, ����������\n");
			continue;
		}
		ytht_strsncpy(newuser.passwd, passbuf, PASSLEN);
		getdata(t_lines - 3, 0, "��������һ��������� (Reconfirm Password): ", passbuf, PASSLEN, NOECHO, YEA);
		if (strncmp(passbuf, newuser.passwd, PASSLEN - 1) != 0) {
			prints("�����������, ��������������.\n");
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

	//�ж��Ƿ�ʹ��emailע��... �����ǲ������������жϵ�? --ylsdd
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
			prints("������!! ����˳����ɱ�վ��ʹ����ע������,\n");
			prints("������������ӵ��һ��ʹ���ߵ�Ȩ��������...\n");
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
//20060313�޸� by clearboy(������)
//����������һ���鷳������,telnet��½��������˳���
//��Ҹ����˺ܾã�����������sysconf_eval
//��һ�������⣬����/home/bbs/deverrlog��������¼�ļ�������2�G
//����sysconf_eval���Ǳ���ģ�����ɾ������
	perm = PERM_DEFAULT;// & sysconf_eval("AUTOSET_PERM");


	while ((strlen(urec->username) < 2)) {
		getdata(2, 0, "�����������ǳ� (Enter nickname): ",
			urec->username, NAMELEN, DOECHO, YEA);
		strcpy(uinfo.username, urec->username);
		update_utmp();
	}
	while ((strlen(urec->realname) < 4)
			|| (strstr(urec->realname, "  "))
			|| (strstr(urec->realname, "��"))) {
		move(3, 0);
		prints("������������ʵ���� (Enter realname):\n");
		getdata(4, 0, "> ", urec->realname, NAMELEN, DOECHO, YEA);
	}
	while ((strlen(urec->address) < 10) || (strstr(urec->address, "   "))) {
		move(5, 0);
		prints("����������ͨѶ��ַ (Enter home address)��\n");
		getdata(6, 0, "> ", urec->address, NAMELEN, DOECHO, YEA);
	}
	#ifndef POP_CHECK
	if (strchr(urec->email, '@') == NULL) {
		move(8, 0);
		prints("���������ʽΪ: \033[1;37muserid@your.domain.name\033[m\n");
		prints("������������� (�����ṩ�߰� <Enter>)");
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
			#ifndef POP_CHECK /* ��ֹ���ʬ��󣬲���������������interma@BMY*/
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
				prints("���ĵ������� ����ͨ��������֤...  \n");
				prints("    ��վ�����ϼ�һ����֤�Ÿ���,\n");
				prints("    ��ֻҪ�� %s ����, �Ϳ��Գ�Ϊ��վ�ϸ���.\n\n", urec->email);
				prints("    ��Ϊ��վ�ϸ���, �������и����Ȩ���!\n");
				move(20, 0);
				if (askyn("��Ҫ�������ھͼ���һ������", YEA, NA) == YEA) {
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
					getdata(21, 0, "ȷ�����Ѽĳ�, ��������Ŷ!! �밴 <Enter> : ", ans, 2, DOECHO, YEA);
				}
			} else {
				showansi = 1;
				if (sysconf_str("EMAILFILE") != NULL) {
					prints("\n������д�ĵ����ʼ���ַ ��\033[1;33m%s\033[m��\n", urec->email);
					prints("���ǺϷ�֮ UNIX �ʺţ�ϵͳ����Ͷ��ע���ţ��뵽\033[1;32mInfoEdit->Info\033[m���޸�...\n");
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
				fprintf(fout, "��Һ�,\n\n");
				fprintf(fout, "���� %s (%s), ���� %s\n",
					currentuser.userid, urec->username,
					fromhost);
				fprintf(fout, "�����ҳ�����վ����, ���Ҷ��ָ�̡�\n");
				move(5, 0);
				prints("��������̵ĸ��˼��, ��վ����ʹ���ߴ���к�\n");
				prints("(�������, д���ֱ�Ӱ� <Enter> ����)....");
				getdata(7, 0, ":", buf2, 75, DOECHO, YEA);
				if (buf2[0] != '\0') {
					fprintf(fout, "\n\n���ҽ���:\n\n");
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
				postfile(buf, "newcomers", "������·....", 2);
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

