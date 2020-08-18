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
#include "time.h"
#include "stdio.h"

int ERROR_READ_SYSTEM_FILE = NA;
int RMSG = YEA;
int autologin = 0;
int have_msg_unread = 0;
int nettyNN = 0;
int count_friends = 0, count_users = 0;
int die;
int iscolor = 1;
int listmode;
int numofsig = 0;
jmp_buf byebye;

FILE *ufp;
int talkrequest = NA;
int enter_uflags;
time_t lastnote;

struct user_info uinfo;

char fromhost[60];

char ULIST[STRLEN];
int utmpent = -1;
time_t login_start_time;
int showansi = 1;
int convcode = 0;

char GoodWish[20][STRLEN - 3];
int WishNum = 0;
int orderWish = 0;
int runtest;
int runssh = 0;

static void u_enter(void);
static void setflags(int mask, int value);
static void u_exit(void);
static void talk_request(int signum);
static void multi_user_check(void);
static int simplepasswd(char *str, int check);
static void reaper(void);
static void system_init(int argc, char **argv);
static int getuptime(void);
static void login_query(void);
static void direct_login(void);
static void notepad_init(void);
static void user_login(void);
static int chk_friend_book(void);
static int getinput_intime(unsigned char *buf, int len, int timeout);
static int check_tty_lines(void);
static char *boardmargin(void);
static void R_endline(int signum);
static void tlog_recover(void);
int
ifinprison(char *name)
{
	FILE *fp;
	char buf[100];
	fp = fopen(MY_BBS_HOME "/etc/prisonor", "r");
	if (fp == NULL)
		return 0;
	while (fgets(buf, 80, fp) != NULL) {
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0;
		if (!strcmp(name, buf))
			return 1;
	}
	fclose(fp);
	return 0;
}

static void
u_enter()
{
	if (currentuser.dietime)
		currentuser.userdefine &=
		    ~(DEF_ANIENDLINE | DEF_ACBOARD | DEF_COLOR | DEF_ENDLINE);
	inprison = ifinprison(currentuser.userid);
	if (inprison) {
		currentuser.userdefine &=
		    ~(DEF_ANIENDLINE | DEF_ACBOARD | DEF_COLOR | DEF_ENDLINE);
		currentuser.dietime = currentuser.dietime + 1;
	}
	if (currentuser.dietime)
		die = 1;
	else
		die = 0;
	enter_uflags = currentuser.flags[0];
	listmode = 0;
	digestmode = NA;
	if (autologin) {
		if (get_utmp() != 0) {
			prints("Can't locate uinfo.\n");
			refresh();
			exit(-1);
		}
		return;
	}
	memset(&uinfo, 0, sizeof (uinfo));

	uinfo.active = YEA;
	uinfo.pid = getpid();
	if ((HAS_PERM(PERM_LOGINCLOAK) && (currentuser.flags[0] & CLOAK_FLAG))
	    || currentuser.dietime)
		uinfo.invisible = YEA;
	uinfo.mode = LOGIN;
	uinfo.pager = 0;
	if (DEFINE(DEF_DELDBLCHAR))
		enabledbchar = 1;
	else
		enabledbchar = 0;
	if (DEFINE(DEF_FRIENDCALL)) {
		uinfo.pager |= FRIEND_PAGER;
	}
	if (currentuser.flags[0] & PAGER_FLAG) {
		uinfo.pager |= ALL_PAGER;
		uinfo.pager |= FRIEND_PAGER;
	}
	if (DEFINE(DEF_FRIENDMSG)) {
		uinfo.pager |= FRIENDMSG_PAGER;
	}
	if (DEFINE(DEF_ALLMSG)) {
		uinfo.pager |= ALLMSG_PAGER;
		uinfo.pager |= FRIENDMSG_PAGER;
	}
	uinfo.uid = usernum;
	uinfo.userlevel = currentuser.userlevel;
	strncpy(uinfo.from, fromhost, 16);
	uinfo.lasttime = time(0);
	if (runssh)
		uinfo.isssh = 1;
	iscolor = (DEFINE(DEF_COLOR)) ? 1 : 0;
	strncpy(uinfo.userid, currentuser.userid, 20);
	strncpy(uinfo.realname, currentuser.realname, 20);
	strncpy(uinfo.username, currentuser.username, 40);
	getrandomstr(uinfo.sessionid);
	getfriendstr();
	getrejectstr();
	if (HAS_PERM(PERM_EXT_IDLE))
		uinfo.ext_idle = YEA;
	uinfo.curboard = 0;
	utmpent = getnewutmpent(&uinfo);
	if (utmpent < 0) {
		errlog("Fault: No utmpent slot for %s\n", uinfo.userid);
	}
}

static void
setflags(mask, value)
int mask, value;
{
	if (((currentuser.flags[0] & mask) && 1) != value) {
		if (value)
			currentuser.flags[0] |= mask;
		else
			currentuser.flags[0] &= ~mask;
	}
}

static void
u_exit()
{
	signal(SIGHUP, SIG_DFL);
	signal(SIGALRM, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	setflags(PAGER_FLAG, (uinfo.pager & ALL_PAGER));
	if (HAS_PERM(PERM_LOGINCLOAK))
		setflags(CLOAK_FLAG, uinfo.invisible);

	if (currentuser.flags[0] != enter_uflags && !ERROR_READ_SYSTEM_FILE) {
		set_safe_record();
		substitute_record(PASSFILE, &currentuser, sizeof (currentuser),
				  usernum);
	}

	uinfo.active = NA;
	uinfo.pid = 0;
	uinfo.invisible = YEA;
	uinfo.sockactive = NA;
	uinfo.sockaddr = 0;
	uinfo.destuid = 0;
	uinfo.lasttime = 0;
	update_utmp2();
}

int
cmpuids(uid, up)
char *uid;
struct userec *up;
{
	return !strncasecmp(uid, up->userid, sizeof (up->userid));
}

int
dosearchuser(userid)
char *userid;
{
	int id;

	if ((id = getuser(userid)) != 0) {
		if (cmpuids(userid, &lookupuser)) {
			memcpy(&currentuser, &lookupuser, sizeof (currentuser));
			return usernum = id;
		}
	}
	memset(&currentuser, 0, sizeof (currentuser));
	return usernum = 0;
}

int started = 0;

static void
talk_request(int signum)
{
	signal(SIGUSR1, talk_request);
	talkrequest = YEA;
	bell();
	bell();
	bell();
	sleep(1);
	bell();
	bell();
	bell();
	bell();
	bell();
	return;
}

void
do_abort_bbs()
{
	time_t stay;
#ifdef CAN_EXEC
	extern char tempfile[];

	unlink(tempfile);
#endif
	if (!started)
		return;

	started = 0;
	if (uinfo.mode == POSTING || uinfo.mode == SMAIL || uinfo.mode == EDIT
	    || uinfo.mode == EDITUFILE || uinfo.mode == EDITSFILE
	    || uinfo.mode == EDITANN)
		keep_fail_post();
	stay = time(0) - login_start_time;
	sprintf(genbuf, "%s drop %ld", currentuser.userid, stay);
	newtrace(genbuf);
	if ((currentuser.userlevel & PERM_BOARDS)
	    && (count_uindex(usernum) == 1))
		setbmstatus(0);
	u_exit();
}

void
abort_bbs()
{
	do_abort_bbs();
	exit(0);
}

static void
multi_user_check()
{
	struct user_info uin, *puin;
	char buffer[STRLEN];
	int logins;

	if (HAS_PERM(PERM_MULTILOG))
		return;		/* don't check sysops */

	if (!strcmp("guest", currentuser.userid)) {
		if (heavyload(0)) {
			prints
			    ("\x1b[1;33m±§Ç¸, Ä¿Ç°ÏµÍ³¸ººÉ¹ýÖØ, ÇëÎðÖØ¸² Login¡£\x1b[m\n");
			refresh();
			exit(1);
		}
		return;
	}
	logins = count_uindex_telnet(usernum);
	if (!logins)
		return;
	puin = query_uindex(usernum, 0);
	if (!puin)
		return;
	uin = *puin;
	if (!uin.active || uin.pid <= 1 || (kill(uin.pid, 0) == -1))
		return;

	getdata(t_lines - 1, 0,
		"\033[1;37mÄúÏëÉ¾³ýÖØ¸´µÄ login Âð (Y/N)? [N]\033[m", buffer, 4,
		DOECHO, YEA);
	if (toupper(buffer[0]) != 'Y') {
		logins = count_uindex_telnet(usernum);
		if (logins >= 2 || (currentuser.dietime > 0 && logins >= 2)) {
			scroll();
			scroll();
			move(t_lines - 2, 0);
			prints
			    ("\033[1;33mºÜ±§Ç¸, ÄúÒÑ Telnet Login ÏàÍ¬ÕÊºÅ%d´Î, "
			     "ÎªÈ·±£ËûÈËÉÏÕ¾È¨Òæ,\n ´ËÁ¬Ïß½«±»È¡Ïû¡£\033[m\n",
			     logins);
			refresh();
			exit(1);
		}
		return;
	}
	//ÒòÎªÎÊ´ðÖ®¼ä¿ÉÒÔ·¢ÉúºÜ¶àÊÂÇé, ÖØÐÂ¶ÁÈ¡
	puin = query_uindex(usernum, 0);
	if (!puin)
		return;
	uin = *puin;
	if (!uin.active || uin.pid <= 1 || (kill(uin.pid, 0) == -1))
		return;

	kill(uin.pid, 9);
	snprintf(buffer, sizeof (buffer), "%s kick %s multi-login",
		 currentuser.userid, currentuser.userid);
	newtrace(buffer);
}

static int
simplepasswd(str, check)
char *str;
int check;
{
	char ch;

	while ((ch = *str++) != '\0') {
		if (check == 1) {
			if (!(ch >= 'a' && ch <= 'z'))
				return 0;
		} else if (!(ch >= '0' && ch <= '9'))
			return 0;
	}
	return 1;
}

static void
reaper()
{
	while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0) ;
	signal(SIGCHLD, (void *) reaper);
}

static void
system_init(argc, argv)
int argc;
char **argv;
{
	struct sigaction act;

	mallopt(M_MMAP_THRESHOLD, 10000);

	login_start_time = time(0);
	gethostname(genbuf, 256);
	sprintf(ULIST, "%s.%s", ULIST_BASE, genbuf);

	if (argc >= 3) {
		strncpy(fromhost, argv[2], 60);
	} else {
		fromhost[0] = '\0';
	}
	fromhost[59] = '\0';

#ifndef lint
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
#ifdef DOTIMEOUT
	init_alarm();
	uinfo.mode = LOGIN;
	alarm(LOGIN_TIMEOUT);
#else
	signal(SIGALRM, SIG_SIG);
#endif
	signal(SIGTERM, SIG_IGN);
	signal(SIGURG, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
#endif
	signal(SIGHUP, (void *) abort_bbs);
	//signal(SIGTTOU,count_msg) ;
	signal(SIGTTOU, R_endline);
	signal(SIGUSR1, talk_request);
	signal(SIGUSR2, (void *) r_msg);
	signal(SIGCHLD, (void *) reaper);

	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NODEFER;
	act.sa_handler = (void *) r_msg;
	sigaction(SIGUSR2, &act, NULL);
}

static int
getuptime()
{
	FILE *fp;
	int n;
	fp = fopen("/proc/uptime", "r");
	if (fp == NULL)
		return 0;
	fscanf(fp, "%d", &n);
	fclose(fp);
	return n;
}

static void
login_query()
{
	char uid[IDLEN + 2], passbuf[PASSLEN];
	int curr_login_num, attempts, n;
	char buf[STRLEN];
	extern struct UTMPFILE *utmpshm;
/*-----------------New Century-----
time_t timenow;
int dis;
char str[40], str1[100];
timenow=time(NULL);
strcpy(str,ctime(&timenow));
str[24]='\0';
dis=946656000-timenow;
if(dis>=0) sprintf(str1,"ÏÖÔÚÊÇ %s , ¾à2000Äê»¹ÓÐ%dÃë\n", str, dis);
else sprintf(str1,"ÏÖÔÚÊÇ %s, ÐÂÊÀ¼ÍÒÑ¾­¿ªÊ¼ÁË%dÃë\n",str,-dis);
---------------------------------*/

	resolve_utmp();
	resolve_ucache();
	curr_login_num = num_active_users();
	if (curr_login_num >= MAXACTIVE || curr_login_num >= MAXACTIVERUN) {
		ansimore("etc/loginfull", NA);
		refresh();
		sleep(1);
		exit(1);
	}
	fill_shmfile(5, "etc/endline", ENDLINE1_SHMKEY);
	currentuser.userdefine |= DEF_COLOR;
	ansimore2("etc/issue", NA, 0, 20);
	if (uidshm->usersum == 0)
		uidshm->usersum = allusers();
	if (utmpshm->maxtoday < curr_login_num)
		utmpshm->maxtoday = curr_login_num;
	if (utmpshm->maxuser < curr_login_num) {	/* Added by deardragon 1999.12.15 ±£´æ×î¸ßÈËÊý¼ÇÂ¼µ½ÎÄ¼þ */
		FILE *maxfp;
		utmpshm->maxuser = curr_login_num;
		maxfp = fopen(".max_login_num", "r");
		if (maxfp == NULL) {
			maxfp = fopen(".max_login_num", "w+");
			fprintf(maxfp, "%d", utmpshm->maxuser);
			fclose(maxfp);
		} else {
			int temp_max;
			fscanf(maxfp, "%d", &temp_max);
			fclose(maxfp);
			if (temp_max > utmpshm->maxuser) {
				utmpshm->maxuser = temp_max;
			} else {
				maxfp = fopen(".max_login_num", "w+");
				fprintf(maxfp, "%d", utmpshm->maxuser);
				fclose(maxfp);
			}
		}
	}

	move(t_lines - 4, 0);
	n = getuptime();
	prints("\033[1;32m»¶Ó­¹âÁÙ\033[1;33m %s\033[32m ", MY_BBS_NAME);
	prints
	    ("Ä¿Ç°ÉÏÕ¾ÈËÊý [\033[36m%d/%d\033[32m] WWWÄäÃû[\033[36m%d\033[32m] ",
	     curr_login_num, MAXACTIVERUN, utmpshm->wwwguest);
	prints("×¢²áÓÃ»§Êý[\033[36m%d\033[32m]\n", uidshm->usersum);
	prints("ÏµÍ³³ÖÐøÔËÐÐ [\033[36m%dÌì%dÐ¡Ê±%d·ÖÖÓ\033[32m] ",
	       n / (3600 * 24), n % (3600 * 24) / 3600, n % 3600 / 60);
	prints("×î¸ßÈËÊý¼ÇÂ¼ [\033[36m%d\033[32m] ", utmpshm->maxuser);
	prints("±¾ÈÕ×î¸ßÈËÊý [\033[36m%d\033[32m]\n", utmpshm->maxtoday);
	prints
	    ("\033[mÊÔÓÃÇëÊäÈë '\033[1;36mguest\033[m', "
	     "×¢²áÇëÊäÈë '\033[1;31mnew\033[m', "
	     "add '.' after YourID to login for BIG5\n");
#ifndef SSHBBS
	attempts = 0;
	while (1) {
		if (attempts++ >= LOGINATTEMPTS) {
			ansimore("etc/goodbye", NA);
			refresh();
			exit(1);
		}
		move(t_lines - 1, 0);
		clrtoeol();
		getdata(t_lines - 1, 0, "ÇëÊäÈëÕÊºÅ: ",
			uid, IDLEN + 1, DOECHO, YEA);
		scroll();
		{
			int l = strlen(uid);
			if (l > 0 && uid[l - 1] == '.') {
				uid[l - 1] = 0;
				if (!convcode)
					switch_code();
			}
		}
		/* ppfoong */
		if ((strcasecmp(uid, "guest") == 0)
		    && (MAXACTIVE - curr_login_num < 10)) {
			ansimore("etc/loginfull", NA);
			refresh();
			exit(1);
		}

		if (strcmp(uid, "new") == 0) {
#ifdef LOGINASNEW
			memset(&currentuser, 0, sizeof (currentuser));
			new_register();
			ansimore("etc/firstlogin", YEA);
			break;
#else
			prints
			    ("[1;37m±¾ÏµÍ³Ä¿Ç°ÎÞ·¨ÒÔ [36mnew[37m ×¢²á, ÇëÓÃ[36m guest[37m ½øÈë...[m\n");
			scroll();
#endif
		} else if (*uid == '\0' || !dosearchuser(uid)) {
			move(t_lines - 1, 0);
			clrtoeol();
			prints("[1;31m´íÎóµÄÊ¹ÓÃÕßÕÊºÅ...[0m\n");
			scroll();
		} else if (strcasecmp(uid, "guest") == 0) {
			currentuser.userlevel = 0;
			break;
		} else if (userbansite(currentuser.userid, fromhost)) {
			prints("\033[1;31mÓÃ»§%sÒÑ¾­½ûÖ¹´Ó%s³¢ÊÔµÇÂ¼\033[0m\n",
			       currentuser.userid, fromhost);
			scroll();
		} else {
			if (!convcode)
				convcode =
				    !(currentuser.userdefine & DEF_USEGB);
			move(t_lines - 1, 0);
			clrtoeol();
			getdata(t_lines - 1, 0, "ÇëÊäÈëÃÜÂë: ",
				passbuf, PASSLEN, NOECHO, YEA);
			scroll();
			passbuf[8] = '\0';
			if (!checkpasswd(currentuser.passwd, passbuf)) {
				logattempt(currentuser.userid, fromhost, "",
					   now_t);
				move(t_lines - 1, 0);
				clrtoeol();
				prints("[1;31mÃÜÂëÊäÈë´íÎó...[m\n");
				scroll();
			} else {
				if (!HAS_PERM(PERM_BASIC)) {
					move(t_lines - 1, 0);
					clrtoeol();
					prints
					    ("[1;32m±¾ÕÊºÅÒÑÍ£»ú¡£ÇëÏò [36mSYSOP[32m ²éÑ¯Ô­Òò[m\n");
					refresh();
					sleep(5);
					exit(1);
				}
				if (simplepasswd(passbuf, 1)
				    || simplepasswd(passbuf, 2)
				    || strstr(passbuf, currentuser.userid)) {
					move(t_lines - 1, 0);
					clrtoeol();
					prints
					    ("[1;33m* ÃÜÂë¹ýÓÚ¼òµ¥, ÇëÑ¡ÔñÒ»¸öÒÔÉÏµÄÌØÊâ×ÖÔª.[m\n");
					scroll();
					move(t_lines - 1, 0);
					clrtoeol();
					getdata(t_lines - 1, 0,
						"-°´ <ENTER> ¼ÌÐø", genbuf, 5,
						NOECHO, YEA);
					scroll();
				}
				bzero(passbuf, PASSLEN - 1);
				break;
			}
		}
	}
#else
	if (userbansite(currentuser.userid, fromhost)) {
		move(t_lines - 1, 0);
		clrtoeol();
		prints("\033[1;31mÓÃ»§%sÒÑ¾­½ûÖ¹´Ó%s³¢ÊÔµÇÂ¼\033[0m\n",
		       currentuser.userid, fromhost);
		exit(0);
	}
	if (!HAS_PERM(PERM_BASIC)) {
					move(t_lines - 1, 0);
					clrtoeol();
					prints
					    ("[1;32m±¾ÕÊºÅÒÑÍ£»ú¡£ÇëÏò [36mSYSOP[32m ²éÑ¯Ô­Òò[m\n");
					refresh();
					sleep(5);
					exit(1);
				}
	move(t_lines - 1, 0);
	clrtoeol();
	prints("%s »¶Ó­ÄúÊ¹ÓÃssh·½Ê½·ÃÎÊ %s °´ [RETURN] ¼ÌÐø",
	       currentuser.userid, MY_BBS_NAME);
	*genbuf = egetch();
#endif
	multi_user_check();
	sprintf(buf, "home/%c/%s/%s.deadve", mytoupper(currentuser.userid[0]),
		currentuser.userid, currentuser.userid);
	if (dashf(buf)) {
		if (strcasecmp("guest", currentuser.userid))
			mail_file(buf, currentuser.userid,
				  "²»Õý³£¶ÏÏßËù±£ÁôµÄ²¿·Ý...");
		unlink(buf);
	}
	sethomepath(genbuf, currentuser.userid);
	mkdir(genbuf, 0775);
}

static void
direct_login()
{
	int randnum;
	fill_shmfile(5, "etc/endline", ENDLINE1_SHMKEY);
	resolve_utmp();
	resolve_ucache();
	if (uinfo.userid[0] == '\0' || !dosearchuser(uinfo.userid)) {
		prints("\x1b[1;31m´íÎóµÄÊ¹ÓÃÕßÕÊºÅ...\x1b[0m\n");
		refresh();
		exit(1);
	}
	if (strcasecmp(uinfo.userid, "guest")) {
		currentuser.userlevel = 0;
	} else {
		if (!HAS_PERM(PERM_BASIC)) {
			prints
			    ("\x1b[1;32m±¾ÕÊºÅÒÑÍ£»ú¡£ÇëÏò \x1b[36mSYSOP\x1b[32m ²éÑ¯[[m \n ");
			refresh();
			sleep(5);
			exit(1);
		}
	}
	sethomepath(genbuf, currentuser.userid);
	mkdir(genbuf, 0775);
	u_enter();
	started = 1;
	WishNum = 9999;
	strncpy(currentuser.lasthost, fromhost, 16);
	currentuser.lasthost[15] = '\0';	/* dumb mistake on my part */
	currentuser.lastlogin = time(NULL);
	if(uinfo.invisible){			//add by mintbaggio@BMY for normal cloak
		//currentuser.pseudo_lastlogout = currentuser.lastlogin+10;
		srand((unsigned)time(NULL));
		randnum=1+(int) (10000.0*rand()/(RAND_MAX+1.0));//add by bjgyt
		currentuser.lastlogout = currentuser.lastlogin+randnum;
	}else	currentuser.lastlogout = 0;
	set_safe_record();
	if (HAS_PERM(PERM_LOGINOK)) {
		FILE *fp;
		int tempnum;
		char fname[STRLEN];
		setuserfile(fname, "clubrights");
		if ((fp = fopen(fname, "r")) == NULL) {
			memset(&(uinfo.clubrights), 0, 4 * sizeof (int));
		} else {
			while (fgets(genbuf, STRLEN, fp) != NULL) {
				tempnum = atoi(genbuf);
				uinfo.clubrights[tempnum / 32] |=
				    (1 << tempnum % 32);
			}
			fclose(fp);
		}
	}
	if (strcmp(currentuser.userid, "SYSOP") == 0)
		currentuser.userlevel = ~0;	/* SYSOP gets all permission bits */

	if (strcmp(currentuser.userid, "beta") == 0)
               {
		 currentuser.userlevel = ~0;     /* SYSOP gets all permission bits */
		// currentuser.userlevel &= ~PERM_DENYMAIL;
		}
	substitute_record(PASSFILE, &currentuser, sizeof (currentuser),
			  usernum);

	login_start_time = time(0);
	m_init();
	RMSG = NA;
	nettyNN = NNread_init();
}

static void
notepad_init()
{
	FILE *check;
	char notetitle[STRLEN];
	char tmp[STRLEN * 2];
	char *fname, *bname, *ntitle;
	long int maxsec;
	time_t now;
	maxsec = 24 * 60 * 60;
	lastnote = 0;
	now = time(NULL);
	if ((check = fopen("etc/checknotepad", "r")) != NULL) {
		fgets(tmp, sizeof (tmp), check);
		lastnote = atol(tmp);
		fclose(check);
	} else
		lastnote = 0;
	if (lastnote == 0) {
		lastnote = now - (now % maxsec);
		check = fopen("etc/checknotepad", "w");
		fprintf(check, "%d", (int) lastnote);
		fclose(check);
	}
	if ((now - lastnote) >= maxsec) {
		move(t_lines - 1, 0);
		prints("¶Ô²»Æð£¬ÏµÍ³×Ô¶¯·¢ÐÅ£¬ÇëÉÔºò.....");
		refresh();
		check = fopen("etc/checknotepad", "w");
		lastnote = now - (now % maxsec);
		fprintf(check, "%d", (int) lastnote);
		fclose(check);
		if ((check = fopen("etc/autopost", "r")) != NULL) {
			while (fgets(tmp, STRLEN, check) != NULL) {
				fname = strtok(tmp, " \n\t:@");
				bname = strtok(NULL, " \n\t:@");
				ntitle = strtok(NULL, " \n\t:@");
				if (fname == NULL || bname == NULL
				    || ntitle == NULL) continue;
				else {
					sprintf(notetitle, "[%.10s] %s",
						ctime(&now), ntitle);
					if (dashf(fname)) {
						postfile(fname, bname,
							 notetitle, 1);
						sprintf(tmp,
							"system post %s %s",
							bname, ntitle);
						newtrace(tmp);
					}
				}
			}
			fclose(check);
		}
		sprintf(notetitle, "[%.10s] ÁôÑÔ°æ¼ÇÂ¼", ctime(&now));
		if (dashf("etc/notepad")) {
			postfile("etc/notepad", "notepad", notetitle, 1);
			unlink("etc/notepad");
		}
	}
	return;
}

static void
user_login()
{
	char fname[STRLEN];
	time_t dtime;
	int day;
	int randnum;
	struct tm *local1,*local2;
	int mon1,mon2,day1,day2,year1,year2;
	sprintf(genbuf, "%s enter %s", currentuser.userid, fromhost);
	newtrace(genbuf);
	u_enter();
	started = 1;

	//initscr() ;
	if (!currentuser.dietime && DEFINE(DEF_SEESTATINLOG)) {
#if USE_NOTEPAD
		notepad_init();
		if (strcmp(currentuser.userid, "guest") || DEFINE(DEF_NOTEPAD))
			shownotepad();
#endif

		ansimore("0Announce/bbslist/countusr", 1);
#if 0
		if ((rec_flag(NULL, '\0', 2 /*¼ì²é¶Á¹ýÐÂµÄWelcome Ã» */ ) == 0)
		    && DEFINE(DEF_SEEWELC1)) {
			if (dashf("Welcome")) {
				ansimore("Welcome", YEA);
				rec_flag(NULL, 'R',
					 2 /*Ð´Èë¶Á¹ýÐÂµÄWelcome */ );
			}
		} else {
			//ansimore("HappyNewYear",YEA);
			if (fill_shmfile(3, "Welcome2", WELCOME_SHMKEY))
				show_welcomeshm();
		}
#endif
		ansimore2("Welcome2", 1, 0, 24);
		if (DEFINE(DEF_FILTERXXX))
			ansimore("etc/dayf", 1);
		else
			ansimore("etc/posts/day", 1);
		ansimore("etc/posts/newsday", 1);
		ansimore("etc/posts/good10", 0);


		//add
		local1=localtime(&login_start_time);
		day1=local1->tm_mday;
		mon1=local1->tm_mon;
		year1=local1->tm_year;
		local2=localtime(&currentuser.firstlogin);
		day2=local2->tm_mday;
		mon2=local2->tm_mon;
		year2=local2->tm_year;
		if(mon1==mon2&&day1==day2&&year1!=year2)
			user_display("etc/birthday",1,YEA);
		//end add


		move(t_lines - 2, 0);
		prints
		    ("[1;36m¡î ÕâÊÇÄúµÚ [33m%d[36m ´Î°Ý·Ã±¾Õ¾£¬ÉÏ´ÎÄúÊÇ´Ó [33m%s[36m Á¬Íù±¾Õ¾¡£\n",
		     currentuser.numlogins + 1, currentuser.lasthost);
		/*landefeng@BMY add for ²¹È«ÀÏÓÃ»§×Ô¶¨ÒåËøÆÁ¹¦ÄÜ£¬Ê±¼äÒ»¸öÔÂ
		{
			currentuser.userlevel |= PERM_SELFLOCK;
			substitute_record(PASSFILE, &currentuser, sizeof (struct userec), getuser(currentuser.userid));
		}
		*/
		prints("¡î ÉÏ´ÎÁ¬ÏßÊ±¼äÎª [33m%s[m",
		       Ctime(currentuser.lastlogin));
		igetkey();
		if(DEFINE(DEF_NEWSTOP10))//add by bjgyt
			show_help("0Announce/bbslist/newsday");
	}
	WishNum = 9999;
	setuserfile(fname, BADLOGINFILE);
	if (ansimore(fname, NA) != -1) {
		char ans[3];
		getdata(t_lines - 1, 0,
			"ÈçºÎ´¦ÀíÒÔÉÏÃÜÂëÊäÈë´íÎó¼ÇÂ¼ (m) ÓÊ»ØÐÅÏä (r) Çå³ý (c) ¼ÌÐø [c]: ",
			ans, 2, DOECHO, YEA);
		if (ans[0] == 'm' || ans[0] == 'M') {
			char title[STRLEN];
			time_t now = time(NULL);
			sprintf(title, "[%12.12s] ÃÜÂëÊäÈë´íÎó¼ÇÂ¼",
				ctime(&now) + 4);
			mail_file(fname, currentuser.userid, title);
			have_msg_unread = 0;
			unlink(fname);
		} else if (ans[0] == 'r' || ans[0] == 'R') {
			unlink(fname);
		}
	}

	set_safe_record();
	strncpy(currentuser.lasthost, fromhost, 16);
	currentuser.lasthost[15] = '\0';	/* dumb mistake on my part */
	dtime = time(NULL) - 4 * 3600;
	day = localtime(&dtime)->tm_mday;
	dtime = currentuser.lastlogin - 4 * 3600;
	if (day > localtime(&dtime)->tm_mday && currentuser.numdays < 800)
		currentuser.numdays++;
	currentuser.lastlogin = time(NULL);
	if(uinfo.invisible){                     //add by mintbaggio@BMY for normal cloak
               //currentuser.pseudo_lastlogout = currentuser.lastlogin+10;
		 srand((unsigned)time(NULL));
		 randnum=1+(int) (10000.0*rand()/(RAND_MAX+1.0));//add by bjgyt
		 currentuser.lastlogout = currentuser.lastlogin+randnum;
        }else{
		currentuser.lastlogout = 0;
		//currentuser.pseudo_lastlogout = 0;
	}
	if (HAS_PERM(PERM_LOGINOK)) {
		FILE *fp;
		int tempnum;
		setuserfile(fname, "clubrights");
		if ((fp = fopen(fname, "r")) == NULL) {
			memset(&(uinfo.clubrights), 0, 4 * sizeof (int));
		} else {
			while (fgets(genbuf, STRLEN, fp) != NULL) {
				tempnum = atoi(genbuf);
				uinfo.clubrights[tempnum / 32] |=
				    (1 << tempnum % 32);
			}
			fclose(fp);
		}
	}
#ifdef REG_EXPIRED
	/* ppfoong - Ã¿ REG_EXPIRED ÌìÖØÐÂ½øÐÐÉí·ÝÈ·ÈÏ */
	if (HAS_PERM(PERM_LOGINOK) &&
	    strcmp(currentuser.userid, "SYSOP") &&
	    strcmp(currentuser.userid, "guest")) {
		struct stat st;
		time_t now;
		int expired = 0;
		now = time(0);
		setuserfile(fname, "mailcheck");
		if (stat(fname, &st) == -1
		    || now - st.st_mtime >= REG_EXPIRED * 86400) {
			setuserfile(fname, "register");
			if (stat(fname, &st) == 0) {
				if (now - st.st_mtime >= REG_EXPIRED * 86400) {
					setuserfile(fname, "register.old");
					if (stat(fname, &st) == -1
					    || now - st.st_mtime >=
					    REG_EXPIRED * 86400)
						expired = 1;
					else
						expired = 0;
				}
			} else
				expired = 1;	/*  Â©ÍøÖ®Óã??  */
		}
		if (expired) {
			strcpy(currentuser.email, "");
			strcpy(currentuser.address, "");
			currentuser.userlevel &= ~(PERM_LOGINOK | PERM_PAGE);
			mail_file("etc/expired", currentuser.userid,
				  "¸üÐÂ¸öÈË×ÊÁÏËµÃ÷¡£");
			setuserfile(fname, "sucessreg");
			unlink(fname);
		}
	}
#endif
	currentuser.numlogins++;
	if (strcmp(currentuser.userid, "SYSOP") == 0)
		{
 			currentuser.userlevel = ~0; /* SYSOP gets all permission bits */
			currentuser.userlevel &= ~PERM_DENYMAIL; //add by wjbta
			}
	substitute_record(PASSFILE, &currentuser,
			  sizeof (currentuser), usernum);
	if (currentuser.firstlogin == 0) {
		currentuser.firstlogin = login_start_time - 7 * 86400;
	}
	check_register_info();
}

void
set_numofsig()
{
	int sigln;
	char signame[STRLEN];
	setuserfile(signame, "signatures");
	sigln = countln(signame);
	numofsig = sigln / MAXSIGLINES;
	if ((sigln % MAXSIGLINES) != 0)
		numofsig += 1;
}

static int
chk_friend_book()
{
	FILE *fp;
	int idnum, n = 0;
	char buf[STRLEN], *ptr;
	if ((fp = fopen("friendbook", "r")) == NULL)
		return n;
	move(5, 0);
	prints("[1mÏµÍ³Ñ°ÈËÃû²áÁÐ±í:[m\n\n");
	while (fgets(buf, sizeof (buf), fp) != NULL) {
		char uid[14];
		char msg[STRLEN];
		struct user_info *uin;
		ptr = strstr(buf, "@");
		if (ptr == NULL)
			continue;
		ptr++;
		strcpy(uid, ptr);
		ptr = strstr(uid, "\n");
		*ptr = '\0';
		idnum = atoi(buf);
		if (idnum != usernum || idnum <= 0)
			continue;
		uin = t_search(uid, NA, 1);
		sprintf(msg, "%s ÒÑ¾­ÉÏÕ¾¡£", currentuser.userid);
		if (!uinfo.invisible && uin != NULL && !DEFINE(DEF_NOLOGINSEND)
		    && do_sendmsg(uin->userid, uin, msg, 2, uin->pid) == 1) {
			prints
			    ("[1m%s[m ÕÒÄã£¬ÏµÍ³ÒÑ¾­¸æËßËûÄãÉÏÕ¾µÄÏûÏ¢¡£\n",
			     uid);
		} else
			prints
			    ("[1m%s[m ÕÒÄã£¬ÏµÍ³ÎÞ·¨ÁªÂçµ½Ëû£¬ÇëÄã¸úËûÁªÂç¡£\n",
			     uid);
		n++;
		del_from_file("friendbook", buf);
		if (n > 15) {
			pressanykey();
			move(7, 0);
			clrtobot();
		}
	}
	fclose(fp);
	return n;
}

static int
getinput_intime(unsigned char *buf, int len, int timeout)
{
	fd_set rfds;
	struct timeval tv;
	int retval, n, r;
	n = 0;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	//such code is only suit for linux,it is not portable for other unix like system.only linux
	//will change the value of tv as we hope.
	while (1) {
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		retval = select(1, &rfds, NULL, NULL, &tv);
		if (retval > 0) {
			r = read(0, buf + n, len - n);
			if (r <= 0)
				return 0;
			n += r;
			if (n >= 9 && buf[n - 9] == 255
			    && buf[n - 8] == 250 && buf[n - 7] == 31
			    && buf[n - 2] == 255 && buf[n - 1] == 240) {
				t_lines = buf[n - 3];
				t_columns = buf[n - 5];
				if (t_lines < 24 || t_lines > 100)
					t_lines = 24;
				if (t_columns < 80 || t_columns > LINELEN)
					t_columns = 80;
				return 1;
			}
			if (n >= len)
				return 0;
		} else
			return 0;
	}
}

static int
check_tty_lines()
{				/* dii.nju.edu.cn  zhch  2000.4.11 */
	unsigned char buf1[] = {
		255, 253, 31
	};
#ifdef SSHBBS
	return 0;
#else
	unsigned char buf2[100];
	write(0, buf1, 3);
	return getinput_intime(buf2, 80, 1);
	return 0;
#endif
}

#ifndef SSHBBS
int
main(argc, argv)
int argc;
char **argv;
#else
int
bbs_entry(argc, argv)
int argc;
char *argv[];
#endif
{
	char fname[STRLEN];
	umask(0007);
	time(&now_t);
	signal(SIGALRM, SIG_DFL);
	alarm(LOGIN_TIMEOUT);
	check_tty_lines();
	term_init();
	initscr();
	srand(time(NULL) + getpid());
	load_sysconf();
	conv_init();
	if (argc < 2 || ((*argv[1] != 'h') && (*argv[1] != 'e')
			 && (*argv[1] != 'd'))) {
		prints("You cannot execute this program directly.\n");
		refresh();
		exit(-1);
	}
	if (!strstr(argv[0], "bbstest"))
		runtest = 0;
	else
		runtest = 1;
#ifdef SSHBBS
	runssh = 1;
#endif
	if (*argv[1] == 'e')
		convcode = 1;
	system_init(argc, argv);
	if (setjmp(byebye)) {
		abort_bbs();
	}
	set_cpu_limit(MAXACTIVERUN / (2 * 2));	//µÚÒ»¸ö2±íÊ¾Á½cpu,µÚ¶þ¸öÊÇÎªÁË¸øµãÓàÁ¿
	init_tty();
	if (argc == 4) {
		prints("pid without username\n");
		refresh();
		exit(-1);
	}
	if (argc >= 5) {
		autologin = 1;
		utmpent = atoi(argv[3]);
		strncpy(uinfo.userid, argv[4], IDLEN + 1);
		uinfo.userid[IDLEN + 1] = 0;
		direct_login();
	} else {
		login_query();
		user_login();
		login_start_time = time(0);
		m_init();
		RMSG = NA;
		clear();
//#ifdef TALK_LOG
		tlog_recover();	/* 990713.edwardc for talk_log recover */
//#endif
		if (strcmp(currentuser.userid, "guest")) {
			if (HAS_PERM(PERM_ACCOUNTS)
			    && dashf("new_register")) {
				prints
				    ("[1;33mÓÐÐÂÊ¹ÓÃÕßÕýÔÚµÈÄúÍ¨¹ý×¢²á×ÊÁÏ¡£[m");
				pressanykey();
				clear();
			}
			if (chk_friend_book())
				pressanykey();
			move(7, 0);
			clrtobot();
			if (!DEFINE(DEF_NOLOGINSEND))
				if (!uinfo.invisible)
					apply_ulist(friend_login_wall);
/*       pressanykey();
       clear();*/
			set_numofsig();
			if (DEFINE(DEF_INNOTE)) {
				setuserfile(fname, "notes");
				if (dashf(fname))
					ansimore(fname, YEA);
			}
		}
		nettyNN = NNread_init();
		b_closepolls();
		if ((currentuser.userlevel & PERM_BOARDS)) {
			setbmstatus(1);
			bmfilesync(&currentuser);
		}
		num_alcounter();
		if (count_friends > 0 && DEFINE(DEF_LOGFRIEND))
			t_friends();
		loaduserkeys();
		if ((!(currentuser.userlevel & PERM_LOGINOK))
		    && strcmp("guest", currentuser.userid)
		    && strcmp("SYSOP", currentuser.userid)) {
			x_fillform();
		}
	}
	if (strcmp(currentuser.userid, "guest")) {
		r_msg();
	}
	while (1) {
		if (inprison)
			domenu("PRISONMENU");
		else if (currentuser.dietime)
			domenu("DIEMENU");
		else {
			if (DEFINE(DEF_NORMALSCR))
				domenu("TOPMENU");
			else
				domenu("TOPMENU2");
		}
		Goodbye();
	}
}

int refscreen = NA;
int
egetch()
{
	int rval;
	check_calltime();
	if (talkrequest) {
		talkreply();
		refscreen = YEA;
		return -1;
	}
/*    if (ntalkrequest) {
        ntalkreply() ;
        refscreen = YEA ;
        return -1 ;
    } */
	while (1) {
		rval = igetkey();
		if (talkrequest) {
			talkreply();
			refscreen = YEA;
			return -1;
		}
/*        if(ntalkrequest) {
            ntalkreply() ;
            refscreen = YEA ;
            return -1 ;
        } */
		if (rval != Ctrl('L'))
			break;
		redoscr();
	}
	refscreen = NA;
	return rval;
}

static char *
boardmargin()
{
	static char buf[STRLEN];
	if (selboard)
		sprintf(buf, "ÌÖÂÛÇø [%s]", currboard);
	else {
		brc_initial(DEFAULTBOARD, 0);
		strcpy(currboard, DEFAULTBOARD);
		if (getbnum(currboard)) {
			selboard = 1;
			sprintf(buf, "ÌÖÂÛÇø [%s]", currboard);
		} else
			sprintf(buf, "Ä¿Ç°²¢Ã»ÓÐÉè¶¨ÌÖÂÛÇø");
	}
	return buf;
}

int endlineoffset = 0;
/*Add by SmallPig*/
void
update_endline()
{
	char buf[STRLEN], fname[STRLEN], *ptr;
	time_t now;
	FILE *fp;
	int allstay, i, foo, foo2;
	static int linetype = 0;
	if (!DEFINE(DEF_ENDLINE)) {
		move(t_lines - 1 + endlineoffset, 0);
		clrtoeol();
		return;
	}

	move(t_lines - 1 + endlineoffset, 0);
	clrtoeol();
	if (currentuser.userdefine & DEF_ANIENDLINE) {
		linetype = !linetype;
		if (linetype) {
			if (show_endline())
				return;
		}
	}

	now = time(0);
	allstay = now % 9;
	if (allstay < 5) {
	      nowishfile:
		sprintf(buf, "[\033[36m%.12s\033[33m]", currentuser.userid);
		num_alcounter();
		prints
		    ("[1;44;33mÊ±¼ä:[[36m%16s[33m] ÔÚÏß/ÅóÓÑ:[[36m%4d[33m/[1;36m%3d[33m] ×´Ì¬:[[36m%1s%1s%1s%1s%1s%1s[33m] Ê¹ÓÃÕß:%-s[m",
		     ctime(&now), count_users, count_friends,
		     (uinfo.pager & ALL_PAGER) ? "P" : "p",
		     (uinfo.pager & FRIEND_PAGER) ? "O" : "o",
		     (uinfo.pager & ALLMSG_PAGER) ? "M" : "m",
		     (uinfo.pager & FRIENDMSG_PAGER) ? "F" : "f",
		     (DEFINE(DEF_MSGGETKEY)) ? "X" : "x",
		     (uinfo.invisible == 1) ? "C" : "c", buf);
		return;
	}
	setuserfile(fname, "HaveNewWish");
	if (WishNum == 9999 || dashf(fname)) {
		if (WishNum != 9999)
			unlink(fname);
		WishNum = 0;
		orderWish = 0;
		setuserfile(fname, "GoodWish");
		if ((fp = fopen(fname, "r")) != NULL) {
			for (; WishNum < 20;) {
				if (fgets(buf, STRLEN - 1, fp) == NULL)
					break;
				buf[STRLEN - 4] = '\0';
				ptr = strtok(buf, "\n\r");
				if (ptr == NULL || ptr[0] == '#'
				    || ptr[0] == '\n') continue;
				strcpy(buf, ptr);
				for (ptr = buf; *ptr == ' ' && *ptr != 0;
				     ptr++) ;
				if (*ptr == 0 || ptr[0] == '#')
					continue;
				for (i = strlen(ptr) - 1; i < 0; i--)
					if (ptr[i] != ' ')
						break;
				if (i < 0)
					continue;
				foo = strlen(ptr);
				foo2 = (STRLEN - 3 - foo) / 2;
				strcpy(GoodWish[WishNum], "");
				for (i = 0; i < foo2; i++)
					strcat(GoodWish[WishNum], " ");
				strcat(GoodWish[WishNum], ptr);
				for (i = 0; i < STRLEN - 3 - (foo + foo2); i++)
					strcat(GoodWish[WishNum], " ");
				GoodWish[WishNum][STRLEN - 4] = '\0';
				WishNum++;
			}
			fclose(fp);
		}
	}
	if (WishNum == 0)
		goto nowishfile;
	if (orderWish >= WishNum * 2)
		orderWish = 0;
	prints("[0;1;44;33m[[36m%s[33m][m", GoodWish[orderWish / 2]);
	orderWish++;
}

/*ReWrite by SmallPig*/
void
showtitle(title, mid)
char *title, *mid;
{
	char buf[STRLEN], *note;
	int spc1, spc2;
	note = boardmargin();
	spc1 = 39 - num_noans_chr(title) - strlen(mid) / 2;
	spc2 = 40 - strlen(note) - strlen(mid) + strlen(mid) / 2;	//Ö±½Ó¼õstrlen(mid)/2ÓÐÕû³ýÎÊÌâ
	if (spc1 < 2) {
		spc2 -= 2 - spc1;	//ecnegrevid: °æÃæ±êÌâÐÐÌ«¿íÏÔÊ¾²»ºÃ
		spc1 = 2;
	}
	if (spc2 < 2) {
		note[strlen(note) - (2 - spc2)] = 0;
		spc2 = 2;
	}
	move(0, 0);
	clrtoeol();
	sprintf(buf, "%*s", spc1, "");
	if (!strcmp(mid, MY_BBS_NAME))
		prints("[1;44;33m%s%s[37m%s[1;44m", title, buf, mid);
	else if (mid[0] == '[')
		prints("[1;44;33m%s%s[5;36m%s[m[1;44m", title, buf, mid);
	else
		prints("[1;44;33m%s%s[36m%s", title, buf, mid);
	sprintf(buf, "%*s", spc2, "");
	prints("%s[33m%s[m\n", buf, note);
	//update_endline(); //´ÓÀ´¶¼ÊÇ»­ÁËÈ»ºóÇåµô,ºÎ±Ø»­Ö® ylsdd
	move(1, 0);
}

void
docmdtitle(title, prompt)
char *title, *prompt;
{
	char *middoc;
	if (chkmail()) {
		if (!strncmp("[ÌÖÂÛÇøÁÐ±í]", title, 12))
			middoc = "[ÄúÓÐÐÅ¼þ,Çë°´ w ²é¿´ÐÅ¼þ]";
		else
			middoc = "[ÄúÓÐÐÅ¼þ]";
	} else
		middoc = MY_BBS_NAME;
	showtitle(title, middoc);
	move(1, 0);
	clrtoeol();
	prints("%s", prompt);
	clrtoeol();
}

static void
R_endline(int signum)
{
	signal(SIGTTOU, R_endline);
	if (!can_R_endline)
		return;
	if (uinfo.mode != READBRD
	    && uinfo.mode != READNEW
	    && uinfo.mode != SELECT
	    && uinfo.mode != LUSERS
	    && uinfo.mode != FRIEND
	    && uinfo.mode != READING
	    && uinfo.mode != RMAIL && uinfo.mode != DIGEST) return;
/*---------
    if (uinfo.mode != READBRD
        &&uinfo.mode != READNEW
        &&uinfo.mode != SELECT
        &&uinfo.mode != LUSERS
        &&uinfo.mode != FRIEND
        &&!(uinfo.mode==READING&&can_R_endline)
        &&!(uinfo.mode==RMAIL&&can_R_endline)
        &&!(uinfo.mode==DIGEST&&can_R_endline) )
        return;
----------*/
	update_endline();
}

//#ifdef TALK_LOG
static void
tlog_recover()
{
	char buf[256];
	sprintf(buf, "home/%c/%s/talk_log",
		toupper(currentuser.userid[0]), currentuser.userid);
	if (strcasecmp(currentuser.userid, "guest") == 0 || !dashf(buf))
		return;
	clear();
	strcpy(genbuf, "");
	getdata(0, 0,
		"[1;32mÄúÓÐÒ»¸ö²»Õý³£¶ÏÏßËùÁôÏÂÀ´µÄÁÄÌì¼ÇÂ¼, ÄúÒª .. (M) ¼Ä»ØÐÅÏä (Q) ËãÁË£¿[Q]£º[m",
		genbuf, 2, DOECHO, YEA);
	if (genbuf[0] == 'M' || genbuf[0] == 'm') {
		mail_file(buf, currentuser.userid, "ÁÄÌì¼ÇÂ¼");
	}
	unlink(buf);
}

//#endif

void
relogin()
{
	char bbs_prog_path[STRLEN], bbstest_prog_path[STRLEN];
	char utmppos[STRLEN];
	int n;
	sprintf(utmppos, "%d", utmpent);
	sprintf(bbs_prog_path, "%s/bin/bbs", MY_BBS_HOME);
	sprintf(bbstest_prog_path, "%s/bin/bbstest", MY_BBS_HOME);
	for (n = 1; n < 10; n++)
		close(n);
	if (convcode) {
		if (!runtest)
			execl(bbs_prog_path, "bbs", "e", currentuser.lasthost,
			      utmppos, currentuser.userid, NULL);	/*µ÷ÓÃBBS */
		else
			execl(bbstest_prog_path, "bbstest", "e",
			      currentuser.lasthost, utmppos, currentuser.userid, NULL);	/*µ÷ÓÃBBS */
	} else {
		if (!runtest)
			execl(bbs_prog_path, "bbs", "d", currentuser.lasthost,
			      utmppos, currentuser.userid, NULL);	/*µ÷ÓÃBBS */
		else
			execl(bbstest_prog_path, "bbstest", "d",
			      currentuser.lasthost, utmppos, currentuser.userid, NULL);	/*µ÷ÓÃBBS */
	}
}

/* youzi quick goodbye */
int
Q_Goodbye()
{
	extern int started;
	time_t stay;
	char fname[STRLEN], notename[STRLEN];
	int logouts, mylogout = NA;
	clear();
	prints("\n\n\n\n");
	if (DEFINE(DEF_OUTNOTE)) {
		setuserfile(notename, "notes");
		if (dashf(notename))
			ansimore(notename, YEA);
	}
	if (DEFINE(DEF_LOGOUT)) {
		setuserfile(fname, "logout");
		if (dashf(fname))
			mylogout = YEA;
	}
	if (mylogout) {
		logouts = countlogouts(fname);
		if (logouts >= 1) {
			user_display(fname, (logouts == 1) ? 1 :
				     (currentuser.numlogins % (logouts)) + 1,
				     YEA);
		}
	} else {
		if (fill_shmfile(2, "etc/logout", GOODBYE_SHMKEY)) {
			show_goodbyeshm();
		}
	}

	stay = time(NULL) - login_start_time;
	if (started) {
		sprintf(genbuf, "%s exitbbs %ld", currentuser.userid, stay);
		newtrace(genbuf);
		if ((currentuser.userlevel & PERM_BOARDS)
		    && (count_uindex(usernum) == 1))
			setbmstatus(0);
		u_exit();
	}
	set_safe_record();
	currentuser.stay += stay;
	currentuser.lastlogout = time(NULL);
	if (inprison)
		currentuser.dietime = currentuser.dietime - 1;
	if (currentuser.stay > currentuser.dietime && currentuser.dietime != 0)
		currentuser.dietime = 2;
	substitute_record(PASSFILE, &currentuser, sizeof (currentuser),
			  usernum);
	pressreturn();
	if (strcmp(currentuser.userid, "guest") && count_uindex(usernum) == 0) {
		FILE *fp;
		char buf[STRLEN], *ptr;
		if ((fp = fopen("friendbook", "r")) != NULL) {
			while (fgets(buf, sizeof (buf), fp) != NULL) {
				char uid[14];
				ptr = strstr(buf, "@");
				if (ptr == NULL) {
					del_from_file("friendbook", buf);
					continue;
				}
				ptr++;
				strcpy(uid, ptr);
				ptr = strstr(uid, "\n");
				*ptr = '\0';
				if (!strcmp(uid, currentuser.userid))
					del_from_file("friendbook", buf);
			}
			fclose(fp);
		}
	}
	sleep(1);
	started = 0;
	exit(0);
}

