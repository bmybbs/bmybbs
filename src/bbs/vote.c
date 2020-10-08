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
#include "bcache.h"
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

extern int page, range;
extern char IScurrBM;
extern struct boardmem *bcache;
extern struct BCACHE *brdshm;
static char *const vote_type[] = { "ÊÇ·Ç", "µ¥Ñ¡", "¸´Ñ¡", "Êý×Ö", "ÎÊ´ð" ,"ÏÞ¶¨Æ±Êý¸´Ñ¡"};
struct votebal currvote;
extern int numboards;
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

	pos =
	    new_search_record(BOARDS, &fh, sizeof (fh), (void *) cmpbnames,
			      bname);
	if (flag == 0)
		fh.flag = fh.flag & ~VOTE_FLAG;
	else
		fh.flag = fh.flag | VOTE_FLAG;
	if (substitute_record(BOARDS, &fh, sizeof (fh), pos) == -1)
		prints("Error updating BOARDS file...\n");
	reload_boards();
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
	resolve_boards();

	if (now < brdshm->pollvote) {
		return 0;
	}

	move(t_lines - 1, 0);
	prints("¶Ô²»Æð£¬ÏµÍ³¹Ø±ÕÍ¶Æ±ÖÐ£¬ÇëÉÔºò...");
	refresh();

	nextpoll = now + 7 * 3600;

	strcpy(buf, currboard);
	for (i = 0; i < brdshm->number; i++) {
		strcpy(currboard, (&bcache[i])->header.filename);
		setcontrolfile();
		end = get_num_records(controlfile, sizeof (currvote));
		for (vnum = end; vnum >= 1; vnum--) {
			time_t closetime;
			get_record(controlfile, &currvote, sizeof (currvote),
				   vnum);
			closetime =
			    currvote.opendate + currvote.maxdays * 86400;
			if (now > closetime)
				mk_result();
			else if (nextpoll > closetime)
				nextpoll = closetime + 300;
		}
	}
	strcpy(currboard, buf);
	brdshm->pollvote = nextpoll;
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
			fprintf(sug, "[1m%.12s [mµÄ×÷´ðÈçÏÂ£º\n", ptr->uid);
		} else
			fprintf(sug, "[1m%.12s [mµÄ½¨ÒéÈçÏÂ£º\n", ptr->uid);
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
	char buf[STRLEN];

	fprintf(sug, "¡Ñ Í¶Æ±¿ªÆôÓÚ£º[1m%.24s[m  Àà±ð£º[1m%s[m\n",
		ctime(&currvote.opendate), vote_type[currvote.type - 1]);
	fprintf(sug, "¡Ñ Ö÷Ìâ£º[1m%s[m\n", currvote.title);
	if (currvote.type == VOTE_VALUE)
		fprintf(sug, "¡Ñ ´Ë´ÎÍ¶Æ±µÄÖµ²»¿É³¬¹ý£º[1m%d[m\n\n",
			currvote.maxtkt);
	fprintf(sug, "¡Ñ Æ±Ñ¡ÌâÄ¿ÃèÊö£º\n\n");
	sprintf(buf, "vote/%s/desc.%ld", currboard,
		(long int) currvote.opendate);
	b_suckinfile(sug, buf);
}

static int
compareip(a, b)
struct votelog *a, *b;
{
	return cmpIP(a->ip, b->ip);
}

static void
mk_result()
{
	char fname[STRLEN], nname[STRLEN];
	char sugname[STRLEN];
	char logfname[STRLEN], sortedlogfname[STRLEN];
	char title[STRLEN];
	int i;
	int postout = 0;
	unsigned int total = 0;

	setcontrolfile();
	sprintf(fname, "vote/%s/flag.%ld", currboard,
		(long int) currvote.opendate);
	count_result(NULL);
	sprintf(sugname, "vote/%s/tmp.%d", currboard, uinfo.pid);

	if ((sug = fopen(sugname, "w")) == NULL) {
		errlog("open vote tmp file error %d", errno);
		prints("Error: ½áÊøÍ¶Æ±´íÎó...\n");
		pressanykey();
		return;
	}
	(void) memset(vote_result, 0, sizeof (vote_result));
	if (apply_record(fname, (void *) count_result, sizeof (struct ballot))
	    == -1) {
		errlog("Vote apply flag error");
	}
	fprintf(sug,
		"[1;44;36m¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©ÈÊ¹ÓÃÕß%s©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª[m\n\n\n",
		(currvote.type != VOTE_ASKING) ? "½¨Òé»òÒâ¼û" : "´Ë´ÎµÄ×÷´ð");
	fclose(sug);
	sprintf(nname, "vote/%s/results", currboard);
	if ((sug = fopen(nname, "w")) == NULL) {
		errlog("open vote newresult file error %d", errno);
		prints("Error: ½áÊøÍ¶Æ±´íÎó...\n");
		return;
	}

	get_result_title();

	fprintf(sug, "** Í¶Æ±½á¹û:\n\n");
	if (currvote.type == VOTE_VALUE) {
		total = vote_result[32];
		for (i = 0; i < 10; i++) {
			fprintf(sug,
				"[1m  %4d[m µ½ [1m%4d[m Ö®¼äÓÐ [1m%4d[m Æ±  Ô¼Õ¼ [1m%d%%[m\n",
				(i * currvote.maxtkt) / 10 + ((i == 0) ? 0 : 1),
				((i + 1) * currvote.maxtkt) / 10, vote_result[i]
				,
				(vote_result[i] * 100) / ((total <= 0) ? 1 : total));
		}
		fprintf(sug, "´Ë´ÎÍ¶Æ±½á¹ûÆ½¾ùÖµÊÇ: [1m%d[m\n",
				vote_result[31] / ((total <= 0) ? 1 : total));
	} else if (currvote.type == VOTE_ASKING) {
		total = vote_result[32];
	} else {
		for (i = 0; i < currvote.totalitems; i++) {
			total += vote_result[i];
		}
		for (i = 0; i < currvote.totalitems; i++) {
			fprintf(sug, "(%c) %-40s  %4d Æ±  Ô¼Õ¼ [1m%d%%[m\n",
				'A' + i, currvote.items[i], vote_result[i],
				(vote_result[i] * 100) / ((total <= 0) ? 1 : total));
		}
	}
	fprintf(sug, "\nÍ¶Æ±×ÜÈËÊý = [1m%d[m ÈË\n", vote_result[32]);
	fprintf(sug, "Í¶Æ±×ÜÆ±Êý =[1m %d[m Æ±\n\n", total);
	fprintf(sug,
		"[1;44;36m¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©ÈÊ¹ÓÃÕß%s©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª[m\n\n\n",
		(currvote.type != VOTE_ASKING) ? "½¨Òé»òÒâ¼û" : "´Ë´ÎµÄ×÷´ð");
	b_suckinfile(sug, sugname);
	unlink(sugname);
	fclose(sug);

	sug = NULL;

	resolve_boards();
	for (i = 0; i < numboards; i++)
		if (!strncmp(currboard, bcache[i].header.filename, STRLEN))
			break;
	if (i != numboards)
		if (normal_board(currboard)) {
			if (bcache[i].header.clubnum == 0)
				postout = 1;
			else if (bcache[i].header.flag & CLUBTYPE_FLAG)
				postout = 1;
		}
	if (currvote.flag & VOTE_FLAG_OPENED) {
		char *mem = NULL;
		struct stat buf;
		int fd;

		sprintf(fname, "vote/%s/newlog.%ld", currboard,
			(long int) currvote.opendate);
		sprintf(logfname, "vote/%s/log", currboard);
		if ((sug = fopen(logfname, "w")) == NULL) {
			errlog("open vote tmp file error %d", errno);
			prints("Error: ½áÊøÍ¶Æ±´íÎó...\n");
			pressanykey();
			return;
		}
		fprintf(sug, "%12s   %16s   %24s\n", "ID", "IP", "Í¶Æ±Ê±¼ä");
		apply_record(fname, (void *) count_log,
			     sizeof (struct votelog));
		fclose(sug);
		if ((fd = open(fname, O_RDWR, 0644)) != -1) {
			flock(fd, LOCK_EX);
			fstat(fd, &buf);
			MMAP_TRY {
				mem =
				    mmap(0, buf.st_size, PROT_READ | PROT_WRITE,
					 MAP_FILE | MAP_SHARED, fd, 0);
				qsort(mem,
				      buf.st_size / sizeof (struct votelog),
				      sizeof (struct votelog),
				      (void *) compareip);
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
			prints("Error: ½áÊøÍ¶Æ±´íÎó...\n");
			pressanykey();
			return;
		}
		fprintf(sug, "%12s   %16s   %24s\n", "ID", "IP", "Í¶Æ±Ê±¼ä");
		apply_record(fname, (void *) count_log,
			     sizeof (struct votelog));
		fclose(sug);
		sug = NULL;
	}
	if (postout) {
		sprintf(title, "[¹«¸æ] %s °æµÄÍ¶Æ±½á¹û", currboard);
		postfile(nname, "vote", title, 1);
		if (currvote.flag & VOTE_FLAG_OPENED) {
			sprintf(title, "[¹«¸æ] %s °æµÄÍ¶Æ±²ÎÓëÇé¿ö", currboard);
			postfile(logfname, "vote", title, 1);
			sprintf(title, "[¹«¸æ] %s °æµÄÍ¶Æ±²ÎÓëÇé¿ö(by IP)",
				currboard);
			postfile(sortedlogfname, "vote", title, 1);
		}
	}
	if (strncmp(currboard, "vote", STRLEN)) {
		sprintf(title, "[¹«¸æ] %s °æµÄÍ¶Æ±½á¹û", currboard);
		postfile(nname, currboard, title, 1);
		if (currvote.flag & VOTE_FLAG_OPENED) {
			sprintf(title, "[¹«¸æ] %s °æµÄÍ¶Æ±²ÎÓëÇé¿ö", currboard);
			postfile(logfname, currboard, title, 1);
			sprintf(title, "[¹«¸æ] %s °æµÄÍ¶Æ±²ÎÓëÇé¿ö(by IP)",
				currboard);
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
	prints("ÇëÒÀÐòÊäÈë¿ÉÑ¡ÔñÏî, °´ ENTER Íê³ÉÉè¶¨.\n");
	num = 0;
	for (num = 0; num < 32; num++) {
		sprintf(buf, "%c) ", num + 'A');
		getdata((num % 16) + 4, (num / 16) * 40, buf, bal->items[num],
			36, DOECHO, YEA);
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
	char buf[STRLEN * 2];
	struct votebal *ball = &currvote;
	int aborted;

	setcontrolfile();
	if (!HAS_PERM(PERM_OVOTE, currentuser))
		if (!IScurrBM) {
			return 0;
		}
	stand_title("¿ªÆôÍ¶Æ±Ïä");
	makevdir(bname);
	for (;;) {
		getdata(2, 0,
			"(1)ÊÇ·Ç, (2)µ¥Ñ¡, (3)¸´Ñ¡, (4)ÊýÖµ (5)ÎÊ´ð(6)ÏÞ¶¨Æ±Êý¸´Ñ¡  (7)È¡Ïû ? : ",
			genbuf, 2, DOECHO, YEA);
		genbuf[0] -= '0';
		if (genbuf[0] == 7) {
			prints("È¡Ïû´Ë´ÎÍ¶Æ±\n");
			sleep(1);
			return FULLUPDATE;
		}
		if (genbuf[0] < 1 || genbuf[0] > 7)
			continue;
		ball->type = (int) genbuf[0];
		break;
	}
	if (askyn("´Ë´ÎÍ¶Æ±½á¹ûÊÇ·ñ¹«¿ª?", NA, NA))
		ball->flag |= VOTE_FLAG_OPENED;
	else
		ball->flag &= ~VOTE_FLAG_OPENED;
	ball->flag &= ~VOTE_FLAG_LIMITED;
	if (HAS_PERM(PERM_SYSOP, currentuser) || seek_in_file(MY_BBS_HOME"/etc/voteidboards", currboard)) {
		if (askyn
		    ("´Ë´ÎÍ¶Æ±ÊÇ·ñÏÞÖÆÍ¶Æ±ÈË? (ÐèÒªÏÈÔÚ°æÃæ°´VÉú³ÉÃûµ¥)", NA,
		     NA))
		{
			ball->flag |= VOTE_FLAG_LIMITED;
			getdata(5, 0, "Ñ¡ÓÃÃûµ¥µÄÐòºÅ: ", buf, 4, DOECHO, YEA);
			if (*buf == '\n' || atoi(buf) == 0 || *buf == '\0')
				strcpy(buf, "1");
			if (choose_voter_list(ball->listfname, atoi(buf))==-1)
				return DONOTHING;
		}
		else
			ball->flag &= ~VOTE_FLAG_LIMITED;
	}
	ball->opendate = time(NULL);
	prints("Çë°´ÈÎºÎ¼ü¿ªÊ¼±à¼­´Ë´Î [Í¶Æ±µÄÃèÊö]: \n");
	igetkey();
	setvfile(genbuf, bname, "desc");
	sprintf(buf, "%s.%ld", genbuf, (long int) ball->opendate);

	aborted = vedit(buf, NA, YEA);
	if (aborted) {
		clear();
		prints("È¡Ïû´Ë´ÎÍ¶Æ±\n");
		pressreturn();
		return FULLUPDATE;
	}

	clear();
	getdata(0, 0, "´Ë´ÎÍ¶Æ±ËùÐëÌìÊý (²»¿É£°Ìì): ", buf, 4, DOECHO, YEA);

	if (*buf == '\n' || atoi(buf) == 0 || *buf == '\0')
		strcpy(buf, "1");

	ball->maxdays = atoi(buf);
	if (999 == ball->maxdays) {
		prints("ÕæÊÇ±äÌ¬...\n");
		pressanykey();
	}
	for (;;) {
		getdata(1, 0, "Í¶Æ±ÏäµÄ±êÌâ: ", ball->title, 61, DOECHO, YEA);
		if (strlen(ball->title) > 0)
			break;
		bell();
	}
	switch (ball->type) {
	case VOTE_YN:
		ball->maxtkt = 0;
		strcpy(ball->items[0], "ÔÞ³É  £¨ÊÇµÄ£©");
		strcpy(ball->items[1], "²»ÔÞ³É£¨²»ÊÇ£©");
		strcpy(ball->items[2], "Ã»Òâ¼û£¨²»Çå³þ£©");
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
			getdata(21, 0, "Ò»¸öÈË×î¶à¼¸Æ±? [1]: ", buf, 5, DOECHO,
				YEA);
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
			getdata(21, 0, "Ò»¸öÈËÏÞ¶¨¼¸Æ±? [1]: ", buf, 5, DOECHO,
				YEA);
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
			getdata(3, 0, "ÊäÈëÊýÖµ×î´ó²»µÃ³¬¹ý [100] : ", buf, 4,
				DOECHO, YEA);
			ball->maxtkt = atoi(buf);
			if (ball->maxtkt <= 0)
				ball->maxtkt = 100;
			break;
		}
		break;
	case VOTE_ASKING:
/*                    getdata(3,0,"´ËÎÊ´ðÌâ×÷´ðÐÐÊýÖ®ÏÞÖÆ :",buf,3,DOECHO,YEA) ;
                    ball->maxtkt = atof(buf) ;
                    if(ball->maxtkt <= 0) ball->maxtkt = 10;*/
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
		prints("·¢ÉúÑÏÖØµÄ´íÎó£¬ÎÞ·¨¿ªÆôÍ¶Æ±£¬ÇëÍ¨¸æÕ¾³¤");
		errlog("Append Control file Error!! board=%s", currboard);
	} else {
		char votename[STRLEN];
		int i;

		prints("Í¶Æ±Ïä¿ªÆôÁË£¡\n");
		range++;;
		sprintf(votename, "tmp/votetmp.%s.%05d", currentuser.userid,
			uinfo.pid);
		if ((sug = fopen(votename, "w")) != NULL) {
			sprintf(buf, "[Í¨Öª] %s ¾Ù°ìÍ¶Æ±£º%s", currboard,
				ball->title);
			get_result_title();
			if (ball->type != VOTE_ASKING
			    && ball->type != VOTE_VALUE) {
				fprintf(sug, "\n¡¾[1mÑ¡ÏîÈçÏÂ[m¡¿\n");
				for (i = 0; i < ball->totalitems; i++) {
					fprintf(sug, "([1m%c[m) %-40s\n",
						'A' + i, ball->items[i]);
				}
			}
			fclose(sug);
			sug = NULL;
			resolve_boards();
			for (i = 0; i < numboards; i++)
				if (!strncmp
				    (currboard, bcache[i].header.filename,
				     STRLEN)) break;
			if (i != numboards)
				if (normal_board(currboard)) {
					if (bcache[i].header.clubnum == 0)
						postfile(votename, "vote", buf,
							 1);
					else if (bcache[i].
						 header.flag & CLUBTYPE_FLAG)
						    postfile(votename, "vote",
							     buf, 1);
				}
			postfile(votename, currboard, buf, 1);
			unlink(votename);
		}
	}
	pressreturn();
	return FULLUPDATE;
}

#if 0
int
rec_flag(bname, val, mode)
char *bname, val;
int mode;
{
	char buf[STRLEN], flag;
	int fd, num, size;

	num = usernum - 1;
	switch (mode) {
	case 2:
		sprintf(buf, "Welcome.rec");	/*½øÕ¾µÄ Welcome »­Ãæ */
		break;
	case 1:
		setvfile(buf, bname, "noterec");	/*ÌÖÂÛÇø±¸ÍüÂ¼µÄÆì±ê */
		break;
	default:
		return -1;
	}
	if (num >= MAXUSERS) {
		errlog("Vote Flag, Out of User Numbers. num=%d", num);
		return -1;
	}
	if ((fd = open(buf, O_RDWR | O_CREAT, 0600)) == -1) {
		return -1;
	}
	//flock(fd, LOCK_EX);
	size = (int) lseek(fd, 0, SEEK_END);
	/*
	   memset(buf, 0, sizeof (buf));
	   while (size <= num) {
	   write(fd, buf, sizeof (buf));
	   size += sizeof (buf);
	   }
	 */
	if (size > num) {
		lseek(fd, num, SEEK_SET);
		read(fd, &flag, 1);
	} else
		flag = 0;
	if ((flag == 0 && val != 0)) {
		lseek(fd, num, SEEK_SET);
		write(fd, &val, 1);
	}
	//flock(fd, LOCK_UN);
	close(fd);
	return flag;
}
#endif

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
		prints("ÄúÒÑ¾­Í¶ÁË [1m%d[m Æ±", count);
	}

	sprintf(buf, "%c.%2.2s%-36.36s", 'A' + i,
		((pbits >> i) & 1 ? "¡õ" : "  "), currvote.items[i]);
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
		sprintf(buf, "¿ÉÍ¶Æ±Êý: [1m%d[m Æ±", currvote.maxtkt);
	else
		buf[0] = '\0';
	closedate = currvote.opendate + currvote.maxdays * 86400;
	prints("Í¶Æ±½«½áÊøÓÚ: [1m%24s[m  %s  %s\n",
	       ctime(&closedate), buf,
	       (voted_flag) ? "([5;1mÐÞ¸ÄÇ°´ÎÍ¶Æ±[m)" : "");
	prints("Í¶Æ±Ö÷ÌâÊÇ: [1m%-50s[mÀàÐÍ: [1m%s[m \n", currvote.title,
	       vote_type[currvote.type - 1]);
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
		prints("ÇëÌîÈëÄúµÄ×÷´ð(ÈýÐÐ):\n");
	} else {
		line = 1;
		prints("ÇëÌîÈëÄú±¦¹óµÄÒâ¼û(ÈýÐÐ):\n");
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
	if (vote_check(uv->voted) > currvote.maxtkt)	//ÐÞÕýÏÈÇ°bugµÄ´íÎó½á¹û
		uv->voted = 0;
	uv->voted =
	    setperms(uv->voted, "Ñ¡Æ±", currvote.totalitems, showvoteitems, 1);
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

	uv->voted =
	    setperms(uv->voted, "Ñ¡Æ±", currvote.totalitems, showvoteitems, 1);
	strollvote(uv, &currvote, -multivotestroll);
	if (vote_check(uv->voted) != currvote.maxtkt)	{
		clear();
		move(4,0);
		prints("ÄúËùÍ¶Æ±ÊýÓë±¾Í¶Æ±Ö®ÒªÇó²»Ò»ÖÂ!");
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
	prints("´Ë´Î×÷´ðµÄÖµ²»ÄÜ³¬¹ý [1m%d[m", currvote.maxtkt);
	if (uv->voted != 0)
		sprintf(buf, "%d", uv->voted);
	else
		memset(buf, 0, sizeof (buf));
	do {
		getdata(3, 0, "ÇëÊäÈëÒ»¸öÖµ? [0]: ", buf, 5, DOECHO, NA);
		uv->voted = abs(atoi(buf));
	} while (uv->voted > currvote.maxtkt && buf[0] != '\n'
		 && buf[0] != '\0');
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
	char fname[STRLEN], bname[STRLEN];
	char buf[STRLEN];
	struct ballot uservote, tmpbal;
	int votevalue;
	int aborted = NA, pos;

	move(t_lines - 2, 0);
	get_record(controlfile, &currvote, sizeof (struct votebal), num);
//add by gluon for sm_vote
	if (!(currentuser.userlevel & PERM_LOGINOK)) {
		prints("¶Ô²»Æð, Äú»¹Ã»ÓÐÍ¨¹ý×¢²áÄØ\n");
		pressanykey();
		return;
	}
	if (currvote.flag & VOTE_FLAG_LIMITED) {
		int retv = valid_voter(currboard, currentuser.userid, currvote.listfname);
		if (retv == 0 || retv == -1) {
			prints("%s", retv == 0 ? "¶Ô²»Æð£¬Äú²»ÄÜ²Î¼Ó±¾´ÎÍ¶Æ±\n"
			       : "¶Ô²»Æð£¬Äú´Ó´©ËóÕ¾Á¬À´£¬²»ÄÜÍ¶Æ±\n");
			pressanykey();
			return;
		}
	}
//end
	if (currvote.flag & VOTE_FLAG_OPENED) {
		prints("Çë×¢Òâ£¬¸ÃÍ¶Æ±½áÊøºó½«¹«²¼Í¶Æ±ID¡¢IP¡¢Í¶Æ±Ê±¼ä");
		pressanykey();
	}

	sprintf(fname, "vote/%s/flag.%ld", currboard,
		(long int) currvote.opendate);
	if ((pos =
	     search_record(fname, &uservote, sizeof (uservote),
			   (void *) cmpvuid, currentuser.userid)) <= 0) {
		(void) memset(&uservote, 0, sizeof (uservote));
		voted_flag = NA;
	} else {
		voted_flag = YEA;
	}
	strncpy(uservote.uid, currentuser.userid, IDLEN);
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
		prints("±£Áô ¡¾[1m%s[m¡¿Ô­À´µÄµÄÍ¶Æ±¡£\n", currvote.title);
	} else {
		if (currvote.type != VOTE_ASKING)
			getsug(&uservote);
		pos =
		    search_record(fname, &tmpbal, sizeof (tmpbal),
				  (void *) cmpvuid, currentuser.userid);
		if (pos) {
			substitute_record(fname, &uservote, sizeof (uservote),
					  pos);
		} else if (append_record(fname, &uservote, sizeof (uservote)) ==
			   -1) {
			move(2, 0);
			clrtoeol();
			prints("Í¶Æ±Ê§°Ü! ÇëÍ¨ÖªÕ¾³¤²Î¼ÓÄÇÒ»¸öÑ¡ÏîÍ¶Æ±\n");
			pressreturn();
		}
		prints("\nÒÑ¾­°ïÄúÍ¶ÈëÆ±ÏäÖÐ...\n");
		if (currvote.flag & VOTE_FLAG_OPENED) {
			char votelogfile[STRLEN];
			struct votelog log;
			strcpy(log.uid, currentuser.userid);
			log.uid[IDLEN] = 0;
			log.votetime = time(NULL);
			log.voted = uservote.voted;
			strcpy(log.ip, currentuser.lasthost);
			sprintf(votelogfile, "vote/%s/newlog.%ld", currboard,
				(long int) currvote.opendate);
			append_record(votelogfile, &log, sizeof (log));
		}
		if (!strcmp(currboard, "SM_Election")) {
			int now;
			now = time(NULL);
			sprintf(buf, "%s %s %s", currentuser.userid,
					currentuser.lasthost, ytht_ctime(now));
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
	prints("[1;44m±àºÅ ¿ªÆôÍ¶Æ±ÏäÕß ¿ªÆôÈÕ %-37s   Àà±ð ÌìÊý ÈËÊý[m\n",
	       "Í¶Æ±Ö÷Ìâ");
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
	if (search_record
	    (flagname, &uservote, sizeof (uservote), (void *) cmpvuid,
	     currentuser.userid) <= 0) {
		voted_flag = NA;
	} else
		voted_flag = YEA;
	num_voted = get_num_records(flagname, sizeof (struct ballot));
	date = ctime(&ent->opendate) + 4;
	sprintf(buf, " %s%3d %-12.12s %-6.6s %-37.37s %c %-4.4s %3d  %4d[m\n",
		(voted_flag == NA) ? "[1m" : "", i, ent->userid, date,
		ent->title, ent->flag & VOTE_FLAG_OPENED ? 'O' : ' ',
		vote_type[ent->type - 1], ent->maxdays, num_voted);
	prints("%s", buf);
	return 0;
}

static void
dele_vote()
{
	char buf[STRLEN];
	int num = 1;
	struct votebal tmpvote;
	while (get_record(controlfile, &tmpvote, sizeof (struct votebal), num)
	       == 0) {
		if (currvote.opendate == tmpvote.opendate) {
			if (delete_record(controlfile, sizeof (currvote), num)
			    == -1) {
				prints("·¢Éú´íÎó£¬ÇëÍ¨ÖªÕ¾³¤....");
				pressanykey();
			}
			range--;
			sprintf(buf, "vote/%s/flag.%ld", currboard,
				(long int) currvote.opendate);
			unlink(buf);
			sprintf(buf, "vote/%s/desc.%ld", currboard,
				(long int) currvote.opendate);
			unlink(buf);
			sprintf(buf, "vote/%s/newlog.%ld", currboard,
				(long int) currvote.opendate);
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
		prints("Ä¿Ç°Ã»ÓÐÈÎºÎÍ¶Æ±µÄ½á¹û¡£\n");
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

	docmdtitle("[Í¶Æ±ÏäÁÐ±í]",
		   "[[1;32m¡û[m,[1;32me[m] Àë¿ª [[1;32mh[m] ÇóÖú [[1;32m¡ú[m,[1;32mr <cr>[m] ½øÐÐÍ¶Æ± [[1;32m¡ü[m,[1;32m¡ý[m] ÉÏ,ÏÂÑ¡Ôñ [1m¸ßÁÁ¶È[m±íÊ¾ÉÐÎ´Í¶Æ±");
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
		prints("ÊäÈëµÄID ÒÑ¾­´æÔÚ!");
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
	if (fn==-1)
		return 0;
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
	docmdtitle("[Í¶Æ±Ãûµ¥ÉèÖÃ]",
               "Ìí¼Ó[\x1b[1;32ma\x1b[0;37m] É¾³ý[\x1b[1;32md\x1b[0;37m]\x1b[m ¸ÄÃû[\033[1;32mt\033[0;37m]");
	move(2, 0);
	prints("\033[0;1;37;44m %4s %-13s %-44s %10s", "ÐòºÅ", "´´½¨ÈË","Ãûµ¥Ãû³Æ","Í¶Æ±ÈË¸öÊý");
	clrtoeol();
	update_endline();
}

static int
voter_key(int key, int allnum, int pagenum)
{
	char titlebuf[80];
	switch(key) {
		case 'a':
		case 'A':
		{
			char ans[STRLEN];
			if( listsnum>=MAX_VOTERLIST_NUM) {
				move(t_lines - 1, 0);
				clrtoeol();
				a_prompt(-1, "Ãûµ¥ÊýÄ¿ÒÑÂú£¬°´»Ø³µ¼ÌÐø...", ans, 2);
				move(t_lines - 1, 0);
				clrtoeol();
				return -1;
			}
			add_list();
			save_list();
			a_prompt(-1, "Ãûµ¥Ìí¼Ó³É¹¦£¬°´»Ø³µ¼ÌÐø...", ans, 2);
			return -1;
		}
			break;
		case 'd':
		case 'D':
		{
			if (askyn("È·ÊµÒªÉ¾³ýÂð", NA, YEA)) {
				sprintf(titlebuf,"%sÔÚ%s°æÃæÉ¾³ýÍ¶Æ±Ãûµ¥:%s",
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
			strcpy(newtitle, vlists[allnum]->listname);
			getdata(t_lines - 1, 0, "ÐÂ±êÌâ: ", newtitle, 50, DOECHO, NA);
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
		if (!askyn("±¾°æÏÖÔÚÃ»ÓÐÍ¶Æ±idÃûµ¥£¬ÐèÒªÏÖÔÚÔö¼ÓÂð", NA, YEA)){
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
	char buf[60], titlebuf[80];
	char fbuf[60];
	FILE* fn;
	int i;
	struct voterlist vltemp;
	if( listsnum >= MAX_VOTERLIST_NUM)
		return -1;
	bzero(&vltemp, sizeof(struct voterlist));
	clear();
	buf[0]='\0';
	getdata(t_lines - 1, 0, "Ãûµ¥±êÌâ: ", buf, 50, DOECHO, YEA);
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
	sprintf(titlebuf,"%sÔÚ%s°æÃæ½¨Á¢Í¶Æ±Ãûµ¥:%s",
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
	char uident[STRLEN];
	char ans[8], buf[STRLEN], titlebuf[STRLEN];
	int count, i;
	setbfile(genbuf, currboard, vlists[listnum]->listfname);
	ansimore(genbuf, YEA);
	while (1) {
		clear();
		prints("Éè¶¨Í¶Æ±ÈËÔ±Ãûµ¥\n");
		setbfile(genbuf, currboard, vlists[listnum]->listfname);
		count = listfilecontent(genbuf);
		if (count)
			getdata(1, 0,
				"(A)Ôö¼Ó (D)É¾³ýor (E)Àë¿ªor (M)Ð´ÐÅ¸øËùÓÐ³ÉÔ± [E]:  ",
				ans, 7, DOECHO, YEA);
		else
			getdata(1, 0, "(A)Ôö¼Ó or (E)Àë¿ª [E]: ", ans, 7,
				DOECHO, YEA);
		if (*ans == 'A' || *ans == 'a') {
			move(1, 0);
			usercomplete("Ôö¼ÓÍ¶Æ±ÈËÔ±: ", uident);
			if (*uident != '\0') {
				if (addvoter(uident) == 1) {
					sprintf(titlebuf,
						"%sÓÉ%sÊÚÓè%s°æÃæ·â±ÕÍ¶Æ±È¨Àû",
						uident, currentuser.userid,
						currboard);
					sprintf(buf, "×é±ð: %s", vlists[listnum]->listname);
					securityreport(titlebuf, genbuf);
					deliverreport(titlebuf, buf);
					mail_buf(buf, uident, titlebuf);
				}
			}
		} else if ((*ans == 'D' || *ans == 'd') && count) {
			move(1, 0);
			namecomplete("É¾³ýÍ¶Æ±ÈËÔ±: ", uident);
			move(1, 0);
			clrtoeol();
			if (uident[0] != '\0') {
				if (delvoter(uident)) {
					sprintf(titlebuf,
						"%s±»%sÈ¡Ïû%s°æÃæ·â±ÕÍ¶Æ±È¨Àû",
						uident, currentuser.userid, currboard);
					sprintf(buf, "×é±ð: %s", vlists[listnum]->listname);
					securityreport(titlebuf, genbuf);
					deliverreport(titlebuf, buf);
					mail_buf(buf, uident, titlebuf);
				}
			}
			}
		 else if ((*ans == 'M' || *ans == 'm') && count) {
			voter_send(vlists[listnum]->listfname);
		 }
		 else
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
	char buf[STRLEN];
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
		get_record(controlfile, &currvote, sizeof (struct votebal),
			   allnum + 1);
		prints("[5;1;31m¾¯¸æ!![m\n");
		prints("Í¶Æ±Ïä±êÌâ£º[1m%s[m\n", currvote.title);
		ans = askyn("ÄãÈ·¶¨ÒªÌáÔç½áÊøÕâ¸öÍ¶Æ±Âð", NA, NA);

		if (ans != 1) {
			move(2, 0);
			prints("È¡ÏûÉ¾³ýÐÐ¶¯\n");
			pressreturn();
			clear();
			break;
		}
		mk_result();
		sprintf(buf, "ÌáÔç½áÊøÍ¶Æ± %s", currvote.title);
		securityreport(buf, buf);
		break;
	case 'D':
	case 'd':
		if (!HAS_PERM(PERM_OVOTE, currentuser))
			if (!IScurrBM) {
				return 1;
			}
		deal = 1;
		get_record(controlfile, &currvote, sizeof (struct votebal),
			   allnum + 1);
		clear();
		prints("[5;1;31m¾¯¸æ!![m\n");
		prints("Í¶Æ±Ïä±êÌâ£º[1m%s[m\n", currvote.title);
		ans = askyn("ÄúÈ·¶¨ÒªÇ¿ÖÆ¹Ø±ÕÕâ¸öÍ¶Æ±Âð", NA, NA);

		if (ans != 1) {
			move(2, 0);
			prints("È¡ÏûÉ¾³ýÐÐ¶¯\n");
			pressreturn();
			clear();
			break;
		}
		sprintf(buf, "Ç¿ÖÆ¹Ø±ÕÍ¶Æ± %s", currvote.title);
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
	if (apply_record
	    (controlfile, (void *) printvote, sizeof (struct votebal)) == -1) {
		prints("´íÎó£¬Ã»ÓÐÍ¶Æ±Ïä¿ªÆô....");
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
		prints("±§Ç¸, Ä¿Ç°²¢Ã»ÓÐÈÎºÎÍ¶Æ±¾ÙÐÐ¡£\n");
		pressreturn();
		setvoteflag(currboard, 0);
		return FULLUPDATE;
	}
	setlistrange(num_of_vote);
	clear();
	voting =
	    choose(NA, 0, vote_title, vote_key, Show_Votes, (void *) user_vote);
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
