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
#include "io.h"
#include "one_key.h"
#include "bbs_global_vars.h"
#include "bbs-internal.h"

static char boarddir1984[STRLEN * 2];
static int do1984title(void);
static char *do1984doent(int num, struct fileheader *ent, char buf[512]);
static int do1984_read(int, void *, char *);
static int do1984_done(int, void *, char *);
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

	// 审查文章
	showtitle("\xC9\xF3\xB2\xE9\xCE\xC4\xD5\xC2", MY_BBS_NAME);
	prints(
			// 离开[\033[1;32m←\033[m,\033[1;32me\033[m]  选择[\033[1;32m↑\033[m,\033[1;32m↓\033[m]  阅读
			"\xC0\xEB\xBF\xAA[\033[1;32m\xA1\xFB\033[m,\033[1;32me\033[m]  \xD1\xA1\xD4\xF1[\033[1;32m\xA1\xFC\033[m,\033[1;32m\xA1\xFD\033[m]  \xD4\xC4\xB6\xC1"
			// [\033[1;32m→\033[m,\033[1;32mRtn\033[m] 求助[\033[1;32mh\033[m]\033[m\n
			"[\033[1;32m\xA1\xFA\033[m,\033[1;32mRtn\033[m] \xC7\xF3\xD6\xFA[\033[1;32mh\033[m]\033[m\n");
	// \033[1;44m编号   %-12s %6s  %-50s\033[m\n
	// 刊登者
	// 日期
	// 标题
	prints("\033[1;44m\xB1\xE0\xBA\xC5   %-12s %6s  %-50s\033[m\n", "\xBF\xAF\xB5\xC7\xD5\xDF", "\xC8\xD5\xC6\xDA", "\xB1\xEA\xCC\xE2");
	clrtobot();
	return 0;
}

static char *do1984doent(int num, struct fileheader *ent, char buf[512])
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
			// -已经通过审查 by
			"-\xD2\xD1\xBE\xAD\xCD\xA8\xB9\xFD\xC9\xF3\xB2\xE9 by ", ent->title + 35);
	} else if (!strncmp("Re:", ent->title, 3)) {
		sprintf(buf, " %s%3d\033[m %c %-12.12s %6.6s %c%s%.50s\033[m",
			same ? c1 : "", num, ' ', b2, date, attached,
			same ? c1 : "", ent->title);
	} else {
		//  %s%3d\033[m %c %-12.12s %6.6s %c%s● %.47s\033[m
		sprintf(buf, " %s%3d\033[m %c %-12.12s %6.6s %c%s\xA1\xF1 %.47s\033[m",
			same ? c2 : "", num, ' ', b2, date, attached,
			same ? c2 : "", ent->title);
	}
	return buf;
}

static int do1984_read(int ent, void *record, char *direct)
{
	struct fileheader *fileinfo = record;
	char notgenbuf[128];
	int ch;

	clear();
	setqtitle(fileinfo->title);
	directfile(notgenbuf, direct, fh2fname(fileinfo));
/*	if (fileinfo->accessed[1] & FILE1_1984) {
		move(10, 30);
		// 本文已经通过审查!
		prints("\xB1\xBE\xCE\xC4\xD2\xD1\xBE\xAD\xCD\xA8\xB9\xFD\xC9\xF3\xB2\xE9!");
		pressanykey();
		return FULLUPDATE;
	} else*/
	ch = ansimore(notgenbuf, NA);
	move(t_lines - 1, 0);
	// \033[1;44;31m[阅读文章] \033[33m结束 Q,←│上一封 ↑,l│
	prints("\033[1;44;31m[\xD4\xC4\xB6\xC1\xCE\xC4\xD5\xC2] \033[33m\xBD\xE1\xCA\xF8 Q,\xA1\xFB\xA9\xA6\xC9\xCF\xD2\xBB\xB7\xE2 \xA1\xFC,l\xA9\xA6"
			// 下一封 n, <Space>,<Enter>,↓│主题阅读 x p \033[m
			"\xCF\xC2\xD2\xBB\xB7\xE2 n, <Space>,<Enter>,\xA1\xFD\xA9\xA6\xD6\xF7\xCC\xE2\xD4\xC4\xB6\xC1 x p \033[m");
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

static int do1984_done(int ent, void *record, char *direct)
{
	char titlebuf[60 /* see fileheader.title */];
	struct fileheader *fileinfo = record;
	if (fileinfo->accessed & FH_1984)
		return (PARTUPDATE);
	post_1984_to_board(direct, fileinfo);
	fileinfo->accessed |= FH_1984;
	snprintf(titlebuf, sizeof titlebuf, "%-32.32s - %s", fileinfo->title, currentuser.userid);
	ytht_strsncpy(fileinfo->title, titlebuf, sizeof(fileinfo->title));
	substitute_record(direct, fileinfo, sizeof (*fileinfo), ent);
	return (PARTUPDATE);
}

struct one_key do1984_comms[] = {
	// 阅读信件
	{'r', do1984_read, "\xD4\xC4\xB6\xC1\xD0\xC5\xBC\xFE"},
	// 查看消息
	{'L', show_allmsgs, "\xB2\xE9\xBF\xB4\xCF\xFB\xCF\xA2"},
// 查看帮助
//      {'h', do1984help, "\xB2\xE9\xBF\xB4\xB0\xEF\xD6\xFA"},
	// 通过文章
	{'c', do1984_done, "\xCD\xA8\xB9\xFD\xCE\xC4\xD5\xC2"},
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
	// 信区:
	ptr = strstr(buf, "\xD0\xC5\xC7\xF8: ");
	if (ptr == NULL) {
		fclose(fp);
		return -1;
	}
	// 信区:
	snprintf(board, 20, "%s", ptr + sizeof ("\xD0\xC5\xC7\xF8: ") - 1);
	ptr = strrchr(board, '\n');
	if (ptr != NULL)
		*ptr = 0;
	fgets(buf, sizeof (buf), fp);
	fclose(fp);
	// 标  题:
	ptr = strstr(buf, "\xB1\xEA  \xCC\xE2:");
	if (ptr == NULL) {
		return -3;
	}
	// 标  题:
	snprintf(title, 60, "%s", ptr + sizeof ("\xB1\xEA  \xCC\xE2: ") - 1);
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
		sprintf(newfname, "M.%ld.A", now);
		setbfile(newfilepath, sizeof(newfilepath), targetboard, newfname);
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
	ythtbbs_cache_Board_updatelastpost(targetboard);
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
		sprintf(newfname, "M.%ld.A",  now);
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
		ythtbbs_cache_Board_updatelastpost("tochecktoday");
	case 1:
		ythtbbs_cache_Board_updatelastpost("delete4request");
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
		// 请选择:
		"\xC7\xEB\xD1\xA1\xD4\xF1:"
		// 0)查看待审查文章
		"0)\xB2\xE9\xBF\xB4\xB4\xFD\xC9\xF3\xB2\xE9\xCE\xC4\xD5\xC2 "
		// 1)查看已删除的文章
		"1)\xB2\xE9\xBF\xB4\xD2\xD1\xC9\xBE\xB3\xFD\xB5\xC4\xCE\xC4\xD5\xC2 "
		// 2)解锁版面
		"2)\xBD\xE2\xCB\xF8\xB0\xE6\xC3\xE6 "
		// 3)锁住版面 [0]
		"3)\xCB\xF8\xD7\xA1\xB0\xE6\xC3\xE6 [0]",
		buf, 2, DOECHO, NA);
	switch (atoi(buf)) {
	case 0:
		// 要检查距今几天的文章？(默认为当天文章) [0]:
		getdata(7, 0, "\xD2\xAA\xBC\xEC\xB2\xE9\xBE\xE0\xBD\xF1\xBC\xB8\xCC\xEC\xB5\xC4\xCE\xC4\xD5\xC2\xA3\xBF(\xC4\xAC\xC8\xCF\xCE\xAA\xB5\xB1\xCC\xEC\xCE\xC4\xD5\xC2) [0]: ",
			buf, 3, DOECHO, YEA);
		day = atoi(buf);
		do1984(now - day * 86400, 0);
		return;
	case 1:
		do1984(now, 1);
		return;
	case 2:
		if (!ythtbbs_cache_utmp_get_watchman()) {
			// 目前版面并没有锁住!
			prints("\xC4\xBF\xC7\xB0\xB0\xE6\xC3\xE6\xB2\xA2\xC3\xBB\xD3\xD0\xCB\xF8\xD7\xA1!");
			pressreturn();
			return;
		}
		// 请输入你的用户登录密码:
		getdata(7, 0, "\xC7\xEB\xCA\xE4\xC8\xEB\xC4\xE3\xB5\xC4\xD3\xC3\xBB\xA7\xB5\xC7\xC2\xBC\xC3\xDC\xC2\xEB: ", buf, PASSLEN, NOECHO,
			YEA);
		if (*buf == '\0' || !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
			// \n\n很抱歉, 您输入的密码不正确。\n
			prints("\n\n\xBA\xDC\xB1\xA7\xC7\xB8, \xC4\xFA\xCA\xE4\xC8\xEB\xB5\xC4\xC3\xDC\xC2\xEB\xB2\xBB\xD5\xFD\xC8\xB7\xA1\xA3\n");
			pressreturn();
			return;
		}
		sprintf(tmpid, "%u", ythtbbs_cache_utmp_get_unlock() % 10000);
		// 请输入解锁码:
		getdata(8, 0, "\xC7\xEB\xCA\xE4\xC8\xEB\xBD\xE2\xCB\xF8\xC2\xEB:", buf, 5, DOECHO, YEA);

		if (strcmp(tmpid, buf)) {
			// 解锁码不对!\n
			prints("\xBD\xE2\xCB\xF8\xC2\xEB\xB2\xBB\xB6\xD4!\n");
			pressreturn();
			return;
		}
		if (!ythtbbs_cache_utmp_get_watchman()) {
			// 目前版面并没有锁住,看来有人比你先解锁了!
			prints("\xC4\xBF\xC7\xB0\xB0\xE6\xC3\xE6\xB2\xA2\xC3\xBB\xD3\xD0\xCB\xF8\xD7\xA1,\xBF\xB4\xC0\xB4\xD3\xD0\xC8\xCB\xB1\xC8\xC4\xE3\xCF\xC8\xBD\xE2\xCB\xF8\xC1\xCB!");
			pressreturn();
			return;
		}
		if (time(NULL) < ythtbbs_cache_utmp_get_watchman())
			// 及时
			strcpy(buf, "\xBC\xB0\xCA\xB1");
		else
			buf[0] = 0;
		ythtbbs_cache_utmp_set_watchman(0);
		// 用户 %s %s进行了政治类版面解锁操作
		sprintf(tmpid, "\xD3\xC3\xBB\xA7 %s %s\xBD\xF8\xD0\xD0\xC1\xCB\xD5\xFE\xD6\xCE\xC0\xE0\xB0\xE6\xC3\xE6\xBD\xE2\xCB\xF8\xB2\xD9\xD7\xF7",
			currentuser.userid, buf);
		postfile("help/watchmanhelp", "deleterequest", tmpid, 1);
		// 成功解锁!\n
		prints("\xB3\xC9\xB9\xA6\xBD\xE2\xCB\xF8!\n");
		pressreturn();
		return;
	case 3:
		if (ythtbbs_cache_utmp_get_watchman()) {
			// 目前版面已经被锁住了!
			prints("\xC4\xBF\xC7\xB0\xB0\xE6\xC3\xE6\xD2\xD1\xBE\xAD\xB1\xBB\xCB\xF8\xD7\xA1\xC1\xCB!");
			pressreturn();
			return;
		}
		// 请输入你的用户登录密码:
		getdata(7, 0, "\xC7\xEB\xCA\xE4\xC8\xEB\xC4\xE3\xB5\xC4\xD3\xC3\xBB\xA7\xB5\xC7\xC2\xBC\xC3\xDC\xC2\xEB: ", buf, PASSLEN, NOECHO, YEA);
		if (*buf == '\0' || !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
			// \n\n很抱歉, 您输入的密码不正确。\n
			prints("\n\n\xBA\xDC\xB1\xA7\xC7\xB8, \xC4\xFA\xCA\xE4\xC8\xEB\xB5\xC4\xC3\xDC\xC2\xEB\xB2\xBB\xD5\xFD\xC8\xB7\xA1\xA3\n");
			pressreturn();
			return;
		}
		if (ythtbbs_cache_utmp_get_watchman()) {
			// 目前版面已经被锁住了,看来有人比你先锁住版面了!
			prints("\xC4\xBF\xC7\xB0\xB0\xE6\xC3\xE6\xD2\xD1\xBE\xAD\xB1\xBB\xCB\xF8\xD7\xA1\xC1\xCB,\xBF\xB4\xC0\xB4\xD3\xD0\xC8\xCB\xB1\xC8\xC4\xE3\xCF\xC8\xCB\xF8\xD7\xA1\xB0\xE6\xC3\xE6\xC1\xCB!");
			pressreturn();
			return;
		}
		ythtbbs_cache_utmp_set_watchman(time(NULL) + 600);
		// 用户 %s 锁版!解锁码: %u
		sprintf(tmpid, "\xD3\xC3\xBB\xA7 %s \xCB\xF8\xB0\xE6!\xBD\xE2\xCB\xF8\xC2\xEB: %u",
			currentuser.userid, ythtbbs_cache_utmp_get_unlock() % 10000);
		postfile("help/watchmanhelp", "deleterequest", tmpid, 1);
		// 成功锁住!
		prints("\xB3\xC9\xB9\xA6\xCB\xF8\xD7\xA1!");
		pressreturn();
		return;
	default:
		return;
	}
}
