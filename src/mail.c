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
#include "bbstelnet.h"

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
	int i, offset;
	int numfiles;
	int accessed;
	extern char currmaildir[STRLEN];

	if (!HAS_PERM(PERM_BASIC)) {
		return 0;
	}
	offset = (int) &((struct fileheader *) 0)->accessed;

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
	lseek(fd, (st.st_size - (sizeof (struct fileheader) - offset)),
	      SEEK_SET);
	for (i = 0; i < numfiles && i < 10; i++) {
		read(fd, &accessed, sizeof (accessed));
		if (!(accessed & FH_READ)) {
			close(fd);
			return (ismail = 1);
		}
		lseek(fd, -sizeof (struct fileheader) - sizeof (accessed),
		      SEEK_CUR);
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
	int offset;
	int numfiles;
	int accessed;

	offset = (int) &((struct fileheader *) 0)->accessed;
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
/*���߲�ѯ����ֻҪ��ѯ���һ���Ƿ�Ϊ���ţ�����������Ҫ*/
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

	strcpy(title, "û����");
	modify_user_mode(SMAIL);
	clear();
	move(0, 0);
	sprintf(fname, "tmp/mailall.%s", currentuser.userid);
	prints("��Ҫ�ĸ����еģ�\n");
	prints("(0) ����\n");
	strcpy(doc[0], "(1) ��δͨ�����ȷ�ϵ�ʹ����");
	strcpy(doc[1], "(2) ����ͨ�����ȷ�ϵ�ʹ����");
	strcpy(doc[2], "(3) ���еİ���");
	strcpy(doc[3], "(4) ��վ������");
	strcpy(doc[4], "(5) ���б���֯��Ա");
	strcpy(doc[5], "(6) ����SYSOP");
	for (i = 0; i < 6; i++)
		prints("%s\n", doc[i]);
	getdata(9, 0, "������ģʽ (0~6)? [0]: ", ans, 2, DOECHO, YEA);
	if (ans[0] - '0' < 1 || ans[0] - '0' > 6) {
		return NA;
	}
	getdata(10, 0, "��������վʱ������(��Сʱ)[0]: ", str, 5, DOECHO, YEA);
	hour = atoi(str);
	sprintf(buf, "�Ƿ�ȷ���ĸ�%s ������վʱ�䲻С��%dСʱ?",
		doc[ans[0] - '0' - 1], hour);
	move(11, 0);
	if (askyn(buf, NA, NA) == NA)
		return NA;
	save_in_mail = in_mail;
	in_mail = YEA;
	header.reply_mode = NA;
	strcpy(header.title, "û����");
	strcpy(header.ds, doc[ans[0] - '0' - 1]);
	header.postboard = NA;
	if (post_header(&header))
		sprintf(save_title, "[Type %c ����] %.60s", ans[0],
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
	    ("[5;1;32;44m���ڼļ��У����Ժ�.....                                                        [m");
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

	getdata(1, 0, "������E-mail��", receiver, 65, DOECHO, YEA);
	sprintf(genbuf, ".bbs@%s", email_domain());
	if (strstr(receiver, genbuf)
	    || strstr(receiver, ".bbs@localhost")) {
		move(3, 0);
		prints("վ���ż�, ���� (S)end ָ������\n");
		pressreturn();
	} else if (!invalidaddr(receiver)) {
		*quote_file = '\0';
		clear();
		do_send(receiver, NULL);
	} else {
		move(3, 0);
		prints("�����˲���ȷ, ������ѡȡָ��\n");
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
	strsncpy(newmessage.owner, currentuser.userid,
		 sizeof (newmessage.owner));

      edit_mail_file:
	if (title == NULL) {
		header.reply_mode = NA;
		strcpy(header.title, "û����");
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
		strsncpy(newmessage.title, header.title,
			 sizeof (newmessage.title));
		strsncpy(save_title, newmessage.title, sizeof (save_title));
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
		prints("�ż������ĸ� %s \n", uid);
		prints("����Ϊ�� %s \n", header.title);
		if (askyn("ȷ��Ҫ�ĳ���", YEA, NA) == NA) {
			prints("\n�ż���ȡ��...\n");
			res = -2;
		} else {
#ifdef SENDMAIL_MIME_AUTOCONVERT
			int ans;
			filter_ansi = 0;
			if (askyn("�Ƿ񱸷ݸ��Լ�", NA, NA) == YEA)
				mail_file(tmp_fname, currentuser.userid,
					  save_title2);
			ans = askyn("�� MIME ��ʽ����", NA, NA);
			prints("���Ժ�, �ż�������...\n");
			refresh();
			res =
			    bbs_sendmail(tmp_fname, header.title, uid, ans);
#else
			if (askyn("�Ƿ񱸷ݸ��Լ�", NA, NA) == YEA)
				mail_file(tmp_fname, currentuser.userid,
					  save_title2);
			prints("���Ժ�, �ż�������...\n");
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
		if (askyn("�Ƿ񱸷ݸ��Լ�", NA, NA) == YEA)
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
	// ��Զ���Ը� SYSOP ����
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
                } //add by wjbta@bmy  ���ż��������������������ʱ����ֹ����
	modify_user_mode(SMAIL);
	if (			/*(uinfo.mode != LUSERS && uinfo.mode != LAUSERS
				   && uinfo.mode != FRIEND && uinfo.mode != GMENU)
				   || */ userid == NULL) {
		move(1, 0);
		clrtoeol();
		usercomplete("�����ˣ� ", uident);
		if (uident[0] == '\0') {
			return FULLUPDATE;
		}
	} else
		strcpy(uident, userid);
	clear();
	*quote_file = '\0';
	switch (do_send(uident, NULL)) {
	case -1:
		prints("�����߲���ȷ\n");
		break;
	case -2:
		prints("ȡ��\n");
		break;
	case -3:
		prints("[%s] �޷�����\n", uident);
		break;
	default:
		prints("�ż��Ѽĳ�\n");
	}
	pressreturn();
	return FULLUPDATE;
}

int
M_send()
{
	if (!HAS_PERM(PERM_LOGINOK))
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
	prints("��ȡ %s ������ '%s' ?\n", fptr->owner, fptr->title);
	prints("(Yes, or No): ");
	getdata(1, 0, "(Y)��ȡ (N)���� (Q)�뿪 [Y]: ", genbuf, 3, DOECHO, YEA);
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
		prints("(R)����, (D)ɾ��, (G)���� ? [G]: ");
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
		prints("ɾ���ż� '%s' ", fptr->title);
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
	showtitle("�ʼ�ѡ��    ", MY_BBS_NAME);

	prints
	    ("�뿪[[1;32m��[m,[1;32me[m]  ѡ��[[1;32m��[m,[1;32m��[m]  �Ķ��ż�[[1;32m��[m,[1;32mRtn[m]  ����[[1;32mR[m]  ���ţ��������[[1;32md[m,[1;32mD[m]  ����[[1;32mh[m][m\n");
	prints(" [1;44m���    %-12s %6s  %-24s %d %s %d %s[m\n", "������",
	       "��  ��", "��  ��      ��ǰ��������", max_mail_size(),
	       "k,���ÿռ�", get_mail_size(), "k");
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
	strsncpy(b2, ent->owner, sizeof (b2));

	//add by gluon
	/* Added by deardragon 1999.11.15 ���ѻ��ż����� "����" ��� ('R') */
	if (ent->accessed & FH_REPLIED)	/* �ʼ��ѻظ� */
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
			" %s%3d\033[m %c%c %-12.12s %6.6s %c�� %s%.47s\033[m",
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
		prints("(R)����, (D)ɾ��, (G)����? [G]: ");
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
	strsncpy(uid, fh2owner(fileinfo), sizeof (uid));
	if (strchr(uid, '.')) {
		char filename[STRLEN];
		directfile(filename, direct, fh2fname(fileinfo));
		if (!getdocauthor(filename, uid, sizeof (uid))) {
			prints("�޷�Ͷ��\n");
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
		prints("�޷�Ͷ��\n");
		break;
	case -2:
		prints("ȡ������\n");
		break;
	case -3:
		prints("[%s] �޷�����\n", uid);
		break;
	default:
		//add by gluon
		/* Added bye deardragon 1999.11.15 ���ѻ��ż����� "����" ��� ('R') */
		if (ent >= 0) {
			fileinfo->accessed |= FH_REPLIED;
			substitute_record(direct, fileinfo, sizeof (*fileinfo),
					  ent);
		}
		/* Added End. */
		//end
		prints("�ż��Ѽĳ�\n");
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
	prints("ɾ���ż� [%s] ", fileinfo->title);
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
	if (!HAS_PERM(PERM_FORWARD)) {
		return DONOTHING;
	}
	directfile(buf, direct, fh2fname(fileinfo));
	switch (doforward(buf, fileinfo->title, 0)) {
	case 0:
		prints("����ת�����!\n");
		break;
	case -1:
		prints("ת��ʧ��: ϵͳ��������.\n");
		break;
	case -2:
		prints("ת��ʧ��: ����ȷ�����ŵ�ַ.\n");
		break;
	default:
		prints("ȡ��ת��...\n");
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
	if (!HAS_PERM(PERM_FORWARD)) {
		return DONOTHING;
	}
	directfile(buf, direct, fh2fname(fileinfo));
	switch (doforward(buf, fileinfo->title, 1)) {
	case 0:
		prints("����ת�����!\n");
		break;
	case -1:
		prints("ת��ʧ��: ϵͳ��������.\n");
		break;
	case -2:
		prints("ת��ʧ��: ����ȷ�����ŵ�ַ.\n");
		break;
	default:
		prints("ȡ��ת��...\n");
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

// �ȼ���
typedef int(*equalor)(const struct fileheader*, void *);
char *strstr2(char *s, char *s2);
// ��ǵ��ʼ�
static int ismark(const struct fileheader *mail, void *ext)
{
	if (mail->accessed & FH_MARKED)
		return 1;
    return 0;
}
// ���������ʼ�
static int isattach(const struct fileheader *mail, void *ext)
{
	if (mail->accessed & FH_ATTACHED)
		return 1;
    return 0;
}
// �ض�������ʼ�
static int istitle(const struct fileheader *mail, void *ext)
{
	if (strstr2(mail->title, (char*)ext) != NULL)
		return 1;
    return 0;
}
// �ض������˵��ʼ�
static int issender(const struct fileheader *mail, void *ext)
{
	if (strncasecmp(mail->owner, (char*)ext, sizeof(mail->owner)+1) == 0) // �����ִ�Сд
		return 1;
    return 0;
}

// ��������������������µ�.DIR�ļ�����Ϊ���������
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

// �����������ļ�(.DIR)��·������ȡ����
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

// �����ѯ���ܣ����������ctrl+g
// interma@bmy, 2006-11-23
static int
mail_query(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	//power_action(currmaildir, 1, -1, "��Ǻ� m", 9);
	char ans[3];
	int type;
	char whattosearch[31];

	ans[0] = '\0';
	getdata(t_lines - 1 , 0, "��ѡ��:0)ȡ�� 1)��m�ʼ� 2)���� 3)����ؼ��� 4)ͬ���� [0]:",
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
		getdata(t_lines - 1, 0, "���������ؼ���:", whattosearch, 31, DOECHO,NA);
		if (whattosearch[0] == '\0')
			return PARTUPDATE;
		break;
	case 4:
		getdata(t_lines - 1, 0, "����������ID:",whattosearch, 31, DOECHO, NA);
		if (whattosearch[0] == '\0')
			return PARTUPDATE;
		break;
	}

	//char tempdir[L_tmpnam]; 
	//tmpnam(tempdir);
	char maildir[256];
	strncpy(maildir, currmaildir, 256);
	char *tempdir = tempnam(truncateDIR(maildir), ".DIR");
	do_search_mailbox(type, whattosearch, tempdir);
	// m_init();

	i_read(RMAIL, tempdir, mailtitle, (void *) maildoent, query_comms, sizeof (struct fileheader));

	unlink(tempdir);
	free(tempdir);
	i_read(RMAIL, currmaildir, mailtitle, (void *) maildoent, mail_comms, sizeof (struct fileheader));
	return DOQUIT; // �������DOQUIT���Ǿ�Ҫ�ఴ����"����"�����˳����Ǻ�
}

// �����ѯ�����п���ʹ�õ�����
struct one_key query_comms[] = {
	//{'d', mail_del, "ɾ���ż�"},
	//{'D', mail_del_range, "����ɾ��"},
	{Ctrl('P'), M_send, "�����ż�"},
	{'E', edit_post, "�༭�ż�"},
	{'r', mail_read, "�Ķ��ż�"},
	{'R', mail_reply, "�ظ��ż�"},
	//{'m', mail_mark, "mark�ż�"},
	{'i', Save_post, "�����´����ݴ浵"},
	{'I', Import_post, "���ż����뾫����"},
	{'x', into_announce, "���뾫����"},
	{KEY_TAB, show_user_notes, "�鿴�û�����¼"},
#ifdef INTERNET_EMAIL
	{'F', mail_forward, "ת���ż�"},
	{'U', mail_u_forward, "uuencode ת��"},
#endif
	{'a', auth_search_down, "�����������"},
	{'A', auth_search_up, "��ǰ��������"},
	{'/', t_search_down, "�����������"},
	{'?', t_search_up, "��ǰ��������"},
	{'\'', post_search_down, "�����������"},
	{'\"', post_search_up, "��ǰ��������"},
	{']', thread_down, "���ͬ����"},
	{'[', thread_up, "��ǰͬ����"},
	{Ctrl('A'), show_author, "���߼��"},
	{'\\', SR_last, "���һƪͬ��������"},
	{'=', SR_first, "��һƪͬ��������"},
	{'L', show_allmsgs, "�鿴��Ϣ"},
	{Ctrl('C'), do_cross, "ת������"},
	{'n', SR_first_new, "����δ���ĵ�һƪ"},
	{'p', SR_read, "��ͬ������Ķ�"},
	{Ctrl('U'), SR_author, "��ͬ�����Ķ�"},
	{'h', mailreadhelp, "�鿴����"},
	{'!', Q_Goodbye, "������վ"},
	{'S', s_msg, "����ѶϢ"},
	{'c', t_friends, "�鿴����"},
	{'C', friend_author, "��������Ϊ����"},
	{Ctrl('E'), mail_rjunk, "��������"},
	//{Ctrl('G'), mail_query, "��ѯģʽ"},
	{'\0', NULL, ""}
};

// �����Ķ����棨����ģʽ���п���ʹ�õ�����
struct one_key mail_comms[] = {
	{'d', mail_del, "ɾ���ż�"},
	{'D', mail_del_range, "����ɾ��"},
	{Ctrl('P'), M_send, "�����ż�"},
	{'E', edit_post, "�༭�ż�"},
	{'T', edit_title, "�޸ı���"},
	{'r', mail_read, "�Ķ��ż�"},
	{'R', mail_reply, "�ظ��ż�"},
	{'m', mail_mark, "mark�ż�"},
	{'i', Save_post, "�����´����ݴ浵"},
	{'I', Import_post, "���ż����뾫����"},
	{'x', into_announce, "���뾫����"},
	{KEY_TAB, show_user_notes, "�鿴�û�����¼"},
#ifdef INTERNET_EMAIL
	{'F', mail_forward, "ת���ż�"},
	{'U', mail_u_forward, "uuencode ת��"},
#endif
	{'a', auth_search_down, "�����������"},
	{'A', auth_search_up, "��ǰ��������"},
	{'/', t_search_down, "�����������"},
	{'?', t_search_up, "��ǰ��������"},
	{'\'', post_search_down, "�����������"},
	{'\"', post_search_up, "��ǰ��������"},
	{']', thread_down, "���ͬ����"},
	{'[', thread_up, "��ǰͬ����"},
	{Ctrl('A'), show_author, "���߼��"},
	{'\\', SR_last, "���һƪͬ��������"},
	{'=', SR_first, "��һƪͬ��������"},
	{'L', show_allmsgs, "�鿴��Ϣ"},
	{Ctrl('C'), do_cross, "ת������"},
	{'n', SR_first_new, "����δ���ĵ�һƪ"},
	{'p', SR_read, "��ͬ������Ķ�"},
	{Ctrl('U'), SR_author, "��ͬ�����Ķ�"},
	{'h', mailreadhelp, "�鿴����"},
	{'!', Q_Goodbye, "������վ"},
	{'S', s_msg, "����ѶϢ"},
	{'c', t_friends, "�鿴����"},
	{'C', friend_author, "��������Ϊ����"},
	{Ctrl('E'), mail_rjunk, "��������"},
	{Ctrl('G'), mail_query, "��ѯģʽ"},
	{'\0', NULL, ""}
};

// �����Ķ����棨����ģʽ���п���ʹ�õ�����
// Ĭ��ֵ
const struct one_key mail_default_comms[] = {
	{'d', mail_del, "ɾ���ż�"},
	{'D', mail_del_range, "����ɾ��"},
	{Ctrl('P'), M_send, "�����ż�"},
	{'E', edit_post, "�༭�ż�"},
	{'T', edit_title, "�޸ı���"},
	{'r', mail_read, "�Ķ��ż�"},
	{'R', mail_reply, "�ظ��ż�"},
	{'m', mail_mark, "mark�ż�"},
	{'i', Save_post, "�����´����ݴ浵"},
	{'I', Import_post, "���ż����뾫����"},
	{'x', into_announce, "���뾫����"},
	{KEY_TAB, show_user_notes, "�鿴�û�����¼"},
#ifdef INTERNET_EMAIL
	{'F', mail_forward, "ת���ż�"},
	{'U', mail_u_forward, "uuencode ת��"},
#endif
	{'a', auth_search_down, "�����������"},
	{'A', auth_search_up, "��ǰ��������"},
	{'/', t_search_down, "�����������"},
	{'?', t_search_up, "��ǰ��������"},
	{'\'', post_search_down, "�����������"},
	{'\"', post_search_up, "��ǰ��������"},
	{']', thread_down, "���ͬ����"},
	{'[', thread_up, "��ǰͬ����"},
	{Ctrl('A'), show_author, "���߼��"},
	{'\\', SR_last, "���һƪͬ��������"},
	{'=', SR_first, "��һƪͬ��������"},
	{'L', show_allmsgs, "�鿴��Ϣ"},
	{Ctrl('C'), do_cross, "ת������"},
	{'n', SR_first_new, "����δ���ĵ�һƪ"},
	{'p', SR_read, "��ͬ������Ķ�"},
	{Ctrl('U'), SR_author, "��ͬ�����Ķ�"},
	{'h', mailreadhelp, "�鿴����"},
	{'!', Q_Goodbye, "������վ"},
	{'S', s_msg, "����ѶϢ"},
	{'c', t_friends, "�鿴����"},
	{'C', friend_author, "��������Ϊ����"},
	{Ctrl('E'), mail_rjunk, "��������"},
	{Ctrl('G'), mail_query, "��ѯģʽ"},
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
	int len;

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

	fprintf(fout, "X-Disclaimer: %s �Ա�������ˡ������\n", MY_BBS_NAME);
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
			prints("Ŀǰ���Ƽ��Ÿ� [1m%d[m ��", maxrecp);
		}
		if (keepgoing) {
			tmp[0]='a';
			tmp[1]=0;
		}
		else { 
		getdata(0, 0,
			"(A)���� (D)ɾ�� (I)������� (C)���Ŀǰ���� (E)���� (S)�ĳ�? [S]�� ",
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
				    ("����������ʹ���ߴ���(ֻ�� ENTER ��������): ",
				     uident);
			else
				namecomplete
				    ("����������ʹ���ߴ���(ֻ�� ENTER ��������): ",
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
				prints("���ʹ���ߴ����Ǵ����.\n");
			}
		}
		switch (tmp[0]) {
		case 'A':
		case 'a':
			if (!(lookupuser.userlevel & PERM_READMAIL)) {
				move(2, 0);
				prints("�޷����Ÿ�: [1m%s[m\n",
				       lookupuser.userid);
				break;
			} else if (seek_in_file(maillists, uident)) {
				move(2, 0);
				prints("�Ѿ���Ϊ�ռ���֮һ \n");
				break;
			}
			addtofile(maillists, uident);
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
					del_from_file(maillists, uident);
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
				    ("(A)ȫ������ (Y)���� (N)������ (Q)����? [Y]:");
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
						    ("���ʹ���ߴ����Ǵ����.\n");
						i--;
						continue;
					} else
					    if (!
						(lookupuser.userlevel &
						 PERM_READMAIL)) {
						move(4, 0);
						prints
						    ("�޷����Ÿ�: [1m%s[m\n",
						     lookupuser.userid);
						i--;
						continue;
					} else
					    if (seek_in_file(maillists, uident))
					{
						i--;
						continue;
					}
					addtofile(maillists, uident);
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
			prints("�ż�Ŀ¼����\n");
			break;
		case -2:
			prints("ȡ��\n");
			break;
		default:
			prints("�ż��Ѽĳ�\n");
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
	strcpy(header.title, "û����");
	strcpy(header.ds, "���Ÿ�һȺ��");
	header.postboard = NA;
	sprintf(tmpfile, "tmp/gsend.%s.%05d", currentuser.userid, uinfo.pid);
	if (post_header(&header)) {
		sprintf(save_title, "[Ⱥ���ż�] %.60s", header.title);
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
	prints("[5;1;32m���ڼļ��У����Ժ�...[m");
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
	strcpy(header.title, "û����");
	strcpy(header.ds, "���Ÿ�һȺ��");
	header.postboard = NA;
	sprintf(tmpfile, "tmp/gsend.%s.%05d", currentuser.userid, uinfo.pid);
	if (post_header(&header)) {
		sprintf(save_title, "[Ⱥ���ż�] %.60s", header.title);
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
	prints("[5;1;32m���ڼļ��У����Ժ�...[m");
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
	strsncpy(newmessage.owner, currentuser.userid,
		 sizeof (newmessage.owner));
	strsncpy(newmessage.title, title, sizeof (newmessage.title));
	strsncpy(save_title, newmessage.title, sizeof (save_title));

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
	strsncpy(newmessage.owner, currentuser.userid,
		 sizeof (newmessage.owner));
	strsncpy(newmessage.title, title, sizeof (newmessage.title));
	strsncpy(save_title, newmessage.title, sizeof (save_title));

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
	prints("���Ÿ����������е��ˣ�Ŀǰ��վ���ƽ����Լĸ� [1m%d[m λ��\n",
	       maxrecp);
	if (uinfo.fnum <= 0) {
		prints("�㲢û���趨���ѡ�\n");
		pressanykey();
		clear();
		return 0;
	} else {
		prints("�������£�\n");
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
		prints("�ż�Ŀ¼����\n");
		break;
	case -2:
		prints("�ż�ȡ��\n");
		break;
	default:
		prints("�ż��Ѽĳ�\n");
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
		prints("�ż�Ŀ¼����\n");
		break;
	case -2:
		prints("�ż�ȡ��\n");
		break;
	default:
		prints("�ż��Ѽĳ�\n");
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
		prints("�ż�Ŀ¼����\n");
		break;
	case -2:
		prints("�ż�ȡ��\n");
		break;
	default:
		prints("�ż��Ѽĳ�\n");
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
	if (HAS_PERM(PERM_SETADDR)) {
		prints
		    ("��ֱ�Ӱ� Enter ������������ʾ�ĵ�ַ, ��������������ַ\n");
		prints("���ż�ת�ĸ� [%s]\n", address);
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
	sprintf(genbuf, "ȷ�������¼ĸ� %s ��", address);
	if (askyn(genbuf, YEA, NA) == 0)
		return 1;
	if (invalidaddr(address))
		if (!getuser(address) || check_mail_perm())
			return -2;
	sprintf(tmpfname, "tmp/forward.%s.%05d", currentuser.userid, uinfo.pid);
	copyfile(filepath, tmpfname);
	if (askyn("�Ƿ��޸���������", NA, NA) == 1) {
		if (vedit(tmpfname, NA, NA) == -1) {
			if (askyn("�Ƿ�ĳ�δ�޸ĵ�����", YEA, NA) == 0) {
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
	prints("ת���ż��� %s, ���Ժ�....\n", address);
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
	sprintf(title, "[ת��] %.70s", oldtitle);
	if (!strpbrk(address, "@.")) {
		return_no = mail_file(fname, lookupuser.userid, title);
		if (return_no != -1)
			return_no = 0;
	} else {
#ifdef SENDMAIL_MIME_AUTOCONVERT
		filter_ansi = askyn("���� ANSI ������", NA, NA);
		if (askyn("�� MIME ��ʽ����", NA, NA) == YEA)
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
	strcpy(rst->title, "����");
	strcpy(rst->owner, "XJTU-XANET");
	if ((fp = fopen(path, "r")) == NULL)
		return;
	buf1[0] = 0;
	while (fgets(buf2, sizeof (buf2), fp) != NULL) {
		if ((strncmp(buf1, "������: ", 8)
		     && strncmp(buf1, "������: ", 8))
		    || (strncmp(buf2, "��  ��: ", 8)
			&& strncmp(buf2, "�ꡡ��: ", 8))) {
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
		strsncpy(rst->title, buf2 + 8, sizeof (rst->title));
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
	char buf[256], rpath[256], npath[256];
	struct fileheader rstmsg;

	clear();
	getdata(1, 0, "Ҫ��������?(Yes, or No) [N]: ", genbuf, 2, DOECHO, YEA);
	if (genbuf[0] != 'Y' && genbuf[0] != 'y') {
		move(2, 0);
		prints("Ŷ~~~~~,���డ...\n");
		pressreturn();
		clear();
		return FULLUPDATE;
	}

	len =
	    sprintf(buf, "%c/%s/", mytoupper(currentuser.userid[0]),
		    currentuser.userid);
	normalize(buf);
	dirp = opendir(MY_BBS_HOME "/mail/.junk");
	if (dirp == NULL)
		return -2;
	count = 0;
	while ((direntp = readdir(dirp)) != NULL) {
		if (strncmp(buf, direntp->d_name, len)
		    || strncmp(direntp->d_name + len, "M.", 2))
			continue;
		sprintf(rpath, MY_BBS_HOME "/mail/.junk/%s", direntp->d_name);
		sprintf(npath, "mail/%c/%s/%s",
			mytoupper(currentuser.userid[0]), currentuser.userid,
			direntp->d_name + len);
		rename(rpath, npath);
		bzero(&rstmsg, sizeof(struct fileheader));
		getmailinfo(npath, &rstmsg);
		rstmsg.filetime = atoi(direntp->d_name + 2 + len);
		rstmsg.thread = rstmsg.filetime;
		setmailfile(genbuf, currentuser.userid, DOT_DIR);
		if (append_record(genbuf, &rstmsg, sizeof (struct fileheader))
		    == -1)
			break;
		count++;
	}
	closedir(dirp);
	if (count) {
		prints("Ŷ,���� %d ����������.\n", count);
	} else
		prints("��ѽ,����ͷ����Ҳ����...\n");
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
	snprintf(buf, sizeof (buf), "��Ҫ�����ʼ�<%.50s>��?", fh->title);
	if (YEA == askyn(buf, NA, NA)) {
		setmailfile(buf, receiver, fh2fname(fh));
		mail_file(buf, currentuser.userid, "[ϵͳ]���ص��ʼ�����");
		fp = fopen(buf, "r+");
		if (NULL == fp) {
			prints("���ܴ��ļ�д,����ϵϵͳά��!");
			return 2;
		}
		keepoldheader(fp, SKIPHEADER);
		now = time(0);
		fprintf(fp, "�����Ѿ��� %s �� %s ����\n", Ctime(now),
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
	maxsize= (HAS_PERM(PERM_SYSOP))?MAX_SYSOPMAIL_HOLD:HAS_PERM(PERM_SPECIAL1)?MAX_MAIL_HOLD*20:
		(HAS_PERM(PERM_BOARDS))?MAX_MAIL_HOLD*8:MAX_MAIL_HOLD*3;
	maxsize=maxsize*10;
	//modified by wjbta@bmy �޸����������������
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
		usercomplete("���ظ�˭���ż��� ", uident);
		if (uident[0] == '\0') {
			return FULLUPDATE;
		} else if (!strcmp(currentuser.userid, uident)) {
			prints("�Լ����Լ�д�ž͹���̬��,��Ȼ��Ҫ�Լ����ذ�?");
			pressreturn();
			return FULLUPDATE;
		}
	} else
		strcpy(uident, userid);
	clear();
	setmailfile(buf, uident, ".DIR");
	if (!new_apply_record
	    (buf, sizeof (struct fileheader), (void *) m_cancel_1, uident))
		prints("û���ҵ����Գ��ص��ż�\n");
	pressreturn();
	return FULLUPDATE;
}

static int
check_maxmail()
{
	int currsize, maxsize;
	currsize = 0;
	if(HAS_PERM(PERM_SYSOP|PERM_OBOARDS))
                return 0;//add by bjgyt
	maxsize = max_mail_size();
	currsize = get_mail_size();
	if (currsize > maxsize+20) {
        clear();
        move(2,2);
        prints
             ("\n\033[1;37m ����˽���ż��ܴ�С\033[1;31m�ߴ� %d k\033[0m,\033[1;37m ��ɾ�������ż�,\033[1;32m����%d k �ڡ�\033[0m\n\n ",
              currsize, maxsize);
         prints("\033[1;37m���ż���վ����Ϣ�ܴ�С\033[1;33m���� %d k\033[0m \033[1;37mʱ, �㽫�޷�ʹ�ñ�վ�����Ź��ܡ� \033[0m\n",
                maxsize + 20);
//		if (currsize > maxsize * 2) {
//			sprintf(genbuf, "˽���ż�����: %d k", currsize);
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
		strsncpy(uid, quote_user, sizeof (uid));
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
		prints("ϵͳ�޷�����\n");
		break;
	case -2:
		prints("���Ŷ����Ѿ���ֹ\n");
		break;
	case -3:
		prints("ʹ���� '%s' �޷�����\n", uid);
		break;
	default:
		prints("�ż��ѳɹ��ؼĸ�ԭ���� %s\n", uid);
	}
	pressreturn();
	return FULLUPDATE;
}

static int
check_mail_perm()
{
	if (HAS_PERM(PERM_DENYMAIL)) {
		prints("������ֹ����");
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
	prints("����δ�� InfoEdit->WriteFile �༭���˱���¼��\n");
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

int
mailtoall(mode, hour)
int mode, hour;
{
	char filename[STRLEN];
	sprintf(filename, "tmp/mailall.%s", currentuser.userid);
	mailmode = mode;
	mailhour = hour;
	mailallmode = mail_file(filename, "SYSOP", save_title);
	if (mailallmode == -1) {
		prints("SYSOP ������ϣ�����ϵϵͳά����Ա��");
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
