//writen by ecnegrevid, 2001.7
#include "bbs.h"
#include "bbstelnet.h"
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

	showtitle("ÔÄ¶Á¹ı¿¯", MY_BBS_NAME);
	prints
	    ("Àë¿ª[[1;32m¡û[m,[1;32me[m]  Ñ¡Ôñ[[1;32m¡ü[m,[1;32m¡ı[m]  ÔÄ¶Á"
	     "[[1;32m¡ú[m,[1;32mRtn[m] ÇóÖú[[1;32mh[m][m\n");
	prints("[1;44m±àºÅ   %-12s %6s  %-50s[m\n", "¿¯µÇÕß", "ÈÕÆÚ", "±êÌâ");
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
			"-±¾ÎÄÒÑ±»É¾³ı-");
	} else if (!strncmp("Re:", ent->title, 3)) {
		sprintf(buf, " %s%3d[m %c %-12.12s %6.6s %c%s%.50s[m",
			same ? c1 : "", num, type, b2, date, attached,
			same ? c1 : "", ent->title);
	} else {
		sprintf(buf, " %s%3d[m %c %-12.12s %6.6s %c%s¡ñ %.47s[m",
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
		prints("¶Ô²»Æğ£¬±¾ÎÄ±»ÄÚÈİ²»¿É¶Á£¡");
		pressanykey();
		return FULLUPDATE;
	} else
		ch = ansimore(notgenbuf, NA);
	move(t_lines - 1, 0);
	prints("\033[1;44;31m[ÔÄ¶ÁÎÄÕÂ] \033[33m½áÊø Q,¡û©¦ÉÏÒ»·â ¡ü,l©¦"
	       "ÏÂÒ»·â n, <Space>,<Enter>,¡ı©¦Ö÷ÌâÔÄ¶Á x p \033[m");
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
		if (DEFINE(DEF_THESIS)) {
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
	{'r', backnumber_read, "ÔÄ¶ÁĞÅ¼ş"},
	{'i', Save_post, "½«ÎÄÕÂ´æÈëÔİ´æµµ"},
	{'I', Import_post, "½«ĞÅ¼ş·ÅÈë¾«»ªÇø"},
	{'x', into_announce, "½øÈë¾«»ªÇø"},
#ifdef INTERNET_EMAIL
	{'F', forward_post, "×ª¼ÄĞÅ¼ş"},
	{'U', forward_u_post, "uuencode ×ª¼Ä"},
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
	{'h', backnumberhelp, "²é¿´°ïÖú"},
	{'!', Q_Goodbye, "¿ìËÙÀëÕ¾"},
	{'S', s_msg, "´«ËÍÑ¶Ï¢"},
	{'c', t_friends, "²é¿´ºÃÓÑ"},
	{'d', backnumber_hide, "Òş²ØĞÅ¼ş"},
	{'C', friend_author, "Ôö¼Ó×÷ÕßÎªºÃÓÑ"},
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
	sprintf(buf, "Ñ¡Ôñ¹ı¿¯");
	showtitle(buf, MY_BBS_NAME);
	prints
	    ("Àë¿ª[[1;32m¡û[m,[1;32me[m]  Ñ¡Ôñ[[1;32m¡ü[m,[1;32m¡ı[m]  ÔÄ¶Á[[1;32m¡ú[m,[1;32mRtn[m]  ¿ª±ÙĞÂ¹ı¿¯[[1;32m^P[m]  ÇóÖú[[1;32mh[m][m\n");
	prints("[1;44m±àºÅ %-12s %6s  %-50s[m\n", "°æÃæ", "ÈÕ  ÆÚ", "±ê  Ìâ");
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

	sprintf(buf, " %3d[m %-12.12s %6.6s  ¡ï %.47s[m",
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
	getdata(t_lines - 1, 0, "ÊäÈë¹ı¿¯±êÌâ: ", bn.title, 50, DOECHO, YEA);
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
	sprintf(content, "%s ´´½¨ %s °æ¹ı¿¯ %s", currentuser.userid, currboard,
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
	if (askyn("È·¶¨É¾³ı?", NA, NA) == NA)
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
	getdata(t_lines - 1, 0, "ĞÂ¹ı¿¯±êÌâ: ", buf, 50, DOECHO, NA);
	if (buf[0] != '\0') {
		ytht_strsncpy(bkninfo->title, buf, sizeof(bkninfo->title));
		substitute_record(direct, bkninfo, sizeof (*bkninfo), ent);
		//change_dir(direct, bkninfo, (void *)DIR_do_changetitle, ent, digestmode, 1);
	}
	return PARTUPDATE;
}

static const struct one_key selectbacknumber_comms[] = {
	{'r', readbacknumber, "ÔÄ¶Á¹ı¿¯"},
	{'L', show_allmsgs, "²é¿´ÏûÏ¢"},
	{'h', selbacknumberhelp, "²é¿´°ïÖú"},
	{'!', Q_Goodbye, "¿ìËÙÀëÕ¾"},
	{'S', s_msg, "´«ËÍÑ¶Ï¢"},
	{'c', t_friends, "²é¿´ºÃÓÑ"},
	{Ctrl('P'), new_backnumber, "¿ªÆôĞÂ¹ı¿¯"},
	{'T', change_title, "ĞŞ¸Ä¹ı¿¯±êÌâ"},
	{'D', delete_backnumber, "É¾³ı¹ı¿¯"},
	{'\0', NULL, ""}
};

int
selectbacknumber()
{
	char backnumberboarddir[MAXPATHLEN];
	//ÔİÊ±¶ÔÒ»°ãÓÃ»§¹Ø±Õ¹ı¿¯
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
		prints("ÉĞÎ´½¨Á¢¹ı¿¯");
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
