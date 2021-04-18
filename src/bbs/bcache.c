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
#include "smth_screen.h"
#include "talk.h"
#include "record.h"
#include "list.h"
#include "bcache.h"
#include "bbsinc.h"
#include "io.h"
#include "bbs_global_vars.h"
//CHINPUT_SHMKEY=5102

struct userec lookupuser;

//struct PINYINARRAY *pa;
int usernumber;
int numboards = -1;
extern int die;

static int setbmhat(struct boardmanager *bm, int *online);

void
attach_err(shmkey, name)
int shmkey;
char *name;
{
	sprintf(genbuf, "Error! %s error! key = %x.\n", name, shmkey);
	prints("%s", genbuf);
	refresh();
	exit(1);
}

void *
attach_shm(shmkey, shmsize)
int shmkey, shmsize;
{
	void *shmptr;

	shmptr = get_shm(shmkey, shmsize);
	if (shmptr == NULL)
		attach_err(shmkey, "shmat");
	return shmptr;
}

int
hasreadperm(struct boardheader *bh)
{
	if (bh->clubnum != 0)
		return ((HAS_CLUBRIGHT(bh->clubnum, uinfo.clubrights))
			|| (bh->flag & CLUBTYPE_FLAG) || HAS_PERM(PERM_SYSOP, currentuser));
	if (bh->level & PERM_SPECIAL3)
		return die;
	return (bh->level & PERM_POSTMASK) || HAS_PERM(bh->level, currentuser) || (bh->level & PERM_NOZAP);
}

int hasreadperm_ext(char *username,  char *boardname)
{
	struct boardmem *x = ythtbbs_cache_Board_get_board_by_name(boardname);
	if(x==0)
		return 0;

	int tuid = getuser(username);
	if(tuid == 0)	//用户不存在
		return 0;

	const struct user_info *ui = ythtbbs_cache_UserTable_query_user_by_uid(currentuser.userid, HAS_PERM(PERM_SYSOP | PERM_SEECLOAK, currentuser), tuid, false); // 调用 ui 前先检查 ui 是否不在线（主要为俱乐部、封闭版面）

	if(x->header.flag != 0) {
		if((x->header.flag & CLUBTYPE_FLAG))
			return 1;
		if(strcasecmp(username, "guest")==0)
			return 0;
		return (ui == 0) ? 0 : (ui->clubrights[x->header.clubnum / 32] & (1<<((x->header.clubnum) % 32)));
	}

	if(x->header.level == 0)
		return 1;
	if(x->header.level & (PERM_POSTMASK | PERM_NOZAP))
		return 1;
	if(!(lookupuser.userlevel & PERM_BASIC))
		return 0;
	if(lookupuser.userlevel & x->header.level)
		return 1;

	return 0;
}

static int getbnum_callback(struct boardmem *board, int curr_idx, va_list ap) {
	const char *bname = va_arg(ap, const char *);
	int *cache_idx = va_arg(ap, int *);

	if (strncasecmp(bname, board->header.filename, STRLEN))
		return 0;

	if (hasreadperm(&board->header)) {
		*cache_idx = curr_idx;
		return QUIT;
	}

	return 0;
}

int getbnum(const char *bname) {
	static int cachei = 0;

	ythtbbs_cache_Board_resolve();

	if (!strncasecmp(bname, ythtbbs_cache_Board_get_board_by_idx(cachei)->header.filename, STRLEN))
		if (hasreadperm(&ythtbbs_cache_Board_get_board_by_idx(cachei)->header))
			return cachei + 1;

	ythtbbs_cache_Board_foreach_v(getbnum_callback, bname, &cachei);

	if (!strncasecmp(bname, ythtbbs_cache_Board_get_board_by_idx(cachei)->header.filename, STRLEN))
		if (hasreadperm(&ythtbbs_cache_Board_get_board_by_idx(cachei)->header))
			return cachei + 1;

	return 0;
}

int canberead(const char *bname) {
	int i;
	if ((i = getbnum(bname)) == 0)
		return 0;
	return hasreadperm(&ythtbbs_cache_Board_get_board_by_idx(i - 1)->header);
}

int noadm4political(const char *bname) {
	time_t now = time(NULL);
	time_t t = ythtbbs_cache_utmp_get_watchman();
	return (!t || now < t) ? 0 : ythtbbs_board_is_political(bname);
}

int
haspostperm(bname)
char *bname;
{
	register int i;
	const struct boardmem *board_ptr = NULL;

	if (digestmode)
		return 0;
	if (strcmp(bname, DEFAULTBOARD) == 0)
		return 1;
	if (strcmp(bname, "Freshman")==0)
		return 1;
	if (!HAS_PERM(PERM_POST, currentuser))
		return 0;
	if ((i = getbnum(bname)) == 0)
		return 0;

	board_ptr = ythtbbs_cache_Board_get_board_by_idx(i - 1);
	if (board_ptr->header.clubnum != 0)
		return (HAS_PERM((board_ptr->header.level & ~PERM_NOZAP) & ~PERM_POSTMASK, currentuser)) &&
			(HAS_CLUBRIGHT(board_ptr->header.clubnum, uinfo.clubrights));
	if (board_ptr->header.level & PERM_SPECIAL3)
		return die;
	return (HAS_PERM((board_ptr->header.level & ~PERM_NOZAP) & ~PERM_POSTMASK, currentuser));
}

int
posttest(uid, bname)
char *uid;
char *bname;
{
	register int i;
	const struct boardmem *board_ptr = NULL;

	i = getuser(uid);
	if (i == 0)
		return 0;
	if (strcmp(bname, DEFAULTBOARD) == 0)
		return 1;
	if ((i = getbnum(bname)) == 0)
		return 0;

	board_ptr = ythtbbs_cache_Board_get_board_by_idx(i - 1);
	if (board_ptr->header.clubnum != 0) {
		setbfile(genbuf, sizeof(genbuf), currboard, "club_users");
		return seek_in_file(genbuf, uid);
	}
	if (board_ptr->header.level == 0)
		return 1;
	return (lookupuser.userlevel & ((board_ptr->header.level & ~PERM_NOZAP) & ~PERM_POSTMASK));
}

int
normal_board(bname)
char *bname;
{
	register int i;
	const struct boardmem *board_ptr = NULL;

	if (strcmp(bname, DEFAULTBOARD) == 0)
		return 1;
	if ((i = getbnum(bname)) == 0)
		return 0;

	board_ptr = ythtbbs_cache_Board_get_board_by_idx(i - 1);
	if (board_ptr->header.clubnum != 0)
		return (board_ptr->header.flag & CLUBTYPE_FLAG);
	if (board_ptr->header.level & PERM_NOZAP)
		return 1;
	return (board_ptr->header.level == 0);
}

int
innd_board(bname)
char *bname;
{
	register int i;
	const struct boardmem *board_ptr = NULL;

	if ((i = getbnum(bname)) == 0)
		return 0;

	board_ptr = ythtbbs_cache_Board_get_board_by_idx(i - 1);
	return (board_ptr->header.flag & INNBBSD_FLAG);
}

int
is1984_board(bname)
char *bname;
{
	register int i;
	const struct boardmem *board_ptr = NULL;

	if ((i = getbnum(bname)) == 0)
		return 0;

	board_ptr = ythtbbs_cache_Board_get_board_by_idx(i - 1);
	return (board_ptr->header.flag & IS1984_FLAG);
}

int
club_board(bname)
char *bname;
{
	register int i;
	if ((i = getbnum(bname)) == 0)
		return 0;
	return (ythtbbs_cache_Board_get_board_by_idx(i - 1)->header.clubnum != 0) ? 1 : 0;
}

int
clubsync(boardname)
char *boardname;
{
	FILE *fp;
	int i, old_right;
	static int club_rights_time = 0;
	char fn[STRLEN];
	struct stat st1, st2;
	const struct boardmem *board_ptr = NULL;

	if ((i = getbnum(boardname)) == 0)
		return 0;
	board_ptr = ythtbbs_cache_Board_get_board_by_idx(i - 1);
	if (board_ptr->header.clubnum == 0)
		return 1;

	sethomefile_s(fn, sizeof(fn), currentuser.userid, "clubrights");
	if (stat(fn, &st1))
		memset(&(uinfo.clubrights), 0, 4 * sizeof (int));
	else if (club_rights_time < st1.st_mtime) {
		if ((fp = fopen(fn, "r")) != NULL) {
			memset(&(uinfo.clubrights), 0, 4 * sizeof (int));
			while (fgets(genbuf, STRLEN, fp) != NULL) {
				old_right = atoi(genbuf);
				uinfo.clubrights[old_right / 32] |= (1 << old_right % 32);
			}
			club_rights_time = st1.st_mtime;
			fclose(fp);
		} else
			memset(&(uinfo.clubrights), 0, 4 * sizeof (int));
	}
	old_right = HAS_CLUBRIGHT(board_ptr->header.clubnum, uinfo.clubrights);
	setbfile(fn, sizeof(fn), boardname, "club_users");
	if (!stat(fn, &st2))
		if (club_rights_time < st2.st_mtime) {
			if (seek_in_file(fn, currentuser.userid))
				uinfo.clubrights[board_ptr->header.clubnum / 32] |= (1 << board_ptr->header.clubnum % 32);
			else
				uinfo.clubrights[board_ptr->header.clubnum / 32] &= ~(1 << board_ptr->header.clubnum % 32);
			if (old_right != HAS_CLUBRIGHT(board_ptr->header.clubnum, uinfo.clubrights)) {
				char towrite[STRLEN];
				sethomefile_s(fn, sizeof(fn), currentuser.userid, "clubrights");
				sprintf(towrite, "%d", board_ptr->header.clubnum);
				old_right ? ytht_del_from_file(fn, towrite, true) : ytht_add_to_file(fn, towrite);
			}
		}
	if (!(board_ptr->header.flag & CLUBTYPE_FLAG))
		return HAS_CLUBRIGHT(board_ptr->header.clubnum, uinfo.clubrights);
	return 1;
}

int getuser(const char *userid) {
	int uid = ythtbbs_cache_UserTable_search_usernum(userid);

	if (uid == 0)
		return 0;
	get_record(PASSFILE, &lookupuser, sizeof (lookupuser), uid);
	return uid;
}

int search_ulist(struct user_info *uentp, int (*fptr) (int, struct user_info *), int farg) {
	int i;
	const struct user_info *ptr_user_info;

	ythtbbs_cache_utmp_resolve();
	for (i = 0; i < USHM_SIZE; i++) {
		ptr_user_info = ythtbbs_cache_utmp_get_by_idx(i);
		if (ptr_user_info->active == 0)
			continue;
		*uentp = *ptr_user_info;
		if ((*fptr) (farg, uentp)) {
			return i + 1;
		}
	}
	return 0;
}

int search_ulistn(struct user_info *uentp, int (*fptr) (int, struct user_info *), int farg, int unum) {
	int i, j;
	j = 1;
	ythtbbs_cache_utmp_resolve();
	for (i = 0; i < USHM_SIZE; i++) {
		memcpy(uentp, ythtbbs_cache_utmp_get_by_idx(i), sizeof(struct user_info));
		if ((*fptr) (farg, uentp)) {
			if (j == unum)
				return i + 1;
			else
				j++;
		}

	}
	return 0;
}

/*Function Add by SmallPig*/
int count_logins(struct user_info *uentp, int (*fptr) (int, struct user_info *), int farg, int show) {
	int i, j;

	j = 0;
	ythtbbs_cache_utmp_resolve();
	for (i = 0; i < USHM_SIZE; i++) {
		memcpy(uentp, ythtbbs_cache_utmp_get_by_idx(i), sizeof(struct user_info));
		if ((*fptr) (farg, uentp)) {
			if (show == 0)
				j++;
			else {
				j++;
				prints("(%d) 目前在干嘛: %s, 来自: %s \n", j, ModeType(uentp->mode), uentp->from);
			}
		}
	}
	return j;
}

int t_search_ulist(int (*fptr) (int, struct user_info *), int farg) {
	int i, num;
	struct user_info *uentp;
	ythtbbs_cache_utmp_resolve();
	num = 0;

	for (i = 0; i < USHM_SIZE; i++) {
		uentp = ythtbbs_cache_utmp_get_by_idx(i);
		if (!uentp->active)
			continue;
		if ((*fptr) (farg, uentp)) {
			if (!uentp->active || !uentp->pid || isreject(uentp))
				continue;
			if (uentp->invisible == 1) {
				if (HAS_PERM(PERM_SYSOP | PERM_SEECLOAK, currentuser)) {
					prints("\033[1;32m隐身中   \033[m");
					continue;
				} else
					continue;
			}
			num++;
			if (num == 1)
				prints("目前在站上，状态如下：\n");

			if (uentp->mode != USERDF4) {
				//非用户自定义状态 add by leoncom@bmy
				if (uentp->mode == USERDF1 || uentp->mode == USERDF2 || uentp->mode == USERDF3 )
					prints("%s%-16s\033[m ",uentp->pid == 1 ? "\033[35m" : "\033[1;34m", ModeType(uentp->mode));
				else
					prints("%s%-16s\033[m ",uentp->pid == 1 ? "\033[35m" : "\033[1m", ModeType(uentp->mode));
			} else {
				//用户自定义状态
				if(uentp->user_state_temp[0] != '\0')
					prints("%s%-16s\033[m ",uentp->pid == 1 ? "\033[35m" : "\033[1;34m", uentp->user_state_temp);
				else
					prints("%s%-16s\033[m ",uentp->pid == 1 ? "\033[35m" : "\033[1m", ModeType(LOCKSCREEN));
			}
			if ((num) % 5 == 0)
				outc('\n');
		}
	}
	outc('\n');
	return 0;
}


//add by mintbaggio@BMY for the person who can cloak
int user_isonline(char* userid) {
	struct user_info *uentp;
	int i;

	ythtbbs_cache_utmp_resolve();
	for (i = 0; i < USHM_SIZE; i++) {
		uentp = ythtbbs_cache_utmp_get_by_idx(i);
		if (uentp != NULL) {
			if (!uentp->active)
				continue;
			if (!strcmp(uentp->userid, userid)){
				return (uentp->active && uentp->pid) ? 1 : 0;
			}
		}
	}
	return 0;
}

void update_ulist(struct user_info *uentp, int uent) {
	struct user_info *ptr;
	ythtbbs_cache_utmp_resolve();
	if (uent > 0 && uent <= USHM_SIZE) {
		ptr = ythtbbs_cache_utmp_get_by_idx(uent - 1);
		// 局部更新结构体
		memcpy(&ptr->invisible, &uentp->invisible, (sizeof(struct user_info) - offsetof(struct user_info, invisible)));
	}
}

void
update_utmp()
{
	extern time_t old;
	old = uinfo.lasttime;
	update_ulist(&uinfo, utmpent);
}

int get_utmp(void) {
	const struct user_info *ptr;

	ptr = ythtbbs_cache_utmp_get_by_idx(utmpent - 1);
	if (!strcmp(uinfo.userid, ptr->userid)) {
		memcpy(&uinfo, ptr, sizeof (uinfo));
		return 0;
	}
	return -1;
}

/* added by djq 99.7.19 */
/* function added by douglas 990305
 * set uentp to the user who is calling me
 * solve the "one of 2 line call sb. to five" problem
*/
int who_callme(struct user_info *uentp, int (*fptr) (int, struct user_info *), int farg, int me) {
	int i;

	ythtbbs_cache_utmp_resolve();
	for (i = 0; i < USHM_SIZE; i++) {
		memcpy(uentp, ythtbbs_cache_utmp_get_by_idx(i), sizeof(struct user_info));
		if ((*fptr) (farg, uentp) && uentp->destuid == me)
			return i + 1;
	}
	return 0;
}

int
getbmnum(userid)
char *userid;
{
	int i, k, oldbm = 0;
	ythtbbs_cache_Board_resolve();
	const struct boardmem *board_ptr = NULL;

	for (k = 0; k < numboards; k++) {
		board_ptr = ythtbbs_cache_Board_get_board_by_idx(k);
		for (i = 0; i < BMNUM; i++) {
			if (board_ptr->header.bm[i][0] == 0) {
				if (k < 4) {
					k = 3;
					continue;
				}
				break;
			}
			if (!strcmp(board_ptr->header.bm[i], userid))
				oldbm++;
		}
	}
	return oldbm;
}

// term 下取 sessionid 的开头部分，未来不需要
char *get_temp_sessionid(char *temp_sessionid, size_t len) {
	snprintf(temp_sessionid, len, "%c%c%c%s",
		(utmpent - 1) / (26 * 26) + 'A',
		(utmpent - 1) / 26 % 26 + 'A',
		(utmpent - 1) % 26 + 'A', uinfo.sessionid);
	temp_sessionid[len - 1] = 0;
	return temp_sessionid;
}

void
show_small_bm(char *board)
{
	struct boardmem *bptr;
	int i;
	short active, invisible;
	bptr = ythtbbs_cache_Board_get_board_by_name(board);
	if (bptr == NULL)
		return;
	move(0, 0);
	prints("\033[m");
	prints("\033[1;44;33m");
	for (i = 4; i < 10; i++) {	//仅对12个小版务的显示
		if (bptr->header.bm[i][0] == 0) {
			if (i == 4) {
				prints("本版无小版主 ");
				continue;
			} else {
				prints("             ");
				continue;
			}
		}
		active = bptr->bmonline & (1 << i);
		invisible = bptr->bmcloak & (1 << i);
		if (active && !invisible)
			prints("\033[32m%13s\033[33m", bptr->header.bm[i]);
		else if (active && invisible && (HAS_PERM(PERM_SEECLOAK, currentuser) || !strcmp(bptr->header.bm[i], currentuser.userid)))
			prints("\033[36m%13s\033[33m", bptr->header.bm[i]);
		else
			prints("%13s", bptr->header.bm[i]);
	}
	move(1, 0);
	prints("\033[1;44;33m");
	for (i = 10; i < BMNUM; i++) {
		if (bptr->header.bm[i][0] == 0) {
			prints("             ");
			continue;
		}
		active = bptr->bmonline & (1 << i);
		invisible = bptr->bmcloak & (1 << i);
		if (active && !invisible)
			prints("\033[32m%13s\033[33m", bptr->header.bm[i]);
		else if (active && invisible && (HAS_PERM(PERM_SEECLOAK, currentuser) || !strcmp(bptr->header.bm[i], currentuser.userid)))
			prints("\033[36m%13s\033[33m", bptr->header.bm[i]);
		else
			prints("%13s", bptr->header.bm[i]);

	}
	prints("\033[m");
	return;
}

int
setbmstatus(int online)
{
	char path[256];
	sethomefile_s(path, sizeof(path), currentuser.userid, "mboard");
	bmfilesync(&currentuser);
	new_apply_record(path, sizeof (struct boardmanager), (void *) setbmhat, &online);
	return 0;
}

static int
setbmhat(struct boardmanager *bm, int *online)
{
	struct boardmem *board_ptr = ythtbbs_cache_Board_get_board_by_idx(bm->bid);
	if (strcmp(board_ptr->header.filename, bm->board)) {
		errlog("error board name %s, %s", board_ptr->header.filename, bm->board);
		return -1;
	}
	if (*online) {
		board_ptr->bmonline |= (1 << bm->bmpos);
		if (uinfo.invisible)
			board_ptr->bmcloak |= (1 << bm->bmpos);
		else
			board_ptr->bmcloak &= ~(1 << bm->bmpos);
	} else {
		board_ptr->bmonline &= ~(1 << bm->bmpos);
		board_ptr->bmcloak &= ~(1 << bm->bmpos);
	}
	return 0;
}

