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
#include "bbs_global_vars.h"
#include "editboard.h"
#include "bbsinc.h"
#include "io.h"
#include "smth_screen.h"
#include "main.h"
#include "stuff.h"
#include "bcache.h"
#include "xyz.h"
#include "sendmsg.h"
#include "more.h"
#include "1984.h"
#include "read.h"
#include "mail.h"
#include "edit.h"
#include "maintain.h"
#include "namecomplete.h"
#include "help.h"
#include "boards.h"
#include "talk.h"
#include "list.h"
#include "announce.h"
#include "vote.h"
#include "bbs-internal.h"
#include "ythtbbs/mybrd.h"

#define BBS_PAGESIZE    (t_lines - 4)

struct allbrc *allbrc = NULL;
struct onebrc brc = { 0, 0, 0, "", { 0 }, 0 };
extern char *sysconf_str(char *);
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
static char boardprefix[5];

struct goodboard GoodBrd;

static void load_GoodBrd(void);
static void save_GoodBrd(void);
static void load_zapbuf(void);
static void save_zapbuf(void);
static int zapped(int n, struct boardmem *bptr);
static int load_boards(int *brdnum, int secnum);
static int search_board(int *num, int brdnum, int secnum);
static int check_newpostt(struct newpostdata *ptr);
static void show_brdlist(int page, int clsflag, int newflag, int brdnum, const struct sectree *sec);
static int cmpboard(struct newpostdata *brd, struct newpostdata *tmp);
static int choose_board(int newflag, const struct sectree *sec);
static void readwritebrc(struct allbrc *allbrc);
static int readtitle();   // 输出阅读头部
static char *readdoent(int num, struct fileheader *ent, char buf[512]);
static char *makedatestar(char *datestr, struct fileheader *ent);

// 菜单的调用函数
int GoodBrds(const char *s) {
	(void) s;
//  if(!strcmp(currentuser.userid,"guest")) return;
	GoodBrd.num = 9999;
	boardprefix[0] = 0xFF;
	boardprefix[1] = 0;
	choose_board(1, NULL);
	return 0;
}

bool load_GoodBrd_has_read_perm(const char *userid, const char *boardname) {
	(void) userid;
	return canberead(boardname);
}

//从文件中获取订阅版面，填充数据结构 GoodBrd
static void load_GoodBrd() {
	ythtbbs_mybrd_load(currentuser.userid, &GoodBrd, load_GoodBrd_has_read_perm);
}

static bool term_has_read_perm(const char *userid, const char *boardname) {
	(void) userid;
	return (canberead(boardname) != 0);
}

static void
save_GoodBrd()			// 保存用户订阅的版面
{
	if (GoodBrd.num <= 0) {
		GoodBrd.num = 1;
		if (ythtbbs_cache_Board_get_board_by_name(DEFAULTBOARD))
			strcpy(GoodBrd.ID[0], DEFAULTBOARD);
		else
			strcpy(GoodBrd.ID[0], currboard);
	}
	ythtbbs_mybrd_save(currentuser.userid, &GoodBrd, term_has_read_perm);
}

int EGroup(const char *cmd) {
	const struct sectree *sec;
	GoodBrd.num = 0;
	boardprefix[0] = cmd[0];
	boardprefix[1] = 0;
	sec = getsectree(boardprefix);
	choose_board(DEFINE(DEF_NEWPOST, currentuser) ? 1 : 0, sec);
	return 0;
}

int Boards(const char *s) {
	(void) s;
	boardprefix[0] = 0xFF;
	boardprefix[1] = 0;
	GoodBrd.num = 0;
	choose_board(0, NULL);
	return 0;
}

int New(const char *s) {
	(void) s;
	boardprefix[0] = 0xFF;
	boardprefix[1] = 0;
	GoodBrd.num = 0;
	choose_board(1, NULL);
	return 0;
}

static void
load_zapbuf()
{
	char fname[STRLEN];
	int fd, size;

	size = MAXBOARD * sizeof (int);
	zapbuf = (int *) malloc(size);
	bzero(zapbuf, size);
	sethomefile_s(fname, sizeof(fname), currentuser.userid, ".newlastread");
	if ((fd = open(fname, O_RDONLY, 0600)) != -1) {
		size = ythtbbs_cache_Board_get_number() * sizeof (int);
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

	sethomefile_s(fname, sizeof(fname), currentuser.userid, ".newlastread");
	if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) != -1) {
		size = ythtbbs_cache_Board_get_number() * sizeof (int);
		write(fd, zapbuf, size);
		close(fd);
	}
}

static int
zapped(int n, struct boardmem *bptr)
{
	if (zapbuf[n] == 0)	//没被 z
		return 0;
	if (zapbuf[n] < bptr->header.board_ctime)	//z掉了，但是不是这个版了
		return 0;
	return 1;
}

static int load_boards_callback(struct boardmem *board, int curr_idx, va_list ap) {
	int local_goodbrd = va_arg(ap, int);
	char *local_boardprefix = va_arg(ap, char *); // TODO unsigned char *?
	int local_yank_flag = va_arg(ap, int);

	struct newpostdata *local_nbrd = va_arg(ap, struct newpostdata *);
	int *local_brdnum = va_arg(ap, int *);

	int local_addto = 0;
	struct newpostdata *local_ptr;

	if (board->header.filename[0] == '\0')
		return 0;

	if (local_goodbrd == 0) {
		if((unsigned char)local_boardprefix[0] != 0xFF /* 255*/
				&& local_boardprefix[0] != '*'
				&& strcmp(local_boardprefix, board->header.sec1)
				&& (board->header.sec2[0] == 0 || strcmp(local_boardprefix, board->header.sec2)))
			return 0;

		if (!hasreadperm(&board->header))
			return 0;

		local_addto = local_yank_flag || !zapped(curr_idx, board) || (board->header.level & PERM_NOZAP);
	} else {
		// 判断是否是订阅的版面
		local_addto = ythtbbs_mybrd_exists(&GoodBrd, board->header.filename);
	}

	if (local_addto) {
		// addto 标志该版面应该可以阅读
		local_ptr = &local_nbrd[*local_brdnum];
		*local_brdnum = *local_brdnum + 1;

		local_ptr->name = board->header.filename;
		local_ptr->flag = board->header.flag | ((board->header.level & PERM_NOZAP) ? NOZAP_FLAG : 0);
		local_ptr->pos = curr_idx;
		local_ptr->unread = -1;
		local_ptr->zap = zapped(curr_idx, board);

		if (board->header.level & PERM_POSTMASK)
			local_ptr->status = 'p';
		else if (board->header.level & PERM_NOZAP)
			local_ptr->status = 'z';
		else if ((board->header.level & ~PERM_POSTMASK) != 0)
			local_ptr->status = 'r';
		else {
			local_ptr->status = ' ';

			if (board->header.clubnum != 0) {
				if (board->header.flag & CLUBTYPE_FLAG) {
					if (HAS_CLUBRIGHT(board->header.clubnum, uinfo.clubrights)) {
						local_ptr->status = 'O';
					} else {
						local_ptr->status = 'o';
					}
				} else {
					local_ptr->status = 'c';
				}
			}
		}
	}
	return 0;
}

static int
load_boards(int *brdnum, int secnum)
{
	int goodbrd = 0;
	static int loadtime = 0;

	ythtbbs_cache_Board_resolve();
	if (!(GoodBrd.num == 9999 || ythtbbs_cache_Board_get_uptime() >= loadtime || zapbuf == NULL || *brdnum <= 0))
		return 0;
	loadtime = time(NULL);
	if (zapbuf == NULL) {
		load_zapbuf();
	}

	*brdnum = 0;
	if (GoodBrd.num)
		goodbrd = 1;	// 表示处于阅读定制版面状态
	if (GoodBrd.num == 9999)	// 强制 load 订阅版面
		load_GoodBrd();
	ythtbbs_cache_Board_foreach_v(load_boards_callback, goodbrd, boardprefix, yank_flag, nbrd, brdnum);
	if (*brdnum == 0 && secnum == 0 && !yank_flag) {
		if (goodbrd) {	// 如果处于定制版面中，但没有任何版面的话，则刷新
			GoodBrd.num = 0;
			save_GoodBrd();
			GoodBrd.num = 9999;
		} else {
			char ans[3];
			getdata(t_lines - 1, 0,
				// 该讨论区组的版面已经被你全部取消了，是否查看所有讨论区？(Y/N)[N]
				"\xB8\xC3\xCC\xD6\xC2\xDB\xC7\xF8\xD7\xE9\xB5\xC4\xB0\xE6\xC3\xE6\xD2\xD1\xBE\xAD\xB1\xBB\xC4\xE3\xC8\xAB\xB2\xBF\xC8\xA1\xCF\xFB\xC1\xCB\xA3\xAC\xCA\xC7\xB7\xF1\xB2\xE9\xBF\xB4\xCB\xF9\xD3\xD0\xCC\xD6\xC2\xDB\xC7\xF8\xA3\xBF" "(Y/N)[N]",
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

static int search_board(int *num, int brdnum, int secnum)
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
		// 请输入要找寻的 board 名称：%s
		prints("\xC7\xEB\xCA\xE4\xC8\xEB\xD2\xAA\xD5\xD2\xD1\xB0\xB5\xC4" " board " "\xC3\xFB\xB3\xC6\xA3\xBA" "%s", bname);
		ch = egetch();

		if (isprint2(ch)) {
			bname[i++] = ch;
			for (n = secnum; n < brdnum + secnum; n++) {
				if (!strncasecmp(nbrd[n - secnum].name, bname, i)) {
					tmpn = YEA;
					*num = n;
					if (!strcmp(nbrd[n - secnum].name, bname))
						return 1;	/*找到类似的版，画面重画 */
				}
			}
			if (tmpn)
				return 1;
			if (find == NA) {
				bname[--i] = '\0';
			}
			continue;
		} else if (ch == Ctrl('H') || ch == KEY_LEFT || ch == KEY_DEL || ch == '\177') {
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
		return 2 /*结束了 */ ;
	}
	return 1;
}

static int check_newpostt(struct newpostdata *ptr)
{
	struct boardmem *bptr;
	if (!allbrc) {
		allbrc = malloc(sizeof (struct allbrc));
		readwritebrc(allbrc);
	}

	//prints("pos=%d\n", ptr->pos);
	bptr = ythtbbs_cache_Board_get_board_by_idx(ptr->pos);
	if (bptr->total <= 0){		//mint
	//	prints("if");
		ptr->unread = 0;
	}
	else{
	//	prints("else");
		ptr->unread = brc_unreadt_quick(allbrc, ptr->name, bptr->lastpost);
	}
	//pressanykey();
	return 0;
}

int unread_position(char *dirfile, struct boardmem *bptr) {
	int fd, offset, step, num, filetime;

	num = bptr->total + 1;
	if ((fd = open(dirfile, O_RDONLY)) >= 0) {
		if (!brc_initial(bptr->header.filename, 1)) {
			num = 1;
		} else {
			offset = offsetof(struct fileheader, filetime);
			num = bptr->total - 1;
			step = 4;
			while (num > 0) {
				lseek(fd, offset + num * sizeof (struct fileheader), SEEK_SET);
				if (read(fd, &filetime, sizeof (filetime)) <= 0 || !brc_unreadt(&brc, filetime))
					break;
				num -= step;
				if (step < 32)
					step += step / 2;
			}
			if (num < 0)
				num = 0;
			while (num < bptr->total) {
				lseek(fd, offset + num * sizeof (struct fileheader), SEEK_SET);
				if (read(fd, &filetime, sizeof (filetime)) <= 0 || brc_unreadt(&brc, filetime))
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

static void show_brdlist(int page, int clsflag, int newflag, int brdnum, const struct sectree *sec)
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
		// [讨论区列表] [分类]
		strcpy(title, "[" "\xCC\xD6\xC2\xDB\xC7\xF8\xC1\xD0\xB1\xED" "] [" "\xB7\xD6\xC0\xE0" "]");
		break;
	case 0x10:
		// [讨论区列表] [人气]
		strcpy(title, "[" "\xCC\xD6\xC2\xDB\xC7\xF8\xC1\xD0\xB1\xED" "] [" "\xC8\xCB\xC6\xF8" "]");
		break;
	case 0x20:
		// [讨论区列表] [字母]
		strcpy(title, "[" "\xCC\xD6\xC2\xDB\xC7\xF8\xC1\xD0\xB1\xED" "] [" "\xD7\xD6\xC4\xB8" "]");
		break;
	case 0x30:
		// [讨论区列表] [在线]
		strcpy(title, "[" "\xCC\xD6\xC2\xDB\xC7\xF8\xC1\xD0\xB1\xED" "] [" "\xD4\xDA\xCF\xDF" "]");
		break;
	default:
		// [讨论区列表]
		strcpy(title, "[" "\xCC\xD6\xC2\xDB\xC7\xF8\xC1\xD0\xB1\xED" "]");
		break;
	}

	if (clsflag) {
		clear();
		docmdtitle(title,
				//   \033[m主选单[\033[1;32m←\033[m,\033[1;32me\033[m] 阅读[\033[1;32m→\033[m,\033[1;32mRtn\033[m] 选择[\033[1;32m↑\033[m,\033[1;32m↓\033[m] 列出[\033[1;32my\033[m] 排序[\033[1;32ms\033[m] 搜寻[\033[1;32m/\033[m] 切换[\033[1;32mc\033[m] 求助[\033[1;32mh\033[m]\n
				"  \033[m" "\xD6\xF7\xD1\xA1\xB5\xA5" "[\033[1;32m" "\xA1\xFB" "\033[m,\033[1;32me\033[m] " "\xD4\xC4\xB6\xC1" "[\033[1;32m" "\xA1\xFA" "\033[m,\033[1;32mRtn\033[m] " "\xD1\xA1\xD4\xF1" "[\033[1;32m" "\xA1\xFC" "\033[m,\033[1;32m" "\xA1\xFD" "\033[m] " "\xC1\xD0\xB3\xF6" "[\033[1;32my\033[m] " "\xC5\xC5\xD0\xF2" "[\033[1;32ms\033[m] " "\xCB\xD1\xD1\xB0" "[\033[1;32m/\033[m] " "\xC7\xD0\xBB\xBB" "[\033[1;32mc\033[m] " "\xC7\xF3\xD6\xFA" "[\033[1;32mh\033[m]\n");
		// \033[1;44;37m %s 讨论区名称   V  类别  %-21sS 版  主      在线   人气\033[m\n
		prints("\033[1;44;37m %s " "\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB\xB3\xC6" "   V  " "\xC0\xE0\xB1\xF0" "  %-21sS " "\xB0\xE6" "  " "\xD6\xF7" "      " "\xD4\xDA\xCF\xDF" "   " "\xC8\xCB\xC6\xF8" "\033[m\n",
				//  全部 未
				//  编号 未
				// 中  文  叙  述
				newflag ? " " "\xC8\xAB\xB2\xBF" " " "\xCE\xB4" : " " "\xB1\xE0\xBA\xC5" " " "\xCE\xB4", "\xD6\xD0" "  " "\xCE\xC4" "  " "\xD0\xF0" "  " "\xCA\xF6");
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
				// ＋
				prints("\xA3\xAB" " ");
				prints("%s\n", sec->subsec[n]->title);
			}
		} else {
			ptr = &nbrd[n - secnum];
			bptr = ythtbbs_cache_Board_get_board_by_idx(ptr->pos);
			if (ptr->unread == -1)
				check_newpostt(ptr);
			if (!newflag)
				prints(" %4d  ", n + 1 - secnum);
			else
				prints(" %5d ", bptr->total);
			prints("%s%c",
					// \033[1;32m●\033[m
					// \033[1;32m○\033[m
					// ◆
					// ◇
					(ptr->flag & INNBBSD_FLAG) ? (ptr->unread ?  "\033[1;32m" "\xA1\xF1" "\033[m" : "\033[1;32m" "\xA1\xF0" "\033[m") : (ptr->unread ? "\xA1\xF4" : "\xA1\xF3"),
					(ptr->zap && !(ptr->flag & NOZAP_FLAG)) ? '-' : ' ');
			strncpy(tmpBM, bptr->header.bm[0], IDLEN);
			sprintf(buf, "[%s] %s", bptr->header.type, bptr->header.title);
			if (ptr->status == 'p')
				// [只读]
				memcpy(buf, "[" "\xD6\xBB\xB6\xC1" "]", 6);
			prints("%-13s%s%-28s %c %-12s%4d %6d\n", ptr->name,
					(ptr->flag & VOTE_FLAG) ? "\033[1;31mV\033[m" : " ",
					buf, ptr->status,
					// 诚征版主中
					(tmpBM[0] == '\0' ? "\xB3\xCF\xD5\xF7\xB0\xE6\xD6\xF7\xD6\xD0" : tmpBM),
					bptr->inboard, bptr->score);	//HAS_PERM(PERM_POST)?ptr->status:' ',
		}
	}
}
static int cmpboard(struct newpostdata *brd, struct newpostdata *tmp)
{
	int type = 0;
	unsigned char sorttype;
	struct boardmem *bptrbrd, *bptrtmp;
	sorttype = currentuser.flags[0] & BRDSORT_MASK;
	bptrbrd = ythtbbs_cache_Board_get_board_by_idx(brd->pos);
	bptrtmp = ythtbbs_cache_Board_get_board_by_idx(tmp->pos);

	switch (sorttype) {
	case 0x00:
		type = bptrbrd->header.sec1[0] - bptrtmp->header.sec1[0];
		if (type == 0)
			type = strncasecmp(bptrbrd->header.type, bptrtmp->header.type, 4);
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

static int choose_board(int newflag, const struct sectree *sec)
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
			qsort(nbrd, brdnum, sizeof (nbrd[0]), (void *) cmpboard);
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
			show_allmsgs(0, NULL, NULL);
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
			return Q_Goodbye(0, NULL, NULL);
		case 'w':
			if ((in_mail != YEA) && HAS_PERM(PERM_READMAIL, currentuser))
				m_read(NULL);
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
			qsort(nbrd, brdnum, sizeof (nbrd[0]), (void *) cmpboard);
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
			if (HAS_PERM(PERM_BASIC, currentuser) && !(nbrd[num - secnum].flag & NOZAP_FLAG)) {
				ptr = &nbrd[num - secnum];
				ptr->zap = !ptr->zap;
				ptr->unread = -1;
				zapbuf[ptr->pos] = (ptr->zap ? now_t : 0);
				zapbufchanged = 1;
				page = 999;
			}
			break;
		case 'C':
			if (HAS_PERM(PERM_SPECIAL2, currentuser) || clubsync("deleterequest")) {
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
				if (DEFINE(DEF_FIRSTNEW, currentuser)) {
					char buf[STRLEN];
					setbdir(buf, currboard, digestmode);
					if (getkeep(buf, -1, 0) == NULL) {
						tmp = unread_position(buf, ythtbbs_cache_Board_get_board_by_idx(ptr->pos));
						page = tmp - t_lines / 2;
						getkeep(buf, page > 1 ? page : 1, tmp + 1);
					}
				}
				Read(NULL);
				ptr->unread = page = -1;
				modify_user_mode(newflag ? READNEW : READBRD);
			} else {
				if (sec) {
					ytht_strsncpy(boardprefix, sec->subsec[num]->basestr, sizeof boardprefix);
					choose_board(newflag, sec->subsec[num]);
					ytht_strsncpy(boardprefix, sec->basestr, sizeof boardprefix);
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
			// 选择阅读: (0)取消 (1)BMY推荐文章 (2)本日十大热门话题 [2]:
			getdata(t_lines - 2, 0, "\xD1\xA1\xD4\xF1\xD4\xC4\xB6\xC1" ": (0)" "\xC8\xA1\xCF\xFB" " (1)BMY" "\xCD\xC6\xBC\xF6\xCE\xC4\xD5\xC2" " (2)" "\xB1\xBE\xC8\xD5\xCA\xAE\xB4\xF3\xC8\xC8\xC3\xC5\xBB\xB0\xCC\xE2" " [2]:", ans, 3, DOECHO, YEA);
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
				if (DEFINE(DEF_FILTERXXX, currentuser))
				{
					// 以下为了实现telnet下边直接看10大,modified by interma@BMY 2005.6.25

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
							// 选择阅读（请输入序号, Enter或0为退出）:
							getdata(t_lines - 2, 0, "\xD1\xA1\xD4\xF1\xD4\xC4\xB6\xC1\xA3\xA8\xC7\xEB\xCA\xE4\xC8\xEB\xD0\xF2\xBA\xC5" ", Enter" "\xBB\xF2" "0" "\xCE\xAA\xCD\xCB\xB3\xF6\xA3\xA9" ":", ans, 3, DOECHO, YEA);
							num = atoi(ans);
						}
						while (num < 0 || num >totalnum);

						if (num == 0)
							break;

						char dir[80 * 2];
						struct mmapfile mf = { .ptr = NULL };
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
			// 查询网友状态
			prints("\xB2\xE9\xD1\xAF\xCD\xF8\xD3\xD1\xD7\xB4\xCC\xAC");
			t_query(NULL);
			page = -1;
			break;
		case 'H':
			ansimore("0Announce/bbslist/good", NA);
			what_to_do();
			page = -1;
			break;
		case 'S':	/* sendmsg ... youzi */
			if (!HAS_PERM(PERM_PAGE, currentuser))
				break;
			s_msg(0, NULL, NULL);
			page = -1;
			break;
		case 'c':	/* show friends ... youzi */
			if (!HAS_PERM(PERM_BASIC, currentuser))
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

			bptr = ythtbbs_cache_Board_get_board_by_name(ptr->name);
			if (!bptr)
				break;
			if (!chk_currBM(&(bptr->header), 0) && !clubsync("deleterequest"))
				break;
			if (ptr->status == 'p') {
				if (!HAS_PERM(PERM_BLEVELS, currentuser))
					break;
			}
			page = -1;
			// 确定要%s%s版吗?
			sprintf(genbuf, "\xC8\xB7\xB6\xA8\xD2\xAA" "%s%s" "\xB0\xE6\xC2\xF0" "?",
				// 解封
				// 封
				(ptr->status == 'p') ? "\xBD\xE2\xB7\xE2" : "\xB7\xE2",
				ptr->name);
			if (askyn(genbuf, NA, YEA) == NA)
				break;

			{
				struct boardheader fh;
				int pos;
				if (!(pos = new_search_record(BOARDS, &fh, sizeof (fh), (void *) cmpbnames, ptr->name))) {
					// 错误的讨论区名称
					prints("\xB4\xED\xCE\xF3\xB5\xC4\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB\xB3\xC6");
					pressreturn();
					break;
				}
				if (fh.level & ~(PERM_BLEVELS | PERM_POSTMASK)) {
					// 包含不能直接封版的权限, 无法操作, 请用修改版面设置功能
					prints("\xB0\xFC\xBA\xAC\xB2\xBB\xC4\xDC\xD6\xB1\xBD\xD3\xB7\xE2\xB0\xE6\xB5\xC4\xC8\xA8\xCF\xDE" ", " "\xCE\xDE\xB7\xA8\xB2\xD9\xD7\xF7" ", " "\xC7\xEB\xD3\xC3\xD0\xDE\xB8\xC4\xB0\xE6\xC3\xE6\xC9\xE8\xD6\xC3\xB9\xA6\xC4\xDC");
					pressreturn();
					break;
				}
				if (ptr->status == 'p') {
					fh.level &= ~(PERM_POSTMASK | PERM_BLEVELS);
					ptr->status = ' ';
				} else {
					fh.level |= (PERM_POSTMASK | PERM_BLEVELS);
					ptr->status = 'p';
				}
				substitute_record(BOARDS, &fh, sizeof (fh), pos);
				ythtbbs_cache_Board_resolve();
				// %s讨论区: %s
				sprintf(genbuf, "%s" "\xCC\xD6\xC2\xDB\xC7\xF8" ": %s",
						// 封
						// 解封
						(ptr->status == 'p') ? "\xB7\xE2" : "\xBD\xE2\xB7\xE2",
						ptr->name);
				securityreport(genbuf, genbuf);
				// 已经%s了讨论区: %s
				prints("\xD2\xD1\xBE\xAD" "%s" "\xC1\xCB\xCC\xD6\xC2\xDB\xC7\xF8" ": %s",
						// 封
						// 解封
						(ptr->status == 'p') ? "\xB7\xE2" : "\xBD\xE2\xB7\xE2",
						ptr->name);
			}
			pressreturn();
			break;

		case Ctrl('A'):
			if (num >= secnum + brdnum || num < secnum)
				break;
			ptr = &nbrd[num - secnum];
			bptr = ythtbbs_cache_Board_get_board_by_name(ptr->name);
			if (!bptr)
				break;
			page = -1;
			if (bptr->header.level & PERM_POSTMASK)
				// 限制 %s 权利
				sprintf(property, "\xCF\xDE\xD6\xC6" " %s " "\xC8\xA8\xC0\xFB", "POST");
			else if (bptr->header.level & PERM_NOZAP)
				// 限制 %s 权利
				sprintf(property, "\xCF\xDE\xD6\xC6" " %s " "\xC8\xA8\xC0\xFB", "ZAP");
			else
				// 限制 %s 权利
				sprintf(property, "\xCF\xDE\xD6\xC6" " %s " "\xC8\xA8\xC0\xFB", "READ");
			if ((bptr->header.level & ~PERM_POSTMASK) == 0){
				property[0] = '\0';
				if (bptr->header.flag & ANONY_FLAG)
					// 匿名
					strcat(property, "\xC4\xE4\xC3\xFB");
				if (bptr->header.flag & CLUB_FLAG)
					// 俱乐部
					strcat(property, "\xBE\xE3\xC0\xD6\xB2\xBF");
				if (property[0] == '\0')
					// 普通
					strcpy(property, "\xC6\xD5\xCD\xA8");
			}
			strcpy(boardbuf, currboard);
			strcpy(currboard, bptr->header.filename);

			sprintf(genbuf, "boards/%s/boardrelation", bptr->header.filename);
			FILE *fp = fopen(genbuf, "r");
			char linebuf[128] = "";
			if (fp != NULL) {
				fgets(linebuf, 128, fp);
				fclose(fp);
			}

			clear();
			move(1, 0);
			// 英文版名      \033[1;32m%-16s\033[m中文版名      \033[1;32m%s\033[m\n
			prints("\xD3\xA2\xCE\xC4\xB0\xE6\xC3\xFB" "      \033[1;32m%-16s\033[m" "\xD6\xD0\xCE\xC4\xB0\xE6\xC3\xFB" "      \033[1;32m%s\033[m\n"
				// 主 分 区      \033[1;32m%-16s\033[m映射分区      \033[1;32m%s\033[m\n
				"\xD6\xF7" " " "\xB7\xD6" " " "\xC7\xF8" "      \033[1;32m%-16s\033[m" "\xD3\xB3\xC9\xE4\xB7\xD6\xC7\xF8" "      \033[1;32m%s\033[m\n"
				// 版面类别      \033[1;32m%-16s\033[m版面属性      \033[1;32m%s\033[m\n
				"\xB0\xE6\xC3\xE6\xC0\xE0\xB1\xF0" "      \033[1;32m%-16s\033[m" "\xB0\xE6\xC3\xE6\xCA\xF4\xD0\xD4" "      \033[1;32m%s\033[m\n"
				// 记文章数      \033[1;32m%-16s\033[m转    信      \033[1;32m%s\033[m\n\n
				"\xBC\xC7\xCE\xC4\xD5\xC2\xCA\xFD" "      \033[1;32m%-16s\033[m" "\xD7\xAA" "    " "\xD0\xC5" "      \033[1;32m%s\033[m\n\n"
				// 版面关键字    \033[1;32m%s\033[m\n
				"\xB0\xE6\xC3\xE6\xB9\xD8\xBC\xFC\xD7\xD6" "    \033[1;32m%s\033[m\n"
				// 相关版面      \033[1;32m%s\033[m\n
				"\xCF\xE0\xB9\xD8\xB0\xE6\xC3\xE6" "      \033[1;32m%s\033[m\n"
				// 版面简介
				"\xB0\xE6\xC3\xE6\xBC\xF2\xBD\xE9" "      ",
				bptr->header.filename, bptr->header.title,
				// \033[37m(无)
				bptr->header.sec1, bptr->header.sec2[0] ? bptr->header.sec2 : "\033[37m(" "\xCE\xDE" ")",
				bptr->header.type,  property,
				// 不
				// 是
				junkboard() ? "\xB2\xBB" : "\xCA\xC7",
				// 是
				// 不
				(bptr->header.flag & INNBBSD_FLAG) ? "\xCA\xC7" : "\xB2\xBB",
				// \033[37m(暂无)
				bptr->header.keyword[0] ? bptr->header.keyword : "\033[37m(" "\xD4\xDD\xCE\xDE" ")",
				// \033[37m(暂无)
				linebuf[0] ? linebuf: "\033[37m(" "\xD4\xDD\xCE\xDE" ")");
			setbfile(property, sizeof(property), bptr->header.filename, "introduction");
			if (file_exist(property))
				ansimore2(property, NA, 9, 14);
			else
				// (暂无)
				prints("\033[1;37m%-16s\033[m\n", "(" "\xD4\xDD\xCE\xDE" ")");
			pressanykey();
			strcpy(currboard, boardbuf);
			break;

		case 'a':
			if (num >= brdnum + secnum || num < secnum)
				break;
			if (GoodBrd.num) {
				if (GoodBrd.num >= GOOD_BRD_NUM) {
					move(t_lines - 1, 0);
					// 个人热门版数已经达上限(%d)
					prints("\xB8\xF6\xC8\xCB\xC8\xC8\xC3\xC5\xB0\xE6\xCA\xFD\xD2\xD1\xBE\xAD\xB4\xEF\xC9\xCF\xCF\xDE" "(%d)", GOOD_BRD_NUM);
					//pressreturn();
				} else {
					char bname[STRLEN], bpath[STRLEN];
					struct stat st;
					move(0, 0);
					clrtoeol();
					// 选择讨论区 [ \033[1;32m# \033[0;37m- \033[1;31m版面名称/关键字搜索\033[0;37m, \033[1;32mSPACE \033[0;37m- 自动补全, \033[1;32mENTER \033[0;37m- 退出 ] \033[m\n
					prints("\xD1\xA1\xD4\xF1\xCC\xD6\xC2\xDB\xC7\xF8" " [ \033[1;32m# \033[0;37m- \033[1;31m" "\xB0\xE6\xC3\xE6\xC3\xFB\xB3\xC6" "/" "\xB9\xD8\xBC\xFC\xD7\xD6\xCB\xD1\xCB\xF7" "\033[0;37m, \033[1;32mSPACE \033[0;37m- " "\xD7\xD4\xB6\xAF\xB2\xB9\xC8\xAB" ", \033[1;32mENTER \033[0;37m- " "\xCD\xCB\xB3\xF6" " ] \033[m\n");
					// 输入讨论区名 (英文字母大小写皆可):
					prints("\xCA\xE4\xC8\xEB\xCC\xD6\xC2\xDB\xC7\xF8\xC3\xFB" " (" "\xD3\xA2\xCE\xC4\xD7\xD6\xC4\xB8\xB4\xF3\xD0\xA1\xD0\xB4\xBD\xD4\xBF\xC9" "): ");
					clrtoeol();
					make_blist();
					if((namecomplete((char *) NULL, bname))=='#')
						super_select_board(bname);
					setbpath(bpath, sizeof(bpath), bname);
					//if (*bname == '\0');
					if (stat(bpath, &st) == -1) {
						move(2, 0);
						// 不正确的讨论区.\n
						prints("\xB2\xBB\xD5\xFD\xC8\xB7\xB5\xC4\xCC\xD6\xC2\xDB\xC7\xF8" ".\n");
						pressreturn();
					} else {
						if (!ythtbbs_mybrd_exists(&GoodBrd, bname)) {
							ythtbbs_mybrd_append(&GoodBrd, bname);
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
				if (GoodBrd.num >= GOOD_BRD_NUM) {
					move(t_lines - 1, 0);
					clrtoeol();
					// 个人热门版数已经达上限(%d)
					prints("\xB8\xF6\xC8\xCB\xC8\xC8\xC3\xC5\xB0\xE6\xCA\xFD\xD2\xD1\xBE\xAD\xB4\xEF\xC9\xCF\xCF\xDE" "(%d)", GOOD_BRD_NUM);
					GoodBrd.num = 0;
					//pressreturn();
				} else {
					if (!ythtbbs_mybrd_exists(&GoodBrd, ptr->name)) {
						ythtbbs_mybrd_append(&GoodBrd, ptr->name);
						save_GoodBrd();
						GoodBrd.num = 0;
						move(t_lines - 1, 0);
						clrtoeol();
						// 版面%s已被加入收藏夹
						prints("\xB0\xE6\xC3\xE6" "%s" "\xD2\xD1\xB1\xBB\xBC\xD3\xC8\xEB\xCA\xD5\xB2\xD8\xBC\xD0", ptr->name);
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
				char ans[5];
				// 要把 %s 从收藏夹中去掉 (Y/N)? [N]
				sprintf(genbuf, "\xD2\xAA\xB0\xD1" " %s " "\xB4\xD3\xCA\xD5\xB2\xD8\xBC\xD0\xD6\xD0\xC8\xA5\xB5\xF4" " (Y/N)? [N]",
					nbrd[num - secnum].name);
				getdata(t_lines - 1, 0, genbuf, ans, 2, DOECHO,
					YEA);
				if (ans[0] != 'y' && ans[0] != 'Y') {
					page = -1;
					break;
				}
				ythtbbs_mybrd_remove(&GoodBrd, nbrd[num - secnum].name);
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
		sethomefile_s(dirfile, sizeof(dirfile), currentuser.userid, "brc");
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

static void brc_update() {
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
	const struct boardmem *board;
	if (!t) {
		t = time(NULL);
		bnum = getbnum(brc.board);
		board = ythtbbs_cache_Board_get_board_by_idx(bnum - 1);
		if (bnum && board->lastpost > t)
			t = board->lastpost;
	}
	brc_clearto(&brc, t);
}

int clear_all_new_flag(const char *s) {
	(void) s;
	int i;
	char ans[3];
	int brdnum;
	boardprefix[0] = 0xFF;
	boardprefix[1] = 0;
	GoodBrd.num = 0;
	brdnum = -1;
	// 确定要清除所有版面的未读标记？(Y/N) [N]:
	getdata(t_lines - 2, 0, "\xC8\xB7\xB6\xA8\xD2\xAA\xC7\xE5\xB3\xFD\xCB\xF9\xD3\xD0\xB0\xE6\xC3\xE6\xB5\xC4\xCE\xB4\xB6\xC1\xB1\xEA\xBC\xC7\xA3\xBF" "(Y/N) [N]:", ans,
		2, DOECHO, YEA);
	if (ans[0] != 'y' && ans[0] != 'Y')
		return 0;
	if (load_boards(&brdnum, 0) < 0)
		return 0;
	for (i = 0; i < brdnum; i++) {
		brc_initial(nbrd[i].name, (i == brdnum - 1) ? 0 : 1);
		clear_new_flag_quick(0);
	}
	brc_update();
	return 0;
}

int Read(const char *s) {
	(void) s;
	char buf[STRLEN];
	char notename[STRLEN];
	time_t usetime;
	struct stat st;
	struct boardmem *board;
	if (!selboard || !strcmp(currboard, "")) {
		move(2, 0);
		// 请先选择讨论区\n
		prints("\xC7\xEB\xCF\xC8\xD1\xA1\xD4\xF1\xCC\xD6\xC2\xDB\xC7\xF8" "\n");
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

	board = ythtbbs_cache_Board_get_board_by_idx(uinfo.curboard - 1);
	if (uinfo.curboard && board && board->inboard > 0)
		board->inboard--;
	uinfo.curboard = getbnum(currboard);
	update_utmp();
	board = ythtbbs_cache_Board_get_board_by_idx(uinfo.curboard - 1);
	board->inboard++;
	setvfile(notename, currboard, "notes");
	clear();
	if (stat(notename, &st) != -1) {
		if (st.st_mtime > brc.notetime || now_t - brc.notetime > 7 * 86400) {
			show_board_notes(currboard);
			brc.notetime = now_t;
			brc.changed = 1;
			if (!DEFINE(DEF_INTOANN, currentuser) || brc.num > 1)
				pressanykey();
		}
	}

	if (DEFINE(DEF_INTOANN, currentuser) && brc_unreadt(&brc, 2)) {
		char ans[3];
		getdata(t_lines - 1, 0,
			// \033[0m\033[1m您初次访问本版, 是否首先察看精华区?
			"\033[0m\033[1m" "\xC4\xFA\xB3\xF5\xB4\xCE\xB7\xC3\xCE\xCA\xB1\xBE\xB0\xE6" ", " "\xCA\xC7\xB7\xF1\xCA\xD7\xCF\xC8\xB2\xEC\xBF\xB4\xBE\xAB\xBB\xAA\xC7\xF8" "?"
			//  (选A不再做此提示)(Y/N/A) [Y]:
			" (" "\xD1\xA1" "A" "\xB2\xBB\xD4\xD9\xD7\xF6\xB4\xCB\xCC\xE1\xCA\xBE" ")(Y/N/A) [Y]:", ans, 3, DOECHO, YEA);
		brc_addlistt(&brc, 2);
		if (ans[0] == 'A' || ans[0] == 'a') {
			set_safe_record();
			currentuser.userdefine &= ~DEF_INTOANN;
			substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
		} else if (ans[0] != 'N' && ans[0] != 'n') {
			into_announce(0, NULL, NULL);
			show_board_notes(currboard);
			move(t_lines - 1, 0);
			// \033[0m\033[1m欢迎光临, 按任意键进入本版版面, 在版面按'x'可以随时进入精华区
			prints("\033[0m\033[1m" "\xBB\xB6\xD3\xAD\xB9\xE2\xC1\xD9" ", " "\xB0\xB4\xC8\xCE\xD2\xE2\xBC\xFC\xBD\xF8\xC8\xEB\xB1\xBE\xB0\xE6\xB0\xE6\xC3\xE6" ", " "\xD4\xDA\xB0\xE6\xC3\xE6\xB0\xB4" "'x'" "\xBF\xC9\xD2\xD4\xCB\xE6\xCA\xB1\xBD\xF8\xC8\xEB\xBE\xAB\xBB\xAA\xC7\xF8");
			egetch();
		}
	}
	usetime = time(NULL);
	ISdelrq = clubsync("deleterequest");
	i_read(READING, buf, readtitle, (void *) readdoent, read_comms, sizeof (struct fileheader));
	now_t = time(NULL);
	if (now_t - usetime > 1) {
		snprintf(genbuf, 256, "%s use %s %ld",
			currentuser.userid, currboard,
			(long int) (now_t - usetime));
		newtrace(genbuf);
	}
	brc_update();

	board = ythtbbs_cache_Board_get_board_by_idx(uinfo.curboard - 1);
	if (uinfo.curboard && board->inboard > 0)
		board->inboard--;
	uinfo.curboard = 0;
	update_utmp();
	return 0;
}

int showhell(const char *s) {
	(void) s;
	selboard = 1;
	strcpy(currboard, "hell");
	return Read(NULL);
}

int showprison(const char *s) {
	(void) s;
	selboard = 1;
	strcpy(currboard, "prison");
	return Read(NULL);
}

static int
readtitle()
{
	struct boardmem *bp;

	char header[200], title[STRLEN];
	char readmode[10];
	int active, invisible, i;
	char tmp[40];
	bp = ythtbbs_cache_Board_get_board_by_name(currboard);
	if (bp == NULL)
		return -1;
	IScurrBM = chk_currBM(&(bp->header), 0);
	//ISdelrq = clubsync("deleterequest");

	if (bp->header.bm[0][0] == 0) {
		// 诚征版主中
		strcpy(header, "\xB3\xCF\xD5\xF7\xB0\xE6\xD6\xF7\xD6\xD0");
	} else {
		// 版主:
		strcpy(header, "\xB0\xE6\xD6\xF7" ": ");
		for (i = 0; i < 4; i++) {	//只显示前四个大班长
			if (bp->header.bm[i][0] == 0)
				break;
			active = bp->bmonline & (1 << i);
			invisible = bp->bmcloak & (1 << i);
			if (active && !invisible)
				sprintf(tmp, "\x1b[32m%s\x1b[33m ", bp->header.bm[i]);
			else if (active && invisible && (HAS_PERM(PERM_SEECLOAK, currentuser) || !strcmp(bp->header.bm[i], currentuser.userid)))
				sprintf(tmp, "\x1b[36m%s\x1b[33m ", bp->header.bm[i]);
			else
				sprintf(tmp, "%s ", bp->header.bm[i]);
			strcat(header, tmp);
		}
	}
	if (chkmail())
		// [您有信件,请按 w 查看信件]
		strcpy(title, "[" "\xC4\xFA\xD3\xD0\xD0\xC5\xBC\xFE" "," "\xC7\xEB\xB0\xB4" " w " "\xB2\xE9\xBF\xB4\xD0\xC5\xBC\xFE" "]");
	else if ((bp->header.flag & VOTE_FLAG))
		// ※ 投票中，按 v 进入投票 ※
		sprintf(title, "\xA1\xF9" " " "\xCD\xB6\xC6\xB1\xD6\xD0\xA3\xAC\xB0\xB4" " v " "\xBD\xF8\xC8\xEB\xCD\xB6\xC6\xB1" " " "\xA1\xF9");
	else
		strcpy(title, bp->header.title);

	showtitle(header, title);
	// 离开[\x1b[1;32m←\x1b[m,\x1b[1;32mq\x1b[m] 选择[\x1b[1;32m↑\x1b[m,\x1b[1;32m↓\x1b[m] 阅读[\x1b[1;32m→\x1b[m,\x1b[1;32mRtn\x1b[m] 发表文章[\x1b[1;32mCtrl-P\x1b[m] 砍信[\x1b[1;32md\x1b[m] 备忘录[\x1b[1;32mTAB\x1b[m] 求助[\x1b[1;32mh\x1b[m]\n
	prints("\xC0\xEB\xBF\xAA" "[\x1b[1;32m" "\xA1\xFB" "\x1b[m,\x1b[1;32mq\x1b[m] " "\xD1\xA1\xD4\xF1" "[\x1b[1;32m" "\xA1\xFC" "\x1b[m,\x1b[1;32m" "\xA1\xFD" "\x1b[m] " "\xD4\xC4\xB6\xC1" "[\x1b[1;32m" "\xA1\xFA" "\x1b[m,\x1b[1;32mRtn\x1b[m] " "\xB7\xA2\xB1\xED\xCE\xC4\xD5\xC2" "[\x1b[1;32mCtrl-P\x1b[m] " "\xBF\xB3\xD0\xC5" "[\x1b[1;32md\x1b[m] " "\xB1\xB8\xCD\xFC\xC2\xBC" "[\x1b[1;32mTAB\x1b[m] " "\xC7\xF3\xD6\xFA" "[\x1b[1;32mh\x1b[m]\n");
	switch (digestmode) {
	case 0:
		if (DEFINE(DEF_THESIS, currentuser))	/* youzi 1997.7.8 */
			// 主题
			strcpy(readmode, "\xD6\xF7\xCC\xE2");
		else
			// 一般
			strcpy(readmode, "\xD2\xBB\xB0\xE3");
		break;
	case 1:
		// 文摘
		strcpy(readmode, "\xCE\xC4\xD5\xAA");
		break;
	case 2:
		// 主题
		strcpy(readmode, "\xD6\xF7\xCC\xE2");
		break;
	case 3:
		// 防水
		strcpy(readmode, "\xB7\xC0\xCB\xAE");
		break;
	case 4:
		// 回收
		strcpy(readmode, "\xBB\xD8\xCA\xD5");
		break;
	case 5:
		// 纸篓
		strcpy(readmode, "\xD6\xBD\xC2\xA8");
		break;
	}
	if (DEFINE(DEF_THESIS, currentuser) && digestmode == 0)
		// \x1b[1;37;44m 编号   %-12s %6s %-28s在线:%4d [%4s式看版] \x1b[m\n
		prints("\x1b[1;37;44m " "\xB1\xE0\xBA\xC5" "   %-12s %6s %-28s" "\xD4\xDA\xCF\xDF" ":%4d [%4s" "\xCA\xBD\xBF\xB4\xB0\xE6" "] \x1b[m\n",
				// 刊 登 者
				// 日  期
				//  标  题
				"\xBF\xAF" " " "\xB5\xC7" " " "\xD5\xDF", "\xC8\xD5" "  " "\xC6\xDA", " " "\xB1\xEA" "  " "\xCC\xE2", bp->inboard, readmode);
	else
		// \x1b[1;37;44m 编号   %-12s %6s %-30s在线:%4d [%4s模式] \x1b[m\n
		prints("\x1b[1;37;44m " "\xB1\xE0\xBA\xC5" "   %-12s %6s %-30s" "\xD4\xDA\xCF\xDF" ":%4d [%4s" "\xC4\xA3\xCA\xBD" "] \x1b[m\n",
				// 刊 登 者
				// 日  期
				//  标  题
				"\xBF\xAF" " " "\xB5\xC7" " " "\xD5\xDF", "\xC8\xD5" "  " "\xC6\xDA", " " "\xB1\xEA" "  " "\xCC\xE2", bp->inboard, readmode);
	clrtobot();
	return 0;
}

static char *readdoent(int num, struct fileheader *ent, char buf[512])
{
	char date[80], owner[13];
	char *TITLE;
	char type, *type1, type2, danger = 0;
	char typestring[32];
	int noreply = 0, attached = 0, allcanre = 0;

	{
		char *ptr;
		ytht_strsncpy(owner, ent->owner, 13);
		if (!owner[0])
			strcpy(owner, "Anonymous");
		if ((ptr = strchr(owner, '.')) != NULL)
			*(ptr + 1) = 0;
	}

	type = (UNREAD(ent, &brc) ? '*' : ' ');
	//add by hace 2003.05.02
//comment by bjgyt	if(ent->accessed & FILE_TOP1){
	if(IScurrBM && (ent->accessed & FILE_TOP1)){
		type='#';
	}
	//end

	if ((ent->accessed & FH_DIGEST)) {
		if (type == ' ')
			type = 'g';
		else
			type = 'G';
	}
	if ((ent->accessed & FH_DANGEROUS) && (IScurrBM || ISdelrq))
		danger = 1;
	if ((ent->accessed & FH_DEL) && (IScurrBM || HAS_PERM(PERM_ARBITRATE, currentuser))) {
		if (danger)
			type1 = "\033[1;31mX\033[0m";
		else
			type1 = "X";
	}
	else if((ent->accessed & FH_MINUSDEL) && (IScurrBM || HAS_PERM(PERM_ARBITRATE, currentuser))){	//add by mintbaggio 040322 for minus-numposts delete
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
	danger = (ent->accessed & FH_DANGEROUS) ? ((IScurrBM || ISdelrq) ? 1 : 0) : 0;
	noreply = ent->accessed & FH_NOREPLY;
	attached = ent->accessed & FH_ATTACHED;
	allcanre = ent->accessed & FH_ALLREPLY;
	if((HAS_PERM(PERM_SYSOP|PERM_OBOARDS, currentuser)||has_perm_commend(currentuser.userid)) && is_in_commend(currboard, ent)){	//add by mintbaggio 040327 for front page commend
		if(type == '*')
			type1 = "\033[1;45mM\033[0m";
		else
			type1 = "\033[1;45mm\033[0m";
	}
	if((HAS_PERM(PERM_SYSOP|PERM_OBOARDS, currentuser)||has_perm_commend(currentuser.userid)) && is_in_commend2(currboard, ent)){	//add by mintbaggio 040327 for front page commend
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

	if(IScurrBM) {
		if((ent->accessed & FH_ANNOUNCE) && (ent->accessed & FH_ISWATER)) {
			sprintf(typestring, "%s\x1b[1;4;32;42m%c\x1b[m%c", type1, type, type2);
		} else if(ent->accessed & FH_ANNOUNCE) {
			sprintf(typestring, "%s\x1b[1;32;42m%c\x1b[m%c", type1, type, type2);
		} else if(ent->accessed & FH_ISWATER) {
			sprintf(typestring, "%s\x1b[4m%c\x1b[m%c", type1, type, type2);
		} else
			sprintf(typestring, "%s%c%c", type1, type, type2);
	} else
		sprintf(typestring, "%s%c%c", type1, type, type2);

	makedatestar(date, ent);
	/*  Re-Write By Excellent */

	TITLE = ent->title;
	filter(TITLE);
	if (uinfo.mode != RMAIL && digestmode != 1 && digestmode != 4 && digestmode != 5) {	//用新方法
		if (ent->thread != ent->filetime && !strncmp(TITLE, "Re: ", 4)) {	//Re文
			if (readingthread == ent->thread)	//当前正在读的主题
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
					//  \033[1;33m%4d\033[m%s%-12.12s%s\033[1;33m.%c%s●\033[0;1;33m %-.45s\033[m
					" \033[1;33m%4d\033[m%s%-12.12s%s\033[1;33m.%c%s" "\xA1\xF1" "\033[0;1;33m %-.45s\033[m",
					num, typestring, owner, date,
					attached ? '@' : ' ',
					noreply ? "\033[0;1;4;33m" : "", TITLE);
			else
				sprintf(buf,
					" %4d%s%-12.12s%s %c%s%s %-.45s\033[m",
					num, typestring, owner, date,
					attached ? '@' : ' ',
					noreply ? "\033[0;1;4;33m" : "",
					// \033[1;31m●
					// ●\033[m
					allcanre ? "\033[1;31m" "\xA1\xF1" : "\xA1\xF1" "\033[m",
					TITLE);
		}
	} else {
		if (!strncmp("Re:", ent->title, 3) || !strncmp("RE:", ent->title, 3)) {
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
					//  \x1b[1;33m%4d\x1b[m%s%-12.12s%s\x1b[1;33m.%c%s●\033[0;1;33m %-.45s\033[m
					" \x1b[1;33m%4d\x1b[m%s%-12.12s%s\x1b[1;33m.%c%s" "\xA1\xF1" "\033[0;1;33m %-.45s\033[m",
					num, typestring, owner, date,
					attached ? '@' : ' ',
					noreply ? "\033[0;1;4;33m" : "", TITLE);
			} else {
				sprintf(buf,
					" %4d%s%-12.12s%s %c%s%s %-.45s\033[m",
					num, typestring, owner, date,
					attached ? '@' : ' ',
					noreply ? "\033[0;1;4;33m" : "",
					// \033[1;31m●
					// ●\033[m
					allcanre ? "\033[1;31m" "\xA1\xF1" : "\xA1\xF1" "\033[m",
					TITLE);
			}
		}
	}
	// add by hace 2003.05.05
	char path[64];
	struct stat st;
	char msg[32];
	setbdir(path, currboard, digestmode);
	if (stat(path, &st) == -1)
		errlog("error");
	if ((stat(path, &st) != -1) && (num > 0 && (st.st_size / sizeof(struct fileheader)) < (unsigned) num)) {
		ent->accessed |= FILE_TOP1;
		//errlog("slowaction");
		// \033[1;31m[提示]\033[0m
		if((ent->accessed& FH_MARKED)&&(ent->accessed&FH_DIGEST)) strcpy(msg,"\033[1;31m[" "\xCC\xE1\xCA\xBE" "]\033[0m");
		// \033[1;33m[推荐]\033[0m
		else if(ent->accessed & FH_DIGEST) strcpy(msg,"\033[1;33m[" "\xCD\xC6\xBC\xF6" "]\033[0m");
		// \033[1;36m[提示]\033[0m
		else if(ent->accessed & FH_MARKED) strcpy(msg,"\033[1;36m[" "\xCC\xE1\xCA\xBE" "]\033[0m");
		// \033[1;32m[提示]\033[0m
		else strcpy(msg,"\033[1;32m[" "\xCC\xE1\xCA\xBE" "]\033[0m");
		msg[31]=0;
		//  %6s %-12.12s%s\033[m %s◆ %-.45s
		sprintf(buf, " %6s %-12.12s%s\033[m %s" "\xA1\xF4" " %-.45s ",msg,ent->owner, date," ", TITLE);
	}
	//end
	return buf;
}

static char *
makedatestar(char *datestr, struct fileheader *ent)
{
	char str[30] = " ", buf[30];
	char backcolor[7];
	int i, j, better, sz;
	char fg, bg;
	time_t filetime;

	ytht_strsncpy(backcolor, "062351", sizeof backcolor);
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
		strcpy(str + 1, "      ");
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
