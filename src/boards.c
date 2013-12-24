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

#define BBS_PAGESIZE    (t_lines - 4)

struct allbrc *allbrc = NULL;
struct onebrc brc = { 0, 0, 0, "" };
char *sysconf_str();
struct newpostdata {
	char *name;
	char flag;
	short pos;
	char unread:2;
	unsigned char zap:1;
	char status;
	//int inboard;
} __attribute__ ((__packed__));

struct newpostdata nbrd[MAXBOARD];

int *zapbuf;
int zapbufchanged = 0;
int yank_flag = 0;
unsigned char boardprefix[5];
struct boardmem *getbcache(char *);

//¶¨ÖÆ°æÃæµÄ´úÂë, È¡×Ôfb2000.dhs.org.     --ecnegrevid
struct goodboard {
	char ID[GOOD_BRC_NUM][20];	//°æÃû×î¶à¿´À´ÊÇ17+1×Ö½Ú
	int num;
} GoodBrd;

static int inGoodBrds(char *bname);
static void load_GoodBrd(void);
static void save_GoodBrd(void);
static void load_zapbuf(void);
static void save_zapbuf(void);
static int zapped(int n, struct boardmem *bptr);
static int load_boards(int *brdnum, int secnum);
static int search_board(int *num, int brdnum, int secnum);
static int check_newpostt(struct newpostdata *ptr);
static void show_brdlist(int page, int clsflag, int newflag, int brdnum,
			 const struct sectree *sec);
static int cmpboard(struct newpostdata *brd, struct newpostdata *tmp);
static int choose_board(int newflag, const struct sectree *sec);
static void readwritebrc(struct allbrc *allbrc);
static int readtitle();   // Êä³öÔÄ¶ÁÍ·²¿
static char *readdoent(int num, struct fileheader *ent, char buf[512]);
static char *makedatestar(char *datestr, struct fileheader *ent);

void
GoodBrds()			// ²Ëµ¥µÄµ÷ÓÃº¯Êý
{
//        if(!strcmp(currentuser.userid,"guest")) return;
	GoodBrd.num = 9999;
	boardprefix[0] = 255;
	boardprefix[1] = 0;
	choose_board(1, NULL);
}

static int
inGoodBrds(char *bname)		// ÅÐ¶Ï°æÃæÊÇ·ñÊÇ¶©ÔÄ°æÃæ
{
	int i;
	for (i = 0; i < GoodBrd.num && i < GOOD_BRC_NUM; i++)
		if (!strcmp(bname, GoodBrd.ID[i]))
			return i + 1;
	return 0;
}

static void
load_GoodBrd()			//´ÓÎÄ¼þÖÐ»ñÈ¡¶©ÔÄ°æÃæ£¬Ìî³äÊý¾Ý½á¹¹ GoodBrd
{
	char buf[STRLEN];
	FILE *fp;

	GoodBrd.num = 0;
	setuserfile(buf, ".goodbrd");
	if ((fp = fopen(buf, "r"))) {
		for (GoodBrd.num = 0; GoodBrd.num < GOOD_BRC_NUM;) {
			if (!fgets(buf, sizeof (buf), fp))
				break;
			strsncpy(GoodBrd.ID[GoodBrd.num], strtrim(buf),
				 sizeof (GoodBrd.ID[GoodBrd.num]));
			if (canberead(GoodBrd.ID[GoodBrd.num]))
				GoodBrd.num++;
		}
		fclose(fp);
	}
	if (GoodBrd.num == 0) {
		GoodBrd.num++;
		if (getbcache(DEFAULTBOARD))
			strcpy(GoodBrd.ID[0], DEFAULTBOARD);
		else
			strcpy(GoodBrd.ID[0], currboard);
	}
}
static void
save_GoodBrd()			// ±£´æÓÃ»§¶©ÔÄµÄ°æÃæ
{
	int i;
	FILE *fp;
	char fname[STRLEN];

	if (GoodBrd.num <= 0) {
		GoodBrd.num = 1;
		if (getbcache(DEFAULTBOARD))
			strcpy(GoodBrd.ID[0], DEFAULTBOARD);
		else
			strcpy(GoodBrd.ID[0], currboard);
	}
	setuserfile(fname, ".goodbrd");
	if ((fp = fopen(fname, "wb+")) != NULL) {
		for (i = 0; i < GoodBrd.num; i++)
			fprintf(fp, "%s\n", GoodBrd.ID[i]);
		fclose(fp);
	}
}

void
EGroup(cmd)
char *cmd;
{
	const struct sectree *sec;
	GoodBrd.num = 0;
	boardprefix[0] = cmd[0];
	boardprefix[1] = 0;
	sec = getsectree(boardprefix);
	choose_board(DEFINE(DEF_NEWPOST) ? 1 : 0, sec);
}

void
Boards()
{
	boardprefix[0] = 255;
	boardprefix[1] = 0;
	GoodBrd.num = 0;
	choose_board(0, NULL);
}

void
New()
{
	boardprefix[0] = 255;
	boardprefix[1] = 0;
	GoodBrd.num = 0;
	choose_board(1, NULL);
}

static void
load_zapbuf()
{
	char fname[STRLEN];
	int fd, size;

	size = MAXBOARD * sizeof (int);
	zapbuf = (int *) malloc(size);
	bzero(zapbuf, size);
	setuserfile(fname, ".newlastread");
	if ((fd = open(fname, O_RDONLY, 0600)) != -1) {
		size = numboards * sizeof (int);
		read(fd, zapbuf, size);
		close(fd);
	}
	zapbufchanged = 0;
}

static void
save_zapbuf()
{
	char fname[STRLEN];
	int fd, size;

	if (!zapbufchanged)
		return;
	zapbufchanged = 0;

	setuserfile(fname, ".newlastread");
	if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) != -1) {
		size = numboards * sizeof (int);
		write(fd, zapbuf, size);
		close(fd);
	}
}

static int
zapped(int n, struct boardmem *bptr)
{
	if (zapbuf[n] == 0)	//Ã»±» z 
		return 0;
	if (zapbuf[n] < bptr->header.board_ctime)	//zµôÁË£¬µ«ÊÇ²»ÊÇÕâ¸ö°æÁË
		return 0;
	return 1;
}

static int
load_boards(int *brdnum, int secnum)
{
	struct boardmem *bptr;
	struct newpostdata *ptr;
	int n, addto = 0, goodbrd = 0;
	static int loadtime = 0;

	resolve_boards();
	if (!
	    (GoodBrd.num == 9999 || brdshm->uptime >= loadtime || zapbuf == NULL
	     || *brdnum <= 0))
		return 0;
	loadtime = time(NULL);
	if (zapbuf == NULL) {
		load_zapbuf();
	}

	*brdnum = 0;
	if (GoodBrd.num)
		goodbrd = 1;	// ±íÊ¾´¦ÓÚÔÄ¶Á¶¨ÖÆ°æÃæ×´Ì¬
	if (GoodBrd.num == 9999)	// Ç¿ÖÆ load ¶©ÔÄ°æÃæ
		load_GoodBrd();
	for (n = 0; n < numboards; n++) {
		bptr = &bcache[n];
		if (!(bptr->header.filename[0]))
			continue;
		if (goodbrd == 0) {	//Èç¹û²»ÊÇÔÄ¶Á¶¨ÖÆµÄ°æÃæ, Ôò...
			if (boardprefix[0] != 255 && boardprefix[0] != '*' &&
			    strcmp(boardprefix, bptr->header.sec1) &&
			    (strcmp(boardprefix, bptr->header.sec2)
			     || bptr->header.sec2[0] == 0))
				continue;
			if (!hasreadperm(&(bptr->header)))
				continue;
			addto = yank_flag || !zapped(n, bptr)
			    || (bptr->header.level & PERM_NOZAP);
		} else
			addto = inGoodBrds(bptr->header.filename);	//·ñÔòÅÐ¶ÏÊÇ·ñÊÇ¶©ÔÄµÄ°æÃæ
		if (addto) {	// addto ±êÖ¾¸Ã°æÃæÓ¦¸Ã¿ÉÒÔÔÄ¶Á
			ptr = &nbrd[*brdnum];
			(*brdnum)++;
			ptr->name = bptr->header.filename;
			ptr->flag = bptr->header.flag |
			    ((bptr->header.
			      level & PERM_NOZAP) ? NOZAP_FLAG : 0);
			ptr->pos = n;
			ptr->unread = -1;	//ÉèÖÃÎª -1 ±íÊ¾Î´³õÊ¼»¯
			ptr->zap = (zapped(n, bptr));
			//ptr->inboard = bptr->inboard;
			if (bptr->header.level & PERM_POSTMASK)
				ptr->status = 'p';
			else if (bptr->header.level & PERM_NOZAP)
				ptr->status = 'z';
			else if ((bptr->header.level & ~PERM_POSTMASK) != 0)
				ptr->status = 'r';
			else
				ptr->status = ' ';
			if (ptr->status == ' ') {
				if (bptr->header.clubnum != 0) {
					if (bptr->header.flag & CLUBTYPE_FLAG)
						if (HAS_CLUBRIGHT
						    (bptr->header.clubnum,
						     uinfo.clubrights))
				      ptr->status = 'O';
						else
							ptr->status = 'o';
					else
						ptr->status = 'c';
				}
			}
		}
	}
	if (*brdnum == 0 && secnum == 0 && !yank_flag) {
		if (goodbrd) {	// Èç¹û´¦ÓÚ¶¨ÖÆ°æÃæÖÐ£¬µ«Ã»ÓÐÈÎºÎ°æÃæµÄ»°£¬ÔòË¢ÐÂ
			GoodBrd.num = 0;
			save_GoodBrd();
			GoodBrd.num = 9999;
		} else {
			char ans[3];
			getdata(t_lines - 1, 0,
				"¸ÃÌÖÂÛÇø×éµÄ°æÃæÒÑ¾­±»ÄãÈ«²¿È¡ÏûÁË£¬ÊÇ·ñ²é¿´ËùÓÐÌÖÂÛÇø£¿(Y/N)[N]",
				ans, 2, DOECHO, YEA);
			if (toupper(ans[0]) == 'Y') {
				*brdnum = -1;
				yank_flag = 1;
				return -1;
			}
		}
	}
	return 1;
}

static int
search_board(num, brdnum, secnum)
int *num, brdnum, secnum;
{
	static int i = 0, find = YEA;
	static char bname[STRLEN];
	int n, ch, tmpn = NA;

	if (find == YEA) {
		bzero(bname, sizeof (bname));
		find = NA;
		i = 0;
	}
	while (1) {
		move(t_lines - 1, 0);
		clrtoeol();
		prints("ÇëÊäÈëÒªÕÒÑ°µÄ board Ãû³Æ£º%s", bname);
		ch = egetch();

		if (isprint2(ch)) {
			bname[i++] = ch;
			for (n = secnum; n < brdnum + secnum; n++) {
				if (!strncasecmp
				    (nbrd[n - secnum].name, bname, i)) {
					tmpn = YEA;
					*num = n;
					if (!strcmp
					    (nbrd[n - secnum].name,
					     bname)) return 1;	/*ÕÒµ½ÀàËÆµÄ°æ£¬»­ÃæÖØ»­ */
				}
			}
			if (tmpn)
				return 1;
			if (find == NA) {
				bname[--i] = '\0';
			}
			continue;
		} else if (ch == Ctrl('H') || ch == KEY_LEFT
			   || ch == KEY_DEL || ch == '\177') {
			i--;
			if (i < 0) {
				find = YEA;
				break;
			} else {
				bname[i] = '\0';
				continue;
			}
		} else if (ch == '\t') {
			find = YEA;
			break;
		} else if (ch == '\n' || ch == '\r' || ch == KEY_RIGHT) {
			find = YEA;
			break;
		}
		bell();
	}
	if (find) {
		move(t_lines - 1, 0);
		clrtoeol();
		return 2 /*½áÊøÁË */ ;
	}
	return 1;
}

static int
check_newpostt(ptr)
struct newpostdata *ptr;
{
	struct boardmem *bptr;
	if (!allbrc) {
		allbrc = malloc(sizeof (struct allbrc));
		readwritebrc(allbrc);
	}
	
	//prints("pos=%d\n", ptr->pos);
	bptr = &bcache[ptr->pos];
	if (bptr->total <= 0){		//mint
	//	prints("if");
		ptr->unread = 0;
	}
	else{
	//	prints("else");
		ptr->unread =
		    brc_unreadt_quick(allbrc, ptr->name, bptr->lastpost);
	}
	//pressanykey();
	return 0;
}

int
unread_position(dirfile, bptr)
char *dirfile;
struct boardmem *bptr;
{
	int fd, offset, step, num, filetime;
	//bptr = &bcache[ptr->pos];

	num = bptr->total + 1;
	if ((fd = open(dirfile, O_RDONLY)) > 0) {
		if (!brc_initial(bptr->header.filename, 1)) {
			num = 1;
		} else {
			offset = (int) &((struct fileheader *) 0)->filetime;
			num = bptr->total - 1;
			step = 4;
			while (num > 0) {
				lseek(fd,
				      offset + num * sizeof (struct fileheader),
				      SEEK_SET);
				if (read(fd, &filetime, sizeof (filetime)) <= 0
				    || !brc_unreadt(&brc, filetime))
					break;
				num -= step;
				if (step < 32)
					step += step / 2;
			}
			if (num < 0)
				num = 0;
			while (num < bptr->total) {
				lseek(fd,
				      offset + num * sizeof (struct fileheader),
				      SEEK_SET);
				if (read(fd, &filetime, sizeof (filetime)) <= 0
				    || brc_unreadt(&brc, filetime))
					break;
				num++;
			}
		}
		close(fd);
	}
	if (num < 0)
		num = 0;
	return num;
}

static void
show_brdlist(page, clsflag, newflag, brdnum, sec)
int page, clsflag, newflag, brdnum;
const struct sectree *sec;
{
	struct newpostdata *ptr;
	struct boardmem *bptr;
	int n;
	int sorttype;
	char tmpBM[IDLEN + 1], buf[STRLEN];
	char title[20];
	int secnum;

	sorttype = currentuser.flags[0] & BRDSORT_MASK;
	switch (sorttype) {
	case 0x00:
		strcpy(title, "[ÌÖÂÛÇøÁÐ±í] [·ÖÀà]");
		break;
	case 0x10:
		strcpy(title, "[ÌÖÂÛÇøÁÐ±í] [ÈËÆø]");
		break;
	case 0x20:
		strcpy(title, "[ÌÖÂÛÇøÁÐ±í] [×ÖÄ¸]");
		break;
	case 0x30:
		strcpy(title, "[ÌÖÂÛÇøÁÐ±í] [ÔÚÏß]");
		break;
	default:
		strcpy(title, "[ÌÖÂÛÇøÁÐ±í]");
		break;
	}

	if (clsflag) {
		clear();
		docmdtitle(title,
			   "  [mÖ÷Ñ¡µ¥[[1;32m¡û[m,[1;32me[m] ÔÄ¶Á[[1;32m¡ú[m,[1;32mRtn[m] Ñ¡Ôñ[[1;32m¡ü[m,[1;32m¡ý[m] ÁÐ³ö[[1;32my[m] ÅÅÐò[[1;32ms[m] ËÑÑ°[[1;32m/[m] ÇÐ»»[[1;32mc[m] ÇóÖú[[1;32mh[m]\n");
		prints
		    ("[1;44;37m %s ÌÖÂÛÇøÃû³Æ   V  Àà±ð  %-21sS °æ  Ö÷      ÔÚÏß   ÈËÆø[m\n",
		     newflag ? " È«²¿ Î´" : " ±àºÅ Î´", "ÖÐ  ÎÄ  Ðð  Êö");
	}

	if (sec)
		secnum = sec->nsubsec;
	else
		secnum = 0;
	move(3, 0);
	for (n = page; n < page + BBS_PAGESIZE; n++) {
		if (n >= brdnum + secnum) {
			clrtoeol();
			prints("\n");
			continue;
		}
		if (n < secnum) {
			if (sec) {
				prints(" %4d  ", n + 1);
				prints("£« ");
				prints("%s\n", sec->subsec[n]->title);
			}
		} else {
			ptr = &nbrd[n - secnum];
			bptr = &bcache[ptr->pos];
			if (ptr->unread == -1)
				check_newpostt(ptr);
			if (!newflag)
				prints(" %4d  ", n + 1 - secnum);
			else
				prints(" %5d ", bptr->total);
			prints("%s%c",
			       (ptr->flag & INNBBSD_FLAG) ? (ptr->unread ?
							     "[1;32m¡ñ[m" :
							     "[1;32m¡ð[m")
			       : (ptr->unread ? "¡ô" : "¡ó"), (ptr->zap
							       && !(ptr->flag &
								    NOZAP_FLAG))
			       ? '-' : ' ');
			strncpy(tmpBM, bptr->header.bm[0], IDLEN);
			sprintf(buf, "[%s] %s", bptr->header.type,
				bptr->header.title);
			if (ptr->status == 'p')
				memcpy(buf, "[Ö»¶Á]", 6);
			prints("%-13s%s%-28s %c %-12s%4d %6d\n", ptr->name,
			       (ptr->flag & VOTE_FLAG) ? "[1;31mV[m" : " ",
			       buf, ptr->status,
			       (tmpBM[0] == '\0' ? "³ÏÕ÷°æÖ÷ÖÐ" : tmpBM),
			       bptr->inboard, bptr->score);	//HAS_PERM(PERM_POST)?ptr->status:' ',
		}
	}
}
static int
cmpboard(brd, tmp)
struct newpostdata *brd, *tmp;
{
	int type = 0;
	unsigned char sorttype;
	struct boardmem *bptrbrd, *bptrtmp;
	sorttype = currentuser.flags[0] & BRDSORT_MASK;
	bptrbrd = &bcache[brd->pos];
	bptrtmp = &bcache[tmp->pos];

	switch (sorttype) {
	case 0x00:
		type = bptrbrd->header.sec1[0] - bptrtmp->header.sec1[0];
		if (type == 0)
			type =
			    strncasecmp(bptrbrd->header.type,
					bptrtmp->header.type, 4);
		if (type == 0)
			type = strcasecmp(brd->name, tmp->name);
		break;
	case 0x10:
		type = bptrtmp->score - bptrbrd->score;
		break;
	case 0x20:
		type = strcasecmp(brd->name, tmp->name);
		break;
	case 0x30:
		type = bptrtmp->inboard - bptrbrd->inboard;
		break;
	default:
		break;
	}
	return type;
}

void
update_postboards(void)
{
	int i, begin = 0;
	char buf[64], *bname;
	FILE *fp, *fw;
	sprintf(buf, "tmp/postb.pl.%d", uinfo.pid);
	fw = fopen(buf, "w");
	if (NULL == fw) {
		errlog("can't open postb.pl to write!");
		return;
	}
	fputs("#!/usr/bin/perl\n@board=(\n", fw);
	for (i = 0; i < brdshm->number; i++) {
		bname = &(bcache[i].header.filename[0]);
		if (!bname[0])
			continue;
		snprintf(buf, 64, "boards/%s/.POSTBOARDS", bname);
		if (valid_fname(bname)) {
			if (begin)
				fputs(",\n", fw);
			else
				begin = 1;
			fprintf(fw, "\" %s", bname);
		} else
			continue;
		fp = fopen(buf, "r");
		if (NULL == fp)
			goto end;

		while (fgets(buf, sizeof (buf), fp)) {
			if ('\n' == buf[strlen(buf) - 1])
				buf[strlen(buf) - 1] = 0;
			if (valid_fname(buf))
				fprintf(fw, " %s", buf);
		}
		fclose(fp);
	      end:
		fputs(" \"", fw);
	}
	fputs("\n);\n", fw);
	fclose(fw);
	sprintf(buf, "tmp/postb.pl.%d", uinfo.pid);
	rename(buf, "etc/postb.pm");
	return;
}

static int
choose_board(newflag, sec)
int newflag;
const struct sectree *sec;
{
	int num=0;
	struct newpostdata *ptr;
	int page = 0, ch = 0, tmp, number, tmpnum;
	int loop_mode = 0, retv;
	int brdnum, secnum;
	unsigned char sorttype;
	struct boardmem *bptr;
	char ans[4];			//add by mintbaggio 040330 for front page commend
	int type;			//add by mintbaggio 040330 for front page commend
	char property[STRLEN], boardbuf[24];

	if (sec)
		secnum = sec->nsubsec;
	else
		secnum = 0;
	if (!strcmp(currentuser.userid, "guest"))
		yank_flag = 1;
	modify_user_mode(newflag ? READNEW : READBRD);
	brdnum = number = 0;
	while (1) {
		retv = load_boards(&brdnum, secnum);
		if (retv < 0)
			retv = load_boards(&brdnum, secnum);
		if (brdnum + secnum <= 0)
			break;
		if (retv) {
			qsort(nbrd, brdnum, sizeof (nbrd[0]),
			      (void *) cmpboard);
			page = -1;
		}
		if (num < 0)
			num = 0;
		if (num >= brdnum + secnum)
			num = brdnum + secnum - 1;
		if (page < 0) {
			if (newflag && (num >= secnum)) {
				for (tmp = 0; tmp < brdnum; tmp++)
					check_newpostt(&nbrd[tmp]);
				tmp = num;
				while (num < brdnum + secnum) {
					ptr = &nbrd[num - secnum];
					if (ptr->unread == -1)
						check_newpostt(ptr);
					if (ptr->unread)
						break;
					num++;
				}
				if (num >= brdnum + secnum) {
					num = tmp;
				}
			}
			page = (num / BBS_PAGESIZE) * BBS_PAGESIZE;
			show_brdlist(page, 1, newflag, brdnum, sec);
			update_endline();
		}
		if (num < page || num >= page + BBS_PAGESIZE) {
			page = (num / BBS_PAGESIZE) * BBS_PAGESIZE;
			show_brdlist(page, 1, newflag, brdnum, sec);
			update_endline();
		}
		//since allbrc is large, we should not waste too much memory on it, 
		//free it as soon as possible
		if (allbrc) {
			free(allbrc);
			allbrc = NULL;
		}
		move(3 + num - page, 0);
		prints(">", number);
		move(3 + num - page, 0);
		if (loop_mode == 0) {
			can_R_endline = 1;
			ch = egetch();
			can_R_endline = 0;
		}
		move(3 + num - page, 0);
		prints(" ");
		if (ch == 'q' || ch == 'e' || ch == KEY_LEFT || ch == EOF)
			break;
		switch (ch) {
		case 'P':
		case 'b':
		case Ctrl('B'):
		case KEY_PGUP:
			if (num == 0)
				num = brdnum + secnum - 1;
			else
				num -= BBS_PAGESIZE;
			break;
		case 'F':
		case 'f':
			if (newflag == 1)
				newflag = 0;
			else
				newflag = 1;
			show_brdlist(page, 1, newflag, brdnum, sec);
			update_endline();
			break;
		case 'L':	/* ppfoong */
			show_allmsgs();
			page = -1;
			break;
		case 'N':
		case ' ':
		case Ctrl('F'):
		case KEY_PGDN:
			if (num == brdnum + secnum - 1)
				num = 0;
			else
				num += BBS_PAGESIZE;
			break;
		case 'p':
		case 'k':
		case KEY_UP:
			if (num-- <= 0)
				num = brdnum + secnum - 1;
			break;
		case 'n':
		case 'j':
		case KEY_DOWN:
			if (++num >= brdnum + secnum)
				num = 0;
			break;
		case '$':
			num = brdnum + secnum - 1;
			break;
		case '!':	/* youzi leave */
			return Q_Goodbye();
		case 'w':
			if ((in_mail != YEA) && HAS_PERM(PERM_READMAIL))
				m_read();
			page = -1;
			break;
		case 'h':
			show_help("help/boardreadhelp");
			page = -1;
			break;
		case '/':
			move(3 + num - page, 0);
			prints(">");
			tmpnum = num;
			tmp = search_board(&num, brdnum, secnum);
			move(3 + tmpnum - page, 0);
			prints(" ");
			if (tmp == 1)
				loop_mode = 1;
			else {
				loop_mode = 0;
				update_endline();
			}
			break;
		case 's':	/* sort/unsort -mfchen */
			sorttype = currentuser.flags[0] & BRDSORT_MASK;
			sorttype += 0x10;
			sorttype = sorttype % 0x40;
			currentuser.flags[0] &= ~BRDSORT_MASK;
			currentuser.flags[0] |= sorttype;
			qsort(nbrd, brdnum, sizeof (nbrd[0]),
			      (void *) cmpboard);
			page = -1;
			break;
		case 'y':
			if (!GoodBrd.num) {
				yank_flag = !yank_flag;
				brdnum = -1;
			}
			break;
		case 'z':
			if (num >= secnum + brdnum || num < secnum)
				break;
			if (HAS_PERM(PERM_BASIC)
			    && !(nbrd[num - secnum].flag & NOZAP_FLAG)) {
				ptr = &nbrd[num - secnum];
				ptr->zap = !ptr->zap;
				ptr->unread = -1;
				zapbuf[ptr->pos] = (ptr->zap ? now_t : 0);
				zapbufchanged = 1;
				page = 999;
			}
			break;
		case 'C':
			if (HAS_PERM(PERM_SPECIAL2)
			    || clubsync("deleterequest")) {
				do1984menu();
				page = -1;
			}
			break;
		case KEY_HOME:
			num = 0;
			break;
		case KEY_END:
			num = brdnum + secnum - 1;
			break;
		case '\n':
		case '\r':
			if (number > 0) {
				num = number - 1;
				break;
			}
			/* fall through */
		case KEY_RIGHT:
			if (num >= secnum) {
				ptr = &nbrd[num - secnum];
				brc_initial(ptr->name, 1);
				strcpy(currboard, ptr->name);
				if (DEFINE(DEF_FIRSTNEW)) {
					char buf[STRLEN];
					setbdir(buf, currboard, digestmode);
					if (getkeep(buf, -1, 0) == NULL) {
						tmp =
						    unread_position(buf,
								    &bcache
								    [ptr->pos]);
						page = tmp - t_lines / 2;
						getkeep(buf,
							page > 1 ? page : 1,
							tmp + 1);
					}
				}
				Read();
				ptr->unread = page = -1;
				modify_user_mode(newflag ? READNEW : READBRD);
			} else {
				if (sec) {
					strcpy(boardprefix,
					       sec->subsec[num]->basestr);
					choose_board(newflag, sec->subsec[num]);
					strcpy(boardprefix, sec->basestr);
					page = -1;
					brdnum = -1;
				}
			}
			break;
		case KEY_TAB:
			//add by mintbaggio 040330 for front page commend
			//saveline(t_lines - 2, 0, NULL);
			move(t_lines - 2, 0);
			clrtoeol();
			getdata(t_lines - 2, 0, "Ñ¡ÔñÔÄ¶Á: (0)È¡Ïû (1)BMYÍÆ¼öÎÄÕÂ (2)±¾ÈÕÊ®´óÈÈÃÅ»°Ìâ [2]:", ans, 3, DOECHO, YEA);
			if(ans[0] == '\0')
				type = 2;
			else	
				type = atoi(ans);
			
			if(type>2 || type<1){
				move(t_lines-2, 0);
				clrtoeol();
				break;
			}
			else if(type == 1)
			{
				show_commend();
				pressreturn();
			}
			else
			{
				if (DEFINE(DEF_FILTERXXX))
				{
					// ÒÔÏÂÎªÁËÊµÏÖtelnetÏÂ±ßÖ±½Ó¿´10´ó,modified by interma@BMY 2005.6.25

					char board[10][80];
					char title[10][80];
						
					FILE *fp2 = fopen("etc/dayf_index", "r");
					int totalnum = 0;
					while (fgets(board[totalnum], 80, fp2) != NULL)
					{
						board[totalnum][strlen(board[totalnum]) - 1] = 0;
						fgets(title[totalnum], 80, fp2);
						title[totalnum][strlen(title[totalnum]) - 1] = 0;
						totalnum ++;
					}
					fclose(fp2);
					
					while (1)
					{
						ansimore("etc/dayf", NA);
						int num = 0;
						do
						{
							getdata(t_lines - 2, 0, "Ñ¡ÔñÔÄ¶Á£¨ÇëÊäÈëÐòºÅ, Enter»ò0ÎªÍË³ö£©:", ans, 3, DOECHO, YEA);
							num = atoi(ans);
						}
						while (num < 0 || num >totalnum);

						if (num == 0)
							break;

						char dir[80];
						struct mmapfile mf = { ptr:NULL };
						struct fileheader *x = NULL;

						sprintf(dir, "boards/%s/.DIR", board[num - 1]);
						mmapfile(dir, &mf);
						int numrecords = mf.size / sizeof (struct fileheader);
			 
						int i;
						int isfind = 0;
						for (i = 0; i < numrecords; i++) 
						{
							x = (struct fileheader *) (mf.ptr + i * sizeof (struct fileheader));
							if (strcmp(title[num - 1], x->title) == 0)
							{
								isfind = 1;
								break;
							}
						}

						if (isfind)
						{
							uinfo.curboard = getbnum(board[num - 1]);
							update_utmp();
							strcpy(currboard, board[num - 1]);
							sprintf(dir, "boards/%s/%s", board[num - 1], fh2fname(x));
							ansimore(dir, NA);
							pressreturn();
						}

					}
				}
				else
					ansimore("0Announce/bbslist/day", NA);
			}
			//what_to_do();
			//pressreturn();
			page = -1;
			break;
		case 'u':
			clear();
			prints("²éÑ¯ÍøÓÑ×´Ì¬");
			t_query(NULL);
			page = -1;
			break;
		case 'H':
			ansimore("0Announce/bbslist/good", NA);
			what_to_do();
			page = -1;
			break;
		case 'S':	/* sendmsg ... youzi */
			if (!HAS_PERM(PERM_PAGE))
				break;
			s_msg();
			page = -1;
			break;
		case 'c':	/* show friends ... youzi */
			if (!HAS_PERM(PERM_BASIC))
				break;
			t_friends();
			modify_user_mode(newflag ? READNEW : READBRD);
			page = -1;
			break;
		case 'E':
			if (num < secnum || num >= secnum + brdnum)
				break;
			editboard(nbrd[num - secnum].name);
			page = -1;
			break;
		case Ctrl('D'):
			if (num >= secnum + brdnum || num < secnum)
				break;
			ptr = &nbrd[num - secnum];
			if (ptr->status == 'r')
				break;

			bptr = getbcache(ptr->name);
			if (!bptr)
				break;
			if (!chk_currBM(&(bptr->header), 0)
			    && !clubsync("deleterequest"))
				break;
			if (ptr->status == 'p') {
				if (!HAS_PERM(PERM_BLEVELS))
					break;
			}
			page = -1;
			sprintf(genbuf, "È·¶¨Òª%s%s°æÂð?",
				(ptr->status == 'p') ? "½â·â" : "·â",
				ptr->name);
			if (askyn(genbuf, NA, YEA) == NA)
				break;

			{
				struct boardheader fh;
				int pos;
				if (!
				    (pos =
				     new_search_record(BOARDS, &fh, sizeof (fh),
						       (void *) cmpbnames,
						       ptr->name))) {
					prints("´íÎóµÄÌÖÂÛÇøÃû³Æ");
					pressreturn();
					break;
				}
				if (fh.level & ~(PERM_BLEVELS | PERM_POSTMASK)) {
					prints
					    ("°üº¬²»ÄÜÖ±½Ó·â°æµÄÈ¨ÏÞ, ÎÞ·¨²Ù×÷, ÇëÓÃÐÞ¸Ä°æÃæÉèÖÃ¹¦ÄÜ");
					pressreturn();
					break;
				}
				if (ptr->status == 'p') {
					fh.level &=
					    ~(PERM_POSTMASK | PERM_BLEVELS);
					ptr->status = ' ';
				} else {
					fh.level |=
					    (PERM_POSTMASK | PERM_BLEVELS);
					ptr->status = 'p';
				}
				substitute_record(BOARDS, &fh, sizeof (fh),
						  pos);
				reload_boards();
				sprintf(genbuf, "%sÌÖÂÛÇø: %s",
					(ptr->status == 'p') ? "·â" : "½â·â",
					ptr->name);
				securityreport(genbuf, genbuf);
				prints("ÒÑ¾­%sÁËÌÖÂÛÇø: %s",
				       (ptr->status == 'p') ? "·â" : "½â·â",
				       ptr->name);
			}
			pressreturn();
			break;
		case 'B':
			if (num >= brdnum + secnum || num < secnum)
				break;
			ptr = &nbrd[num - secnum];
			if (!HAS_PERM(PERM_BLEVELS))
				break;
			page = -1;
			sprintf(genbuf, "È·¶¨Òª±à¼­ %s °æÃæ×ªÐÅ¶ÔÓ¦ÁÐ±íÂð?",
				ptr->name);
			if (askyn(genbuf, NA, YEA) == NA)
				break;
			snprintf(genbuf, 64, "boards/%s/.POSTBOARDS",
				 ptr->name);
			if (vedit(genbuf, 0, YEA) == -1)
				break;
			update_postboards();
			pressreturn();
			break;

		case Ctrl('A'):
			if (num >= secnum + brdnum || num < secnum)
				break;
			ptr = &nbrd[num - secnum];
			bptr = getbcache(ptr->name);
			if (!bptr)
				break;
			page = -1;
			if (bptr->header.level & PERM_POSTMASK)
				sprintf(property, "ÏÞÖÆ %s È¨Àû", "POST");
			else if (bptr->header.level & PERM_NOZAP)
				sprintf(property, "ÏÞÖÆ %s È¨Àû", "ZAP");
			else
				sprintf(property, "ÏÞÖÆ %s È¨Àû", "READ");
			if ((bptr->header.level & ~PERM_POSTMASK) == 0){
				property[0] = '\0';
				if (bptr->header.flag & ANONY_FLAG) 
					strcat(property, "ÄäÃû");
				if (bptr->header.flag & CLUB_FLAG)
					strcat(property, "¾ãÀÖ²¿");
				if (property[0] == '\0')
					strcpy(property, "ÆÕÍ¨");
			}
			strcpy(boardbuf, currboard);
			strcpy(currboard, bptr->header.filename);

			sprintf(genbuf, "boards/%s/boardrelation", bptr->header.filename);
			FILE *fp = fopen(genbuf, "r");
			char linebuf[128] = "";
			if (fp != NULL)
    			{
    				fgets(linebuf, 128, fp);
				fclose(fp);
    			}
			
			clear();
			move(1, 0);
			prints("Ó¢ÎÄ°æÃû      [1;32m%-16s[mÖÐÎÄ°æÃû      [1;32m%s[m\n"
				"Ö÷ ·Ö Çø      [1;32m%-16s[mÓ³Éä·ÖÇø      [1;32m%s[m\n"
				"°æÃæÀà±ð      [1;32m%-16s[m°æÃæÊôÐÔ      [1;32m%s[m\n"
				"¼ÇÎÄÕÂÊý      [1;32m%-16s[m×ª    ÐÅ      [1;32m%s[m\n\n"
				"°æÃæ¹Ø¼ü×Ö    [1;32m%s[m\n"
				"Ïà¹Ø°æÃæ      [1;32m%s[m\n"
				"°æÃæ¼ò½é      ",
				bptr->header.filename, bptr->header.title,
				bptr->header.sec1, bptr->header.sec2[0] ? bptr->header.sec2 : "[37m(ÎÞ)",
				bptr->header.type,  property, 
				junkboard() ? "²»" : "ÊÇ", 
				(bptr->header.flag & INNBBSD_FLAG) ? "ÊÇ" : "²»",
				bptr->header.keyword[0] ? bptr->header.keyword : "[37m(ÔÝÎÞ)",
				linebuf[0] ? linebuf: "[37m(ÔÝÎÞ)");
			setbfile(property, bptr->header.filename, "introduction");
			if (file_exist(property))
				ansimore2(property, NA, 9, 14);
			else
				prints("[1;37m%-16s[m\n", "(ÔÝÎÞ)");
			pressanykey();
			strcpy(currboard, boardbuf);
			break;

		case 'a':
			if (num >= brdnum + secnum || num < secnum)
				break;
			if (GoodBrd.num) {
				if (GoodBrd.num >= GOOD_BRC_NUM) {
					move(t_lines - 1, 0);
					prints("¸öÈËÈÈÃÅ°æÊýÒÑ¾­´ïÉÏÏÞ(%d)",
					       GOOD_BRC_NUM);
					//pressreturn();
				} else {
					char bname[STRLEN], bpath[STRLEN];
					struct stat st;
					move(0, 0);
					clrtoeol();
					prints("Ñ¡ÔñÌÖÂÛÇø [ [1;32m# [0;37m- [1;31m°æÃæÃû³Æ/¹Ø¼ü×ÖËÑË÷[0;37m, [1;32mSPACE [0;37m- ×Ô¶¯²¹È«, [1;32mENTER [0;37m- ÍË³ö ] [m\n");
					prints("ÊäÈëÌÖÂÛÇøÃû (Ó¢ÎÄ×ÖÄ¸´óÐ¡Ð´½Ô¿É): ");
					clrtoeol();
					make_blist();	
					if((namecomplete((char *) NULL, bname))=='#')
						super_select_board(bname);	
					setbpath(bpath, bname);
					if (*bname == '\0');
					if (stat(bpath, &st) == -1) {
						move(2, 0);
						prints("²»ÕýÈ·µÄÌÖÂÛÇø.\n");
						pressreturn();
					} else {
						if (!inGoodBrds(bname)) {
							strcpy(GoodBrd.ID
							       [GoodBrd.num++],
							       bname);
							save_GoodBrd();
							GoodBrd.num = 9999;
							brdnum = -1;
							break;
						}
					}
					page = -1;
				}
			} else {
				load_GoodBrd();
				ptr = &nbrd[num - secnum];
				if (GoodBrd.num >= GOOD_BRC_NUM) {
					move(t_lines - 1, 0);
					clrtoeol();
					prints("¸öÈËÈÈÃÅ°æÊýÒÑ¾­´ïÉÏÏÞ(%d)",
					       GOOD_BRC_NUM);
					GoodBrd.num = 0;
					//pressreturn();
				} else {
					if (!inGoodBrds(ptr->name)) {
						strcpy(GoodBrd.ID
						       [GoodBrd.num++],
						       ptr->name);
						save_GoodBrd();
						GoodBrd.num = 0;
						move(t_lines - 1, 0);
						clrtoeol();
						prints("°æÃæ%sÒÑ±»¼ÓÈëÊÕ²Ø¼Ð",
						       ptr->name);
						break;
					}
					GoodBrd.num = 0;
				}
			}
			break;
		case 'd':
			if (num >= brdnum + secnum || num < secnum)
				break;
			if (GoodBrd.num) {
				int i, pos;
				char ans[5];
				sprintf(genbuf, "Òª°Ñ %s ´ÓÊÕ²Ø¼ÐÖÐÈ¥µô (Y/N)? [N]",
					nbrd[num - secnum].name);
				getdata(t_lines - 1, 0, genbuf, ans, 2, DOECHO,
					YEA);
				if (ans[0] != 'y' && ans[0] != 'Y') {
					page = -1;
					break;
				}
				pos = inGoodBrds(nbrd[num - secnum].name);
				for (i = pos - 1; i < GoodBrd.num - 1; i++)
					strcpy(GoodBrd.ID[i],
					       GoodBrd.ID[i + 1]);
				GoodBrd.num--;
				save_GoodBrd();
				GoodBrd.num = 9999;
				brdnum = -1;
			}
			break;
		default:
			;
		}
		if (ch >= '0' && ch <= '9') {
			number = number * 10 + (ch - '0');
			ch = '\0';
		} else {
			number = 0;
		}
	}
	if (allbrc) {
		free(allbrc);
		allbrc = NULL;
	}
	clear();
	save_zapbuf();
	return -1;
}

static void
readwritebrc(struct allbrc *allbrc)
{
	char dirfile[STRLEN];
	if (strcmp(currentuser.userid, "guest")) {
		setuserfile(dirfile, "brc");
		brc_init(allbrc, currentuser.userid, dirfile);
		brc_putboard(allbrc, &brc);
		brc_fini(allbrc, currentuser.userid);
	} else {
		sprintf(dirfile, "%s.%s", "guest", uinfo.from);
		brc_init(allbrc, dirfile, NULL);
		brc_putboard(allbrc, &brc);
		brc_fini(allbrc, dirfile);
	}

}

void
brc_update()
{
	if (!brc.changed)
		return;
	if (allbrc) {
		readwritebrc(allbrc);
		return;
	}
	allbrc = malloc(sizeof (struct allbrc));
	readwritebrc(allbrc);
	free(allbrc);
	allbrc = NULL;

}

int
brc_initial(char *boardname, int keep)
{
	if (!strncmp(brc.board, boardname, BRC_STRLEN - 1)) {
		if (!keep && allbrc) {
			free(allbrc);
			allbrc = NULL;
		}
		return brc.num;
	}
	if (!allbrc) {
		allbrc = malloc(sizeof (struct allbrc));
		readwritebrc(allbrc);
	} else if (brc.changed)
		readwritebrc(allbrc);
	brc_getboard(allbrc, &brc, boardname);
	if (!keep) {
		free(allbrc);
		allbrc = NULL;
	}
	return brc.num;
}

void
clear_new_flag_quick(int t)
{
	int bnum;
	if (!t) {
		t = time(NULL);
		bnum = getbnum(brc.board);
		if (bnum && bcache[bnum - 1].lastpost > t)
			t = bcache[bnum - 1].lastpost;
	}
	brc_clearto(&brc, t);
}

void
clear_all_new_flag()
{
	int i;
	char ans[3];
	int brdnum;
	boardprefix[0] = 255;
	boardprefix[1] = 0;
	GoodBrd.num = 0;
	brdnum = -1;
	getdata(t_lines - 2, 0, "È·¶¨ÒªÇå³ýËùÓÐ°æÃæµÄÎ´¶Á±ê¼Ç£¿(Y/N) [N]:", ans,
		2, DOECHO, YEA);
	if (ans[0] != 'y' && ans[0] != 'Y')
		return;
	if (load_boards(&brdnum, 0) < 0)
		return;
	for (i = 0; i < brdnum; i++) {
		brc_initial(nbrd[i].name, (i == brdnum - 1) ? 0 : 1);
		clear_new_flag_quick(0);
	}
	brc_update();
}

int
Read()
{
	char buf[STRLEN];
	char notename[STRLEN];
	time_t usetime;
	struct stat st;
	if (!selboard || !strcmp(currboard, "")) {
		move(2, 0);
		prints("ÇëÏÈÑ¡ÔñÌÖÂÛÇø\n");
		pressreturn();
		move(2, 0);
		clrtoeol();
		return -1;
	}
	modify_user_mode(READING);
	brc_initial(currboard, 0);
	setbdir(buf, currboard, digestmode);
	if (!clubsync(currboard))
		return 0;
	if (uinfo.curboard && bcache[uinfo.curboard - 1].inboard > 0)
		bcache[uinfo.curboard - 1].inboard--;
	uinfo.curboard = getbnum(currboard);
	update_utmp();
	bcache[uinfo.curboard - 1].inboard++;
	setvfile(notename, currboard, "notes");
	clear();
	if (stat(notename, &st) != -1) {
		if (st.st_mtime > brc.notetime
		    || now_t - brc.notetime > 7 * 86400) {
			show_board_notes(currboard);
			brc.notetime = now_t;
			brc.changed = 1;
			if (!DEFINE(DEF_INTOANN) || brc.num > 1)
				pressanykey();
		}
	}

	if (DEFINE(DEF_INTOANN) && brc_unreadt(&brc, 2)) {
		char ans[3];
		getdata(t_lines - 1, 0,
			"\033[0m\033[1mÄú³õ´Î·ÃÎÊ±¾°æ, ÊÇ·ñÊ×ÏÈ²ì¿´¾«»ªÇø?"
			" (Ñ¡A²»ÔÙ×ö´ËÌáÊ¾)(Y/N/A) [Y]:", ans, 3, DOECHO, YEA);
		brc_addlistt(&brc, 2);
		if (ans[0] == 'A' || ans[0] == 'a') {
			set_safe_record();
			currentuser.userdefine &= ~DEF_INTOANN;
			substitute_record(PASSFILE, &currentuser,
					  sizeof (currentuser), usernum);
		} else if (ans[0] != 'N' && ans[0] != 'n') {
			into_announce();
			show_board_notes(currboard);
			move(t_lines - 1, 0);
			prints
			    ("\033[0m\033[1m»¶Ó­¹âÁÙ, °´ÈÎÒâ¼ü½øÈë±¾°æ°æÃæ, ÔÚ°æÃæ°´'x'¿ÉÒÔËæÊ±½øÈë¾«»ªÇø");
			egetch();
		}
	}
	usetime = time(NULL);
	ISdelrq = clubsync("deleterequest");
	i_read(READING, buf, readtitle,
	       (void *) readdoent, read_comms, sizeof (struct fileheader));
	now_t = time(NULL);
	if (now_t - usetime > 1) {
		snprintf(genbuf, 256, "%s use %s %ld",
			 currentuser.userid, currboard,
			 (long int) (now_t - usetime));
		newtrace(genbuf);
	}
	brc_update();
	if (uinfo.curboard && bcache[uinfo.curboard - 1].inboard > 0)
		bcache[uinfo.curboard - 1].inboard--;
	uinfo.curboard = 0;
	update_utmp();
	return 0;
}

int
showhell()
{
	selboard = 1;
	strcpy(currboard, "hell");
	return Read();
}

int
showprison()
{
	selboard = 1;
	strcpy(currboard, "prison");
	return Read();
}

static int
readtitle()
{
	struct boardmem *bp;

	char header[200], title[STRLEN];
	char readmode[10];
	int active, invisible, i, bnum;
	char tmp[40];
	bp = getbcache(currboard);
	if (bp == NULL)
		return -1;
	IScurrBM = chk_currBM(&(bp->header), 0);
	//ISdelrq = clubsync("deleterequest");

	bnum = 0;
	if (bp->header.bm[0][0] == 0) {
		strcpy(header, "³ÏÕ÷°æÖ÷ÖÐ");
	} else {
		strcpy(header, "°æÖ÷: ");
		for (i = 0; i < 4; i++) {	//Ö»ÏÔÊ¾Ç°ËÄ¸ö´ó°à³¤
			if (bp->header.bm[i][0] == 0)
				break;
			active = bp->bmonline & (1 << i);
			invisible = bp->bmcloak & (1 << i);
			if (active && !invisible)
				sprintf(tmp, "\x1b[32m%s\x1b[33m ",
					bp->header.bm[i]);
			else if (active && invisible && (HAS_PERM(PERM_SEECLOAK)
							 || !strcmp(bp->
								    header.bm
								    [i],
								    currentuser.
								    userid)))
				    sprintf(tmp, "\x1b[36m%s\x1b[33m ",
					    bp->header.bm[i]);
			else
				sprintf(tmp, "%s ", bp->header.bm[i]);
			strcat(header, tmp);
		}
	}
	if (chkmail())
		strcpy(title, "[ÄúÓÐÐÅ¼þ,Çë°´ w ²é¿´ÐÅ¼þ]");
	else if ((bp->header.flag & VOTE_FLAG))
		sprintf(title, "¡ù Í¶Æ±ÖÐ£¬°´ v ½øÈëÍ¶Æ± ¡ù");
	else
		strcpy(title, bp->header.title);

	showtitle(header, title);
	prints
	    ("Àë¿ª[\x1b[1;32m¡û\x1b[m,\x1b[1;32mq\x1b[m] Ñ¡Ôñ[\x1b[1;32m¡ü\x1b[m,\x1b[1;32m¡ý\x1b[m] ÔÄ¶Á[\x1b[1;32m¡ú\x1b[m,\x1b[1;32mRtn\x1b[m] ·¢±íÎÄÕÂ[\x1b[1;32mCtrl-P\x1b[m] ¿³ÐÅ[\x1b[1;32md\x1b[m] ±¸ÍüÂ¼[\x1b[1;32mTAB\x1b[m] ÇóÖú[\x1b[1;32mh\x1b[m]\n");
	switch (digestmode) {
	case 0:
		if (DEFINE(DEF_THESIS))	/* youzi 1997.7.8 */
			strcpy(readmode, "Ö÷Ìâ");
		else
			strcpy(readmode, "Ò»°ã");
		break;
	case 1:
		strcpy(readmode, "ÎÄÕª");
		break;
	case 2:
		strcpy(readmode, "Ö÷Ìâ");
		break;
	case 3:
		strcpy(readmode, "·ÀË®");
		break;
	case 4:
		strcpy(readmode, "»ØÊÕ");
		break;
	case 5:
		strcpy(readmode, "Ö½Â¨");
		break;
	}
	if (DEFINE(DEF_THESIS) && digestmode == 0)
		prints
		    ("\x1b[1;37;44m ±àºÅ   %-12s %6s %-28sÔÚÏß:%4d [%4sÊ½¿´°æ] \x1b[m\n",
		     "¿¯ µÇ Õß", "ÈÕ  ÆÚ", " ±ê  Ìâ", bp->inboard, readmode);
	else
		prints
		    ("\x1b[1;37;44m ±àºÅ   %-12s %6s %-30sÔÚÏß:%4d [%4sÄ£Ê½] \x1b[m\n",
		     "¿¯ µÇ Õß", "ÈÕ  ÆÚ", " ±ê  Ìâ", bp->inboard, readmode);
	clrtobot();
	return 0;
}

static char *
readdoent(num, ent, buf)
int num;
struct fileheader *ent;
char buf[512];
{
	char date[80], owner[13];
	char *TITLE;
	char type, *type1, type2, danger = 0;
	char typestring[32];
	int noreply = 0, attached = 0, allcanre = 0;

	{
		char *ptr;
		strsncpy(owner, ent->owner, 13);
		if (!owner[0])
			strcpy(owner, "Anonymous");
		if ((ptr = strchr(owner, '.')) != NULL)
			*(ptr + 1) = 0;
	}

	type = (UNREAD(ent, &brc) ? '*' : ' ');
	//add by hace 2003.05.02
//comment by bjgyt	if(ent->accessed & FILE_TOP1){
	if(IScurrBM&&ent->accessed & FILE_TOP1){
	    type='#';
	}
	//end
	if(IScurrBM && (ent->accessed & FH_ISWATER)) { // type Î»Ë®ÎÄ±ê¼ÇÏÔÊ¾ by IronBlood
		if(type == ' ')
			type = 'w';
		else
			type = 'W';
	}

	if ((ent->accessed & FH_DIGEST)) {
		if (type == ' ')
			type = 'g';
		else
			type = 'G';
	}
	if ((ent->accessed & FH_DANGEROUS) && (IScurrBM || ISdelrq))
		danger = 1;
	if ((ent->accessed & FH_DEL) && (IScurrBM || HAS_PERM(PERM_ARBITRATE))) {
		if (danger)
			type1 = "\033[1;31mX\033[0m";
		else
			type1 = "X";
	} 
	else if((ent->accessed & FH_MINUSDEL) && (IScurrBM || HAS_PERM(PERM_ARBITRATE))){	//add by mintbaggio 040322 for minus-numposts delete
		if (danger)
                        type1 = "\033[1;31mx\033[0m";
                else
                        type1 = "x";
	}
	else {
		if (danger)
			type1 = "\033[1;31m!\033[0m";
		else
			type1 = " ";
	}
	type2 = (ent->accessed & FH_SPEC) ? (IScurrBM ? '$' : ' ') : ' ';
	danger =
	    (ent->accessed & FH_DANGEROUS) ? ((IScurrBM || ISdelrq) ? 1 : 0) :
	    0;
	noreply = ent->accessed & FH_NOREPLY;
	attached = ent->accessed & FH_ATTACHED;
	allcanre = ent->accessed & FH_ALLREPLY;
	if((HAS_PERM(PERM_SYSOP|PERM_OBOARDS)||has_perm_commend(currentuser.userid)) && is_in_commend(currboard, ent)){	//add by mintbaggio 040327 for front page commend
		if(type == '*')	
			type1 = "\033[1;45mM\033[0m";
		else
			type1 = "\033[1;45mm\033[0m";
	}
	if((HAS_PERM(PERM_SYSOP|PERM_OBOARDS)||has_perm_commend(currentuser.userid)) && is_in_commend2(currboard, ent)){	//add by mintbaggio 040327 for front page commend
		if(type == '*')	
			type1 = "\033[1;42mM\033[0m";
		else
			type1 = "\033[1;42mm\033[0m";
	}
	if (ent->accessed & FH_MARKED) {
		switch (type) {
		case ' ':
			type = 'm';
			break;
		case '*':
			type = 'M';
			break;
		case 'g':
			type = 'b';
			break;
		case 'G':
			type = 'B';
			break;
		}
	}
	if (IScurrBM && (ent->accessed & FH_ANNOUNCE)) {
		sprintf(typestring, "%s\x1b[1;32;42m%c\x1b[m%c", type1, type,
			type2);
	} else
		sprintf(typestring, "%s%c%c", type1, type, type2);

	makedatestar(date, ent);
	/*  Re-Write By Excellent */

	TITLE = ent->title;
	filter(TITLE);
	if (uinfo.mode != RMAIL && digestmode != 1 && digestmode != 4
	    && digestmode != 5) {	//ÓÃÐÂ·½·¨
		if (ent->thread != ent->filetime && !strncmp(TITLE, "Re: ", 4)) {	//ReÎÄ
			if (readingthread == ent->thread)	//µ±Ç°ÕýÔÚ¶ÁµÄÖ÷Ìâ
				sprintf(buf,
					" \033[1;36m%4d\033[m%s%-12.12s%s\033[1;36m.%c%sRe:\033[0;1;36m%-.45s\033[m",
					num, typestring, owner, date,
					attached ? '@' : ' ',
					noreply ? "\033[0;1;4;33m" : "",
					TITLE + 3);
			else
				sprintf(buf,
					" %4d%s%-12.12s%s %c%sRe:\033[m%-.45s\033[m",
					num, typestring, owner, date,
					attached ? '@' : ' ',
					noreply ? "\033[0;1;4;33m" : "",
					TITLE + 3);
		} else {
			if (readingthread == ent->thread)
				sprintf(buf,
					" \033[1;33m%4d\033[m%s%-12.12s%s\033[1;33m.%c%s¡ñ\033[0;1;33m %-.45s\033[m",
					num, typestring, owner, date,
					attached ? '@' : ' ',
					noreply ? "\033[0;1;4;33m" : "", TITLE);
			else
				sprintf(buf,
					" %4d%s%-12.12s%s %c%s%s %-.45s\033[m",
					num, typestring, owner, date,
					attached ? '@' : ' ',
					noreply ? "\033[0;1;4;33m" : "",
					allcanre ? "\033[1;31m¡ñ" : "¡ñ\033[m",
					TITLE);
		}
	} else {
		if (!strncmp("Re:", ent->title, 3)
		    || !strncmp("RE:", ent->title, 3)) {
			if (strncmp(ReplyPost, ent->title, 45) == 0) {
				sprintf(buf,
					" \x1b[1;36m%4d\x1b[m%s%-12.12s%s\x1b[1;36m.%c%sRe:\033[0;1;36m%-.45s\033[m",
					num, typestring, owner, date,
					attached ? '@' : ' ',
					noreply ? "\033[0;1;4;33m" : "",
					TITLE + 3);
			} else {
				sprintf(buf,
					" %4d%s%-12.12s%s %c%sRe:\033[m%-.45s\033[m",
					num, typestring, owner, date,
					attached ? '@' : ' ',
					noreply ? "\033[0;1;4;33m" : "",
					TITLE + 3);
			}
		} else {
			if (strncmp(ReadPost, ent->title, 45) == 0) {
				sprintf(buf,
					" \x1b[1;33m%4d\x1b[m%s%-12.12s%s\x1b[1;33m.%c%s¡ñ\033[0;1;33m %-.45s\033[m",
					num, typestring, owner, date,
					attached ? '@' : ' ',
					noreply ? "\033[0;1;4;33m" : "", TITLE);
			} else {
				sprintf(buf,
					" %4d%s%-12.12s%s %c%s%s %-.45s\033[m",
					num, typestring, owner, date,
					attached ? '@' : ' ',
					noreply ? "\033[0;1;4;33m" : "",
					allcanre ? "\033[1;31m¡ñ" : "¡ñ\033[m",
					TITLE);
			}
		}
	}
	// add by hace 2003.05.05
	char path[64];
	struct stat st;
	char msg[32];
	setbdir(path, currboard, digestmode);
	if(stat(path,&st)==-1 ) errlog("error");
	if((stat(path,&st)!=-1 )&& (st.st_size/sizeof(struct fileheader))< num ){
	    ent->accessed |= FILE_TOP1; 
	    //errlog("slowaction");
	    if((ent->accessed& FH_MARKED)&&(ent->accessed&FH_DIGEST)) strcpy(msg,"\033[1;31m[ÌáÊ¾]\033[0m");
	    else if(ent->accessed & FH_DIGEST) strcpy(msg,"\033[1;33m[ÍÆ¼ö]\033[0m");
	    else if(ent->accessed & FH_MARKED) strcpy(msg,"\033[1;36m[ÌáÊ¾]\033[0m");
	    else strcpy(msg,"\033[1;32m[ÌáÊ¾]\033[0m");
	    msg[31]=0;
	    sprintf(buf, " %6s %-12.12s%s\033[m %s¡ô %-.45s ",msg,ent->owner, date," ", TITLE);
	}
	//end
	return buf;
}

static char *
makedatestar(char *datestr, struct fileheader *ent)
{
	char str[30] = " ", buf[30];
	char backcolor[6] = "062351";
	int i, j, better, sz;
	char fg, bg;
	time_t filetime;

	filetime = ent->filetime;
	fg = '1' + localtime(&filetime)->tm_wday;
	bg = backcolor[ent->staravg50 / 50];

	better = (ent->hasvoted - 1) / 5 + 1;
	if (better >= 7)
		better = 7;
	if (ent->hasvoted == 0)
		better = 0;

	sz = ent->sizebyte;
	if (sz > 30)
		sz = 1 + (sz - 30) / 25;
	else
		sz = 0;

	if (sz > 7)
		sz = 7;

	if (filetime > 740000000)
		strncpy(str + 1, ctime(&filetime) + 4, 6);
	else
		strncpy(str + 1, "      ", 6);
	sprintf(datestr, "\033[1;4;3%c;4%cm", fg, bg);
	j = strlen(datestr);
	for (i = 0; i < 7; i++) {
		if (i == better && better == sz) {
			sprintf(buf, "\033[0;1;3%cm", fg);
			strcpy(datestr + j, buf);
			j += strlen(buf);
		} else if (i == better) {
			if (sz > better)
				sprintf(buf, "\033[0;1;4;3%cm", fg);
			else
				sprintf(buf, "\033[0;1;3%cm", fg);
			strcpy(datestr + j, buf);
			j += strlen(buf);
		} else if (i == sz) {
			if (sz > better)
				sprintf(buf, "\033[0;1;3%cm", fg);
			else
				sprintf(buf, "\033[0;1;3%c;4%cm", fg, bg);
			strcpy(datestr + j, buf);
			j += strlen(buf);
		}
		datestr[j] = str[i];
		j++;
	}
	datestr[j] = 0;
	strcat(datestr, "\033[m");
	return datestr;
}

int
clubtest(char *board)
{
        char buf[256];
        sprintf(buf, "boards/%s/club_users", board);
        return seek_in_file(buf, currentuser.userid);
}
//add by wjbta for moneycenter
