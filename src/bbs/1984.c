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

	showtitle("审查文章", MY_BBS_NAME);
	prints(
			"离开[\033[1;32m←\033[m,\033[1;32me\033[m]  选择[\033[1;32m↑\033[m,\033[1;32m↓\033[m]  阅读"
			"[\033[1;32m→\033[m,\033[1;32mRtn\033[m] 求助[\033[1;32mh\033[m]\033[m\n");
	prints("\033[1;44m编号   %-12s %6s  %-50s\033[m\n", "刊登者", "日期", "标题");
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
	strcpy(c1, "\033[1;36m");
	strcpy(c2, "\033[1;33m");
	if (!strcmp(ReadPost, ent->title) || !strcmp(ReplyPost, ent->title))
		same = YEA;
	strncpy(b2, ent->owner, STRLEN);
	if ((t = strchr(b2, ' ')) != NULL)
		*t = '\0';

	if (ent->accessed & FH_1984) {
		sprintf(buf, " %s%3d\033[m %c %-12.12s %6.6s %c%s%.36s%.14s\033[m",
			same ? c1 : "", num, ' ', "", "", ' ', same ? c1 : "",
			"-已经通过审查 by ", ent->title + 35);
	} else if (!strncmp("Re:", ent->title, 3)) {
		sprintf(buf, " %s%3d\033[m %c %-12.12s %6.6s %c%s%.50s\033[m",
			same ? c1 : "", num, ' ', b2, date, attached,
			same ? c1 : "", ent->title);
	} else {
		sprintf(buf, " %s%3d\033[m %c %-12.12s %6.6s %c%s● %.47s\033[m",
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
		prints("本文已经通过审查!");
		pressanykey();
		return FULLUPDATE;
	} else*/
	ch = ansimore(notgenbuf, NA);
	move(t_lines - 1, 0);
	prints("\033[1;44;31m[阅读文章] \033[33m结束 Q,←│上一封 ↑,l│"
			"下一封 n, <Space>,<Enter>,↓│主题阅读 x p \033[m");
	//usleep(300000l);
	if (!(ch == KEY_RIGHT || ch == KEY_UP || ch == KEY_PGUP
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
	{'r', do1984_read, "阅读信件"},
	{'L', show_allmsgs, "查看消息"},
//      {'h', do1984help, "查看帮助"},
	{'c', do1984_done, "通过文章"},
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
	ptr = strstr(buf, "信区: ");
	if (ptr == NULL) {
		fclose(fp);
		return -1;
	}
	snprintf(board, 20, "%s", ptr + sizeof ("信区: ") - 1);
	ptr = strrchr(board, '\n');
	if (ptr != NULL)
		*ptr = 0;
	fgets(buf, sizeof (buf), fp);
	fclose(fp);
	ptr = strstr(buf, "标  题:");
	if (ptr == NULL) {
		return -3;
	}
	snprintf(title, 60, "%s", ptr + sizeof ("标  题: ") - 1);
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
	char newfilepath[STRLEN], newfname[STRLEN], targetboard[STRLEN], title[STRLEN];
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
		errlog("checking '%s' on '%s': append_record failed!", postfile.title, targetboard);
		pressreturn();
		return;
	}

	if (postfile.accessed & FH_INND)
		outgo_post(&postfile, targetboard, currentuser.userid, currentuser.username);
	updatelastpost(targetboard);
	snprintf(genbuf, 256, "%s check1984 %s %s", currentuser.userid, currboard, postfile.title);
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
		errlog("post1984 '%s' on '%s': append_record failed!", postfile.title, currboard);
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
	snprintf(genbuf, 256, "%s post %s %s", currentuser.userid, currboard, postfile.title);
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
		"请选择:0)查看待审查文章 1)查看已删除的文章 2)解锁版面 3)锁住版面 [0]",
		buf, 2, DOECHO, NA);
	switch (atoi(buf)) {
	case 0:
		getdata(7, 0, "要检查距今几天的文章？(默认为当天文章) [0]: ",
			buf, 3, DOECHO, YEA);
		day = atoi(buf);
		do1984(now - day * 86400, 0);
		return;
	case 1:
		do1984(now, 1);
		return;
	case 2:
		if (!ythtbbs_cache_utmp_get_watchman()) {
			prints("目前版面并没有锁住!");
			pressreturn();
			return;
		}
		getdata(7, 0, "请输入你的用户登录密码: ", buf, PASSLEN, NOECHO,
			YEA);
		if (*buf == '\0' || !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
			prints("\n\n很抱歉, 您输入的密码不正确。\n");
			pressreturn();
			return;
		}
		sprintf(tmpid, "%u", ythtbbs_cache_utmp_get_unlock() % 10000);
		getdata(8, 0, "请输入解锁码:", buf, 5, DOECHO, YEA);

		if (strcmp(tmpid, buf)) {
			prints("解锁码不对!\n");
			pressreturn();
			return;
		}
		if (!ythtbbs_cache_utmp_get_watchman()) {
			prints("目前版面并没有锁住,看来有人比你先解锁了!");
			pressreturn();
			return;
		}
		if (time(NULL) < ythtbbs_cache_utmp_get_watchman())
			strcpy(buf, "及时");
		else
			buf[0] = 0;
		ythtbbs_cache_utmp_set_watchman(0);
		sprintf(tmpid, "用户 %s %s进行了政治类版面解锁操作",
			currentuser.userid, buf);
		postfile("help/watchmanhelp", "deleterequest", tmpid, 1);
		prints("成功解锁!\n");
		pressreturn();
		return;
	case 3:
		if (ythtbbs_cache_utmp_get_watchman()) {
			prints("目前版面已经被锁住了!");
			pressreturn();
			return;
		}
		getdata(7, 0, "请输入你的用户登录密码: ", buf, PASSLEN, NOECHO, YEA);
		if (*buf == '\0' || !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
			prints("\n\n很抱歉, 您输入的密码不正确。\n");
			pressreturn();
			return;
		}
		if (ythtbbs_cache_utmp_get_watchman()) {
			prints("目前版面已经被锁住了,看来有人比你先锁住版面了!");
			pressreturn();
			return;
		}
		ythtbbs_cache_utmp_set_watchman(time(NULL) + 600);
		sprintf(tmpid, "用户 %s 锁版!解锁码: %u",
			currentuser.userid, ythtbbs_cache_utmp_get_unlock() % 10000);
		postfile("help/watchmanhelp", "deleterequest", tmpid, 1);
		prints("成功锁住!");
		pressreturn();
		return;
	default:
		return;
	}
}
