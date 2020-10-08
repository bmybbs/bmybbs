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
extern int mailallmode;
int G_SENDMODE = NA;

char currmaildir[STRLEN];
int filter_ansi;
int mailmode, mailhour;
int mailallmode = 0;
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
static int do_gsend(char *userid[], char *title, int num);
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
	lseek(fd, (st.st_size - (sizeof (struct fileheader) - offset)),
	      SEEK_SET);
/*ÀëÏß²éÑ¯ĞÂĞÅÖ»Òª²éÑ¯×îºóÒ»·âÊÇ·ñÎªĞÂĞÅ£¬ÆäËû²¢²»ÖØÒª*/
/*Modify by SmallPig*/
	read(fd, &accessed, sizeof (accessed));
	if (!(accessed & FH_READ)) {
		close(fd);
		return YEA;
	}
	close(fd);
	return NA;
}

int
mailall()
{
	char ans[4], fname[STRLEN], title[STRLEN];
	char doc[6][STRLEN], buf[STRLEN], str[7];
	int i;
	int hour;
	int save_in_mail;

	strcpy(title, "Ã»Ö÷Ìâ");
	modify_user_mode(SMAIL);
	clear();
	move(0, 0);
	sprintf(fname, "tmp/mailall.%s", currentuser.userid);
	prints("ÄãÒª¼Ä¸øËùÓĞµÄ£º\n");
	prints("(0) ·ÅÆú\n");
	strcpy(doc[0], "(1) ÉĞÎ´Í¨¹ıÉí·İÈ·ÈÏµÄÊ¹ÓÃÕß");
	strcpy(doc[1], "(2) ËùÓĞÍ¨¹ıÉí·İÈ·ÈÏµÄÊ¹ÓÃÕß");
	strcpy(doc[2], "(3) ËùÓĞµÄ°æÖ÷");
	strcpy(doc[3], "(4) ±¾Õ¾ÖÇÄÒÍÅ");
	strcpy(doc[4], "(5) ËùÓĞ±¾×éÖ¯»áÔ±");
	strcpy(doc[5], "(6) ËùÓĞSYSOP");
	for (i = 0; i < 6; i++)
		prints("%s\n", doc[i]);
	getdata(9, 0, "ÇëÊäÈëÄ£Ê½ (0~6)? [0]: ", ans, 2, DOECHO, YEA);
	if (ans[0] - '0' < 1 || ans[0] - '0' > 6) {
		return NA;
	}
	getdata(10, 0, "ÇëÊäÈëÉÏÕ¾Ê±¼äÏÂÏŞ(°´Ğ¡Ê±)[0]: ", str, 5, DOECHO, YEA);
	hour = atoi(str);
	sprintf(buf, "ÊÇ·ñÈ·¶¨¼Ä¸ø%s ²¢ÇÒÉÏÕ¾Ê±¼ä²»Ğ¡ÓÚ%dĞ¡Ê±?",
		doc[ans[0] - '0' - 1], hour);
	move(11, 0);
	if (askyn(buf, NA, NA) == NA)
		return NA;
	save_in_mail = in_mail;
	in_mail = YEA;
	header.reply_mode = NA;
	strcpy(header.title, "Ã»Ö÷Ìâ");
	strcpy(header.ds, doc[ans[0] - '0' - 1]);
	header.postboard = NA;
	if (post_header(&header))
		sprintf(save_title, "[Type %c ¹«¸æ] %.60s", ans[0],
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
	prints
	    ("[5;1;32;44mÕıÔÚ¼Ä¼şÖĞ£¬ÇëÉÔºò.....                                                        [m");
	refresh();
	mailtoall(ans[0] - '0', hour);
	move(t_lines - 1, 0);
	clrtoeol();
	unlink(fname);
	in_mail = save_in_mail;
	return 0;
}

#ifdef INTERNET_EMAIL

void
m_internet()
{
	char receiver[STRLEN];

	if (check_maxmail()) {
		pressreturn();
		return;
	}
	if (check_mail_perm()) {
		pressreturn();
		return;
	}
	modify_user_mode(SMAIL);

	getdata(1, 0, "ÊÕĞÅÈËE-mail£º", receiver, 65, DOECHO, YEA);
	sprintf(genbuf, ".bbs@%s", email_domain());
	if (strstr(receiver, genbuf)
	    || strstr(receiver, ".bbs@localhost")) {
		move(3, 0);
		prints("Õ¾ÄÚĞÅ¼ş, ÇëÓÃ (S)end Ö¸ÁîÀ´¼Ä\n");
		pressreturn();
	} else if (!invalidaddr(receiver)) {
		*quote_file = '\0';
		clear();
		do_send(receiver, NULL);
	} else {
		move(3, 0);
		prints("ÊÕĞÅÈË²»ÕıÈ·, ÇëÖØĞÂÑ¡È¡Ö¸Áî\n");
		pressreturn();
	}
	clear();
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
	int now;
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
	if (inoverride(currentuser.userid, uid, "rejects"))
		return -3;
/*add by KCN :) */

	if (!(lookupuser.userlevel & PERM_READMAIL))
		return -3;
	setmailfile(filepath, uid, "");
	if (stat(filepath, &st) == -1) {
		if (mkdir(filepath, 0775) == -1)
			return -1;
	} else {
		if (!(st.st_mode & S_IFDIR))
			return -1;
	}
	memset(&newmessage, 0, sizeof (newmessage));
	now = time(NULL);
	sprintf(fname, "M.%d.A", now);
	setmailfile(filepath, uid, fname);
	count = 0;
	while ((fp = open(filepath, O_CREAT | O_EXCL | O_WRONLY, 0644)) == -1) {
		now++;
		sprintf(fname, "M.%d.A", now);
		setmailfile(filepath, uid, fname);
		if (count++ > MAX_POSTRETRY) {
			return -1;
		}
	}
	close(fp);
	newmessage.filetime = now;
	newmessage.thread = now;
	ytht_strsncpy(newmessage.owner, currentuser.userid,
				  sizeof(newmessage.owner));

      edit_mail_file:
	if (title == NULL) {
		header.reply_mode = NA;
		strcpy(header.title, "Ã»Ö÷Ìâ");
	} else {
		header.reply_mode = YEA;
		strcpy(header.title, title);
	}
	header.postboard = NA;
	save_in_mail = in_mail;
	in_mail = YEA;

	setuserfile(genbuf, "signatures");
	ansimore2(genbuf, NA, 0, 18);
	strcpy(header.ds, uid);
	if (post_header(&header)) {
		ytht_strsncpy(newmessage.title, header.title,
					  sizeof(newmessage.title));
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
		prints("ĞÅ¼ş¼´½«¼Ä¸ø %s \n", uid);
		prints("±êÌâÎª£º %s \n", header.title);
		if (askyn("È·¶¨Òª¼Ä³öÂğ", YEA, NA) == NA) {
			prints("\nĞÅ¼şÒÑÈ¡Ïû...\n");
			res = -2;
		} else {
#ifdef SENDMAIL_MIME_AUTOCONVERT
			int ans;
			filter_ansi = 0;
			if (askyn("ÊÇ·ñ±¸·İ¸ø×Ô¼º", NA, NA) == YEA)
				mail_file(tmp_fname, currentuser.userid,
					  save_title2);
			ans = askyn("ÒÔ MIME ¸ñÊ½ËÍĞÅ", NA, NA);
			prints("ÇëÉÔºò, ĞÅ¼ş´«µİÖĞ...\n");
			refresh();
			res =
			    bbs_sendmail(tmp_fname, header.title, uid, ans);
#else
			if (askyn("ÊÇ·ñ±¸·İ¸ø×Ô¼º", NA, NA) == YEA)
				mail_file(tmp_fname, currentuser.userid,
					  save_title2);
			prints("ÇëÉÔºò, ĞÅ¼ş´«µİÖĞ...\n");
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
		if (askyn("ÊÇ·ñ±¸·İ¸ø×Ô¼º", NA, NA) == YEA)
			mail_file(filepath, currentuser.userid, save_title2);
		setmailfile(genbuf, uid, DOT_DIR);
		if (append_record(genbuf, &newmessage, sizeof (newmessage)) ==
		    -1) {
			in_mail = save_in_mail;
			return -1;
		}
		sprintf(genbuf, "%s mail %s", currentuser.userid, uid);
		newtrace(genbuf);
		in_mail = save_in_mail;
		return 0;
	}
}

int
m_send(userid)
char userid[];
{
	char uident[STRLEN];
	// ÓÀÔ¶¿ÉÒÔ¸ø SYSOP ·¢ĞÅ
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
                } //add by wjbta@bmy  µ±ĞÅ¼şÈİÁ¿³¬¹ıĞÅÏä×î´óÈİÁ¿Ê±£¬½ûÖ¹·¢ĞÅ
	modify_user_mode(SMAIL);
	if (			/*(uinfo.mode != LUSERS && uinfo.mode != LAUSERS
				   && uinfo.mode != FRIEND && uinfo.mode != GMENU)
				   || */ userid == NULL) {
		move(1, 0);
		clrtoeol();
		usercomplete("ÊÕĞÅÈË£º ", uident);
		if (uident[0] == '\0') {
			return FULLUPDATE;
		}
	} else
		strcpy(uident, userid);
	clear();
	*quote_file = '\0';
	switch (do_send(uident, NULL)) {
	case -1:
		prints("ÊÕĞÅÕß²»ÕıÈ·\n");
		break;
	case -2:
		prints("È¡Ïû\n");
		break;
	case -3:
		prints("[%s] ÎŞ·¨ÊÕĞÅ\n", uident);
		break;
	default:
		prints("ĞÅ¼şÒÑ¼Ä³ö\n");
	}
	pressreturn();
	return FULLUPDATE;
}

int
M_send()
{
	if (!HAS_PERM(PERM_LOGINOK, currentuser))
		return 0;
	return m_send(NULL);
}
static int
read_mail(fptr)
struct fileheader *fptr;
{
	setmailfile(genbuf, currentuser.userid, fh2fname(fptr));
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
	prints("¶ÁÈ¡ %s ¼ÄÀ´µÄ '%s' ?\n", fptr->owner, fptr->title);
	prints("(Yes, or No): ");
	getdata(1, 0, "(Y)¶ÁÈ¡ (N)²»¶Á (Q)Àë¿ª [Y]: ", genbuf, 3, DOECHO, YEA);
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
		prints("(R)»ØĞÅ, (D)É¾³ı, (G)¼ÌĞø ? [G]: ");
		switch (egetch()) {
		case 'R':
		case 'r':
			*quote_file = '\0';
			mail_reply(idc, fptr, currmaildir);
			break;
		case 'D':
		case 'd':
			delete_it = YEA;
		default:
			done = YEA;
		}
		if (!done) {
			setmailfile(fname, currentuser.userid, fh2fname(fptr));
			ansimore(fname, NA);	/* re-read */
		}
	}
	if (delete_it) {
		clear();
		prints("É¾³ıĞÅ¼ş '%s' ", fptr->title);
		getdata(1, 0, "(Y)es ,(N)o [N]: ", genbuf, 3, DOECHO, YEA);
		if (genbuf[0] == 'Y' || genbuf[0] == 'y') {	/* if not yes quit */
			setmailfile(fname, currentuser.userid, fh2fname(fptr));
			unlink(fname);
			delmsgs[delcnt++] = idc;
		}
	}
	clear();
	return 0;
}

int
m_new()
{
	clear();
	mrd = 0;
	modify_user_mode(RMAIL);
	read_new_mail(NULL);
	if (apply_record
	    (currmaildir, (void *) read_new_mail,
	     sizeof (struct fileheader)) == -1) {
		clear();
		move(0, 0);
		prints("No new messages\n\n\n");
		return -1;
	}
	if (delcnt) {
		while (delcnt--)
			delete_record(currmaildir, sizeof (struct fileheader),
				      delmsgs[delcnt]);
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
	showtitle("ÓÊ¼şÑ¡µ¥    ", MY_BBS_NAME);

	prints
	    ("Àë¿ª[[1;32m¡û[m,[1;32me[m]  Ñ¡Ôñ[[1;32m¡ü[m,[1;32m¡ı[m]  ÔÄ¶ÁĞÅ¼ş[[1;32m¡ú[m,[1;32mRtn[m]  »ØĞÅ[[1;32mR[m]  ¿³ĞÅ£¯Çå³ı¾ÉĞÅ[[1;32md[m,[1;32mD[m]  ÇóÖú[[1;32mh[m][m\n");
	prints(" [1;44m±àºÅ    %-12s %6s  %-24s %d %s %d %s[m\n", "·¢ĞÅÕß",
	       "ÈÕ  ÆÚ", "±ê  Ìâ      µ±Ç°ĞÅÏäÈİÁ¿", max_mail_size(),
	       "k,ÒÑÓÃ¿Õ¼ä", get_mail_size(), "k");
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
	/* Added by deardragon 1999.11.15 ¸øÒÑ»ØĞÅ¼ş¼ÓÉÏ "»ØĞÅ" ±ê¼Ç ('R') */
	if (ent->accessed & FH_REPLIED)	/* ÓÊ¼şÒÑ»Ø¸´ */
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
			" %s%3d\033[m %c%c %-12.12s %6.6s %c¡ï %s%.47s\033[m",
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
		prints("(R)»ØĞÅ, (D)É¾³ı, (G)¼ÌĞø? [G]: ");
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
		default:
			done = YEA;
		}
	}
	if (!delete_it && !(fileinfo->accessed & FH_READ)) {
		fileinfo->accessed |= FH_READ;
		substitute_record(currmaildir, fileinfo, sizeof (*fileinfo),
				  ent);
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
			prints("ÎŞ·¨Í¶µİ\n");
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
		setmailfile(quote_file, currentuser.userid, fh2fname(fileinfo));
	strcpy(quote_user, fileinfo->owner);
	switch (do_send(uid, title)) {
	case -1:
		prints("ÎŞ·¨Í¶µİ\n");
		break;
	case -2:
		prints("È¡Ïû»ØĞÅ\n");
		break;
	case -3:
		prints("[%s] ÎŞ·¨ÊÕĞÅ\n", uid);
		break;
	default:
		//add by gluon
		/* Added bye deardragon 1999.11.15 ¸øÒÑ»ØĞÅ¼ş¼ÓÉÏ "»ØĞÅ" ±ê¼Ç ('R') */
		if (ent >= 0) {
			fileinfo->accessed |= FH_REPLIED;
			substitute_record(direct, fileinfo, sizeof (*fileinfo),
					  ent);
		}
		/* Added End. */
		//end
		prints("ĞÅ¼şÒÑ¼Ä³ö\n");
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
	prints("É¾³ıĞÅ¼ş [%s] ", fileinfo->title);
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

int
mail_forward(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char buf[STRLEN];
	if (!HAS_PERM(PERM_FORWARD, currentuser)) {
		return DONOTHING;
	}
	directfile(buf, direct, fh2fname(fileinfo));
	switch (doforward(buf, fileinfo->title, 0)) {
	case 0:
		prints("ÎÄÕÂ×ª¼ÄÍê³É!\n");
		break;
	case -1:
		prints("×ª¼ÄÊ§°Ü: ÏµÍ³·¢Éú´íÎó.\n");
		break;
	case -2:
		prints("×ª¼ÄÊ§°Ü: ²»ÕıÈ·µÄÊÕĞÅµØÖ·.\n");
		break;
	default:
		prints("È¡Ïû×ª¼Ä...\n");
	}
	do_delay(1);
	pressreturn();
	clear();
	return FULLUPDATE;
}

int
mail_u_forward(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char buf[STRLEN];
	if (!HAS_PERM(PERM_FORWARD, currentuser)) {
		return DONOTHING;
	}
	directfile(buf, direct, fh2fname(fileinfo));
	switch (doforward(buf, fileinfo->title, 1)) {
	case 0:
		prints("ÎÄÕÂ×ª¼ÄÍê³É!\n");
		break;
	case -1:
		prints("×ª¼ÄÊ§°Ü: ÏµÍ³·¢Éú´íÎó.\n");
		break;
	case -2:
		prints("×ª¼ÄÊ§°Ü: ²»ÕıÈ·µÄÊÕĞÅµØÖ·.\n");
		break;
	default:
		prints("È¡Ïû×ª¼Ä...\n");
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

static int
mail_mark(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	if (fileinfo->accessed & FH_MARKED)
		fileinfo->accessed &= ~FH_MARKED;
	else
		fileinfo->accessed |= FH_MARKED;
	substitute_record(currmaildir, fileinfo, sizeof (*fileinfo), ent);
	return (PARTUPDATE);
}

// µÈ¼Û×Ó
typedef int(*equalor)(const struct fileheader*, void *);
// ±ê¼ÇµÄÓÊ¼ş
static int ismark(const struct fileheader *mail, void *ext)
{
	if (mail->accessed & FH_MARKED)
		return 1;
    return 0;
}
// ´ø¸½¼şµÄÓÊ¼ş
static int isattach(const struct fileheader *mail, void *ext)
{
	if (mail->accessed & FH_ATTACHED)
		return 1;
    return 0;
}
// ÌØ¶¨±êÌâµÄÓÊ¼ş
static int istitle(const struct fileheader *mail, void *ext)
{
	if (strstr2(mail->title, (char*)ext) != NULL)
		return 1;
    return 0;
}
// ÌØ¶¨·¢ĞÅÈËµÄÓÊ¼ş
static int issender(const struct fileheader *mail, void *ext)
{
	if (strncasecmp(mail->owner, (char*)ext, sizeof(mail->owner)+1) == 0) // ²»Çø·Ö´óĞ¡Ğ´
		return 1;
    return 0;
}

// ¶ÔÓÊÏä½øĞĞËÑË÷£¬²úÉúĞÂµÄ.DIRÎÄ¼ş£¨×÷ÎªËÑË÷½á¹û£©
// interma@bmy, 2006-11-23
static void do_search_mailbox(int type, const char *whattosearch, const char *tempdir)
{
	FILE *fdir;
	FILE *ftempdir;
	equalor isequal;

	fdir = fopen(currmaildir, "r");
	if (fdir == NULL)
		return;
	ftempdir = fopen(tempdir, "w");
	if (ftempdir == NULL)
		return;

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

// ½«ÓÊÏäË÷ÒıÎÄ¼ş(.DIR)µÄÂ·¾¶Ãû½ØÈ¡³öÀ´
// interma@bmy, 2006-11-23
static char *truncateDIR(char *dest)
{
	char *p = NULL;
	if ((p=strstr(dest, ".DIR")) != NULL)
	{
		*p = '\0';
	}
	return dest;
}

struct one_key query_comms[];

// ÓÊÏä²éÑ¯¹¦ÄÜ£¬ÓÊÏä½çÃæÖĞctrl+g
// interma@bmy, 2006-11-23
static int
mail_query(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	//power_action(currmaildir, 1, -1, "±ê¼Çº¬ m", 9);
	char ans[3];
	int type;
	char whattosearch[31];

	ans[0] = '\0';
	getdata(t_lines - 1 , 0, "ÇëÑ¡Ôñ:0)È¡Ïû 1)±»mÓÊ¼ş 2)¸½¼ş 3)±êÌâ¹Ø¼ü×Ö 4)Í¬×÷Õß [0]:",
	   ans, 2, DOECHO, NA);
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
		getdata(t_lines - 1, 0, "ÇëÊäÈë±êÌâ¹Ø¼ü×Ö:", whattosearch, 31, DOECHO,NA);
		if (whattosearch[0] == '\0')
			return PARTUPDATE;
		break;
	case 4:
		getdata(t_lines - 1, 0, "ÇëÊäÈë×÷ÕßID:",whattosearch, 31, DOECHO, NA);
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
	return DOQUIT; // Èç¹û²»ÊÇDOQUIT£¬ÄÇ¾ÍÒª¶à°´¼¸ÏÂ"ºóÍË"²ÅÄÜÍË³ö£¬ºÇºÇ
}

// ÓÊÏä²éÑ¯½çÃæÖĞ¿ÉÒÔÊ¹ÓÃµÄÃüÁî
struct one_key query_comms[] = {
	//{'d', mail_del, "É¾³ıĞÅ¼ş"},
	//{'D', mail_del_range, "Çø¶ÎÉ¾³ı"},
	{Ctrl('P'), M_send, "·¢ËÍĞÅ¼ş"},
	{'E', edit_post, "±à¼­ĞÅ¼ş"},
	{'r', mail_read, "ÔÄ¶ÁĞÅ¼ş"},
	{'R', mail_reply, "»Ø¸´ĞÅ¼ş"},
	//{'m', mail_mark, "markĞÅ¼ş"},
	{'i', Save_post, "½«ÎÄÕÂ´æÈëÔİ´æµµ"},
	{'I', Import_post, "½«ĞÅ¼ş·ÅÈë¾«»ªÇø"},
	{'x', into_announce, "½øÈë¾«»ªÇø"},
	{KEY_TAB, show_user_notes, "²é¿´ÓÃ»§±¸ÍüÂ¼"},
#ifdef INTERNET_EMAIL
	{'F', mail_forward, "×ª¼ÄĞÅ¼ş"},
	{'U', mail_u_forward, "uuencode ×ª¼Ä"},
#endif
	{'a', auth_search_down, "ÏòºóËÑË÷×÷Õß"},
	{'A', auth_search_up, "ÏòÇ°ËÑË÷×÷Õß"},
	{'/', t_search_down, "ÏòºóËÑË÷±êÌâ"},
	{'?', t_search_up, "ÏòÇ°ËÑË÷±êÌâ"},
	{'\'', post_search_down, "ÏòºóËÑË÷ÄÚÈİ"},
	{'\"', post_search_up, "ÏòÇ°ËÑË÷ÄÚÈİ"},
	{']', thread_down, "ÏòºóÍ¬Ö÷Ìâ"},
	{'[', thread_up, "ÏòÇ°Í¬Ö÷Ìâ"},
	{Ctrl('A'), show_author, "×÷Õß¼ò½é"},
	{'\\', SR_last, "×îºóÒ»ÆªÍ¬Ö÷ÌâÎÄÕÂ"},
	{'=', SR_first, "µÚÒ»ÆªÍ¬Ö÷ÌâÎÄÕÂ"},
	{'L', show_allmsgs, "²é¿´ÏûÏ¢"},
	{Ctrl('C'), do_cross, "×ªÌùÎÄÕÂ"},
	{'n', SR_first_new, "Ö÷ÌâÎ´¶ÁµÄµÚÒ»Æª"},
	{'p', SR_read, "ÏàÍ¬Ö÷ÌâµÄÔÄ¶Á"},
	{Ctrl('U'), SR_author, "ÏàÍ¬×÷ÕßÔÄ¶Á"},
	{'h', mailreadhelp, "²é¿´°ïÖú"},
	{'!', Q_Goodbye, "¿ìËÙÀëÕ¾"},
	{'S', s_msg, "´«ËÍÑ¶Ï¢"},
	{'c', t_friends, "²é¿´ºÃÓÑ"},
	{'C', friend_author, "Ôö¼Ó×÷ÕßÎªºÃÓÑ"},
	{Ctrl('E'), mail_rjunk, "·­¼ğÀ¬»ø"},
	//{Ctrl('G'), mail_query, "²éÑ¯Ä£Ê½"},
	{'\0', NULL, ""}
};

// ÓÊÏäÔÄ¶Á½çÃæ£¨Õı³£Ä£Ê½£©ÖĞ¿ÉÒÔÊ¹ÓÃµÄÃüÁî
struct one_key mail_comms[] = {
	{'d', mail_del, "É¾³ıĞÅ¼ş"},
	{'D', mail_del_range, "Çø¶ÎÉ¾³ı"},
	{Ctrl('P'), M_send, "·¢ËÍĞÅ¼ş"},
	{'E', edit_post, "±à¼­ĞÅ¼ş"},
	{'T', edit_title, "ĞŞ¸Ä±êÌâ"},
	{'r', mail_read, "ÔÄ¶ÁĞÅ¼ş"},
	{'R', mail_reply, "»Ø¸´ĞÅ¼ş"},
	{'m', mail_mark, "markĞÅ¼ş"},
	{'i', Save_post, "½«ÎÄÕÂ´æÈëÔİ´æµµ"},
	{'I', Import_post, "½«ĞÅ¼ş·ÅÈë¾«»ªÇø"},
	{'x', into_announce, "½øÈë¾«»ªÇø"},
	{KEY_TAB, show_user_notes, "²é¿´ÓÃ»§±¸ÍüÂ¼"},
#ifdef INTERNET_EMAIL
	{'F', mail_forward, "×ª¼ÄĞÅ¼ş"},
	{'U', mail_u_forward, "uuencode ×ª¼Ä"},
#endif
	{'a', auth_search_down, "ÏòºóËÑË÷×÷Õß"},
	{'A', auth_search_up, "ÏòÇ°ËÑË÷×÷Õß"},
	{'/', t_search_down, "ÏòºóËÑË÷±êÌâ"},
	{'?', t_search_up, "ÏòÇ°ËÑË÷±êÌâ"},
	{'\'', post_search_down, "ÏòºóËÑË÷ÄÚÈİ"},
	{'\"', post_search_up, "ÏòÇ°ËÑË÷ÄÚÈİ"},
	{']', thread_down, "ÏòºóÍ¬Ö÷Ìâ"},
	{'[', thread_up, "ÏòÇ°Í¬Ö÷Ìâ"},
	{Ctrl('A'), show_author, "×÷Õß¼ò½é"},
	{'\\', SR_last, "×îºóÒ»ÆªÍ¬Ö÷ÌâÎÄÕÂ"},
	{'=', SR_first, "µÚÒ»ÆªÍ¬Ö÷ÌâÎÄÕÂ"},
	{'L', show_allmsgs, "²é¿´ÏûÏ¢"},
	{Ctrl('C'), do_cross, "×ªÌùÎÄÕÂ"},
	{'n', SR_first_new, "Ö÷ÌâÎ´¶ÁµÄµÚÒ»Æª"},
	{'p', SR_read, "ÏàÍ¬Ö÷ÌâµÄÔÄ¶Á"},
	{Ctrl('U'), SR_author, "ÏàÍ¬×÷ÕßÔÄ¶Á"},
	{'h', mailreadhelp, "²é¿´°ïÖú"},
	{'!', Q_Goodbye, "¿ìËÙÀëÕ¾"},
	{'S', s_msg, "´«ËÍÑ¶Ï¢"},
	{'c', t_friends, "²é¿´ºÃÓÑ"},
	{'C', friend_author, "Ôö¼Ó×÷ÕßÎªºÃÓÑ"},
	{Ctrl('E'), mail_rjunk, "·­¼ğÀ¬»ø"},
	{Ctrl('G'), mail_query, "²éÑ¯Ä£Ê½"},
	{'\0', NULL, ""}
};

// ÓÊÏäÔÄ¶Á½çÃæ£¨Õı³£Ä£Ê½£©ÖĞ¿ÉÒÔÊ¹ÓÃµÄÃüÁî
// Ä¬ÈÏÖµ
const struct one_key mail_default_comms[] = {
	{'d', mail_del, "É¾³ıĞÅ¼ş"},
	{'D', mail_del_range, "Çø¶ÎÉ¾³ı"},
	{Ctrl('P'), M_send, "·¢ËÍĞÅ¼ş"},
	{'E', edit_post, "±à¼­ĞÅ¼ş"},
	{'T', edit_title, "ĞŞ¸Ä±êÌâ"},
	{'r', mail_read, "ÔÄ¶ÁĞÅ¼ş"},
	{'R', mail_reply, "»Ø¸´ĞÅ¼ş"},
	{'m', mail_mark, "markĞÅ¼ş"},
	{'i', Save_post, "½«ÎÄÕÂ´æÈëÔİ´æµµ"},
	{'I', Import_post, "½«ĞÅ¼ş·ÅÈë¾«»ªÇø"},
	{'x', into_announce, "½øÈë¾«»ªÇø"},
	{KEY_TAB, show_user_notes, "²é¿´ÓÃ»§±¸ÍüÂ¼"},
#ifdef INTERNET_EMAIL
	{'F', mail_forward, "×ª¼ÄĞÅ¼ş"},
	{'U', mail_u_forward, "uuencode ×ª¼Ä"},
#endif
	{'a', auth_search_down, "ÏòºóËÑË÷×÷Õß"},
	{'A', auth_search_up, "ÏòÇ°ËÑË÷×÷Õß"},
	{'/', t_search_down, "ÏòºóËÑË÷±êÌâ"},
	{'?', t_search_up, "ÏòÇ°ËÑË÷±êÌâ"},
	{'\'', post_search_down, "ÏòºóËÑË÷ÄÚÈİ"},
	{'\"', post_search_up, "ÏòÇ°ËÑË÷ÄÚÈİ"},
	{']', thread_down, "ÏòºóÍ¬Ö÷Ìâ"},
	{'[', thread_up, "ÏòÇ°Í¬Ö÷Ìâ"},
	{Ctrl('A'), show_author, "×÷Õß¼ò½é"},
	{'\\', SR_last, "×îºóÒ»ÆªÍ¬Ö÷ÌâÎÄÕÂ"},
	{'=', SR_first, "µÚÒ»ÆªÍ¬Ö÷ÌâÎÄÕÂ"},
	{'L', show_allmsgs, "²é¿´ÏûÏ¢"},
	{Ctrl('C'), do_cross, "×ªÌùÎÄÕÂ"},
	{'n', SR_first_new, "Ö÷ÌâÎ´¶ÁµÄµÚÒ»Æª"},
	{'p', SR_read, "ÏàÍ¬Ö÷ÌâµÄÔÄ¶Á"},
	{Ctrl('U'), SR_author, "ÏàÍ¬×÷ÕßÔÄ¶Á"},
	{'h', mailreadhelp, "²é¿´°ïÖú"},
	{'!', Q_Goodbye, "¿ìËÙÀëÕ¾"},
	{'S', s_msg, "´«ËÍÑ¶Ï¢"},
	{'c', t_friends, "²é¿´ºÃÓÑ"},
	{'C', friend_author, "Ôö¼Ó×÷ÕßÎªºÃÓÑ"},
	{Ctrl('E'), mail_rjunk, "·­¼ğÀ¬»ø"},
	{Ctrl('G'), mail_query, "²éÑ¯Ä£Ê½"},
	{'\0', NULL, ""}
};

int
m_read()
{
	int savemode;
	int save_in_mail;
	save_in_mail = in_mail;
	in_mail = YEA;
	savemode = uinfo.mode;
	m_init();
	i_read(RMAIL, currmaildir, mailtitle, (void *) maildoent, mail_comms,
	       sizeof (struct fileheader));
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
	if (fin == NULL || fout == NULL)
		return -1;

	fprintf(fout, "Return-Path: %s.bbs@%s\n", currentuser.userid,
		email_domain());
	fprintf(fout, "Reply-To: %s.bbs@%s\n", currentuser.userid,
		email_domain());
	fprintf(fout, "From: %s.bbs@%s\n", currentuser.userid, email_domain());
	fprintf(fout, "To: %s\n", receiver);
	fprintf(fout, "Subject: %s\n", title);
	fprintf(fout, "X-Forwarded-By: %s (%s)\n", currentuser.userid,
		currentuser.username);

	fprintf(fout, "X-Disclaimer: %s ¶Ô±¾ĞÅÄÚÈİË¡²»¸ºÔğ¡£\n", MY_BBS_NAME);
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

int
g_send()
{
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
	sethomefile(maillists, currentuser.userid, "maillist");
	cnt = listfilecontent(maillists);
	while (1) {
		if (cnt > maxrecp - 10) {
			move(2, 0);
			prints("Ä¿Ç°ÏŞÖÆ¼ÄĞÅ¸ø [1m%d[m ÈË", maxrecp);
		}
		if (keepgoing) {
			tmp[0]='a';
			tmp[1]=0;
		}
		else {
		getdata(0, 0,
			"(A)Ôö¼Ó (D)É¾³ı (I)ÒıÈëºÃÓÑ (C)Çå³ıÄ¿Ç°Ãûµ¥ (E)·ÅÆú (S)¼Ä³ö? [S]£º ",
			tmp, 2, DOECHO, YEA);

		}
		if (tmp[0] == '\n' || tmp[0] == '\0' || tmp[0] == 's'
		    || tmp[0] == 'S') {
			break;
		}
		if (tmp[0] == 'a' || tmp[0] == 'd' || tmp[0] == 'A'
		    || tmp[0] == 'D') {
			move(1, 0);
			if (tmp[0] == 'a' || tmp[0] == 'A')
				usercomplete
				    ("ÇëÒÀ´ÎÊäÈëÊ¹ÓÃÕß´úºÅ(Ö»°´ ENTER ½áÊøÊäÈë): ",
				     uident);
			else
				namecomplete
				    ("ÇëÒÀ´ÎÊäÈëÊ¹ÓÃÕß´úºÅ(Ö»°´ ENTER ½áÊøÊäÈë): ",
				     uident);
			move(1, 0);
			clrtoeol();
			if (uident[0] == '\0') {
				keepgoing = 0;
				continue;
			}
			keepgoing = 1;
			if (!getuser(uident)) {
				move(2, 0);
				prints("Õâ¸öÊ¹ÓÃÕß´úºÅÊÇ´íÎóµÄ.\n");
			}
		}
		switch (tmp[0]) {
		case 'A':
		case 'a':
			if (!(lookupuser.userlevel & PERM_READMAIL)) {
				move(2, 0);
				prints("ÎŞ·¨ËÍĞÅ¸ø: [1m%s[m\n",
				       lookupuser.userid);
				break;
			} else if (seek_in_file(maillists, uident)) {
				move(2, 0);
				prints("ÒÑ¾­ÁĞÎªÊÕ¼şÈËÖ®Ò» \n");
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
				getuserid(uident, uinfo.friend[n]);
				prints("%s\n", uident);
				move(3, 0);
				n++;
				prints
				    ("(A)È«²¿¼ÓÈë (Y)¼ÓÈë (N)²»¼ÓÈë (Q)½áÊø? [Y]:");
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
				if (key == '\0' || key == '\n' || key == 'y'
				    || key == 'Y') {
					if (!getuser(uident)) {
						move(4, 0);
						prints
						    ("Õâ¸öÊ¹ÓÃÕß´úºÅÊÇ´íÎóµÄ.\n");
						i--;
						continue;
					} else
					    if (!
						(lookupuser.userlevel &
						 PERM_READMAIL)) {
						move(4, 0);
						prints
						    ("ÎŞ·¨ËÍĞÅ¸ø: [1m%s[m\n",
						     lookupuser.userid);
						i--;
						continue;
					} else
					    if (seek_in_file(maillists, uident))
					{
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
		switch (do_gsend(NULL, NULL, cnt)) {
		case -1:
			prints("ĞÅ¼şÄ¿Â¼´íÎó\n");
			break;
		case -2:
			prints("È¡Ïû\n");
			break;
		default:
			prints("ĞÅ¼şÒÑ¼Ä³ö\n");
		}
		G_SENDMODE = 0;
		pressreturn();
	}
	return 0;
}

/*Add by SmallPig*/

static int
do_gsend(userid, title, num)
char *userid[], *title;
int num;
{
	struct stat st;
	char filepath[STRLEN], tmpfile[STRLEN];
	int cnt;
	FILE *mp = NULL;
	int save_in_mail;

	save_in_mail = in_mail;
	in_mail = YEA;
	sprintf(genbuf, "%s (%s)", currentuser.userid, currentuser.username);
	header.reply_mode = NA;
	strcpy(header.title, "Ã»Ö÷Ìâ");
	strcpy(header.ds, "¼ÄĞÅ¸øÒ»ÈºÈË");
	header.postboard = NA;
	sprintf(tmpfile, "tmp/gsend.%s.%05d", currentuser.userid, uinfo.pid);
	if (post_header(&header)) {
		sprintf(save_title, "[ÈºÌåĞÅ¼ş] %.60s", header.title);
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
	prints("[5;1;32mÕıÔÚ¼Ä¼şÖĞ£¬ÇëÉÔºò...[m");
	if (G_SENDMODE == 2) {
		char maillists[STRLEN];

		setuserfile(maillists, "maillist");
		if ((mp = fopen(maillists, "r")) == NULL) {
			in_mail = save_in_mail;
			return -3;
		}
	}
	if (G_SENDMODE == 3) {
		char maillists[STRLEN];
		G_SENDMODE = 2;
		setbfile(maillists, currboard, "club_users");
		if ((mp = fopen(maillists, "r")) == NULL) {
			in_mail = save_in_mail;
			return -3;
		}
	}
	for (cnt = 0; cnt < num; cnt++) {
		char uid[13];
		char buf[STRLEN];

		if (G_SENDMODE == 1)
			getuserid(uid, uinfo.friend[cnt]);
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

static int
do_gsend_voter(userid, title, num,fname)
char *userid[], *title;
int num;
char* fname;
{
	struct stat st;
	char filepath[STRLEN], tmpfile[STRLEN];
	int cnt;
	FILE *mp = NULL;
	int save_in_mail;

	save_in_mail = in_mail;
	in_mail = YEA;
	sprintf(genbuf, "%s (%s)", currentuser.userid, currentuser.username);
	header.reply_mode = NA;
	strcpy(header.title, "Ã»Ö÷Ìâ");
	strcpy(header.ds, "¼ÄĞÅ¸øÒ»ÈºÈË");
	header.postboard = NA;
	sprintf(tmpfile, "tmp/gsend.%s.%05d", currentuser.userid, uinfo.pid);
	if (post_header(&header)) {
		sprintf(save_title, "[ÈºÌåĞÅ¼ş] %.60s", header.title);
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
	prints("[5;1;32mÕıÔÚ¼Ä¼şÖĞ£¬ÇëÉÔºò...[m");
	if (G_SENDMODE == 4) {
		char maillists[STRLEN];
		G_SENDMODE = 2;
		setbfile(maillists, currboard,fname);
		if ((mp = fopen(maillists, "r")) == NULL) {
			in_mail = save_in_mail;
			return -3;
		}
	}
	for (cnt = 0; cnt < num; cnt++) {
		char uid[13];
		char buf[STRLEN];

		if (G_SENDMODE == 1)
			getuserid(uid, uinfo.friend[cnt]);
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

int
mail_file(tmpfile, userid, title)
char tmpfile[STRLEN], userid[STRLEN], title[STRLEN];
{
	struct fileheader newmessage;
	struct stat st;
	char fname[STRLEN], filepath[STRLEN];
	int fp, count, now;

	memset(&newmessage, 0, sizeof (newmessage));
	ytht_strsncpy(newmessage.owner, currentuser.userid,
				  sizeof(newmessage.owner));
	ytht_strsncpy(newmessage.title, title, sizeof(newmessage.title));
	ytht_strsncpy(save_title, newmessage.title, sizeof(save_title));

	setmailfile(filepath, userid, "");
	if (stat(filepath, &st) == -1) {
		if (mkdir(filepath, 0775) == -1)
			return -1;
	} else {
		if (!(st.st_mode & S_IFDIR))
			return -1;
	}
	now = time(NULL);
	sprintf(fname, "M.%d.A", now);
	setmailfile(filepath, userid, fname);
	count = 0;
	while ((fp = open(filepath, O_CREAT | O_EXCL | O_WRONLY, 0644)) == -1) {
		now++;
		sprintf(fname, "M.%d.A", now);
		setmailfile(filepath, userid, fname);
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
	setmailfile(genbuf, userid, DOT_DIR);
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
	int count, tmpinmail, now, fd;
	FILE *fp;

	memset(&newmessage, 0, sizeof (newmessage));
	ytht_strsncpy(newmessage.owner, currentuser.userid,
				  sizeof(newmessage.owner));
	ytht_strsncpy(newmessage.title, title, sizeof(newmessage.title));
	ytht_strsncpy(save_title, newmessage.title, sizeof(save_title));

	setmailfile(filepath, userid, "");
	if (stat(filepath, &st) == -1) {
		if (mkdir(filepath, 0775) == -1)
			return -1;
	} else {
		if (!(st.st_mode & S_IFDIR))
			return -1;
	}
	now = time(NULL);
	sprintf(fname, "M.%d.A", now);
	setmailfile(filepath, userid, fname);
	count = 0;
	while ((fd = open(filepath, O_CREAT | O_EXCL | O_WRONLY, 0644)) == -1) {
		now++;
		sprintf(fname, "M.%d.A", now);
		setmailfile(filepath, userid, fname);
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
	setmailfile(genbuf, userid, DOT_DIR);
	if (append_record(genbuf, &newmessage, sizeof (newmessage)) == -1)
		return -1;
	sprintf(genbuf, "%s mail %s", currentuser.userid, userid);
	newtrace(genbuf);
	return 0;
}

/*Add by SmallPig*/
int
ov_send()
{
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
	prints("¼ÄĞÅ¸øºÃÓÑÃûµ¥ÖĞµÄÈË£¬Ä¿Ç°±¾Õ¾ÏŞÖÆ½ö¿ÉÒÔ¼Ä¸ø [1m%d[m Î»¡£\n",
	       maxrecp);
	if (uinfo.fnum <= 0) {
		prints("Äã²¢Ã»ÓĞÉè¶¨ºÃÓÑ¡£\n");
		pressanykey();
		clear();
		return 0;
	} else {
		prints("Ãûµ¥ÈçÏÂ£º\n");
	}
	G_SENDMODE = 1;
	all = (uinfo.fnum >= maxrecp) ? maxrecp : uinfo.fnum;
	for (i = 0; i < all; i++) {
		char uid[IDLEN + 2];

		getuserid(uid, uinfo.friend[i]);
		prints("%-12s ", uid);
		if ((i + 1) % 6 == 0)
			outc('\n');
	}
	pressanykey();
	switch (do_gsend(NULL, NULL, all)) {
	case -1:
		prints("ĞÅ¼şÄ¿Â¼´íÎó\n");
		break;
	case -2:
		prints("ĞÅ¼şÈ¡Ïû\n");
		break;
	default:
		prints("ĞÅ¼şÒÑ¼Ä³ö\n");
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
	setbfile(maillists, currboard, fname);
	all = listfilecontent(maillists);
	switch (do_gsend_voter(NULL, NULL, all,fname)) {
	case -1:
		prints("ĞÅ¼şÄ¿Â¼´íÎó\n");
		break;
	case -2:
		prints("ĞÅ¼şÈ¡Ïû\n");
		break;
	default:
		prints("ĞÅ¼şÒÑ¼Ä³ö\n");
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
	setbfile(maillists, currboard, "club_users");
	all = listfilecontent(maillists);
	switch (do_gsend(NULL, NULL, all)) {
	case -1:
		prints("ĞÅ¼şÄ¿Â¼´íÎó\n");
		break;
	case -2:
		prints("ĞÅ¼şÈ¡Ïû\n");
		break;
	default:
		prints("ĞÅ¼şÒÑ¼Ä³ö\n");
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
		prints
		    ("ÇëÖ±½Ó°´ Enter ½ÓÊÜÀ¨ºÅÄÚÌáÊ¾µÄµØÖ·, »òÕßÊäÈëÆäËûµØÖ·\n");
		prints("°ÑĞÅ¼ş×ª¼Ä¸ø [%s]\n", address);
		move(2,0);
		usercomplete("==>", receiver);
		if (receiver[0] != '\0') {
			strncpy(address, receiver, STRLEN);
		}
	}
	sprintf(genbuf, ".bbs@%s", email_domain());
	if (strstr(receiver, genbuf)
	    || strstr(receiver, ".bbs@localhost")) {
		char *pos;

		pos = strchr(address, '.');
		*pos = '\0';
	}
	if (check_maxmail())
		return -1;
	sprintf(genbuf, "È·¶¨½«ÎÄÕÂ¼Ä¸ø %s Âğ", address);
	if (askyn(genbuf, YEA, NA) == 0)
		return 1;
	if (invalidaddr(address))
		if (!getuser(address) || check_mail_perm())
			return -2;
	sprintf(tmpfname, "tmp/forward.%s.%05d", currentuser.userid, uinfo.pid);
	copyfile(filepath, tmpfname);
	if (askyn("ÊÇ·ñĞŞ¸ÄÎÄÕÂÄÚÈİ", NA, NA) == 1) {
		if (vedit(tmpfname, NA, NA) == -1) {
			if (askyn("ÊÇ·ñ¼Ä³öÎ´ĞŞ¸ÄµÄÎÄÕÂ", YEA, NA) == 0) {
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
	prints("×ª¼ÄĞÅ¼ş¸ø %s, ÇëÉÔºò....\n", address);
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
		snprintf(uuname, sizeof (uuname), "%s-BBSMAIL.%d", MY_BBS_ID,
			 (int) time(NULL));
		uuencode(fr, fw, file_size(tmpfname), uuname);
		fclose(fw);
		fclose(fr);
	}
	sprintf(title, "[×ª¼Ä] %.70s", oldtitle);
	if (!strpbrk(address, "@.")) {
		return_no = mail_file(fname, lookupuser.userid, title);
		if (return_no != -1)
			return_no = 0;
	} else {
#ifdef SENDMAIL_MIME_AUTOCONVERT
		filter_ansi = askyn("¹ıÂË ANSI ¿ØÖÆÂë", NA, NA);
		if (askyn("ÒÔ MIME ¸ñÊ½ËÍĞÅ", NA, NA) == YEA)
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
	strcpy(rst->title, "ÎŞÌâ");
	strcpy(rst->owner, "XJTU-XANET");
	if ((fp = fopen(path, "r")) == NULL)
		return;
	buf1[0] = 0;
	while (fgets(buf2, sizeof (buf2), fp) != NULL) {
		if ((strncmp(buf1, "¼ÄĞÅÈË: ", 8)
		     && strncmp(buf1, "·¢ĞÅÈË: ", 8))
		    || (strncmp(buf2, "±ê  Ìâ: ", 8)
			&& strncmp(buf2, "±ê¡¡Ìâ: ", 8))) {
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
	getdata(1, 0, "Òª¼ğÀ¬»øÂğ?(Yes, or No) [N]: ", genbuf, 2, DOECHO, YEA);
	if (genbuf[0] != 'Y' && genbuf[0] != 'y') {
		move(2, 0);
		prints("Å¶~~~~~,ÅÂÔà°¡...\n");
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
		setmailfile(genbuf, currentuser.userid, DOT_DIR);
		if (append_record(genbuf, &rstmsg, sizeof (struct fileheader)) == -1)
			break;
		count++;
	}
	closedir(dirp);
	if (count) {
		prints("Å¶,¼ğÁË %d ¼şÀ¬»ø»ØÀ´.\n", count);
	} else
		prints("°¥Ñ½,ÕâÄêÍ·À¬»øÒ²ÄÑÕÒ...\n");
	pressreturn();
	return FULLUPDATE;
}

static int
m_cancel_1(struct fileheader *fh, char *receiver)
{
	char buf[256];
	FILE *fp;
	time_t now;
	if (strncmp(currentuser.userid, fh2owner(fh), IDLEN + 1)
	    || (fh->accessed & FH_READ))
		return 0;
	snprintf(buf, sizeof (buf), "ÄúÒª³·»ØÓÊ¼ş<%.50s>Âğ?", fh->title);
	if (YEA == askyn(buf, NA, NA)) {
		setmailfile(buf, receiver, fh2fname(fh));
		mail_file(buf, currentuser.userid, "[ÏµÍ³]³·»ØµÄÓÊ¼ş±¸·İ");
		fp = fopen(buf, "r+");
		if (NULL == fp) {
			prints("²»ÄÜ´ò¿ªÎÄ¼şĞ´,ÇëÁªÏµÏµÍ³Î¬»¤!");
			return 2;
		}
		keepoldheader(fp, SKIPHEADER);
		now = time(0);
		fprintf(fp, "±¾ÎÄÒÑ¾­ÓÚ %s ±» %s ³·»Ø\n", ytht_ctime(now),
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
		   || HAS_PERM(PERM_SPECIAL1)) ?
	    MAX_SYSOPMAIL_HOLD : (HAS_PERM(PERM_ARBITRATE)
				  || HAS_PERM(PERM_BOARDS)) ?
	    MAX_MAIL_HOLD * 8 : MAX_MAIL_HOLD;
	maxsize = maxsize * 10;
	return maxsize;*/
	maxsize= (HAS_PERM(PERM_SYSOP, currentuser))?MAX_SYSOPMAIL_HOLD:HAS_PERM(PERM_SPECIAL1, currentuser)?MAX_MAIL_HOLD*20:
		(HAS_PERM(PERM_BOARDS, currentuser))?MAX_MAIL_HOLD*8:MAX_MAIL_HOLD*3;
	maxsize=maxsize*10;
	//modified by wjbta@bmy ĞŞ¸ÄĞÅÏä×î´óÈİÁ¿¿ØÖÆ
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
			setmailfile(tmpmail, currentuser.userid,
				    fh2fname(&tmpfh));
			currmailsize += file_size(tmpmail);
		}
		close(fd);
	}
	sethomefile(buf, currentuser.userid, "msgindex");
	if (file_time(buf))
		currmsgsize = file_size(buf);
	sethomefile(buf, currentuser.userid, "msgindex2");
	if (file_time(buf))
		currmsgsize += file_size(buf);
	sethomefile(buf, currentuser.userid, "msgcontent");
	if (file_time(buf))
		currmsgsize += file_size(buf);
	currmsgsize = (currmailsize + currmsgsize) / 1024;
	return currmsgsize;
}

int
m_cancel(userid)
char userid[];
{
	char uident[STRLEN], buf[STRLEN];

	if (check_mail_perm()) {
		pressreturn();
		return 0;
	}
	if ((uinfo.mode != LUSERS && uinfo.mode != LAUSERS
	     && uinfo.mode != FRIEND && uinfo.mode != GMENU)
	    || userid == NULL) {
		move(1, 0);
		clrtoeol();
		modify_user_mode(SMAIL);
		usercomplete("³·»Ø¸øË­µÄĞÅ¼ş£º ", uident);
		if (uident[0] == '\0') {
			return FULLUPDATE;
		} else if (!strcmp(currentuser.userid, uident)) {
			prints("×Ô¼º¸ø×Ô¼ºĞ´ĞÅ¾Í¹»±äÌ¬ÁË,¾ÓÈ»»¹Òª×Ô¼º³·»Ø°¡?");
			pressreturn();
			return FULLUPDATE;
		}
	} else
		strcpy(uident, userid);
	clear();
	setmailfile(buf, uident, ".DIR");
	if (!new_apply_record
	    (buf, sizeof (struct fileheader), (void *) m_cancel_1, uident))
		prints("Ã»ÓĞÕÒµ½¿ÉÒÔ³·»ØµÄĞÅ¼ş\n");
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
        prints
             ("\n\033[1;37m ÄúµÄË½ÈËĞÅ¼ş×Ü´óĞ¡\033[1;31m¸ß´ï %d k\033[0m,\033[1;37m ÇëÉ¾³ı¹ıÆÚĞÅ¼ş,\033[1;32m±£³Ö%d k ÄÚ¡£\033[0m\n\n ",
              currsize, maxsize);
         prints("\033[1;37mµ±ĞÅ¼şºÍÕ¾ÄÚÏûÏ¢×Ü´óĞ¡\033[1;33m³¬¹ı %d k\033[0m \033[1;37mÊ±, Äã½«ÎŞ·¨Ê¹ÓÃ±¾Õ¾µÄËÍĞÅ¹¦ÄÜ¡£ \033[0m\n",
                maxsize + 20);
//		if (currsize > maxsize * 2) {
//			sprintf(genbuf, "Ë½ÈËĞÅ¼ş¹ıÁ¿: %d k", currsize);
//			securityreport(genbuf, genbuf);
//		}
	}
	if (currsize > maxsize + 20)
		return (1);
	else
		return (0);
}

 /*ARGSUSED*/ int
post_reply(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
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
	setbfile(quote_file, currboard, fh2fname(fileinfo));
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
	if (toupper(fileinfo->title[0]) != 'R'
	    || fileinfo->title[1] != 'e' || fileinfo->title[2] != ':')
		strcpy(title, "Re: ");
	else
		title[0] = '\0';
	strncat(title, fileinfo->title, STRLEN - 5);
/* edit, then send the mail */
	switch (do_send(uid, title)) {
	case -1:
		prints("ÏµÍ³ÎŞ·¨ËÍĞÅ\n");
		break;
	case -2:
		prints("ËÍĞÅ¶¯×÷ÒÑ¾­ÖĞÖ¹\n");
		break;
	case -3:
		prints("Ê¹ÓÃÕß '%s' ÎŞ·¨ÊÕĞÅ\n", uid);
		break;
	default:
		prints("ĞÅ¼şÒÑ³É¹¦µØ¼Ä¸øÔ­×÷Õß %s\n", uid);
	}
	pressreturn();
	return FULLUPDATE;
}

static int
check_mail_perm()
{
	if (HAS_PERM(PERM_DENYMAIL, currentuser)) {
		prints("Äú±»½ûÖ¹·¢ĞÅ");
		return -1;
	}
	return 0;
}

static int
show_user_notes()
{
	char buf[256];
	setuserfile(buf, "notes");
	if (dashf(buf)) {
		ansimore(buf, YEA);
		return FULLUPDATE;
	}
	clear();
	move(10, 15);
	prints("ÄúÉĞÎ´ÔÚ InfoEdit->WriteFile ±à¼­¸öÈË±¸ÍüÂ¼¡£\n");
	pressanykey();
	return FULLUPDATE;
}

static int
mailto(uentp)
struct userec *uentp;
{
	char filename[STRLEN];

//      sprintf(filename, "tmp/mailall.%s", currentuser.userid);
	sprintf(filename, "%s/mail/S/SYSOP/M.%d.A", MY_BBS_HOME, mailallmode);
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
		prints("SYSOP ĞÅÏä¹ÊÕÏ£¬ÇëÁªÏµÏµÍ³Î¬»¤ÈËÔ±¡£");
		pressreturn();
		return -1;
	}
	if (apply_record(PASSFILE, (void *) mailto, sizeof (struct userec)) ==
	    -1) {
		prints("No Users Exist");
		pressreturn();
		return -1;
	}
	mailallmode = 0;
	return 0;
}
