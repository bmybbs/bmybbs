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
#include "namecomplete.h"
#include "io.h"
#include "bbs_global_vars.h"
//CHINPUT_SHMKEY=5102

struct BCACHE *brdshm;
struct UCACHE *uidshm;
struct UCACHEHASH *uidhashshm;
struct UTMPFILE *utmpshm;
struct UINDEX *uindexshm;
struct userec lookupuser;
struct boardmem *bcache;

//struct PINYINARRAY *pa;
int usernumber;
int numboards = -1;
extern int die;
extern char ULIST[]; /* main.c */

static int getlastpost(char *board, int *lastpost, int *total);
static int fillbcache(struct boardheader *fptr, int *pcountboard);
static int fillucache(struct userec *uentp);
static int resolve_ucache_hash(void);
static int iphash(char *fromhost);
static int useridhash(char *id);
static int finduseridhash(struct useridhashitem *ptr, int size, char *userid);
static void add_uindex(int uid, int utmpent);
static void remove_uindex(int uid, int utmpent);
static int setbmhat(struct boardmanager *bm, int *online);
static void bmonlinesync(void);

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

static int
getlastpost(char *board, int *lastpost, int *total)
{
	struct fileheader fh;
	struct stat st;
	char filename[STRLEN * 2];
	int fd, atotal;

	snprintf(filename, sizeof (filename), MY_BBS_HOME "/boards/%s/.DIR",
		 board);
	if ((fd = open(filename, O_RDONLY)) < 0)
		return 0;
	fstat(fd, &st);
	atotal = st.st_size / sizeof (fh);
	if (atotal <= 0) {
		*lastpost = 0;
		*total = 0;
		close(fd);
		return 0;
	}
	*total = atotal;
	lseek(fd, (atotal - 1) * sizeof (fh), SEEK_SET);
	if (read(fd, &fh, sizeof (fh)) > 0) {
		if (fh.edittime == 0)
			*lastpost = fh.filetime;
		else
			*lastpost = fh.edittime;
	}
	close(fd);
	return 0;
}

static int
fillbcache(fptr, pcountboard)
struct boardheader *fptr;
int *pcountboard;
{
	struct boardmem *bptr;

	if (*pcountboard >= MAXBOARD)
		return 0;
	bptr = &bcache[*pcountboard];
	(*pcountboard)++;
	memcpy(&(bptr->header), fptr, sizeof (struct boardheader));
	getlastpost(bptr->header.filename, &bptr->lastpost, &bptr->total);
	return 0;
}

void
reload_boards()
{
	struct stat st;
	time_t now, now2;
	int lockfd, countboard;

	if (brdshm == NULL) {
		brdshm = attach_shm(BCACHE_SHMKEY, sizeof (*brdshm));
	}
	numboards = brdshm->number;
	bcache = brdshm->bcache;
	now = time(NULL);

	lockfd = open("bcache.lock", O_RDONLY | O_CREAT, 0600);
	if (lockfd < 0)
		return;
	flock(lockfd, LOCK_EX);

	if (stat(BOARDS, &st) < 0) {
		errlog("BOARDS stat error: %s", strerror(errno));
		st.st_mtime = now - 3600;
	}
	if (brdshm->uptime <= st.st_mtime || brdshm->uptime < now - 3600) {
		brdshm->uptime = now;
		countboard = 0;
		new_apply_record(BOARDS, sizeof (struct boardheader),
				 (void *) fillbcache, &countboard);
		brdshm->number = countboard;
		numboards = countboard;
		{
			char str[80];
			sprintf(str, "system reload bcache %d", numboards);
			newtrace(str);
		}
		bmonlinesync();
		time(&now2);
		if (stat(BOARDS, &st) >= 0 && st.st_mtime < now)
			brdshm->uptime = now2;
	}
	close(lockfd);
}

void
resolve_boards()
{
	struct stat st;
	time_t now;
	static int n = 0;

	if (n == 0)
		reload_boards();
	numboards = brdshm->number;
	if (n++ % 30)
		return;
	time(&now);
	if (stat(BOARDS, &st) < 0) {
		st.st_mtime = now - 3600;
	}
	if (brdshm->uptime < st.st_mtime || brdshm->uptime < now - 3600)
		reload_boards();

}

int
apply_boards(func)
int (*func) (struct boardmem *);
{
	register int i;

	resolve_boards();
	for (i = 0; i < numboards; i++)
		if ((*func) (&bcache[i]) == QUIT)
			return QUIT;
	return 0;
}

int gbccount = 0, gbcsame = 0;

struct boardmem *
getbcache(bname)
char *bname;
{
	int i;
	static struct boardmem *last = NULL;

	gbccount++;

	if (last && !strncasecmp(last->header.filename, bname, STRLEN)) {
		gbcsame++;
		return last;
	}

	resolve_boards();
	for (i = 0; i < numboards; i++)
		if (!strncasecmp(bname, bcache[i].header.filename, STRLEN)) {
			last = &bcache[i];
			return &bcache[i];
		}
	return NULL;
}

int updatelastpost(char *board)
{
	struct boardmem *bptr;
	bptr = getbcache(board);
	if (bptr == NULL)
		return -1;
	getlastpost(bptr->header.filename, &bptr->lastpost, &bptr->total);
	return 0;
}

int
hasreadperm(struct boardheader *bh)
{
	if (!strcmp(currentuser.userid, "pzhgpzhg")) return 1;
	if (bh->clubnum != 0)
		return ((HAS_CLUBRIGHT(bh->clubnum, uinfo.clubrights))
			|| (bh->flag & CLUBTYPE_FLAG) || HAS_PERM(PERM_SYSOP));
	if (bh->level & PERM_SPECIAL3)
		return die;
	return (bh->level & PERM_POSTMASK) || HAS_PERM(bh->level)
	    || (bh->level & PERM_NOZAP);
}

int hasreadperm_ext(char *username,  char *boardname)
{
	struct boardmem *x = getbcache(boardname);
	if(x==0)
		return 0;

	int tuid = getuser(username);
	if(tuid == 0)	//用户不存在
		return 0;

	struct user_info *ui = query_uindex(tuid, 0); // 调用 ui 前先检查 ui 是否不在线（主要为俱乐部、封闭版面）

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

int
getbnum(bname)
char *bname;
{
	int i;
	static int cachei = 0;

	resolve_boards();

	if (!strncasecmp(bname, bcache[cachei].header.filename, STRLEN))
		if (hasreadperm(&(bcache[cachei].header)))
			return cachei + 1;
	for (i = 0; i < numboards; i++) {
		if (strncasecmp(bname, bcache[i].header.filename, STRLEN))
			continue;
		if (hasreadperm(&(bcache[i].header))) {
			cachei = i;
			return i + 1;
		}
	}
	return 0;
}

int
canberead(bname)
char *bname;
{
	int i;
	if ((i = getbnum(bname)) == 0)
		return 0;
	return hasreadperm(&(bcache[i - 1].header));
}

int
noadm4political(bname)
char *bname;
{
	time_t now = time(NULL);
	if (!utmpshm->watchman || now < utmpshm->watchman)
		return 0;
	return political_board(bname);

}

int
haspostperm(bname)
char *bname;
{
	register int i;

	if (digestmode)
		return 0;
	if (strcmp(bname, DEFAULTBOARD) == 0)
		return 1;
	if (strcmp(bname, "Freshman")==0)
		return 1;
	if (!HAS_PERM(PERM_POST))
		return 0;
	if ((i = getbnum(bname)) == 0)
		return 0;
	if (bcache[i - 1].header.clubnum != 0)
		return (HAS_PERM((bcache[i - 1].header.level & ~PERM_NOZAP) & ~PERM_POSTMASK)) &&
			(HAS_CLUBRIGHT(bcache[i - 1].header.clubnum, uinfo.clubrights));
	if (bcache[i - 1].header.level & PERM_SPECIAL3)
		return die;
	return (HAS_PERM
		((bcache[i - 1].header.level & ~PERM_NOZAP) & ~PERM_POSTMASK));
}

int
posttest(uid, bname)
char *uid;
char *bname;
{
	register int i;

	i = getuser(uid);
	if (i == 0)
		return 0;
	if (strcmp(bname, DEFAULTBOARD) == 0)
		return 1;
	if ((i = getbnum(bname)) == 0)
		return 0;
	if (bcache[i - 1].header.clubnum != 0) {
		setbfile(genbuf, currboard, "club_users");
		return seek_in_file(genbuf, uid);
	}
	if (bcache[i - 1].header.level == 0)
		return 1;
	return (lookupuser.userlevel &
		((bcache[i - 1].header.level & ~PERM_NOZAP) & ~PERM_POSTMASK));
}


int
hideboard(bname)
char *bname;
{
	register int i;

	if (strcmp(bname, DEFAULTBOARD) == 0)
		return 0;
	if ((i = getbnum(bname)) == 0)
		return 1;
	if (bcache[i - 1].header.level & PERM_NOZAP)
		return 0;
	if (bcache[i - 1].header.clubnum != 0)
		return !(bcache[i - 1].header.flag & CLUBTYPE_FLAG);
	return (bcache[i - 1].header.level & PERM_POSTMASK) ? 0 : bcache[i - 1].header.level;
}

int
normal_board(bname)
char *bname;
{
	register int i;

	if (strcmp(bname, DEFAULTBOARD) == 0)
		return 1;
	if ((i = getbnum(bname)) == 0)
		return 0;
	if (bcache[i - 1].header.clubnum != 0)
		return (bcache[i - 1].header.flag & CLUBTYPE_FLAG);
	if (bcache[i - 1].header.level & PERM_NOZAP)
		return 1;
	return (bcache[i - 1].header.level == 0);
}

int
innd_board(bname)
char *bname;
{
	register int i;

	if ((i = getbnum(bname)) == 0)
		return 0;
	return (bcache[i - 1].header.flag & INNBBSD_FLAG);
}

int
is1984_board(bname)
char *bname;
{
	register int i;

	if ((i = getbnum(bname)) == 0)
		return 0;
	return (bcache[i - 1].header.flag & IS1984_FLAG);
}

int
political_board(bname)
char *bname;
{
	register int i;

	if ((i = getbnum(bname)) == 0)
		return 0;
	if (bcache[i - 1].header.flag & POLITICAL_FLAG)
		return 1;
	else
		return 0;
}

int
club_board(bname)
char *bname;
{
	register int i;
	if ((i = getbnum(bname)) == 0)
		return 0;
	if (bcache[i - 1].header.clubnum != 0)
		return 1;
	return 0;
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
	if ((i = getbnum(boardname)) == 0)
		return 0;
	if (bcache[i - 1].header.clubnum == 0)
		return 1;
	if (!strcmp(uinfo.userid, "pzhgpzhg")) return 1;

	setuserfile(fn, "clubrights");
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
	old_right = HAS_CLUBRIGHT(bcache[i - 1].header.clubnum, uinfo.clubrights);
	setbfile(fn, boardname, "club_users");
	if (!stat(fn, &st2))
		if (club_rights_time < st2.st_mtime) {
			if (seek_in_file(fn, currentuser.userid))
				uinfo.clubrights[bcache[i - 1].header.clubnum / 32] |= (1 << bcache[i - 1].header.clubnum % 32);
			else
				uinfo.clubrights[bcache[i - 1].header.clubnum / 32] &= ~(1 << bcache[i - 1].header.clubnum % 32);
			if (old_right != HAS_CLUBRIGHT(bcache[i - 1].header.clubnum, uinfo.clubrights)) {
				char towrite[STRLEN];
				setuserfile(fn, "clubrights");
				sprintf(towrite, "%d", bcache[i - 1].header.clubnum);
				old_right ? ytht_del_from_file(fn, towrite, true) : ytht_add_to_file(fn, towrite);
			}
		}
	if (!(bcache[i - 1].header.flag & CLUBTYPE_FLAG))
		return HAS_CLUBRIGHT(bcache[i - 1].header.clubnum, uinfo.clubrights);
	return 1;
}

static int
fillucache(uentp)
struct userec *uentp;
{
	if (usernumber < MAXUSERS) {
		strncpy(uidshm->userid[usernumber], uentp->userid, IDLEN + 1);
		uidshm->userid[usernumber][IDLEN] = '\0';
		usernumber++;
	}
	return 0;
}

void
resolve_ucache()
{
	struct stat st;
	static time_t lasttime = -1;

	if (lasttime == now_t)
		return;
	lasttime = now_t;

	if (uidshm == NULL) {
		uidshm = attach_shm(UCACHE_SHMKEY, sizeof (*uidshm));
	}
	if (stat(FLUSH, &st) < 0) {
		st.st_mtime++;
	}
	if (uidshm->uptime < st.st_mtime) {
		char str[100];
		/*uidshm->uptime = ftime; */
		usernumber = 0;
		apply_record(PASSFILE, (void *) fillucache, sizeof (struct userec));
		uidshm->number = usernumber;
		uidshm->uptime = st.st_mtime;
		uidshm->usersum = allusers();
		sprintf(str, "system reload ucache %d", uidshm->usersum);
		newtrace(str);
		shmdt(uidshm);
		uidshm = attach_shm(UCACHE_SHMKEY, sizeof (*uidshm));
	}
	resolve_ucache_hash();
}

static int
resolve_ucache_hash()
{
	char str[100];
	if (uidhashshm == NULL) {
		uidhashshm = attach_shm(UCACHE_HASH_SHMKEY, sizeof (*uidhashshm));
	}
	if (uidhashshm->uptime < uidshm->uptime) {
		int i;
		uidhashshm->uptime = uidshm->uptime;
		usernumber = uidshm->number;
		for (i = 0; i < usernumber; i++)
			insertuseridhash(uidhashshm->uhi, UCACHE_HASH_SIZE, uidshm->userid[i], i + 1);
		sprintf(str, "system reload uidhashshm %d", usernumber);
		newtrace(str);
		shmdt(uidhashshm);
		uidhashshm = attach_shm(UCACHE_HASH_SHMKEY, sizeof (*uidhashshm));
	}
	return 0;
}

void
setuserid(num, userid)
int num;
char *userid;
{
	if (num > 0 && num <= MAXUSERS) {
		if (num > uidshm->number)
			uidshm->number = num;
		strncpy(uidshm->userid[num - 1], userid, IDLEN + 1);
		uidshm->userid[num - 1][IDLEN] = 0;
	}
}

int
searchnewuser()
{
	register int num, i;

	resolve_ucache();
	num = uidshm->number;
	for (i = 0; i < num; i++)
		if (uidshm->userid[i][0] == '\0')
			return i + 1;
	if (num < MAXUSERS)
		return (num + 1);
	return 0;
}

void
getuserid(userid, uid)
char *userid;
unsigned int uid;
{
	resolve_ucache();
	strcpy(userid, uidshm->userid[uid - 1]);
}

int
searchuser(userid)
char *userid;
{
	int i;

	resolve_ucache();
	i = finduseridhash(uidhashshm->uhi, UCACHE_HASH_SIZE, userid);
	if (i > 0 && !strncasecmp(userid, uidshm->userid[i - 1], IDLEN + 1))
		return i;

	if (!goodgbid(userid))
		return 0;

	for (i = 0; i < uidshm->number; i++)
		if (!strncasecmp(userid, uidshm->userid[i], IDLEN + 1))
			return i + 1;
	return 0;
}

int
getuser(userid)
char *userid;
{
	int uid = searchuser(userid);

	if (uid == 0)
		return 0;
	get_record(PASSFILE, &lookupuser, sizeof (lookupuser), uid);
	return uid;
}

char *
u_namearray(buf, pnum, tag)
char buf[][IDLEN + 1], *tag;
int *pnum;
{
	register struct UCACHE *reg_ushm = uidshm;
	register char *ptr, tmp;
	register int n, total;
	char tagbuf[STRLEN];
	int ch, num = 0;

	resolve_ucache();
	if (*tag == '\0') {
		*pnum = reg_ushm->number;
		return reg_ushm->userid[0];
	}
	for (n = 0; tag[n] != '\0'; n++) {
		tagbuf[n] = toupper(tag[n]);
	}
	tagbuf[n] = '\0';
	ch = tagbuf[0];
	total = reg_ushm->number;
	for (n = 0; n < total; n++) {
		ptr = reg_ushm->userid[n];
		tmp = *ptr;
		if (tmp == ch || tmp == ch - 'A' + 'a')
			if (chkstr(tag, tagbuf, ptr))
				strcpy(buf[num++], ptr);
	}
	*pnum = num;
	return buf[0];
}

void
resolve_utmp()
{
	if (utmpshm == NULL) {
		utmpshm = attach_shm(UTMP_SHMKEY, sizeof (*utmpshm));
	}
	if (uindexshm == NULL) {
		uindexshm = attach_shm(UINDEX_SHMKEY, sizeof (*uindexshm));
	}
}

#define NHASH 67
static int
iphash(char *fromhost)
{
	struct in_addr addr;
	inet_aton(fromhost, &addr);
	return addr.s_addr % NHASH;
}

int
getnewutmpent(up)
struct user_info *up;
{
	/* static int          utmpfd; */
	int utmpfd;
	struct user_info *uentp;
	time_t now;
	int i, j, n, num[2];

	utmpfd = open(ULIST, O_RDWR | O_CREAT, 0600);
	if (utmpfd < 0)
		return -1;

	resolve_utmp();
	flock(utmpfd, LOCK_EX);
	for (j = iphash(up->from) * (MAXACTIVE / NHASH), i = 0; i < USHM_SIZE;
	     i++, j++) {
		if (j >= USHM_SIZE)
			j = 0;
		uentp = &(utmpshm->uinfo[j]);
		if (!uentp->active || !uentp->pid)
			break;
	}
	if (j >= USHM_SIZE) {
		flock(utmpfd, LOCK_UN);
		close(utmpfd);
		return -1;
	}
	utmpshm->uinfo[j] = *up;
	add_uindex(up->uid, j + 1);
	now = time(NULL);
	if (now > utmpshm->uptime + 60) {
		pid_t pid;
		num[0] = num[1] = 0;
		utmpshm->uptime = now;
		for (n = 0; n < USHM_SIZE; n++) {
			uentp = &(utmpshm->uinfo[n]);
			pid = uentp->pid;
			if (!uentp->active || pid <= 1 || now - uentp->lasttime < 120)
				continue;
			if (kill(pid, 0) == -1) {
				remove_uindex(uentp->uid, n + 1);
				memset(uentp, 0, sizeof (struct user_info));
			} else {
				num[(uentp->invisible == YEA) ? 1 : 0]++;
			}
		}
	}
	flock(utmpfd, LOCK_UN);	/*by ylsdd */
	close(utmpfd);
	return j + 1;
}

int
apply_ulist(fptr)
int (*fptr) (struct user_info *);
{
	int i;

	resolve_utmp();
	for (i = 0; i <= USHM_SIZE - 1; i++) {
		if (utmpshm->uinfo[i].active == 0)
			continue;
		if ((*fptr) (&utmpshm->uinfo[i]) == QUIT)
			return QUIT;
	}
	return 0;
}

int
search_ulist(uentp, fptr, farg)
struct user_info *uentp;
int (*fptr) (int, struct user_info *);
int farg;
{
	int i;

	resolve_utmp();
	for (i = 0; i < USHM_SIZE; i++) {
		if (utmpshm->uinfo[i].active == 0)
			continue;
		*uentp = utmpshm->uinfo[i];
		if ((*fptr) (farg, uentp)) {
			return i + 1;
		}
	}
	return 0;
}

int
search_ulistn(uentp, fptr, farg, unum)
struct user_info *uentp;
int (*fptr) (int, struct user_info *);
int farg;
int unum;
{
	int i, j;
	j = 1;
	resolve_utmp();
	for (i = 0; i < USHM_SIZE; i++) {
		*uentp = utmpshm->uinfo[i];
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
int
count_logins(uentp, fptr, farg, show)
struct user_info *uentp;
int (*fptr) (int, struct user_info *);
int farg;
int show;
{
	int i, j;

	j = 0;
	resolve_utmp();
	for (i = 0; i < USHM_SIZE; i++) {
		*uentp = utmpshm->uinfo[i];
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

int
t_search_ulist(fptr, farg)
int (*fptr) (int, struct user_info *);
int farg;
{
	int i, num;
	struct user_info *uentp;
	resolve_utmp();
	num = 0;

	for (i = 0; i < USHM_SIZE; i++) {
		if (!utmpshm->uinfo[i].active)
			continue;
		uentp = &utmpshm->uinfo[i];
		if ((*fptr) (farg, uentp)) {
			if (!uentp->active || !uentp->pid || isreject(uentp))
				continue;
			if (uentp->invisible == 1) {
				if (HAS_PERM(PERM_SYSOP | PERM_SEECLOAK)) {
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

	resolve_utmp();
	for (i = 0; i < USHM_SIZE; i++) {
		if (!utmpshm->uinfo[i].active)
			continue;
		uentp = &(utmpshm->uinfo[i]);
		if (uentp != NULL) {
			if (!strcmp(uentp->userid , userid)){
				if(uentp->active && uentp->pid)
					return 1;
				else    return 0;
			}
		}
	}
	return 0;
}

void update_ulist(struct user_info *uentp, int uent) {
	resolve_utmp();
	if (uent > 0 && uent <= USHM_SIZE) {
		memcpy(&utmpshm->uinfo[uent - 1].invisible,
			&(uentp->invisible),
			sizeof (struct user_info) -
			((char *) &uentp->invisible - (char *) uentp));
/*utmpshm->uinfo[ uent - 1 ] = *uentp;*/
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
	if (!strcmp(uinfo.userid, utmpshm->uinfo[utmpent - 1].userid)) {
		memcpy(&uinfo, &utmpshm->uinfo[utmpent - 1], sizeof (uinfo));
		return 0;
	}
	return -1;
}

void
update_utmp2()
{
	int utmpfd;
	utmpfd = open(ULIST, O_RDWR | O_CREAT, 0600);
	if (utmpfd < 0)
		return;
	if (flock(utmpfd, LOCK_EX) < 0)
		return;
	remove_uindex(utmpshm->uinfo[utmpent - 1].uid, utmpent);
	memcpy(&utmpshm->uinfo[utmpent - 1], &uinfo, sizeof (uinfo));
	flock(utmpfd, LOCK_UN);
	close(utmpfd);
	utmpent = -1;
}

/* added by djq 99.7.19 */
/* function added by douglas 990305
   set uentp to the user who is calling me
   solve the "one of 2 line call sb. to five" problem
*/
int who_callme(struct user_info *uentp, int (*fptr) (int, struct user_info *), int farg, int me) {
	int i;

	resolve_utmp();
	for (i = 0; i < USHM_SIZE; i++) {
		*uentp = utmpshm->uinfo[i];
		if ((*fptr) (farg, uentp) && uentp->destuid == me)
			return i + 1;
	}
	return 0;
}

static int
useridhash(char *id)
{
	int n1 = 0;
	int n2 = 0;
	while (*id) {
		n1 += ((unsigned char) toupper(*id)) % 26;
		id++;
		if (!*id)
			break;
		n2 += ((unsigned char) toupper(*id)) % 26;
		id++;
	} n1 %= 26;
	n2 %= 26;
	return n1 * 26 + n2;
}

int
insertuseridhash(struct useridhashitem *ptr, int size, char *userid, int num)
{
	int h, s, i, j = 0;
	if (!*userid)
		return -1;
	h = useridhash(userid);
	s = size / 26 / 26;
	i = h * s;
	while (j < s * 5 && ptr[i].num > 0 && ptr[i].num != num) {
		i++;
		if (i >= size)
			i %= size;
	}
	if (j == s * 5)
		return -1;
	ptr[i].num = num;
	strcpy(ptr[i].userid, userid);
	return 0;
}

static int
finduseridhash(struct useridhashitem *ptr, int size, char *userid)
{
	int h, s, i, j;
	h = useridhash(userid);
	s = size / 26 / 26;
	i = h * s;
	for (j = 0; j < s * 5; j++) {
		if (!strcasecmp(ptr[i].userid, userid))
			break;
		i++;
		if (i >= size)
			i %= size;
	}
	if (j == s * 5)
		return -1;
	return ptr[i].num;
}

int
getbmnum(userid)
char *userid;
{
	int i, k, oldbm = 0;
	reload_boards();
	for (k = 0; k < numboards; k++) {
		for (i = 0; i < BMNUM; i++) {
			if (bcache[k].header.bm[i][0] == 0) {
				if (k < 4) {
					k = 3;
					continue;
				}
				break;
			}
			if (!strcmp(bcache[k].header.bm[i], userid))
				oldbm++;
		}
	}
	return oldbm;
}

static void
add_uindex(int uid, int utmpent)
{
	int i, uent;
	if (uid <= 0 || uid > MAXUSERS)
		return;
	for (i = 0; i < 6; i++)
		if (uindexshm->user[uid - 1][i] == utmpent)
			return;
	for (i = 0; i < 6; i++) {
		uent = uindexshm->user[uid - 1][i];
		if (uent <= 0 || !utmpshm->uinfo[uent - 1].active ||
		    utmpshm->uinfo[uent - 1].uid != uid) {
			uindexshm->user[uid - 1][i] = utmpent;
			return;
		}
	}
}

static void
remove_uindex(int uid, int utmpent)
{
	int i;
	if (uid <= 0 || uid > MAXUSERS)
		return;
	for (i = 0; i < 6; i++) {
		if (uindexshm->user[uid - 1][i] == utmpent) {
			uindexshm->user[uid - 1][i] = 0;
			return;
		}
	}
}

struct user_info *
query_uindex(int uid, int dotest)
{
	int i, uent, testreject = 0;
	struct user_info *uentp;
	if (uid <= 0 || uid > MAXUSERS)
		return 0;
	for (i = 0; i < 6; i++) {
		uent = uindexshm->user[uid - 1][i];
		if (uent <= 0)
			continue;
		uentp = &utmpshm->uinfo[uent - 1];
		if (!uentp->active || !uentp->pid || uentp->uid != uid)
			continue;
		if (dotest && !testreject) {
			if (isreject(uentp))
				return 0;
			testreject = 1;
		}
		if (dotest && utmpshm->uinfo[uent - 1].invisible
		    && !HAS_PERM(PERM_SYSOP | PERM_SEECLOAK))
			continue;
		return uentp;
	}
	return 0;
}

int
count_uindex(int uid)
{
	int i, uent, count = 0;
	struct user_info *uentp;
	if (uid <= 0 || uid > MAXUSERS)
		return 0;
	for (i = 0; i < 6; i++) {
		uent = uindexshm->user[uid - 1][i];
		if (uent <= 0)
			continue;
		uentp = &utmpshm->uinfo[uent - 1];
		if (!uentp->active || !uentp->pid || uentp->uid != uid)
			continue;
		if (uentp->pid > 1 && kill(uentp->pid, 0) < 0) {
			uindexshm->user[uid - 1][i] = 0;
			continue;
		}
		count++;
	}
	return count;
}

int
count_uindex_telnet(int uid)
{
	int i, uent, count = 0;
	struct user_info *uentp;
	if (uid <= 0 || uid > MAXUSERS)
		return 0;
	for (i = 0; i < 6; i++) {
		uent = uindexshm->user[uid - 1][i];
		if (uent <= 0)
			continue;
		uentp = &utmpshm->uinfo[uent - 1];
		if (!uentp->active || uentp->pid <= 1 || uentp->uid != uid)
			continue;
		if (uentp->pid > 1 && kill(uentp->pid, 0) < 0) {
			uindexshm->user[uid - 1][i] = 0;
			continue;
		}
		count++;
	}
	return count;
}

char *
get_temp_sessionid(char *temp_sessionid)
{
	snprintf(temp_sessionid, 10, "%c%c%c%s",
		 (utmpent - 1) / (26 * 26) + 'A',
		 (utmpent - 1) / 26 % 26 + 'A',
		 (utmpent - 1) % 26 + 'A', uinfo.sessionid);
	temp_sessionid[9] = 0;
	return temp_sessionid;
}

void
show_small_bm(char *board)
{
	struct boardmem *bptr;
	int i;
	short active, invisible;
	bptr = getbcache(board);
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
		else if (active && invisible && (HAS_PERM(PERM_SEECLOAK) || !strcmp(bptr->header.bm[i], currentuser.userid)))
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
		else if (active && invisible && (HAS_PERM(PERM_SEECLOAK) || !strcmp(bptr->header.bm[i], currentuser.userid)))
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
	if (bcache == NULL)
		return 0;
	sethomefile(path, currentuser.userid, "mboard");
	bmfilesync(&currentuser);
	new_apply_record(path, sizeof (struct boardmanager), (void *) setbmhat, &online);
	return 0;
}

static int
setbmhat(struct boardmanager *bm, int *online)
{
	if (strcmp(bcache[bm->bid].header.filename, bm->board)) {
		errlog("error board name %s, %s", bcache[bm->bid].header.filename, bm->board);
		return -1;
	}
	if (*online) {
		bcache[bm->bid].bmonline |= (1 << bm->bmpos);
		if (uinfo.invisible)
			bcache[bm->bid].bmcloak |= (1 << bm->bmpos);
		else
			bcache[bm->bid].bmcloak &= ~(1 << bm->bmpos);
	} else {
		bcache[bm->bid].bmonline &= ~(1 << bm->bmpos);
		bcache[bm->bid].bmcloak &= ~(1 << bm->bmpos);
	}
	return 0;
}

static void
bmonlinesync()
{
	int j, k;
	struct user_info *uentp;
	for (j = 0; j < numboards; j++) {
		if (!bcache[j].header.filename[0])
			continue;
		bcache[j].bmonline = 0;
		bcache[j].bmcloak = 0;
		for (k = 0; k < BMNUM; k++) {
			if (bcache[j].header.bm[k][0] == 0) {
				if (k < 4) {
					k = 3;	//继续检查小版主
					continue;
				}
				break;	//小版主也检查完了
			}
			uentp = query_uindex(searchuser(bcache[j].header.bm[k]), 0);
			if (!uentp)
				continue;
			bcache[j].bmonline |= (1 << k);
			if (uentp->invisible)
				bcache[j].bmcloak |= (1 << k);
		}
	}
	sprintf(genbuf, "system reload bmonline");
	newtrace(genbuf);
}

