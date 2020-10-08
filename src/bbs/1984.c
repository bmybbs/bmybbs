//copy by lepton from backnumber.c writen by ecnegrevid, 2002.9.30
#include "bbs.h"

#include "main.h"
#include "smth_screen.h"
#include "bbsinc.h"
#include "xyz.h"
#include "more.h"
#include "read.h"
#include "sendmsg.h"
#include "stuff.h"
#include "bcache.h"
#include "io.h"
#include "one_key.h"
#include "bbs_global_vars.h"
#include "bbs-internal.h"

extern struct UTMPFILE *utmpshm;

static char boarddir1984[STRLEN * 2];
static int do1984title(void);
static char *do1984doent(int num, struct fileheader *ent, char buf[512]);
static int do1984_read(int ent, struct fileheader *fileinfo, char *direct);
static int do1984_done(int ent, struct fileheader *fileinfo, char *direct);
static int gettarget_board_title(char *board, char *title, char *filename);
static int do1984(time_t dtime, int mode);
static void post_1984_to_board(char *dir, struct fileheader *fileinfo);

void
set1984file(char *path, char *filename)
{
	strcpy(path, boarddir1984);
	*(strrchr(path, '/') + 1) = 0;
	strcat(path, filename);
}

static int
do1984title()
{

	showtitle("Éó²éÎÄÕÂ", MY_BBS_NAME);
	prints
	    ("Àë¿ª[[1;32m¡û[m,[1;32me[m]  Ñ¡Ôñ[[1;32m¡ü[m,[1;32m¡ı[m]  ÔÄ¶Á"
	     "[[1;32m¡ú[m,[1;32mRtn[m] ÇóÖú[[1;32mh[m][m\n");
	prints("[1;44m±àºÅ   %-12s %6s  %-50s[m\n", "¿¯µÇÕß", "ÈÕÆÚ", "±êÌâ");
	clrtobot();
	return 0;
}

static char *
do1984doent(num, ent, buf)
int num;
struct fileheader *ent;
char buf[512];
{
	char b2[512];
	time_t filetime;
	char *date;
	char *t;
	char attached;
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

	attached = (ent->accessed & FH_ATTACHED) ? '@' : ' ';
	strcpy(c1, "[1;36m");
	strcpy(c2, "[1;33m");
	if (!strcmp(ReadPost, ent->title) || !strcmp(ReplyPost, ent->title))
		same = YEA;
	strncpy(b2, ent->owner, STRLEN);
	if ((t = strchr(b2, ' ')) != NULL)
		*t = '\0';

	if (ent->accessed & FH_1984) {
		sprintf(buf, " %s%3d[m %c %-12.12s %6.6s %c%s%.36s%.14s[m",
			same ? c1 : "", num, ' ', "", "", ' ', same ? c1 : "",
			"-ÒÑ¾­Í¨¹ıÉó²é by ", ent->title + 35);
	} else if (!strncmp("Re:", ent->title, 3)) {
		sprintf(buf, " %s%3d[m %c %-12.12s %6.6s %c%s%.50s[m",
			same ? c1 : "", num, ' ', b2, date, attached,
			same ? c1 : "", ent->title);
	} else {
		sprintf(buf, " %s%3d[m %c %-12.12s %6.6s %c%s¡ñ %.47s[m",
			same ? c2 : "", num, ' ', b2, date, attached,
			same ? c2 : "", ent->title);
	}
	return buf;
}

static int
do1984_read(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char notgenbuf[128];
	int ch;

	clear();
	setqtitle(fileinfo->title);
	directfile(notgenbuf, direct, fh2fname(fileinfo));
/*	if (fileinfo->accessed[1] & FILE1_1984) {
		move(10, 30);
		prints("±¾ÎÄÒÑ¾­Í¨¹ıÉó²é!");
		pressanykey();
		return FULLUPDATE;
	} else*/
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
do1984_done(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	if (fileinfo->accessed & FH_1984)
		return (PARTUPDATE);
	post_1984_to_board(direct, fileinfo);
	fileinfo->accessed |= FH_1984;
	sprintf(fileinfo->title, "%-32.32s - %s", fileinfo->title,
		currentuser.userid);
	substitute_record(direct, fileinfo, sizeof (*fileinfo), ent);
	return (PARTUPDATE);
}

struct one_key do1984_comms[] = {
	{'r', do1984_read, "ÔÄ¶ÁĞÅ¼ş"},
	{'L', show_allmsgs, "²é¿´ÏûÏ¢"},
//      {'h', do1984help, "²é¿´°ïÖú"},
	{'c', do1984_done, "Í¨¹ıÎÄÕÂ"},
	{'\0', NULL, ""}
};

static int
do1984(time_t dtime, int mode)
{
	struct tm *n;

	if (0 == mode) {
		n = localtime(&dtime);
		sprintf(boarddir1984, "boards/.1984/%04d%02d%02d/",
			n->tm_year + 1900, n->tm_mon + 1, n->tm_mday);
	} else if (1 == mode) {
		sprintf(boarddir1984, "boards/.1985/");
	} else
		return -1;
	if (!dashd(boarddir1984))
		return -1;
	strcat(boarddir1984, DOT_DIR);
	i_read(DO1984, boarddir1984, do1984title,
	       (void *) do1984doent, do1984_comms, sizeof (struct fileheader));
	return 999;
}

static int
gettarget_board_title(char *board, char *title, char *filename)
{
	FILE *fp;
	char buf[256], *ptr;
	fp = fopen(filename, "r");
	if (fp == NULL)
		return -1;
	fgets(buf, sizeof (buf), fp);
	ptr = strstr(buf, "ĞÅÇø: ");
	if (ptr == NULL) {
		fclose(fp);
		return -1;
	}
	snprintf(board, 20, "%s", ptr + sizeof ("ĞÅÇø: ") - 1);
	ptr = strrchr(board, '\n');
	if (ptr != NULL)
		*ptr = 0;
	fgets(buf, sizeof (buf), fp);
	fclose(fp);
	ptr = strstr(buf, "±ê  Ìâ:");
	if (ptr == NULL) {
		fclose(fp);
		return -3;
	}
	snprintf(title, 60, "%s", ptr + sizeof ("±ê  Ìâ: ") - 1);
	ptr = strrchr(title, '\n');
	if (ptr != NULL)
		*ptr = 0;
	return 0;

}

static void
post_1984_to_board(char *dir, struct fileheader *fileinfo)
{
	char *ptr;
	char buf[STRLEN * 2];
	char newfilepath[STRLEN], newfname[STRLEN], targetboard[STRLEN],
	    title[STRLEN];
	struct fileheader postfile;
	time_t now;
	int count;
	snprintf(buf, STRLEN * 2, "%s", dir);
	ptr = strrchr(buf, '/');
	if (NULL == ptr)
		return;
	*(ptr + 1) = 0;
	strcat(ptr, fh2fname(fileinfo));

	memcpy(&postfile, fileinfo, sizeof (struct fileheader));

	now = time(NULL);
	count = 0;
	if (gettarget_board_title(targetboard, title, buf))
		return;
	strcpy(postfile.title, title);
	while (1) {
		sprintf(newfname, "M.%d.A", (int) now);
		setbfile(newfilepath, targetboard, newfname);
		if (link(buf, newfilepath) == 0) {
			//              unlink(buf);
			postfile.filetime = now;
			break;
		}
		now++;
		if (count++ > MAX_POSTRETRY)
			return;
	}
	if (postfile.thread == 0)
		postfile.thread = postfile.filetime;
	setbdir(buf, targetboard, NA);
	if (append_record(buf, &postfile, sizeof (postfile)) == -1) {
		errlog
		    ("checking '%s' on '%s': append_record failed!",
		     postfile.title, targetboard);
		pressreturn();
		return;
	}

	if (postfile.accessed & FH_INND)
		outgo_post(&postfile, targetboard, currentuser.userid,
			   currentuser.username);
	updatelastpost(targetboard);
	snprintf(genbuf, 256, "%s check1984 %s %s",
		 currentuser.userid, currboard, postfile.title);
	genbuf[256] = 0;
	newtrace(genbuf);

}

void
post_to_1984(char *file, struct fileheader *fileinfo, int mode)
{
	char buf[STRLEN * 2];
	char newfilepath[STRLEN], newfname[STRLEN];
	struct fileheader postfile;
	time_t now;
	int count;
	struct tm *n;

	now = time(NULL);
	if (0 == mode) {
		n = localtime(&now);
		sprintf(buf, "boards/.1984/%04d%02d%02d", n->tm_year + 1900,
			n->tm_mon + 1, n->tm_mday);
	} else if (1 == mode) {
		sprintf(buf, "boards/.1985");
	} else
		return;
	if (!dashd(buf))
		if (mkdir(buf, 0770) < 0)
			return;

	memcpy(&postfile, fileinfo, sizeof (struct fileheader));

	count = 0;
	while (1) {
		sprintf(newfname, "M.%d.A", (int) now);
		sprintf(newfilepath, "%s/%s", buf, newfname);
		if (link(file, newfilepath) == 0) {
			postfile.filetime = now;
			break;
		}
		now++;
		if (count++ > MAX_POSTRETRY)
			break;
	}
	strcat(buf, "/" DOT_DIR);
	if (append_record(buf, &postfile, sizeof (postfile)) == -1) {
		errlog
		    ("post1984 '%s' on '%s': append_record failed!",
		     postfile.title, currboard);
		pressreturn();
		return;
	}
	switch (mode) {
	case 0:
		updatelastpost("tochecktoday");
	case 1:
		updatelastpost("delete4request");
	default:
		break;
	}
	snprintf(genbuf, 256, "%s post %s %s",
		 currentuser.userid, currboard, postfile.title);
	genbuf[256] = 0;
	newtrace(genbuf);
	return;
}

void
do1984menu()
{
	time_t now;
	char buf[13];
	char tmpid[STRLEN];
	int day;
	now = time(NULL);
	modify_user_mode(DO1984);
	clear();
	move(5, 0);
	buf[0] = 0;
	getdata(6, 0,
		"ÇëÑ¡Ôñ:0)²é¿´´ıÉó²éÎÄÕÂ 1)²é¿´ÒÑÉ¾³ıµÄÎÄÕÂ 2)½âËø°æÃæ 3)Ëø×¡°æÃæ [0]",
		buf, 2, DOECHO, NA);
	switch (atoi(buf)) {
	case 0:
		getdata(7, 0, "Òª¼ì²é¾à½ñ¼¸ÌìµÄÎÄÕÂ£¿(Ä¬ÈÏÎªµ±ÌìÎÄÕÂ) [0]: ",
			buf, 3, DOECHO, YEA);
		day = atoi(buf);
		do1984(now - day * 86400, 0);
		return;
	case 1:
		do1984(now, 1);
		return;
	case 2:
		if (!utmpshm->watchman) {
			prints("Ä¿Ç°°æÃæ²¢Ã»ÓĞËø×¡!");
			pressreturn();
			return;
		}
		getdata(7, 0, "ÇëÊäÈëÄãµÄÓÃ»§µÇÂ¼ÃÜÂë: ", buf, PASSLEN, NOECHO,
			YEA);
		if (*buf == '\0' || !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
			prints("\n\nºÜ±§Ç¸, ÄúÊäÈëµÄÃÜÂë²»ÕıÈ·¡£\n");
			pressreturn();
			return;
		}
		sprintf(tmpid, "%u", utmpshm->unlock % 10000);
		getdata(8, 0, "ÇëÊäÈë½âËøÂë:", buf, 5, DOECHO, YEA);

		if (strcmp(tmpid, buf)) {
			prints("½âËøÂë²»¶Ô!\n");
			pressreturn();
			return;
		}
		if (!utmpshm->watchman) {
			prints("Ä¿Ç°°æÃæ²¢Ã»ÓĞËø×¡,¿´À´ÓĞÈË±ÈÄãÏÈ½âËøÁË!");
			pressreturn();
			return;
		}
		if (time(NULL) < utmpshm->watchman)
			strcpy(buf, "¼°Ê±");
		else
			buf[0] = 0;
		utmpshm->watchman = 0;
		sprintf(tmpid, "ÓÃ»§ %s %s½øĞĞÁËÕşÖÎÀà°æÃæ½âËø²Ù×÷",
			currentuser.userid, buf);
		postfile("help/watchmanhelp", "deleterequest", tmpid, 1);
		prints("³É¹¦½âËø!\n");
		pressreturn();
		return;
	case 3:
		if (utmpshm->watchman) {
			prints("Ä¿Ç°°æÃæÒÑ¾­±»Ëø×¡ÁË!");
			pressreturn();
			return;
		}
		getdata(7, 0, "ÇëÊäÈëÄãµÄÓÃ»§µÇÂ¼ÃÜÂë: ", buf, PASSLEN, NOECHO,
			YEA);
		if (*buf == '\0' || !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
			prints("\n\nºÜ±§Ç¸, ÄúÊäÈëµÄÃÜÂë²»ÕıÈ·¡£\n");
			pressreturn();
			return;
		}
		if (utmpshm->watchman) {
			prints
			    ("Ä¿Ç°°æÃæÒÑ¾­±»Ëø×¡ÁË,¿´À´ÓĞÈË±ÈÄãÏÈËø×¡°æÃæÁË!");
			pressreturn();
			return;
		}
		utmpshm->watchman = time(NULL) + 600;
		sprintf(tmpid, "ÓÃ»§ %s Ëø°æ!½âËøÂë: %u",
			currentuser.userid, utmpshm->unlock % 10000);
		postfile("help/watchmanhelp", "deleterequest", tmpid, 1);
		prints("³É¹¦Ëø×¡!");
		pressreturn();
		return;
	default:
		return;
	}
}
