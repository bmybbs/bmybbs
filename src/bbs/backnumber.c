//writen by ecnegrevid, 2001.7
#include "bbs.h"
#include "smth_screen.h"
#include "main.h"
#include "xyz.h"
#include "stuff.h"
#include "more.h"
#include "read.h"
#include "bbsinc.h"
#include "io.h"
#include "term.h"
#include "sendmsg.h"
#include "help.h"
#include "maintain.h"
#include "list.h"
#include "one_key.h"
#include "bbs_global_vars.h"

extern char quote_file[], quote_user[];
char currbacknumberdir[STRLEN * 2];
static int backnumbertitle(void);
static char *backnumberdoent(int num, struct fileheader *ent, char buf[512]);
static int backnumber_read(int ent, struct fileheader *fileinfo, char *direct);
static int backnumber_hide(int ent, struct fileheader *fileinfo, char *direct);
static int selectbacknumbertitle(void);
static char *selectbacknumberdoent(int num, struct bknheader *ent,
				   char buf[512]);
static int delete_backnumber(int ent, struct bknheader *bkninfo, char *direct);
static int change_title(int ent, struct bknheader *bkninfo, char *direct);
int do_intobacknumber(char *filename, time_t t);

void
setbacknumberfile(char *path, char *filename)
{
	strcpy(path, currbacknumberdir);
	*(strrchr(path, '/') + 1) = 0;
	strcat(path, filename);
}

static int
backnumbertitle()
{

	showtitle("�Ķ�����", MY_BBS_NAME);
	prints
	    ("�뿪[[1;32m��[m,[1;32me[m]  ѡ��[[1;32m��[m,[1;32m��[m]  �Ķ�"
	     "[[1;32m��[m,[1;32mRtn[m] ����[[1;32mh[m][m\n");
	prints("[1;44m���   %-12s %6s  %-50s[m\n", "������", "����", "����");
	clrtobot();
	return 0;
}

static char *
backnumberdoent(num, ent, buf)
int num;
struct fileheader *ent;
char buf[512];
{
	char b2[512];
	time_t filetime;
	char *date;
	char *t;
	char type, attached;
	extern char ReadPost[];
	extern char ReplyPost[];
	char c1[8];
	char c2[8];
	int same = NA;

	filetime = ent->filetime;
	if (filetime > 740000000) {
		date = ctime(&filetime) + 4;
	} else {
		date = "";
	}

	type = (ent->accessed & FH_MARKED) ? 'm' : ' ';
	attached = (ent->accessed & FH_ATTACHED) ? '@' : ' ';
	type = (ent->accessed & FH_HIDE) ? 'd' : type;
	strcpy(c1, "[1;36m");
	strcpy(c2, "[1;33m");
	if (!strcmp(ReadPost, ent->title) || !strcmp(ReplyPost, ent->title))
		same = YEA;
	strncpy(b2, ent->owner, STRLEN);
	if ((t = strchr(b2, ' ')) != NULL)
		*t = '\0';

	if ((ent->accessed & FH_HIDE) && !IScurrBM) {
		sprintf(buf, " %s%3d[m %c %-12.12s %6.6s %c%s%.50s[m",
			same ? c1 : "", num, ' ', "", "", ' ', same ? c1 : "",
			"-�����ѱ�ɾ��-");
	} else if (!strncmp("Re:", ent->title, 3)) {
		sprintf(buf, " %s%3d[m %c %-12.12s %6.6s %c%s%.50s[m",
			same ? c1 : "", num, type, b2, date, attached,
			same ? c1 : "", ent->title);
	} else {
		sprintf(buf, " %s%3d[m %c %-12.12s %6.6s %c%s�� %.47s[m",
			same ? c2 : "", num, type, b2, date, attached,
			same ? c2 : "", ent->title);
	}
	return buf;
}

static int
backnumber_read(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char notgenbuf[128];
	int ch;

	clear();
	setqtitle(fileinfo->title);
	directfile(notgenbuf, direct, fh2fname(fileinfo));
	if ((fileinfo->accessed & FH_HIDE) && !IScurrBM) {
		move(10, 30);
		prints("�Բ��𣬱��ı����ݲ��ɶ���");
		pressanykey();
		return FULLUPDATE;
	} else
		ch = ansimore(notgenbuf, NA);
	move(t_lines - 1, 0);
	prints("\033[1;44;31m[�Ķ�����] \033[33m���� Q,������һ�� ��,l��"
	       "��һ�� n, <Space>,<Enter>,���������Ķ� x p \033[m");
	//usleep(300000l);
	if (!
	    (ch == KEY_RIGHT || ch == KEY_UP || ch == KEY_PGUP
	     || ch == KEY_DOWN) && (ch <= 0 || strchr("RrEexp", ch) == NULL))
		ch = egetch();
	switch (ch) {
	case 'Q':
	case 'q':
	case KEY_LEFT:
		break;
	case 'j':
	case KEY_RIGHT:
		if (DEFINE(DEF_THESIS, currentuser)) {
			sread(0, 0, ent, 0, fileinfo);
			break;
		} else {
			return READ_NEXT;
		}
	case KEY_DOWN:
	case KEY_PGDN:
	case 'n':
	case ' ':
		return READ_NEXT;
	case KEY_UP:
	case KEY_PGUP:
	case 'l':
		return READ_PREV;
	case 'x':
		sread(0, 0, ent, 0, fileinfo);
		break;
	case 'p':		/*Add by SmallPig */
		sread(4, 0, ent, 0, fileinfo);
		break;
	}
	return FULLUPDATE;
}

static int
backnumber_hide(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	if (!IScurrBM)
		return DONOTHING;
	if (fileinfo->accessed & FH_HIDE)
		fileinfo->accessed &= ~FH_HIDE;
	else
		fileinfo->accessed |= FH_HIDE;
	substitute_record(currbacknumberdir, fileinfo, sizeof (*fileinfo), ent);
	return (PARTUPDATE);
}

static const struct one_key backnumber_comms[] = {
	{'r', backnumber_read, "�Ķ��ż�"},
	{'i', Save_post, "�����´����ݴ浵"},
	{'I', Import_post, "���ż����뾫����"},
	{'x', into_announce, "���뾫����"},
#ifdef INTERNET_EMAIL
	{'F', forward_post, "ת���ż�"},
	{'U', forward_u_post, "uuencode ת��"},
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
	{'h', backnumberhelp, "�鿴����"},
	{'!', Q_Goodbye, "������վ"},
	{'S', s_msg, "����ѶϢ"},
	{'c', t_friends, "�鿴����"},
	{'d', backnumber_hide, "�����ż�"},
	{'C', friend_author, "��������Ϊ����"},
	{'\0', NULL, ""}
};

static int readbacknumber(int ent, struct bknheader *bkninfo, char *direct) {
	char buf[MAXPATHLEN], *t;
	strcpy(buf, direct);
	if ((t = strrchr(buf, '/')) != NULL)
		*t = '\0';
	sprintf(currbacknumberdir, "%s/%s/%s", buf, bknh2bknname(bkninfo),
		DOT_DIR);
	i_read(BACKNUMBER, currbacknumberdir, backnumbertitle,
	       (void *) backnumberdoent, backnumber_comms,
	       sizeof (struct fileheader));
	return 999;
}

static int
selectbacknumbertitle()
{

	char buf[STRLEN];
	sprintf(buf, "ѡ�����");
	showtitle(buf, MY_BBS_NAME);
	prints
	    ("�뿪[[1;32m��[m,[1;32me[m]  ѡ��[[1;32m��[m,[1;32m��[m]  �Ķ�[[1;32m��[m,[1;32mRtn[m]  �����¹���[[1;32m^P[m]  ����[[1;32mh[m][m\n");
	prints("[1;44m��� %-12s %6s  %-50s[m\n", "����", "��  ��", "��  ��");
	clrtobot();
	return 0;
}

static char *
selectbacknumberdoent(num, ent, buf)
int num;
struct bknheader *ent;
char buf[512];
{
	time_t filetime;
	char *date;

	filetime = ent->filetime;
	if (filetime > 740000000) {
		date = ctime(&filetime) + 4;
	} else {
		date = "";
	}

	sprintf(buf, " %3d[m %-12.12s %6.6s  �� %.47s[m",
		num, ent->boardname, date, ent->title);
	return buf;
}

int
new_backnumber()
{
	char backnumberboarddir[MAXPATHLEN], dirname[MAXPATHLEN], dpath[MAXPATHLEN];
	char content[1024];
	int now;
	struct bknheader bn;
	int count;
	if (!IScurrBM)
//by bjgyt	if (!HAS_PERM(PERM_OBOARDS) && !HAS_PERM(PERM_SYSOP))
		return DONOTHING;
	bzero(&bn, sizeof (bn));
	getdata(t_lines - 1, 0, "�����������: ", bn.title, 50, DOECHO, YEA);
	if (bn.title[0] == '\0')
		return FULLUPDATE;
	now = time(NULL);
	sprintf(dirname, "B.%d", now);
	sprintf(dpath, "boards/.backnumbers/%s/%s", currboard, dirname);
	count = 0;
	while (mkdir(dpath, 0770) == -1) {
		now++;
		sprintf(dirname, "B.%d", now);
		sprintf(dpath, "boards/.backnumbers/%s/%s", currboard, dirname);
		if (count++ > MAX_POSTRETRY) {
			return FULLUPDATE;
		}
	}
	bn.filetime = now;
	ytht_strsncpy(bn.boardname, currboard, sizeof(bn.boardname));
	sprintf(backnumberboarddir, "boards/.backnumbers/%s/%s", currboard,
		DOT_DIR);
	append_record(backnumberboarddir, &bn, sizeof (bn));
	sprintf(genbuf, "creat backnumber, %s", currboard);
	sprintf(content, "%s ���� %s ����� %s", currentuser.userid, currboard,
		bn.title);
	securityreport(genbuf, content);
	return FULLUPDATE;
}

static int
delete_backnumber(ent, bkninfo, direct)
int ent;
struct bknheader *bkninfo;
char *direct;
{
	char dpath[MAXPATHLEN];
	if (!IScurrBM)
//by bjgyt        if (!HAS_PERM(PERM_OBOARDS) && !HAS_PERM(PERM_SYSOP))
                return DONOTHING;
	if (askyn("ȷ��ɾ��?", NA, NA) == NA)
		return FULLUPDATE;
	sprintf(dpath, "boards/.backnumbers/%s/%s", currboard,
		bknh2bknname(bkninfo));
	rmdir(dpath);
	if (dashd(dpath))
		return FULLUPDATE;
	delete_record(direct, sizeof (*bkninfo), ent);
	return FULLUPDATE;
}

static int
change_title(ent, bkninfo, direct)
int ent;
struct bknheader *bkninfo;
char *direct;
{
	char buf[STRLEN];
	if (!IScurrBM)
//by bjgyt        if (!HAS_PERM(PERM_OBOARDS) && !HAS_PERM(PERM_SYSOP))
                return DONOTHING;
	ytht_strsncpy(buf, bkninfo->title, 60);
	getdata(t_lines - 1, 0, "�¹�������: ", buf, 50, DOECHO, NA);
	if (buf[0] != '\0') {
		ytht_strsncpy(bkninfo->title, buf, sizeof(bkninfo->title));
		substitute_record(direct, bkninfo, sizeof (*bkninfo), ent);
		//change_dir(direct, bkninfo, (void *)DIR_do_changetitle, ent, digestmode, 1);
	}
	return PARTUPDATE;
}

static const struct one_key selectbacknumber_comms[] = {
	{'r', readbacknumber, "�Ķ�����"},
	{'L', show_allmsgs, "�鿴��Ϣ"},
	{'h', selbacknumberhelp, "�鿴����"},
	{'!', Q_Goodbye, "������վ"},
	{'S', s_msg, "����ѶϢ"},
	{'c', t_friends, "�鿴����"},
	{Ctrl('P'), new_backnumber, "�����¹���"},
	{'T', change_title, "�޸Ĺ�������"},
	{'D', delete_backnumber, "ɾ������"},
	{'\0', NULL, ""}
};

int
selectbacknumber()
{
	char backnumberboarddir[MAXPATHLEN];
	//��ʱ��һ���û��رչ���
	//if (!chk_currBM(currBM) && politics(currboard))
	//      return 0;
	sprintf(backnumberboarddir, "boards/.backnumbers/%s", currboard);
	if (!dashd(backnumberboarddir))
		if (mkdir(backnumberboarddir, 0770) < 0)
			return -1;
	sprintf(backnumberboarddir, "boards/.backnumbers/%s/%s", currboard,
		DOT_DIR);
	i_read(SELBACKNUMBER, backnumberboarddir, selectbacknumbertitle,
	       (void *) selectbacknumberdoent, selectbacknumber_comms,
	       sizeof (struct bknheader));
	return 0;
}

int
do_intobacknumber(filename, t)
char *filename;
time_t t;
{
	struct bknheader bknhdr;
	struct fileheader fhdr;
	char tmpfile[MAXPATHLEN], deleted[MAXPATHLEN];
	char bnpath[MAXPATHLEN], buf1[MAXPATHLEN], buf2[MAXPATHLEN];
	int fdr, fdw, fd;
	int count;
	time_t filet;

	if (digestmode != NA)
		return -1;

	sprintf(buf1, "boards/.backnumbers/%s/%s", currboard, DOT_DIR);
	fd = open(buf1, O_RDONLY);
	if (fd < 0) {
		prints("��δ��������");
		pressanykey();
		return -1;
	}
	if (lseek(fd, -sizeof (bknhdr), SEEK_END) < 0) {
		close(fd);
		return -1;
	}
	if (read(fd, &bknhdr, sizeof (bknhdr)) != sizeof (bknhdr)) {
		close(fd);
		return -1;
	}
	close(fd);
	sprintf(bnpath, "boards/.backnumbers/%s/%s", currboard,
		bknh2bknname(&bknhdr));
	if (!dashd(bnpath))
		return -1;

	tmpfilename(filename, tmpfile, deleted);
	if ((fdr = open(filename, O_RDONLY, 0)) == -1) {
		return -2;
	}
	if ((fdw = open(tmpfile, O_WRONLY | O_CREAT | O_EXCL, 0660)) == -1) {
		close(fdr);
		return -3;
	}
	flock(fdw, LOCK_EX);

	count = 1;
	while (read(fdr, &fhdr, sizeof (fhdr)) == sizeof (fhdr)) {
		filet = fhdr.filetime;
		if (filet > 0 && filet < t) {
			setbfile(buf1, currboard, fh2fname(&fhdr));
			if (!dashf(buf1))
				continue;
			sprintf(buf2, "%s/%s", bnpath, fh2fname(&fhdr));
			if (crossfs_rename(buf1, buf2) >= 0) {
				sprintf(buf2, "%s/%s", bnpath, DOT_DIR);
				append_record(buf2, &fhdr, sizeof (fhdr));
				continue;
			}
		}
		if ((safewrite(fdw, &fhdr, sizeof (fhdr)) == -1)) {
			unlink(tmpfile);
			flock(fdw, LOCK_UN);
			close(fdw);
			close(fdr);
			return -4;
		}
	}
	close(fdr);
	if (rename(filename, deleted) == -1) {
		flock(fdw, LOCK_UN);
		close(fdw);
		return -6;
	}
	if (rename(tmpfile, filename) == -1) {
		flock(fdw, LOCK_UN);
		close(fdw);
		return -7;
	}
	flock(fdw, LOCK_UN);
	close(fdw);
	return 0;
}
