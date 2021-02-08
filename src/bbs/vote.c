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
	Copyright (C) 1999, Zhou Lin, kcn@cic.tsinghua.edu.cn

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 1, or (at your option)
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include <sys/mman.h>
#include "bbs.h"
#include "vote.h"
#include "common.h"
#include "bbsinc.h"
#include "smth_screen.h"
#include "term.h"
#include "stuff.h"
#include "record.h"
#include "io.h"
#include "edit.h"
#include "more.h"
#include "talk.h"
#include "announce.h"
#include "maintain.h"
#include "mail.h"
#include "xyz.h"
#include "namecomplete.h"
#include "help.h"
#include "list.h"
#include "main.h"
#include "bbs_global_vars.h"
#include "bbs-internal.h"
#include "ythtbbs/cache.h"
#include "bcache.h"

extern int page, range;
extern char IScurrBM;
static char *const vote_type[] = { "是非", "单选", "复选", "数字", "问答" ,"限定票数复选"};
struct votebal currvote;
char controlfile[STRLEN];
static unsigned int vote_result[33];
int vnum;
int voted_flag;
FILE *sug;
int multivotestroll;
struct voterlist* vlists[MAX_VOTERLIST_NUM];
int listsnum=0;
int currlist;

static int cmpvuid(char *userid, struct ballot *uv);
static void setvoteflag(char *bname, int flag);
static void setcontrolfile(void);
static int count_result(struct ballot *ptr);
static int count_log(struct votelog *ptr);
static void get_result_title(void);
static int compareip(struct votelog *a, struct votelog *b);
static void mk_result(void);
static int get_vitems(struct votebal *bal);
static int vote_check(int bits);
static int showvoteitems(unsigned int pbits, int i, int flag);
static void show_voteing_title(void);
static int getsug(struct ballot *uv);
static void strollvote(struct ballot *uv, struct votebal *vote, int s);
static int multivote(struct ballot *uv);
static int smultivote(struct ballot *uv);
static int valuevote(struct ballot *uv);
static int valid_voter(char *board, char *name, char* listfname);
static void user_vote(int num);
static void voteexp(void);
static int printvote(struct votebal *ent);
static void dele_vote(void);
static int vote_results(char *bname);
static void vote_title(void);
static int vote_key(int ch, int allnum, int pagenum);
static int Show_Votes(void);
static int b_suckinfile(FILE * fp, char *fname);
static int choose_voter_list(char* ballistname, int listnum);
static int voterlist_show();
static int add_list();
static int save_list();
static int voter(int listnum);


static int
cmpvuid(userid, uv)
char *userid;
struct ballot *uv;
{
	return !strcmp(userid, uv->uid);
}

static void
setvoteflag(bname, flag)
char *bname;
int flag;
{
	int pos;
	struct boardheader fh;

	pos = new_search_record(BOARDS, &fh, sizeof (fh), (void *) cmpbnames, bname);
	if (flag == 0)
		fh.flag = fh.flag & ~VOTE_FLAG;
	else
		fh.flag = fh.flag | VOTE_FLAG;
	if (substitute_record(BOARDS, &fh, sizeof (fh), pos) == -1)
		prints("Error updating BOARDS file...\n");
	ythtbbs_cache_Board_resolve();
}

void
makevdir(bname)
char *bname;
{
	struct stat st;
	char buf[STRLEN];

	sprintf(buf, "vote/%s", bname);
	if (stat(buf, &st) != 0)
		mkdir(buf, 0777);
}

void
setvfile(buf, bname, filename)
char *buf, *bname, *filename;
{
	sprintf(buf, "vote/%s/%s", bname, filename);
}

static void setvfile_s (char *buf, size_t len, char *bname, char *filename) {
	snprintf(buf, len, "vote/%s/%s", bname, filename);
}

static void
setcontrolfile()
{
	setvfile(controlfile, currboard, "control");
}

static int
b_suckinfile(fp, fname)
FILE *fp;
char *fname;
{
	char inbuf[256];
	FILE *sfp;
	if ((sfp = fopen(fname, "r")) == NULL)
		return -1;
	while (fgets(inbuf, sizeof (inbuf), sfp) != NULL)
		fputs(inbuf, fp);
	fclose(sfp);
	sfp = 0;
	return 0;
}

int
b_closepolls()
{
	char buf[80];
	time_t now, nextpoll;
	int i, end;

	now = time(NULL);
	ythtbbs_cache_Board_resolve();

	if (now < ythtbbs_cache_Board_get_pollvote()) {
		return 0;
	}

	move(t_lines - 1, 0);
	prints("对不起，系统关闭投票中，请稍候...");
	refresh();

	nextpoll = now + 7 * 3600;

	strcpy(buf, currboard);
	// TODO
	for (i = 0; i < ythtbbs_cache_Board_get_number(); i++) {
		setcontrolfile();
		end = get_num_records(controlfile, sizeof (currvote));
		for (vnum = end; vnum >= 1; vnum--) {
			time_t closetime;
			get_record(controlfile, &currvote, sizeof (currvote), vnum);
			closetime = currvote.opendate + currvote.maxdays * 86400;
			if (now > closetime)
				mk_result();
			else if (nextpoll > closetime)
				nextpoll = closetime + 300;
		}
	}
	strcpy(currboard, buf);
	ythtbbs_cache_Board_set_pollvote(nextpoll);
	return 0;
}

static int
count_result(ptr)
struct ballot *ptr;
{
	int i;

	if (ptr == NULL) {
		if (sug != NULL) {
			fclose(sug);
			sug = NULL;
		}
		return 0;
	}
	if (ptr->msg[0][0] != '\0') {
		if (currvote.type == VOTE_ASKING) {
			fprintf(sug, "\033[1m%.12s \033[m的作答如下：\n", ptr->uid);
		} else
			fprintf(sug, "\033[1m%.12s \033[m的建议如下：\n", ptr->uid);
		for (i = 0; i < 3; i++)
			fprintf(sug, "%s\n", ptr->msg[i]);
	}
	vote_result[32]++;
	if (currvote.type == VOTE_ASKING) {
		return 0;
	}
	if (currvote.type != VOTE_VALUE) {
		for (i = 0; i < 32; i++) {
			if ((ptr->voted >> i) & 1)
				(vote_result[i])++;
		}

	} else {
		vote_result[31] += ptr->voted;
		vote_result[(ptr->voted * 10) / (currvote.maxtkt + 1)]++;
	}
	return 0;
}

static int
count_log(ptr)
struct votelog *ptr;
{
	if (ptr == NULL) {
		if (sug != NULL) {
			fclose(sug);
			sug = NULL;
		}
		return 0;
	}
	fprintf(sug, "%12s   %16s   %s %s\n", ptr->uid, ptr->ip,
			ytht_ctime(ptr->votetime), (ptr->voted == 0) ? "0" : "");
	return 0;
}

static void
get_result_title()
{
	char buf[STRLEN * 2];

	fprintf(sug, "⊙ 投票开启于：\033[1m%.24s\033[m  类别：\033[1m%s\033[m\n",
		ctime(&currvote.opendate), vote_type[currvote.type - 1]);
	fprintf(sug, "⊙ 主题：\033[1m%s\033[m\n", currvote.title);
	if (currvote.type == VOTE_VALUE)
		fprintf(sug, "⊙ 此次投票的值不可超过：\033[1m%d\033[m\n\n", currvote.maxtkt);
	fprintf(sug, "⊙ 票选题目描述：\n\n");
	sprintf(buf, "vote/%s/desc.%ld", currboard, currvote.opendate);
	b_suckinfile(sug, buf);
}

static int
compareip(a, b)
struct votelog *a, *b;
{
	return cmpIP(a->ip, b->ip);
}

static int get_board_by_name(struct boardmem *board, int curr_idx, va_list ap) {
	const char *name = va_arg(ap, const char *);
	int *idx         = va_arg(ap, int *);
	if (strncmp(name, board->header.filename, STRLEN) == 0) {
		*idx = curr_idx;
		return QUIT;
	}

	return 0;
}

static void
mk_result()
{
	char fname[STRLEN * 2], nname[STRLEN * 2];
	char sugname[STRLEN * 2];
	char logfname[STRLEN * 2], sortedlogfname[STRLEN * 2];
	char title[STRLEN * 2];
	int i;
	int postout = 0;
	unsigned int total = 0;

	setcontrolfile();
	sprintf(fname, "vote/%s/flag.%ld", currboard, currvote.opendate);
	count_result(NULL);
	sprintf(sugname, "vote/%s/tmp.%d", currboard, uinfo.pid);

	if ((sug = fopen(sugname, "w")) == NULL) {
		errlog("open vote tmp file error %d", errno);
		prints("Error: 结束投票错误...\n");
		pressanykey();
		return;
	}
	(void) memset(vote_result, 0, sizeof (vote_result));
	if (apply_record(fname, (void *) count_result, sizeof (struct ballot)) == -1) {
		errlog("Vote apply flag error");
	}
	fprintf(sug,
		"\033[1;44;36m——————————————┤使用者%s├——————————————\033[m\n\n\n",
		(currvote.type != VOTE_ASKING) ? "建议或意见" : "此次的作答");
	fclose(sug);
	sprintf(nname, "vote/%s/results", currboard);
	if ((sug = fopen(nname, "w")) == NULL) {
		errlog("open vote newresult file error %d", errno);
		prints("Error: 结束投票错误...\n");
		return;
	}

	get_result_title();

	fprintf(sug, "** 投票结果:\n\n");
	if (currvote.type == VOTE_VALUE) {
		total = vote_result[32];
		for (i = 0; i < 10; i++) {
			fprintf(sug,
				"\033[1m  %4d\033[m 到 \033[1m%4d\033[m 之间有 \033[1m%4d\033[m 票  约占 \033[1m%d%%\033[m\n",
				(i * currvote.maxtkt) / 10 + ((i == 0) ? 0 : 1),
				((i + 1) * currvote.maxtkt) / 10, vote_result[i]
				,
				(vote_result[i] * 100) / ((total <= 0) ? 1 : total));
		}
		fprintf(sug, "此次投票结果平均值是: \033[1m%d\033[m\n", vote_result[31] / ((total <= 0) ? 1 : total));
	} else if (currvote.type == VOTE_ASKING) {
		total = vote_result[32];
	} else {
		for (i = 0; i < currvote.totalitems; i++) {
			total += vote_result[i];
		}
		for (i = 0; i < currvote.totalitems; i++) {
			fprintf(sug, "(%c) %-40s  %4d 票  约占 \033[1m%d%%\033[m\n",
				'A' + i, currvote.items[i], vote_result[i],
				(vote_result[i] * 100) / ((total <= 0) ? 1 : total));
		}
	}
	fprintf(sug, "\n投票总人数 = \033[1m%d\033[m 人\n", vote_result[32]);
	fprintf(sug, "投票总票数 =\033[1m %d\033[m 票\n\n", total);
	fprintf(sug,
		"\033[1;44;36m——————————————┤使用者%s├——————————————\033[m\n\n\n",
		(currvote.type != VOTE_ASKING) ? "建议或意见" : "此次的作答");
	b_suckinfile(sug, sugname);
	unlink(sugname);
	fclose(sug);

	sug = NULL;

	ythtbbs_cache_Board_resolve();
	ythtbbs_cache_Board_foreach_v(get_board_by_name, currboard, &i);
	if (i != ythtbbs_cache_Board_get_number())
		if (normal_board(currboard)) {
			if (ythtbbs_cache_Board_get_board_by_idx(i)->header.clubnum == 0)
				postout = 1;
			else if (ythtbbs_cache_Board_get_board_by_idx(i)->header.flag & CLUBTYPE_FLAG)
				postout = 1;
		}
	if (currvote.flag & VOTE_FLAG_OPENED) {
		char *mem = NULL;
		struct stat buf;
		int fd;

		sprintf(fname, "vote/%s/newlog.%ld", currboard, (long int) currvote.opendate);
		sprintf(logfname, "vote/%s/log", currboard);
		if ((sug = fopen(logfname, "w")) == NULL) {
			errlog("open vote tmp file error %d", errno);
			prints("Error: 结束投票错误...\n");
			pressanykey();
			return;
		}
		fprintf(sug, "%12s   %16s   %24s\n", "ID", "IP", "投票时间");
		apply_record(fname, (void *) count_log, sizeof (struct votelog));
		fclose(sug);
		if ((fd = open(fname, O_RDWR, 0644)) != -1) {
			flock(fd, LOCK_EX);
			fstat(fd, &buf);
			MMAP_TRY {
				mem = mmap(0, buf.st_size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0);
				qsort(mem, buf.st_size / sizeof (struct votelog), sizeof (struct votelog), (void *) compareip);
			}
			MMAP_CATCH {
			}
			MMAP_END munmap(mem, buf.st_size);
			flock(fd, LOCK_UN);
			close(fd);
		}
		sprintf(sortedlogfname, "vote/%s/slog", currboard);
		if ((sug = fopen(sortedlogfname, "w")) == NULL) {
			errlog("open vote tmp file error %d", errno);
			prints("Error: 结束投票错误...\n");
			pressanykey();
			return;
		}
		fprintf(sug, "%12s   %16s   %24s\n", "ID", "IP", "投票时间");
		apply_record(fname, (void *) count_log, sizeof (struct votelog));
		fclose(sug);
		sug = NULL;
	}
	if (postout) {
		sprintf(title, "[公告] %s 版的投票结果", currboard);
		postfile(nname, "vote", title, 1);
		if (currvote.flag & VOTE_FLAG_OPENED) {
			sprintf(title, "[公告] %s 版的投票参与情况", currboard);
			postfile(logfname, "vote", title, 1);
			sprintf(title, "[公告] %s 版的投票参与情况(by IP)", currboard);
			postfile(sortedlogfname, "vote", title, 1);
		}
	}
	if (strncmp(currboard, "vote", STRLEN)) {
		sprintf(title, "[公告] %s 版的投票结果", currboard);
		postfile(nname, currboard, title, 1);
		if (currvote.flag & VOTE_FLAG_OPENED) {
			sprintf(title, "[公告] %s 版的投票参与情况", currboard);
			postfile(logfname, currboard, title, 1);
			sprintf(title, "[公告] %s 版的投票参与情况(by IP)", currboard);
			postfile(sortedlogfname, currboard, title, 1);
		}

	}
	dele_vote();
	return;
}

static int
get_vitems(bal)
struct votebal *bal;
{
	int num;
	char buf[STRLEN];

	move(3, 0);
	prints("请依序输入可选择项, 按 ENTER 完成设定.\n");
	num = 0;
	for (num = 0; num < 32; num++) {
		sprintf(buf, "%c) ", num + 'A');
		getdata((num % 16) + 4, (num / 16) * 40, buf, bal->items[num], 36, DOECHO, YEA);
		if (strlen(bal->items[num]) == 0) {
			if (num != 0)
				break;
			num = -1;
		}

	}
	bal->totalitems = num;
	return num;
}

int
vote_maintain(bname)
char *bname;
{
	char buf[STRLEN * 4];
	struct votebal *ball = &currvote;
	int aborted;

	setcontrolfile();
	if (!HAS_PERM(PERM_OVOTE, currentuser))
		if (!IScurrBM) {
			return 0;
		}
	stand_title("开启投票箱");
	makevdir(bname);
	for (;;) {
		getdata(2, 0,
			"(1)是非, (2)单选, (3)复选, (4)数值 (5)问答(6)限定票数复选  (7)取消 ? : ",
			genbuf, 2, DOECHO, YEA);
		genbuf[0] -= '0';
		if (genbuf[0] == 7) {
			prints("取消此次投票\n");
			sleep(1);
			return FULLUPDATE;
		}
		if (genbuf[0] < 1 || genbuf[0] > 7)
			continue;
		ball->type = (int) genbuf[0];
		break;
	}
	if (askyn("此次投票结果是否公开?", NA, NA))
		ball->flag |= VOTE_FLAG_OPENED;
	else
		ball->flag &= ~VOTE_FLAG_OPENED;
	ball->flag &= ~VOTE_FLAG_LIMITED;
	if (HAS_PERM(PERM_SYSOP, currentuser) || seek_in_file(MY_BBS_HOME"/etc/voteidboards", currboard)) {
		if (askyn("此次投票是否限制投票人? (需要先在版面按V生成名单)", NA, NA)) {
			ball->flag |= VOTE_FLAG_LIMITED;
			getdata(5, 0, "选用名单的序号: ", buf, 4, DOECHO, YEA);
			if (*buf == '\n' || atoi(buf) == 0 || *buf == '\0')
				strcpy(buf, "1");
			if (choose_voter_list(ball->listfname, atoi(buf))==-1)
				return DONOTHING;
		} else
			ball->flag &= ~VOTE_FLAG_LIMITED;
	}
	ball->opendate = time(NULL);
	prints("请按任何键开始编辑此次 [投票的描述]: \n");
	igetkey();
	char tmp_buf[STRLEN];
	setvfile_s(tmp_buf, sizeof(tmp_buf), bname, "desc");
	sprintf(buf, "%s.%ld", tmp_buf, ball->opendate);

	aborted = vedit(buf, NA, YEA);
	if (aborted) {
		clear();
		prints("取消此次投票\n");
		pressreturn();
		return FULLUPDATE;
	}

	clear();
	getdata(0, 0, "此次投票所须天数 (不可０天): ", buf, 4, DOECHO, YEA);

	if (*buf == '\n' || atoi(buf) == 0 || *buf == '\0')
		strcpy(buf, "1");

	ball->maxdays = atoi(buf);
	if (999 == ball->maxdays) {
		prints("真是变态...\n");
		pressanykey();
	}
	for (;;) {
		getdata(1, 0, "投票箱的标题: ", ball->title, 61, DOECHO, YEA);
		if (strlen(ball->title) > 0)
			break;
		bell();
	}
	switch (ball->type) {
	case VOTE_YN:
		ball->maxtkt = 0;
		strcpy(ball->items[0], "赞成  （是的）");
		strcpy(ball->items[1], "不赞成（不是）");
		strcpy(ball->items[2], "没意见（不清楚）");
		ball->maxtkt = 1;
		ball->totalitems = 3;
		break;
	case VOTE_SINGLE:
		get_vitems(ball);
		ball->maxtkt = 1;
		break;
	case VOTE_MULTI:
		get_vitems(ball);
		for (;;) {
			getdata(21, 0, "一个人最多几票? [1]: ", buf, 5, DOECHO, YEA);
			ball->maxtkt = atoi(buf);
			if (ball->maxtkt <= 0)
				ball->maxtkt = 1;
			if (ball->maxtkt > ball->totalitems)
				continue;
			break;
		}
		break;
	case VOTE_SMULTI:
		get_vitems(ball);
		for (;;) {
			getdata(21, 0, "一个人限定几票? [1]: ", buf, 5, DOECHO, YEA);
			ball->maxtkt = atoi(buf);
			if (ball->maxtkt <= 0)
				ball->maxtkt = 1;
			if (ball->maxtkt > ball->totalitems)
				continue;
			break;
		}
		break;
	case VOTE_VALUE:
		for (;;) {
			getdata(3, 0, "输入数值最大不得超过 [100] : ", buf, 4, DOECHO, YEA);
			ball->maxtkt = atoi(buf);
			if (ball->maxtkt <= 0)
				ball->maxtkt = 100;
			break;
		}
		break;
	case VOTE_ASKING:
		ball->maxtkt = 0;
		currvote.totalitems = 0;
		break;
	default:
		ball->maxtkt = 1;
		break;
	}
	setvoteflag(currboard, 1);
	clear();
	strcpy(ball->userid, currentuser.userid);
	if (append_record(controlfile, ball, sizeof (*ball)) == -1) {
		prints("发生严重的错误，无法开启投票，请通告站长");
		errlog("Append Control file Error!! board=%s", currboard);
	} else {
		char votename[STRLEN];
		int i;

		prints("投票箱开启了！\n");
		range++;;
		sprintf(votename, "tmp/votetmp.%s.%05d", currentuser.userid, uinfo.pid);
		if ((sug = fopen(votename, "w")) != NULL) {
			sprintf(buf, "[通知] %s 举办投票：%s", currboard, ball->title);
			get_result_title();
			if (ball->type != VOTE_ASKING && ball->type != VOTE_VALUE) {
				fprintf(sug, "\n【\033[1m选项如下\033[m】\n");
				for (i = 0; i < ball->totalitems; i++) {
					fprintf(sug, "(\033[1m%c\033[m) %-40s\n", 'A' + i, ball->items[i]);
				}
			}
			fclose(sug);
			sug = NULL;
			ythtbbs_cache_Board_resolve();
			ythtbbs_cache_Board_foreach_v(get_board_by_name, currboard, &i);
			if (i != ythtbbs_cache_Board_get_number())
				if (normal_board(currboard)) {
					if (ythtbbs_cache_Board_get_board_by_idx(i)->header.clubnum == 0)
						postfile(votename, "vote", buf, 1);
					else if (ythtbbs_cache_Board_get_board_by_idx(i)->header.flag & CLUBTYPE_FLAG)
						postfile(votename, "vote", buf, 1);
				}
			postfile(votename, currboard, buf, 1);
			unlink(votename);
		}
	}
	pressreturn();
	return FULLUPDATE;
}

static int
vote_check(bits)
int bits;
{
	int i, count;

	for (i = count = 0; i < 32; i++) {
		if ((bits >> i) & 1)
			count++;
	}
	return count;
}

static int
showvoteitems(pbits, i, flag)
unsigned int pbits;
int i, flag;
{
	char buf[STRLEN];
	int count;

	if (flag == YEA) {
		count = vote_check(pbits);
		if (count > currvote.maxtkt)
			return NA;
		move(2, 0);
		clrtoeol();
		prints("您已经投了 \033[1m%d\033[m 票", count);
	}

	sprintf(buf, "%c.%2.2s%-36.36s", 'A' + i,
		((pbits >> i) & 1 ? "□" : "  "), currvote.items[i]);
	move(i + 6 - ((i > 15) ? 16 : 0), 0 + ((i > 15) ? 40 : 0));
	prints(buf);
	return YEA;
}

static void
show_voteing_title()
{
	time_t closedate;
	char buf[STRLEN];

	if (currvote.type != VOTE_VALUE && currvote.type != VOTE_ASKING)
		sprintf(buf, "可投票数: \033[1m%d\033[m 票", currvote.maxtkt);
	else
		buf[0] = '\0';
	closedate = currvote.opendate + currvote.maxdays * 86400;
	prints("投票将结束于: \033[1m%24s\033[m  %s  %s\n", ctime(&closedate), buf, (voted_flag) ? "(\033[5;1m修改前次投票\033[m)" : "");
	prints("投票主题是: \033[1m%-50s\033[m类型: \033[1m%s\033[m \n", currvote.title, vote_type[currvote.type - 1]);
}

static int
getsug(uv)
struct ballot *uv;
{
	int i, line;

	move(0, 0);
	clrtobot();
	if (currvote.type == VOTE_ASKING) {
		show_voteing_title();
		line = 3;
		prints("请填入您的作答(三行):\n");
	} else {
		line = 1;
		prints("请填入您宝贵的意见(三行):\n");
	}
	move(line, 0);
	for (i = 0; i < 3; i++) {
		prints(": %s\n", uv->msg[i]);
	}
	for (i = 0; i < 3; i++) {
		getdata(line + i, 0, ": ", uv->msg[i], STRLEN - 2, DOECHO, NA);
		if (uv->msg[i][0] == '\0')
			break;
	}
	return i;
}

static void
strollvote(struct ballot *uv, struct votebal *vote, int s)
{
	struct ballot nuv;
	struct votebal nvote;
	int i, j;
	memcpy(&nuv, uv, sizeof (nuv));
	memcpy(&nvote, vote, sizeof (nvote));
	nuv.voted = 0;
	for (i = 0; i < vote->totalitems; i++) {
		j = (i + s) % vote->totalitems;
		if (j < 0)
			j += vote->totalitems;
		if (uv->voted & (1 << i))
			nuv.voted |= (1 << j);
		strcpy(nvote.items[j], vote->items[i]);
	}
	memcpy(uv, &nuv, sizeof (nuv));
	memcpy(vote, &nvote, sizeof (nvote));
}

static int
multivote(uv)
struct ballot *uv;
{
	unsigned int i;
	int multivotestroll = time(NULL) % currvote.totalitems;
	i = uv->voted;

	move(0, 0);
	show_voteing_title();

	strollvote(uv, &currvote, multivotestroll);
	if (vote_check(uv->voted) > currvote.maxtkt)	//修正先前bug的错误结果
		uv->voted = 0;
	uv->voted = setperms(uv->voted, "选票", currvote.totalitems, showvoteitems, 1);
	strollvote(uv, &currvote, -multivotestroll);

	if (uv->voted == i)
		return -1;
	return 1;
}

static int
smultivote(uv)
struct ballot *uv;
{
	unsigned int i;
	int multivotestroll = time(NULL) % currvote.totalitems;
	i = uv->voted;
begin:
	clear();
	move(0, 0);
	show_voteing_title();

	strollvote(uv, &currvote, multivotestroll);

	uv->voted = setperms(uv->voted, "选票", currvote.totalitems, showvoteitems, 1);
	strollvote(uv, &currvote, -multivotestroll);
	if (vote_check(uv->voted) != currvote.maxtkt)	{
		clear();
		move(4,0);
		prints("您所投票数与本投票之要求不一致!");
		pressreturn();
		goto begin;

	}
	if (uv->voted == i)
		return -1;
	return 1;
}


static int
valuevote(uv)
struct ballot *uv;
{
	unsigned int chs;
	char buf[10];

	chs = uv->voted;
	move(0, 0);
	show_voteing_title();
	prints("此次作答的值不能超过 \033[1m%d\033[m", currvote.maxtkt);
	if (uv->voted != 0)
		sprintf(buf, "%d", uv->voted);
	else
		memset(buf, 0, sizeof (buf));
	do {
		getdata(3, 0, "请输入一个值? [0]: ", buf, 5, DOECHO, NA);
		uv->voted = abs(atoi(buf));
	} while (uv->voted > currvote.maxtkt && buf[0] != '\n' && buf[0] != '\0');
	if (buf[0] == '\n' || buf[0] == '\0' || uv->voted == chs)
		return -1;
	return 1;
}

static int
valid_voter(char *board, char *name, char* listname)
{
	FILE *in;
	char buf[100];
	int i;

	in = fopen(MY_BBS_HOME "/etc/untrust", "r");
	if (in) {
		while (fgets(buf, 80, in)) {
			i = strlen(buf);
			if (buf[i - 1] == '\n')
				buf[i - 1] = 0;
			if (!strcmp(buf, currentuser.lasthost)) {
				fclose(in);
				return -1;
			}
		}
		fclose(in);
	}
	sprintf(genbuf, "%s/boards/%s/%s", MY_BBS_HOME, board, listname);
	in = fopen(genbuf, "r");
	if (in != NULL) {
		while (fgets(buf, 80, in)) {
			i = strlen(buf);
			if (buf[i - 1] == '\n')
				buf[i - 1] = 0;
			//prints(buf);
			if (!strcmp(buf, name)) {
				fclose(in);
				return 1;
			}
		}
		fclose(in);
	}
	return 0;
}

static void
user_vote(num)
int num;
{
	char fname[STRLEN * 2], bname[STRLEN];
	char buf[STRLEN];
	struct ballot uservote, tmpbal;
	int votevalue;
	int aborted = NA, pos;

	move(t_lines - 2, 0);
	get_record(controlfile, &currvote, sizeof (struct votebal), num);
//add by gluon for sm_vote
	if (!(currentuser.userlevel & PERM_LOGINOK)) {
		prints("对不起, 您还没有通过注册呢\n");
		pressanykey();
		return;
	}
	if (currvote.flag & VOTE_FLAG_LIMITED) {
		int retv = valid_voter(currboard, currentuser.userid, currvote.listfname);
		if (retv == 0 || retv == -1) {
			prints("%s", retv == 0 ? "对不起，您不能参加本次投票\n" : "对不起，您从穿梭站连来，不能投票\n");
			pressanykey();
			return;
		}
	}
//end
	if (currvote.flag & VOTE_FLAG_OPENED) {
		prints("请注意，该投票结束后将公布投票ID、IP、投票时间");
		pressanykey();
	}

	sprintf(fname, "vote/%s/flag.%ld", currboard, currvote.opendate);
	if ((pos = search_record(fname, &uservote, sizeof (uservote), (void *) cmpvuid, currentuser.userid)) <= 0) {
		(void) memset(&uservote, 0, sizeof (uservote));
		voted_flag = NA;
	} else {
		voted_flag = YEA;
	}
	ytht_strsncpy(uservote.uid, currentuser.userid, sizeof(uservote.uid));
	sprintf(bname, "desc.%ld", (long int) currvote.opendate);
	setvfile(buf, currboard, bname);
	ansimore(buf, YEA);
	move(0, 0);
	clrtobot();
	switch (currvote.type) {
	case VOTE_SMULTI:
		votevalue = smultivote(&uservote);
		if (votevalue == -1)
			aborted = YEA;
		break;
	case VOTE_SINGLE:
	case VOTE_MULTI:
	case VOTE_YN:
		votevalue = multivote(&uservote);
		if (votevalue == -1)
			aborted = YEA;
		break;
	case VOTE_VALUE:
		votevalue = valuevote(&uservote);
		if (votevalue == -1)
			aborted = YEA;
		break;
	case VOTE_ASKING:
		uservote.voted = 0;
		aborted = !getsug(&uservote);
		break;
	}
	clear();
	if (aborted == YEA) {
		prints("保留 【\033[1m%s\033[m】原来的的投票。\n", currvote.title);
	} else {
		if (currvote.type != VOTE_ASKING)
			getsug(&uservote);
		pos = search_record(fname, &tmpbal, sizeof (tmpbal), (void *) cmpvuid, currentuser.userid);
		if (pos) {
			substitute_record(fname, &uservote, sizeof (uservote), pos);
		} else if (append_record(fname, &uservote, sizeof (uservote)) == -1) {
			move(2, 0);
			clrtoeol();
			prints("投票失败! 请通知站长参加那一个选项投票\n");
			pressreturn();
		}
		prints("\n已经帮您投入票箱中...\n");
		if (currvote.flag & VOTE_FLAG_OPENED) {
			char votelogfile[STRLEN * 2];
			struct votelog vlog;
			memset(&vlog, 0, sizeof(struct votelog));
			ytht_strsncpy(vlog.uid, currentuser.userid, sizeof(vlog.uid));
			vlog.uid[IDLEN] = 0;
			vlog.votetime = time(NULL);
			vlog.voted = uservote.voted;
			strcpy(vlog.ip, currentuser.lasthost);
			sprintf(votelogfile, "vote/%s/newlog.%ld", currboard, currvote.opendate);
			append_record(votelogfile, &vlog, sizeof (vlog));
		}
		if (!strcmp(currboard, "SM_Election")) {
			int now;
			now = time(NULL);
			sprintf(buf, "%s %s %s", currentuser.userid, currentuser.lasthost, ytht_ctime(now));
			ytht_add_to_file(MY_BBS_HOME "/vote.log", buf);
		}
	}
	pressanykey();
	return;
}

static void
voteexp()
{
	clrtoeol();
	prints("\033[1;44m编号 开启投票箱者 开启日 %-37s   类别 天数 人数\033[m\n", "投票主题");
}

static int
printvote(ent)
struct votebal *ent;
{
	static int i;
	struct ballot uservote;
	char buf[STRLEN + 10], *date;
	char flagname[STRLEN];
	int num_voted;

	if (ent == NULL) {
		move(2, 0);
		voteexp();
		i = 0;
		return 0;
	}
	i++;
	if (i > page + 19 || i > range)
		return QUIT;
	else if (i <= page)
		return 0;
	sprintf(buf, "flag.%ld", (long int) ent->opendate);
	setvfile(flagname, currboard, buf);
	if (search_record(flagname, &uservote, sizeof (uservote), (void *) cmpvuid, currentuser.userid) <= 0) {
		voted_flag = NA;
	} else
		voted_flag = YEA;
	num_voted = get_num_records(flagname, sizeof (struct ballot));
	date = ctime(&ent->opendate) + 4;
	sprintf(buf, " %s%3d %-12.12s %-6.6s %-37.37s %c %-4.4s %3d  %4d\033[m\n",
		(voted_flag == NA) ? "\033[1m" : "", i, ent->userid, date,
		ent->title, ent->flag & VOTE_FLAG_OPENED ? 'O' : ' ',
		vote_type[ent->type - 1], ent->maxdays, num_voted);
	prints("%s", buf);
	return 0;
}

static void
dele_vote()
{
	char buf[STRLEN * 2];
	int num = 1;
	struct votebal tmpvote;
	while (get_record(controlfile, &tmpvote, sizeof (struct votebal), num) == 0) {
		if (currvote.opendate == tmpvote.opendate) {
			if (delete_record(controlfile, sizeof (currvote), num) == -1) {
				prints("发生错误，请通知站长....");
				pressanykey();
			}
			range--;
			sprintf(buf, "vote/%s/flag.%ld", currboard, currvote.opendate);
			unlink(buf);
			sprintf(buf, "vote/%s/desc.%ld", currboard, currvote.opendate);
			unlink(buf);
			sprintf(buf, "vote/%s/newlog.%ld", currboard, currvote.opendate);
			unlink(buf);
			break;
		}
		num++;
	}
	if (get_num_records(controlfile, sizeof (currvote)) == 0) {
		setvoteflag(currboard, 0);
	}
}

static int
vote_results(bname)
char *bname;
{
	char buf[STRLEN];
	setvfile(buf, bname, "results");
	if (ansimore(buf, YEA) == -1) {
		move(3, 0);
		prints("目前没有任何投票的结果。\n");
		clrtobot();
		pressreturn();
	} else
		clear();
	return FULLUPDATE;
}

int
b_vote_maintain()
{
	return vote_maintain(currboard);
}

static void
vote_title()
{

	docmdtitle("[投票箱列表]", "[\033[1;32m←\033[m,\033[1;32me\033[m] 离开 [\033[1;32mh\033[m] 求助 [\033[1;32m→\033[m,\033[1;32mr <cr>\033[m] 进行投票 [\033[1;32m↑\033[m,\033[1;32m↓\033[m] 上,下选择 \033[1m高亮度\033[m表示尚未投票");
	update_endline();
}

static int
addvoter(uident)
char *uident;
{
	char buf[80];
	int id;
	int i;
	int seek;
	if (!(id = getuser(uident))) {
		move(3, 0);
		prints("Invalid User Id");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	setbfile(buf, currboard, "validlist");
	seek = seek_in_file(buf, uident);
	if (seek) {
		move(2, 0);
		prints("输入的ID 已经存在!");
		pressreturn();
		return -1;
	}
	setbfile(buf, currboard, vlists[currlist]->listfname);
	if ((i = getbnum(currboard)) == 0)
		return DONOTHING;
	vlists[currlist]->voternum++;
	return ytht_add_to_file(buf, uident);

}

static int
delvoter(uident)
char *uident;
{
	char fn[STRLEN];
	int id;
	int i;
	if (!(id = getuser(uident))) {
		move(3, 0);
		prints("Invalid User Id");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	if ((i = getbnum(currboard)) == 0)
		return DONOTHING;
	setbfile(fn, currboard, vlists[currlist]->listfname);
	if (vlists[currlist]->voternum>0)
		vlists[currlist]->voternum--;
	return ytht_del_from_file(fn, uident, true);
}

static int
init_lists()
{
	int fn;
	int listnum=0;
	char listbuf[80];
	int index;
	for (index=0; index<MAX_VOTERLIST_NUM; ++index)
		vlists[index]=NULL;
	struct voterlist* vltemp=(struct voterlist*)malloc(sizeof(struct voterlist));
	setbfile(listbuf, currboard, "validlist");
	fn = open( listbuf, O_RDONLY );
	if (fn == -1) {
		free(vltemp);
		return 0;
	}
	while(read(fn, vltemp, sizeof(struct voterlist))==sizeof(struct voterlist))
	{
		vlists[listnum]=(struct voterlist*)malloc(sizeof(struct voterlist));
		memcpy(vlists[listnum], vltemp, sizeof(struct voterlist));
		++listnum;
		if (listnum>=MAX_VOTERLIST_NUM)
			break;
	}
	free(vltemp);
	close(fn);
	return listnum;
}

static void
free_lists()
{
	int index;
	for (index=0; index<MAX_VOTERLIST_NUM; ++index) {
		if (vlists[index]!=NULL)
			free(vlists[index]);
		vlists[index]=NULL;
	}
}

static void
list_refresh()
{
	docmdtitle("[投票名单设置]", "添加[\x1b[1;32ma\x1b[0;37m] 删除[\x1b[1;32md\x1b[0;37m]\x1b[m 改名[\033[1;32mt\033[0;37m]");
	move(2, 0);
	prints("\033[0;1;37;44m %4s %-13s %-44s %10s", "序号", "创建人","名单名称","投票人个数");
	clrtoeol();
	update_endline();
}

static int
voter_key(int key, int allnum, int pagenum)
{
	char titlebuf[STRLEN * 4];
	switch(key) {
		case 'a':
		case 'A':
		{
			char ans[STRLEN];
			if( listsnum>=MAX_VOTERLIST_NUM) {
				move(t_lines - 1, 0);
				clrtoeol();
				a_prompt(-1, "名单数目已满，按回车继续...", ans, 2);
				move(t_lines - 1, 0);
				clrtoeol();
				return -1;
			}
			add_list();
			save_list();
			a_prompt(-1, "名单添加成功，按回车继续...", ans, 2);
			return -1;
		}
			break;
		case 'd':
		case 'D':
		{
			if (askyn("确实要删除吗", NA, YEA)) {
				sprintf(titlebuf,"%s在%s版面删除投票名单:%s",
					currentuser.userid, currboard, vlists[allnum]->listname);
				free(vlists[allnum]);
				vlists[allnum]=NULL;
				deliverreport(titlebuf, "");
				save_list();
				--listsnum;
				return -1;
			}
			return -1;
		}
			break;
		case 't':
		case 'T':
		{
			char newtitle[60];
			ytht_strsncpy(newtitle, vlists[allnum]->listname, sizeof(newtitle));
			getdata(t_lines - 1, 0, "新标题: ", newtitle, 50, DOECHO, NA);
			if( newtitle[0] == '\0' || newtitle[0]=='\n' || !strcmp(newtitle,vlists[allnum]->listname) )
				return 1;
			strncpy(vlists[allnum]->listname, newtitle, 50);
			vlists[allnum]->listname[49]='\0';
			save_list();
			voterlist_show();
			return 1;
		}
			break;
		default:
			break;
	}
	return 0;
}


static int
voter_list_select(int star, int curr)
{
	currlist = curr;
	voter(currlist);
	return DOQUIT;
}

int
m_voter()
{
	int votelist;
	int i;
	int IScurrSYS=currentuser.userlevel & PERM_SYSOP;
	int ISvoteBOARD=seek_in_file(MY_BBS_HOME"/etc/voteidboards", currboard);
	if (!IScurrSYS && (!IScurrBM || !ISvoteBOARD))
		return DONOTHING;
	if ((i = getbnum(currboard)) == 0)
		return DONOTHING;
	listsnum=init_lists();
	setlistrange(listsnum);
	if(range == 0 ){
		clear();
		if (!askyn("本版现在没有投票id名单，需要现在增加吗", NA, YEA)){
			free_lists();
			return FULLUPDATE;
		}
		if( add_list() < 0 ){
			free_lists();
			return FULLUPDATE;
		}
	}
	clear();
	votelist =choose(NA, 0, list_refresh, voter_key,voterlist_show, voter_list_select);
	free_lists();
	return FULLUPDATE;
}

static int
choose_voter_list(char* ballistname, int listnum)
{
	listsnum=init_lists();
	if (listnum<1 || listnum>listsnum)
		return -1;
	strcpy(ballistname, vlists[listnum-1]->listfname);
	free_lists();
	return 0;
}

static int
voterlist_show(){
	int i;
	setlistrange(listsnum);
	if (range < 1)
		return -1;
	clrtoeol();
	for (i = 0; i < MAX_VOTERLIST_NUM; i++) {
		if (vlists[i]==NULL)
			continue;
		move(i+3, 0);
		prints(" %4d %-13s %-46s %3d", i+1, vlists[i]->authorid, vlists[i]->listname, vlists[i]->voternum);
	}
	clrtobot();
	update_endline();
	return 0;
}

static int
compare_fname(char* fname)
{
	int i;
	for (i=0; i<MAX_VOTERLIST_NUM; ++i) {
		if (vlists[i]==NULL)
			continue;
		if (!strcmp(vlists[i]->listfname, fname))
			return 0;
	}
	return 1;
}

static int
add_list()
{
	char buf[60], titlebuf[STRLEN * 4];
	char fbuf[60];
	FILE* fn;
	int i;
	struct voterlist vltemp;
	if( listsnum >= MAX_VOTERLIST_NUM)
		return -1;
	bzero(&vltemp, sizeof(struct voterlist));
	clear();
	buf[0]='\0';
	getdata(t_lines - 1, 0, "名单标题: ", buf, 50, DOECHO, YEA);
	if( buf[0]=='\0' || buf[0]=='\n' ){
		return -1;
	}
	strncpy(vltemp.listname, buf, 50);
	strncpy(vltemp.authorid, currentuser.userid, IDLEN);
	vltemp.listname[49] = '\0';
	for (i=1; i<=MAX_VOTERLIST_NUM; ++i) {
		sprintf(fbuf, "validlist%d", i);
		if (compare_fname(fbuf))
			break;
	}
	strcpy(vltemp.listfname, fbuf);
	setbfile(buf,currboard, vltemp.listfname);
	fn=fopen(buf, "w");
	fclose(fn);
	vltemp.voternum=0;
	for (i=0; i<MAX_VOTERLIST_NUM; ++i) {
		if (vlists[i]==NULL)
			break;
	}
	vlists[i]= (struct voterlist*) malloc( sizeof(struct voterlist) );
	memcpy(vlists[i], &vltemp, sizeof(struct voterlist) );
	listsnum++;
	sprintf(titlebuf,"%s在%s版面建立投票名单:%s",
		currentuser.userid, currboard, vltemp.listname);
	deliverreport(titlebuf, "");

	save_list();
	return 0;
}

static int
save_list()
{
	int i;
	FILE *fp;
	char buf[60];
	setbfile(buf, currboard, "validlist");
	if( (fp = fopen( buf, "w") ) == NULL ){
		return -1;
	}
	for(i=0; i<listsnum; ++i){
		if (vlists[i]==NULL)
			continue;
		fwrite( vlists[i], sizeof(struct voterlist), 1, fp );
	}
	fclose(fp);

	return 0;
}


static int
voter(int listnum)
{
	char uident[IDLEN + 2];
	char ans[8], buf[STRLEN * 2], titlebuf[STRLEN * 2];
	int count, i;
	setbfile(genbuf, currboard, vlists[listnum]->listfname);
	ansimore(genbuf, YEA);
	while (1) {
		clear();
		prints("设定投票人员名单\n");
		setbfile(genbuf, currboard, vlists[listnum]->listfname);
		count = listfilecontent(genbuf);
		if (count)
			getdata(1, 0,
				"(A)增加 (D)删除or (E)离开or (M)写信给所有成员 [E]:  ",
				ans, 7, DOECHO, YEA);
		else
			getdata(1, 0, "(A)增加 or (E)离开 [E]: ", ans, 7, DOECHO, YEA);
		if (*ans == 'A' || *ans == 'a') {
			move(1, 0);
			usercomplete("增加投票人员: ", uident);
			if (*uident != '\0') {
				if (addvoter(uident) == 1) {
					sprintf(titlebuf,
						"%s由%s授予%s版面封闭投票权利",
						uident, currentuser.userid,
						currboard);
					sprintf(buf, "组别: %s", vlists[listnum]->listname);
					securityreport(titlebuf, genbuf);
					deliverreport(titlebuf, buf);
					mail_buf(buf, uident, titlebuf);
				}
			}
		} else if ((*ans == 'D' || *ans == 'd') && count) {
			move(1, 0);
			namecomplete("删除投票人员: ", uident);
			move(1, 0);
			clrtoeol();
			if (uident[0] != '\0') {
				if (delvoter(uident)) {
					sprintf(titlebuf,
						"%s被%s取消%s版面封闭投票权利",
						uident, currentuser.userid, currboard);
					sprintf(buf, "组别: %s", vlists[listnum]->listname);
					securityreport(titlebuf, genbuf);
					deliverreport(titlebuf, buf);
					mail_buf(buf, uident, titlebuf);
				}
			}
		} else if ((*ans == 'M' || *ans == 'm') && count) {
			voter_send(vlists[listnum]->listfname);
		} else
			break;
	}
	save_list();
	clear();
	return FULLUPDATE;
}

static int
vote_key(ch, allnum, pagenum)
int ch;
int allnum, pagenum;
{
	int deal = 0, ans;
	char buf[STRLEN * 2];
	int count;

	switch (ch) {
	case 'v':
	case 'V':
	case '\n':
	case '\r':
	case 'r':
	case KEY_RIGHT:
		user_vote(allnum + 1);
		deal = 1;
		break;
	case 'R':
		vote_results(currboard);
		deal = 1;
		break;
	case 'H':
	case 'h':
		show_help("help/votehelp");
		deal = 1;
		break;
	case 'A':
	case 'a':
		if (!IScurrBM)
			return YEA;
		vote_maintain(currboard);
		deal = 1;
		break;
	case 'O':
	case 'o':
		if (!IScurrBM)
			return YEA;
		clear();
		deal = 1;
		get_record(controlfile, &currvote, sizeof (struct votebal), allnum + 1);
		prints("\033[5;1;31m警告!!\033[m\n");
		prints("投票箱标题：\033[1m%s\033[m\n", currvote.title);
		ans = askyn("你确定要提早结束这个投票吗", NA, NA);

		if (ans != 1) {
			move(2, 0);
			prints("取消删除行动\n");
			pressreturn();
			clear();
			break;
		}
		mk_result();
		sprintf(buf, "提早结束投票 %s", currvote.title);
		securityreport(buf, buf);
		break;
	case 'D':
	case 'd':
		if (!HAS_PERM(PERM_OVOTE, currentuser))
			if (!IScurrBM) {
				return 1;
			}
		deal = 1;
		get_record(controlfile, &currvote, sizeof (struct votebal), allnum + 1);
		clear();
		prints("\033[5;1;31m警告!!\033[m\n");
		prints("投票箱标题：\033[1m%s\033[m\n", currvote.title);
		ans = askyn("您确定要强制关闭这个投票吗", NA, NA);

		if (ans != 1) {
			move(2, 0);
			prints("取消删除行动\n");
			pressreturn();
			clear();
			break;
		}
		sprintf(buf, "强制关闭投票 %s", currvote.title);
		securityreport(buf, buf);
		dele_vote();
		break;
	default:
		return 0;
	}
	if (deal) {
		Show_Votes();
		vote_title();
	}
	return 1;
}

static int
Show_Votes()
{

	move(3, 0);
	clrtobot();
	printvote(NULL);
	setcontrolfile();
	if (apply_record(controlfile, (void *) printvote, sizeof (struct votebal)) == -1) {
		prints("错误，没有投票箱开启....");
		pressreturn();
		return -1;
	}
	clrtobot();
	return 0;
}

int
b_vote()
{
	int num_of_vote;
	int voting;

	if (!HAS_PERM(PERM_VOTE, currentuser) || (currentuser.stay < 1800)) {
		return -1;
	}
	if (!haspostperm(currboard)) {
		return -1;
	}
	setcontrolfile();
	num_of_vote = get_num_records(controlfile, sizeof (struct votebal));
	if (num_of_vote == 0) {
		move(3, 0);
		clrtobot();
		prints("抱歉, 目前并没有任何投票举行。\n");
		pressreturn();
		setvoteflag(currboard, 0);
		return FULLUPDATE;
	}
	setlistrange(num_of_vote);
	clear();
	voting = choose(NA, 0, vote_title, vote_key, Show_Votes, (void *) user_vote);
	clear();
	return /*user_vote( currboard ) */ FULLUPDATE;
}

int
b_results()
{
	return vote_results(currboard);
}

void
m_vote()
{
	char buf[STRLEN];
	strcpy(buf, currboard);
	strcpy(currboard, DEFAULTBOARD);
	modify_user_mode(ADMIN);
	vote_maintain(DEFAULTBOARD);
	strcpy(currboard, buf);
	return;
}

