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
#include "bbs_global_vars.h"
#include "list.h"
#include "io.h"
#include "smth_screen.h"
#include "xyz.h"
#include "bcache.h"
#include "stuff.h"
#include "main.h"
#include "sendmsg.h"
#include "talk.h"
#include "mail.h"
#include "read.h"
#include "record.h"
#include "namecomplete.h"
#include "bbsinc.h"
#include "help.h"
#include "chat.h"
#include "announce.h"
#include "bbs-internal.h"
#include "ythtbbs/cache.h"
#include "ythtbbs/override.h"

#define M_INT 8			/* monitor mode update interval */
#define P_INT 20		/* interval to check for page req. in talk/chat */

int talkidletime = 0;
int ulistpage;
int friendflag = 1;
//#ifdef TALK_LOG
int talkrec = -1;
char partner[IDLEN + 1];
//#endif

static int show_special(char *id2);
extern void five_pk(int fd, int first); // five.c TODO

struct talk_win {
	int curcol, curln;
	int sline, eline;
};

int nowmovie;

static char *const refuse[] = {
	"��Ǹ����������ר�Ŀ� Board��    ", "�ҽ�����ۣ�������������졣    ",
	"���������£���һ���� Call �㡣  ", "������Ҫ�뿪�ˣ��´����İɡ�    ",
	"���㲻Ҫ�� Page���Ҳ�������ġ� ", "����дһ�����ҽ��ܸ��ң�����  ",
	"�Բ����������ڵ��ˡ�          ", "�벻Ҫ���ң�����..... :)      ",
	NULL
};

char save_page_requestor[STRLEN];

static char canpage(int friend, int pager);
static int listcuent(const struct user_info *uentp, void *);
static int show_user_plan(char *userid);
static int bm_printboard(struct boardmanager *bm, void *farg);
static int count_useshell(const struct user_info *uentp, void *);
static int cmpfnames(const char *userid, const struct ythtbbs_override *uv);
static int cmpunums(int unum, struct user_info *up);
static int cmpmsgnum(int unum, struct user_info *up);
static int setpagerequest(int mode);
static void do_talk_nextline(struct talk_win *twin);
static void do_talk_char(struct talk_win *twin, int ch);
static void talkflush(void);
static void moveto(int mode, struct talk_win *twin);
static int do_talk(int fd);
static int override_title(void);
static char *override_doentry(int ent, struct ythtbbs_override *fh, char buf[512]);
static int override_edit(int ent, struct ythtbbs_override *fh, char *direc);
static int override_dele(int ent, struct ythtbbs_override *fh, char *direct);
static int friend_edit(int ent, struct ythtbbs_override *fh, char *direct);
static int friend_add(int ent, struct ythtbbs_override *fh, char *direct);
static int friend_dele(int ent, struct ythtbbs_override *fh, char *direct);
static int friend_mail(int ent, struct ythtbbs_override *fh, char *direct);
static int friend_query(int ent, struct ythtbbs_override *fh, char *direct);
static int friend_help(void);
static int reject_edit(int ent, struct ythtbbs_override *fh, char *direct);
static int reject_add(int ent, struct ythtbbs_override *fh, char *direct);
static int reject_dele(int ent, struct ythtbbs_override *fh, char *direct);
static int reject_query(int ent, struct ythtbbs_override *fh, char *direct);
static int reject_help(void);
static int cmpfuid(unsigned *a, unsigned *b);
static void do_log(char *msg, int who);
static char *Cdate(time_t * clock);

struct one_key friend_list[] = {
	{'r', friend_query, "��ѯ����"},
	{'m', friend_mail, "������д��"},
	{'M', friend_mail, "������д��"},
	{'a', friend_add, "��Ӻ���"},
	{'A', friend_add, "��Ӻ���"},
	{'d', friend_dele, "ɾ������"},
	{'D', friend_dele, "ɾ������"},
	{'E', friend_edit, "�༭����"},
	{'h', friend_help, "�鿴����"},
	{'H', friend_help, "�鿴����"},
	{'\0', NULL, ""}
};

struct one_key reject_list[] = {
	{'r', reject_query, "��ѯ����"},
	{'a', reject_add, "��ӻ���"},
	{'A', reject_add, "���ӻ���"},
	{'d', reject_dele, "ɾ������"},
	{'D', reject_dele, "ɾ������"},
	{'E', reject_edit, "�༭����"},
	{'h', reject_help, "�鿴����"},
	{'H', reject_help, "�鿴����"},
	{'\0', NULL, ""}
};

static char
canpage(int friend, int pager)
{
	if ((pager & ALL_PAGER) || HAS_PERM(PERM_SYSOP | PERM_FORCEPAGE, currentuser))
		return YEA;
	if ((pager & FRIEND_PAGER)) {
		if (friend)
			return YEA;
	}
	return NA;
}

static int listcuent(const struct user_info *uentp, void *x_param) {
	(void) x_param;
	if (uentp == NULL) {
		CreateNameList();
		return 0;
	}
	if (uentp->uid == usernum)
		return 0;
	if (!uentp->active || !uentp->pid || isreject(uentp))
		return 0;
	if (!HAS_PERM(PERM_SYSOP | PERM_SEECLOAK, currentuser) && uentp->invisible)
		return 0;
	AddNameList(uentp->userid);
	return 0;
}

static void creat_list() {
	listcuent(NULL, NULL);
	ythtbbs_cache_utmp_apply(listcuent, NULL);
}

int t_pager(const char *s) {
	(void) s;

	if (uinfo.pager & ALL_PAGER) {
		uinfo.pager &= ~ALL_PAGER;
		if (DEFINE(DEF_FRIENDCALL, currentuser))
			uinfo.pager |= FRIEND_PAGER;
		else
			uinfo.pager &= ~FRIEND_PAGER;
	} else {
		uinfo.pager |= ALL_PAGER;
		uinfo.pager |= FRIEND_PAGER;
	}

	if (!uinfo.in_chat && uinfo.mode != TALK) {
		move(1, 0);
		prints("���ĺ����� (pager) �Ѿ�\033[1m%s\033[m��!", (uinfo.pager & ALL_PAGER) ? "��" : "�ر�");
		pressreturn();
	}
	update_utmp();
	return 0;
}

/*Add by SmallPig*/
/*�˺���ֻ������ӡ˵�����������������λ�����⡣*/
static int
show_user_plan(char *userid)
{
	int i;
	char pfile[STRLEN], pbuf[256];
	FILE *pf;

	sethomefile_s(pfile, sizeof(pfile), userid, "plans");
	if ((pf = fopen(pfile, "r")) == NULL) {
		prints("\033[1;36mû�и���˵����\033[m\n");
		return NA;
	} else {
		prints("\033[1;36m����˵�������£�\033[m\n");
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
int t_query(const char *q_id) {
	char uident[IDLEN + 1];
	int tuid = 0;
	int exp;		/*Add by SmallPig */
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
		prints("��ѯ˭:\n<����ʹ���ߴ���, ���հ׼����г������ִ�>\n");
		move(1, 8);
		usercomplete(NULL, uident);
		if (uident[0] == '\0') {
			return 0;
		}
	} else {
		if (*q_id == '\0')
			return 0;
		strncpy(uident, q_id, sizeof (uident));
		uident[sizeof (uident) - 1] = '\0';
		if (strchr(uident, ' '))
			strtok(uident, " ");
	}
	if (!(tuid = getuser(uident))) {
		move(2, 0);
		clrtoeol();
		prints("\033[1m����ȷ��ʹ���ߴ���\033[m\n");
		pressanykey();
		return -1;
	}
	uinfo.destuid = tuid;
	update_utmp();

	move(1, 0);
	clrtobot();
	sprintf(qry_mail_dir, "mail/%c/%s/%s", mytoupper(lookupuser.userid[0]), lookupuser.userid, DOT_DIR);

	exp = countexp(&lookupuser);
	strcpy(expbuf,charexp(exp));	//add for displaying exp type.  rbb@bmy
	prints("\033[1m%s \033[m(\033[1m%s\033[m) ����վ \033[1;33m%d\033[m �Σ������ \033[1;33m%d\033[m ƪ����",
			lookupuser.userid, lookupuser.username, lookupuser.numlogins,
			lookupuser.numposts);
	show_special(lookupuser.userid); //add by wjbta@bmy
	strcpy(planid, lookupuser.userid);
	strcpy(genbuf, lookupuser.dietime ? ytht_ctime(lookupuser.dietime) : ytht_ctime(lookupuser.lastlogin));
	if (ifinprison(lookupuser.userid)) {
		strcpy(genbuf, ytht_ctime(lookupuser.lastlogin));
		prints("\n�ڼ������̣��ϴηŷ�ʱ��[\033[1m%s\033[m]\n", genbuf);
		if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS
				&& uinfo.mode != FRIEND && uinfo.mode != GMENU)
			pressanykey();
		uinfo.destuid = 0;
		return 0;
	}

	if (lookupuser.dietime) {
		prints("\n�Ѿ��뿪������,����...\n���� [\033[1m%d\033[m] ���Ҫת��Ͷ̥��\n", countlife(&lookupuser));
		if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS && uinfo.mode != FRIEND && uinfo.mode != GMENU)
			pressanykey();
		uinfo.destuid = 0;
		return 0;
	}
	prints("\n�ϴ��� [\033[1;32m%s\033[m] �� [\033[1;32m%s\033[m] ����վһ�Ρ�\n",
	genbuf, (lookupuser.lasthost[0] == '\0' ? "(����)" : lookupuser.lasthost));
	// show_special(lookupuser.userid); //add by wjbta@bmy
	if(HAS_PERM(PERM_SYSOP | PERM_SEECLOAK, currentuser)){		//add by mintbaggio@BMY
		if(lookupuser.userlevel&PERM_CLOAK)
			strcpy(genbuf, (lookupuser.lastlogout>=lookupuser.lastlogin) ? (user_isonline(lookupuser.userid) ? "�������ϻ��������߲���" : ytht_ctime(lookupuser.lastlogout)) : "�������ϻ��������߲���");
		else
			strcpy(genbuf, (lookupuser.lastlogout>=lookupuser.lastlogin) ? ytht_ctime(lookupuser.lastlogout) : "�������ϻ��������߲���");
	}
	else
		strcpy(genbuf, (lookupuser.lastlogout>=lookupuser.lastlogin) ? ytht_ctime(lookupuser.lastlogout) : "�������ϻ��������߲���");

	prints("��վʱ�䣺[\033[1;36m%s\033[m] ���䣺[\033[1;5m%2s\033[m]����������[\033[1;32m%d\033[m] ���䣺[\033[1;32m%d\033[m]�졣\n",
			genbuf, (check_query_mail(qry_mail_dir) == 1) ? "��" : "  ",
			countlife(&lookupuser),
			(strcmp(lookupuser.userid, "SYSOP") == 0) ? 9999 : ((time(0)-lookupuser.firstlogin)/86400));//by bjgyt add networkage
	if (lookupuser.userlevel & (PERM_BOARDS | PERM_ARBITRATE | PERM_OBOARDS | PERM_SYSOP | PERM_ACCOUNTS | PERM_WELCOME | PERM_SPECIAL7 | PERM_SPECIAL4)) {
		prints("���ΰ���");
		sethomefile_s(path, sizeof(path), lookupuser.userid, "mboard");
		new_apply_record(path, sizeof (struct boardmanager), (void *) bm_printboard, NULL);
		// show_special(lookupuser.userid); //add by wjbta@bmy
		if (!strcmp(lookupuser.userid, "SYSOP") || !strcmp(lookupuser.userid, "lanboy"))
			prints("[\033[1;36mϵͳ����Ա\033[m]");
		else if ((lookupuser.userlevel&PERM_SYSOP) && (lookupuser.userlevel&PERM_ARBITRATE) )
			prints("[\033[1;36m��վ������\033[m]");
		else if (lookupuser.userlevel & PERM_SYSOP)
			prints("[\033[1;36m����վ��\033[m]");
		else if (lookupuser.userlevel & PERM_OBOARDS)
			prints("[\033[1;36mʵϰվ��\033[m]");
		else if (lookupuser.userlevel & PERM_ARBITRATE)
			prints("[\033[1;36m���μ�ί\033[m]");
		else if (lookupuser.userlevel & PERM_SPECIAL4)
			prints("[\033[1;36m����\033[m]");
		else if (lookupuser.userlevel & PERM_WELCOME)
			prints("[\033[1;36mϵͳ����\033[m]");
		else if (lookupuser.userlevel & PERM_SPECIAL7) {
			// ������&&!����
			if ((lookupuser.userlevel & PERM_SPECIAL1) && !(lookupuser.userlevel & PERM_CLOAK))
				prints("[\033[1;36m���γ���Ա\033[m]");
			else
				prints("[\033[1;36m�������Ա\033[m]");
		}
		else if (lookupuser.userlevel & PERM_ACCOUNTS)
			prints ("[\033[1;36m�ʺŹ���Ա\033[m]");
		prints(" ");
	}
	prints("����[\033[1;36m%s\033[m]",expbuf); //add for displaying exp type rbb@bmy
	prints("%s","                                              ");
	prints("\n");
	t_search_ulist(t_cmpuids, tuid);
#if defined(QUERY_REALNAMES)
	// if (HAS_PERM(PERM_SYSOP))  by bjgyt
		//prints("��ʵ����: %s \n", lookupuser.realname);
	if (HAS_PERM(PERM_ACCOUNTS, currentuser)) {
		char secu[35];
		size_t num;

		strcpy(secu, "bTCPRD#@XWBA#VS-DOM-F0s23456789H");
		for (num = 0; num < strlen(secu); num++)
			if (!(lookupuser.userlevel & (1 << num)))
				secu[num] = '-';
		prints("��ʵ����: %s (Ȩ��: %s )\n", lookupuser.realname, secu);
	}
#endif
	show_user_plan(planid);
	if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS && uinfo.mode != FRIEND && uinfo.mode != GMENU){
		int ch, oldmode;
		char buf[STRLEN];
		oldmode = uinfo.mode;
		move(t_lines - 1, 0);
		prints("\033[m\033[44m\033[1;37m����[\033[1;32mm\033[1;37m] ��ѶϢ[\033[1;32ms\033[1;37m] ��,������[\033[1;32mo\033[1;37m,\033[1;32md\033[1;37m] ����˵����[\033[1;32ma\033[1;37m] �����ļ�[\033[1;32mx\033[1;37m] ����������");
		clrtoeol();
		resetcolor();
		ch = igetkey();
		switch (toupper(ch)) {
		case 'S':
			if (strcmp(uident, "guest") && !HAS_PERM(PERM_PAGE, currentuser))
				break;
			do_sendmsg(uident, 0, NULL, 2, 0);
			break;
		case 'M':
			if (!HAS_PERM(PERM_POST, currentuser))
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
				sprintf(buf, "%s ���ں�������", uident);
			else
				sprintf(buf, "%s �����������", uident);
			move(t_lines - 1, 0);
			clrtoeol();
			prints("%s", buf);
			refresh();
			sleep(1);
			break;
		case 'D':
			if (!strcmp("guest", currentuser.userid))
				break;
			sprintf(buf, "ȷ��Ҫ�� %s �Ӻ�������ɾ���� (Y/N) [N]: ", uident);
			move(t_lines - 1, 0);
			clrtoeol();
			getdata(t_lines - 1, 0, buf, genbuf, 4, DOECHO, YEA);
			move(t_lines - 1, 0);
			clrtoeol();
			if (genbuf[0] != 'Y' && genbuf[0] != 'y')
				break;
			if (deleteoverride(uident, YTHTBBS_OVERRIDE_FRIENDS) == -1)
				sprintf(buf, "%s �����Ͳ��ں���������", uident);
			else
				sprintf(buf, "%s �ѴӺ��������Ƴ�", uident);
			move(t_lines - 1, 0);
			clrtoeol();
			prints("%s", buf);
			refresh();
			sleep(1);
			break;
		case 'A':
			move(0, 0);
			clrtobot();
			prints("��ѯ����״̬ (����˵����)\n");
			sprintf(qry_mail_dir, "mail/%c/%s/%s", mytoupper(lookupuser.userid[0]), lookupuser.userid, DOT_DIR);
			exp = countexp(&lookupuser);
			prints("\033[1m%s \033[m(\033[1m%s\033[m) ����վ \033[1;33m%d\033[m �Σ������ \033[1;33m%d\033[m ƪ����",
					lookupuser.userid, lookupuser.username, lookupuser.numlogins,
					lookupuser.numposts);
			show_special(lookupuser.userid); //add by wjbta@bmy
			strcpy(planid, lookupuser.userid);
			strcpy(genbuf, lookupuser.dietime ? ytht_ctime(lookupuser.dietime) : ytht_ctime(lookupuser.lastlogin));
			if (ifinprison(lookupuser.userid)) {
				strcpy(genbuf, ytht_ctime(lookupuser.lastlogin));
				prints("\n�ڼ������̣��ϴηŷ�ʱ��[\033[1m%s\033[m]\n", genbuf);
				if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS && uinfo.mode != FRIEND && uinfo.mode != GMENU)
					pressanykey();
				uinfo.destuid = 0;
				break;
			}
			if (lookupuser.dietime) {
				prints("\n�Ѿ��뿪������,����...\n���� [\033[1m%d\033[m] ���Ҫת��Ͷ̥��\n", countlife(&lookupuser));
				if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS && uinfo.mode != FRIEND && uinfo.mode != GMENU)
					pressanykey();
				uinfo.destuid = 0;
				break;
			}
			prints("\n�ϴ��� [\033[1;32m%s\033[m] �� [\033[1;32m%s\033[m] ����վһ�Ρ�\n",
				genbuf, (lookupuser.lasthost[0] =='\0' ? "(����)" : lookupuser.lasthost));//add for displaying exp type rbb@bmy
			if(HAS_PERM(PERM_SYSOP | PERM_SEECLOAK, currentuser)){		//add by mintbaggio@BMY
				if(lookupuser.userlevel&PERM_CLOAK)
					strcpy(genbuf, (lookupuser.lastlogout>=lookupuser.lastlogin) ? (user_isonline(lookupuser.userid)? "�������ϻ��������߲���": ytht_ctime(
							lookupuser.lastlogout)) : "�������ϻ��������߲���");
				else
					strcpy(genbuf, (lookupuser.lastlogout>=lookupuser.lastlogin) ? ytht_ctime(lookupuser.lastlogout) : "�������ϻ��������߲���");
			}
			else
				strcpy(genbuf, (lookupuser.lastlogout>=lookupuser.lastlogin) ? ytht_ctime(lookupuser.lastlogout) : "�������ϻ��������߲���");
			prints ("��վʱ�䣺[\033[1;36m%s\033[m] ���䣺[\033[1;5m%2s\033[m]����������[\033[1;32m%d\033[m] ���䣺[\033[1;32m%d\033[m]�졣\n",
					genbuf, (check_query_mail(qry_mail_dir) == 1) ? "��" : "  ",
					countlife(&lookupuser),
					(strcmp(lookupuser.userid, "SYSOP") == 0) ? 9999 : ((time(0)-lookupuser.firstlogin)/86400));//by bjgyt add networkage
			if (lookupuser.userlevel & (PERM_BOARDS | PERM_ARBITRATE
				| PERM_OBOARDS | PERM_SYSOP | PERM_ACCOUNTS | PERM_WELCOME
				| PERM_SPECIAL7 | PERM_SPECIAL4)) {
				prints("���ΰ���");
				sethomefile_s(path, sizeof(path), lookupuser.userid, "mboard");
				new_apply_record(path, sizeof (struct boardmanager), (void *) bm_printboard, NULL);
			if (lookupuser.userlevel & !strcmp(currentuser.userid, "SYSOP")) prints("[\033[1;36mϵͳ����Ա\033[m]");
			else if (lookupuser.userlevel & !strcmp(currentuser.userid, "lanboy")) prints("[\033[1;36mϵͳ����Ա\033[m]");
			else if ((lookupuser.userlevel&PERM_SYSOP) && (lookupuser.userlevel&PERM_ARBITRATE) )	prints("[\033[1;36m��վ������\033[m]");
			else if (lookupuser.userlevel & PERM_SYSOP)	prints("[\033[1;36m����վ��\033[m]");
			else if (lookupuser.userlevel & PERM_OBOARDS)   prints("[\033[1;36mʵϰվ��\033[m]");
			else if (lookupuser.userlevel & PERM_ARBITRATE)	prints("[\033[1;36m���μ�ί\033[m]");
			else if (lookupuser.userlevel & PERM_SPECIAL4)	prints("[\033[1;36m����\033[m]");
			else if (lookupuser.userlevel & PERM_WELCOME) prints("[\033[1;36mϵͳ����\033[m]");
			else if (lookupuser.userlevel & PERM_SPECIAL7)
			{
				// ������&&!����
				if ( (lookupuser.userlevel & PERM_SPECIAL1) && !(lookupuser.userlevel & PERM_CLOAK) )
					prints("[\033[1;36m���γ���Ա\033[m]");
				else
					prints("[\033[1;36m�������Ա\033[m]");
			}
			else if (lookupuser.userlevel & PERM_ACCOUNTS) prints ("[\033[1;36m�ʺŹ���Ա\033[m]");
			prints(" ");
			}
			prints("����[\033[1;36m%s\033[m]",expbuf);
			prints ("%s","                                              ");
			prints("\n");
			t_search_ulist(t_cmpuids, tuid);
			if (HAS_PERM(PERM_ACCOUNTS, currentuser)) {
				char secu[35];
				size_t num;

				strcpy(secu, "bTCPRD#@XWBA#VS-DOM-F0s23456789H");
				for (num = 0; num < strlen(secu); num++)
					if (!(lookupuser.userlevel & (1 << num)))
						secu[num] = '-';
				prints("��ʵ����: %s (Ȩ��: %s )\n", lookupuser.realname, secu);
			}
			pressanykey();
			break;
		}
		uinfo.mode = oldmode;
	}
	uinfo.destuid = 0;
	return 0;
}


//add by wjbta@bmy
static int show_special(char *id2) {
	FILE *fp;
	char id1[80], name[80], buf[256];
	fp=fopen("etc/special", "r");
	if(fp==0) return 0;
	while(1) {
		if(fgets(buf, 256, fp)==0) break;
		if(sscanf(buf, "%s %s", id1, name)<2) continue;
		if(!strcasecmp(id1, id2)) prints("\033[1;33m��\033[36m%s\033[1;33m��\033[m",name);
	}
	fclose(fp);

	return 0;
}//add by wjbta@bmy

static int bm_printboard(struct boardmanager *bm, void *farg) {
	(void) farg;
	if (canberead(bm->board))
		prints("%s ", bm->board);
	return 0;
}

static int count_useshell(const struct user_info *uentp, void *x_param) {
	int *p_i = x_param;
	if (!uentp->active || !uentp->pid)
		return 0;
	if (uentp->mode == WWW || uentp->mode == SYSINFO
			|| uentp->mode == HYTELNET || uentp->mode == DICT
			|| uentp->mode == ARCHIE || uentp->mode == IRCCHAT
			|| uentp->mode == BBSNET || uentp->mode == GAME)
		*p_i = *p_i + 1;
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
		count_friends += (ythtbbs_cache_UserTable_query_user_by_uid(currentuser.userid, HAS_PERM(PERM_SYSOP | PERM_SEECLOAK, currentuser), uinfo.friend[i], true) != NULL) ? 1 : 0;
	count_users = ythtbbs_cache_utmp_count_active();
}

int
num_useshell()
{
	int count = 0;
	ythtbbs_cache_utmp_apply(count_useshell, &count);
	return count;
}

static int cmpfnames(const char *userid, const struct ythtbbs_override *uv) {
	return !strcmp(userid, uv->id);
}

int
t_cmpuids(int uid, struct user_info *up)
{
	return (up->active && uid == up->uid);
}

int t_talk(const char *s) {
	(void) s;
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

int ttt_talk(const struct user_info *userinfo) {
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
		prints("��Ǹ, �˹���������״̬�²���ִ��...\n");
		pressreturn();
		return 0;
	}
	if (userinfo == NULL) {
		move(2, 0);
		prints("<����ʹ���ߴ���>\n");
		move(1, 0);
		clrtoeol();
		prints("��˭����: ");
		creat_list();
		namecomplete(NULL, uident);
		if (uident[0] == '\0') {
			clear();
			return 0;
		}
		if (!(tuid = ythtbbs_cache_UserTable_search_usernum(uident)) || tuid == usernum) {
wrongid:
			move(2, 0);
			prints("�������\n");
			pressreturn();
			move(2, 0);
			clrtoeol();
			return -1;
		}
		ucount = count_logins(&uin, t_cmpuids, tuid, 0);
		move(3, 0);
		prints("Ŀǰ %s �� %d logins ����: \n", uident, ucount);
		clrtobot();
		if (ucount > 1) {
list:
			move(5, 0);
			prints("(0) �������ˣ������ˡ�\n");
			ucount = count_logins(&uin, t_cmpuids, tuid, 0);
			count_logins(&uin, t_cmpuids, tuid, 1);
			clrtobot();
			tmp = ucount + 8;
			getdata(tmp, 0, "��ѡһ���㿴�ıȽ�˳�۵� [0]: ", genbuf, 4, DOECHO, YEA);
			unum = atoi(genbuf);
			if (unum == 0) {
				clear();
				return 0;
			}
			if (unum > ucount || unum < 0) {
				move(tmp, 0);
				prints("��������ѡ��������\n");
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
		prints("��˭����: %s", uin.userid);
	}
	/* youzi : check guest */
	if (!strcmp(uin.userid, "guest") && !HAS_PERM(PERM_FORCEPAGE, currentuser))
		return -1;

	/*  check if pager on/off       --gtv */
	if (!canpage(hisfriend(&uin), uin.pager)) {
		move(2, 0);
		prints("�Է��������ѹر�.\n");
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
		prints("Ŀǰ�޷�����.\n");
		clrtobot();
		pressreturn();
		return -1;
	}
	if (!uin.active || uin.pid <= 0 || uin.pid == 1 || (kill(uin.pid, 0) == -1)) {
		move(2, 0);
		if (uin.active && uin.pid == 1)
			prints("�Է���WWW���ߣ��޷�����.\n");
		else
			prints("�Է����뿪\n");
		pressreturn();
		move(2, 0);
		clrtoeol();
		return -1;
	} else {
		int sock, msgsock;
		socklen_t length;
		struct sockaddr_in server;
		char c, answer[2] = "";
		char buf[512];

		move(3, 0);
		clrtobot();
		show_user_plan(uident);
		getdata(2, 0, "���ҶԷ�̸���밴'y'(Y/N)[N]:", answer, 4, DOECHO, YEA);
		if (*answer != 'y') {
			clear();
			return 0;
		}
		five = 0;
		if (five == 1)
			sprintf(buf, "%s five %s", currentuser.userid, uident);
		else
			sprintf(buf, "%s talk %s", currentuser.userid, uident);

		newtrace(buf);
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock < 0) {
			perror("socket err\n");
			return -1;
		}

		server.sin_family = AF_INET;
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = 0;
		if (bind(sock, (struct sockaddr *) &server, sizeof (server)) < 0) {
			perror("bind err");
			close(sock);
			return -1;
		}
		length = sizeof (server);
		if (getsockname(sock, (struct sockaddr *) &server, &length) < 0) {
			perror("socket name err");
			close(sock);
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
		ytht_strsncpy(partner, uin.userid, sizeof(partner));
//#endif

		kill(uin.pid, SIGUSR1);
		clear();
		prints("���� %s ��...\n���� Ctrl-D ����\n", uident);

		listen(sock, 1);
		add_io(sock, 20);
		while (YEA) {
			int ch;
			ch = igetkey();
			if (ch == I_TIMEOUT) {
				move(0, 0);
				add_io(0, 0);
				add_io(sock, 20);
				prints("�ٴκ���.\n");
				bell();
				if (kill(uin.pid, SIGUSR1) == -1) {
					move(0, 0);
					prints("�Է�������\n");
					pressreturn();
					/*Add by SmallPig 2 lines */
					uinfo.sockactive = NA;
					uinfo.destuid = 0;
					close(sock);
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

		msgsock = accept(sock, (struct sockaddr *) 0, (unsigned int *) 0);
		if (msgsock == -1) {
			perror("accept");
			close(sock);
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
			prints("%s (%s)˵��%s\n", uin.userid, uin.username, refuse[0]);
			pressreturn();
			break;
		case 'b':
		case 'B':
			prints("%s (%s)˵��%s\n", uin.userid, uin.username, refuse[1]);
			pressreturn();
			break;
		case 'c':
		case 'C':
			prints("%s (%s)˵��%s\n", uin.userid, uin.username, refuse[2]);
			pressreturn();
			break;
		case 'd':
		case 'D':
			prints("%s (%s)˵��%s\n", uin.userid, uin.username, refuse[3]);
			pressreturn();
			break;
		case 'e':
		case 'E':
			prints("%s (%s)˵��%s\n", uin.userid, uin.username, refuse[4]);
			pressreturn();
			break;
		case 'f':
		case 'F':
			prints("%s (%s)˵��%s\n", uin.userid, uin.username, refuse[5]);
			pressreturn();
			break;
		case 'g':
		case 'G':
			prints("%s (%s)˵��%s\n", uin.userid, uin.username, refuse[6]);
			pressreturn();
			break;
		case 'n':
		case 'N':
			prints("%s (%s)˵��%s\n", uin.userid, uin.username, refuse[7]);
			pressreturn();
			break;
		case 'm':
		case 'M':
			read(msgsock, reason, sizeof (reason));
			prints("%s (%s)˵��%s\n", uin.userid, uin.username, reason);
			pressreturn();
			break;
		default:
			sprintf(save_page_requestor, "%s (%s)", uin.userid, uin.username);
//#ifdef TALK_LOG
			ytht_strsncpy(partner, uin.userid, sizeof(partner));
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
cmpunums(int unum, struct user_info *up)
{
	if (!up->active)
		return 0;
	return (unum == up->destuid);
}

static int
cmpmsgnum(int unum, struct user_info *up)
{
	if (!up->active)
		return 0;
	return (unum == up->destuid && up->sockactive == 2);
}

static int
setpagerequest(int mode)
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
servicepage(int line, char *mesg)
{
	static time_t last_check;
	time_t now;
	char buf[STRLEN * 2];
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
				sprintf(buf, "** %s ��ֹͣ����.", page_requestor);
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
			if (!page_requestor[0] && setpagerequest(0 /*For Talk */ ))
				return NA;
			else
				switch (uinfo.mode) {
				case TALK:
					move(line, 0);
					sprintf(buf, "** %s ���ں�����", page_requestor);
					printdash(buf);
					break;
				default:	/* chat */
					sprintf(buf, "** %s ���ں�����", page_requestor);
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
	prints("(A)��%s��(B)��%s��\n", refuse[0], refuse[1]);
	prints("(C)��%s��(D)��%s��\n", refuse[2], refuse[3]);
	prints("(E)��%s��(F)��%s��\n", refuse[4], refuse[5]);
	prints("(G)��%s��(N)��%s��\n", refuse[6], refuse[7]);
	prints("(M)�����Ը� %-13s            ��\n", page_requestorid);

/* modified by djq ,99.07.19, for FIVE */
/*
	sprintf( inbuf, "����� %s ��������? (Y N A B C D E F G M)[Y]: ", page_requestor );
*/
	sprintf(inbuf, "����� %s %s����ѡ��(Y/N/A/B/C/D)[Y] ",
		page_requestor, (five) ? "��������" : "������");

	strcpy(save_page_requestor, page_requestor);
//#ifdef TALK_LOG
	ytht_strsncpy(partner, page_requestorid, sizeof(partner));
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
		close(a);
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
		getdata(1, 0, "������", reason, 50, DOECHO, YEA);
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
do_talk_nextline(struct talk_win *twin)
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
do_talk_char(struct talk_win *twin, int ch)
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
moveto(int mode, struct talk_win *twin)
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
do_talk(int fd)
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
	sprintf(mid_line, " %s (%s) �� %s ���ڳ�̸��",
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
		if (ch == '\033') {
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
				/* Sonny.990514 add an robust and fix some logic problem */
				/* Sonny.990606 change to different algorithm and fix the existing do_log() overflow problem */
				else if (isprint2(data[i])) {
					if (ilen >= 80) {
						itswords[79] = '\0';
						(void) do_log(itswords, 2);
						ilen = 0;
					} else {
						itswords[ilen] = data[i];
						ilen++;
					}
				} else if ((data[i] == Ctrl('H') || data[i] == '\177') && !ilen) {
					itswords[ilen--] = '\0';
				} else if (data[i] == Ctrl('M') || data[i] == '\r' || data[i] == '\n') {
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
			if (isprint2(ch) || ch == Ctrl('H') || ch == '\177' || ch == Ctrl('G') || ch == Ctrl('M')) {
				talkobuf[talkobuflen++] = ch;
				if (talkobuflen == 80)
					talkflush();
//#ifdef TALK_LOG
				if (mlen < 80) {
					if ((ch == Ctrl('H') || ch == '\177') && mlen != 0) {
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
			} else if (ch == Ctrl('P') && HAS_PERM(PERM_BASIC, currentuser)) {
				t_pager(NULL);
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
	/* edwardc.990106 �����¼ */
	mywords[mlen] = '\0';
	itswords[ilen] = '\0';
	if (mywords[0] != '\0')
		do_log(mywords, 1);
	if (itswords[0] != '\0')
		do_log(itswords, 2);

	now = time(0);
	sprintf(talkbuf, "\n\033[1;34mͨ������, ʱ��: %s \033[m\n", Cdate(&now));
	write(talkrec, talkbuf, strlen(talkbuf));
	close(talkrec);

	sethomefile_s(genbuf, sizeof(genbuf), currentuser.userid, "talklog");
	if (!dashf(genbuf))
		return 0;

	getdata(23, 0, "�Ƿ�Ļ������¼ [Y/n]: ", ans, 2, DOECHO, YEA);

	switch (ans[0]) {
	case 'n':
	case 'N':
		break;
	default:
		sethomefile_s(talkbuf, sizeof(talkbuf), currentuser.userid, "talklog");
		sprintf(mywords, "�� %s �������¼ [%s]", partner, Cdate(&now) + 4);
		mail_file(talkbuf, currentuser.userid, mywords);
	}
	sethomefile_s(talkbuf, sizeof(talkbuf), currentuser.userid, "talklog");
	unlink(talkbuf);
//#endif
	return 0;
}

int
listfilecontent(char *fname)
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
			if (askyn("�Ƿ�����ۿ���һ��?", 1, 1)) {
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

int addtooverride(const char *uident) {
	struct ythtbbs_override tmp;
	int n;
	char buf[STRLEN];
	char desc[5];

	memset(&tmp, 0, sizeof (tmp));
	if (friendflag) {
		n = MAXFRIENDS;
		snprintf(desc, sizeof desc, "����");
	} else {
		n = MAXREJECTS;
		snprintf(desc, sizeof desc, "����");
	}

	if (ythtbbs_override_count(currentuser.userid, friendflag ? YTHTBBS_OVERRIDE_FRIENDS : YTHTBBS_OVERRIDE_REJECTS) >= n) {
		move(t_lines - 2, 0);
		clrtoeol();
		prints("��Ǹ����վĿǰ�������趨 %d ��%s, �밴�κμ�����...", n, desc);
		igetkey();
		move(t_lines - 2, 0);
		clrtoeol();
		return -1;
	} else {
		if (ythtbbs_override_included(currentuser.userid, friendflag ? YTHTBBS_OVERRIDE_FRIENDS : YTHTBBS_OVERRIDE_REJECTS, uident) > 0) {
			sprintf(buf, "%s ����%s����", uident, desc);
			show_message(buf);
			return -1;
		}
	}
	if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS && uinfo.mode != FRIEND)
		n = 2;
	else
		n = t_lines - 2;

	strcpy(tmp.id, uident);
	move(n, 0);
	clrtoeol();
	sprintf(genbuf, "�������%s��%s����˵��: ", desc, tmp.id);
	getdata(n, 0, genbuf, tmp.exp, 40, DOECHO, YEA);

	n = ythtbbs_override_add(currentuser.userid, &tmp, (friendflag) ? YTHTBBS_OVERRIDE_FRIENDS : YTHTBBS_OVERRIDE_REJECTS);
	if (n != -1)
		(friendflag) ? getfriendstr() : getrejectstr();
	else
		errlog("append override error");
	return n;
}

int deleteoverride(const char *uident, const enum ythtbbs_override_type override_type) {
	int deleted;
	struct ythtbbs_override fh;
	char buf[STRLEN];

	sethomefile_s(buf, sizeof(buf), currentuser.userid, (override_type == YTHTBBS_OVERRIDE_FRIENDS) ? "friends" : "rejects");
	deleted = search_record(buf, &fh, sizeof (fh), (void *) cmpfnames, (void *)uident); // cmpfnames ���� uident���˴�ʡ�� const �ǰ�ȫ��
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
		strcpy(genbuf, "[�����ż�]");
	else
		strcpy(genbuf, MY_BBS_NAME);
	if (friendflag) {
		showtitle("[�༭��������]", genbuf);
		snprintf(desc, sizeof desc, "����");
	} else {
		showtitle("[�༭��������]", genbuf);
		snprintf(desc, sizeof desc, "����");
	}
	prints(" [\033[1;32m��\033[m,\033[1;32me\033[m] �뿪 [\033[1;32mh\033[m] ���� [\033[1;32m��\033[m,\033[1;32mRtn\033[m] %s˵���� [\033[1;32m��\033[m,\033[1;32m��\033[m] ѡ�� [\033[1;32ma\033[m] ����%s [\033[1;32md\033[m] ɾ��%s\n", desc, desc, desc);
	prints("\033[1;44m ���  %s����      %s˵��                                                   \033[m\n", desc, desc);
	return 0;
}

static char *
override_doentry(int ent, struct ythtbbs_override *fh, char *buf)
{
	sprintf(buf, " %4d  %-12.12s  %s", ent, fh->id, fh->exp);
	return buf;
}

static int override_edit(int ent, struct ythtbbs_override *fh, char *direc) {
	(void) ent;
	struct ythtbbs_override nh;
	char buf[STRLEN / 2];
	int pos;

	pos = search_record(direc, &nh, sizeof (nh), (void *) cmpfnames, fh->id);
	move(t_lines - 2, 0);
	clrtoeol();
	if (pos > 0) {
		sprintf(buf, "������ %s ����%s˵��: ", fh->id, (friendflag) ? "����" : "����");
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
	usercomplete("������Ҫ���ӵĴ���: ", uident);
	if (uident[0] != '\0') {
		if (ythtbbs_cache_UserTable_search_usernum(uident) <= 0) {
			move(2, 0);
			prints("�����ʹ���ߴ���...");
			pressanykey();
			return FULLUPDATE;
		} else
			addtooverride(uident);
	}
	prints("\n�� %s ����%s������...", uident, (friendflag) ? "����" : "����");
	pressanykey();
	return FULLUPDATE;
}

static int override_dele(int ent, struct ythtbbs_override *fh, char *direct) {
	(void) ent;
	(void) direct;
	char buf[STRLEN];
	char desc[5];
	char fname[10];
	int deleted = NA;

	if (friendflag) {
		snprintf(desc, sizeof desc, "����");
		snprintf(fname, sizeof fname, "friends");
	} else {
		snprintf(desc, sizeof desc, "����");
		snprintf(fname, sizeof fname, "rejects");
	}
	saveline(t_lines - 2, 0, NULL);
	move(t_lines - 2, 0);
	sprintf(buf, "�Ƿ�ѡ�%s����%s������ȥ��", fh->id, desc);
	if (askyn(buf, NA, NA) == YEA) {
		move(t_lines - 2, 0);
		clrtoeol();
		if (deleteoverride(fh->id, (friendflag) ? YTHTBBS_OVERRIDE_FRIENDS : YTHTBBS_OVERRIDE_REJECTS) == 1) {
			prints("�Ѵ�%s�������Ƴ���%s��,���κμ�����...", desc, fh->id);
			deleted = YEA;
		} else
			prints("�Ҳ�����%s��,���κμ�����...", fh->id);
	} else {
		move(t_lines - 2, 0);
		clrtoeol();
		prints("ȡ��ɾ��%s...", desc);
	}
	igetkey();
	move(t_lines - 2, 0);
	clrtoeol();
	saveline(t_lines - 2, 1, NULL);
	return (deleted) ? PARTUPDATE : DONOTHING;
}

static int
friend_edit(int ent, struct ythtbbs_override *fh, char *direct)
{
	friendflag = YEA;
	return override_edit(ent, fh, direct);
}

static int friend_add(int ent, struct ythtbbs_override *fh, char *direct) {
	(void) ent;
	(void) fh;
	(void) direct;
	friendflag = YEA;
	return override_add();
}

static int
friend_dele(int ent, struct ythtbbs_override *fh, char *direct)
{
	friendflag = YEA;
	return override_dele(ent, fh, direct);
}

static int friend_mail(int ent, struct ythtbbs_override *fh, char *direct) {
	(void) ent;
	(void) direct;
	if (!HAS_PERM(PERM_POST, currentuser))
		return DONOTHING;
	m_send(fh->id);
	return FULLUPDATE;
}

static int friend_query(int ent, struct ythtbbs_override *fh, char *direct) {
	(void) ent;
	(void) direct;
	int ch;

	if (t_query(fh->id) == -1)
		return FULLUPDATE;
	move(t_lines - 1, 0);
	clrtoeol();
	prints("\033[0;1;44;31m[��ȡ����˵����]\033[33m ���Ÿ����� m �� ���� Q,�� ����һλ ������һλ <Space>,��      \033[m");
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
reject_edit(int ent, struct ythtbbs_override *fh, char *direct)
{
	friendflag = NA;
	return override_edit(ent, fh, direct);
}

static int reject_add(int ent, struct ythtbbs_override *fh, char *direct) {
	(void) ent;
	(void) fh;
	(void) direct;
	friendflag = NA;
	return override_add();
}

static int reject_dele(int ent, struct ythtbbs_override *fh, char *direct) {
	friendflag = NA;
	return override_dele(ent, fh, direct);
}

static int reject_query(int ent, struct ythtbbs_override *fh, char *direct) {
	(void) ent;
	(void) direct;
	int ch;

	if (t_query(fh->id) == -1)
		return FULLUPDATE;
	move(t_lines - 1, 0);
	clrtoeol();
	prints("\033[0;1;44;31m[��ȡ����˵����]\033[33m ���� Q,�� ����һλ ������һλ <Space>,��                      \033[m");
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

int t_friend(const char *s) {
	(void) s;
	char buf[STRLEN];

	friendflag = YEA;
	sethomefile_s(buf, sizeof(buf), currentuser.userid, "friends");
	i_read(GMENU, buf, override_title, (void *) override_doentry, friend_list, sizeof (struct ythtbbs_override));
	clear();
	return 0;
}

int t_reject(const char *s) {
	(void) s;
	char buf[STRLEN];

	friendflag = NA;
	sethomefile_s(buf, sizeof(buf), currentuser.userid, "rejects");
	i_read(GMENU, buf, override_title, (void *) override_doentry, reject_list, sizeof (struct ythtbbs_override));
	clear();
	return 0;
}

struct user_info *t_search(char *sid, int pid, int invisible_check) {
	int i;
	struct user_info *cur, *tmp = NULL;

	ythtbbs_cache_utmp_resolve();
	for (i = 0; i < USHM_SIZE; i++) {
		cur = ythtbbs_cache_utmp_get_by_idx(i);
		if (!cur->active || !cur->pid)
			continue;
		if (!strcasecmp(cur->userid, sid)) {
			if (pid == 0)
				return (isreject(cur) || (invisible_check && (cur->invisible && !HAS_PERM(PERM_SEECLOAK | PERM_SYSOP, currentuser)))) ? NULL : cur;
			tmp = cur;
			if (pid == cur->pid)
				break;
		}
	}
	return isreject(cur) ? NULL : tmp;
}

static int cmpfuid(unsigned *a, unsigned *b) {
	return *a - *b;
}

int
getfriendstr()
{
	int i;
	struct ythtbbs_override *tmp;

	memset(uinfo.friend, 0, sizeof (uinfo.friend));
	sethomefile_s(genbuf, sizeof(genbuf), currentuser.userid, "friends");
	uinfo.fnum = get_num_records(genbuf, sizeof (struct ythtbbs_override));
	if (uinfo.fnum <= 0)
		return -1;
	uinfo.fnum = (uinfo.fnum >= MAXFRIENDS) ? MAXFRIENDS : uinfo.fnum;
	tmp = (struct ythtbbs_override *) calloc(sizeof (struct ythtbbs_override), uinfo.fnum);
	get_records(genbuf, tmp, sizeof (struct ythtbbs_override), 1, uinfo.fnum);
	struct ythtbbs_override EMPTY;
	for (i = 0; i < uinfo.fnum; i++) {
		tmp[i].id[sizeof(EMPTY.id) - 1] = 0;
		uinfo.friend[i] = ythtbbs_cache_UserTable_search_usernum(tmp[i].id);
		if (uinfo.friend[i] == 0)
			deleteoverride(tmp[i].id, YTHTBBS_OVERRIDE_FRIENDS);
		/* ˳��ɾ���Ѳ������ʺŵĺ��� */
	}
	free(tmp);
	qsort(&uinfo.friend, uinfo.fnum, sizeof (uinfo.friend[0]), (void *) cmpfuid);
	update_ulist(&uinfo, utmpent);
	return 0;
}

int
getrejectstr()
{
	int nr, i;
	struct ythtbbs_override *tmp;

	memset(uinfo.reject, 0, sizeof (uinfo.reject));
	sethomefile_s(genbuf, sizeof(genbuf), currentuser.userid, "rejects");
	nr = get_num_records(genbuf, sizeof (struct ythtbbs_override));
	if (nr <= 0)
		return -1;
	nr = (nr >= MAXREJECTS) ? MAXREJECTS : nr;
	tmp = (struct ythtbbs_override *) calloc(sizeof (struct ythtbbs_override), nr);
	get_records(genbuf, tmp, sizeof (struct ythtbbs_override), 1, nr);
	struct ythtbbs_override EMPTY;
	for (i = 0; i < nr; i++) {
		tmp[i].id[sizeof(EMPTY.id) - 1] = 0;
		uinfo.reject[i] = ythtbbs_cache_UserTable_search_usernum(tmp[i].id);
		if (uinfo.reject[i] == 0)
			deleteoverride(tmp[i].id, YTHTBBS_OVERRIDE_REJECTS);
	}
	free(tmp);
	return 0;
}

int wait_friend(const char *s) {
	(void) s;
	FILE *fp;
	int tuid;
	char buf[STRLEN];
	char uid[13];

	modify_user_mode(WFRIEND);
	clear();
	move(1, 0);
	usercomplete("������ʹ���ߴ����Լ���ϵͳ��Ѱ������: ", uid);
	if (uid[0] == '\0') {
		clear();
		return 0;
	}
	if (!(tuid = ythtbbs_cache_UserTable_search_usernum(uid))) {
		move(2, 0);
		prints("\033[1m����ȷ��ʹ���ߴ���\033[m\n");
		pressanykey();
		clear();
		return -1;
	}
	sprintf(buf, "��ȷ��Ҫ�� \033[1m%s\033[m ����ϵͳѰ��������", uid);
	move(2, 0);
	if (askyn(buf, YEA, NA) == NA) {
		clear();
		return -1;
	}
	if ((fp = fopen("friendbook", "a")) == NULL) {
		prints("ϵͳ��Ѱ�������޷���������֪ͨվ��...\n");
		pressanykey();
		return -1;
	}
	sprintf(buf, "%d@%s", tuid, currentuser.userid);
	if (!seek_in_file("friendbook", buf))
		fprintf(fp, "%s\n", buf);
	fclose(fp);
	move(3, 0);
	prints("�Ѿ��������Ѱ�������У�\033[1m%s\033[m ��վϵͳһ����֪ͨ��...\n", uid);
	pressanykey();
	clear();
	return 0;
}

//#ifdef TALK_LOG
/* edwardc.990106 �ֱ�Ϊ��λ�����������¼ */
/* -=> �Լ�˵�Ļ� */
/* --> �Է�˵�Ļ� */

static void
do_log(char *msg, int who)
{
/* Sonny.990514 ����ץ overflow ������... */
/* Sonny.990606 overflow ������. buf[100] ����ȷ��. �ο� man sprintf() */
	time_t now;
	char buf[128];
	now = time(0);
	msg[79] = 0;
	if (msg[strlen(msg) - 1] == '\n')
		msg[strlen(msg) - 1] = 0;
	if (strlen(msg) < 1 || msg[0] == '\r' || msg[0] == '\n')
		return;

	/* ֻ���Լ��� */
	sethomefile_s(buf, sizeof(buf), currentuser.userid, "talklog");

	if (!dashf(buf) || talkrec == -1) {
		talkrec = open(buf, O_RDWR | O_CREAT | O_TRUNC, 0644);
		buf[127] = 0;
		snprintf(buf, 127, "\033[1;32m�� %s ���黰����, ����: %s \033[m\n", save_page_requestor, Cdate(&now));
		write(talkrec, buf, strlen(buf));
		sprintf(buf, "\t��ɫ�ֱ����: \033[1;33m%s\033[m \033[1;36m%s\033[m \n\n", currentuser.userid, partner);
		write(talkrec, buf, strlen(buf));
	}
	if (who == 1) {		/* �Լ�˵�Ļ� */
		sprintf(buf, "\033[1;33m-=> %s \033[m\n", msg);
		write(talkrec, buf, strlen(buf));
	} else if (who == 2) {	/* ����˵�Ļ� */
		sprintf(buf, "\033[1;36m--> %s \033[m\n", msg);
		write(talkrec, buf, strlen(buf));
	}
}

//#endif
static char *
Cdate(time_t *clock)
{
	static char foo[22];
	struct tm *mytm = localtime(clock);

	strftime(foo, 22, "%m/%d/%Y %T %a", mytm);
	return (foo);
}

