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
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include "ythtbbs/commend.h"
#include "ythtbbs/override.h"
#include "bbs.h"
#include "bbs_global_vars.h"
#include "one_key.h"
#include "talk.h"
#include "vote.h"
#include "bbsinc.h"
#include "sendmsg.h"
#include "mail.h"
#include "list.h"
#include "stuff.h"
#include "smth_screen.h"
#include "io.h"
#include "edit.h"
#include "boards.h"
#include "bcache.h"
#include "power_select.h"
#include "tmpl.h"
#include "main.h"
#include "read.h"
#include "bm.h"
#include "comm_list.h"
#include "xyz.h"
#include "more.h"
#include "namecomplete.h"
#include "1984.h"
#include "postheader.h"
#include "announce.h"
#include "maintain.h"
#include "backnumber.h"
#include "record.h"
#include "chat.h"
#include "help.h"
#include "bbs-internal.h"
#include "bmy/article.h"
#include "bmy/board.h"

struct postheader header;
int continue_flag;
int readpost;
int digestmode;
struct userec currentuser;
int usernum;
int local_article;
char currboard[STRLEN];
char IScurrBM = 0;
char ISdelrq = 0;
int selboard = 0;
int isattached = 0;
int inprison = 0;
int inBBSNET = 0;
struct mmapfile mf_badwords;
struct mmapfile mf_sbadwords;
struct mmapfile mf_pbadwords;

//void logbuf(char *buf, char *title, int headerindex);

extern int SR_BMDELFLAG;
static int show_cake(char *filename, int num);
static float myexp(float x);
static int isowner(struct userec *user, struct fileheader *fileinfo);

static int UndeleteArticle(int ent, struct fileheader *fileinfo, char *direct);
static void cpyfilename(struct fileheader *fhdr);
static int read_post(int ent, struct fileheader *fileinfo, char *direct);
static int do_select(int ent, struct fileheader *fileinfo, char *direct);
static int dele_digest(int filetime, char *direc);
static int garbage_line(char *str);
static void getcross(char *filepath, int mode);
static int post_cross(char *bname, int mode, int islocal, int hascheck, int dangerous);
static int post_article(struct fileheader *sfh);
static int dofilter(char *title, char *fn, int mode);
//static int edit_title(int ent, struct fileheader *fileinfo, char *direct);
static int markspec_post(int ent, struct fileheader *fileinfo, char *direct);
static int import_spec(void);
static int moveintobacknumber(int ent, struct fileheader *fileinfo, char *direct);
static int del_post_backup(int ent, struct fileheader *fileinfo, char *direct);
static int sequent_messages(struct fileheader *fptr);
static int sequential_read(int ent, struct fileheader *fileinfo, char *direct);
static int sequential_read2(int ent);
static void quickviewpost(int ent, struct fileheader *fileinfo, char *direct);
static int change_t_lines(void);
static int post_saved(void);
static int show_b_secnote(void);
static int show_b_note(void);
static int show_file_info(int ent, struct fileheader *fileinfo, char *direct);
static int do_t_query(void);
static int into_backnumber(void);
static int into_my_Personal(void);
static int select_Personal(void);
static void notepad(void);
static int Origin2(char text[256]);
static int deny_me_global(void);
static int do_thread(void);
static int skipattach(char *buf, int size, FILE * fp);
static int clear_new_flag(int ent, struct fileheader *fileinfo, char *direct);
static int b_notes_edit();
static int b_notes_passwd();
static int catnotepad(FILE * fp, char *fname);
static int change_content_title(char *fname, char *title);
static int get_mention_ids(char *article_path, char *mention_ids[]);
static int mark_commend(int ent, struct fileheader *fileinfo, char *direct);
static int mark_commend2(int ent, struct fileheader *fileinfo, char *direct);
static int commend_article(char* board, struct fileheader* fileinfo);
static int commend_article2(char* board, struct fileheader* fileinfo);
static int del_commend(int offset);
static int del_commend2(int offset);
static int do_commend(char* board, struct fileheader* fileinfo);
static int do_commend2(char* board, struct fileheader* fileinfo);
static int count_commend();
static int count_commend2();

/*-------by ylsdd- ------*/
/*进行一个问答*/
static int
show_cake(char *filename, int num)
{
	int i, count, n;
	static unsigned int num0 = 0;
	int tmp_mod;
	FILE *fp;
	char line[200], str[30];

	fp = fopen(filename, "r");
	if (fp == NULL)
		goto CAKEERROR0;
	if (NULL == fgets(line, 200, fp) || sscanf(line, "%d", &count) != 1)
		goto CAKEERROR1;

	num0 += num;
	n = num0 % count;

	while (1) {
		if (NULL == fgets(line, 200, fp) || sscanf(line, "%29s %d", str, &count) != 2)
			goto CAKEERROR1;
		if (count > n)
			break;
		n -= count;
	}

	fclose(fp);

	for (i = strlen(filename); i >= 0; i--) {
		if (filename[i] == '/')
			break;
	}
	if (i > 160)
		goto CAKEERROR0;

	memmove(line, filename, i + 1);
	strcpy(line + i + 1, str);

	fp = fopen(line, "r");
	if (fp == NULL)
		goto CAKEERROR0;
	for (i = 0; i < n;) {
		if (NULL == fgets(line, 200, fp))
			goto CAKEERROR1;
		if (line[0] == '#' && line[1] != '#')
			i++;
	}
	i = 0;
	prints("\033[1;28H\033[35mA PIECE OF CAKE\033[2;29H小 菜 一 碟\033[0m\n\n\n");

	while (NULL != fgets(line, 200, fp)) {
		i++;
		if (i > 25)
			goto CAKEERROR1;
		if (line[0] == '#' && line[1] != '#') {
			for (i = strlen(line); i >= 0; i--) {
				if (line[i] == ' ' || line[i] == '\r' || line[i] == '\t' || line[i] < 32)
					line[i] = '\0';
				else
					break;
			}
			if (i == 0) {
				tmp_mod = num0 % 10;
				sprintf(line, " 请输入数字'%d': ", tmp_mod);
				getdata(20, 1, line, str, 29, DOECHO, YEA);
				if (atoi(str) != tmp_mod) {
					prints ("\033[22;1H??..好象不大对吧, 您还有一次机会...");
					getdata(20, 1, line, str, 29, DOECHO, YEA);
					if (atoi(str) != tmp_mod) {
						prints ("啊? 竟然还不对? 真的是你么, 我好伤心...");
						pressreturn();
						Q_Goodbye();
					}
				}
				fclose(fp);
				pressreturn();
				return 0;
			}
			getdata(20, 1, "您的答案: ", str, 29, DOECHO, YEA);
			for (i = strlen(str); i >= 0; i--) {
				if (str[i] == ' ' || str[i] == '\r' || str[i] == '\t' || str[i] < 32)
					str[i] = '\0';
				else
					break;
			}
			if (strcasecmp(str, line + 1) == 0)
				prints("\033[22;1H哈, 正确!");
			else
				prints("\033[22;1H恩...好象不大对吧? //think");
			fclose(fp);
			pressreturn();
			return 0;
		}
		if (line[0] == '#' && line[1] == '#')
			prints(line + 1);
		else
			prints(line);
	}
CAKEERROR1:
	fclose(fp);
CAKEERROR0:
	prints("\033[22;1H面包变质了?");
	pressreturn();
	return -1;
}

/* 在post_article中使用, 用来限制灌水 */
#define DELAYFACTOR1 20.
#define DELAYFACTOR2 100
#define DELAYFACTOR3 25
#define DELAYTIME 10

/* *INDENT-OFF* */
static float
myexp(float x)
{
	if (x < -4)
		return 0;
	return 1 + x * (1 + x / 2 * (1 + x / 3 * (1 + x / 4 * (1 + x / 5 * (1 + x / 6 *
		(1 + x / 7 * (1 + x / 8 * (1 + x / 9 * (1 + x / 10 * (1 + x / 11 * (1 + x
		/ 12 * (1 + x / 13 * (1 + x / 14 * (1 + x / 15 / 1.4))))))))))))));
}
/* *INDENT-ON* */

void
do_delay(int i)
{
	static time_t postlasttime = 0, ppt;
	static float postactivelevel = 0, ppa;
	time_t timenow;

	float ee;
	if (i == -1) {
		postactivelevel = ppa;
		postlasttime = ppt;
		return;
	}
	ppa = postactivelevel;
	ppt = postlasttime;
	timenow = time(NULL);
	ee = myexp(-(timenow - postlasttime) / DELAYFACTOR1 -
		0.5 * ((timenow - postlasttime < 4) ? (timenow - postlasttime - 4) : 0));
	if (postactivelevel * ee > DELAYFACTOR2) {
		show_cake("etc/cake/show_cakelist", timenow);
		ppa /= 2;
		postactivelevel /= 2;
	}
	postactivelevel = postactivelevel * ee + DELAYFACTOR3 * i;
	postlasttime = timenow;
}

/*------end by ylsdd-----*/
/* start by gluon for no reply */
/* Added by deardragon 1999.11.21 增加不可 RE 属性 */
int
underline_post(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	if (!IScurrBM && !isowner(&currentuser, fileinfo)) {
		return DONOTHING;
	}
	change_dir(direct, fileinfo, (void *) DIR_do_underline, ent, digestmode, 0);
	if (!bmy_board_is_system_board(currboard) && fileinfo->thread == fileinfo->filetime) {
		bmy_article_update_thread_accessed(ythtbbs_cache_Board_get_idx_by_name(currboard) + 1, fileinfo->thread, fileinfo->accessed);
	}
	return PARTUPDATE;
}

static int allcanre_post(int ent, struct fileheader *fileinfo, char *direct) {
	if (!HAS_PERM(PERM_SYSOP, currentuser))
		return DONOTHING;
	change_dir(direct, fileinfo, (void *) DIR_do_allcanre, ent, digestmode, 0);
	if (!bmy_board_is_system_board(currboard) && fileinfo->thread == fileinfo->filetime) {
		bmy_article_update_thread_accessed(ythtbbs_cache_Board_get_idx_by_name(currboard) + 1, fileinfo->thread, fileinfo->accessed);
	}
	return PARTUPDATE;
}

/* Added End. */

char ReadPost[STRLEN] = "";
char ReplyPost[STRLEN] = "";
int readingthread;

extern time_t login_start_time;
extern int toggle1, toggle2;
extern char fromhost[];

char genbuf[1024];
char quote_title[120], quote_board[120];
char quote_file[120], quote_user[120];
int totalusers, usercounter;

static int
isowner(user, fileinfo)
struct userec *user;
struct fileheader *fileinfo;
{
	if (strcmp(fileinfo->owner, user->userid))
		return 0;
	if (fileinfo->filetime < user->firstlogin)
		return 0;
	if (!strcmp(user->userid, "guest"))
		return 0;
	return 1;
}

int
set_safe_record()
{
	struct userec tmp;
	extern int ERROR_READ_SYSTEM_FILE;

	if (get_record(PASSFILE, &tmp, sizeof (currentuser), usernum) == -1) {
		errlog("Error:Read Passfile %4d %12.12s", usernum, currentuser.userid);
		ERROR_READ_SYSTEM_FILE = YEA;
		abort_bbs();
		return -1;
	}
	currentuser.numposts = tmp.numposts;
	currentuser.userlevel = tmp.userlevel;
	currentuser.numlogins = tmp.numlogins;
	currentuser.numdays = tmp.numdays;
	currentuser.lastlogin = tmp.lastlogin;
	currentuser.dietime = tmp.dietime;
	ytht_strsncpy(currentuser.lasthost, fromhost, BMY_IPV6_LEN);
	currentuser.stay = tmp.stay;
	return 0;
}

/*Add by SmallPig*/
void setqtitle(char *stitle) {
	if (strncmp(stitle, "Re: ", 4) != 0 && strncmp(stitle, "RE: ", 4) != 0) {
		snprintf(ReplyPost, 55, "Re: %s", stitle);
		strcpy(ReadPost, stitle);
	} else {
		strcpy(ReplyPost, stitle);
		strcpy(ReadPost, ReplyPost + 4);
	}
}

int
chk_currBM(bh, isbig)
struct boardheader *bh;
int isbig;
{
	if (HAS_PERM(PERM_BLEVELS, currentuser))
		return YEA;

	if (HAS_PERM(PERM_SPECIAL4, currentuser) && issecm(bh->sec1, currentuser.userid))
		return YEA;

	return (HAS_PERM(PERM_BOARDS, currentuser) && chk_BM(&currentuser, bh, isbig));
}

void
setquotefile(filepath)
char filepath[];
{
	strcpy(quote_file, filepath);
}

char *
setbpath(buf, boardname)
char *buf, *boardname;
{
	strcpy(buf, "boards/");
	strcat(buf, boardname);
	return buf;
}

char *
setbfile(buf, boardname, filename)
char *buf, *boardname, *filename;
{
	sprintf(buf, "boards/%s/%s", boardname, filename);
	return buf;
}

int
deny_me(char *bname)
{
	char buf[STRLEN];
	int deny1, deny2;
	setbfile(buf, bname, "deny_users");
	deny1 = seek_in_file(buf, currentuser.userid);
	setbfile(buf, bname, "deny_anony");
	deny2 = seek_in_file(buf, currentuser.userid);
	return (deny1 || deny2);
}

static int
deny_me_global()
{
	return seek_in_file("deny_users", currentuser.userid);
}

/*Add by SmallPig*/
void
shownotepad()
{
	modify_user_mode(NOTEPAD);
	ansimore("etc/notepad", YEA);
	clear();
	return;
}

static int g_board_names(struct boardmem *fhdrp, int curr_idx, va_list ap) {
	(void) curr_idx;
	(void) ap;
	if (fhdrp->header.filename[0] && hasreadperm(&(fhdrp->header))) {
		AddNameList(fhdrp->header.filename);
	}
	return 0;
}

void
make_blist()
{
	CreateNameList();
	ythtbbs_cache_Board_foreach_v(g_board_names);
}

static int g_board_names_full(struct boardmem *fhdrp, int curr_idx, va_list ap) {
	(void) curr_idx;
	(void) ap;
	if (fhdrp->header.filename[0])
		AddNameList(fhdrp->header.filename);
	return 0;
}

void
make_blist_full()
{
	CreateNameList();
	ythtbbs_cache_Board_foreach_v(g_board_names_full);
}

int
junkboard()
{
	return seek_in_file("etc/junkboards", currboard) || club_board(currboard);
}

void
Select()
{
	modify_user_mode(SELECT);
	do_select(0, NULL, NULL);
	Read();
}

#if 0
int
Post()
{
	if (!selboard) {
		prints("\n\n先用 (S)elect 去选择一个讨论区。\n");
		pressreturn();
		clear();
		return 0;
	}
	do_post();
	return 0;
}
#endif
int postfile(char *filename, char *nboard, char *posttitle, int mode) {
	int retv;
	int save_in_mail;
	save_in_mail = in_mail;
	in_mail = NA;
//	strcpy(quote_board, nboard);
	memset(quote_board, 0, 120);
	memcpy(quote_board, currboard, strlen(currboard));
	strcpy(quote_file, filename);
	strcpy(quote_title, posttitle);
	retv = post_cross(nboard, mode, 1, 0, 0);
	in_mail = save_in_mail;
	return retv;
}

int
get_a_boardname(bname, prompt)
char *bname, *prompt;
{
	struct boardheader fh;

	make_blist();
	namecomplete(prompt, bname);
	if (*bname == '\0') {
		return 0;
	}
	if (new_search_record(BOARDS, &fh, sizeof (fh), (void *) cmpbnames, bname) <= 0) {
		move(1, 0);
		prints("错误的讨论区名称\n");
		pressreturn();
		move(1, 0);
		return 0;
	}
	return 1;
}

/* undelete 一篇文章 Leeward 98.05.18 */
/* modified by ylsdd */
static int
UndeleteArticle(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char *p, buf[1024];
	char UTitle[128];
	char filepath[STRLEN * 2];
	struct fileheader UFile;
	int i;
	FILE *fp;

	if (digestmode != 4 && digestmode != 5)
		return DONOTHING;
	if (!IScurrBM)
		return DONOTHING;
	snprintf(filepath, sizeof(filepath), "boards/%s/%s", currboard, fh2fname(fileinfo));
	if (!dashf(filepath)) {
		clear();
		move(2, 0);
		prints("该文章不存在，已被恢复, 删除或列表出错");
		pressreturn();
		return FULLUPDATE;
	}
	fp = fopen(filepath, "r");
	if (!fp)
		return DONOTHING;

	strcpy(UTitle, fileinfo->title);
	if ((p = strrchr(UTitle, '-'))) {	/* create default article title */
		*p = 0;
		for (i = strlen(UTitle) - 1; i >= 0; i--) {
			if (UTitle[i] != ' ')
				break;
			else
				UTitle[i] = 0;
		}
	}

	i = 0;
	while (!feof(fp) && i < 2) {
		fgets(buf, 1024, fp);
		if (feof(fp))
			break;
		if (strstr(buf, "发信人: ") && strstr(buf, "), 信区: ")) {
			i++;
		} else if (strstr(buf, "标  题: ")) {
			i++;
			strcpy(UTitle, buf + 8);
			if ((p = strchr(UTitle, '\n')))
				*p = 0;
		}
	}
	fclose(fp);

	UFile = *fileinfo;
	ytht_strsncpy(UFile.title, UTitle, sizeof(UFile.title));
	UFile.deltime = 0;
	UFile.accessed &= ~FH_DEL;

	if ((fileinfo->accessed & FH_ISDIGEST))
		sprintf(buf, "boards/%s/.DIGEST", currboard);
	else
		sprintf(buf, "boards/%s/.DIR", currboard);
	if (1) {
		char newfilepath[STRLEN], newfname[STRLEN];
		int count, now;
		now = time(NULL);
		count = 0;
		while (1) {
			sprintf(newfname, "%c.%d.A",
				(fileinfo->accessed & FH_ISDIGEST) ? 'G' : 'M',
				(int) now);
			setbfile(newfilepath, currboard, newfname);
			if (link(filepath, newfilepath) == 0) {
				unlink(filepath);
				UFile.filetime = now;
				break;
			}
			now++;
			if (count++ > MAX_POSTRETRY)
				break;
		}
	}
	fh_find_thread(&UFile, currboard);
	append_record(buf, &UFile, sizeof (UFile));
	updatelastpost(currboard);
	fileinfo->filetime = 0;
	substitute_record(direct, fileinfo, sizeof (*fileinfo), ent);
	sprintf(buf, "%s undel %s %s %s", currentuser.userid, currboard,
		UFile.owner, UFile.title);
	newtrace(buf);

	clear();
	move(2, 0);
	prints("'%s' 已恢复到版面 \n", UFile.title);
	pressreturn();

	return FULLUPDATE;
}

/* Add by SmallPig */
int
do_cross(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char bname[STRLEN];
	char ispost[10];
	int ddigestmode;
	int islocal;
	int hide1, hide2;
	int dangerous;

	if (!HAS_PERM(PERM_POST, currentuser))
		return DONOTHING;

	if (inprison) {
		move(0, 0);
		clear();
		prints("安心坐牢,不要胡闹! :)");
		pressanykey();
		return FULLUPDATE;
	}

	if (currentuser.dietime) {
		move(0, 0);
		clear();
		prints("不颡镥族稹弩篑啊,豚犭卷囡甑点喔.\n(这是鬼话,你不懂的,反正就是不让转贴就是啦!!!)");
		pressanykey();
		return FULLUPDATE;
	}

	if (uinfo.mode == RMAIL || in_mail)
		setmailfile_s(genbuf, sizeof(genbuf), currentuser.userid, fh2fname(fileinfo));
	else if (uinfo.mode == BACKNUMBER)
		directfile(genbuf, direct, fh2fname(fileinfo));
	else
		sprintf(genbuf, "boards/%s/%s", currboard, fh2fname(fileinfo));;
	ytht_strsncpy(quote_file, genbuf, sizeof(quote_file));
	ytht_strsncpy(quote_title, fileinfo->title, sizeof(quote_title));

	clear();
	prints("\033[1m请注意：本站站规规定：内容相同或类似的文章严禁在\033[31m3(不含)\033[37m个以上讨论区重复张贴。\n");
	prints("\033[1m转贴超过3个讨论区者除所贴文章会被全部删除之外，还将被剥夺全站发表文章的权利。\n");
	prints("\033[1m             请大家共同维护 BBS 的环境，节省系统资源。谢谢合作。\n\033[0m");
	move(4, 0);
	if (!get_a_boardname(bname, "请输入要转贴的讨论区名称: ")) {
		return FULLUPDATE;
	}
	hide1 = hideboard(currboard);
	hide2 = hideboard(bname);
	if (hide1 && !hide2)
		return FULLUPDATE;
	dangerous = dofilter(quote_title, quote_file, political_board(bname));
	if (!hide2 && ((uinfo.mode == RMAIL) || (uinfo.mode == BACKNUMBER)) && dangerous == -1)
		return FULLUPDATE;
	if (is1984_board(bname))
		return FULLUPDATE;
	move(5, 0);
	clrtoeol();
	prints("转载 ' %s ' 到 %s 版 ", quote_title, bname);
	move(6, 0);
	if (innd_board(bname)) {
		getdata(7, 0, "(S)发表 (L)不转信 (A)取消? [L]: ", ispost,
			9, DOECHO, YEA);
		if (ispost[0] != 'a' && ispost[0] != 'A') ispost[0] = 'l';//add by bjgyt
		islocal = 0;
		if (ispost[0] == 'l' || ispost[0] == 'L') {
			islocal = 1;
			ispost[0] = 's';
		}
	} else {
		getdata(7, 0, "(L)发表 (A)取消? [L]: ", ispost, 9, DOECHO, YEA);
		if (ispost[0] != 'a' && ispost[0] != 'A') ispost[0] = 'l';//add by bjgyt
		if (ispost[0] == 'l' || ispost[0] == 'L')
			ispost[0] = 's';
		islocal = 1;
	}
	if (ispost[0] == 's' || ispost[0] == 'S') {
		if (deny_me(bname) && !HAS_PERM(PERM_SYSOP, currentuser)) {
			move(8, 0);
			clrtobot();
			prints("\n\n                 很抱歉，你被版主停止 POST 的权利。");
			pressreturn();
			clear();
			return FULLUPDATE;
		}
		if (deny_me_global() && !HAS_PERM(PERM_SYSOP, currentuser)) {
			move(8, 0);
			clrtobot();
			prints("\n\n                 很抱歉，你被站务停止全站 POST 的权利。");
			pressreturn();
			clear();
			return FULLUPDATE;
		}
		strcpy(quote_board, currboard);
		ddigestmode = digestmode;
		digestmode = 0;

		if (post_cross(bname, 0, islocal, 1, dangerous) == -1) {
			move(8, 0);
			prints("Failed!");
			pressreturn();
			digestmode = ddigestmode;
			return FULLUPDATE;
		}
		digestmode = ddigestmode;
		move(8, 0);
		prints("' %s ' 已转贴到 %s 版 \n", quote_title, bname);
	} else {
		move(8, 0);
		prints("取消");
	}
	pressreturn();
	return FULLUPDATE;
}

int currfiletime;

int
cmpfilename(fhdr)
struct fileheader *fhdr;
{
	return (fhdr->filetime == currfiletime);
}

static void
cpyfilename(fhdr)
struct fileheader *fhdr;
{
	char buf[STRLEN];
	time_t tnow;
	struct tm *now;

	tnow = time(NULL);
	now = localtime(&tnow);

	sprintf(buf, "-%s", fhdr->owner);
	ytht_strsncpy(fhdr->owner, buf, sizeof(fhdr->owner));
	sprintf(buf, "<< 本文被 %s 于 %d/%d %d:%02d:%02d 删除 >>",
		currentuser.userid, now->tm_mon + 1, now->tm_mday,
		now->tm_hour, now->tm_min, now->tm_sec);
	ytht_strsncpy(fhdr->title, buf, sizeof(fhdr->title));
	fhdr->filetime = 0;	//表示文章被删除了
}

#ifdef ENABLE_MYSQL
static int
do_evaluate(int ent, struct fileheader *fhdr, char *direct, int mode)
{
	int cl, count, ch, oldstar;
	float avg;
	if (hideboard(currboard))
		return -1;
	modify_user_mode(POSTING);
	move(20, 0);
	clrtobot();
	if (now_t - fhdr->filetime > 3 * 86400) {
		if (mode)
			return -1;
		prints("\n这么老的文章就别评价了吧,去评点新文章吧 :)");
		pressreturn();
		return -1;
	}

	move(t_lines - 4, 0);
	clrtobot();
	prints("请问您对这篇文章的印象如何?\n");
	prints("0. 放弃 1. 一般 2. 好 3. 很好 4. 强烈推荐 [0]: \n");
	ch = igetkey();
	if (!isdigit(ch))
		cl = 0;
	else
		cl = ch - '0';
	if (cl == 0 && !mode)
		return 0;
	cl++;
	if (cl > 5)
		cl = 5;
	if (cl < 0)
		return 0;
	//oldstar = 0/1都认为是未评价状态
	if (bbseva_qset(utmpent, currboard, fh2fname(fhdr), currentuser.userid, cl, &oldstar, &count, &avg) < 0)
		return 0;
	if (oldstar != cl) {
		fhdr->staravg50 = (int) (50 * avg);
		fhdr->hasvoted = count > 255 ? 255 : count;;
		change_dir(direct, fhdr, (void *) DIR_do_evaluate, ent, digestmode, 1);
	}

	if (cl == 1 || cl == oldstar) {
		if (!mode) {
			prints("您没有改变您对这篇文章的评价\n");
			pressanykey();
		}
	} else {
		if (oldstar != 1 && oldstar != 0)
			prints("您把您对这篇文章的评价从%d星级改到%d星级", oldstar - 1, cl - 1);
		else
			prints("这篇文章被您评价为%d星级", cl - 1);
		pressanykey();
	}
	return 0;
}
#endif

static int // slowaction
dele_digest_top(filetime, direc)
int filetime;
char *direc;
{
	char digest_name[STRLEN];
	char new_dir[STRLEN];
	int tmpcurrfiletime;
	struct fileheader fh;
	int pos;
	sprintf(digest_name, "T.%d.A", filetime);
	directfile(new_dir, direc, TOPFILE_DIR);
	tmpcurrfiletime = currfiletime;
	currfiletime = filetime;
	pos = new_search_record(new_dir, &fh, sizeof (fh),(void *) cmpfilename, NULL);
	if (pos <= 0)
		return 0;
	delete_file(new_dir, sizeof (struct fileheader),pos, (void *) cmpfilename);
	currfiletime = tmpcurrfiletime;
	directfile(new_dir, direc, digest_name);
	unlink(new_dir);
	return 0;
}

static int topfile_post(int ent, struct fileheader *fhdr, char *direct) //slowaction
{
	if (!IScurrBM) return DONOTHING;

	if (fhdr->accessed & FILE_TOP1) {
		fhdr->accessed &= ~FILE_TOP1;
		//dele_digest(fhdr->filetime, direct);
		dele_digest_top(fhdr->filetime, direct); //add by hace
		snprintf(genbuf, 256, "%s 去掉置顶 %s %s %s",currentuser.userid, currboard, fhdr->owner,fhdr->title);
		newtrace(genbuf);
	} else {
		struct fileheader digest;
		char digestdir[STRLEN], digestfile[STRLEN], oldfile[STRLEN];
		directfile(digestdir, direct, TOPFILE_DIR);
		if (get_num_records(digestdir, sizeof (digest)) > 8 && strcmp(currentuser.userid, "SYSOP")!=0) {
			move(3, 0);
			clrtobot();
			move(4, 10);
			prints ("抱歉，置底数量超过5篇，无法再加入...\n");
			pressanykey();
			return PARTUPDATE;
		}
		digest = *fhdr;
		digest.accessed |= FILE_ISTOP1;
		directfile(digestfile, direct, fh2fname(&digest));
		directfile(oldfile, direct, fh2fname(fhdr));

		if (!dashf(digestfile)) {
			digest.accessed |= FILE_ISTOP1;
			link(oldfile, digestfile);
			append_record(digestdir, &digest, sizeof (digest));
			//topfile_record(direct,fhdr->filename,ent);
			//add by hace
			snprintf(genbuf, 256, "%s 置顶 %s %s %s",currentuser.userid, currboard, fhdr->owner,fhdr->title);
			newtrace(genbuf);
		}
	}
	change_dir(direct, fhdr, (void *) DIR_do_top, ent, 0, 0);
	if (!bmy_board_is_system_board(currboard) && fhdr->thread == fhdr->filetime) {
		bmy_article_update_thread_accessed(ythtbbs_cache_Board_get_idx_by_name(currboard) + 1, fhdr->thread, fhdr->accessed);
	}
	//topfile_record(direct,fhdr->filename,ent); //add by hace
	return PARTUPDATE;
}

static int
read_post(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	int ch;
	time_t starttime;
#ifdef ENABLE_MYSQL
	int cou;
	extern int effectiveline;
	static time_t lastevaluetime = 0;
#endif
	starttime = time(NULL);
	clear();
	directfile(genbuf, direct, fh2fname(fileinfo));
	SETREAD(fileinfo, &brc);
	ytht_strsncpy(quote_file, genbuf, sizeof(quote_file));
	ytht_strsncpy(quote_board, currboard, 24);
	ytht_strsncpy(quote_title, fileinfo->title, sizeof(quote_title));
	ytht_strsncpy(quote_user, fh2owner(fileinfo), sizeof (quote_user));
	isattached = 0;
#ifndef NOREPLY
	ch = ansimore_withzmodem(genbuf, NA, fileinfo->title);
#else
	ch = ansimore_withzmodem(genbuf, YEA, fileinfo->title);
#endif
	if (fileinfo->accessed & FH_ALLREPLY) {
		FILE *fp;
		time_t n;
		fp = fopen("bbstmpfs/dynamic/Bvisit_log", "a");
		if (NULL != fp) {
			n = time(0);
			fprintf(fp, "telnet/ssh user %s from %s visit %s %s %s",
				currentuser.userid, fromhost, currboard,
				fh2fname(fileinfo), ctime(&n));
			fclose(fp);
		}
	}
	//fileinfo->viewtime++;
	if (!(fileinfo->accessed & FH_ATTACHED) && isattached) {
		change_dir(direct, fileinfo, (void *) DIR_do_attach, ent, digestmode, 0);
	}
#ifndef NOREPLY
	move(t_lines - 1, 0);
	clrtoeol();
	if (haspostperm(currboard) && (time(NULL) - fileinfo->filetime < 86400 * 3)) {
		prints("\033[1;44;31m[阅读文章] \033[33m回文 R│推荐给网友 E│结束 ←│上一封↑l│下一封↓n│主题阅读 x p\033[m");
	} else {
		prints("\033[1;44;31m[阅读文章] \033[33m结束 Q,←│上一封 ↑,l│下一封 n, <Space>,<Enter>,↓│主题阅读 x p \033[m");
	}

	// 删除提醒开始
	if(is_post_in_notification(currentuser.userid, currboard, fileinfo->filetime)) {
		del_post_notification(currentuser.userid, currboard, fileinfo->filetime);
	}

	/* Re-Write By Excellent */

	readingthread = fileinfo->thread;
	if (strncmp(fileinfo->title, "Re: ", 4) != 0) {
		strcpy(ReplyPost, "Re: ");
		ytht_strsncpy(ReplyPost + 4, fileinfo->title, sizeof(ReplyPost) - 4);
		ytht_strsncpy(ReadPost, fileinfo->title, sizeof(ReadPost));
	} else {
		ytht_strsncpy(ReplyPost, fileinfo->title, sizeof(ReplyPost));
		ytht_strsncpy(ReadPost, fileinfo->title + 4, sizeof(ReadPost));
	}

	if (!(ch == KEY_RIGHT || ch == KEY_UP || ch == KEY_PGUP || ch == KEY_DOWN) && (ch <= 0 || strchr("RrEexp", ch) == NULL))
		ch = egetch();

#ifdef ENABLE_MYSQL
	cou = now_t - starttime;

	if (ch != 'E' && ch != 'e' && cou >= 3 && effectiveline >= 5 && ((now_t - lastevaluetime) >= 60 * 5)) {
		if (cou > 10)
			cou = 5;
		else if (cou > 5)
			cou = 8;
		else
			cou = 10;
		if (rand() % cou == 0) {
			if (do_evaluate(ent, fileinfo, direct, 1) >= 0)
				lastevaluetime = time(NULL);
		}
	}
#endif

	switch (ch) {
	case 'Q':
	case 'q':
	case KEY_LEFT:
		break;
	case 'j':
	case KEY_RIGHT:
		if (DEFINE(DEF_THESIS, currentuser)) {	/* youzi */
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
	case 'Y':
	case 'R':
	case 'y':
	case 'r':
/* Added by deardragon 1999.11.21 增加不可 RE 属性 */
// comment by bjgyt
		if (!(fileinfo->accessed & FH_NOREPLY))
//		if (!(fileinfo->accessed & FH_NOREPLY) && !(fileinfo->accessed & FILE_TOP1))
			do_reply(fileinfo);
		else {
			move(3, 0);
			clrtobot();
			prints("\n\n    对不起, 本文被设置为不可Re!!!    ");
			pressreturn();
			clear();
		}
/* Added End. */
		break;
	case Ctrl('R'):
		post_reply(ent, fileinfo, direct);
		break;
#ifdef ENABLE_MYSQL
	case 'E':
	case 'e':
		do_evaluate(ent, fileinfo, direct, 0);
		break;
#endif
	case 'g':
		digest_post(ent, fileinfo, direct);
		break;
	case Ctrl('U'):
		sread(0, 1, ent, 1, fileinfo);
		break;
	case Ctrl('N'):
		SR_first(ent, fileinfo, direct);
		ent = sread(3, 0, ent, 0, fileinfo);
		sread(0, 1, ent, 0, fileinfo);
		break;
	case 'x':
		sread(0, 0, ent, 0, fileinfo);
		break;
	case 'p':		/*Add by SmallPig */
		sread(4, 0, ent, 0, fileinfo);
		break;
	case Ctrl('A'):	/*Add by SmallPig */
		clear();
		show_author(0, fileinfo, NULL);
		return READ_NEXT;
		break;
	case 'C':
		friend_author(0, fileinfo, NULL);
		return READ_NEXT;
		break;
	case 'S':		/* by youzi */
		if (!HAS_PERM(PERM_PAGE, currentuser))
			break;
		clear();
		s_msg();
		break;
	case Ctrl('D'):	/*by yuhuan for deny anonymous */
		if (!IScurrBM)
			break;
		clear();
		deny_from_article(ent, fileinfo, direct);
		break;
	case Ctrl('Y'):
		zmodem_sendfile(ent, fileinfo, direct);
		clear();
		break;
	case '#':                                                                			topfile_post(ent, fileinfo, direct);                             			break;
	default:
		break;
	}
#endif
	return FULLUPDATE;
}

/* 超级版面选择，add by macintosh 07.05.28 */
#define BBS_PAGESIZE (t_lines - 4)
extern int page, range;
int result[MAXBOARD];
int super_board_count;
int super_board_now=0;

static int fill_super_board_callback(struct boardmem *board, int curr_idx, va_list ap) {
	const char *searchname = va_arg(ap, const char *);
	int *total = va_arg(ap, int *);
	int *max = va_arg(ap, int *);
	int *res_array = va_arg(ap, int *);

	if (*total >= *max)
		return QUIT;

	if (board->header.filename[0] == '\0')
		return 0;

	if (hasreadperm(&(board->header))) {
		if (strstr2(board->header.filename, searchname) || strstr2(board->header.title, searchname) || strstr2(board->header.keyword, searchname)) {
			res_array[*total] = curr_idx;
			*total = *total + 1;
		}
	}

	return 0;
}
static int
fill_super_board(char *searchname, int result[], int max)
{
	int total=0;

	ythtbbs_cache_Board_foreach_v(fill_super_board_callback, searchname, &total, &max, result);

	return total;
}

static int
sb_show()
{
	int i;
	struct boardmem *bptr;
	struct boardheader *ptr;
	char tmpBM[IDLEN + 1], buf[STRLEN];

	setlistrange(super_board_count);
	if (range < 1)
		return -1;
	clrtoeol();
	for (i = 0; i < BBS_PAGESIZE && i + page < range; i++) {
		move(i+3, 0);
		bptr = ythtbbs_cache_Board_get_board_by_idx(result[i]);
		ptr = &(bptr->header);

		prints(" %5d  ", bptr->total);
		strncpy(tmpBM, bptr->header.bm[0], IDLEN);
		sprintf(buf, "[%s] %s", bptr->header.type, bptr->header.title);
		if (ptr->level & PERM_POSTMASK)
			memcpy(buf, "[只读]", 6);
		prints("%-16s%s%-28s   %-12s%4d %6d\n", ptr->filename,
				(ptr->flag & VOTE_FLAG) ? "\033[1;31mV\033[m" : " ",
				buf, (tmpBM[0] == '\0' ? "诚征版主中" : tmpBM),
				bptr->inboard, bptr->score);
	}
	clrtobot();
	update_endline();
	return 0;
}

static int
sb_key(int key, int allnum, int pagenum)
{
	switch(key){
	default:
		break;
	}
	return 0;
}

static void
sb_refresh()
{
	docmdtitle("[超级版面选择]", "退出[\x1b[1;32m←\x1b[0;37m] 选择[\x1b[1;32m↑\x1b[0;37m,\x1b[1;32m↓\x1b[0;37m] 进入版面[\x1b[1;32mENTER\x1b[0;37m]");
	move(2, 0);
	prints("\033[1;44;37m %s 讨论区名称      V 类别  %-21s   版  主      在线   人气\033[m\n", " 全部 ", "中  文  叙  述");
	clrtoeol();
	update_endline();
}

static int
sb_select(int page, int number)
{
	super_board_now = page * BBS_PAGESIZE + number;
	return -1;
}

int
super_select_board(char *bname)
{
	char searchname[64], buf[64];//关键字就给了64bytes.....
	int ret;

	clear();
	bname[0]='\0';
	move(1, 0);
	prints("\033[1;31m在这里可以输入版面中文名称/英文名称/版面关键字进行搜索，支持模糊搜索。\033[m\n"
		"\033[1;31m例如，输入“铁路”“车迷”，均可定位至traffic版。\033[m");
	getdata(4, 0, "搜索版面关键字: ", buf, 64, DOECHO, YEA);
	strcpy(searchname, ytht_strtrim(buf));
	if (searchname[0] == '\0')
		return -1;
	if ((super_board_count = fill_super_board(searchname, result, MAXBOARD)) <= 0){
		move(5, 0);
		prints("没有找到任何相关版面\n");
		pressanykey();
		return -1;
	}
	super_board_now=0;
	ret = choose(NA, 0, sb_refresh, sb_key, sb_show, sb_select);
	if (ret < 0){
		strcpy(bname, currboard);
		return -1;
	}
	if (super_board_now >= 0){
		const struct boardmem*bp;
		bp=ythtbbs_cache_Board_get_board_by_idx(result[super_board_now]);
		if (bp==NULL){
			bname[0]='\0';
			return -1;
		}else
			strcpy(bname, bp->header.filename);
	}
	return 0;
}

static int
do_select(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char bname[STRLEN], bpath[STRLEN];
	struct stat st;
	int ret;
	struct boardmem *board;

	if (currentuser.dietime || inprison)
		return DONOTHING;
	if (digestmode == 4 || digestmode == 5)
		return DONOTHING;	//by ylsdd
	move(0, 0);
	clrtoeol();
	prints("选择讨论区 [ \033[1;32m# \033[0;37m- \033[1;31m版面名称/关键字搜索\033[0;37m, \033[1;32mSPACE \033[0;37m- 自动补全, \033[1;32mENTER \033[0;37m- 退出 ] \033[m\n");
	prints("输入讨论区名 (英文字母大小写皆可): ");
	clrtoeol();

	make_blist();
	if((ret=namecomplete((char *) NULL, bname))=='#') //super_select_board
		super_select_board(bname);
	setbpath(bpath, bname);
	if (*bname == '\0')
		return FULLUPDATE;
	if (stat(bpath, &st) == -1) {
		move(2, 0);
		prints("不正确的讨论区.\n");
		pressreturn();
		return FULLUPDATE;
	}
	if (!(st.st_mode & S_IFDIR)) {
		move(2, 0);
		prints("不正确的讨论区.\n");
		pressreturn();
		return FULLUPDATE;
	}
	if (!clubsync(bname)) {
		move(2, 0);
		prints("不正确的讨论区.\n");
		pressreturn();
		return FULLUPDATE;
	}

	board = ythtbbs_cache_Board_get_board_by_idx(uinfo.curboard - 1);
	if (uinfo.curboard && board->inboard > 0) {
		board->inboard--;
	}
	uinfo.curboard = getbnum(bname);
	update_utmp();
	if (uinfo.curboard)
		board->inboard++;
	selboard = 1;
	brc_initial(bname, 0);
	strcpy(currboard, bname);

	move(0, 0);
	clrtoeol();
	move(1, 0);
	clrtoeol();
	digestmode = 0;
	if (direct) {
		setbdir(direct, currboard, digestmode);
		if (DEFINE(DEF_FIRSTNEW, currentuser)) {
			int tmp;
			if (getkeep(direct, -1, 0) == NULL) {
				tmp = unread_position(direct, board);
				page = tmp - t_lines / 2;
				getkeep(direct, page > 1 ? page : 1, tmp + 1);
			}
		}
	}
	return NEWDIRECT;
}

static int
dele_digest(filetime, direc)
int filetime;
char *direc;
{
	char digest_name[STRLEN];
	char new_dir[STRLEN];
	int tmpcurrfiletime;
	struct fileheader fh;
	int pos;
	sprintf(digest_name, "G.%d.A", filetime);
	directfile(new_dir, direc, DIGEST_DIR);
	tmpcurrfiletime = currfiletime;
	currfiletime = filetime;
	pos = new_search_record(new_dir, &fh, sizeof (fh), (void *) cmpfilename, NULL);
	if (pos <= 0)
		return 0;
	delete_file(new_dir, sizeof (struct fileheader), pos, (void *) cmpfilename);
	currfiletime = tmpcurrfiletime;
	directfile(new_dir, direc, digest_name);
	unlink(new_dir);
	return 0;
}

int water_post(int ent, struct fileheader *fileinfo, char *dirent)
{
	if (!IScurrBM) {
		return DONOTHING;
	}

	if (fileinfo->accessed & FH_ISWATER) {
		snprintf(genbuf, 256, "%s unwater %s %s %s",
			currentuser.userid, currboard,
			fh2owner(fileinfo), fileinfo->title);
	} else {
		snprintf(genbuf, 256, "%s water %s %s %s",
			currentuser.userid, currboard,
			fh2owner(fileinfo), fileinfo->title);
	}

	newtrace(genbuf);
	change_dir(dirent, fileinfo, (void *) DIR_do_water, ent, digestmode, 0);
	if (!bmy_board_is_system_board(currboard) && fileinfo->thread == fileinfo->filetime) {
		bmy_article_update_thread_accessed(ythtbbs_cache_Board_get_idx_by_name(currboard) + 1, fileinfo->thread, fileinfo->accessed);
	}
	return PARTUPDATE;
}

int
digest_post(ent, fhdr, direct)
int ent;
struct fileheader *fhdr;
char *direct;
{

	if (!IScurrBM) {
		return DONOTHING;
	}
	if (digestmode != NA)
		return DONOTHING;

	if (fhdr->accessed & FH_DIGEST) {
		dele_digest(fhdr->filetime, direct);
		snprintf(genbuf, 256, "%s undigest %s %s %s",
			currentuser.userid, currboard, fhdr->owner,
			fhdr->title);
		newtrace(genbuf);
	} else {
		struct fileheader digest;
		char digestdir[STRLEN], digestfile[STRLEN], oldfile[STRLEN];
		directfile(digestdir, direct, DIGEST_DIR);
		if (get_num_records(digestdir, sizeof (digest)) > MAX_DIGEST) {
			move(3, 0);
			clrtobot();
			move(4, 10);
			prints("抱歉，你的文摘文章已经超过 %d 篇，无法再加入...\n", MAX_DIGEST);
			pressanykey();
			return PARTUPDATE;
		}
		digest = *fhdr;
		digest.accessed |= FH_ISDIGEST;
		directfile(digestfile, direct, fh2fname(&digest));
		directfile(oldfile, direct, fh2fname(fhdr));
		if (!dashf(digestfile)) {
			digest.accessed = FH_ISDIGEST;
			link(oldfile, digestfile);
			append_record(digestdir, &digest, sizeof (digest));
			snprintf(genbuf, 256, "%s digest %s %s %s",
				currentuser.userid, currboard, fhdr->owner,
				fhdr->title);
			newtrace(genbuf);
		}
	}
	change_dir(direct, fhdr, (void *) DIR_do_digest, ent, digestmode, 0);
	if (!bmy_board_is_system_board(currboard) && fhdr->thread == fhdr->filetime) {
		bmy_article_update_thread_accessed(ythtbbs_cache_Board_get_idx_by_name(currboard) + 1, fhdr->thread, fhdr->accessed);
	}
	return PARTUPDATE;
}



#ifndef NOREPLY
int
do_reply(fh)
struct fileheader *fh;
{
	if (fh->accessed & FH_INND || strchr(fh->owner, '.'))
		local_article = 0;
	else
		local_article = 1;
	post_article(fh);
	return FULLUPDATE;
}
#endif

static int
garbage_line(str)
char *str;
{
	int qlevel = 0;
	while (*str == ':' || *str == '>') {
		str++;
		if (*str == ' ')
			str++;
		if (qlevel++ >= 1)
			return 1;
	}
	while (*str == ' ' || *str == '\t')
		str++;
	if (qlevel >= 1)
		if (strstr(str, "提到:\n") || strstr(str, ": 】\n") || strncmp(str, "==>", 3) == 0 || strstr(str, "的文章 □"))
			return 1;
	return (*str == '\n');
}

int
transferattach(char *buf, size_t size, FILE * fp, FILE * fpto)
{
	char ch;
	size_t len, n;

	if (!strncmp(buf, "begin 644 ", 10)) {
		fwrite(buf, 1, strlen(buf), fpto);
		while (fgets(buf, size, fp) != NULL) {
			fwrite(buf, 1, strlen(buf), fpto);
			if (!strncmp(buf, "end", 3))
				break;
		}
		return 1;
	}
	if (!strncmp(buf, "beginbinaryattach ", 18)) {
		fwrite(buf, 1, strlen(buf), fpto);
		fread(&ch, 1, 1, fp);
		if (ch != 0) {
			ungetc(ch, fp);
			return 0;
		}
		fwrite(&ch, 1, 1, fpto);
		fread(&len, 4, 1, fp);
		fwrite(&len, 4, 1, fpto);
		len = ntohl(len);
		while (len > 0) {
			n = (len > size) ? size : len;
			n = fread(buf, 1, n, fp);
			if (n <= 0) {
				fseek(fpto, len, SEEK_CUR);
				break;
			}
			fwrite(buf, 1, n, fpto);
			len -= n;
		}
		return 1;
	}
	return 0;
}

static int
skipattach(char *buf, int size, FILE * fp)
{
	if (!strncmp(buf, "begin 644 ", 10)) {
		while (fgets(buf, size, fp) != NULL)
			if (!strncmp(buf, "end", 3))
				break;
		return 1;
	}
	if (!strncmp(buf, "beginbinaryattach ", 18)) {
		char ch;
		unsigned int len;
		fread(&ch, 1, 1, fp);
		if (ch != 0) {
			ungetc(ch, fp);
			return 0;
		}
		fread(&len, 4, 1, fp);
		len = ntohl(len);
		fseek(fp, len, SEEK_CUR);
		return 1;
	}
	return 0;
}

/* When there is an old article that can be included -jjyang */
void
do_quote(filepath, quote_mode)
char *filepath;
char quote_mode;
{
	FILE *inf, *outf;
	char *qfile, *quser;
	char buf[256], *ptr;
	char op;
	int bflag;
	int line_count = 0;
	int attach;
	qfile = quote_file;
	quser = quote_user;
	bflag = strncmp(qfile, "mail", 4);
	outf = fopen(filepath, "w");
	if (*qfile != '\0' && (inf = fopen(qfile, "r")) != NULL) {
		op = quote_mode;
		if (op != 'N') {
			fgets(buf, 256, inf);
			if ((ptr = strrchr(buf, ')')) != NULL) {
				ptr[1] = '\0';
				if ((ptr = strchr(buf, ':')) != NULL) {
					quser = ptr + 1;
					while (*quser == ' ')
						quser++;
				}
			}

			if (bflag)
				fprintf(outf,
					"\n【 在 %-.55s 的大作中提到: 】\n",
					quser);
			else
				fprintf(outf,
					"\n【 在 %-.55s 的来信中提到: 】\n",
					quser);
			attach = 0;
			if (op == 'A') {
				while (fgets(buf, 256, inf) != NULL) {
					if (skipattach(buf, sizeof (buf), inf))
						continue;
					fprintf(outf, ": %s", buf);
				}
			} else if (op == 'R') {
				while (fgets(buf, 256, inf) != NULL)
					if (buf[0] == '\n')
						break;
				while (fgets(buf, 256, inf) != NULL) {
					if (skipattach(buf, sizeof (buf), inf))
						continue;
					if (Origin2(buf))
						continue;
					fprintf(outf, "%s", buf);
				}
			} else if (op == 'Y') {

				while (fgets(buf, 256, inf) != NULL)
					if (buf[0] == '\n')
						break;
				while (fgets(buf, 256, inf) != NULL) {
					if (skipattach(buf, sizeof (buf), inf))
						continue;
					if (strcmp(buf, "--\n") == 0)
						break;
					if (buf[250] != '\0')
						strcpy(buf + 250, "\n");
					if (!garbage_line(buf))
						fprintf(outf, ": %s", buf);
				}
			} else {
				while (fgets(buf, 256, inf) != NULL)
					if (buf[0] == '\n')
						break;
				while (fgets(buf, 256, inf) != NULL) {
					if (strcmp(buf, "--\n") == 0)
						break;
					if (skipattach(buf, sizeof (buf), inf))
						continue;
					if (buf[250] != '\0')
						strcpy(buf + 250, "\n");
					if (!garbage_line(buf))
						fprintf(outf, ": %s", buf);
					line_count++;
					if (line_count > 10) {
						fprintf(outf,
							": ...................");
						break;
					}
				}
			}
		}
		fprintf(outf, "\n");
		fclose(inf);
	}
	*quote_file = '\0';
	*quote_user = '\0';
	if (!(currentuser.signature == 0 || header.chk_anony == 1)) {
		addsignature(outf, 1);
	}
	fclose(outf);
}

/* Add by SmallPig */
static void
getcross(filepath, mode)
char *filepath;
int mode;
{
	FILE *inf, *of;
	char buf[256],oritime[256],temptime[256];
	char owner[248];
	int count,i,j,mark=0;
	time_t now;
	int hashead = 1;
	now = time(NULL);
	inf = fopen(quote_file, "r");
	of = fopen(filepath, "w");
	if (inf == NULL || of == NULL) {
		errlog("Cross Post error");

		if (inf) fclose(inf);
		if (of)  fclose(of);

		return;
	}
	if (mode == 0) {
		if (in_mail == YEA) {
			in_mail = NA;
			write_header(of, 1 /*不写入 .posts */ );
			in_mail = YEA;
		} else
			write_header(of, 1 /*不写入 .posts */ );
		if (fgets(buf, 256, inf) != NULL) {
			if (in_mail && strncmp(buf, "寄信人: ", 8)) {
				hashead = 0;
				strcpy(owner, currentuser.userid);
			} else {
				for (count = 8; buf[count] != ' ' && buf[count] != '\n' && buf[count] != '\0'; count++)
					owner[count - 8] = buf[count];
				owner[count - 8] = '\0';
			}
		}

		// read original time
		fgets(temptime,256,inf);
		if(fgets(temptime,256,inf)!=NULL)
		{
			if(!strncmp(temptime,"发信站: ",8))
			{
				mark=1;
				for(i=0;temptime[i]!='('&&temptime[i]!='\n'&&temptime[i]!='\0';i++);
				if(temptime[i]=='(')
				{
					i++;
					for(j=0;temptime[i]!=')'&&temptime[i]!='\n'&&temptime[i]!='\0';i++,j++)
						oritime[j]=temptime[i];
					oritime[j]='\0';
				}
			}
		}
		//

		if (in_mail == YEA)
			fprintf(of,
				"\033[1;37m【 以下文字转载自 \033[32m%s \033[37m的信箱 】\n",
				currentuser.userid);
		else
			fprintf(of,
				"\033[1;37m【 以下文字转载自 \033[32m%s \033[37m讨论区 】\n",
				quote_board);
		if(mark)
			fprintf(of, "【 原文由\033[32m %s \033[37m于\033[32m %s \033[37m发表】\033[m\n", owner,oritime);
		else
			fprintf(of, "【 原文由\033[32m %s \033[37m发表】\033[m\n", owner);

		if (hashead) {
			while (fgets(buf, 256, inf) != NULL)	/*Clear Post header */
				if (buf[0] == '\n' || buf[0] == '\r')
					break;
		} else {
			fseek(inf, 0, SEEK_SET);
		}
	} else if (mode == 1) {
		fprintf(of, "发信人: XJTU-XANET (自动发信系统), 信区: %s\n",
			quote_board);
		fprintf(of, "标  题: %s\n", quote_title);
		fprintf(of,
			"发信站: %s自动发信系统 (%24.24s)\n\n",
			MY_BBS_NAME, ctime(&now));
		fprintf(of, "【此篇文章是由自动发信系统所张贴】\n\n");
	} else if (mode == 2) {
		write_header(of, 0 /*写入 .posts */ );
	}

	while ((count = fread(buf, 1, sizeof (buf), inf)) > 0)
		fwrite(buf, 1, count, of);
	fclose(inf);
	fclose(of);
	*quote_file = '\0';
}

int do_post() {
	*quote_file = '\0';
	*quote_user = '\0';
	return post_article(NULL);
}

/* Add by SmallPig */
static int
post_cross(char *bname, int mode, int islocal, int hascheck, int dangerous)
{
	struct fileheader postfile;
	char filepath[STRLEN], fname[STRLEN];
	char buf[256], buf4[STRLEN], whopost[IDLEN + 2];
	char bkcurrboard[STRLEN];
	int fp, count, ddigestmode;
	time_t now;
	if (!haspostperm(bname) && !mode && strcasecmp(bname, "AnonyLog")!=0) {
		move(1, 0);
		prints("您尚无权限在 %s 发表文章.\n", bname);
		return -1;
	}
	if (noadm4political(bname) && !mode && strcasecmp(bname, "AnonyLog")!=0) {
		move(1, 0);
		prints("对不起,因为没有版面管理人员在线,本版暂时封闭.");
		return -1;
	}
	bzero(&postfile, sizeof (postfile));
	now = time(NULL);
	sprintf(fname, "M.%ld.A", now);
	if (!mode) {
		if (!strstr(quote_title, "[转载]"))
			sprintf(buf4, "[转载] %.70s", quote_title);
		else
			strcpy(buf4, quote_title);
	} else
		strcpy(buf4, quote_title);
	strncpy(save_title, buf4, STRLEN);
	save_title[STRLEN - 1] = 0;
	setbfile(filepath, bname, fname);
	count = 0;
	while ((fp = open(filepath, O_CREAT | O_EXCL | O_WRONLY, 0660)) == -1) {
		now++;
		sprintf(fname, "M.%ld.A", now);
		setbfile(filepath, bname, fname);
		if (count++ > MAX_POSTRETRY) {
			return -1;
		}
	}
	close(fp);
	postfile.filetime = now;
	postfile.thread = postfile.filetime;
	if ((!islocal) & (mode != 1)) {
		postfile.accessed |= FH_INND;
		local_article = 0;
	} else {
		local_article = 1;
	}

	if (mode == 1) //here&following 3 line by bjgyt
		strcpy(whopost, "XJTU-XANET");
	else {
		if(strcasecmp(bname, "AnonyLog")==0) {
			ytht_strsncpy(whopost, quote_user, sizeof(whopost));
		} else {
			ytht_strsncpy(whopost, currentuser.userid, sizeof(whopost));
		}
	}

	if (mode == 1 && strcmp(bname, "millionaires") != 0 && strncmp(bname, "BM_exam", 7) != 0)
		postfile.accessed |= FH_MARKED;

	ytht_strsncpy(postfile.owner, whopost, sizeof(postfile.owner));
	setbfile(filepath, bname, fname);
	modify_user_mode(POSTING);
	strcpy(bkcurrboard, currboard);
	strcpy(currboard, bname);
	getcross(filepath, mode);
	strcpy(currboard, bkcurrboard);
	postfile.sizebyte = ytht_num2byte(eff_size(filepath));
	ytht_strsncpy(postfile.title, save_title, sizeof(postfile.title));

	if (mode != 1) {
		if (!hascheck)
			dangerous = dofilter(postfile.title, filepath, political_board(bname));
		if (dangerous) {
			char mtitle[256];
			snprintf(mtitle, sizeof (mtitle), "[转载报警] %s %.60s", bname, postfile.title);
			mail_file(filepath, "delete", mtitle);
			updatelastpost("deleterequest");
			postfile.accessed |= FH_DANGEROUS;
		}
	}

	int i = strlen(postfile.title) - 1;
	while (i > 0 && isspace(postfile.title[i]))
		postfile.title[i--] = 0;

	ddigestmode = digestmode;
	digestmode = 0;
	setbdir(buf, bname, digestmode);
	digestmode = ddigestmode;
	if (append_record(buf, &postfile, sizeof (postfile)) == -1) {
		if (!mode) {
			errlog("cross_posting '%s' on '%s': append_record failed!", postfile.title, quote_board);
		} else {
			errlog("Posting '%s' on '%s': append_record failed!", postfile.title, quote_board);
		}
		pressreturn();
		clear();
		return (int)now; // return filetime instead of 1 by IronBlood 20130807
	}
	outgo_post(&postfile, bname, currentuser.userid, currentuser.username);
	updatelastpost(bname);
	if (!mode) {
		add_crossinfo(filepath, 1);
		sprintf(buf, "%s crosspost %s %s", currentuser.userid, bname, postfile.title);
		newtrace(buf);

		bmy_article_add_thread(ythtbbs_cache_Board_get_idx_by_name(bname) + 1, postfile.thread, postfile.title, currentuser.username, postfile.accessed);
	}
	return (int)now;  // return filetime instead of 1 by IronBlood 20130807
}

void
add_loginfo(filepath)
char *filepath;
{
	FILE *fp;
	int color, noidboard;
	char fname[STRLEN];
	noidboard = header.chk_anony;
	color = (currentuser.numlogins % 7) + 31;
	sethomefile_s(fname, sizeof(fname), currentuser.userid, "signatures");
	if ((fp = fopen(filepath, "a")) == NULL)
		return;
	if (!dashf(fname) || currentuser.signature == 0 || noidboard)
		fputs("\n--", fp);
	fprintf(fp,
		"\n\033[m\033[1;%2dm※ 来源:．%s %s．[FROM: %-.40s]\033[m\n",
		color, MY_BBS_NAME, email_domain(),
		(noidboard) ? "匿名天使的家" : fromhost);
	fclose(fp);
	return;
}

void
add_crossinfo(filepath, mode)
char *filepath;
int mode;
{
	FILE *fp;
	int color;
	color = (currentuser.numlogins % 7) + 31;
	if ((fp = fopen(filepath, "a")) == NULL)
		return;
	fprintf(fp,
		"--\n\033[m\033[1;%2dm※ 转%s:．%s %s．[FROM: %-.40s]\033[m\n",
		color, (mode == 1) ? "载" : "寄",
		MY_BBS_NAME, email_domain(), fromhost);
	fclose(fp);
	return;
}

int
show_board_notes(bname)
char bname[30];
{
	char buf[256];
	move(2, 0);
	sprintf(buf, "vote/%s/notes", bname);
	if (dashf(buf)) {
		ansimore2stuff(buf, NA, 2, 24);
		return 1;
	} else if (dashf("vote/notes")) {
		ansimore2stuff("vote/notes", NA, 2, 24);
		return 1;
	}
	return -1;
}

static int
post_article(struct fileheader *sfh)
{
	struct fileheader postfile;
	char filepath[STRLEN], buf[STRLEN], edittmp[STRLEN];
	int aborted, mailback = 0;
	time_t t;
	char *replytitle;
	if (sfh == NULL)
	{
		replytitle = NULL;
		if (innd_board (currboard)) // added by linux @ 2006.6.6 for the post status to no outgo by default
			local_article = 1;
	}
	else
	{
		replytitle = sfh->title;
		if (sfh->accessed & FH_MAILREPLY)
			mailback = 1;
	}
	modify_user_mode(POSTING);
	if (! ( /**/ (strcmp(currboard, "welcome") == 0 || strcmp(currboard, "KaoYan") == 0) /**/ && strcmp(currentuser.userid, "guest") == 0))
	{
		if (!haspostperm(currboard))
		{
			move(3, 0);
			clrtobot();

			if (digestmode == NA)
			{
				if (ythtbbs_cache_Board_get_board_by_idx(getbnum(currboard) - 1)->header.secnumber2 == 'C')
					prints
					("\n\n     俱乐部版面，请联系版主，申请加入俱乐部方能发文.");
				else
					prints
					("\n\n        此讨论区是唯读的, 或是您尚无权限在此发表文章。");
			}
			else
			{
				prints
				("\n\n     目前是文摘或主题模式, 所以不能发表文章 (按左键可离开此模式)。");
			}
			pressreturn();
			clear();
			return FULLUPDATE;
		}
		if (noadm4political(currboard))
		{
			move(3, 0);
			clrtobot();
			prints
			("\n\n               对不起,因为没有版面管理人员在线,本版暂时封闭.");
			pressreturn();
			clear();
			return FULLUPDATE;
		}
		if (deny_me(currboard) && !HAS_PERM(PERM_SYSOP, currentuser))
		{
			move(3, 0);
			clrtobot();
			prints
			("\n\n                 很抱歉，你被版主停止 POST 的权利。");
			pressreturn();
			clear();
			return FULLUPDATE;
		}
		if (deny_me_global()
				&& strcmp(currboard, "sysop") && strcmp(currboard, "committee")
				&& strcmp(currboard, "welcome") && strcmp(currboard, "KaoYan")
				&& strcmp(currboard, "Appeal") && !HAS_PERM(PERM_SYSOP, currentuser))
		{
			move(3, 0);
			clrtobot();
			prints
			("\n\n                 很抱歉，你被站务停止全站 POST 的权利。");
			pressreturn();
			clear();
			return FULLUPDATE;
		}
	}
	else if (seek_in_file(MY_BBS_HOME"/etc/guestbanip", fromhost))
	{
		move(3, 0);
		clrtobot();
		prints
		("\n\n                 很抱歉，你的ip被禁止使用guest在本版发文。");
		pressreturn();
		clear();
		return FULLUPDATE;
	}

	bzero(&postfile, sizeof (postfile));
	clear();
	if (!innd_board(currboard))
		local_article = -1;
	do_delay(1);		/*by ylsdd */
	show_board_notes(currboard);
#ifndef NOREPLY
	if (replytitle != NULL)
	{
		if (strncasecmp(replytitle, "Re:", 3) == 0)
			strcpy(header.title, replytitle);
		else
			snprintf(header.title, STRLEN - 1, "Re: %s",
					replytitle);
		header.reply_mode = 1;
	}
	else
#endif

	{
		header.title[0] = '\0';
		header.reply_mode = 0;
	}
	strcpy(header.ds, currboard);
	header.postboard = YEA;
	{
		int i = strlen(header.title) - 1;
		while (i > 0 && isspace(header.title[i]))
			header.title[i--] = 0;
	}
	header.mailreply = 0; //no reply mail to author by defauit , by macintosh
	if (post_header(&header))
	{
		ytht_strsncpy(postfile.title, header.title, sizeof(postfile.title));
		ytht_strsncpy(save_title, postfile.title, sizeof(save_title));
	}
	else
	{
		do_delay( -1);	/* by ylsdd */
		return FULLUPDATE;
	}
	setbfile(filepath, currboard, "");
	t = trycreatefile(filepath, "M.%ld.A", now_t, 100);
	if (t < 0)
		return -1;
	postfile.filetime = t;
	if (sfh != NULL)
		postfile.thread = sfh->thread;
	fh_setowner(&postfile, currentuser.userid, header.chk_anony);
	modify_user_mode(POSTING);
	sprintf(edittmp, MY_BBS_HOME "/bbstmpfs/tmp/%s.%d", currentuser.userid,
			getpid());
	do_quote(edittmp, header.include_mode);
	aborted = vedit(edittmp, YEA, YEA);

	/*Anony=0; */ /*Inital For ShowOut Signature */
	if (aborted == -1)
	{
		unlink(edittmp);
		clear();
		do_delay( -1);	/* by ylsdd */
		/*        pressreturn() ;*/
		return FULLUPDATE;
	}
	add_loginfo(edittmp);
	postfile.sizebyte = ytht_num2byte(eff_size(edittmp));
	crossfs_rename(edittmp, filepath);

	if (local_article == 0)
		postfile.accessed |= FH_INND;
	ytht_strsncpy(postfile.title, save_title, sizeof(postfile.title));

	if (header.canreply == 0)
		postfile.accessed |= FH_NOREPLY;

	if (header.mailreply == 1)
		postfile.accessed |= FH_MAILREPLY;

	if (!hideboard(currboard))
	{
		int dangerous;
		dangerous =
			dofilter(postfile.title, filepath,
					political_board(currboard));
		switch (dangerous)
		{
			char mtitle[256];
		case - 1:
			post_to_1984(filepath, &postfile, 0);
			unlink(filepath);
			clear();
			do_delay( -1);
			return FULLUPDATE;
		case -2:
			snprintf(mtitle, sizeof (mtitle), "[发表报警] %s %.60s",
					currboard, postfile.title);
			mail_file(filepath, "delete", mtitle);
			updatelastpost("deleterequest");
			postfile.accessed |= FH_DANGEROUS;
			break;
		default:
			break;
		}
	}
	if (is1984_board(currboard))
	{
		post_to_1984(filepath, &postfile, 0);
		unlink(filepath);
		if (!junkboard())
		{
			set_safe_record();
			currentuser.numposts++;
			substitute_record(PASSFILE, &currentuser,
					sizeof (currentuser), usernum);
		}
		move(0, 0);
		clear();
		prints("%s", DO1984_NOTICE);
		pressanykey();
		return FULLUPDATE;
	}

	/*重新指定文件名 */

	char newfilepath[STRLEN], newfname[STRLEN];
	int count;
	t = time(NULL);
	count = 0;
	while (1)
	{
		sprintf(newfname, "M.%ld.A", t);
		setbfile(newfilepath, currboard, newfname);
		if (link(filepath, newfilepath) == 0)
		{
			unlink(filepath);
			postfile.filetime = t;
			break;
		}
		t++;
		if (count++ > MAX_POSTRETRY)
			break;
	}


	if (sfh == NULL)
		postfile.thread = postfile.filetime;
	setbdir(buf, currboard, digestmode);
	if (append_record(buf, &postfile, sizeof (postfile)) == -1)
	{
		errlog("posting '%s' on '%s': append_record failed!", postfile.title, currboard);
		pressreturn();
		clear();
		return FULLUPDATE;
	}
	if (local_article == 0)
		outgo_post(&postfile, currboard, currentuser.userid, currentuser.username);
	local_article = 0;
	SETREAD(&postfile, &brc);
	//    if(strcmp(currboard,"triangle")==0) checksomewords();
	updatelastpost(currboard);
	snprintf(genbuf, 256, "%s post %s %s",
			currentuser.userid, currboard, postfile.title);
	genbuf[256] = 0;
	newtrace(genbuf);
	if (!junkboard())
	{
		set_safe_record();
		currentuser.numposts++;
		substitute_record(PASSFILE, &currentuser,
				sizeof (currentuser), usernum);
	}
	if (mailback)
	{
		char copyfrom[STRLEN * 2], replyto[IDLEN + 2];
		snprintf(copyfrom, sizeof(copyfrom), "boards/%s/M.%ld.A", currboard, postfile.filetime);
		if (sfh->owner[0] == 0)
			strcpy(replyto, sfh->owner + 1);
		else
			strcpy(replyto, sfh->owner);
		mail_file(copyfrom, replyto, postfile.title);
	}
	// term 下回帖提醒开始 by IronBlood
	// 逻辑过程与 mailback 相同
	char notito[IDLEN+2];
	if(sfh!=NULL) {
		if (sfh->owner[0] == 0)
			strcpy(notito, sfh->owner+1);
		else
			strcpy(notito, sfh->owner);
		if(strcmp(notito, currentuser.userid) != 0) {
			add_post_notification(notito, (header.chk_anony) ? "Anonymous" : currentuser.userid,
								currboard, postfile.filetime, postfile.title);
		}
	}
	// term 下回帖提醒结束

	// term 下 @ 提醒开始 by IronBlood
	char mention_ids[MAX_MENTION_ID][IDLEN+2];
	memset(mention_ids, 0, MAX_MENTION_ID*(IDLEN+2));
	get_mention_ids(newfilepath, (char **)mention_ids);

	int i=0;
	while(i!=MAX_MENTION_ID && mention_ids[i][0]!=0) {
		if(strcasecmp(currentuser.userid, mention_ids[i])!=0) {
			if(hasreadperm_ext(mention_ids[i], currboard)) {	// 该函数内调用了 getuser() 函数，因此可以直接使用 lookupuser 全局变量
				// 用户存在的情况下，且不为当前用户的情况下，且拥有该版面阅读权限的情况下，且不在黑名单里的时候
				if (!ythtbbs_override_included(lookupuser.userid, YTHTBBS_OVERRIDE_REJECTS, currentuser.userid))
					add_mention_notification(lookupuser.userid, (header.chk_anony) ? "Anonymous" : currentuser.userid,
							currboard, postfile.filetime, postfile.title);
			}
		}
		++i;
	}
	// term 下 @ 提醒结束

	if (!bmy_board_is_system_board(currboard)) {
		if (sfh != NULL) {
			bmy_article_add_comment(ythtbbs_cache_Board_get_idx_by_name(currboard) + 1, sfh->thread);
		} else {
			bmy_article_add_thread(ythtbbs_cache_Board_get_idx_by_name(currboard) + 1, postfile.thread, postfile.title, postfile.owner, postfile.accessed);
		}
	}

	return FULLUPDATE;
}

static int
get_mention_ids(char *article_path, char **mention_ids)
{
	int fd;
	char *p, *s;
	struct stat statbuf;

	fd = open(article_path, O_RDONLY);
	if(fd == -1)
		return -1;
	if(fstat(fd, &statbuf) == -1) {
		close(fd);
		return -1;
	}

	p = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);

	if(p == MAP_FAILED)
		return -2;

	s = strstr(p, "\n\n");	// 跳过头部
	if(s!=NULL)
		parse_mentions(s, mention_ids, 0);

	munmap(p, statbuf.st_size);
	return 0;
}

static int
dofilter(title, fn, mode)
char *title, *fn;
int mode;			// 1: politics    0: non-politics
{
	//this function return 0 if article is safe, return -1 if article is dangerous
	//return -2 if article is possbily dangerous.
	struct mmapfile *mf;
	char *bf;
	switch (mode) {
	case 1:
		mf = &mf_badwords;
		bf = BADWORDS;
		break;
	case 0:
		mf = &mf_sbadwords;
		bf = SBADWORDS;
		break;
	case 2:
		mf = &mf_pbadwords;
		bf = PBADWORDS;
		break;
	default:
		return -1;
	}
	if (mmapfile(bf, mf) < 0)
		goto CHECK2;
	if (ytht_smth_filter_article(title, fn, mf)) {
		if (mode != 2) {
			move(0, 0);
			clear();
			prints("%s", BAD_WORD_NOTICE);
			pressanykey();
			mail_file(fn, currentuser.userid, title);
			return -1;
		}
		return -2;
	}
CHECK2:
	if (1 != mode)
		return 0;
	mf = &mf_pbadwords;
	bf = PBADWORDS;
	if (mmapfile(bf, mf) < 0)
		return 0;
	if (ytht_smth_filter_article(title, fn, mf))
		return -2;
	else
		return 0;
}

int
stringfilter(char *title, int mode)
{
	struct mmapfile *mf;
	char *bf;
	switch (mode) {
	case 1:
		mf = &mf_badwords;
		bf = BADWORDS;
		break;
	case 0:
		mf = &mf_sbadwords;
		bf = SBADWORDS;
		break;
	case 2:
		mf = &mf_pbadwords;
		bf = PBADWORDS;
		break;
	default:
		return -1;
	}
	if (mmapfile(bf, mf) < 0)
		return 0;
	if (ytht_smth_filter_string(title, mf)) {
		return -1;
	}
	return 0;
}

static int
change_content_title(fname, title)
char *fname;
char *title;
{
	FILE *fp, *out;
	char buf[256];
	char outname[STRLEN];
	if ((fp = fopen(fname, "r")) == NULL)
		return 0;
	sprintf(outname, "bbstmpfs/tmp/editpost.%s.%05d", currentuser.userid,
		uinfo.pid);
	if ((out = fopen(outname, "w")) == NULL) {
		fclose(fp);
		return 0;
	}
	while ((fgets(buf, 256, fp)) != NULL) {
		if (transferattach(buf, sizeof (buf), fp, out))
			continue;
		if (!strncmp(buf, "标  题: ", 8)) {
			fprintf(out, "标  题: %s\n", title);
			continue;
		}
		fputs(buf, out);
	}
	fclose(fp);
	fclose(out);
	copyfile(outname, fname);
	unlink(outname);
	return 0;
}

/*ARGSUSED*/
int edit_post(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	extern char currmaildir[STRLEN];
	char filepath[STRLEN];
	char tmpfile[STRLEN];
	char attach_path[256];
	if (!in_mail) {
		if (digestmode == 4 || digestmode == 5)
			return DONOTHING;
		if (!strcmp(currboard, "syssecurity"))
			return DONOTHING;
		if (!IScurrBM && !isowner(&currentuser, fileinfo))
			return DONOTHING;
		if (is1984_board(currboard))
			return DONOTHING;
	}

	if (in_mail){
		directfile(filepath, currmaildir, fh2fname(fileinfo));
		if (dashl(filepath))
			return DONOTHING;//信箱中ln过来的文件禁止编辑
	}else
		directfile(filepath, direct, fh2fname(fileinfo));
	modify_user_mode(EDIT);
	clear();
	if (!dashf(filepath)) {
		return FULLUPDATE;
	}
	sprintf(tmpfile, "boards/.tmp/editpost.%s.%05d", currentuser.userid,
		uinfo.pid);
	copyfile_attach(filepath, tmpfile);
	if (vedit(tmpfile, NA, NA) == -1) {
		unlink(tmpfile);
		return FULLUPDATE;
	}
	if (!in_mail && !hideboard(currboard)) {
		int dangerous = dofilter(fileinfo->title, tmpfile, political_board(currboard));
		switch (dangerous) {
			char mtitle[256];
		case -1:
			fileinfo->sizebyte = ytht_num2byte(eff_size(tmpfile));
			fileinfo->edittime = time(0);
			post_to_1984(tmpfile, fileinfo, 0);
			unlink(tmpfile);
			return FULLUPDATE;
		case -2:
			snprintf(mtitle, sizeof (mtitle), "[修改报警] %s %.60s", currboard, fileinfo->title);
			change_dir(direct, fileinfo, (void *) DIR_do_dangerous, ent, digestmode, 1);
			mail_file(tmpfile, "delete", mtitle);
			updatelastpost("deleterequest");
			break;
		default:
			break;
		}
	}
	snprintf(attach_path, sizeof (attach_path), PATHUSERATTACH "/%s", currentuser.userid);
	clearpath(attach_path);

	decode_attach(filepath, attach_path);
	insertattachments_byfile(filepath, tmpfile, currentuser.userid);
	unlink(tmpfile);
	fileinfo->sizebyte = ytht_num2byte(eff_size(filepath));
	fileinfo->edittime = time(0);
	change_dir(direct, fileinfo, (void *) DIR_do_edit, ent, digestmode, 1);
	if (!in_mail) {
		outgo_post(fileinfo, currboard, currentuser.userid, currentuser.username);
		updatelastpost(currboard);
		sprintf(genbuf, "%s edit %s %s %s",
			currentuser.userid, currboard,
			fh2owner(fileinfo), fileinfo->title);
		newtrace(genbuf);
		SETREAD(fileinfo, &brc);
	}
	if (ADD_EDITMARK)
		add_edit_mark(filepath, currentuser.userid, fileinfo->edittime, fromhost);
	/*有关置顶文章的修改 add by leoncom 2007.12.30 */
	char file_name[32];
	strcpy(file_name,fh2fname(fileinfo));
	if(file_name[0]=='T')                       //如果是置顶贴则生成原帖
	{
		char filepathtemp[STRLEN];
		strcpy(filepathtemp,filepath);
		char *temp;
		temp=strrchr(filepathtemp,'T');
		*temp='M';
		char command[STRLEN * 3];
		snprintf(command, sizeof(command), "rm %s", filepathtemp);
		system(command);
		snprintf(command, sizeof(command), "cp %s %s", filepath, filepathtemp);
		system(command);
	}
	else
	{
		char filepathtemp[STRLEN];
		strcpy(filepathtemp,filepath);
		char *temp;
		temp=strrchr(filepathtemp,'M');
		*temp='T';
		struct stat test_exist;
		if(stat(filepathtemp,&test_exist)==0)     //如果原帖存在置顶贴则重新生成置顶贴
		{
			char command[STRLEN * 3];
			snprintf(command, sizeof(command), "rm %s", filepathtemp);
			system(command);
			snprintf(command, sizeof(command), "cp %s %s", filepath, filepathtemp);
			system(command);
		}
	}
	return FULLUPDATE;
}

// 邮箱界面中也要用这个函数
int
edit_title(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct stat st; //add by hace
	char buf[STRLEN], filepath[STRLEN];
	int now;
	if(ent < 0 || stat(direct,&st)==-1 || (st.st_size/sizeof(struct fileheader) < (unsigned int /* safe */) ent))
		return DONOTHING;//add by hace
	if (!in_mail)
	{
		if (is1984_board(currboard))
			return DONOTHING;
		if (!IScurrBM)
		{
			if (digestmode == 4 || digestmode == 5 || !strcmp(currboard, "syssecurity"))
				return DONOTHING;
			if (!isowner(&currentuser, fileinfo))
				return DONOTHING;
		}
	} else{
		directfile(filepath, currmaildir, fh2fname(fileinfo));
		if (dashl(filepath))
			return DONOTHING;//信箱中ln过来的文件禁止编辑
	}

	ytht_strsncpy(buf, fileinfo->title, sizeof(buf));
	getdata(t_lines - 1, 0, "新文章标题: ", buf, 50, DOECHO, NA);

	if (buf[0] != '\0' && strcmp(fileinfo->title, buf) && (!stringfilter(buf, politics(currboard)))) {
		char str[300];
		sprintf(str,
			"%s changetitle %s %s oldtitle:%s newtitle:%s",
			currentuser.userid, currboard,
			fh2owner(fileinfo), fileinfo->title, buf);
		newtrace(str);

		if (!bmy_board_is_system_board(currboard) && fileinfo->filetime == fileinfo->thread && strcmp(fileinfo->title, buf) != 0) {
			bmy_article_update_thread_title(ythtbbs_cache_Board_get_idx_by_name(currboard) + 1, fileinfo->thread, buf);
		}

		ytht_strsncpy(fileinfo->title, buf, sizeof(fileinfo->title));
		directfile(genbuf, direct, fh2fname(fileinfo));
		change_content_title(genbuf, buf);
		now = time(NULL);
		add_edit_mark(genbuf, currentuser.userid, now, fromhost);
		change_dir(direct, fileinfo,
			(void *) DIR_do_changetitle, ent, digestmode, 1);
	}
	return PARTUPDATE;
}

int
mark_post(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	if (!IScurrBM) {
		return DONOTHING;
	}

	if (fileinfo->accessed & FH_MARKED) {
		snprintf(genbuf, 256, "%s unmark %s %s %s",
			currentuser.userid, currboard,
			fh2owner(fileinfo), fileinfo->title);
	} else {
		snprintf(genbuf, 256, "%s mark %s %s %s",
			currentuser.userid, currboard,
			fh2owner(fileinfo), fileinfo->title);
	}
	newtrace(genbuf);
//	if(HAS_PERM(PERM_COMMEND))
//		commend_article(currboard, fileinfo);
//	else
	change_dir(direct, fileinfo, (void *) DIR_do_mark, ent, digestmode, 0);
	if (!bmy_board_is_system_board(currboard) && fileinfo->thread == fileinfo->filetime) {
		bmy_article_update_thread_accessed(ythtbbs_cache_Board_get_idx_by_name(currboard) + 1, fileinfo->thread, fileinfo->accessed);
	}
	return PARTUPDATE;
}

int has_perm_commend(char* userid)			//add by mintbaggio 040406 for front page commend
{
	FILE* fp;
	char buf[IDLEN+2];
	int ret = 0;

	fp = fopen(MY_BBS_HOME"/etc/commendlist", "r");
	if(!fp){
		prints("fatal error, couldn't open commendlist, please contact the SYSOP\n");
		return -1;
	}
	while(fgets(buf, IDLEN+2, fp)){
		buf[strlen(buf)-1] = 0;                  //get rid of '\n'
		if(!strcmp(buf, userid))
			ret = 1;
	}
	fclose(fp);
	return ret;
}

//add by mintbaggio 040331 for front page commend
static int mark_commend(int ent, struct fileheader *fileinfo, char *direct) {
	if(!HAS_PERM(PERM_SYSOP, currentuser) && !has_perm_commend(currentuser.userid))
		return DONOTHING;
	commend_article(currboard, fileinfo);
	return PARTUPDATE;
}

//add by mintbaggio 040331 for front page commend
static int mark_commend2(int ent, struct fileheader *fileinfo, char *direct) {
	if(!HAS_PERM(PERM_SYSOP, currentuser) && !has_perm_commend(currentuser.userid))
		return DONOTHING;
	commend_article2(currboard, fileinfo);
	return PARTUPDATE;
}

int
markdel_post(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	if (!strcmp(currboard, "deleted") || !strcmp(currboard, "junk") || !IScurrBM)
		return DONOTHING;
	if (fileinfo->owner[0] == '-')
		return PARTUPDATE;
	change_dir(direct, fileinfo, (void *) DIR_do_markdel, ent, digestmode, 0);
	return PARTUPDATE;
}

int
mark_minus_del_post(ent, fileinfo, direct)			//add by mintbaggio 040321 for minus-postnums delete
int ent;
struct fileheader *fileinfo;
char *direct;
{
	if (!strcmp(currboard, "deleted") || !strcmp(currboard, "junk") || !IScurrBM)
		return DONOTHING;
	if (fileinfo->owner[0] == '-')
		return PARTUPDATE;
	change_dir(direct, fileinfo, (void *) DIR_do_mark_minus_del, ent, digestmode, 0);
	return PARTUPDATE;
}

static int
markspec_post(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	if (!strcmp(currboard, "deleted") || !strcmp(currboard, "junk") || !IScurrBM)
		return DONOTHING;
	change_dir(direct, fileinfo, (void *) DIR_do_spec, ent, digestmode, 0);
	return PARTUPDATE;
}

static int
import_spec()
{
	int fd;
	int put_announce_flag;
	char direct[STRLEN];
	struct fileheader fileinfo;
	char anboard[STRLEN], tmpboard[STRLEN];
//   if(strcmp(currentuser.userid,"ecnegrevid")!=0) return DONOTHING;
	if (digestmode == 2 || digestmode == 3
			|| digestmode == 4 || digestmode == 5
			|| !strcmp(currboard, "deleted")
			|| !strcmp(currboard, "junk") || !IScurrBM)
		return DONOTHING;
	if (select_anpath() < 0 || check_import(anboard) < 0)
		return FULLUPDATE;
	setbdir(direct, currboard, digestmode);
	fd = open(direct, O_RDWR);
	if (fd == -1)
		return FULLUPDATE;
	put_announce_flag = !strcmp(currboard, anboard);
	strcpy(tmpboard, currboard);
	strcpy(currboard, anboard);
	while (read(fd, &fileinfo, sizeof (fileinfo)) > 0) {
		if (!(fileinfo.accessed & FH_SPEC))
			continue;
		fileinfo.accessed &= ~FH_SPEC;
		if (put_announce_flag)
			fileinfo.accessed |= FH_ANNOUNCE;
		if (a_Import(direct, &fileinfo, YEA) < 0)
			break;
		if (lseek(fd, -sizeof (fileinfo), SEEK_CUR) == (off_t) - 1)
			break;
		if (write(fd, &fileinfo, sizeof (fileinfo)) < 0)
			break;
	}
	strcpy(currboard, tmpboard);
	close(fd);
	return DIRCHANGED;
}

static int
moveintobacknumber(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct tm atm;
	time_t t;
	char buf[STRLEN], content[1024];
	if (uinfo.mode != READING || digestmode != NA || !IScurrBM)
		return DONOTHING;
	if (!HAS_PERM(PERM_OBOARDS, currentuser) && !HAS_PERM(PERM_SYSOP, currentuser))
		return DONOTHING;
	clear();
	memset(&atm, 0, sizeof(atm));
	prints("整理过刊, 请指定日期, 在该日期之前所发表的文章将被\n"
			"迁移到最后一个过刊目录里, 而且不再存在于版面\n");
	if (askyn("要继续吗?", NA, NA) == NA)
		return FULLUPDATE;
	t = time(NULL);
	atm = *gmtime(&t);
	atm.tm_sec = 0;
	atm.tm_min = 0;
	atm.tm_hour = 0;
	while (1) {
		getdata(3, 0, "年: ", buf, 6, DOECHO, YEA);
		atm.tm_year = atoi(buf) - 1900;
		if (atm.tm_year + 1900 >= 1999)
			break;
		prints("不恰当的年份(1999～)");
	}
	while (1) {
		getdata(4, 0, "月: ", buf, 6, DOECHO, YEA);
		atm.tm_mon = atoi(buf) - 1;
		if (atm.tm_mon + 1 > 0 && atm.tm_mon + 1 <= 12)
			break;
		prints("不恰当的月份(1～12)");
	}
	while (1) {
		getdata(5, 0, "日: ", buf, 6, DOECHO, YEA);
		atm.tm_mday = atoi(buf);
		if (atm.tm_mday > 0 && atm.tm_mday <= 31)
			break;
		prints("不恰当的日期(1～31)");
	}
	t = mktime(&atm);
	if (t <= 0 || t >= (time(NULL) - (time_t) 3600 * 24 * 7)) {
		prints("时间指定有误( 您输入的时间为%s )\n(距当前日期一周之内的文章不能放到过刊的)\n", ytht_ctime(t));
		pressanykey();
		return FULLUPDATE;
	}
	prints("您所指定的时间为 %s", ctime(&t));
	if (askyn("确定要把该时间之前的文章放进过刊么(可能需要几分钟)?", NA, NA) == YEA) {
		int retv;
		retv = do_intobacknumber(direct, t);
		updatelastpost(currboard);
		if (retv < 0) {
			prints("retv=%d", retv);
			pressanykey();
		}
		sprintf(genbuf, "into backnumber, %s, %s, %d",
				currboard, ytht_ctime(t), retv);
		sprintf(content, "%s 将文章置入 %s 版过刊 ",
			currentuser.userid, currboard);
		securityreport(genbuf, content);
	}
	return FULLUPDATE;
}

int
del_range(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char num[16], content[1024];
	int inum1, inum2, ret;
	if (uinfo.mode == READING)
		if (!IScurrBM) {
			return DONOTHING;
		}
	//allow sysops to repair, but nobody can del_range
	//if(digestmode==4||digestmode==5) return DONOTHING;

	if (((digestmode >= 2 && digestmode <= 5) || !strcmp(currboard, "syssecurity")) && uinfo.mode == READING)
		return DONOTHING;
	clear();
	prints("区域删除\n");
	getdata(1, 0,
		"首篇文章编号(输入0清除标记为删除的文章): ",
		num, 6, DOECHO, YEA);
	inum1 = atoi(num);
	if (inum1 == 0) {
		inum2 = -1;
		goto THERE;
	}

	if (inum1 <= 0) {
		prints("错误编号\n");
		pressreturn();
		return FULLUPDATE;
	}
	getdata(2, 0, "末篇文章编号: ", num, 14, DOECHO, YEA);
	inum2 = atoi(num);
	if (inum2 - inum1 <= 1) {
		prints("错误编号\n");
		pressreturn();
		return FULLUPDATE;
	}
THERE:
	move(3, 0);
	if (askyn("确定删除", NA, NA) == YEA) {
		if ((ret = delete_range(direct, inum1, inum2)) < 0) {
			char num1[20];
			char fullpath[200];
			prints("无法删除:%d\n", ret);
			getdata(8, 0,
				"区段删除错误,如果想修复,请确定\033[35m无人在本版执行区段删除操作并按'Y'\033[0m (Y/N)? [N]: ",
				num1, 10, DOECHO, YEA);
			if (*num1 == 'Y' || *num1 == 'y') {
				sprintf(fullpath, "boards/%s/.tmpfile",
					currboard);
				unlink(fullpath);
				sprintf(fullpath, "boards/%s/.deleted",
					currboard);
				unlink(fullpath);
				sprintf(fullpath, "boards/%s/.tmpfilD",
					currboard);
				unlink(fullpath);
				sprintf(fullpath, "boards/%s/.tmpfilJ",
					currboard);
				unlink(fullpath);
				prints("\n错误已经消除,请重新执行区段删除!");
			}

			pressreturn();
			return FULLUPDATE;
		}
		fixkeep(direct, (inum1 <= 0) ? 1 : inum1,
			(inum2 <= 0) ? 1 : inum2);
		if (uinfo.mode == READING) {
			updatelastpost(currboard);
			sprintf(genbuf, "Range delete %d-%d on %s",
				inum1, inum2, currboard);
			sprintf(content, "%s 区段删除 %s 版 %d-%d篇",
				currentuser.userid, currboard, inum1, inum2);
			securityreport(genbuf, content);
			sprintf(genbuf, "%s ranged %s %d %d",
				currentuser.userid, currboard, inum1, inum2);
			newtrace(genbuf);
		} else {
			sprintf(genbuf, "%s rangedmail %d %d",
				currentuser.userid, inum1, inum2);
			newtrace(genbuf);
		}
		prints("删除完成\n");
		pressreturn();
		return DIRCHANGED;
	}
	prints("Delete Aborted\n");
	pressreturn();
	return FULLUPDATE;
}

static int
del_post_backup(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	int keep, fail;
	char filepath[STRLEN];
	if (digestmode == 2 || digestmode == 3
			|| digestmode == 4 || digestmode == 5
			|| !strcmp(currboard, "deleted")
			|| !strcmp(currboard, "junk")
			|| !strcmp(currboard, "syssecurity"))
		return DONOTHING;
	if (!strcmp(currboard, "deleterequest")) {
		if (!strncmp(fileinfo->title, "done", 4))
			return DONOTHING;
		snprintf(genbuf, 60, "done %-27.27s - %s", fileinfo->title, currentuser.userid);
		strcpy(fileinfo->title, genbuf);
		change_dir(direct, fileinfo, (void *) DIR_do_changetitle, ent, digestmode, 1);
		return FULLUPDATE;
	}
	if (hideboard(currboard))
		return DONOTHING;
	if (fileinfo->owner[0] == '-')
		return PARTUPDATE;
	keep = sysconf_eval("KEEP_DELETED_HEADER");
	if (qnyjzx(currentuser.userid)) {
		if (!politics(currboard)) {
			return DONOTHING;
		}
	} else if (!ISdelrq)
		return DONOTHING;
	clear();
	sprintf(genbuf, "删除文章 [%-.55s]", fileinfo->title);
	if (askyn(genbuf, NA, NA) == NA) {
		if (fileinfo->accessed | FH_DANGEROUS) {
			sprintf(genbuf, "那要清除本文的危险标记嘛? [%-38s]",
				fileinfo->title);
			if (askyn(genbuf, YEA, NA) == YEA) {
				change_dir(direct, fileinfo,
					(void *) DIR_clear_dangerous, ent,
					digestmode, 1);
				prints("已经清除本文的危险标记\n");
				pressreturn();
				return FULLUPDATE;
			}
		}
		move(2, 0);
		prints("取消\n");
		pressreturn();
		clear();
		return FULLUPDATE;
	}
	snprintf(genbuf, 256, "%s del %s %s %s",
		currentuser.userid, currboard, fh2owner(fileinfo),
		fileinfo->title);
	newtrace(genbuf);
	currfiletime = fileinfo->filetime;
	setbfile(filepath, currboard, fh2fname(fileinfo));
	sprintf(fileinfo->title, "%-32.32s - %s", fileinfo->title,
		currentuser.userid);
	post_to_1984(filepath, fileinfo, 1);
	if (keep <= 0) {
		fail = delete_file(direct, sizeof (struct fileheader), ent, (void *) cmpfilename);
	} else {
		fail = update_file(direct, sizeof (struct fileheader),
				ent, (void *) cmpfilename,
				(void *) cpyfilename);
	}
	if (!fail) {
		updatelastpost(currboard);
		unlink(filepath);
		limit_cpu();
		return DIRCHANGED;
	}

	move(2, 0);
	prints("删除失败\n");
	pressreturn();
	clear();
	return FULLUPDATE;
}

int
del_post(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	int keep, fail;
	int owned;
	struct boardmem *bp;
	if (digestmode == 2 || digestmode == 3
			|| digestmode == 4 || digestmode == 5
			|| !strcmp(currboard, "deleted")
			|| !strcmp(currboard, "junk")
			|| !strcmp(currboard, "syssecurity"))
		return DONOTHING;
	if (fileinfo->owner[0] == '-')
		return PARTUPDATE;
	bp = ythtbbs_cache_Board_get_board_by_name(currboard);
	if (bp == NULL)
		return DONOTHING;
	keep = sysconf_eval("KEEP_DELETED_HEADER");
	owned = isowner(&currentuser, fileinfo);
	if (digestmode == 1)
		owned = 0;
	if (!owned && !IScurrBM)
		return DONOTHING;
	if(!SR_BMDELFLAG){
		clear();
		sprintf(genbuf, "删除文章 [%-.55s]", fileinfo->title);
		if (askyn(genbuf, NA, NA) == NA) {
			move(2, 0);
			prints("取消\n");
			pressreturn();
			clear();
			return FULLUPDATE;
		}
	}
	snprintf(genbuf, 256, "%s del %s %s %s",
		currentuser.userid, currboard, fh2owner(fileinfo),
		fileinfo->title);
	newtrace(genbuf);
	currfiletime = fileinfo->filetime;
	if (keep <= 0) {
		fail = delete_file(direct, sizeof (struct fileheader), ent, (void *) cmpfilename);
	} else {
		fail = update_file(direct, sizeof (struct fileheader),
				ent, (void *) cmpfilename,
				(void *) cpyfilename);
	}
	if (!fail) {
		updatelastpost(currboard);
		cancelpost(currboard, currentuser.userid, fileinfo, owned);

		if (!bmy_board_is_system_board(currboard)) {
			if (fileinfo->filetime != fileinfo->thread) {
				bmy_article_del_comment(ythtbbs_cache_Board_get_idx_by_name(currboard) + 1, fileinfo->thread);
			} else {
				bmy_article_del_thread(ythtbbs_cache_Board_get_idx_by_name(currboard) + 1, fileinfo->thread);
			}
		}

		if (digestmode == NA && owned) {
			set_safe_record();
			if (!junkboard()) {
				if ((currentuser.numposts > 0) && (strncasecmp(fileinfo->title, "【合集】", 8))) {//modify by mintbaggio 040322 for no minus author's numposts if it's a heji
					currentuser.numposts--;
					substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
				}
			}
		}
		if (digestmode == YEA) {
			char normaldir[STRLEN];
			struct fileheader normalfh;
			bzero(&normalfh, sizeof (normalfh));
			normalfh.filetime = fileinfo->filetime;
			digestmode = 0;
			setbdir(normaldir, currboard, digestmode);
			change_dir(normaldir, &normalfh, (void *) DIR_do_digest, bp->total, 0, 0);
			digestmode = 1;
		}
		limit_cpu();
//              sleep(3);
		return DIRCHANGED;
	}

	move(2, 0);
	prints("删除失败\n");
	pressreturn();
	clear();
	return FULLUPDATE;
}

static int sequent_ent;
static int
sequent_messages(fptr)
struct fileheader *fptr;
{
	static int idc;
	if (fptr == NULL) {
		idc = 0;
		return 0;
	}
	idc++;
	if (readpost) {
		if (idc < sequent_ent)
			return 0;
		if (!UNREAD(fptr, &brc))
			return 0;
		if (continue_flag != 0) {
			genbuf[0] = 'y';
		} else {
			prints("讨论区: '%s' 标题:\n\"%s\" posted by %s.\n", currboard, fptr->title, fh2owner(fptr));
			getdata(3, 0, "读取 (Y/N/Quit) [Y]: ", genbuf, 5, DOECHO, YEA);
		}
		if (genbuf[0] != 'y' && genbuf[0] != 'Y' && genbuf[0] != '\0') {
			if (genbuf[0] == 'q' || genbuf[0] == 'Q') {
				clear();
				return QUIT;
			}
			clear();
			return 0;
		}
		setbfile(genbuf, currboard, fh2fname(fptr));
		ytht_strsncpy(quote_file, genbuf, sizeof(quote_file));
		ytht_strsncpy(quote_user, fh2owner(fptr), sizeof(quote_user));
#ifdef NOREPLY
		ansimore_withzmodem(genbuf, YEA, fptr->title);
#else
		ansimore_withzmodem(genbuf, NA, fptr->title);
		move(t_lines - 1, 0);
		clrtoeol();
		prints("\033[1;44;31m[连续读信]  \033[33m回信 R │ 结束 Q,← │下一封 ' ',↓ │^R 回信给作者                \033[m");
		continue_flag = 0;
		switch (egetch()) {
		case 'N':
		case 'Q':
		case 'n':
		case 'q':
		case KEY_LEFT:
			break;
		case 'Y':
		case 'R':
		case 'y':
		case 'r':
			/* Added by deardragon 1999.11.21 增加不可 RE 属性 */
			if (!(fptr->accessed & FH_NOREPLY))
				do_reply(fptr);
			else {
				move(3, 0);
				clrtobot();
				prints("\n\n    老大,有人不让你Re这篇文章啊!!!    ");
				pressreturn();
				clear();
			}
			/* Added End. */
			break;
		case ' ':
		case '\n':
		case KEY_DOWN:
			continue_flag = 1;
			break;
		case Ctrl('R'):
			post_reply(0, fptr, (char *) NULL);
			break;
		default:
			break;
		}
#endif
		clear();
	}
	setbdir(genbuf, currboard, digestmode);
	SETREAD(fptr, &brc);
	return 0;
}

static int
clear_new_flag(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	static int lastf;
	if (now_t - lastf > 2)
		clear_new_flag_quick(max(fileinfo->filetime, fileinfo->edittime));
	else
		clear_new_flag_quick(0);
	lastf = now_t;
	return PARTUPDATE;
}

static int
sequential_read(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	readpost = 1;
	clear();
	return sequential_read2(ent);
}
/*ARGSUSED*/
static int sequential_read2(ent /*,fileinfo,direct */ )
int ent;
/*struct fileheader *fileinfo ;
char *direct ;*/
{
	char buf[STRLEN];
	sequent_messages((struct fileheader *) NULL);
	sequent_ent = ent;
	continue_flag = 0;
	setbdir(buf, currboard, digestmode);
	apply_record(buf, (void *) sequent_messages, sizeof (struct fileheader));
	return FULLUPDATE;
}

/* Added by netty to handle post saving into (0)Announce */
int
Save_post(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	if (!IScurrBM)
		return DONOTHING;
	return (a_Save("0Announce", currboard, fileinfo, NA));
}

/* Added by ylsdd */
static void
quickviewpost(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char buf[STRLEN * 2];
	int i, j, x, y;
	int attach = 0, has_attach;
	FILE *fp;
	getyx(&y, &x);
	directfile(buf, direct, fh2fname(fileinfo));
	move(t_lines - 8, 0);
	clrtobot();
	fp = fopen(buf, "r");
	if (fp == NULL)
		return;
	if (fileinfo->accessed & FH_ATTACHED)
		has_attach = 1;
	else
		has_attach = 0;
	prints("\033[1;32m作者: \033[33m%-12s \033[32m标题: \033[33m%-40s\033[0m", fh2owner(fileinfo), fileinfo->title);
	for (i = 0, j = 0; j < 7 - has_attach; i++) {
		move(t_lines - 7 + j, 0);
		if (fgets(buf, sizeof (buf), fp) == NULL)
			break;
		if (i < 4)
			continue;
		if (!strncmp(buf, "begin 644 ", 10)) {
			char attachname[41], *p;
			attach = 1;
			strncpy(attachname, buf + 10, 40);
			attachname[40] = 0;
			p = strchr(attachname, '\n');
			if (p != NULL)
				*p = 0;
			p = strrchr(attachname, '.');
			if (p != NULL
					&& (!strcasecmp(p, ".bmp") || !strcasecmp(p, ".jpg")
						|| !strcasecmp(p, ".gif")
						|| !strcasecmp(p, ".jpeg")))
				prints("\033[m附图: %s \033[5m(用www方式阅读本文可以浏览此图片)\033[0m\n", attachname);
			else
				prints("\033[m附件: %s \033[5m(用www方式阅读本文可以下载此附件)\033[0m\n", attachname);
			j++;
		} else if (!strncmp(buf, "end", 3) && attach) {
			attach = 0;
			continue;
		}
		if (attach)
			continue;
		j++;
		prints("\033[0m%s", buf);
	}
	if (has_attach) {
		get_temp_sessionid(buf);
		prints("http://%s/" SMAGIC "%s/con?B=%s&F=%s&N=%d",
				MY_BBS_DOMAIN, buf, currboard, fh2fname(fileinfo), ent);
		j++;
	}
	if (j < 6) {
		move(t_lines - 7 + j, 0);
		prints("==========================结束=============================");
	}
	fclose(fp);
	move(y, x);
	refresh();
}

/* Added by ylsdd */
static int
change_t_lines()
{
	extern void (*quickview) ();
	if (0)
		if (!IScurrBM)
			return DONOTHING;
	quickview = quickviewpost;
	return UPDATETLINE;
}

static int
post_saved()
{
	char fname[STRLEN], title[STRLEN];
	if (!IScurrBM)
		return DONOTHING;
	sprintf(fname, "bm/%s", currentuser.userid);
	title[0] = 0;
	if (!dashf(fname)) {
		a_prompt(-1,
			"请先至该讨论区将文章存入暂存档, 按<Enter>继续...",
			title, 2);
		return PARTUPDATE;
	}
	a_prompt(-1, "请输入文件或目录之中文名称： ", title, 50);
	if (strlen(title) == 0)
		return PARTUPDATE;
	if (postfile(fname, currboard, title, 2) != -1) {
		a_prompt(-1,
			"已经帮你将暂存档转贴至版面了, 按<Enter>继续...",
			title, 2);
	}
	unlink(fname);
	return DIRCHANGED;
}

/* Added by netty to handle post saving into (0)Announce */
/* 修改以用于对个人精华区的支持, by ylsdd*/
int
Import_post(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	if (!HAS_PERM(PERM_BOARDS | PERM_SYSOP, currentuser) && !HAS_PERM(PERM_SPECIAL8, currentuser))
		return FULLUPDATE;
	a_Import(direct, fileinfo, NA);
	change_dir(direct, fileinfo, (void *) DIR_do_import, ent, digestmode, 1);
	return FULLUPDATE;
}

static int check_notespasswd() {
	FILE *pass;
	char passbuf[20], prepass[STRLEN];
	char buf[STRLEN];
	setvfile(buf, currboard, "notespasswd");
	if ((pass = fopen(buf, "r")) != NULL) {
		fgets(prepass, STRLEN, pass);
		fclose(pass);
		prepass[strlen(prepass) - 1] = '\0';
		getdata(2, 0, "请输入秘密备忘录密码: ", passbuf, 19, NOECHO, YEA);
		if (passbuf[0] == '\0' || passbuf[0] == '\n')
			return NA;
		if (!ytht_crypt_checkpasswd(prepass, passbuf)) {
			move(3, 0);
			prints("错误的秘密备忘录密码...");
			pressanykey();
			return NA;
		}
	}
	return YEA;
}

static int
show_b_secnote()
{
	char buf[256];
	clear();
	setvfile(buf, currboard, "secnotes");
	if (dashf(buf)) {
		if (!check_notespasswd())
			return FULLUPDATE;
		clear();
		ansimore(buf, NA);
	} else {
		move(3, 25);
		prints("此讨论区尚无「秘密备忘录」。");
	}
	pressanykey();
	return FULLUPDATE;
}

static int
show_b_note()
{
	clear();
	if (show_board_notes(currboard) == -1) {
		move(4, 30);
		prints("此讨论区尚无「备忘录」。");
	}
	show_small_bm(currboard);
	if (!strcmp(currboard, "deleterequest")) {
		move(1, 0);
		if (!ythtbbs_cache_utmp_get_watchman())
			prints("政治性版面当前都处在解锁状态");
		else
			prints("政治性版面已锁定,晚于 %s 就不能发表文章了.解锁码: %d",
				ytht_ctime(ythtbbs_cache_utmp_get_watchman()), ythtbbs_cache_utmp_get_unlock() % 10000);

	}
	return what_to_do();
}

static int
show_file_info(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	struct boardmem *bp;
	char temp_sessionid[10];
	time_t t = fileinfo->filetime;
	bp = ythtbbs_cache_Board_get_board_by_name(currboard);
	if (NULL == bp)
		return DONOTHING;
	get_temp_sessionid(temp_sessionid);
	clear();
	move(0, 0);
	prints("这篇文章的详细信息如下:\n");
	prints("版面名称:     %s\n", currboard);
	prints("本版版主:     %s\n", bp->header.bm[0]);
	prints("版内在线:     %d人\n", bp->inboard);
	prints("文章序号:     %d\n", ent);
	prints("文章标题:     %s\n", fileinfo->title);
	prints("文章作者:     %s\n", fh2owner(fileinfo));
	prints("文章日期:     %s", ctime(&t));
	prints("文章等级:     %d级\n", (fileinfo->staravg50 / 50));
	prints("文件大小:     %d字节\n", ytht_byte2num(fileinfo->sizebyte));
	prints("URL 地址:\n");
	prints("http://%s/" SMAGIC "%s/%scon?B=%s&F=%s\n", MY_BBS_DOMAIN,
			temp_sessionid, (digestmode == YEA) ? "g" : "", currboard,
			fh2fname(fileinfo));
	return what_to_do();
}

int
what_to_do()
{
	int retv = FULLUPDATE;
	move(t_lines - 2, 0);
	prints("\033[m%s%s%s%s%s",
			"(u)查询网友",
			HAS_PERM(PERM_POST, currentuser) ? "(m)书灯絮语" : "",
			HAS_PERM(PERM_POST, currentuser) ? "(i)飞鸽传书" : "",
			HAS_PERM(PERM_CHAT, currentuser) ? "(c)咖啡红茶店" : "",
			HAS_PERM(PERM_BASIC, currentuser) ? "(o)好友名单" : "");
	move(t_lines - 1, 0);
	prints("请选择功能, 或按空格键继续");
	switch (igetkey()) {
	case 'u':
		clear();
		prints("查询网友状态");
		t_query(NULL);
		break;
	case 'm':
		clear();
		move(0, 0);
		prints("发送站内信件");
		if (HAS_PERM(PERM_POST, currentuser))
			m_send(NULL);
		break;
	case 'i':
		clear();
		move(0, 0);
		prints("发送Internet信件");
		if (HAS_PERM(PERM_POST, currentuser))
			m_internet();
		break;
	case 'c':
		if (HAS_PERM(PERM_CHAT, currentuser))
			ent_chat("2");
		break;
	case 'o':
		if (HAS_PERM(PERM_BASIC, currentuser)) {
			t_friend();
			retv = 999;
		}
		break;
	}
	return retv;
}

static int
do_t_query()
{
	t_query(NULL);
	return FULLUPDATE;
}

static int
into_backnumber()
{
	int savemode = uinfo.mode;
	selectbacknumber();
	modify_user_mode(savemode);
	return 999;
}

int
into_announce()
{
	if (a_menusearch("0Announce", currboard, HAS_PERM(PERM_ANNOUNCE | PERM_SYSOP | PERM_OBOARDS, currentuser) ? PERM_BOARDS : 0))
		return 999;
	return DONOTHING;
}

static int
into_my_Personal()
{
	Personal("*");
	return FULLUPDATE;
}

static int
select_Personal()
{
	char uident[STRLEN], cmd[STRLEN];
	move(0, 0);
	clrtoeol();
	usercomplete("您要看谁的文集？", uident);
	sprintf(cmd, "$%.15s", uident);
	Personal(cmd);
	return FULLUPDATE;
}

#ifdef INTERNET_EMAIL
int
forward_post(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	if (strcmp("guest", currentuser.userid) == 0)
		return DONOTHING;
	return (mail_forward(ent, fileinfo, direct));
}

int
forward_u_post(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	if (strcmp("guest", currentuser.userid) == 0)
		return DONOTHING;
	return (mail_u_forward(ent, fileinfo, direct));
}

#endif

struct one_key read_comms[] = {
	{'_', underline_post, "禁止回复"},
	{'r', read_post, "阅读文章"},
	{'u', do_t_query, "查询用户"},
	{'d', del_post, "删除文章"},
	{'D', del_range, "区段删除"},
	{'m', mark_post, "M 文章"},
	{'t', markdel_post, "标记不减文章数删除"},		//modify by mintbaggio 040322
	{'n', mark_minus_del_post, "标记减文章数删除"},		//add by mintbaggio 040322 for minus-numposts delete
	{'E', edit_post, "修改文章"},
	{'Y', UndeleteArticle, "恢复文章到版面"},
	{Ctrl('G'), marked_mode, "文摘模式"},
	{Ctrl('T'), thread_mode, "同主题模式"},
	{Ctrl('Y'), zmodem_sendfile, "ZMODEM 下载"},
	{'.', deleted_mode, "删除模式"},	//add by ylsdd
	{'>', junk_mode, "垃圾模式"},	//add by ylsdd
	{'g', digest_post, "G 文章"},
	{'L', show_allmsgs, "查看消息"},
	{'T', edit_title, "修改标题"},
	{'s', do_select, "快速切换讨论区"},
	{Ctrl('C'), do_cross, "转贴文章"},
	{Ctrl('P'), do_post, "发表文章"},
	{'c', t_friends, "查看好友"},	/*clear_new_flag,          youzi */
	{'o', sequential_read, "循序阅读新文章"},
#ifdef INTERNET_EMAIL
	{'F', forward_post, "寄回邮箱"},
	{'U', forward_u_post, "uuencode 寄回"},
	{Ctrl('R'), post_reply, "回信给原作者"},
#endif
	{'i', Save_post, "将文章存入暂存档"},
	{'I', Import_post, "将文章放入精华区"},
	{'R', b_results, "查看投票结果"},
	{'v', b_vote, "参加投票"},
	{'M', b_vote_maintain, "维护投票"},
	{'W', b_notes_edit, "编辑/删除备忘录"},
	{Ctrl('W'), b_notes_passwd, "设定备忘录密码"},
	{'h', mainreadhelp, "查看帮助"},
	{KEY_TAB, show_b_note, "查看一般备忘录"},
	{'z', show_b_secnote, "查看秘密备忘录"},
	{'x', into_announce, "进入精华区"},
	{'X', into_my_Personal, /* ylsdd 2000.2.27 */ "进入自己的个人文集"},
	{'a', auth_search_down, "向后搜索作者"},
	{'A', auth_search_up, "向前搜索作者"},
	{'/', t_search_down, "向后搜索标题"},
	{'?', t_search_up, "向前搜索标题"},
	{'\'', post_search_down, "向后搜索内容"},
	{'\"', post_search_up, "向前搜索内容"},
	{']', thread_down, "向后同主题"},
	{'[', thread_up, "向前同主题"},
	{Ctrl('D'), deny_user, "取消某人POST权利"},
	{Ctrl('A'), show_author, "作者简介"},
	{'^', SR_first_new, "主题未读的第一篇"},
	{'\\', SR_last, "最后一篇同主题文章"},
	{'=', SR_first, "第一篇同主题文章"},
	{'p', SR_read, "相同主题的阅读"},
	{Ctrl('U'), SR_author, "相同作者阅读"},
	{'b', SR_BMfunc, "相关主题特殊功能"},
	{'!', Q_Goodbye, "快速离站"},
	{'S', s_msg, "传送讯息"},
	{'f', clear_new_flag, "清除所有未读标记"},
	{'e', markspec_post, "标记选定"},
	{Ctrl('E'), import_spec, "将选定文章放入精华区"},
	{Ctrl('K'), post_saved, "将暂存档张贴"},
	{',', change_t_lines, "更改行数,即时显示内容"},
	{'K', moveintobacknumber, "将文章移入过刊"},
	{';', into_backnumber, "查看过刊"},
	{Ctrl('N'), clubmember, "俱乐部成员设置"},
	{'C', friend_author, "增加作者为好友"},
	{Ctrl('S'), select_Personal, "选择个人文集"},
	{Ctrl('X'), power_select, "超强文章操作"},
	{'Z', del_post_backup, "特殊删文"},	//如果是deleterequest版面，则表示完成任务
	{'B', allcanre_post, "设置话题"},
	{'l', show_file_info, "文章详细信息"},
	{'#', topfile_post,"置底"},
	{'&', mark_commend, "标记推荐文章"},	//add by mintbagio 040331 for front page commend
	{'%', m_template, "维护发文模板"},	//add by macintosh 20060315 for post template
	{'V',m_voter,"设定限制投票ID名单"},
	{'*', mark_commend2, "设定通知公告"},
	{'\0', NULL, ""}
};

/*Add by SmallPig*/
static void
notepad()
{
	char tmpname[STRLEN], note1[4];
	char note[3][STRLEN - 4];
	char tmp[STRLEN];
	FILE *in;
	int i, n;
	time_t thetime = time(NULL);
	extern int talkrequest;
	clear();
	move(0, 0);
	prints("开始你的留言吧！大家正拭目以待....\n");
	sprintf(tmpname, "tmp/notepad.%s.%05d", currentuser.userid, uinfo.pid);
	if ((in = fopen(tmpname, "w")) != NULL) {
		for (i = 0; i < 3; i++)
			memset(note[i], 0, STRLEN - 4);
		while (1) {
			for (i = 0; i < 3; i++) {
				getdata(1 + i, 0, ": ", note[i], STRLEN - 5,
					DOECHO, NA);
				if (note[i][0] == '\0')
					break;
			}
			if (i == 0) {
				fclose(in);
				unlink(tmpname);
				return;
			}
			getdata(5, 0,
				"是否把你的大作放入留言版 (Y)是的 (N)不要 (E)再编辑 [Y]: ",
				note1, 3, DOECHO, YEA);
			if (note1[0] == 'e' || note1[0] == 'E')
				continue;
			else
				break;
		}
		if (note1[0] != 'N' && note1[0] != 'n') {
			sprintf(tmp, "\033[1;32m%s\033[37m（%.18s）",
				currentuser.userid, currentuser.username);
			fprintf(in,
				"\033[1;34m□\033[44m□□□□□□□□□□□□□□□□□\033[36m酸\033[32m甜\033[33m苦\033[31m辣\033[37m版\033[34m□□□□□□□□□□□□□□□\033[40m□\033[m\n");
			fprintf(in,
					"\033[1;34m□\033[32;44m %-48s\033[32m在 \033[36m%.19s\033[32m 离开时留下的话  \033[m\n",
					tmp, ytht_ctime(thetime));
			for (n = 0; n < i; n++) {
				if (note[n][0] == '\0')
					break;
				fprintf(in,
					"\033[1;34m□\033[33;44m %-75.75s\033[1;34m\033[m \n",
					note[n]);
			}
			fprintf(in,
				"\033[1;34m□\033[44m ————————————————————————————————————— \033[m \n");
			catnotepad(in, "etc/notepad");
			fclose(in);
			rename(tmpname, "etc/notepad");
		} else {
			fclose(in);
			unlink(tmpname);
		}
	}
	if (talkrequest) {
		talkreply();
	}
	clear();
	return;
}

int
Goodbye()
{
	char spbuf[STRLEN];
	int choose;
	FILE *fp = NULL;
	char* buf = NULL;
	alarm(0);
	if (strcmp(currentuser.userid, "guest") && ythtbbs_cache_UserTable_count(usernum) == 1) {
		if (DEFINE(DEF_MAILMSG, currentuser)) {
			if (get_msgcount(0, currentuser.userid) > 0)
				show_allmsgs();
		} else {
			clear_msg(currentuser.userid);
		}
	}

	*quote_file = '\0';
	move(1, 0);
	clear();
	move(0, 0);
	prints("你就要离开 %s ，可有什么建议吗？\n", MY_BBS_NAME);
	prints("[\033[1;33m1\033[m] 寄信给管理人员\n");
	prints("[\033[1;33m2\033[m] 按错了啦，我还要玩\n");
	if (strcmp(currentuser.userid, "guest") != 0) {
		if (USE_NOTEPAD == 1)
			prints ("[\033[1;33m3\033[m] 写写\033[1;32m留\033[33m言\033[35m版\033[m罗\n");
	}
	prints("[\033[1;33m4\033[m] 不寄罗，要离开啦\n");
	sprintf(spbuf, "你的选择是 [\033[1;32m4\033[m]：");
	getdata(7, 0, spbuf, genbuf, 4, DOECHO, YEA);
	clear();
	choose = genbuf[0] - '0';
	if (choose == 1) {
		if (!strcmp(currentuser.userid, "guest")) {
			prints("先注册再给管理员写信吧。\n");
			pressanykey();
		} else {	//add by mintbaggio 040406 for mail OBOARDS when logout
			clear();
			prints("你想寄信给哪个主管站长？\n");
			char userid[50][IDLEN+2], board[20];
			int i=0, ch;
			char *buf_ptr;

			fp = fopen(MY_BBS_HOME"/etc/secmlist", "r");
			if(!fp){
				prints("fatal error, couldn't open secmlist, please contact the SYSOP\n");
				pressreturn();
				goto goodbye;
			}
			buf = (char*) malloc(IDLEN + 2 + 2);//IDLEN+2 is length of userid, 2 is bnumber and character '  '
			memset(buf, 0, IDLEN + 2 + 2);
			while(fgets(buf, IDLEN + 2 + 2, fp)){
				buf[strlen(buf)-1] = 0;
				if(!buf[0])
					break;
				if(buf[0] == '#')
					continue;
				board[i] = buf[0];
				buf_ptr = buf + 2;
				memset(userid[i], 0, IDLEN + 2);
				strncpy(userid[i], buf_ptr, IDLEN + 1);
/*				if(sscanf(buf, "%c %s", board[i], userid[i])<1){
					prints("break here");
					pressanykey();
					break;
				}*/
				prints("[\033[1;33m%-2d\033[m] %c区: \033[1;32m%s\033[m\n", i, board[i], userid[i]);
				++i;
			}
			sprintf(spbuf, "你的选择是 [\033[1;32m0\033[m]：");
			getdata(i+2, 0, spbuf, genbuf, 4, DOECHO, YEA);
			ch = atoi(genbuf);
			if(ch<0 || ch>i){
				prints("不寄咯，哈哈\n");
				pressreturn();
				goto goodbye;
			}
			else{
				prints("ch=%d, userid=%s\n", ch, userid[ch]);
				pressanykey();
				m_send(userid[ch]);
			}
			//end
		}
	}
	if (choose == 2)
		return FULLUPDATE;
	if (strcmp(currentuser.userid, "guest") != 0) {
		if (choose == 3)
			if (USE_NOTEPAD == 1 && HAS_PERM(PERM_POST, currentuser))
				notepad();
	}
goodbye:
	if (fp) fclose(fp);
	if (buf) free(buf);
	return Q_Goodbye();
}

#if 0
void
report(s)
char *s;
{
	static int disable = NA;
	int fd;
	if (disable)
		return;
	if ((fd = open("trace", O_WRONLY | O_CREAT | O_APPEND, 0660)) != -1) {
		char buf[512];
		char timestr[16], *thetime;
		time_t dtime;
		time(&dtime);
		thetime = ctime(&dtime);
		strncpy(timestr, &(thetime[4]), 15);
		timestr[15] = '\0';
		sprintf(buf, "%s %s %s\n", currentuser.userid, timestr, s);
		write(fd, buf, strlen(buf));
		close(fd);
		return;
	}
	disable = YEA;
	return;
}
#endif

#if 0
void
newtrace_old(s)
char *s;
{
	static int disable = NA;
	char buf[512], logf[256];
	char timestr[16];
	time_t dtime;
	struct tm *n;
	int fd;
	if (disable)
		return;
	time(&dtime);
	n = localtime(&dtime);
	sprintf(logf, "newtrace/%d-%02d-%02d.log",
		1900 + n->tm_year, 1 + n->tm_mon, n->tm_mday);
	sprintf(timestr, "%02d:%02d:%02d", n->tm_hour, n->tm_min, n->tm_sec);
	sprintf(buf, "%s %s\n", timestr, s);
	if ((fd = open(logf, O_WRONLY | O_CREAT | O_APPEND, 0660)) != -1) {
		write(fd, buf, strlen(buf));
		close(fd);
		return;
	}
	disable = YEA;
	return;
}
#endif

int
cmpbnames(brec, bname)
struct boardheader *brec;
char *bname;
{
	if (!strncasecmp(bname, brec->filename, sizeof (brec->filename)))
		return 1;
	else
		return 0;
}

void
setbdir(char *buf, char *boardname, int Digestmode)
{
	char *dir = NULL;

	switch (Digestmode) {
	case NA:
		dir = DOT_DIR;
		break;
	case YEA:
		dir = DIGEST_DIR;
		break;
	case 2:
		dir = THREAD_DIR;
		break;
	case 3:
		dir = ".POWER.";
		break;
	case 4:
		dir = ".DELETED";
		break;
	case 5:
		dir = ".JUNK";
		break;
	}
	if (Digestmode == 3) {
		sprintf(buf, "boards/%s/%s%s%d", boardname, dir,
			currentuser.userid, uinfo.pid);
	} else
		sprintf(buf, "boards/%s/%s", boardname, dir);
}

int
zmodem_sendfile(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char *t;
	char buf1[512];

	strcpy(buf1, direct);
	if ((t = strrchr(buf1, '/')) != NULL)
		*t = '\0';
	snprintf(genbuf, sizeof(genbuf), "%s/%s", buf1, fh2fname(fileinfo));
	return zsend_file(genbuf, fileinfo->title);
}

static int
Origin2(text)
char text[256];
{
	char tmp[STRLEN];

	sprintf(tmp, ":．%s %s．[FROM:", MY_BBS_NAME, email_domain());
	if (strstr(text, tmp))
		return 1;
	else
		return 0;
}

int
deleted_mode()
{
	extern char currdirect[STRLEN];

	if (!IScurrBM && !HAS_PERM(PERM_ARBITRATE, currentuser)
			&& !HAS_PERM(PERM_SPECIAL5, currentuser)) {
		return DONOTHING;
	}
	if (digestmode == 4) {
		digestmode = NA;
		setbdir(currdirect, currboard, digestmode);
	} else {
		digestmode = 4;
		setbdir(currdirect, currboard, digestmode);
		if (!dashf(currdirect)) {
			digestmode = NA;
			setbdir(currdirect, currboard, digestmode);
			return DONOTHING;
		}
	}
	return NEWDIRECT;
}

int
junk_mode()
{
	extern char currdirect[STRLEN];

	if (!HAS_PERM(PERM_BLEVELS, currentuser) && !HAS_PERM(PERM_ARBITRATE, currentuser)) {
		return DONOTHING;
	}

	if (digestmode == 5) {
		digestmode = NA;
		setbdir(currdirect, currboard, digestmode);
	} else {
		digestmode = 5;
		setbdir(currdirect, currboard, digestmode);
		if (!dashf(currdirect)) {
			digestmode = NA;
			setbdir(currdirect, currboard, digestmode);
			return DONOTHING;
		}
	}
	return NEWDIRECT;
}

//modify by macintosh 050427 for recycle_bin_res.
int
marked_mode()
{
	extern char currdirect[STRLEN];
	char ans[3];
	char whattosearch[31];
	char select[80];
	char direct[STRLEN];
	int type;

	if (digestmode == 3 || digestmode == YEA) {
		digestmode = NA;
		setbdir(currdirect, currboard, digestmode);
	} else if (digestmode == 4){
		ans[0] = '\0';
		getdata(t_lines - 1 , 0,
		"请选择:0)取消 1)未读 2)附件 3)防水 4)标题关键字 5)同作者 [0]:",
		ans, 2, DOECHO, NA);
		type = atoi(ans);
		if (ans[0] == '\0') type = 0;
		if (type < 1 || type > 5)
			return PARTUPDATE;
		whattosearch[0] = '\0';
		switch (type) {
		case 1:
			sprintf(select, "属性是 未读");
			break;
		case 2:
			sprintf(select, "标记含 @");
			break;
		case 3:
			sprintf(select, "标记含 m 或 标记含 g 或 属性是 原作");
			break;
		case 4:
			getdata(t_lines - 1, 0,
				"请输入标题关键字:", whattosearch, 31, DOECHO,NA);
			if (whattosearch[0] == '\0')
				return PARTUPDATE;
			sprintf(select, "标题含 %s", whattosearch);
			break;
		case 5:
			getdata(t_lines - 1, 0, "请输入作者ID:",whattosearch, 31, DOECHO, NA);
			if (whattosearch[0] == '\0')
				return PARTUPDATE;
			sprintf(select, "作者是 %s", whattosearch);
			break;
		}
		setbdir(direct, currboard,4);
		return power_action(direct, 1, -1, select, 9);
		setbdir(currdirect, currboard, 4);
		}
	else {
		ans[0] = '\0';
		getdata(t_lines - 1 , 0,
			"请选择:0)取消 1)g文 2)m文 3)防水 4)标题 5)作者 6)纸篓 7)附件 8)全文 [1]:",
			ans, 2, DOECHO, NA);
		type = atoi(ans);
		if (ans[0] == '\0')
			type = 1;
		if (type < 1 || type > 8)
			return PARTUPDATE;
		whattosearch[0] = '\0';
		switch (type) {
		case 1:
			digestmode = YEA;
			break;
		case 3:
			sprintf(select, "标记含 m 或 标记含 g 或 属性是 原作");
			digestmode = NA;
			break;
		case 2:
			sprintf(select, "标记含 m");
			digestmode = NA;
			break;
		case 4:
			getdata(t_lines - 1, 0,
				"请输入标题关键字:", whattosearch, 31, DOECHO,
				NA);
			if (whattosearch[0] == '\0')
				return PARTUPDATE;
			sprintf(select, "标题含 %s", whattosearch);
			digestmode = NA;
			break;
		case 5:
			getdata(t_lines - 1, 0, "请输入作者ID:",
				whattosearch, 31, DOECHO, NA);
			if (whattosearch[0] == '\0')
				return PARTUPDATE;
			sprintf(select, "作者是 %s", whattosearch);
			digestmode = NA;
			break;
		case 6:
			sprintf(select, "作者是 %s", currentuser.userid);
			digestmode = 5;
			break;
		/*case 8:
			sprintf(select, "属性是 未读");
			digestmode = NA;
			break;*/
		case 7:
			sprintf(select, "标记含 @");
			digestmode = NA;
			break;
		// 全文搜索，interma@bmy
		case 8:
			getdata(t_lines - 1, 0, "请输入关键字:", whattosearch, 31, DOECHO,NA);
			if (whattosearch[0] == '\0')
				return PARTUPDATE;
			digestmode = NA;
			break;
		/*case 9:				//作者看回收站自己文章
			sprintf(select, "作者是 %s", currentuser.userid);
			digestmode = 4;
			break;*/
		}
		if (type == 8) {
			return full_search_action(whattosearch);
		}
		if (type != 1) {
			setbdir(direct, currboard, digestmode);
			return power_action(direct, 1, -1, select, 9);
		}
		setbdir(currdirect, currboard, digestmode);
	}
	return NEWDIRECT;
}
/* end */

int
thread_mode()
{
	extern char currdirect[STRLEN];
	if (digestmode == 2) {
		digestmode = NA;
		setbdir(currdirect, currboard, digestmode);
	} else {
		digestmode = 2;
		setbdir(currdirect, currboard, digestmode);
		do_thread();
		if (!dashf(currdirect)) {
			digestmode = NA;
			setbdir(currdirect, currboard, digestmode);
			return PARTUPDATE;
		}
	}
	return NEWDIRECT;
}

static int
do_thread()
{
	char buf[STRLEN * 2];
	snprintf(buf, sizeof(buf), "%s thread %s", currentuser.userid, currboard);
	newtrace(buf);
	move(t_lines - 1, 0);
	clrtoeol();
	prints("\x1b[1;5m系统处理标题中, 请稍候...\x1b[m\n");
	refresh();
	snprintf(buf, sizeof(buf), "bin/thread %s 1>/dev/null 2>/dev/null", currboard);
	system(buf);
	return 0;
}

static int
b_notes_edit()
{
	char buf[200], buf2[STRLEN];
	char ans[4], *ptr;
	int aborted;
	int notetype;

	if (!IScurrBM) {
		return 0;
	}

	clear();
	move(1, 0);
	prints("编辑、删除备忘录\n\n");
	while (1) {
		prints("[\033[1;32m0\033[m] 都不想改\n");
		prints("[\033[1;32m1\033[m] 一般备忘录\n");
		prints("[\033[1;32m2\033[m] 秘密备忘录\n");
		prints("[\033[1;32m3\033[m] 版面简介\n");
		prints("[\033[1;32m4\033[m] 版面关键字\n");
		getdata(9, 0,
			"你要编辑或删除本讨论区的哪一项档案: ",
			ans, 2, DOECHO, YEA);
		if (ans[0] == '0' || ans[0] == '\0')
			return FULLUPDATE;
		if (ans[0] > '0' && ans[0] < '5')
			break;
	}

	makevdir(currboard);
	switch (ans[0]) {
	case '1':
		setvfile(buf, currboard, "notes");
		ptr = "一般备忘录";
		notetype = 1;
		break;
	case '2':
		setvfile(buf, currboard, "secnotes");
		ptr = "秘密备忘录";
		notetype = 2;
		break;
	case '3':
		setbfile(buf, currboard, "introduction");
		ptr = "版面简介";
		notetype = 3;
		break;
	default:
		buf[0]='\0';
		ptr = "版面关键字";
		notetype = 4;
	}

	sprintf(buf2, "(E)编辑 (D)删除 %s? [E]: ", ptr);
	getdata(11, 0, buf2, ans, 2, DOECHO, YEA);

	if (notetype != 4){
		if (ans[0] == 'D' || ans[0] == 'd') {
			move(12, 0);
			sprintf(buf2, "真的要删除%s?", ptr);
			if (askyn(buf2, NA, NA)) {
				move(13, 0);
				prints("%s已经删除...\n",ptr);
				pressanykey();
				unlink(buf);
				aborted = 1;
			} else
				aborted = -1;
		} else
			aborted = vedit(buf, NA, YEA);
		if (aborted == -1) {
			pressreturn();
		} else {
			if (notetype == 1)
				setvfile(buf, currboard, "noterec");
			else if (notetype == 2)
				setvfile(buf, currboard, "notespasswd");
			else
				buf[0]='\0';
			unlink(buf);
		}
	}else {//keywords
		int pos;
		struct boardheader fh;
		char secu[STRLEN];
		if (ans[0] == 'D' || ans[0] == 'd') {
			move(12, 0);
			if (askyn("真的要删除版面关键字?", NA, NA)) {
				pos = new_search_record(BOARDS, &fh, sizeof (fh), (void *) cmpbnames, currboard);
				fh.keyword[0]='\0';
				substitute_record(BOARDS, &fh, sizeof (fh), pos);
				ythtbbs_cache_Board_resolve();
				sprintf(secu, "删除%s版版面关键字", fh.filename);
				securityreport(secu, secu);
				move(14, 0);
				prints("版面关键字已经删除...\n");
				pressanykey();
				aborted = 1;
			} else
				aborted = -1;
		} else{
			char buf3[64];
			pos = new_search_record(BOARDS, &fh, sizeof (fh), (void *) cmpbnames, currboard);
			strcpy(buf3, fh.keyword);
			getdata(12, 0, "请输入新关键字\n: ", buf3, 64, DOECHO, NA);
			move(14, 0);
			if (askyn("要保存修改后的版面关键字吗?", NA, NA)){
				strcpy(fh.keyword, buf3);
				substitute_record(BOARDS, &fh, sizeof (fh), pos);
				ythtbbs_cache_Board_resolve();
				sprintf(secu, "修改%s版版面关键字", fh.filename);
				securityreport(secu, secu);
				move(16, 0);
				prints("版面关键字已经修改...\n");
				pressanykey();
				aborted = 1;
			}else
				aborted = -1;
		}
		if (aborted == -1)
			pressreturn();
	}

	return FULLUPDATE;
}

static int
b_notes_passwd()
{
	FILE *pass;
	char passbuf[20], prepass[20];
	char buf[STRLEN];

	if (!IScurrBM) {
		return 0;
	}
	clear();
	move(1, 0);
	prints("设定/更改「秘密备忘录」密码...");
	setvfile(buf, currboard, "secnotes");
	if (!dashf(buf)) {
		move(3, 0);
		prints("本讨论区尚无「秘密备忘录」。\n\n");
		prints("请先用 W 编好「秘密备忘录」再来设定密码...");
		pressanykey();
		return FULLUPDATE;
	}
	if (!check_notespasswd())
		return FULLUPDATE;
	getdata(3, 0, "请输入新的秘密备忘录密码: ", passbuf, 19, NOECHO, YEA);
	getdata(4, 0, "确认新的秘密备忘录密码: ", prepass, 19, NOECHO, YEA);
	if (strcmp(passbuf, prepass)) {
		prints("\n密码不相符, 无法设定或更改....");
		pressanykey();
		return FULLUPDATE;
	}
	setvfile(buf, currboard, "notespasswd");
	if ((pass = fopen(buf, "w")) == NULL) {
		move(5, 0);
		prints("备忘录密码无法设定....");
		pressanykey();
		return FULLUPDATE;
	}
	fprintf(pass, "%s\n", ytht_crypt_genpasswd(passbuf));
	fclose(pass);
	pass = 0;
	move(5, 0);
	prints("秘密备忘录密码设定完成....");
	pressanykey();
	return FULLUPDATE;
}

static int
catnotepad(fp, fname)
FILE *fp;
char *fname;
{
	char inbuf[256];
	FILE *sfp;
	int count;

	count = 0;
	if ((sfp = fopen(fname, "r")) == NULL) {
		fprintf(fp,
			"\x1b[1;34m  □\x1b[44m__________________________________________________________________________\x1b[m \n\n");
		return -1;
	}
	while (fgets(inbuf, sizeof (inbuf), sfp) != NULL) {
		if (count != 0)
			fputs(inbuf, fp);
		else
			count++;
	}
	fclose(sfp);
	return 0;
}

//add by bjgyt for combine
//modify by macintosh 050425 for combine_with_attach
void Add_Combine(board,fileinfo)
char *board;
struct fileheader *fileinfo;
{
	FILE *fp, *fp1;
	char buf[STRLEN];
	char temp2[200];

	sprintf(buf,"tmp/%s.combine",currentuser.userid);
	fp=fopen(buf,"at");
	fprintf(fp,"\033[1;32m☆──────────────────────────────────────☆\033[0;1m\n");

	int blankline=0;

	setbfile(buf, board, fh2fname(fileinfo));		//modify by mintbaggio 040321 for heji
	fp1=fopen(buf, "rt");
	if (fgets(temp2, 200, fp1)!=NULL){
		keepoldheader(fp1, SKIPHEADER);
		fprintf(fp, "    \033[0;1;32m%s \033[0;1m于 \033[1;36m%s\033[0;1m 提到：\033[0m\n", fh2owner(fileinfo),
				ytht_ctime(fileinfo->filetime));
		while (!feof(fp1)) {
			fgets(temp2, 200, fp1);
			if (transferattach(temp2, 200, fp1, fp))
				continue;
			if ((unsigned)*temp2<'\x1b'){
				if (blankline) continue;
				else blankline=1;
			}
			else blankline=0;
			if (!strncmp(temp2, "发信人:", 7)||!strncmp(temp2, "标  题:", 7)||!strncmp(temp2, "发信站:", 7))
				continue;
			if (!strncmp(temp2, ": ", 2))
				continue;
			if (!strcmp(temp2, "--\n") || !strcmp(temp2, "--\r\n"))
				break;
			fputs(temp2, fp);

		}
	}
	fclose(fp1);
	fprintf(fp,"\n" );
	fclose(fp);
}

//add by mintbaggio 040326 for front page commend
static int commend_article(char* board, struct fileheader* fileinfo) {
	struct commend* x;
	int offset;

	x = (struct commend*) malloc (sizeof(struct commend));
	if((offset=is_in_commend(board, fileinfo))){	//if the file has in commend file list
		del_commend(offset);			//then delete it
	}
	else{
		if(count_commend() >= 20){
			move(t_lines-2, 0);
			prints("您已经推荐了20篇文章啦！不要太贪心哦~~~\n");
			pressreturn();
		}
		else{
			char fname[STRLEN];
			do_commend(board, fileinfo);
			setbfile(fname, currboard, fh2fname(fileinfo));
			postfile(fname, "Commend", fileinfo->title, 0);
		}
	}
	free(x);
	return 0;
}

//add by mintbaggio 040326 for front page commend
//if is, return offset, else 0
int is_in_commend(char* board, struct fileheader* fileinfo) {
	FILE *fp;
	struct commend x;
	int offset;

	fp=fopen(COMMENDFILE, "r");
	if(!fp)
		return 0;

	while(1){
		if(fread(&x, sizeof(struct commend), 1, fp)<=0)
			break;

		x.board[sizeof(x.board) - 1] = 0;
		x.filename[sizeof(x.filename) - 1] = 0;
		if(!strcmp(board, x.board) && !strcmp(fh2fname(fileinfo), x.filename)){
			offset = ftell(fp);
			fclose(fp);
			return offset;
		}
	}
	fclose(fp);
	return 0;
}

//add by mintbaggio 040326 for front page commend
static int del_commend(int offset) {
	FILE *fp, *fp2;
	struct commend x;

	fp = fopen(COMMENDFILE, "r");
	fp2 = fopen(COMMENDFILE".new", "w");
	if(!fp || !fp2) {
		if (fp) fclose(fp);
		if (fp2) fclose(fp2);
		return 1;
	}
	//prints("offset=%d\n");
	//pressanykey();
	while(fread(&x, sizeof(struct commend), 1, fp) == 1){
		if((ftell(fp)==offset) || (labs(time(NULL)-x.time)>7*86400))
			continue;	//超过7天，自动失效。
		fwrite(&x, sizeof(struct commend), 1, fp2);
	}
	fclose(fp);
	fclose(fp2);
	unlink(COMMENDFILE);
	link(COMMENDFILE".new", COMMENDFILE);
	unlink(COMMENDFILE".new");
	return 0;
}

//add by mintbaggio 040326 for front page commend
static int do_commend(char* board, struct fileheader* fileinfo) {
	FILE *fp;
	struct commend y;
	bzero(&y, sizeof(struct commend));

	fp=fopen(COMMENDFILE, "a");
	if(!fp)
		return 0;
	ytht_strsncpy(y.userid, fileinfo->owner, 13);
	ytht_strsncpy(y.com_user, currentuser.userid, 13);
	ytht_strsncpy(y.title, fileinfo->title, 80);
	ytht_strsncpy(y.board, currboard, 24);
	ytht_strsncpy(y.filename, fh2fname(fileinfo), 80);
	y.accessed=fileinfo->accessed;
	y.time=time(NULL);
	if(fwrite(&y, sizeof(struct commend), 1, fp) != 1){
		prints("write fail\n");
		pressanykey();
		if (fp) fclose(fp);
		return 0;
	}
	fclose(fp);
	return 1;
}

//add by mintbaggio 040327 for front page commend
static int count_commend() {
	FILE *fp;
	struct commend x;
	int count = 0;

	fp = fopen(COMMENDFILE, "r");
	if(!fp)
		return 0;
	while(1){
		if(fread(&x, sizeof(struct commend), 1, fp)<=0)
			break;
		x.com_user[sizeof(x.com_user) - 1] = 0;
		if(!strcmp(x.com_user, currentuser.userid))
			count++;
	}
	fclose(fp);
	return count;
}

//add by mintbaggio 040331 for front page commend
int show_commend() {
	int num;
	FILE *fp;
	struct commend x;

	clear();
	fp = fopen(COMMENDFILE, "r");
	if(!fp)
	{
		prints("无法打开推荐文件，请与系统管理员联系\n");
		return -1;
	}
	fseek(fp, -20*sizeof(struct commend), SEEK_END);
	prints("                \033[1;34m-------\033[37m=======\033[41m BMY推荐文章 \033[40m=======\033[34m-------\033[0m\n\n");
	for(num=20; num>0; num--){
		if(fread(&x, sizeof(struct commend), 1, fp) != 1)
			break;
		//prints("\033[37m信区:\033[33m%-16s\033[37m标题:\033[1;44;37m%-60.60s\033[40m\033[37m【推荐时间:\033[32m%s\033[37m,推荐人:\033[32m%s\033[37m】\033[0m\n", x.board, x.title, ytht_ctime(x.time), x.com_user);
		prints("\033[1;44;37m信区:\033[33m%-13s\033[37m标题:\033[37m%-30s \033[37m作者:\033[32m%-12s\033[0m\n", x.board, x.title, x.userid);
	}
	fclose(fp);
	return 0;
}

//add by mintbaggio 040326 for front page commend
static int commend_article2(char* board, struct fileheader* fileinfo) {
	struct commend* x;
	int offset;

	x = (struct commend*) malloc (sizeof(struct commend));
	if((offset=is_in_commend2(board, fileinfo))){	//if the file has in commend file list
		del_commend2(offset);			//then delete it
	}
	else{
		if(count_commend2() >= 15){
			move(t_lines-2, 0);
			prints("您已经推荐了15篇文章啦！不要太贪心哦~~~\n");
			pressreturn();
		}
		else{
			char fname[STRLEN];
			do_commend2(board, fileinfo);
			setbfile(fname, currboard, fh2fname(fileinfo));
			postfile(fname, "Commend", fileinfo->title, 0);
//			do_commend2(board, fileinfo);
		}
	}
	free(x);
	return 0;
}

//add by mintbaggio 040326 for front page commend
//if is, return offset, else 0
int is_in_commend2(char* board, struct fileheader* fileinfo) {
	FILE *fp;
	struct commend x;
	int offset;

	fp=fopen(COMMENDFILE2, "r");
	if(!fp)
		return 0;

	while(1){
		if(fread(&x, sizeof(struct commend), 1, fp)<=0)
			break;

		x.board[sizeof(x.board) - 1] = 0;
		x.filename[sizeof(x.filename) - 1] = 0;
		if(!strcmp(board, x.board) && !strcmp(fh2fname(fileinfo), x.filename)){
			offset = ftell(fp);
			fclose(fp);
			return offset;
		}
	}
	fclose(fp);
	return 0;
}

//add by mintbaggio 040326 for front page commend
static int del_commend2(int offset) {
	FILE *fp, *fp2;
	struct commend x;

	fp = fopen(COMMENDFILE2, "r");
	fp2 = fopen(COMMENDFILE2".new", "w");
	if(!fp || !fp2) {
		if (fp) fclose(fp);
		if (fp2) fclose(fp2);
		return 1;
	}
	//prints("offset=%d\n");
	//pressanykey();
	while(fread(&x, sizeof(struct commend), 1, fp) == 1){
		if((ftell(fp)==offset) || (labs(time(NULL)-x.time)>7*86400))
			continue;	//超过7天，自动失效。
		fwrite(&x, sizeof(struct commend), 1, fp2);
	}
	fclose(fp);
	fclose(fp2);
	unlink(COMMENDFILE2);
	link(COMMENDFILE2".new", COMMENDFILE2);
	unlink(COMMENDFILE2".new");
	return 0;
}

//add by mintbaggio 040326 for front page commend
static int do_commend2(char* board, struct fileheader* fileinfo) {
	FILE *fp;
	struct commend y;
	bzero(&y, sizeof(struct commend));

	fp=fopen(COMMENDFILE2, "a");
	if(!fp)
		return 0;
	ytht_strsncpy(y.userid, fileinfo->owner, 13);
	ytht_strsncpy(y.com_user, currentuser.userid, 13);
	ytht_strsncpy(y.title, fileinfo->title, 80);
	ytht_strsncpy(y.board, currboard, 24);
	ytht_strsncpy(y.filename, fh2fname(fileinfo), 80);
	y.accessed=fileinfo->accessed;
	y.time=time(NULL);
	if(fwrite(&y, sizeof(struct commend), 1, fp) != 1){
		fclose(fp);
		prints("write fail\n");
		pressanykey();
		return 0;
	}
	fclose(fp);
	return 1;
}

//add by mintbaggio 040327 for front page commend
static int count_commend2() {
	FILE *fp;
	struct commend x;
	int count = 0;

	fp = fopen(COMMENDFILE2, "r");
	if(!fp)
		return 0;
	while(1){
		if(fread(&x, sizeof(struct commend), 1, fp)<=0)
			break;
		x.com_user[sizeof(x.com_user) - 1] = 0;
		if(!strcmp(x.com_user, currentuser.userid))
			count++;
	}
	fclose(fp);
	return count;
}

//add by mintbaggio 040331 for front page commend
int show_commend2() {
	int num;
	FILE *fp;
	struct commend x;

	clear();
	fp = fopen(COMMENDFILE2, "r");
	if(!fp)
	{
		prints("无法打开推荐文件，请与系统管理员联系\n");
		return -1;
	}
	fseek(fp, -20*sizeof(struct commend), SEEK_END);
	prints("                \033[1;34m-------\033[37m=======\033[41m BMY推荐文章 \033[40m=======\033[34m-------\033[0m\n\n");
	for(num=20; num>0; num--){
		if(fread(&x, sizeof(struct commend), 1, fp) != 1)
			break;
		//prints("\033[37m信区:\033[33m%-16s\033[37m标题:\033[1;44;37m%-60.60s\033[40m\033[37m【推荐时间:\033[32m%s\033[37m,推荐人:\033[32m%s\033[37m】\033[0m\n", x.board, x.title, ytht_ctime(x.time), x.com_user);
		prints("\033[1;44;37m信区:\033[33m%-13s\033[37m标题:\033[37m%-30s \033[37m作者:\033[32m%-12s\033[0m\n", x.board, x.title, x.userid);
	}
	fclose(fp);
	return 0;
}
