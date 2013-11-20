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
    Copyright (C) 1999	KCN,Zhou lin,kcn@cic.tsinghua.edu.cn

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
#define M_INT 8			/* monitor mode update interval */
#define P_INT 20		/* interval to check for page req. in talk/chat */

int talkidletime = 0;
int ulistpage;
int friendflag = 1;
//#ifdef TALK_LOG
int talkrec = -1;
char partner[IDLEN + 1];
//#endif

struct talk_win {
	int curcol, curln;
	int sline, eline;
};

int nowmovie;

static char *const refuse[] = {
	"±ß«∏£¨Œ“œ÷‘⁄œÎ◊®–ƒø¥ Board°£    ", "Œ“ΩÒÃÏ∫‹¿€£¨≤ªœÎ∏˙±»À¡ƒÃÏ°£    ",
	"Œ“œ÷‘⁄”– ¬£¨µ»“ªœ¬‘Ÿ Call ƒ„°£  ", "Œ“¬Ì…œ“™¿Îø™¡À£¨œ¬¥Œ‘Ÿ¡ƒ∞…°£    ",
	"«Îƒ„≤ª“™‘Ÿ Page£¨Œ“≤ªœÎ∏˙ƒ„¡ƒ°£ ", "«Îœ»–¥“ª∑‚◊‘Œ“ΩÈ…‹∏¯Œ“£¨∫√¬£ø  ",
	"∂‘≤ª∆£¨Œ“œ÷‘⁄‘⁄µ»»À°£          ", "«Î≤ª“™≥≥Œ“£¨∫√¬£ø..... :)      ",
	NULL
};

char save_page_requestor[STRLEN];

static char canpage(int friend, int pager);
static int listcuent(struct user_info *uentp);
static int show_user_plan(char *userid);
static int bm_printboard(struct boardmanager *bm, void *farg);
static int count_active(struct user_info *uentp);
static int count_useshell(struct user_info *uentp);
static int cmpfnames(char *userid, struct override *uv);
static int cmpunums(int unum, struct user_info *up);
static int cmpmsgnum(int unum, struct user_info *up);
static int setpagerequest(int mode);
static void do_talk_nextline(struct talk_win *twin);
static void do_talk_char(struct talk_win *twin, int ch);
static void talkflush(void);
static void moveto(int mode, struct talk_win *twin);
static int do_talk(int fd);
static int override_title(void);
static char *override_doentry(int ent, struct override *fh, char buf[512]);
static int override_edit(int ent, struct override *fh, char *direc);
static int override_dele(int ent, struct override *fh, char *direct);
static int friend_edit(int ent, struct override *fh, char *direct);
static int friend_add(int ent, struct override *fh, char *direct);
static int friend_dele(int ent, struct override *fh, char *direct);
static int friend_mail(int ent, struct override *fh, char *direct);
static int friend_query(int ent, struct override *fh, char *direct);
static int friend_help(void);
static int reject_edit(int ent, struct override *fh, char *direct);
static int reject_add(int ent, struct override *fh, char *direct);
static int reject_dele(int ent, struct override *fh, char *direct);
static int reject_query(int ent, struct override *fh, char *direct);
static int reject_help(void);
static int cmpfuid(unsigned *a, unsigned *b);
static void do_log(char *msg, int who);
static char *Cdate(time_t * clock);

struct one_key friend_list[] = {
	{'r', friend_query, "≤È—Ø∫√”—"},
	{'m', friend_mail, "∏¯∫√”—–¥–≈"},
	{'M', friend_mail, "∏¯∫√”—–¥–≈"},
	{'a', friend_add, "ÃÌº”∫√”—"},
	{'A', friend_add, "ÃÌº”∫√”—"},
	{'d', friend_dele, "…æ≥˝∫√”—"},
	{'D', friend_dele, "…æ≥˝∫√”—"},
	{'E', friend_edit, "±‡º≠∫√”—"},
	{'h', friend_help, "≤Èø¥∞Ô÷˙"},
	{'H', friend_help, "≤Èø¥∞Ô÷˙"},
	{'\0', NULL, ""}
};

struct one_key reject_list[] = {
	{'r', reject_query, "≤È—Øªµ»À"},
	{'a', reject_add, "ÃÌº”ªµ»À"},
	{'A', reject_add, "‘ˆº”ªµ»À"},
	{'d', reject_dele, "…æ≥˝ªµ»À"},
	{'D', reject_dele, "…æ≥˝ªµ»À"},
	{'E', reject_edit, "±‡º≠ªµ»À"},
	{'h', reject_help, "≤Èø¥∞Ô÷˙"},
	{'H', reject_help, "≤Èø¥∞Ô÷˙"},
	{'\0', NULL, ""}
};

#if 0
int
ishidden(user)
char *user;
{
	int tuid;
	struct user_info uin;

	if (!(tuid = searchuser(user)))
		return 0;
	if (!search_ulist(&uin, t_cmpuids, tuid))
		return 0;
	return (uin.invisible);
}
#endif

static char
canpage(friend, pager)
int friend, pager;
{
	if ((pager & ALL_PAGER) || HAS_PERM(PERM_SYSOP | PERM_FORCEPAGE))
		return YEA;
	if ((pager & FRIEND_PAGER)) {
		if (friend)
			return YEA;
	}
	return NA;
}

static int
listcuent(uentp)
struct user_info *uentp;
{
	if (uentp == NULL) {
		CreateNameList();
		return 0;
	}
	if (uentp->uid == usernum)
		return 0;
	if (!uentp->active || !uentp->pid || isreject(uentp))
		return 0;
	if (!HAS_PERM(PERM_SYSOP | PERM_SEECLOAK) && uentp->invisible)
		return 0;
	AddNameList(uentp->userid);
	return 0;
}

void
creat_list()
{
	listcuent(NULL);
	apply_ulist(listcuent);
}

int
t_pager()
{

	if (uinfo.pager & ALL_PAGER) {
		uinfo.pager &= ~ALL_PAGER;
		if (DEFINE(DEF_FRIENDCALL))
			uinfo.pager |= FRIEND_PAGER;
		else
			uinfo.pager &= ~FRIEND_PAGER;
	} else {
		uinfo.pager |= ALL_PAGER;
		uinfo.pager |= FRIEND_PAGER;
	}

	if (!uinfo.in_chat && uinfo.mode != TALK) {
		move(1, 0);
		prints("ƒ˙µƒ∫ÙΩ–∆˜ (pager) “—æ≠[1m%s[m¡À!",
		       (uinfo.pager & ALL_PAGER) ? "¥Úø™" : "πÿ±’");
		pressreturn();
	}
	update_utmp();
	return 0;
}

/*Add by SmallPig*/
/*¥À∫Ø ˝÷ª∏∫‘¡–”°Àµ√˜µµ£¨≤¢≤ªπ‹«Â≥˝ªÚ∂®ŒªµƒŒ Ã‚°£*/
static int
show_user_plan(userid)
char *userid;
{
	int i;
	char pfile[STRLEN], pbuf[256];
	FILE *pf;

	sethomefile(pfile, userid, "plans");
	if ((pf = fopen(pfile, "r")) == NULL) {
		prints("[1;36m√ª”–∏ˆ»ÀÀµ√˜µµ[m\n");
		return NA;
	} else {
		prints("[1;36m∏ˆ»ÀÀµ√˜µµ»Áœ¬£∫[m\n");
		for (i = 1; i <= MAXQUERYLINES; i++) {
			if (fgets(pbuf, sizeof (pbuf), pf)) {
				disable_move = 1;
				outns(pbuf, sizeof (pbuf));
				disable_move = 0;
			} else
				break;
		}
		fclose(pf);
		return YEA;
	}
}

/* Modified By Excellent*/
int
t_query(q_id)
char q_id[IDLEN + 2];
{
	char uident[STRLEN];
	int tuid = 0;
	int exp, perf;		/*Add by SmallPig */
	char qry_mail_dir[STRLEN];
	char path[STRLEN];
	char planid[IDLEN + 2];
	char expbuf[10];  //add for displaying exp type. rbb@bmy

	if ((uinfo.mode != LUSERS && uinfo.mode != LAUSERS
	     && uinfo.mode != FRIEND && uinfo.mode != READING
	     && uinfo.mode != MAIL && uinfo.mode != RMAIL && uinfo.mode != GMENU
	     && uinfo.mode != BACKNUMBER && uinfo.mode != DO1984)
	    || q_id == NULL) {
		modify_user_mode(QUERY);
		move(1, 0);
		clrtobot();
		prints("≤È—ØÀ≠:\n< ‰»Î π”√’ﬂ¥˙∫≈, ∞¥ø’∞◊º¸ø…¡–≥ˆ∑˚∫œ◊÷¥Æ>\n");
		move(1, 8);
		usercomplete(NULL, uident);
		if (uident[0] == '\0') {
			return 0;
		}
	} else {
		if (*q_id == '\0')
			return 0;
		if (strchr(q_id, ' '))
			strtok(q_id, " ");
		strncpy(uident, q_id, sizeof (uident));
		uident[sizeof (uident) - 1] = '\0';
	}
	if (!(tuid = getuser(uident))) {
		move(2, 0);
		clrtoeol();
		prints("[1m≤ª’˝»∑µƒ π”√’ﬂ¥˙∫≈[m\n");
		pressanykey();
		return -1;
	}
	uinfo.destuid = tuid;
	update_utmp();

	move(1, 0);
	clrtobot();
	sprintf(qry_mail_dir, "mail/%c/%s/%s", mytoupper(lookupuser.userid[0]),
		lookupuser.userid, DOT_DIR);

	exp = countexp(&lookupuser);
	perf = countperf(&lookupuser);
	strcpy(expbuf,cexp(exp));	//add for displaying exp type.  rbb@bmy
	prints
	    ("[1m%s [m([1m%s[m) π≤…œ’æ [1;33m%d[m ¥Œ£¨∑¢±Ìπ˝ [1;33m%d[m ∆™Œƒ’¬",
	     lookupuser.userid, lookupuser.username, lookupuser.numlogins,
	     lookupuser.numposts);
	show_special(lookupuser.userid); //add by wjbta@bmy
	strcpy(planid, lookupuser.userid);
	strcpy(genbuf,
	       lookupuser.dietime ? Ctime(lookupuser.dietime) :
	       Ctime(lookupuser.lastlogin));
	if (ifinprison(lookupuser.userid)) {
		strcpy(genbuf, Ctime(lookupuser.lastlogin));
		prints("\n‘⁄º‡”¸∑˛–Ã£¨…œ¥Œ∑≈∑Á ±º‰[\033[1m%s\033[m]\n", genbuf);
		if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS
		    && uinfo.mode != FRIEND && uinfo.mode != GMENU)
			pressanykey();
		uinfo.destuid = 0;
		return 0;

	}

	if (lookupuser.dietime) {
		prints
		    ("\n“—æ≠¿Îø™¡À»À ¿,ŒÿŒÿ...\nªπ”– [\033[1m%d\033[m] ÃÏæÕ“™◊™ ¿Õ∂Ã•¡À\n",
		     countlife(&lookupuser));
		if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS
		    && uinfo.mode != FRIEND && uinfo.mode != GMENU)
			pressanykey();
		uinfo.destuid = 0;
		return 0;
	}
	prints("\n…œ¥Œ‘⁄ [[1;32m%s[m] ¥” [[1;32m%s[m] µΩ±æ’æ“ª”Œ°£\n",
	genbuf,  (lookupuser.lasthost[0] ==
		'\0' ? "(≤ªœÍ)" : lookupuser.lasthost));
//	show_special(lookupuser.userid); //add by wjbta@bmy
	if(HAS_PERM(PERM_SYSOP | PERM_SEECLOAK)){		//add by mintbaggio@BMY
		if(lookupuser.userlevel&PERM_CLOAK)
			strcpy(genbuf, (lookupuser.lastlogout>=lookupuser.lastlogin)? (user_isonline(lookupuser.userid)?"“Ú‘⁄œﬂ…œªÚ≤ª’˝≥£∂œœﬂ≤ªœÍ":Ctime(lookupuser.lastlogout)) : "“Ú‘⁄œﬂ…œªÚ≤ª’˝≥£∂œœﬂ≤ªœÍ");
		else
			strcpy(genbuf, (lookupuser.lastlogout>=lookupuser.lastlogin) ? Ctime(lookupuser.lastlogout) : "“Ú‘⁄œﬂ…œªÚ≤ª’˝≥£∂œœﬂ≤ªœÍ");
	}
	else
		strcpy(genbuf, (lookupuser.lastlogout>=lookupuser.lastlogin) ? Ctime(lookupuser.lastlogout) : "“Ú‘⁄œﬂ…œªÚ≤ª’˝≥£∂œœﬂ≤ªœÍ");
/*	if(testonline(lookupuser.userid))
		strcpy(genbuf, "“Ú‘⁄œﬂ…œªÚ≤ª’˝≥£∂œœﬂ≤ªœÍ");
	else	strcpy(genbuf, Ctime(lookupuser.lastlogout));
*/
	prints
	    ("¿Î’æ ±º‰£∫[\033[1;36m%s\033[m] –≈œ‰£∫[[1;5m%2s[m]£¨…˙√¸¡¶£∫[[1;32m%d[m] Õ¯¡‰£∫[[1;32m%d[m]ÃÏ°£\n",
	     genbuf, (check_query_mail(qry_mail_dir) == 1) ? "°—" : "  ",
	     countlife(&lookupuser),
	     (strcmp(lookupuser.userid, "SYSOP") == 0) ? 9999 : ((time(0)-lookupuser.firstlogin)/86400));//by bjgyt add networkage
	if (lookupuser.userlevel & (PERM_BOARDS | PERM_ARBITRATE | PERM_OBOARDS | PERM_SYSOP | PERM_ACCOUNTS | PERM_WELCOME | PERM_SPECIAL7 | PERM_SPECIAL4)) {
		prints("µ£»Œ∞ÊŒÒ£∫");
		sethomefile(path, lookupuser.userid, "mboard");
		new_apply_record(path, sizeof (struct boardmanager),
				 (void *) bm_printboard, NULL);
	//	show_special(lookupuser.userid); //add by wjbta@bmy
	if (lookupuser.userlevel & !strcmp(currentuser.userid, "SYSOP")) prints("[[1;36mœµÕ≥π‹¿Ì‘±[m]");
	else if (lookupuser.userlevel & !strcmp(currentuser.userid, "lanboy")) prints("[[1;36mœµÕ≥π‹¿Ì‘±[m]");
	else if ((lookupuser.userlevel&PERM_SYSOP) && (lookupuser.userlevel&PERM_ARBITRATE) )	prints("[[1;36m±æ’æπÀŒ Õ≈[m]");
	else if (lookupuser.userlevel & PERM_SYSOP)	prints("[[1;36mœ÷»Œ’æ≥§[m]");
	else if (lookupuser.userlevel & PERM_OBOARDS)   prints("[[1;36m µœ∞’æ≥§[m]");
	else if (lookupuser.userlevel & PERM_ARBITRATE)	prints("[[1;36mœ÷»ŒºÕŒØ[m]");
	else if (lookupuser.userlevel & PERM_SPECIAL4)	prints("[[1;36m«¯≥§[m]");
	else if (lookupuser.userlevel & PERM_WELCOME) prints("[[1;36mœµÕ≥√¿π§[m]");
	else if (lookupuser.userlevel & PERM_SPECIAL7)
	{
		// ¥Û–≈œ‰&&!“˛…Ì
		if ( (lookupuser.userlevel & PERM_SPECIAL1) && !(lookupuser.userlevel & PERM_CLOAK) ) 
			prints("[[1;36m¿Î»Œ≥Ã–Ú‘±[m]");
		else
			prints("[[1;36m≥Ã–Ú◊È≥…‘±[m]");
	}
	else if (lookupuser.userlevel & PERM_ACCOUNTS) prints ("[[1;36m’ ∫≈π‹¿Ì‘±[m]");
	prints(" ");
	}
	prints("º∂±£∫[[1;36m%s[m]",expbuf); //add for displaying exp type rbb@bmy
	prints ("%s","                                              ");
	prints("\n");
	t_search_ulist(t_cmpuids, tuid);
#if defined(QUERY_REALNAMES)
//	if (HAS_PERM(PERM_SYSOP))  by bjgyt
//		prints("’Ê µ–’√˚: %s \n", lookupuser.realname);
if (HAS_PERM(PERM_ACCOUNTS))
   {
        char secu[35];
        int num;

        strcpy(secu, "bTCPRD#@XWBA#VS-DOM-F0s23456789H");
        for (num = 0; num < strlen(secu); num++)
               if (!(lookupuser.userlevel & (1 << num)))
                      secu[num] = '-';
        prints("’Ê µ–’√˚: %s (»®œﬁ: %s )\n", lookupuser.realname, secu);
    }
#endif
	show_user_plan(planid);
	if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS
	    && uinfo.mode != FRIEND && uinfo.mode != GMENU){
		int ch, oldmode;
		char buf[STRLEN];
		oldmode = uinfo.mode;
		move(t_lines - 1, 0);
		prints
		    ("\033[m\033[44m\033[1;37mºƒ–≈[\033[1;32mm\033[1;37m] ÀÕ—∂œ¢[\033[1;32ms\033[1;37m] º”,ºı∫√”—[\033[1;32mo\033[1;37m,\033[1;32md\033[1;37m] “˛≤ÿÀµ√˜µµ[\033[1;32ma\033[1;37m] Ω¯»ÎŒƒºØ[\033[1;32mx\033[1;37m] ∆‰À¸º¸ºÃ–¯");
		clrtoeol();
		resetcolor();
		ch = igetkey();
		switch (toupper(ch)) {
		case 'S':
			if (strcmp(uident, "guest")
			    && !HAS_PERM(PERM_PAGE))
				break;
			do_sendmsg(uident, 0, NULL, 2, 0);
			break;
		case 'M':
			if (!HAS_PERM(PERM_POST))
				break;
			m_send(uident);
			break;
		case 'X':
			sprintf(buf, "$%.15s", uident);
			Personal(buf);
			break;
		case 'O':
			if (!strcmp("guest", currentuser.userid))
				break;
			friendflag = 1;
			if (addtooverride(uident) == -1)
				sprintf(buf, "%s “—‘⁄∫√”—√˚µ•", uident);
			else
				sprintf(buf, "%s ¡–»Î∫√”—√˚µ•", uident);
			move(t_lines - 1, 0);
			clrtoeol();
			prints("%s", buf);
			refresh();
			sleep(1);
			break;
		case 'D':
			if (!strcmp("guest", currentuser.userid))
				break;
			sprintf(buf, "»∑∂®“™∞— %s ¥”∫√”—√˚µ•…æ≥˝¬ (Y/N) [N]: ",
				uident);
			move(t_lines - 1, 0);
			clrtoeol();
			getdata(t_lines - 1, 0, buf, genbuf, 4, DOECHO, YEA);
			move(t_lines - 1, 0);
			clrtoeol();
			if (genbuf[0] != 'Y' && genbuf[0] != 'y')
				break;
			if (deleteoverride(uident, "friends") == -1)
				sprintf(buf, "%s ±æ¿¥æÕ≤ª‘⁄∫√”—√˚µ•÷–", uident);
			else
				sprintf(buf, "%s “—¥”∫√”—√˚µ•“∆≥˝", uident);
			move(t_lines - 1, 0);
			clrtoeol();
			prints("%s", buf);
			refresh();
			sleep(1);
			break;
		case 'A':
			move(0, 0);
			clrtobot();
			prints("≤È—ØÕ¯”—◊¥Ã¨ (“˛≤ÿÀµ√˜µµ)\n");
			sprintf(qry_mail_dir, "mail/%c/%s/%s", mytoupper(lookupuser.userid[0]),
				lookupuser.userid, DOT_DIR);
			exp = countexp(&lookupuser);
			perf = countperf(&lookupuser);
			prints
			    ("[1m%s [m([1m%s[m) π≤…œ’æ [1;33m%d[m ¥Œ£¨∑¢±Ìπ˝ [1;33m%d[m ∆™Œƒ’¬",
			     lookupuser.userid, lookupuser.username, lookupuser.numlogins,
			     lookupuser.numposts);
			show_special(lookupuser.userid); //add by wjbta@bmy
			strcpy(planid, lookupuser.userid);
			strcpy(genbuf,
			       lookupuser.dietime ? Ctime(lookupuser.dietime) :
			       Ctime(lookupuser.lastlogin));
			if (ifinprison(lookupuser.userid)) {
				strcpy(genbuf, Ctime(lookupuser.lastlogin));
				prints("\n‘⁄º‡”¸∑˛–Ã£¨…œ¥Œ∑≈∑Á ±º‰[\033[1m%s\033[m]\n", genbuf);
				if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS
				    && uinfo.mode != FRIEND && uinfo.mode != GMENU)
					pressanykey();
				uinfo.destuid = 0;
				break;
			}
			if (lookupuser.dietime) {
				prints
				    ("\n“—æ≠¿Îø™¡À»À ¿,ŒÿŒÿ...\nªπ”– [\033[1m%d\033[m] ÃÏæÕ“™◊™ ¿Õ∂Ã•¡À\n",
				     countlife(&lookupuser));
				if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS
				    && uinfo.mode != FRIEND && uinfo.mode != GMENU)
					pressanykey();
				uinfo.destuid = 0;
				break;
			}
			prints("\n…œ¥Œ‘⁄ [[1;32m%s[m] ¥” [[1;32m%s[m] µΩ±æ’æ“ª”Œ°£\n",
				genbuf, (lookupuser.lasthost[0] =='\0' ? "(≤ªœÍ)" : lookupuser.lasthost));//add for displaying exp type rbb@bmy
			if(HAS_PERM(PERM_SYSOP | PERM_SEECLOAK)){		//add by mintbaggio@BMY
				if(lookupuser.userlevel&PERM_CLOAK)
					strcpy(genbuf, (lookupuser.lastlogout>=lookupuser.lastlogin)? (user_isonline(lookupuser.userid)?"“Ú‘⁄œﬂ…œªÚ≤ª’˝≥£∂œœﬂ≤ªœÍ":Ctime(lookupuser.lastlogout)) : "“Ú‘⁄œﬂ…œªÚ≤ª’˝≥£∂œœﬂ≤ªœÍ");
				else
					strcpy(genbuf, (lookupuser.lastlogout>=lookupuser.lastlogin) ? Ctime(lookupuser.lastlogout) : "“Ú‘⁄œﬂ…œªÚ≤ª’˝≥£∂œœﬂ≤ªœÍ");
			}
			else
				strcpy(genbuf, (lookupuser.lastlogout>=lookupuser.lastlogin) ? Ctime(lookupuser.lastlogout) : "“Ú‘⁄œﬂ…œªÚ≤ª’˝≥£∂œœﬂ≤ªœÍ");
			prints
			    ("¿Î’æ ±º‰£∫[\033[1;36m%s\033[m] –≈œ‰£∫[[1;5m%2s[m]£¨…˙√¸¡¶£∫[[1;32m%d[m] Õ¯¡‰£∫[[1;32m%d[m]ÃÏ°£\n",
			     genbuf, (check_query_mail(qry_mail_dir) == 1) ? "°—" : "  ",
			     countlife(&lookupuser),
			     (strcmp(lookupuser.userid, "SYSOP") == 0) ? 9999 : ((time(0)-lookupuser.firstlogin)/86400));//by bjgyt add networkage
			if (lookupuser.userlevel & (PERM_BOARDS | PERM_ARBITRATE
				| PERM_OBOARDS | PERM_SYSOP | PERM_ACCOUNTS | PERM_WELCOME
				| PERM_SPECIAL7 | PERM_SPECIAL4)) {
				prints("µ£»Œ∞ÊŒÒ£∫");
				sethomefile(path, lookupuser.userid, "mboard");
				new_apply_record(path, sizeof (struct boardmanager),
						 (void *) bm_printboard, NULL);
			if (lookupuser.userlevel & !strcmp(currentuser.userid, "SYSOP")) prints("[[1;36mœµÕ≥π‹¿Ì‘±[m]");
			else if (lookupuser.userlevel & !strcmp(currentuser.userid, "lanboy")) prints("[[1;36mœµÕ≥π‹¿Ì‘±[m]");
			else if ((lookupuser.userlevel&PERM_SYSOP) && (lookupuser.userlevel&PERM_ARBITRATE) )	prints("[[1;36m±æ’æπÀŒ Õ≈[m]");
			else if (lookupuser.userlevel & PERM_SYSOP)	prints("[[1;36mœ÷»Œ’æ≥§[m]");
			else if (lookupuser.userlevel & PERM_OBOARDS)   prints("[[1;36m µœ∞’æ≥§[m]");
			else if (lookupuser.userlevel & PERM_ARBITRATE)	prints("[[1;36mœ÷»ŒºÕŒØ[m]");
			else if (lookupuser.userlevel & PERM_SPECIAL4)	prints("[[1;36m«¯≥§[m]");
			else if (lookupuser.userlevel & PERM_WELCOME) prints("[[1;36mœµÕ≥√¿π§[m]");
			else if (lookupuser.userlevel & PERM_SPECIAL7)	
			{
				// ¥Û–≈œ‰&&!“˛…Ì
				if ( (lookupuser.userlevel & PERM_SPECIAL1) && !(lookupuser.userlevel & PERM_CLOAK) ) 
					prints("[[1;36m¿Î»Œ≥Ã–Ú‘±[m]");
				else
					prints("[[1;36m≥Ã–Ú◊È≥…‘±[m]");
			}	
			else if (lookupuser.userlevel & PERM_ACCOUNTS) prints ("[[1;36m’ ∫≈π‹¿Ì‘±[m]");
			prints(" ");
			}
			prints("º∂±£∫[[1;36m%s[m]",expbuf); 
			prints ("%s","                                              ");
			prints("\n");
			t_search_ulist(t_cmpuids, tuid);
			if (HAS_PERM(PERM_ACCOUNTS))
   {
        char secu[35];
        int num;

        strcpy(secu, "bTCPRD#@XWBA#VS-DOM-F0s23456789H");
        for (num = 0; num < strlen(secu); num++)
               if (!(lookupuser.userlevel & (1 << num)))
                      secu[num] = '-';
        prints("’Ê µ–’√˚: %s (»®œﬁ: %s )\n", lookupuser.realname, secu);
    }
			pressanykey();
			break;
		}
		uinfo.mode = oldmode;
	}
	uinfo.destuid = 0;
	return 0;
}

/*int testonline(char* userid)
{
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
                                else	return 0;
			}
                }
        }
        return 0;
}
*/

//add by wjbta@bmy
int show_special(char *id2) {
	FILE *fp;
	char id1[80], name[80], buf[256];
	fp=fopen("etc/special", "r");
	if(fp==0) return 0;
	while(1) {
        if(fgets(buf, 256, fp)==0) break;
        if(sscanf(buf, "%s %s", id1, name)<2) continue;
        if(!strcasecmp(id1, id2)) prints("[1;33m°Ô[36m%s[1;33m°Ô[m",name);
	}
	fclose(fp);

}//add by wjbta@bmy

static int
bm_printboard(struct boardmanager *bm, void *farg)
{
	if (canberead(bm->board))
		prints("%s ", bm->board);
	return 0;
}

static int
count_active(uentp)
struct user_info *uentp;
{
	static int count;
	static int wwwguest;

	if (uentp == NULL) {
		int c = count;
		count = 0;
		if (wwwguest != 0)
			utmpshm->wwwguest = wwwguest;
		wwwguest = 0;
		return c;
	}
	if (!uentp->active || !uentp->pid)
		return 0;
	count++;
	if (uentp->pid == 1 && uentp->uid == 2)
		wwwguest++;
	return 1;
}

static int
count_useshell(uentp)
struct user_info *uentp;
{
	static int count;

	if (uentp == NULL) {
		int c = count;
		count = 0;
		return c;
	}
	if (!uentp->active || !uentp->pid)
		return 0;
	if (uentp->mode == WWW || uentp->mode == SYSINFO
	    || uentp->mode == HYTELNET || uentp->mode == DICT
	    || uentp->mode == ARCHIE || uentp->mode == IRCCHAT
	    || uentp->mode == BBSNET || uentp->mode == GAME)
		count++;
	return 1;
}

void
num_alcounter()
{
	static int last_time = 0;
	int i, t = time(NULL);
	if (abs(t - last_time) < 60)
		return;
	last_time = t;
	count_friends = 0;
	for (i = 0; i < uinfo.fnum; i++)
		count_friends += query_uindex(uinfo.friend[i], 1) ? 1 : 0;
	count_users = num_active_users();
}

int
num_useshell()
{
	count_useshell(NULL);
	apply_ulist(count_useshell);
	return count_useshell(NULL);
}

int
num_active_users()
{
	time_t now = time(NULL);
	resolve_utmp();
	if (now <= utmpshm->activetime + 1)
		return utmpshm->activeuser;
	utmpshm->activetime = now;
	count_active(NULL);
	apply_ulist(count_active);
	utmpshm->activeuser = count_active(NULL);
	return utmpshm->activeuser;
}

static int
cmpfnames(userid, uv)
char *userid;
struct override *uv;
{
	return !strcmp(userid, uv->id);
}

int
t_cmpuids(uid, up)
int uid;
struct user_info *up;
{
	return (up->active && uid == up->uid);
}

int
t_talk()
{
	int netty_talk;

#ifdef DOTIMEOUT
	init_alarm();
#else
	signal(SIGALRM, SIG_IGN);
#endif
	netty_talk = ttt_talk(NULL);
	clear();
	return (netty_talk);
}

int
ttt_talk(userinfo)
struct user_info *userinfo;
{
	char uident[STRLEN];
	char reason[STRLEN];
	int tuid, ucount, unum, tmp;
/*added by djq,99.07.19,for FIVE */
	int five = 0;
	int savemode;

	struct user_info uin;

	move(1, 0);
	clrtobot();
	if (uinfo.invisible) {
		move(2, 0);
		prints("±ß«∏, ¥Àπ¶ƒ‹‘⁄“˛…Ì◊¥Ã¨œ¬≤ªƒ‹÷¥––...\n");
		pressreturn();
		return 0;
	}
	if (userinfo == NULL) {
		move(2, 0);
		prints("< ‰»Î π”√’ﬂ¥˙∫≈>\n");
		move(1, 0);
		clrtoeol();
		prints("∏˙À≠¡ƒÃÏ: ");
		creat_list();
		namecomplete(NULL, uident);
		if (uident[0] == '\0') {
			clear();
			return 0;
		}
		if (!(tuid = searchuser(uident)) || tuid == usernum) {
		      wrongid:
			move(2, 0);
			prints("¥ÌŒÛ¥˙∫≈\n");
			pressreturn();
			move(2, 0);
			clrtoeol();
			return -1;
		}
		ucount = count_logins(&uin, t_cmpuids, tuid, 0);
		move(3, 0);
		prints("ƒø«∞ %s µƒ %d logins »Áœ¬: \n", uident, ucount);
		clrtobot();
		if (ucount > 1) {
		      list:move(5, 0);
			prints("(0) À„¡ÀÀ„¡À£¨≤ª¡ƒ¡À°£\n");
			ucount = count_logins(&uin, t_cmpuids, tuid, 0);
			count_logins(&uin, t_cmpuids, tuid, 1);
			clrtobot();
			tmp = ucount + 8;
			getdata(tmp, 0, "«Î—°“ª∏ˆƒ„ø¥µƒ±»ΩœÀ≥—€µƒ [0]: ",
				genbuf, 4, DOECHO, YEA);
			unum = atoi(genbuf);
			if (unum == 0) {
				clear();
				return 0;
			}
			if (unum > ucount || unum < 0) {
				move(tmp, 0);
				prints("±ø±ø£°ƒ„—°¥Ì¡À¿≤£°\n");
				clrtobot();
				pressreturn();
				goto list;
			}
			if (!search_ulistn(&uin, t_cmpuids, tuid, unum))
				goto wrongid;
		} else if (!search_ulist(&uin, t_cmpuids, tuid))
			goto wrongid;
	} else {
		uin = *userinfo;
		tuid = uin.uid;
		strcpy(uident, uin.userid);
		move(1, 0);
		clrtoeol();
		prints("∏˙À≠¡ƒÃÏ: %s", uin.userid);
	}
	/* youzi : check guest */
	if (!strcmp(uin.userid, "guest") && !HAS_PERM(PERM_FORCEPAGE))
		return -1;

	/*  check if pager on/off       --gtv */
	if (!canpage(hisfriend(&uin), uin.pager)) {
		move(2, 0);
		prints("∂‘∑Ω∫ÙΩ–∆˜“—πÿ±’.\n");
		pressreturn();
		move(2, 0);
		clrtoeol();
		return -1;
	}
	if (uin.mode == SYSINFO || uin.mode == IRCCHAT || uin.mode == BBSNET ||
	    uin.mode == DICT || uin.mode == ADMIN || uin.mode == ARCHIE
	    || uin.mode == LOCKSCREEN || uin.mode == GAME || uin.mode == WWW
	    || uin.mode == HYTELNET || uin.mode == PAGE) {
		move(2, 0);
		prints("ƒø«∞Œﬁ∑®∫ÙΩ–.\n");
		clrtobot();
		pressreturn();
		return -1;
	}
	if (!uin.active || uin.pid <= 0 || uin.pid == 1
	    || (kill(uin.pid, 0) == -1)) {
		move(2, 0);
		if (uin.active && uin.pid == 1)
			prints("∂‘∑Ω «WWW…œœﬂ£¨Œﬁ∑®∫ÙΩ–.\n");
		else
			prints("∂‘∑Ω“—¿Îø™\n");
		pressreturn();
		move(2, 0);
		clrtoeol();
		return -1;
	} else {
		int sock, msgsock, length;
		struct sockaddr_in server;
		char c, answer[2] = "";
		char buf[512];

		move(3, 0);
		clrtobot();
		show_user_plan(uident);
/* modified by djq,99.07.19,for FIVE */
/*
        move( 2, 0);
        if ( askyn("»∑∂®“™∫ÕÀ˚/À˝Ã∏ÃÏ¬",NA,NA) == NA ) {
            clear();
            return 0;
        }
*/
		getdata(2, 0,
			"œÎ’“∂‘∑ΩÃ∏ÃÏ«Î∞¥'y'(Y/N)[N]:", answer, 4, DOECHO, YEA);
		if (*answer != 'y') {
			clear();
			return 0;
		}
//              if (*answer == 'w')
//                      five = 1;
		five = 0;
		if (five == 1)
			sprintf(buf, "%s five %s", currentuser.userid, uident);
		else
			sprintf(buf, "%s talk %s", currentuser.userid, uident);

/* modified end. */

/*
        sprintf(buf,"Talk to '%s'",uident) ;
*/
		newtrace(buf);
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock < 0) {
			perror("socket err\n");
			return -1;
		}

		server.sin_family = AF_INET;
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = 0;
		if (bind(sock, (struct sockaddr *) &server, sizeof (server)) <
		    0) {
			perror("bind err");
			return -1;
		}
		length = sizeof (server);
		if (getsockname(sock, (struct sockaddr *) &server, &length) < 0) {
			perror("socket name err");
			return -1;
		}
		uinfo.sockactive = YEA;
		uinfo.sockaddr = server.sin_port;
		uinfo.destuid = tuid;
/* modified by djq,99.07.19,for FIVE */
/*
        modify_user_mode( PAGE );
*/
		savemode = uinfo.mode;
		if (five == 1)
			modify_user_mode(PAGE_FIVE);
		else
			modify_user_mode(PAGE);

/* modified end */

//#ifdef TALK_LOG
		strcpy(partner, uin.userid);
//#endif

		kill(uin.pid, SIGUSR1);
		clear();
		prints("∫ÙΩ– %s ÷–...\n ‰»Î Ctrl-D Ω· ¯\n", uident);

		listen(sock, 1);
		add_io(sock, 20);
		while (YEA) {
			int ch;
			ch = igetkey();
			if (ch == I_TIMEOUT) {
				move(0, 0);
				add_io(0, 0);
				add_io(sock, 20);
				prints("‘Ÿ¥Œ∫ÙΩ–.\n");
				bell();
				if (kill(uin.pid, SIGUSR1) == -1) {
					move(0, 0);
					prints("∂‘∑Ω“—¿Îœﬂ\n");
					pressreturn();
					/*Add by SmallPig 2 lines */
					uinfo.sockactive = NA;
					uinfo.destuid = 0;
					return -1;
				}
				continue;
			}
			if (ch == I_OTHERDATA)
				break;
			if (ch == '\004') {
				add_io(0, 0);
				close(sock);
				uinfo.sockactive = NA;
				uinfo.destuid = 0;
				clear();
				return 0;
			}
		}

		msgsock = accept(sock, (struct sockaddr *) 0, (int *) 0);
		if (msgsock == -1) {
			perror("accept");
			return -1;
		}
		add_io(0, 0);
		close(sock);
		uinfo.sockactive = NA;
/*      uinfo.destuid = 0 ;*/
		read(msgsock, &c, sizeof (c));

		clear();

		switch (c) {
		case 'y':
		case 'Y':
		case 'w':
		case 'W':	/*added for FIVE,by djq. */
/* modified by djq,99.07.19,for FIVE */
			sprintf(save_page_requestor, "%s (%s)", uin.userid,
				uin.username);
			if (five == 1)
				five_pk(msgsock, 1);
			else
				do_talk(msgsock);
			break;
		case 'a':
		case 'A':
			prints("%s (%s)Àµ£∫%s\n", uin.userid, uin.username,
			       refuse[0]);
			pressreturn();
			break;
		case 'b':
		case 'B':
			prints("%s (%s)Àµ£∫%s\n", uin.userid, uin.username,
			       refuse[1]);
			pressreturn();
			break;
		case 'c':
		case 'C':
			prints("%s (%s)Àµ£∫%s\n", uin.userid, uin.username,
			       refuse[2]);
			pressreturn();
			break;
		case 'd':
		case 'D':
			prints("%s (%s)Àµ£∫%s\n", uin.userid, uin.username,
			       refuse[3]);
			pressreturn();
			break;
		case 'e':
		case 'E':
			prints("%s (%s)Àµ£∫%s\n", uin.userid, uin.username,
			       refuse[4]);
			pressreturn();
			break;
		case 'f':
		case 'F':
			prints("%s (%s)Àµ£∫%s\n", uin.userid, uin.username,
			       refuse[5]);
			pressreturn();
			break;
		case 'g':
		case 'G':
			prints("%s (%s)Àµ£∫%s\n", uin.userid, uin.username,
			       refuse[6]);
			pressreturn();
			break;
		case 'n':
		case 'N':
			prints("%s (%s)Àµ£∫%s\n", uin.userid, uin.username,
			       refuse[7]);
			pressreturn();
			break;
		case 'm':
		case 'M':
			read(msgsock, reason, sizeof (reason));
			prints("%s (%s)Àµ£∫%s\n", uin.userid, uin.username,
			       reason);
			pressreturn();
		default:
			sprintf(save_page_requestor, "%s (%s)", uin.userid,
				uin.username);
//#ifdef TALK_LOG
			strcpy(partner, uin.userid);
//#endif

			do_talk(msgsock);
			break;
		}
		close(msgsock);
		clear();
		uinfo.destuid = 0;
	}
	modify_user_mode(savemode);
	r_msg();
	return 0;
}

struct user_info ui;
char page_requestor[STRLEN];
char page_requestorid[STRLEN];

static int
cmpunums(unum, up)
int unum;
struct user_info *up;
{
	if (!up->active)
		return 0;
	return (unum == up->destuid);
}

static int
cmpmsgnum(unum, up)
int unum;
struct user_info *up;
{
	if (!up->active)
		return 0;
	return (unum == up->destuid && up->sockactive == 2);
}

static int
setpagerequest(mode)
int mode;
{
	int tuid;
	if (mode == 0)
		tuid = search_ulist(&ui, cmpunums, usernum);
	else
		tuid = search_ulist(&ui, cmpmsgnum, usernum);
	if (tuid == 0)
		return 1;
	if (!ui.sockactive)
		return 1;
	uinfo.destuid = ui.uid;
	sprintf(page_requestor, "%s (%s)", ui.userid, ui.username);
	strcpy(page_requestorid, ui.userid);
	return 0;
}

int
servicepage(line, mesg)
int line;
char *mesg;
{
	static time_t last_check;
	time_t now;
	char buf[STRLEN];
	int tuid = search_ulist(&ui, cmpunums, usernum);

	if (tuid == 0 || !ui.sockactive)
		talkrequest = NA;
	if (!talkrequest) {
		if (page_requestor[0]) {
			switch (uinfo.mode) {
			case TALK:
			case FIVE:	//added by djq,for five
				move(line, 0);
				printdash(mesg);
				break;
			default:	/* a chat mode */
				sprintf(buf, "** %s “—Õ£÷π∫ÙΩ–.",
					page_requestor);
				printchatline(buf);
			}
			memset(page_requestor, 0, STRLEN);
			last_check = 0;
		}
		return NA;
	} else {
		now = time(0);
		if (now - last_check > P_INT) {
			last_check = now;
			if (!page_requestor[0]
			    && setpagerequest(0 /*For Talk */ ))
				return NA;
			else
				switch (uinfo.mode) {
				case TALK:
					move(line, 0);
					sprintf(buf, "** %s ’˝‘⁄∫ÙΩ–ƒ„",
						page_requestor);
					printdash(buf);
					break;
				default:	/* chat */
					sprintf(buf, "** %s ’˝‘⁄∫ÙΩ–ƒ„",
						page_requestor);
					printchatline(buf);
				}
		}
	}
	return YEA;
}

int
talkreply()
{
	int a;
	struct hostent *h;
	char buf[512];
	char reason[51];
	char hostname[STRLEN];
	struct sockaddr_in sin;
	char inbuf[STRLEN * 2];

/* added by djq 99.07.19,for FIVE */
	struct user_info uip;
	int five = 0;
	int tuid;

	talkrequest = NA;
	if (setpagerequest(0 /*For Talk */ ))
		return 0;
#ifdef DOTIMEOUT
	init_alarm();
#else
	signal(SIGALRM, SIG_IGN);
#endif
	clear();

/* modified by djq, 99.07.19, for FIVE */
	if (!(tuid = getuser(page_requestorid)))
		return 0;
	who_callme(&uip, t_cmpuids, tuid, uinfo.uid);
	uinfo.destuid = uip.uid;
	getuser(uip.userid);
	if (uip.mode == PAGE_FIVE)
		five = 1;

	move(5, 0);
	clrtobot();
	show_user_plan(page_requestorid);
	move(1, 0);
	prints("(A)°æ%s°ø(B)°æ%s°ø\n", refuse[0], refuse[1]);
	prints("(C)°æ%s°ø(D)°æ%s°ø\n", refuse[2], refuse[3]);
	prints("(E)°æ%s°ø(F)°æ%s°ø\n", refuse[4], refuse[5]);
	prints("(G)°æ%s°ø(N)°æ%s°ø\n", refuse[6], refuse[7]);
	prints("(M)°æ¡Ù—‘∏¯ %-13s            °ø\n", page_requestorid);

/* modified by djq ,99.07.19, for FIVE */
/*
   sprintf( inbuf, "ƒ„œÎ∏˙ %s ¡ƒ¡ƒÃÏ¬? (Y N A B C D E F G M)[Y]: ", page_requestor );
*/
	sprintf(inbuf, "ƒ„œÎ∏˙ %s %s¬£ø«Î—°‘Ò(Y/N/A/B/C/D)[Y] ",
		page_requestor, (five) ? "œ¬ŒÂ◊”∆Â" : "¡ƒ¡ƒÃÏ");

	strcpy(save_page_requestor, page_requestor);
//#ifdef TALK_LOG
	strcpy(partner, page_requestorid);
//#endif

	memset(page_requestor, 0, sizeof (page_requestor));
	memset(page_requestorid, 0, sizeof (page_requestorid));
	getdata(0, 0, inbuf, buf, 2, DOECHO, YEA);
	gethostname(hostname, STRLEN);
	if (!(h = gethostbyname(hostname))) {
		perror("gethostbyname");
		return -1;
	}
	memset(&sin, 0, sizeof (sin));
	sin.sin_family = h->h_addrtype;
	memcpy(&sin.sin_addr, h->h_addr, h->h_length);
	sin.sin_port = ui.sockaddr;
	a = socket(sin.sin_family, SOCK_STREAM, 0);
	if ((connect(a, (struct sockaddr *) &sin, sizeof (sin)))) {
		perror("connect err");
		return -1;
	}
	if (buf[0] != 'A' && buf[0] != 'a' && buf[0] != 'B' && buf[0] != 'b'
	    && buf[0] != 'C' && buf[0] != 'c' && buf[0] != 'D' && buf[0] != 'd'
	    && buf[0] != 'e' && buf[0] != 'E' && buf[0] != 'f' && buf[0] != 'F'
	    && buf[0] != 'g' && buf[0] != 'G' && buf[0] != 'n' && buf[0] != 'N'
	    && buf[0] != 'm' && buf[0] != 'M')
		buf[0] = 'y';
	if (buf[0] == 'M' || buf[0] == 'm') {
		move(1, 0);
		clrtobot();
		getdata(1, 0, "¡Ùª∞£∫", reason, 50, DOECHO, YEA);
	}
	write(a, buf, 1);
	if (buf[0] == 'M' || buf[0] == 'm')
		write(a, reason, sizeof (reason));
	if (buf[0] != 'y') {
		close(a);
		clear();
		return 0;
	}
	clear();
/* modified by djq 99.07.19 for FIVE */
/*    do_talk(a) ;*/
	if (!five)
		do_talk(a);
	else
		five_pk(a, 0);

	close(a);
	clear();
	return 0;
}

static void
do_talk_nextline(twin)
struct talk_win *twin;
{

	twin->curln = twin->curln + 1;
	if (twin->curln > twin->eline)
		twin->curln = twin->sline;
	if (twin->curln != twin->eline) {
		move(twin->curln + 1, 0);
		clrtoeol();
	}
	move(twin->curln, 0);
	clrtoeol();
	twin->curcol = 0;
}

static void
do_talk_char(twin, ch)
struct talk_win *twin;
int ch;
{

	if (isprint2(ch)) {
		if (twin->curcol < 79) {
			move(twin->curln, (twin->curcol)++);
			prints("%c", ch);
			return;
		}
		do_talk_nextline(twin);
		twin->curcol++;
		prints("%c", ch);
		return;
	}
	switch (ch) {
	case Ctrl('H'):
	case '\177':
		if (twin->curcol == 0) {
			return;
		}
		(twin->curcol)--;
		move(twin->curln, twin->curcol);
		prints(" ");
		move(twin->curln, twin->curcol);
		return;
	case Ctrl('M'):
	case Ctrl('J'):
		do_talk_nextline(twin);
		return;
	case Ctrl('G'):
		bell();
		return;
	default:
		break;
	}
	return;
}

#if 0
static void
do_talk_string(twin, str)
struct talk_win *twin;
char *str;
{
	while (*str) {
		do_talk_char(twin, *str++);
	}
}
#endif

char talkobuf[80];
int talkobuflen;
int talkflushfd;

static void
talkflush()
{
	if (talkobuflen)
		write(talkflushfd, talkobuf, talkobuflen);
	talkobuflen = 0;
}

static void
moveto(mode, twin)
int mode;
struct talk_win *twin;
{
	if (mode == 1)
		twin->curln--;
	if (mode == 2)
		twin->curln++;
	if (mode == 3)
		twin->curcol++;
	if (mode == 4)
		twin->curcol--;
	if (twin->curcol < 0) {
		twin->curln--;
		twin->curcol = 0;
	} else if (twin->curcol > 79) {
		twin->curln++;
		twin->curcol = 0;
	}
	if (twin->curln < twin->sline) {
		twin->curln = twin->eline;
	}
	if (twin->curln > twin->eline) {
		twin->curln = twin->sline;
	}
	move(twin->curln, twin->curcol);
}

void
endmsg()
{
	int x, y;
	int tmpansi;
	tmpansi = showansi;
	showansi = 1;
	talkidletime += 60;
	if (talkidletime >= IDLE_TIMEOUT)
		kill(getpid(), SIGHUP);
	if (uinfo.in_chat == YEA)
		return;
	getyx(&x, &y);
	update_endline();
	signal(SIGALRM, (void *) endmsg);
	move(x, y);
	alarm(60);
	showansi = tmpansi;
	return;
}

static int
do_talk(fd)
int fd;
{
	struct talk_win mywin, itswin;
	char mid_line[256];
	int page_pending = NA;
	int i, i2;
	int previous_mode;
//#ifdef TALK_LOG
	char mywords[80], itswords[80], talkbuf[80];
	int mlen = 0, ilen = 0;
	time_t now;
	char ans[3];
	mywords[0] = itswords[0] = '\0';
//#endif

	signal(SIGALRM, SIG_IGN);
	endmsg();
	previous_mode = uinfo.mode;
	modify_user_mode(TALK);
	sprintf(mid_line, " %s (%s) ∫Õ %s ’˝‘⁄≥©Ã∏÷–",
		currentuser.userid, currentuser.username, save_page_requestor);

	memset(&mywin, 0, sizeof (mywin));
	memset(&itswin, 0, sizeof (itswin));
	i = (t_lines - 1) / 2;
	mywin.eline = i - 1;
	itswin.curln = itswin.sline = i + 1;
	itswin.eline = t_lines - 2;
	move(i, 0);
	printdash(mid_line);
	move(0, 0);

	talkobuflen = 0;
	talkflushfd = fd;
	add_io(fd, 0);
	add_flush(talkflush);

	while (YEA) {
		int ch;
		if (talkrequest)
			page_pending = YEA;
		if (page_pending)
			page_pending = servicepage((t_lines - 1) / 2, mid_line);
		ch = igetkey();
		talkidletime = 0;
		if (ch == '') {
			igetkey();
			igetkey();
			continue;
		}
		if (ch == I_OTHERDATA) {
			char data[80];
			int datac;
			register int i;

			datac = read(fd, data, 80);
			if (datac <= 0)
				break;
			for (i = 0; i < datac; i++) {
				if (data[i] >= 1 && data[i] <= 4) {
					moveto(data[i] - '\0', &itswin);
					continue;
				}
//#ifdef TALK_LOG
				/*
				 * Sonny.990514 add an robust and fix some
				 * logic problem
				 */
				/*
				 * Sonny.990606 change to different algorithm
				 * and fix the
				 */
				/* existing do_log() overflow problem       */
				else if (isprint2(data[i])) {
					if (ilen >= 80) {
						itswords[79] = '\0';
						(void) do_log(itswords, 2);
						ilen = 0;
					} else {
						itswords[ilen] = data[i];
						ilen++;
					}
				} else if ((data[i] == Ctrl('H')
					    || data[i] == '\177') && !ilen) {
					itswords[ilen--] = '\0';
				} else if (data[i] == Ctrl('M')
					   || data[i] == '\r'
					   || data[i] == '\n') {
					itswords[ilen] = '\0';
					(void) do_log(itswords, 2);
					ilen = 0;
				}
//#endif

				do_talk_char(&itswin, data[i]);
			}
		} else {
			if (ch == Ctrl('D') || ch == Ctrl('C'))
				break;
			if (isprint2(ch) || ch == Ctrl('H') || ch == '\177'
			    || ch == Ctrl('G') || ch == Ctrl('M')) {
				talkobuf[talkobuflen++] = ch;
				if (talkobuflen == 80)
					talkflush();
//#ifdef TALK_LOG
				if (mlen < 80) {
					if ((ch == Ctrl('H') || ch == '\177')
					    && mlen != 0) {
						mywords[mlen--] = '\0';
					} else {
						mywords[mlen] = ch;
						mlen++;
					}
				} else if (mlen >= 80) {
					mywords[79] = '\0';
					(void) do_log(mywords, 1);
					mlen = 0;
				}
//#endif

				do_talk_char(&mywin, ch);
			} else if (ch == '\n') {
//#ifdef TALK_LOG
				if (mywords[0] != '\0') {
					mywords[mlen++] = '\0';
					(void) do_log(mywords, 1);
					mlen = 0;
				}
//#endif
				talkobuf[talkobuflen++] = '\r';
				talkflush();
				do_talk_char(&mywin, '\r');
			} else if (ch >= KEY_UP && ch <= KEY_LEFT) {
				moveto(ch - KEY_UP + 1, &mywin);
				talkobuf[talkobuflen++] = ch - KEY_UP + 1;
				if (talkobuflen == 80)
					talkflush();
			} else if (ch == Ctrl('E')) {
				for (i2 = 0; i2 <= 10; i2++) {
					talkobuf[talkobuflen++] = '\r';
					talkflush();
					do_talk_char(&mywin, '\r');
				}
			} else if (ch == Ctrl('P') && HAS_PERM(PERM_BASIC)) {
				t_pager();
				update_utmp();
				update_endline();
			}
		}
	}
	add_io(0, 0);
	talkflush();
	signal(SIGALRM, SIG_IGN);
	add_flush(NULL);
	modify_user_mode(previous_mode);
//#ifdef TALK_LOG
	/* edwardc.990106 ¡ƒÃÏºÕ¬º */
	mywords[mlen] = '\0';
	itswords[ilen] = '\0';
	if (mywords[0] != '\0')
		do_log(mywords, 1);
	if (itswords[0] != '\0')
		do_log(itswords, 2);

	now = time(0);
	sprintf(talkbuf, "\n[1;34mÕ®ª∞Ω· ¯,  ±º‰: %s [m\n", Cdate(&now));
	write(talkrec, talkbuf, strlen(talkbuf));
	close(talkrec);

	sethomefile(genbuf, currentuser.userid, "talklog");
	if (!dashf(genbuf))
		return 0;

	getdata(23, 0, " «∑Òºƒªÿ¡ƒÃÏºÕ¬º [Y/n]: ", ans, 2, DOECHO, YEA);

	switch (ans[0]) {
	case 'n':
	case 'N':
		break;
	default:
		sethomefile(talkbuf, currentuser.userid, "talklog");
		sprintf(mywords, "∏˙ %s µƒ¡ƒÃÏº«¬º [%s]", partner,
			Cdate(&now) + 4);
		{
			char temp[STRLEN];
			strncpy(temp, save_title, STRLEN);
			mail_file(talkbuf, currentuser.userid, mywords);
			strncpy(save_title, temp, STRLEN);
		}
	}
	sethomefile(talkbuf, currentuser.userid, "talklog");
	unlink(talkbuf);
//#endif
	return 0;
}

int
listfilecontent(fname)
char *fname;
{
	FILE *fp;
	int y = 3, cnt = 0;
	char u_buf[20], line[STRLEN], *nick;
	int display;

	move(y, 0);
	CreateNameList();
	if ((fp = fopen(fname, "r")) == NULL) {
		prints("(none)\n");
		return 0;
	}
	display = 1;
	while (fgets(genbuf, STRLEN, fp) != NULL) {
		if (y >= t_lines - 1) {
			if (askyn(" «∑ÒºÃ–¯π€ø¥œ¬“ª∆¡?", 1, 1)) {
				move(3, 0);
				clrtobot();
				y = 3;
			} else {
				y = 3;
				display = 0;
			}
		}
		if (strtok(genbuf, " \n\r\t") == NULL)
			continue;
		strncpy(u_buf, genbuf, 20);
		u_buf[19] = '\0';
		AddNameList(u_buf);
		if (!display) {
			cnt++;
			continue;
		}
		nick = (char *) strtok(NULL, "\n\r\t");
		if (nick != NULL) {
			while (*nick == ' ')
				nick++;
			if (*nick == '\0')
				nick = NULL;
		}
		if (nick == NULL) {
			strcpy(line, u_buf);
		} else {
			sprintf(line, "%-12s%s", u_buf, nick);
		}
		if (strlen(line) > 78)
			line[78] = '\0';
		prints("%s", line);
		cnt++;
		y++;
		move(y, 0);
	}
	fclose(fp);
	if (cnt == 0)
		prints("(none)\n");
	return cnt;
}

int
addtofile(filename, str)
char filename[STRLEN], str[256];
{
	FILE *fp;
	int rc;

	if ((fp = fopen(filename, "a")) == NULL)
		return -1;
	flock(fileno(fp), LOCK_EX);
	rc = fprintf(fp, "%s\n", str);
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return (rc == EOF ? -1 : 1);
}

int
addtooverride(uident)
char *uident;
{
	struct override tmp;
	int n;
	char buf[STRLEN];
	char desc[5];

	memset(&tmp, 0, sizeof (tmp));
	if (friendflag) {
		setuserfile(buf, "friends");
		n = MAXFRIENDS;
		strcpy(desc, "∫√”—");
	} else {
		setuserfile(buf, "rejects");
		n = MAXREJECTS;
		strcpy(desc, "ªµ»À");
	}
	if (get_num_records(buf, sizeof (struct override)) >= n) {
		move(t_lines - 2, 0);
		clrtoeol();
		prints("±ß«∏£¨±æ’æƒø«∞Ωˆø…“‘…Ë∂® %d ∏ˆ%s, «Î∞¥»Œ∫Œº˛ºÃ–¯...", n,
		       desc);
		igetkey();
		move(t_lines - 2, 0);
		clrtoeol();
		return -1;
	} else {
		if (friendflag) {
			if (myfriend(searchuser(uident))) {
				sprintf(buf, "%s “—‘⁄∫√”—√˚µ•", uident);
				show_message(buf);
				return -1;
			}
		} else
		    if (search_record
			(buf, &tmp, sizeof (tmp), (void *) cmpfnames,
			 uident) > 0) {
			sprintf(buf, "%s “—‘⁄ªµ»À√˚µ•", uident);
			show_message(buf);
			return -1;
		}
	}
	if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS
	    && uinfo.mode != FRIEND) n = 2;
	else
		n = t_lines - 2;

	strcpy(tmp.id, uident);
	move(n, 0);
	clrtoeol();
	sprintf(genbuf, "«Î ‰»Î∏¯%s°æ%s°øµƒÀµ√˜: ", desc, tmp.id);
	getdata(n, 0, genbuf, tmp.exp, 40, DOECHO, YEA);

	n = append_record(buf, &tmp, sizeof (struct override));
	if (n != -1)
		(friendflag) ? getfriendstr() : getrejectstr();
	else
		errlog("append override error");
	return n;
}

int
del_from_file(filename, str)
char filename[STRLEN], str[STRLEN];
{
	FILE *fp, *nfp;
	int deleted = NA;
	char fnnew[STRLEN];

	if ((fp = fopen(filename, "r")) == NULL)
		return -1;
	sprintf(fnnew, "%s.%d", filename, getuid());
	if ((nfp = fopen(fnnew, "w")) == NULL)
		return -1;
	while (fgets(genbuf, STRLEN, fp) != NULL) {
		if ((strncmp(genbuf, str, strlen(str)) == 0)
		    && (genbuf[strlen(str)] <= 32))
			deleted = YEA;
		else		/*if (*genbuf > ' ') */
			fputs(genbuf, nfp);
	}
	fclose(fp);
	fclose(nfp);
	if (!deleted)
		return -1;
	return (rename(fnnew, filename) + 1);
}

int
deleteoverride(uident, filename)
char *uident;
char *filename;
{
	int deleted;
	struct override fh;
	char buf[STRLEN];

	setuserfile(buf, filename);
	deleted =
	    search_record(buf, &fh, sizeof (fh), (void *) cmpfnames, uident);
	if (deleted > 0) {
		if (delete_record(buf, sizeof (fh), deleted) != -1) {
			(friendflag) ? getfriendstr() : getrejectstr();
		} else {
			deleted = -1;
			errlog("delete override error");
		}
	}
	return (deleted > 0) ? 1 : -1;
}

static int
override_title()
{
	char desc[5];

	if (chkmail())
		strcpy(genbuf, "[ƒ˙”––≈º˛]");
	else
		strcpy(genbuf, MY_BBS_NAME);
	if (friendflag) {
		showtitle("[±‡º≠∫√”—√˚µ•]", genbuf);
		strcpy(desc, "∫√”—");
	} else {
		showtitle("[±‡º≠ªµ»À√˚µ•]", genbuf);
		strcpy(desc, "ªµ»À");
	}
	prints
	    (" [[1;32m°˚[m,[1;32me[m] ¿Îø™ [[1;32mh[m] «Û÷˙ [[1;32m°˙[m,[1;32mRtn[m] %sÀµ√˜µµ [[1;32m°¸[m,[1;32m°˝[m] —°‘Ò [[1;32ma[m] ‘ˆº”%s [[1;32md[m] …æ≥˝%s\n",
	     desc, desc, desc);
	prints
	    ("[1;44m ±‡∫≈  %s¥˙∫≈      %sÀµ√˜                                                   [m\n",
	     desc, desc);
	return 0;
}

static char *
override_doentry(ent, fh, buf)
int ent;
struct override *fh;
char buf[512];
{
	sprintf(buf, " %4d  %-12.12s  %s", ent, fh->id, fh->exp);
	return buf;
}

static int
override_edit(ent, fh, direc)
int ent;
struct override *fh;
char *direc;
{
	struct override nh;
	char buf[STRLEN / 2];
	int pos;

	pos =
	    search_record(direc, &nh, sizeof (nh), (void *) cmpfnames, fh->id);
	move(t_lines - 2, 0);
	clrtoeol();
	if (pos > 0) {
		sprintf(buf, "«Î ‰»Î %s µƒ–¬%sÀµ√˜: ", fh->id,
			(friendflag) ? "∫√”—" : "ªµ»À");
		getdata(t_lines - 2, 0, buf, nh.exp, 40, DOECHO, NA);
	}
	if (substitute_record(direc, &nh, sizeof (nh), pos) < 0)
		errlog("Override files subs err");
	move(t_lines - 2, 0);
	clrtoeol();
	return NEWDIRECT;
}

int
override_add()
{
	char uident[13];

	clear();
	move(1, 0);
	usercomplete("«Î ‰»Î“™‘ˆº”µƒ¥˙∫≈: ", uident);
	if (uident[0] != '\0') {
		if (searchuser(uident) <= 0) {
			move(2, 0);
			prints("¥ÌŒÛµƒ π”√’ﬂ¥˙∫≈...");
			pressanykey();
			return FULLUPDATE;
		} else
			addtooverride(uident);
	}
	prints("\n∞— %s º”»Î%s√˚µ•÷–...", uident,
	       (friendflag) ? "∫√”—" : "ªµ»À");
	pressanykey();
	return FULLUPDATE;
}

static int
override_dele(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	char buf[STRLEN];
	char desc[5];
	char fname[10];
	int deleted = NA;

	if (friendflag) {
		strcpy(desc, "∫√”—");
		strcpy(fname, "friends");
	} else {
		strcpy(desc, "ªµ»À");
		strcpy(fname, "rejects");
	}
	saveline(t_lines - 2, 0, NULL);
	move(t_lines - 2, 0);
	sprintf(buf, " «∑Ò∞—°æ%s°ø¥”%s√˚µ•÷–»•≥˝", fh->id, desc);
	if (askyn(buf, NA, NA) == YEA) {
		move(t_lines - 2, 0);
		clrtoeol();
		if (deleteoverride(fh->id, fname) == 1) {
			prints("“—¥”%s√˚µ•÷–“∆≥˝°æ%s°ø,∞¥»Œ∫Œº¸ºÃ–¯...", desc,
			       fh->id);
			deleted = YEA;
		} else
			prints("’“≤ªµΩ°æ%s°ø,∞¥»Œ∫Œº¸ºÃ–¯...", fh->id);
	} else {
		move(t_lines - 2, 0);
		clrtoeol();
		prints("»°œ˚…æ≥˝%s...", desc);
	}
	igetkey();
	move(t_lines - 2, 0);
	clrtoeol();
	saveline(t_lines - 2, 1, NULL);
	return (deleted) ? PARTUPDATE : DONOTHING;
}

static int
friend_edit(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	friendflag = YEA;
	return override_edit(ent, fh, direct);
}

static int
friend_add(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	friendflag = YEA;
	return override_add();
}

static int
friend_dele(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	friendflag = YEA;
	return override_dele(ent, fh, direct);
}

static int
friend_mail(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	if (!HAS_PERM(PERM_POST))
		return DONOTHING;
	m_send(fh->id);
	return FULLUPDATE;
}

static int
friend_query(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	int ch;

	if (t_query(fh->id) == -1)
		return FULLUPDATE;
	move(t_lines - 1, 0);
	clrtoeol();
	prints
	    ("[0;1;44;31m[∂¡»°∫√”—Àµ√˜µµ][33m ºƒ–≈∏¯∫√”— m ©¶ Ω· ¯ Q,°˚ ©¶…œ“ªŒª °¸©¶œ¬“ªŒª <Space>,°˝      [m");
	ch = egetch();
	switch (ch) {
	case 'N':
	case 'Q':
	case 'n':
	case 'q':
	case KEY_LEFT:
		break;
	case 'm':
	case 'M':
		m_send(fh->id);
		break;
	case ' ':
	case 'j':
	case KEY_RIGHT:
	case KEY_DOWN:
	case KEY_PGDN:
		return READ_NEXT;
	case KEY_UP:
	case KEY_PGUP:
		return READ_PREV;
	default:
		break;
	}
	return FULLUPDATE;
}

static int
friend_help()
{
	show_help("help/friendshelp");
	return FULLUPDATE;
}

static int
reject_edit(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	friendflag = NA;
	return override_edit(ent, fh, direct);
}

static int
reject_add(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	friendflag = NA;
	return override_add();
}

static int
reject_dele(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	friendflag = NA;
	return override_dele(ent, fh, direct);
}

static int
reject_query(ent, fh, direct)
int ent;
struct override *fh;
char *direct;
{
	int ch;

	if (t_query(fh->id) == -1)
		return FULLUPDATE;
	move(t_lines - 1, 0);
	clrtoeol();
	prints
	    ("[0;1;44;31m[∂¡»°ªµ»ÀÀµ√˜µµ][33m Ω· ¯ Q,°˚ ©¶…œ“ªŒª °¸©¶œ¬“ªŒª <Space>,°˝                      [m");
	ch = egetch();
	switch (ch) {
	case 'N':
	case 'Q':
	case 'n':
	case 'q':
	case KEY_LEFT:
		break;
	case ' ':
	case 'j':
	case KEY_RIGHT:
	case KEY_DOWN:
	case KEY_PGDN:
		return READ_NEXT;
	case KEY_UP:
	case KEY_PGUP:
		return READ_PREV;
	default:
		break;
	}
	return FULLUPDATE;
}

static int
reject_help()
{
	show_help("help/rejectshelp");
	return FULLUPDATE;
}

void
t_friend()
{
	char buf[STRLEN];

	friendflag = YEA;
	setuserfile(buf, "friends");
	i_read(GMENU, buf, override_title, (void *) override_doentry,
	       friend_list, sizeof (struct override));
	clear();
	return;
}

void
t_reject()
{
	char buf[STRLEN];

	friendflag = NA;
	setuserfile(buf, "rejects");
	i_read(GMENU, buf, override_title, (void *) override_doentry,
	       reject_list, sizeof (struct override));
	clear();
	return;
}

struct user_info *
t_search(sid, pid, invisible_check)
char *sid;
int pid;
int invisible_check;
{
	int i;
	extern struct UTMPFILE *utmpshm;
	struct user_info *cur, *tmp = NULL;

	resolve_utmp();
	for (i = 0; i < USHM_SIZE; i++) {
		cur = &(utmpshm->uinfo[i]);
		if (!cur->active || !cur->pid)
			continue;
		if (!strcasecmp(cur->userid, sid)) {
			if ((pid == 0))
				return (isreject(cur)
					|| (invisible_check
					    && (cur->invisible
						&& !HAS_PERM(PERM_SEECLOAK |
							     PERM_SYSOP)))) ?
				    NULL : cur;
			tmp = cur;
			if (pid == cur->pid)
				break;
		}
	}
	/* by gluon
	   if(tmp != NULL)
	   {
	   if (tmp->invisible && !HAS_PERM(PERM_SEECLOAK|PERM_SYSOP))
	   return NULL;
	   }
	 */
	return isreject(cur) ? NULL : tmp;
}

static int
cmpfuid(a, b)
unsigned *a, *b;
{
	return *a - *b;
}

int
getfriendstr()
{
	int i;
	struct override *tmp;

	memset(uinfo.friend, 0, sizeof (uinfo.friend));
	setuserfile(genbuf, "friends");
	uinfo.fnum = get_num_records(genbuf, sizeof (struct override));
	if (uinfo.fnum <= 0)
		return -1;
	uinfo.fnum = (uinfo.fnum >= MAXFRIENDS) ? MAXFRIENDS : uinfo.fnum;
	tmp = (struct override *) calloc(sizeof (struct override), uinfo.fnum);
	get_records(genbuf, tmp, sizeof (struct override), 1, uinfo.fnum);
	for (i = 0; i < uinfo.fnum; i++) {
		uinfo.friend[i] = searchuser(tmp[i].id);
		if (uinfo.friend[i] == 0)
			deleteoverride(tmp[i].id, "friends");
		/* À≥±„…æ≥˝“—≤ª¥Ê‘⁄’ ∫≈µƒ∫√”— */
	}
	free(tmp);
	qsort(&uinfo.friend, uinfo.fnum, sizeof (uinfo.friend[0]),
	      (void *) cmpfuid);
	update_ulist(&uinfo, utmpent);
	return 0;
}

int
getrejectstr()
{
	int nr, i;
	struct override *tmp;

	memset(uinfo.reject, 0, sizeof (uinfo.reject));
	setuserfile(genbuf, "rejects");
	nr = get_num_records(genbuf, sizeof (struct override));
	if (nr <= 0)
		return -1;
	nr = (nr >= MAXREJECTS) ? MAXREJECTS : nr;
	tmp = (struct override *) calloc(sizeof (struct override), nr);
	get_records(genbuf, tmp, sizeof (struct override), 1, nr);
	for (i = 0; i < nr; i++) {
		uinfo.reject[i] = searchuser(tmp[i].id);
		if (uinfo.reject[i] == 0)
			deleteoverride(tmp[i].id, "rejects");
	}
	free(tmp);
	return 0;
}

int
wait_friend()
{
	FILE *fp;
	int tuid;
	char buf[STRLEN];
	char uid[13];

	modify_user_mode(WFRIEND);
	clear();
	move(1, 0);
	usercomplete("«Î ‰»Î π”√’ﬂ¥˙∫≈“‘º”»ÎœµÕ≥µƒ—∞»À√˚≤·: ", uid);
	if (uid[0] == '\0') {
		clear();
		return 0;
	}
	if (!(tuid = searchuser(uid))) {
		move(2, 0);
		prints("[1m≤ª’˝»∑µƒ π”√’ﬂ¥˙∫≈[m\n");
		pressanykey();
		clear();
		return -1;
	}
	sprintf(buf, "ƒ„»∑∂®“™∞— [1m%s[m º”»ÎœµÕ≥—∞»À√˚µ•÷–", uid);
	move(2, 0);
	if (askyn(buf, YEA, NA) == NA) {
		clear();
		return -1;
	}
	if ((fp = fopen("friendbook", "a")) == NULL) {
		prints("œµÕ≥µƒ—∞»À√˚≤·Œﬁ∑®ø™∆Ù£¨«ÎÕ®÷™’æ≥§...\n");
		pressanykey();
		return -1;
	}
	sprintf(buf, "%d@%s", tuid, currentuser.userid);
	if (!seek_in_file("friendbook", buf))
		fprintf(fp, "%s\n", buf);
	fclose(fp);
	move(3, 0);
	prints("“—æ≠∞Ôƒ„º”»Î—∞»À√˚≤·÷–£¨[1m%s[m …œ’æœµÕ≥“ª∂®ª·Õ®÷™ƒ„...\n",
	       uid);
	pressanykey();
	clear();
	return 0;
}

//#ifdef TALK_LOG
/* edwardc.990106 ∑÷±Œ™¡ΩŒª¡ƒÃÏµƒ»À◊˜ºÕ¬º */
/* -=> ◊‘º∫Àµµƒª∞ */
/* --> ∂‘∑ΩÀµµƒª∞ */

static void
do_log(char *msg, int who)
{
/* Sonny.990514  ‘÷¯◊• overflow µƒŒ Ã‚... */
/* Sonny.990606 overflow Œ Ã‚Ω‚æˆ. buf[100]  «’˝»∑µƒ. ≤Œøº man sprintf() */
	time_t now;
	char buf[128];
	now = time(0);
	msg[79] = 0;
	if (msg[strlen(msg) - 1] == '\n')
		msg[strlen(msg) - 1] = 0;
	if (strlen(msg) < 1 || msg[0] == '\r' || msg[0] == '\n')
		return;

	/* ÷ª∞Ô◊‘º∫◊ˆ */
	sethomefile(buf, currentuser.userid, "talklog");

	if (!dashf(buf) || talkrec == -1) {
		talkrec = open(buf, O_RDWR | O_CREAT | O_TRUNC, 0644);
		buf[127] = 0;
		snprintf(buf, 127, "[1;32m”Î %s µƒ«Èª∞√‡√‡, »’∆⁄: %s [m\n",
			 save_page_requestor, Cdate(&now));
		write(talkrec, buf, strlen(buf));
		sprintf(buf, "\t—’…´∑÷±¥˙±Ì: [1;33m%s[m [1;36m%s[m \n\n",
			currentuser.userid, partner);
		write(talkrec, buf, strlen(buf));
	}
	if (who == 1) {		/* ◊‘º∫Àµµƒª∞ */
		sprintf(buf, "[1;33m-=> %s [m\n", msg);
		write(talkrec, buf, strlen(buf));
	} else if (who == 2) {	/* ±»ÀÀµµƒª∞ */
		sprintf(buf, "[1;36m--> %s [m\n", msg);
		write(talkrec, buf, strlen(buf));
	}
}

//#endif
static char *
Cdate(clock)
time_t *clock;
{
	static char foo[22];
	struct tm *mytm = localtime(clock);

	strftime(foo, 22, "%m/%d/%Y %T %a", mytm);
	return (foo);
}
