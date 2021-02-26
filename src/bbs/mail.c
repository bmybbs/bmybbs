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

#include <stddef.h>
#include "bbs.h"
#include "ythtbbs/override.h"
#include "bbs_global_vars.h"
#include "smth_screen.h"
#include "mail.h"
#include "postheader.h"
#include "io.h"
#include "xyz.h"
#include "stuff.h"
#include "read.h"
#include "more.h"
#include "record.h"
#include "main.h"
#include "list.h"
#include "namecomplete.h"
#include "bcache.h"
#include "edit.h"
#include "bbsinc.h"
#include "sendmsg.h"
#include "help.h"
#include "talk.h"
#include "power_select.h"
#include "bbs-internal.h"

int in_mail;
int G_SENDMODE = NA;

char currmaildir[STRLEN];
int filter_ansi;
int mailmode, mailhour;
time_t mailallmode = 0;
#define maxrecp 300

static int do_send(char *userid, char *title);
static int read_mail(struct fileheader *fptr);
static int read_new_mail(struct fileheader *fptr);
static int mailtitle(void);
static char *maildoent(int num, struct fileheader *ent, char buf[512]);
static int mail_read(int ent, struct fileheader *fileinfo, char *direct);
static int mail_del(int ent, struct fileheader *fileinfo, char *direct);
static int mail_del_range(int ent, struct fileheader *fileinfo, char *direct);
static int mail_mark(int ent, struct fileheader *fileinfo, char *direct);
static int do_gsend(const char *userid[], int num);
static void getmailinfo(char *path, struct fileheader *rst);
static int mail_rjunk(void);
static int m_cancel_1(struct fileheader *fh, char *receiver);
static int max_mail_size(void);
static int get_mail_size(void);
static int check_maxmail();
static int check_mail_perm();
static int show_user_notes();
static int mailto(struct userec *uentp);
static int mailtoall(int mode, int hour);

#ifdef INTERNET_EMAIL

#ifdef SENDMAIL_MIME_AUTOCONVERT
static int bbs_sendmail(char *fname, char *title, char *receiver, int mime);
#else
static int bbs_sendmail(char *fname, char *title, char *receiver);
#endif
#endif

char *
email_domain()
{
	return MY_BBS_DOMAIN;
}

void
filter(char *line)
{
	char temp[256];
	int i, stat, j;
	stat = 0;
	j = 0;
	for (i = 0; line[i] && i < 255; i++) {
		if (line[i] == '\033')
			stat = 1;
		if (!stat)
			temp[j++] = line[i];
		if (stat && ((line[i] > 'a' && line[i] < 'z')
					|| (line[i] > 'A' && line[i] < 'Z')
					|| line[i] == '@'))
			stat = 0;
	}
	temp[j] = 0;
	strcpy(line, temp);
}

int
chkmail()
{
	static time_t lasttime = 0;
	static int ismail = 0;
	struct stat st;
	int fd;
	int i;
	size_t offset;
	int numfiles;
	int accessed;
	extern char currmaildir[STRLEN];

	if (!HAS_PERM(PERM_BASIC, currentuser)) {
		return 0;
	}
	offset = offsetof(struct fileheader, accessed);

	if (stat(currmaildir, &st) < 0)
		return (ismail = 0);
	if (lasttime >= st.st_mtime)
		return ismail;

	if ((fd = open(currmaildir, O_RDONLY)) < 0)
		return (ismail = 0);
	lasttime = st.st_mtime;
	numfiles = st.st_size / sizeof (struct fileheader);
	if (numfiles <= 0) {
		close(fd);
		return (ismail = 0);
	}
	lseek(fd, (st.st_size - (sizeof (struct fileheader) - offset)), SEEK_SET);
	for (i = 0; i < numfiles && i < 10; i++) {
		read(fd, &accessed, sizeof (accessed));
		if (!(accessed & FH_READ)) {
			close(fd);
			return (ismail = 1);
		}
		lseek(fd, -sizeof (struct fileheader) - sizeof (accessed), SEEK_CUR);
	}
	close(fd);
	return (ismail = 0);
}

int
check_query_mail(qry_mail_dir)
char qry_mail_dir[STRLEN];
{
	struct stat st;
	int fd;
	size_t offset;
	int numfiles;
	int accessed;

	offset = offsetof(struct fileheader, accessed);
	if ((fd = open(qry_mail_dir, O_RDONLY)) < 0)
		return 0;
	fstat(fd, &st);
	numfiles = st.st_size / sizeof (struct fileheader);
	if (numfiles <= 0) {
		close(fd);
		return 0;
	}
	lseek(fd, (st.st_size - (sizeof (struct fileheader) - offset)), SEEK_SET);
/*离线查询新信只要查询最后一封是否为新信，其他并不重要*/
/*Modify by SmallPig*/
	read(fd, &accessed, sizeof (accessed));
	if (!(accessed & FH_READ)) {
		close(fd);
		return YEA;
	}
	close(fd);
	return NA;
}

int mailall(const char *s) {
	(void) s;
	char ans[4], fname[STRLEN], title[STRLEN];
	const char *doc[6] = {
		"(1) 尚未通过身份确认的使用者",
		"(2) 所有通过身份确认的使用者",
		"(3) 所有的版主",
		"(4) 本站智囊团",
		"(5) 所有本组织会员",
		"(6) 所有SYSOP",
	};
	char buf[STRLEN], str[7];
	int i;
	int hour;
	int save_in_mail;

	strcpy(title, "没主题");
	modify_user_mode(SMAIL);
	clear();
	move(0, 0);
	sprintf(fname, "tmp/mailall.%s", currentuser.userid);
	prints("你要寄给所有的：\n");
	prints("(0) 放弃\n");
	for (i = 0; i < 6; i++)
		prints("%s\n", doc[i]);
	getdata(9, 0, "请输入模式 (0~6)? [0]: ", ans, 2, DOECHO, YEA);
	if (ans[0] - '0' < 1 || ans[0] - '0' > 6) {
		return NA;
	}
	getdata(10, 0, "请输入上站时间下限(按小时)[0]: ", str, 5, DOECHO, YEA);
	hour = atoi(str);
	sprintf(buf, "是否确定寄给%s 并且上站时间不小于%d小时?",
		doc[ans[0] - '0' - 1], hour);
	move(11, 0);
	if (askyn(buf, NA, NA) == NA)
		return NA;
	save_in_mail = in_mail;
	in_mail = YEA;
	header.reply_mode = NA;
	strcpy(header.title, "没主题");
	strcpy(header.ds, doc[ans[0] - '0' - 1]);
	header.postboard = NA;
	if (post_header(&header))
		sprintf(save_title, "[Type %c 公告] %.60s", ans[0],
			header.title);
	setquotefile("");
	do_quote(fname, header.include_mode);
	if (vedit(fname, YEA, YEA) == -1) {
		in_mail = save_in_mail;
		unlink(fname);
		clear();
		return -2;
	}
	add_loginfo(fname);
	move(t_lines - 1, 0);
	clrtoeol();
	prints("\033[5;1;32;44m正在寄件中，请稍候.....                                                        \033[m");
	refresh();
	mailtoall(ans[0] - '0', hour);
	move(t_lines - 1, 0);
	clrtoeol();
	unlink(fname);
	in_mail = save_in_mail;
	return 0;
}

#ifdef INTERNET_EMAIL

int m_internet(const char *s) {
	(void) s;
	char receiver[STRLEN];

	if (check_maxmail()) {
		pressreturn();
		return 0;
	}
	if (check_mail_perm()) {
		pressreturn();
		return 0;
	}
	modify_user_mode(SMAIL);

	getdata(1, 0, "收信人E-mail：", receiver, 65, DOECHO, YEA);
	sprintf(genbuf, ".bbs@%s", email_domain());
	if (strstr(receiver, genbuf) || strstr(receiver, ".bbs@localhost")) {
		move(3, 0);
		prints("站内信件, 请用 (S)end 指令来寄\n");
		pressreturn();
	} else if (!invalidaddr(receiver)) {
		*quote_file = '\0';
		clear();
		do_send(receiver, NULL);
	} else {
		move(3, 0);
		prints("收信人不正确, 请重新选取指令\n");
		pressreturn();
	}
	clear();
	return 0;
}
#endif

void
m_init()
{
	sprintf(currmaildir, "mail/%c/%s/%s", mytoupper(currentuser.userid[0]),
		currentuser.userid, DOT_DIR);
}

static int
do_send(userid, title)
char *userid, *title;
{
	struct fileheader newmessage;
	struct stat st;
	char filepath[STRLEN], fname[STRLEN];
	char save_title2[STRLEN];
	int fp, count;
	int internet_mail = 0;
	char tmp_fname[STRLEN];
	char uid[80];
	time_t now;
	int save_in_mail;

	strcpy(uid, userid);
	/* I hate go to , but I use it again for the noodle code :-) */
	if (strchr(userid, '@')) {
		internet_mail = YEA;
		sprintf(tmp_fname, "tmp/imail.%s.%05d", currentuser.userid,
			uinfo.pid);
		strcpy(filepath, tmp_fname);
		goto edit_mail_file;
	}
	/* end of kludge for internet mail */

	if (getuser(userid) == 0)
		return -1;
	strncpy(uid, lookupuser.userid, IDLEN+1);
	uid[IDLEN+1]=0;
	if (ythtbbs_override_included(uid, YTHTBBS_OVERRIDE_REJECTS, currentuser.userid))
		return -3;
/*add by KCN :) */

	if (!(lookupuser.userlevel & PERM_READMAIL))
		return -3;
	setmailfile_s(filepath, sizeof(filepath), uid, "");
	if (stat(filepath, &st) == -1) {
		if (mkdir(filepath, 0775) == -1)
			return -1;
	} else {
		if (!(st.st_mode & S_IFDIR))
			return -1;
	}
	memset(&newmessage, 0, sizeof (newmessage));
	now = time(NULL);
	sprintf(fname, "M.%ld.A", now);
	setmailfile_s(filepath, sizeof(filepath), uid, fname);
	count = 0;
	while ((fp = open(filepath, O_CREAT | O_EXCL | O_WRONLY, 0644)) == -1) {
		now++;
		sprintf(fname, "M.%ld.A", now);
		setmailfile_s(filepath, sizeof(filepath), uid, fname);
		if (count++ > MAX_POSTRETRY) {
			return -1;
		}
	}
	close(fp);
	newmessage.filetime = now;
	newmessage.thread = now;
	ytht_strsncpy(newmessage.owner, currentuser.userid, sizeof(newmessage.owner));

edit_mail_file:
	if (title == NULL) {
		header.reply_mode = NA;
		strcpy(header.title, "没主题");
	} else {
		header.reply_mode = YEA;
		strcpy(header.title, title);
	}
	header.postboard = NA;
	save_in_mail = in_mail;
	in_mail = YEA;

	sethomefile_s(genbuf, sizeof(genbuf), currentuser.userid, "signatures");
	ansimore2(genbuf, NA, 0, 18);
	strcpy(header.ds, uid);
	if (post_header(&header)) {
		ytht_strsncpy(newmessage.title, header.title, sizeof(newmessage.title));
		ytht_strsncpy(save_title, newmessage.title, sizeof(save_title));
		sprintf(save_title2, "{%.16s} %.60s", uid, newmessage.title);
	}
	do_quote(filepath, header.include_mode);

	if (internet_mail) {
		int res;
		if (vedit(filepath, NA, YEA) == -1) {
			unlink(filepath);
			clear();
			in_mail = save_in_mail;
			return -2;
		}
		add_loginfo(filepath);
		clear();
		prints("信件即将寄给 %s \n", uid);
		prints("标题为： %s \n", header.title);
		if (askyn("确定要寄出吗", YEA, NA) == NA) {
			prints("\n信件已取消...\n");
			res = -2;
		} else {
#ifdef SENDMAIL_MIME_AUTOCONVERT
			int ans;
			filter_ansi = 0;
			if (askyn("是否备份给自己", NA, NA) == YEA)
				mail_file(tmp_fname, currentuser.userid, save_title2);
			ans = askyn("以 MIME 格式送信", NA, NA);
			prints("请稍候, 信件传递中...\n");
			refresh();
			res = bbs_sendmail(tmp_fname, header.title, uid, ans);
#else
			if (askyn("是否备份给自己", NA, NA) == YEA)
				mail_file(tmp_fname, currentuser.userid, save_title2);
			prints("请稍候, 信件传递中...\n");
			refresh();
			res = bbs_sendmail(tmp_fname, header.title, uid);
#endif
		}
		unlink(tmp_fname);
		sprintf(genbuf, "%s netmail %s", currentuser.userid, uid);
		newtrace(genbuf);
		in_mail = save_in_mail;
		return res;
	} else {
		if (vedit(filepath, YEA, YEA) == -1) {
			unlink(filepath);
			clear();
			in_mail = save_in_mail;
			return -2;
		}
		add_loginfo(filepath);
		clear();
		if (askyn("是否备份给自己", NA, NA) == YEA)
			mail_file(filepath, currentuser.userid, save_title2);
		setmailfile_s(genbuf, sizeof(genbuf), uid, DOT_DIR);
		if (append_record(genbuf, &newmessage, sizeof (newmessage)) == -1) {
			in_mail = save_in_mail;
			return -1;
		}
		sprintf(genbuf, "%s mail %s", currentuser.userid, uid);
		newtrace(genbuf);
		in_mail = save_in_mail;
		return 0;
	}
}

int m_send(const char *userid) {
	char uident[STRLEN];
	// 永远可以给 SYSOP 发信
	if (userid && strcmp(userid, "SYSOP")) {
		if (check_maxmail()) {
			pressreturn();
			return 0;
		}
		if (check_mail_perm()) {
			pressreturn();
			return 0;
		}
	}
	if (check_maxmail()) {
		pressreturn();
		return FULLUPDATE;
	}
	if (check_mail_perm()) {
		pressreturn();
		return FULLUPDATE;
	} //add by wjbta@bmy  当信件容量超过信箱最大容量时，禁止发信
	modify_user_mode(SMAIL);
	if (			/*(uinfo.mode != LUSERS && uinfo.mode != LAUSERS
				&& uinfo.mode != FRIEND && uinfo.mode != GMENU)
				|| */ userid == NULL) {
		move(1, 0);
		clrtoeol();
		usercomplete("收信人： ", uident);
		if (uident[0] == '\0') {
			return FULLUPDATE;
		}
	} else
		strcpy(uident, userid);
	clear();
	*quote_file = '\0';
	switch (do_send(uident, NULL)) {
	case -1:
		prints("收信者不正确\n");
		break;
	case -2:
		prints("取消\n");
		break;
	case -3:
		prints("[%s] 无法收信\n", uident);
		break;
	default:
		prints("信件已寄出\n");
	}
	pressreturn();
	return FULLUPDATE;
}

int M_send(const char *s) {
	(void) s;
	if (!HAS_PERM(PERM_LOGINOK, currentuser))
		return 0;
	return m_send(NULL);
}
static int
read_mail(fptr)
struct fileheader *fptr;
{
	setmailfile_s(genbuf, sizeof(genbuf), currentuser.userid, fh2fname(fptr));
	ansimore(genbuf, NA);
	fptr->accessed |= FH_READ;
	return 0;
}

int mrd;

int delmsgs[1024];
int delcnt;

static int
read_new_mail(fptr)
struct fileheader *fptr;
{
	static int idc;
	char done = NA, delete_it;
	char fname[256];

	if (fptr == NULL) {
		delcnt = 0;
		idc = 0;
		return 0;
	}
	idc++;
	if ((fptr->accessed & FH_READ))
		return 0;
	prints("读取 %s 寄来的 '%s' ?\n", fptr->owner, fptr->title);
	prints("(Yes, or No): ");
	getdata(1, 0, "(Y)读取 (N)不读 (Q)离开 [Y]: ", genbuf, 3, DOECHO, YEA);
	if (genbuf[0] == 'q' || genbuf[0] == 'Q') {
		clear();
		return QUIT;
	}
	if (genbuf[0] != 'y' && genbuf[0] != 'Y' && genbuf[0] != '\0') {
		clear();
		return 0;
	}
	read_mail(fptr);
	mrd = 1;
	if (substitute_record(currmaildir, fptr, sizeof (*fptr), idc))
		return -1;
	delete_it = NA;
	while (!done) {
		move(t_lines - 1, 0);
		prints("(R)回信, (D)删除, (G)继续 ? [G]: ");
		switch (egetch()) {
		case 'R':
		case 'r':
			*quote_file = '\0';
			mail_reply(idc, fptr, currmaildir);
			break;
		case 'D':
		case 'd':
			delete_it = YEA;
			done = YEA;
			break;
		default:
			done = YEA;
		}
		if (!done) {
			setmailfile_s(fname, sizeof(fname), currentuser.userid, fh2fname(fptr));
			ansimore(fname, NA);	/* re-read */
		}
	}
	if (delete_it) {
		clear();
		prints("删除信件 '%s' ", fptr->title);
		getdata(1, 0, "(Y)es ,(N)o [N]: ", genbuf, 3, DOECHO, YEA);
		if (genbuf[0] == 'Y' || genbuf[0] == 'y') {	/* if not yes quit */
			setmailfile_s(fname, sizeof(fname), currentuser.userid, fh2fname(fptr));
			unlink(fname);
			delmsgs[delcnt++] = idc;
		}
	}
	clear();
	return 0;
}

int m_new(const char *s) {
	(void) s;
	clear();
	mrd = 0;
	modify_user_mode(RMAIL);
	read_new_mail(NULL);
	if (apply_record(currmaildir, (void *) read_new_mail, sizeof (struct fileheader)) == -1) {
		clear();
		move(0, 0);
		prints("No new messages\n\n\n");
		return -1;
	}
	if (delcnt) {
		while (delcnt--)
			delete_record(currmaildir, sizeof (struct fileheader), delmsgs[delcnt]);
	}
	clear();
	move(0, 0);
	if (mrd)
		prints("No more messages.\n\n\n");
	else
		prints("No new messages.\n\n\n");
	return -1;
}

static int
mailtitle()
{
	showtitle("邮件选单    ", MY_BBS_NAME);

	prints("离开[\033[1;32m←\033[m,\033[1;32me\033[m]  选择[\033[1;32m↑\033[m,\033[1;32m↓\033[m]  阅读信件[\033[1;32m→\033[m,\033[1;32mRtn\033[m]  回信[\033[1;32mR\033[m]  砍信／清除旧信[\033[1;32md\033[m,\033[1;32mD\033[m]  求助[\033[1;32mh\033[m]\033[m\n");
	prints(" \033[1;44m编号    %-12s %6s  %-24s %d %s %d %s\033[m\n", "发信者",
			"日  期", "标  题      当前信箱容量", max_mail_size(),
			"k,已用空间", get_mail_size(), "k");
	clrtobot();
	return 0;
}

static char *
maildoent(num, ent, buf)
int num;
struct fileheader *ent;
char buf[512];
{
	char b2[512];
	char status;
	char *date;
	extern char ReadPost[];
	extern char ReplyPost[];
	char c1[8];
	char c2[8];
	int same = NA;
	//add by gluon
	char replymark;
	//end

	if (ent->filetime > 740000000) {
		time_t t = ent->filetime;
		date = ctime(&t) + 4;
	} else {
		date = "";
	}

	strcpy(c2, "\033[1;33m");
	strcpy(c1, "\033[1;36m");
	if (!strcmp(ReadPost, ent->title) || !strcmp(ReplyPost, ent->title))
		same = YEA;
	ytht_strsncpy(b2, ent->owner, sizeof(b2));

	//add by gluon
	/* Added by deardragon 1999.11.15 给已回信件加上 "回信" 标记 ('R') */
	if (ent->accessed & FH_REPLIED)	/* 邮件已回复 */
		replymark = 'R';
	else
		replymark = ' ';
	/* Added End. */
	//end

	if (ent->accessed & FH_READ) {
		if (ent->accessed & FH_MARKED)
			status = 'm';
		else
			status = ' ';
	} else {
		if (ent->accessed & FH_MARKED)
			status = 'M';
		else
			status = 'N';
	}
	if (!strncmp("Re:", ent->title, 3)) {
		sprintf(buf, " %s%3d\033[m %c%c %-12.12s %6.6s %c%s%.50s\033[m",
			same ? c1 : "", num, replymark, status, b2, date,
			ent->accessed & FH_ATTACHED ? '@' : ' ',
			same ? c1 : "", ent->title);
	} else {
		sprintf(buf,
			" %s%3d\033[m %c%c %-12.12s %6.6s %c★ %s%.47s\033[m",
			same ? c2 : "", num, replymark, status, b2, date,
			ent->accessed & FH_ATTACHED ? '@' : ' ', same ? c2 : "",
			ent->title);
	}
	return buf;
}

static int
mail_read(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char notgenbuf[128];
	int readnext;
	char done = NA, delete_it, replied;

	clear();
	readnext = NA;
	setqtitle(fileinfo->title);
	directfile(notgenbuf, direct, fh2fname(fileinfo));
	delete_it = replied = NA;
	while (!done) {
		ansimore(notgenbuf, NA);
		move(t_lines - 1, 0);
		prints("(R)回信, (D)删除, (G)继续? [G]: ");
		switch (egetch()) {
		case 'R':
		case 'r':
			replied = YEA;
			*quote_file = '\0';
			mail_reply(ent, fileinfo, direct);
			break;
		case ' ':
		case 'j':
		case KEY_RIGHT:
		case KEY_DOWN:
		case KEY_PGDN:
			done = YEA;
			readnext = YEA;
			break;
		case 'D':
		case 'd':
			delete_it = YEA;
			done = YEA;
			break;
		default:
			done = YEA;
		}
	}
	if (!delete_it && !(fileinfo->accessed & FH_READ)) {
		fileinfo->accessed |= FH_READ;
		substitute_record(currmaildir, fileinfo, sizeof (*fileinfo), ent);
	}
	if (delete_it)
		return mail_del(ent, fileinfo, direct);
	if (readnext == YEA)
		return READ_NEXT;
	return FULLUPDATE;
}

/*ARGSUSED*/ int
mail_reply(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char uid[STRLEN];
	char title[STRLEN];
	char *t;

	sprintf(genbuf, "MAILER-DAEMON@%s", email_domain());
	if (strstr(fileinfo->owner, genbuf)) {
		ansimore("help/mailerror-explain", YEA);
		return FULLUPDATE;
	}
	if (check_maxmail()) {
		pressreturn();
		return 0;
	}
	if (check_mail_perm()) {
		pressreturn();
		return 0;
	}
	clear();
	modify_user_mode(SMAIL);
	ytht_strsncpy(uid, fh2owner(fileinfo), sizeof(uid));
	if (strchr(uid, '.')) {
		char filename[STRLEN];
		directfile(filename, direct, fh2fname(fileinfo));
		if (!getdocauthor(filename, uid, sizeof (uid))) {
			prints("无法投递\n");
			pressreturn();
			return FULLUPDATE;
		}
	}
	if ((t = strchr(uid, ' ')) != NULL)
		*t = '\0';
	if (strncasecmp(fileinfo->title, "Re:", 3))
		strcpy(title, "Re: ");
	else
		title[0] = '\0';
	strncat(title, fileinfo->title, sizeof (title) - 5);

	if (quote_file[0] == '\0')
		setmailfile_s(quote_file, sizeof(quote_file), currentuser.userid, fh2fname(fileinfo));
	strcpy(quote_user, fileinfo->owner);
	switch (do_send(uid, title)) {
	case -1:
		prints("无法投递\n");
		break;
	case -2:
		prints("取消回信\n");
		break;
	case -3:
		prints("[%s] 无法收信\n", uid);
		break;
	default:
		//add by gluon
		/* Added bye deardragon 1999.11.15 给已回信件加上 "回信" 标记 ('R') */
		if (ent >= 0) {
			fileinfo->accessed |= FH_REPLIED;
			substitute_record(direct, fileinfo, sizeof (*fileinfo), ent);
		}
		/* Added End. */
		//end
		prints("信件已寄出\n");
	}
	pressreturn();
	return FULLUPDATE;
}

static int
mail_del(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char buf[512];

	clear();
	prints("删除信件 [%s] ", fileinfo->title);
	getdata(1, 0, "(Yes, or No) [N]: ", genbuf, 2, DOECHO, YEA);
	if (genbuf[0] != 'Y' && genbuf[0] != 'y') {	/* if not yes quit */
		move(2, 0);
		prints("Quitting Delete Mail\n");
		pressreturn();
		clear();
		return FULLUPDATE;
	}
	currfiletime = fileinfo->filetime;
	if (!delete_file(direct, sizeof (*fileinfo), ent, (void *) cmpfilename)) {
		directfile(buf, direct, fh2fname(fileinfo));
		deltree(buf);
		return DIRCHANGED;
	}
	move(2, 0);
	prints("Delete failed\n");
	pressreturn();
	clear();
	return FULLUPDATE;
}

#ifdef INTERNET_EMAIL

int mail_forward(int ent, struct fileheader *fileinfo, char *direct) {
	(void) ent;
	char buf[STRLEN];
	if (!HAS_PERM(PERM_FORWARD, currentuser)) {
		return DONOTHING;
	}
	directfile(buf, direct, fh2fname(fileinfo));
	switch (doforward(buf, fileinfo->title, 0)) {
	case 0:
		prints("文章转寄完成!\n");
		break;
	case -1:
		prints("转寄失败: 系统发生错误.\n");
		break;
	case -2:
		prints("转寄失败: 不正确的收信地址.\n");
		break;
	default:
		prints("取消转寄...\n");
	}
	do_delay(1);
	pressreturn();
	clear();
	return FULLUPDATE;
}

int mail_u_forward(int ent, struct fileheader *fileinfo, char *direct) {
	(void) ent;
	char buf[STRLEN];
	if (!HAS_PERM(PERM_FORWARD, currentuser)) {
		return DONOTHING;
	}
	directfile(buf, direct, fh2fname(fileinfo));
	switch (doforward(buf, fileinfo->title, 1)) {
	case 0:
		prints("文章转寄完成!\n");
		break;
	case -1:
		prints("转寄失败: 系统发生错误.\n");
		break;
	case -2:
		prints("转寄失败: 不正确的收信地址.\n");
		break;
	default:
		prints("取消转寄...\n");
	}
	do_delay(1);
	pressreturn();
	clear();
	return FULLUPDATE;
}

#endif

static int
mail_del_range(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	return (del_range(ent, fileinfo, direct));
}

static int mail_mark(int ent, struct fileheader *fileinfo, char *direct) {
	(void) direct;
	if (fileinfo->accessed & FH_MARKED)
		fileinfo->accessed &= ~FH_MARKED;
	else
		fileinfo->accessed |= FH_MARKED;
	substitute_record(currmaildir, fileinfo, sizeof (*fileinfo), ent);
	return (PARTUPDATE);
}

// 等价子
typedef int(*equalor)(const struct fileheader*, const void *);
// 标记的邮件
static int ismark(const struct fileheader *mail, const void *ext)
{
	(void) ext;
	if (mail->accessed & FH_MARKED)
		return 1;
	return 0;
}
// 带附件的邮件
static int isattach(const struct fileheader *mail, const void *ext)
{
	(void) ext;
	if (mail->accessed & FH_ATTACHED)
		return 1;
	return 0;
}
// 特定标题的邮件
static int istitle(const struct fileheader *mail, const void *ext)
{
	if (strstr2(mail->title, (const char *)ext) != NULL)
		return 1;
	return 0;
}
// 特定发信人的邮件
static int issender(const struct fileheader *mail, const void *ext)
{
	if (strncasecmp(mail->owner, (const char*)ext, sizeof(mail->owner)+1) == 0) // 不区分大小写
		return 1;
	return 0;
}

// 对邮箱进行搜索，产生新的.DIR文件（作为搜索结果）
// interma@bmy, 2006-11-23
static void do_search_mailbox(int type, const char *whattosearch, const char *tempdir)
{
	FILE *fdir;
	FILE *ftempdir;
	equalor isequal = NULL;

	fdir = fopen(currmaildir, "r");
	if (fdir == NULL)
		return;
	ftempdir = fopen(tempdir, "w");
	if (ftempdir == NULL) {
		fclose(fdir);
		return;
	}

	switch (type)
	{
	case 1:
		isequal = ismark;
		break;
	case 2:
		isequal = isattach;
		break;
	case 3:
		isequal = istitle;
		break;
	case 4:
		isequal = issender;
		break;
	}

	if (isequal == NULL) {
		fclose(fdir);
		fclose(ftempdir);
		return;
	}

	size_t n = 0;
	struct fileheader mail;
	while ((n = fread(&mail, sizeof(struct fileheader), 1, fdir)) > 0)
	{
		if (isequal(&mail, whattosearch))
		{
			fwrite(&mail, sizeof(struct fileheader), 1, ftempdir);
		}
	}

	fclose(fdir);
	fclose(ftempdir);
	return;
}

// 将邮箱索引文件(.DIR)的路径名截取出来
// interma@bmy, 2006-11-23
#if 0
static char *truncateDIR(char *dest)
{
	char *p = NULL;
	if ((p=strstr(dest, ".DIR")) != NULL)
	{
		*p = '\0';
	}
	return dest;
}
#endif

struct one_key query_comms[];

// 邮箱查询功能，邮箱界面中ctrl+g
// interma@bmy, 2006-11-23
static int mail_query(int ent, struct fileheader *fileinfo, char *direct) {
	(void) ent;
	(void) fileinfo;
	(void) direct;
	//power_action(currmaildir, 1, -1, "标记含 m", 9);
	char ans[3];
	int type;
	char whattosearch[31];

	ans[0] = '\0';
	getdata(t_lines - 1 , 0, "请选择:0)取消 1)被m邮件 2)附件 3)标题关键字 4)同作者 [0]:", ans, 2, DOECHO, NA);
	type = atoi(ans);
	if (ans[0] == '\0')
		type = 0;
	if (type < 1 || type > 4)
		return PARTUPDATE;
	whattosearch[0] = '\0';
	switch (type)
	{
	case 1:
		break;
	case 2:
		break;
	case 3:
		getdata(t_lines - 1, 0, "请输入标题关键字:", whattosearch, 31, DOECHO,NA);
		if (whattosearch[0] == '\0')
			return PARTUPDATE;
		break;
	case 4:
		getdata(t_lines - 1, 0, "请输入作者ID:",whattosearch, 31, DOECHO, NA);
		if (whattosearch[0] == '\0')
			return PARTUPDATE;
		break;
	}

	//char tempdir[L_tmpnam];
	//tmpnam(tempdir);
	char maildir[256];
	//strncpy(maildir, currmaildir, 256);
	//char *tempdir = tempnam(truncateDIR(maildir), ".DIR");

	snprintf(maildir, 256, "%s-XXXXXX", currmaildir);
	int fd = mkstemp(maildir);

	if (fd != -1) {
		close(fd); // close file descriptor immediately

		do_search_mailbox(type, whattosearch, maildir);
		// m_init();

		i_read(RMAIL, maildir, mailtitle, (void *) maildoent, query_comms, sizeof(struct fileheader));

		unlink(maildir);
	}

	i_read(RMAIL, currmaildir, mailtitle, (void *) maildoent, mail_comms, sizeof (struct fileheader));
	return DOQUIT; // 如果不是DOQUIT，那就要多按几下"后退"才能退出，呵呵
}

// 邮箱查询界面中可以使用的命令
struct one_key query_comms[] = {
	//{'d', mail_del, "删除信件"},
	//{'D', mail_del_range, "区段删除"},
	{Ctrl('P'), M_send, "发送信件"},
	{'E', edit_post, "编辑信件"},
	{'r', mail_read, "阅读信件"},
	{'R', mail_reply, "回复信件"},
	//{'m', mail_mark, "mark信件"},
	{'i', Save_post, "将文章存入暂存档"},
	{'I', Import_post, "将信件放入精华区"},
	{'x', into_announce, "进入精华区"},
	{KEY_TAB, show_user_notes, "查看用户备忘录"},
#ifdef INTERNET_EMAIL
	{'F', mail_forward, "转寄信件"},
	{'U', mail_u_forward, "uuencode 转寄"},
#endif
	{'a', auth_search_down, "向后搜索作者"},
	{'A', auth_search_up, "向前搜索作者"},
	{'/', t_search_down, "向后搜索标题"},
	{'?', t_search_up, "向前搜索标题"},
	{'\'', post_search_down, "向后搜索内容"},
	{'\"', post_search_up, "向前搜索内容"},
	{']', thread_down, "向后同主题"},
	{'[', thread_up, "向前同主题"},
	{Ctrl('A'), show_author, "作者简介"},
	{'\\', SR_last, "最后一篇同主题文章"},
	{'=', SR_first, "第一篇同主题文章"},
	{'L', show_allmsgs, "查看消息"},
	{Ctrl('C'), do_cross, "转贴文章"},
	{'n', SR_first_new, "主题未读的第一篇"},
	{'p', SR_read, "相同主题的阅读"},
	{Ctrl('U'), SR_author, "相同作者阅读"},
	{'h', mailreadhelp, "查看帮助"},
	{'!', Q_Goodbye, "快速离站"},
	{'S', s_msg, "传送讯息"},
	{'c', t_friends, "查看好友"},
	{'C', friend_author, "增加作者为好友"},
	{Ctrl('E'), mail_rjunk, "翻拣垃圾"},
	//{Ctrl('G'), mail_query, "查询模式"},
	{'\0', NULL, ""}
};

// 邮箱阅读界面（正常模式）中可以使用的命令
struct one_key mail_comms[] = {
	{'d', mail_del, "删除信件"},
	{'D', mail_del_range, "区段删除"},
	{Ctrl('P'), M_send, "发送信件"},
	{'E', edit_post, "编辑信件"},
	{'T', edit_title, "修改标题"},
	{'r', mail_read, "阅读信件"},
	{'R', mail_reply, "回复信件"},
	{'m', mail_mark, "mark信件"},
	{'i', Save_post, "将文章存入暂存档"},
	{'I', Import_post, "将信件放入精华区"},
	{'x', into_announce, "进入精华区"},
	{KEY_TAB, show_user_notes, "查看用户备忘录"},
#ifdef INTERNET_EMAIL
	{'F', mail_forward, "转寄信件"},
	{'U', mail_u_forward, "uuencode 转寄"},
#endif
	{'a', auth_search_down, "向后搜索作者"},
	{'A', auth_search_up, "向前搜索作者"},
	{'/', t_search_down, "向后搜索标题"},
	{'?', t_search_up, "向前搜索标题"},
	{'\'', post_search_down, "向后搜索内容"},
	{'\"', post_search_up, "向前搜索内容"},
	{']', thread_down, "向后同主题"},
	{'[', thread_up, "向前同主题"},
	{Ctrl('A'), show_author, "作者简介"},
	{'\\', SR_last, "最后一篇同主题文章"},
	{'=', SR_first, "第一篇同主题文章"},
	{'L', show_allmsgs, "查看消息"},
	{Ctrl('C'), do_cross, "转贴文章"},
	{'n', SR_first_new, "主题未读的第一篇"},
	{'p', SR_read, "相同主题的阅读"},
	{Ctrl('U'), SR_author, "相同作者阅读"},
	{'h', mailreadhelp, "查看帮助"},
	{'!', Q_Goodbye, "快速离站"},
	{'S', s_msg, "传送讯息"},
	{'c', t_friends, "查看好友"},
	{'C', friend_author, "增加作者为好友"},
	{Ctrl('E'), mail_rjunk, "翻拣垃圾"},
	{Ctrl('G'), mail_query, "查询模式"},
	{'\0', NULL, ""}
};

// 邮箱阅读界面（正常模式）中可以使用的命令
// 默认值
const struct one_key mail_default_comms[] = {
	{'d', mail_del, "删除信件"},
	{'D', mail_del_range, "区段删除"},
	{Ctrl('P'), M_send, "发送信件"},
	{'E', edit_post, "编辑信件"},
	{'T', edit_title, "修改标题"},
	{'r', mail_read, "阅读信件"},
	{'R', mail_reply, "回复信件"},
	{'m', mail_mark, "mark信件"},
	{'i', Save_post, "将文章存入暂存档"},
	{'I', Import_post, "将信件放入精华区"},
	{'x', into_announce, "进入精华区"},
	{KEY_TAB, show_user_notes, "查看用户备忘录"},
#ifdef INTERNET_EMAIL
	{'F', mail_forward, "转寄信件"},
	{'U', mail_u_forward, "uuencode 转寄"},
#endif
	{'a', auth_search_down, "向后搜索作者"},
	{'A', auth_search_up, "向前搜索作者"},
	{'/', t_search_down, "向后搜索标题"},
	{'?', t_search_up, "向前搜索标题"},
	{'\'', post_search_down, "向后搜索内容"},
	{'\"', post_search_up, "向前搜索内容"},
	{']', thread_down, "向后同主题"},
	{'[', thread_up, "向前同主题"},
	{Ctrl('A'), show_author, "作者简介"},
	{'\\', SR_last, "最后一篇同主题文章"},
	{'=', SR_first, "第一篇同主题文章"},
	{'L', show_allmsgs, "查看消息"},
	{Ctrl('C'), do_cross, "转贴文章"},
	{'n', SR_first_new, "主题未读的第一篇"},
	{'p', SR_read, "相同主题的阅读"},
	{Ctrl('U'), SR_author, "相同作者阅读"},
	{'h', mailreadhelp, "查看帮助"},
	{'!', Q_Goodbye, "快速离站"},
	{'S', s_msg, "传送讯息"},
	{'c', t_friends, "查看好友"},
	{'C', friend_author, "增加作者为好友"},
	{Ctrl('E'), mail_rjunk, "翻拣垃圾"},
	{Ctrl('G'), mail_query, "查询模式"},
	{'\0', NULL, ""}
};

int m_read(const char *s) {
	(void) s;
	int savemode;
	int save_in_mail;
	save_in_mail = in_mail;
	in_mail = YEA;
	savemode = uinfo.mode;
	m_init();
	i_read(RMAIL, currmaildir, mailtitle, (void *) maildoent, mail_comms, sizeof (struct fileheader));
	modify_user_mode(savemode);
	in_mail = save_in_mail;
	return 0;
}

int
invalidaddr(addr)
char *addr;
{
	if (*addr == '\0' || !strchr(addr, '@'))
		return 1;
	while (*addr) {
		if (!isalnum(*addr) && !strchr(".!@:-_", *addr))
			return 1;
		addr++;
	}
	return 0;
}

#ifdef INTERNET_EMAIL

#ifdef SENDMAIL_MIME_AUTOCONVERT
static int
bbs_sendmail(fname, title, receiver, mime)
char *fname, *title, *receiver;
int mime;
#else
static int
bbs_sendmail(fname, title, receiver)
char *fname, *title, *receiver;
#endif
{
	FILE *fin, *fout;
	char *attach;
	size_t len;

	sprintf(genbuf, "/usr/lib/sendmail -f %s.bbs@%s %s",
		currentuser.userid, email_domain(), receiver);
	fout = popen(genbuf, "w");
	fin = fopen(fname, "r");
	if (fin == NULL || fout == NULL) {
		if (fin) fclose(fin);
		if (fout) pclose(fout);
		return -1;
	}

	fprintf(fout, "Return-Path: %s.bbs@%s\n", currentuser.userid,
		email_domain());
	fprintf(fout, "Reply-To: %s.bbs@%s\n", currentuser.userid,
		email_domain());
	fprintf(fout, "From: %s.bbs@%s\n", currentuser.userid, email_domain());
	fprintf(fout, "To: %s\n", receiver);
	fprintf(fout, "Subject: %s\n", title);
	fprintf(fout, "X-Forwarded-By: %s (%s)\n", currentuser.userid,
		currentuser.username);

	fprintf(fout, "X-Disclaimer: %s 对本信内容恕不负责。\n", MY_BBS_NAME);
#ifdef SENDMAIL_MIME_AUTOCONVERT
	if (mime) {
		fprintf(fout, "MIME-Version: 1.0\n");
		fprintf(fout, "Content-Type: text/plain; charset=US-ASCII\n");
		fprintf(fout, "Content-Transfer-Encoding: 8bit\n");
	}
#endif
	fprintf(fout, "Precedence: junk\n\n");

	while (fgets(genbuf, 255, fin) != NULL) {
		if (NULL != (attach = checkbinaryattach(genbuf, fin, &len))) {
			uuencode(fin, fout, len, attach);
			continue;
		}
		if (filter_ansi)
			filter(genbuf);
		if (genbuf[0] == '.' && genbuf[1] == '\n')
			fputs(". \n", fout);
		else
			fputs(genbuf, fout);
	}

	fprintf(fout, ".\n");

	fclose(fin);
	pclose(fout);
	return 0;
}

#endif

int g_send(const char *s) {
	(void) s;
	char uident[13], tmp[3];
	int cnt, i, n, fmode = NA;
	int keepgoing = 0;
	char maillists[STRLEN];

	if (check_maxmail()) {
		pressreturn();
		return 0;
	}
	if (check_mail_perm()) {
		pressreturn();
		return 0;
	}
	modify_user_mode(SMAIL);
	*quote_file = '\0';
	clear();
	sethomefile_s(maillists, sizeof(maillists), currentuser.userid, "maillist");
	cnt = listfilecontent(maillists);
	while (1) {
		if (cnt > maxrecp - 10) {
			move(2, 0);
			prints("目前限制寄信给 \033[1m%d\033[m 人", maxrecp);
		}
		if (keepgoing) {
			tmp[0]='a';
			tmp[1]=0;
		}
		else {
		getdata(0, 0,
			"(A)增加 (D)删除 (I)引入好友 (C)清除目前名单 (E)放弃 (S)寄出? [S]： ",
			tmp, 2, DOECHO, YEA);

		}
		if (tmp[0] == '\n' || tmp[0] == '\0' || tmp[0] == 's' || tmp[0] == 'S') {
			break;
		}
		if (tmp[0] == 'a' || tmp[0] == 'd' || tmp[0] == 'A' || tmp[0] == 'D') {
			move(1, 0);
			if (tmp[0] == 'a' || tmp[0] == 'A')
				usercomplete("请依次输入使用者代号(只按 ENTER 结束输入): ", uident);
			else
				namecomplete("请依次输入使用者代号(只按 ENTER 结束输入): ", uident);
			move(1, 0);
			clrtoeol();
			if (uident[0] == '\0') {
				keepgoing = 0;
				continue;
			}
			keepgoing = 1;
			if (!getuser(uident)) {
				move(2, 0);
				prints("这个使用者代号是错误的.\n");
			}
		}
		switch (tmp[0]) {
		case 'A':
		case 'a':
			if (!(lookupuser.userlevel & PERM_READMAIL)) {
				move(2, 0);
				prints("无法送信给: \033[1m%s\033[m\n", lookupuser.userid);
				break;
			} else if (seek_in_file(maillists, uident)) {
				move(2, 0);
				prints("已经列为收件人之一 \n");
				break;
			}
			ytht_add_to_file(maillists, uident);
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
			{
				if (seek_in_file(maillists, uident)) {
					ytht_add_to_file(maillists, uident);
					cnt--;
				}
				break;
			}
		case 'I':
		case 'i':
			n = 0;
			clear();
			for (i = cnt; i < maxrecp && n < uinfo.fnum; i++) {
				int key;
				move(2, 0);
				clrtoeol();
				ythtbbs_cache_UserTable_getuserid(uinfo.friend[n], uident, sizeof(uident));
				prints("%s\n", uident);
				move(3, 0);
				n++;
				prints("(A)全部加入 (Y)加入 (N)不加入 (Q)结束? [Y]:");
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
						prints("这个使用者代号是错误的.\n");
						i--;
						continue;
					} else if (!(lookupuser.userlevel & PERM_READMAIL)) {
						move(4, 0);
						prints("无法送信给: \033[1m%s\033[m\n", lookupuser.userid);
						i--;
						continue;
					} else if (seek_in_file(maillists, uident)) {
						i--;
						continue;
					}
					ytht_add_to_file(maillists, uident);
					cnt++;
				}
			}
			fmode = NA;
			clear();
			break;
		case 'C':
		case 'c':
			unlink(maillists);
			cnt = 0;
			break;
		}
		if (strchr("EeQq", tmp[0]))
			break;
		move(5, 0);
		clrtobot();
		if (cnt > maxrecp)
			cnt = maxrecp;
		move(3, 0);
		clrtobot();
		listfilecontent(maillists);
	}
	if (cnt > 0) {
		G_SENDMODE = 2;
		switch (do_gsend(NULL, cnt)) {
		case -1:
			prints("信件目录错误\n");
			break;
		case -2:
			prints("取消\n");
			break;
		default:
			prints("信件已寄出\n");
		}
		G_SENDMODE = 0;
		pressreturn();
	}
	return 0;
}

/*Add by SmallPig*/

static int do_gsend(const char *userid[], int num) {
	struct stat st;
	char filepath[STRLEN], tmpfile[STRLEN];
	int cnt;
	FILE *mp = NULL;
	int save_in_mail;

	save_in_mail = in_mail;
	in_mail = YEA;
	sprintf(genbuf, "%s (%s)", currentuser.userid, currentuser.username);
	header.reply_mode = NA;
	strcpy(header.title, "没主题");
	strcpy(header.ds, "寄信给一群人");
	header.postboard = NA;
	sprintf(tmpfile, "tmp/gsend.%s.%05d", currentuser.userid, uinfo.pid);
	if (post_header(&header)) {
		sprintf(save_title, "[群体信件] %.60s", header.title);
	}
	setquotefile("");
	do_quote(tmpfile, header.include_mode);
	if (vedit(tmpfile, YEA, YEA) == -1) {
		unlink(tmpfile);
		clear();
		in_mail = save_in_mail;
		return -2;
	}
	add_loginfo(tmpfile);
	clear();
	prints("\033[5;1;32m正在寄件中，请稍候...\033[m");
	if (G_SENDMODE == 2) {
		char maillists[STRLEN];

		sethomefile_s(maillists, sizeof(maillists), currentuser.userid, "maillist");
		if ((mp = fopen(maillists, "r")) == NULL) {
			in_mail = save_in_mail;
			return -3;
		}
	}
	if (G_SENDMODE == 3) {
		char maillists[STRLEN];
		G_SENDMODE = 2;
		setbfile(maillists, sizeof(maillists), currboard, "club_users");
		if ((mp = fopen(maillists, "r")) == NULL) {
			in_mail = save_in_mail;
			return -3;
		}
	}
	for (cnt = 0; cnt < num; cnt++) {
		char uid[13];
		char buf[STRLEN];

		if (G_SENDMODE == 1)
			ythtbbs_cache_UserTable_getuserid(uinfo.friend[cnt], uid, sizeof(uid));
		else if (G_SENDMODE == 2) {
			if (fgets(buf, STRLEN, mp) != NULL) {
				if (strtok(buf, " \n\r\t") != NULL)
					strcpy(uid, buf);
				else
					continue;
			} else {
				cnt = num;
				continue;
			}
		} else
			strcpy(uid, userid[cnt]);
		sprintf(filepath, "mail/%c/%s", mytoupper(uid[0]), uid);
		if (stat(filepath, &st) == -1) {
			if (mkdir(filepath, 0775) == -1) {
				if (G_SENDMODE == 2)
					fclose(mp);
				in_mail = save_in_mail;
				return -1;
			}
		} else {
			if (!(st.st_mode & S_IFDIR)) {
				if (G_SENDMODE == 2)
					fclose(mp);
				in_mail = save_in_mail;
				return -1;
			}
		}
		mail_file(tmpfile, uid, save_title);
	}
	unlink(tmpfile);
	clear();
	if (G_SENDMODE == 2)
		fclose(mp);
	in_mail = save_in_mail;
	return 0;
}

static int do_gsend_voter(const char *userid[], int num, const char *fname) {
	struct stat st;
	char filepath[STRLEN], tmpfile[STRLEN];
	int cnt;
	FILE *mp = NULL;
	int save_in_mail;

	save_in_mail = in_mail;
	in_mail = YEA;
	sprintf(genbuf, "%s (%s)", currentuser.userid, currentuser.username);
	header.reply_mode = NA;
	strcpy(header.title, "没主题");
	strcpy(header.ds, "寄信给一群人");
	header.postboard = NA;
	sprintf(tmpfile, "tmp/gsend.%s.%05d", currentuser.userid, uinfo.pid);
	if (post_header(&header)) {
		sprintf(save_title, "[群体信件] %.60s", header.title);
	}
	setquotefile("");
	do_quote(tmpfile, header.include_mode);
	if (vedit(tmpfile, YEA, YEA) == -1) {
		unlink(tmpfile);
		clear();
		in_mail = save_in_mail;
		return -2;
	}
	add_loginfo(tmpfile);
	clear();
	prints("\033[5;1;32m正在寄件中，请稍候...\033[m");
	if (G_SENDMODE == 4) {
		char maillists[STRLEN];
		G_SENDMODE = 2;
		setbfile(maillists, sizeof(maillists), currboard,fname);
		if ((mp = fopen(maillists, "r")) == NULL) {
			in_mail = save_in_mail;
			return -3;
		}
	}
	for (cnt = 0; cnt < num; cnt++) {
		char uid[13];
		char buf[STRLEN];

		if (G_SENDMODE == 1)
			ythtbbs_cache_UserTable_getuserid(uinfo.friend[cnt], uid, sizeof(uid));
		else if (G_SENDMODE == 2) {
			if (fgets(buf, STRLEN, mp) != NULL) {
				if (strtok(buf, " \n\r\t") != NULL)
					strcpy(uid, buf);
				else
					continue;
			} else {
				cnt = num;
				continue;
			}
		} else
			strcpy(uid, userid[cnt]);
		sprintf(filepath, "mail/%c/%s", mytoupper(uid[0]), uid);
		if (stat(filepath, &st) == -1) {
			if (mkdir(filepath, 0775) == -1) {
				if (G_SENDMODE == 2)
					fclose(mp);
				in_mail = save_in_mail;
				return -1;
			}
		} else {
			if (!(st.st_mode & S_IFDIR)) {
				if (G_SENDMODE == 2)
					fclose(mp);
				in_mail = save_in_mail;
				return -1;
			}
		}
		mail_file(tmpfile, uid, save_title);
	}
	unlink(tmpfile);
	clear();
	if (G_SENDMODE == 2)
		fclose(mp);
	in_mail = save_in_mail;
	return 0;
}

time_t
mail_file(tmpfile, userid, title)
char tmpfile[STRLEN], userid[STRLEN], title[STRLEN];
{
	struct fileheader newmessage;
	struct stat st;
	char fname[STRLEN], filepath[STRLEN];
	int fp, count;
	time_t now;

	memset(&newmessage, 0, sizeof (newmessage));
	ytht_strsncpy(newmessage.owner, currentuser.userid, sizeof(newmessage.owner));
	ytht_strsncpy(newmessage.title, title, sizeof(newmessage.title));
	ytht_strsncpy(save_title, newmessage.title, sizeof(save_title));

	setmailfile_s(filepath, sizeof(filepath), userid, "");
	if (stat(filepath, &st) == -1) {
		if (mkdir(filepath, 0775) == -1)
			return -1;
	} else {
		if (!(st.st_mode & S_IFDIR))
			return -1;
	}
	now = time(NULL);
	sprintf(fname, "M.%ld.A", now);
	setmailfile_s(filepath, sizeof(filepath), userid, fname);
	count = 0;
	while ((fp = open(filepath, O_CREAT | O_EXCL | O_WRONLY, 0644)) == -1) {
		now++;
		sprintf(fname, "M.%ld.A", now);
		setmailfile_s(filepath, sizeof(filepath), userid, fname);
		if (count++ > MAX_POSTRETRY) {
			return -1;
		}
	}
	close(fp);
	newmessage.filetime = now;
	newmessage.thread = now;
	if (mailallmode) {
		unlink(filepath);
		symlink(tmpfile, filepath);
	} else
		copyfile(tmpfile, filepath);
	setmailfile_s(genbuf, sizeof(genbuf), userid, DOT_DIR);
	if (append_record(genbuf, &newmessage, sizeof (newmessage)) == -1)
		return -1;
	sprintf(genbuf, "%s mail %s", currentuser.userid, userid);
	newtrace(genbuf);
//      return 0;
	return now;
}

int
mail_buf(buf, userid, title)
char *buf, userid[], title[];
{
	struct fileheader newmessage;
	struct stat st;
	char fname[STRLEN], filepath[STRLEN];
	int count, tmpinmail, fd;
	time_t now;
	FILE *fp;

	memset(&newmessage, 0, sizeof (newmessage));
	ytht_strsncpy(newmessage.owner, currentuser.userid, sizeof(newmessage.owner));
	ytht_strsncpy(newmessage.title, title, sizeof(newmessage.title));
	ytht_strsncpy(save_title, newmessage.title, sizeof(save_title));

	setmailfile_s(filepath, sizeof(filepath), userid, "");
	if (stat(filepath, &st) == -1) {
		if (mkdir(filepath, 0775) == -1)
			return -1;
	} else {
		if (!(st.st_mode & S_IFDIR))
			return -1;
	}
	now = time(NULL);
	sprintf(fname, "M.%ld.A", now);
	setmailfile_s(filepath, sizeof(filepath), userid, fname);
	count = 0;
	while ((fd = open(filepath, O_CREAT | O_EXCL | O_WRONLY, 0644)) == -1) {
		now++;
		sprintf(fname, "M.%ld.A", now);
		setmailfile_s(filepath, sizeof(filepath), userid, fname);
		if (count++ > MAX_POSTRETRY) {
			return -1;
		}
	}
	close(fd);
	newmessage.filetime = now;
	newmessage.thread = now;
	fp = fopen(filepath, "w");
	if (!fp)
		return -1;
	tmpinmail = in_mail;
	in_mail = YEA;
	write_header(fp, 1);
	in_mail = tmpinmail;
	fprintf(fp, "%s", buf);
	fclose(fp);
	setmailfile_s(genbuf, sizeof(genbuf), userid, DOT_DIR);
	if (append_record(genbuf, &newmessage, sizeof (newmessage)) == -1)
		return -1;
	sprintf(genbuf, "%s mail %s", currentuser.userid, userid);
	newtrace(genbuf);
	return 0;
}

/*Add by SmallPig*/
int ov_send(const char *s) {
	(void) s;
	int all, i;

	if (check_maxmail()) {
		pressreturn();
		return 0;
	}
	if (check_mail_perm()) {
		pressreturn();
		return 0;
	}
	modify_user_mode(SMAIL);
	move(1, 0);
	clrtobot();
	move(2, 0);
	prints("寄信给好友名单中的人，目前本站限制仅可以寄给 \033[1m%d\033[m 位。\n", maxrecp);
	if (uinfo.fnum <= 0) {
		prints("你并没有设定好友。\n");
		pressanykey();
		clear();
		return 0;
	} else {
		prints("名单如下：\n");
	}
	G_SENDMODE = 1;
	all = (uinfo.fnum >= maxrecp) ? maxrecp : uinfo.fnum;
	for (i = 0; i < all && i < MAXFRIENDS; i++) {
		char uid[IDLEN + 2];

		ythtbbs_cache_UserTable_getuserid(uinfo.friend[i], uid, sizeof(uid));
		prints("%-12s ", uid);
		if ((i + 1) % 6 == 0)
			outc('\n');
	}
	pressanykey();
	switch (do_gsend(NULL, all)) {
	case -1:
		prints("信件目录错误\n");
		break;
	case -2:
		prints("信件取消\n");
		break;
	default:
		prints("信件已寄出\n");
	}
	pressreturn();
	G_SENDMODE = 0;
	return 0;
}

int
voter_send(char* fname)
{
	int all;
	char maillists[STRLEN];
	if (check_maxmail()) {
		pressreturn();
		return 0;
	}
	if (check_mail_perm()) {
		pressreturn();
		return 0;
	}
	modify_user_mode(SMAIL);
	move(1, 0);
	clrtobot();
	move(2, 0);
	G_SENDMODE = 4;
	setbfile(maillists, sizeof(maillists), currboard, fname);
	all = listfilecontent(maillists);
	switch (do_gsend_voter(NULL, all, fname)) {
	case -1:
		prints("信件目录错误\n");
		break;
	case -2:
		prints("信件取消\n");
		break;
	default:
		prints("信件已寄出\n");
	}
	pressreturn();
	modify_user_mode(READING);
	G_SENDMODE = 0;
	return 0;
}

int
club_send()
{
	int all;
	char maillists[STRLEN];
	if (check_maxmail()) {
		pressreturn();
		return 0;
	}
	if (check_mail_perm()) {
		pressreturn();
		return 0;
	}
	modify_user_mode(SMAIL);
	move(1, 0);
	clrtobot();
	move(2, 0);
	G_SENDMODE = 3;
	setbfile(maillists, sizeof(maillists), currboard, "club_users");
	all = listfilecontent(maillists);
	switch (do_gsend(NULL, all)) {
	case -1:
		prints("信件目录错误\n");
		break;
	case -2:
		prints("信件取消\n");
		break;
	default:
		prints("信件已寄出\n");
	}
	pressreturn();
	modify_user_mode(READING);
	G_SENDMODE = 0;
	return 0;
}

#if 0
int
in_group(uident, cnt)
char uident[maxrecp][STRLEN];
int cnt;
{
	int i;

	for (i = 0; i < cnt; i++)
		if (!strcmp(uident[i], uident[cnt])) {
			return i + 1;
		}
	return 0;
}
#endif

#ifdef INTERNET_EMAIL
int
doforward(filepath, oldtitle, mode)
char *filepath, *oldtitle;
int mode;
{
	static char address[STRLEN];
	char fname[STRLEN], tmpfname[STRLEN];
	char receiver[STRLEN];
	char title[STRLEN];
	int return_no;
	time_t now;

	clear();
	if (address[0] == '\0') {
		//strncpy(address, currentuser.email, STRLEN);
		strncpy(address, currentuser.userid, STRLEN);
	}
	if (HAS_PERM(PERM_SETADDR, currentuser)) {
		prints("请直接按 Enter 接受括号内提示的地址, 或者输入其他地址\n");
		prints("把信件转寄给 [%s]\n", address);
		move(2,0);
		usercomplete("==>", receiver);
		if (receiver[0] != '\0') {
			ytht_strsncpy(address, receiver, STRLEN);
		}
	}
	sprintf(genbuf, ".bbs@%s", email_domain());
	if (strstr(receiver, genbuf) || strstr(receiver, ".bbs@localhost")) {
		char *pos;

		if ((pos = strchr(address, '.')) != NULL)
			*pos = '\0';
	}
	if (check_maxmail())
		return -1;
	sprintf(genbuf, "确定将文章寄给 %s 吗", address);
	if (askyn(genbuf, YEA, NA) == 0)
		return 1;
	if (invalidaddr(address))
		if (!getuser(address) || check_mail_perm())
			return -2;
	sprintf(tmpfname, "tmp/forward.%s.%05d", currentuser.userid, uinfo.pid);
	copyfile(filepath, tmpfname);
	if (askyn("是否修改文章内容", NA, NA) == 1) {
		if (vedit(tmpfname, NA, NA) == -1) {
			if (askyn("是否寄出未修改的文章", YEA, NA) == 0) {
				unlink(tmpfname);
				clear();
				return 1;
			}
		} else if (ADD_EDITMARK) {
			now = time(NULL);
			add_edit_mark(tmpfname, currentuser.userid, now, fromhost);
		}
		clear();
	}
	add_crossinfo(tmpfname, 2);
	prints("转寄信件给 %s, 请稍候....\n", address);
	refresh();

	if (mode == 0)
		strcpy(fname, tmpfname);
	else if (mode == 1) {
		FILE *fr, *fw;
		char uuname[40];
		sprintf(fname, "bbstmpfs/tmp/file.uu%05d", uinfo.pid);
		fr = fopen(tmpfname, "r");
		if (NULL == fr)
			return -3;
		fw = fopen(fname, "w");
		if (NULL == fw) {
			fclose(fr);
			return -4;
		}
		snprintf(uuname, sizeof (uuname), "%s-BBSMAIL.%d", MY_BBS_ID, (int) time(NULL));
		uuencode(fr, fw, file_size(tmpfname), uuname);
		fclose(fw);
		fclose(fr);
	}
	sprintf(title, "[转寄] %.70s", oldtitle);
	if (!strpbrk(address, "@.")) {
		return_no = mail_file(fname, lookupuser.userid, title);
		if (return_no != -1)
			return_no = 0;
	} else {
#ifdef SENDMAIL_MIME_AUTOCONVERT
		filter_ansi = askyn("过滤 ANSI 控制码", NA, NA);
		if (askyn("以 MIME 格式送信", NA, NA) == YEA)
			return_no = bbs_sendmail(fname, title, address, YEA);
		else
			return_no = bbs_sendmail(fname, title, address, NA);
#else
		return_no = bbs_sendmail(fname, title, address);
#endif
	}
	if (mode == 1) {
		unlink(fname);
	}
	unlink(tmpfname);
	return (return_no);
}

#endif

static void
getmailinfo(char *path, struct fileheader *rst)
{
	FILE *fp;
	char *p, buf1[256], buf2[256];
	strcpy(rst->title, "无题");
	strcpy(rst->owner, "XJTU-XANET");
	if ((fp = fopen(path, "r")) == NULL)
		return;
	buf1[0] = 0;
	while (fgets(buf2, sizeof (buf2), fp) != NULL) {
		if ((strncmp(buf1, "寄信人: ", 8) && strncmp(buf1, "发信人: ", 8)) || (strncmp(buf2, "标  题: ", 8) && strncmp(buf2, "标　题: ", 8))) {
			strcpy(buf1, buf2);
			continue;
		}
		p = strchr(buf1 + 8, ' ');
		if (p)
			*p = 0;
		if ((p = strchr(buf1 + 8, '(')))
			*p = 0;
		if ((p = strchr(buf1 + 8, '\n')))
			*p = 0;
		fh_setowner(rst, buf1 + 8, 0);
		if ((p = strchr(buf2 + 8, '\n')))
			*p = 0;
		if ((p = strchr(buf2 + 8, '\r')))
			*p = 0;
		ytht_strsncpy(rst->title, buf2 + 8, sizeof(rst->title));
		break;
	}
	fclose(fp);
	return;
}

static int
mail_rjunk()
{
	DIR *dirp;
	struct dirent *direntp;
	int len, count;
	char buf[256], rpath[512], npath[256];
	struct fileheader rstmsg;

	clear();
	getdata(1, 0, "要拣垃圾吗?(Yes, or No) [N]: ", genbuf, 2, DOECHO, YEA);
	if (genbuf[0] != 'Y' && genbuf[0] != 'y') {
		move(2, 0);
		prints("哦~~~~~,怕脏啊...\n");
		pressreturn();
		clear();
		return FULLUPDATE;
	}

	len = sprintf(buf, "%c/%s/", mytoupper(currentuser.userid[0]), currentuser.userid);
	ytht_normalize(buf);
	dirp = opendir(MY_BBS_HOME "/mail/.junk");
	if (dirp == NULL)
		return -2;
	count = 0;
	while ((direntp = readdir(dirp)) != NULL) {
		if (strncmp(buf, direntp->d_name, len) || strncmp(direntp->d_name + len, "M.", 2))
			continue;
		sprintf(rpath, MY_BBS_HOME "/mail/.junk/%s", direntp->d_name);
		sprintf(npath, "mail/%c/%s/%s", mytoupper(currentuser.userid[0]), currentuser.userid, direntp->d_name + len);
		rename(rpath, npath);
		bzero(&rstmsg, sizeof(struct fileheader));
		getmailinfo(npath, &rstmsg);
		rstmsg.filetime = atoi(direntp->d_name + 2 + len);
		rstmsg.thread = rstmsg.filetime;
		setmailfile_s(genbuf, sizeof(genbuf), currentuser.userid, DOT_DIR);
		if (append_record(genbuf, &rstmsg, sizeof (struct fileheader)) == -1)
			break;
		count++;
	}
	closedir(dirp);
	if (count) {
		prints("哦,拣了 %d 件垃圾回来.\n", count);
	} else
		prints("哎呀,这年头垃圾也难找...\n");
	pressreturn();
	return FULLUPDATE;
}

static int
m_cancel_1(struct fileheader *fh, char *receiver)
{
	char buf[256];
	FILE *fp;
	time_t now;
	if (strncmp(currentuser.userid, fh2owner(fh), IDLEN + 1) || (fh->accessed & FH_READ))
		return 0;
	snprintf(buf, sizeof (buf), "您要撤回邮件<%.50s>吗?", fh->title);
	if (YEA == askyn(buf, NA, NA)) {
		setmailfile_s(buf, sizeof(buf), receiver, fh2fname(fh));
		mail_file(buf, currentuser.userid, "[系统]撤回的邮件备份");
		fp = fopen(buf, "r+");
		if (NULL == fp) {
			prints("不能打开文件写,请联系系统维护!");
			return 2;
		}
		keepoldheader(fp, SKIPHEADER);
		now = time(0);
		fprintf(fp, "本文已经于 %s 被 %s 撤回\n", ytht_ctime(now),
				currentuser.userid);
		ftruncate(fileno(fp), ftell(fp));
		fclose(fp);
		return 1;
	}
	return 0;
}

static int
max_mail_size()
{
	int maxsize;
	/*maxsize = (HAS_PERM(PERM_SYSOP)
		|| HAS_PERM(PERM_SPECIAL1)) ?  MAX_SYSOPMAIL_HOLD : (HAS_PERM(PERM_ARBITRATE)
		|| HAS_PERM(PERM_BOARDS)) ?  MAX_MAIL_HOLD * 8 : MAX_MAIL_HOLD;
	maxsize = maxsize * 10;
	return maxsize;*/
	maxsize= (HAS_PERM(PERM_SYSOP, currentuser))?MAX_SYSOPMAIL_HOLD:HAS_PERM(PERM_SPECIAL1, currentuser)?MAX_MAIL_HOLD*20:
		(HAS_PERM(PERM_BOARDS, currentuser))?MAX_MAIL_HOLD*8:MAX_MAIL_HOLD*3;
	maxsize=maxsize*10;
	//modified by wjbta@bmy 修改信箱最大容量控制
	return maxsize;
}

static int
get_mail_size()
{
	static time_t readtime = 0;
	static int currmailsize;
	int currmsgsize = 0;
	char tmpmail[STRLEN], buf[STRLEN];
	struct fileheader tmpfh;
	int fd;
	time_t t;
	t = file_time(currmaildir);
	if (!t)
		return 0;
	if (t >= readtime) {
		time(&readtime);
		currmailsize = file_size(currmaildir);
		fd = open(currmaildir, O_RDONLY);
		if (fd < 0)
			return 0;
		while (read(fd, &tmpfh, sizeof (tmpfh)) == sizeof (tmpfh)) {
			setmailfile_s(tmpmail, sizeof(tmpmail), currentuser.userid, fh2fname(&tmpfh));
			currmailsize += file_size(tmpmail);
		}
		close(fd);
	}
	sethomefile_s(buf, sizeof(buf), currentuser.userid, "msgindex");
	if (file_time(buf))
		currmsgsize = file_size(buf);
	sethomefile_s(buf, sizeof(buf), currentuser.userid, "msgindex2");
	if (file_time(buf))
		currmsgsize += file_size(buf);
	sethomefile_s(buf, sizeof(buf), currentuser.userid, "msgcontent");
	if (file_time(buf))
		currmsgsize += file_size(buf);
	currmsgsize = (currmailsize + currmsgsize) / 1024;
	return currmsgsize;
}

int m_cancel(const char *userid) {
	char uident[STRLEN], buf[STRLEN];

	if (check_mail_perm()) {
		pressreturn();
		return 0;
	}
	if ((uinfo.mode != LUSERS && uinfo.mode != LAUSERS && uinfo.mode != FRIEND && uinfo.mode != GMENU) || userid == NULL) {
		move(1, 0);
		clrtoeol();
		modify_user_mode(SMAIL);
		usercomplete("撤回给谁的信件： ", uident);
		if (uident[0] == '\0') {
			return FULLUPDATE;
		} else if (!strcmp(currentuser.userid, uident)) {
			prints("自己给自己写信就够变态了,居然还要自己撤回啊?");
			pressreturn();
			return FULLUPDATE;
		}
	} else
		strcpy(uident, userid);
	clear();
	setmailfile_s(buf, sizeof(buf), uident, ".DIR");
	if (!new_apply_record(buf, sizeof (struct fileheader), (void *) m_cancel_1, uident))
		prints("没有找到可以撤回的信件\n");
	pressreturn();
	return FULLUPDATE;
}

static int
check_maxmail()
{
	int currsize, maxsize;
	currsize = 0;
	if(HAS_PERM(PERM_SYSOP|PERM_OBOARDS, currentuser))
		return 0;//add by bjgyt
	maxsize = max_mail_size();
	currsize = get_mail_size();
	if (currsize > maxsize+20) {
		clear();
		move(2,2);
		prints("\n\033[1;37m 您的私人信件总大小\033[1;31m高达 %d k\033[0m,\033[1;37m 请删除过期信件,\033[1;32m保持%d k 内。\033[0m\n\n ", currsize, maxsize);
		prints("\033[1;37m当信件和站内消息总大小\033[1;33m超过 %d k\033[0m \033[1;37m时, 你将无法使用本站的送信功能。 \033[0m\n", maxsize + 20);
//		if (currsize > maxsize * 2) {
//			sprintf(genbuf, "私人信件过量: %d k", currsize);
//			securityreport(genbuf, genbuf);
//		}
	}
	if (currsize > maxsize + 20)
		return (1);
	else
		return (0);
}

int post_reply(int ent, struct fileheader *fileinfo, char *direct) {
	(void) ent;
	(void) direct;
	char uid[STRLEN];
	char title[STRLEN];
	if (!strcmp(currentuser.userid, "guest"))
		return DONOTHING;

	clear();
	if (check_maxmail()) {
		pressreturn();
		return FULLUPDATE;
	}
	if (check_mail_perm()) {
		pressreturn();
		return FULLUPDATE;
	}
	modify_user_mode(SMAIL);
/* indicate the quote file/user */
	setbfile(quote_file, sizeof(quote_file), currboard, fh2fname(fileinfo));
	strcpy(quote_user, fh2owner(fileinfo));
/* find the author */
	if (!getuser(quote_user)) {
		getdocauthor(quote_file, uid, sizeof (uid));
		if (invalidaddr(uid)) {
			prints("Error: Cannot find Author ... \n");
			pressreturn();
		}
	} else
		ytht_strsncpy(uid, quote_user, sizeof(uid));
	/* make the title */
	if (toupper(fileinfo->title[0]) != 'R' || fileinfo->title[1] != 'e' || fileinfo->title[2] != ':')
		strcpy(title, "Re: ");
	else
		title[0] = '\0';
	strncat(title, fileinfo->title, STRLEN - 5);
/* edit, then send the mail */
	switch (do_send(uid, title)) {
	case -1:
		prints("系统无法送信\n");
		break;
	case -2:
		prints("送信动作已经中止\n");
		break;
	case -3:
		prints("使用者 '%s' 无法收信\n", uid);
		break;
	default:
		prints("信件已成功地寄给原作者 %s\n", uid);
	}
	pressreturn();
	return FULLUPDATE;
}

static int
check_mail_perm()
{
	if (HAS_PERM(PERM_DENYMAIL, currentuser)) {
		prints("您被禁止发信");
		return -1;
	}
	return 0;
}

static int
show_user_notes()
{
	char buf[256];
	sethomefile_s(buf, sizeof(buf), currentuser.userid, "notes");
	if (dashf(buf)) {
		ansimore(buf, YEA);
		return FULLUPDATE;
	}
	clear();
	move(10, 15);
	prints("您尚未在 InfoEdit->WriteFile 编辑个人备忘录。\n");
	pressanykey();
	return FULLUPDATE;
}

static int
mailto(uentp)
struct userec *uentp;
{
	char filename[STRLEN];

//      sprintf(filename, "tmp/mailall.%s", currentuser.userid);
	sprintf(filename, MY_BBS_HOME "/mail/S/SYSOP/M.%ld.A", mailallmode);
	if (uentp->stay / 60 / 60 < mailhour)
		return 1;
	if (!strcmp(uentp->userid, "SYSOP"))
		return 1;
	if ((uentp->userlevel == PERM_BASIC && mailmode == 1) ||
			((uentp->userlevel & PERM_POST) && mailmode == 2) ||
			((uentp->userlevel & PERM_BOARDS) && mailmode == 3) ||
			((uentp->userlevel & PERM_ARBITRATE) && mailmode == 4) ||
			((uentp->userdefine & DEF_SEEWELC1) && mailmode == 5) ||
			((uentp->userlevel & PERM_SYSOP) && mailmode == 6)) {
		mail_file(filename, uentp->userid, save_title);
	}
	return 1;
}

static int mailtoall(int mode, int hour) {
	char filename[STRLEN];
	sprintf(filename, "tmp/mailall.%s", currentuser.userid);
	mailmode = mode;
	mailhour = hour;
	mailallmode = mail_file(filename, "SYSOP", save_title);
	if (mailallmode == -1) {
		prints("SYSOP 信箱故障，请联系系统维护人员。");
		pressreturn();
		return -1;
	}
	if (apply_record(PASSFILE, (void *) mailto, sizeof (struct userec)) == -1) {
		prints("No Users Exist");
		pressreturn();
		return -1;
	}
	mailallmode = 0;
	return 0;
}

