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
#include "stuff.h"
#include "xyz.h"
#include "comm_list.h"
#include "boards.h"
#include "announce.h"
#include "maintain.h"
#include "mail.h"
#include "list.h"
#include "talk.h"
#include "sendmsg.h"
#include "userinfo.h"
#include "vote.h"
#include "bbsinc.h"
#include "addressbook.h"
#include "chat.h"
#include "delete.h"
#include "more.h"
#include "convcode.h"
#include "smth_screen.h"
#include "main.h"
#include "fileshm.h"
#include "bbs_global_vars.h"
#include "bbs-internal.h"

extern int moneycenter(void);        // moneycenter.c
extern int x_active_manager(void);   // identify.c

#define SC_BUFSIZE              20480
#define SC_KEYSIZE              256
#define SC_CMDSIZE              256
#define sysconf_ptr( offset )   (&sysconf_buf[ offset ])
#define pm2fptr(pm) sysconf_funcptr(sysconf_ptr((pm).func_off))

struct menupos {
	short col, line;
};

struct sysheader {
	int menu, key, len;
};

struct smenuitem {
	short line, col;
	int level;
	//since we are using short, the sysconf_buf size must be less than 64K
	short name_off, desc_off, arg_off, func_off;
} *menuitem;

struct sdefine {
	int key_off, str_off;
	int val;
} *sysvar;

extern int nettyNN;
char *sysconf_buf;
int sysconf_menu, sysconf_key, sysconf_len;

/*Add By Excellent */
struct scommandlist {
	char *name;
	int (*fptr) (char *);
};

static struct mmapfile sysconf_mf = { .ptr = NULL };

const struct scommandlist sysconf_cmdlist[] = {
	{"domenu", domenu},
	{"EGroups", (void *) EGroup},
	{"BoardsAll", (void *) Boards},
	{"BoardsNew", (void *) New},
	{"GoodBrds", (void *) GoodBrds},
	{"LeaveBBS", (void *) Goodbye},
	{"Announce", (void *) Announce},
	{"SelectBoard", (void *) Select},
	{"ReadBoard", (void *) Read},
	{"SetHelp", (void *) Personal},
	{"Personal", (void *) Personal},
	{"DenyLevel", (void *) x_denylevel},
	{"OrdainBM", (void *) m_ordainBM},
	{"RetireBM", (void *) m_retireBM},
	{"Hell", (void *) showhell},
	{"Prison", (void *) showprison},
	{"Live", (void *) online},
	{"SetAlarm", (void *) setcalltime},
	{"MailAll", (void *) mailall},
	{"LockScreen", (void *) x_lockscreen},
	{"OffLine", (void *) offline},
	{"ReadNewMail", (void *) m_new},
	{"ReadMail", (void *) m_read},
	{"SendMail", (void *) M_send},
	{"GroupSend", (void *) g_send},
	{"OverrideSend", (void *) ov_send},
	{"SendNetMail", (void *) m_internet},
	{"UserDefine", (void *) x_userdefine},
	{"CopyKeys", (void *) x_copykeys},
	{"DefineKeys", (void *) x_setkeys},
	{"DefineKeys2", (void *) x_setkeys2},
	{"DefineKeys3", (void *) x_setkeys3},
	{"DefineKeys4", (void *) x_setkeys4},
	{"DefineKeys5", (void *) x_setkeys5},
	{"ShowFriends", (void *) t_friends},
	{"ShowLogins", (void *) t_users},
	{"QueryUser", t_query},
	{"WaitFriend", (void *) wait_friend},
	{"Talk", (void *) t_talk},
	{"SetPager", (void *) t_pager},
	{"SetCloak", (void *) x_cloak},
	{"SendMsg", (void *) s_msg},
	{"ShowMsg", (void *) show_allmsgs},
	{"SetFriends", (void *) t_friend},
	{"SetRejects", (void *) t_reject},
	{"FriendWall", (void *) friend_wall},
	{"EnterChat", ent_chat},
	{"FillForm", (void *) x_fillform},
	{"SetInfo", (void *) x_info},
	{"EditUFiles", (void *) x_edits},
	{"ExecBBSNet", ent_bnet},
	{"GoodWish", sendgoodwish},
	{"CheckForm", (void *) m_register},
	{"Identify", x_active_manager},
	{"ModifyInfo", (void *) m_info},
	{"ModifyLevel", (void *) x_level},
	{"KickUser", (void *) kick_user},
	{"OpenVote", (void *) m_vote},
	{"NewBoard", (void *) m_newbrd},
	{"EditBoard", (void *) m_editbrd},
	{"EditSFiles", (void *) a_edits},
	{"EditSFiles2",(void*) a_edits2},
	{"Announceall", (void *) wall},
	{"FCode", (void *) switch_code},
	{"ADDRESSBOOK", (void *) addressbook},
	{"CLEARNEWFLAG", (void *) clear_all_new_flag},
	{"ADDPERSONAL", (void *) m_addpersonal},
	{"CancelMail", m_cancel},
	{"MONEYCENTER", (void *) moneycenter},
	{"Wall_telnet", (void *) wall_telnet},
	{NULL, NULL}
};

static void encodestr(register char *str);
static void decodestr(register char *str);
static void *sysconf_funcptr(char *func_name);
static int sysconf_addstr(char *str);
static void sysconf_addkey(char *key, char *str, int val);
static void parse_sysconf(char *fname);
static void build_sysconf(char *configfile, char *imgfile);
static void load_sysconf_image(char *imgfile);
static int domenu_screen(struct smenuitem *pm, char *cmdprompt, struct menupos *pos);
static void sysconf_addmenu(FILE * fp, char *key);
static void sysconf_addblock(FILE * fp, char *key);

static void
encodestr(str)
register char *str;
{
	register char ch, *buf;
	int n;

	buf = str;
	while ((ch = *str++) != '\0') {
		if (*str == ch && str[1] == ch && str[2] == ch) {
			n = 4;
			str += 3;
			while (*str == ch && n < 100) {
				str++;
				n++;
			}
			*buf++ = '\01';
			*buf++ = ch;
			*buf++ = n;
		} else
			*buf++ = ch;
	}
	*buf = '\0';
}

static void
decodestr(str)
register char *str;
{
	register char ch;
	int n, i = 0;
	char buf[4096];
	while (((ch = *str++) != '\0') && i < 4095)
		if (ch != '\01')
			buf[i++] = ch;
		else if (*str != '\0' && str[1] != '\0') {
			ch = *str++;
			n = *str++;
			while (--n >= 0)
				buf[i++] = ch;
		}
	buf[i] = 0;
	outs(buf);
}

static void *
sysconf_funcptr(func_name)
char *func_name;
{
	int n = 0;
	char *str;

	while ((str = sysconf_cmdlist[n].name) != NULL) {
		if (strcmp(func_name, str) == 0)
			return (sysconf_cmdlist[n].fptr);
		n++;
	}
	return NULL;
}

static int
sysconf_addstr(str)
char *str;
{
	int len = sysconf_len;
	char *buf;

	buf = sysconf_buf + len;
	strcpy(buf, str);
	sysconf_len = len + strlen(str) + 1;
	return buf - sysconf_buf;
}

char *
sysconf_str(key)
char *key;
{
	int n;

	for (n = 0; n < sysconf_key; n++)
		if (!strcmp(key, sysconf_ptr(sysvar[n].key_off)))
			return sysconf_ptr(sysvar[n].str_off);
	return NULL;
}

int
sysconf_eval(key)
char *key;
{
	int n;

	for (n = 0; n < sysconf_key; n++)
		if (!strcmp(key, sysconf_ptr(sysvar[n].key_off)))
			return sysvar[n].val;
	if (*key < '0' || *key > '9') {
		errlog("sysconf: unknown key: %s.", key);
	}
	return (strtol(key, NULL, 0));
}

static void
sysconf_addkey(key, str, val)
char *key, *str;
int val;
{
	int num;

	if (sysconf_key < SC_KEYSIZE) {
		num = sysconf_key++;
		if (str == NULL)
			sysvar[num].str_off = 0;
		else
			sysvar[num].str_off = sysconf_addstr(str);
		sysvar[num].key_off = sysconf_addstr(key);
		sysvar[num].val = val;
	}
}

static void
sysconf_addmenu(fp, key)
FILE *fp;
char *key;
{
	struct smenuitem *pm;
	char buf[256];
	char *cmd, *arg[5], *ptr;
	int n;

	sysconf_addkey(key, "menu", sysconf_menu);
	while (fgets(buf, sizeof (buf), fp) != NULL && buf[0] != '%') {
		cmd = strtok(buf, " \t\n");
		if (cmd == NULL || *cmd == '#') {
			continue;
		}
		arg[0] = arg[1] = arg[2] = arg[3] = arg[4] = "";
		n = 0;
		for (n = 0; n < 5; n++) {
			if ((ptr = strtok(NULL, ",\n")) == NULL)
				break;
			while (*ptr == ' ' || *ptr == '\t')
				ptr++;
			if (*ptr == '"') {
				arg[n] = ++ptr;
				while (*ptr != '"' && *ptr != '\0')
					ptr++;
				*ptr = '\0';
			} else {
				arg[n] = ptr;
				while (*ptr != ' ' && *ptr != '\t' && *ptr != '\0') ptr++;
				*ptr = '\0';
			}
		}
		pm = &menuitem[sysconf_menu++];
		pm->line = sysconf_eval(arg[0]);
		pm->col = sysconf_eval(arg[1]);
		if (*cmd == '@') {
			pm->level = sysconf_eval(arg[2]);
			pm->name_off = sysconf_addstr(arg[3]);
			pm->desc_off = sysconf_addstr(arg[4]);
			pm->func_off = sysconf_addstr(cmd + 1);
			pm->arg_off = pm->name_off;
		} else if (*cmd == '!') {
			pm->level = sysconf_eval(arg[2]);
			pm->name_off = sysconf_addstr(arg[3]);
			pm->desc_off = sysconf_addstr(arg[4]);
			pm->func_off = sysconf_addstr("domenu");
			pm->arg_off = sysconf_addstr(cmd + 1);
		} else {
			pm->level = -2;
			pm->name_off = sysconf_addstr(cmd);
			pm->desc_off = sysconf_addstr(arg[2]);
			pm->func_off = 0;
			pm->arg_off = 0;
		}
	}
	pm = &menuitem[sysconf_menu++];
	pm->name_off = pm->desc_off = pm->arg_off = pm->func_off = 0;
	pm->level = -1;
}

static void
sysconf_addblock(fp, key)
FILE *fp;
char *key;
{
	char buf[256];
	int num;

	if (sysconf_key < SC_KEYSIZE) {
		num = sysconf_key++;
		sysvar[num].key_off = sysconf_addstr(key);
		sysvar[num].str_off = sysconf_len;
		sysvar[num].val = -1;
		while (fgets(buf, sizeof (buf), fp) != NULL && buf[0] != '%') {
			encodestr(buf);
			strcpy(sysconf_buf + sysconf_len, buf);
			sysconf_len += strlen(buf);
		}
		sysconf_len++;
	} else {
		while (fgets(buf, sizeof (buf), fp) != NULL && buf[0] != '%') {
		}
	}
}

static void
parse_sysconf(fname)
char *fname;
{
	FILE *fp;
	char buf[256];
	char tmp[256], *ptr;
	char *key, *str;
	int val;

	if ((fp = fopen(fname, "r")) == NULL) {
		return;
	}
	sysconf_addstr("(null ptr)");
	while (fgets(buf, sizeof (buf), fp) != NULL) {
		ptr = buf;
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;

		if (*ptr == '%') {
			strtok(ptr, " \t\n");
			if (strcmp(ptr, "%menu") == 0) {
				str = strtok(NULL, " \t\n");
				if (str != NULL)
					sysconf_addmenu(fp, str);
			} else {
				sysconf_addblock(fp, ptr + 1);
			}
		} else if (*ptr == '#') {
			key = strtok(ptr, " \t\"\n");
			str = strtok(NULL, " \t\"\n");
			if (key != NULL && str != NULL && strcmp(key, "#include") == 0) {
				parse_sysconf(str);
			}
		} else if (*ptr != '\n') {
			key = strtok(ptr, "=#\n");
			str = strtok(NULL, "#\n");
			if ((key != NULL && str != NULL)) {
				strtok(key, " \t");
				while (*str == ' ' || *str == '\t')
					str++;
				if (*str == '"') {
					str++;
					strtok(str, "\"");
					val = atoi(str);
					sysconf_addkey(key, str, val);
				} else {
					val = 0;
					strcpy(tmp, str);
					ptr = strtok(tmp, ", \t");
					while (ptr != NULL) {
						val |= sysconf_eval(ptr);
						ptr = strtok(NULL, ", \t");
					}
					sysconf_addkey(key, NULL, val);
				}
			} else {
				errlog("parse syscon error, ptr=%s", ptr);
			}
		}
	}
	fclose(fp);
}

static void
build_sysconf(configfile, imgfile)
char *configfile, *imgfile;
{
	struct smenuitem *old_menuitem;
	struct sdefine *old_sysvar;
	char *old_buf;
	int old_menu, old_key, old_len;
	char tmpfile[STRLEN];
	struct sysheader shead;
	int fh;

	old_menuitem = menuitem;
	old_menu = sysconf_menu;
	old_sysvar = sysvar;
	old_key = sysconf_key;
	old_buf = sysconf_buf;
	old_len = sysconf_len;
	menuitem = (void *) malloc(SC_CMDSIZE * sizeof (struct smenuitem));
	sysvar = (void *) malloc(SC_KEYSIZE * sizeof (struct sdefine));
	sysconf_buf = (void *) malloc(SC_BUFSIZE);
	sysconf_menu = 0;
	sysconf_key = 0;
	sysconf_len = 0;
	parse_sysconf(configfile);
	snprintf(tmpfile, sizeof (tmpfile), "%s.tmp", imgfile);
	if ((fh = open(tmpfile, O_WRONLY | O_CREAT, 0644)) >= 0) {
		ftruncate(fh, 0);
		shead.menu = sysconf_menu;
		shead.key = sysconf_key;
		shead.len = sysconf_len;
		write(fh, &shead, sizeof (shead));
		write(fh, menuitem, sysconf_menu * sizeof (struct smenuitem));
		write(fh, sysvar, sysconf_key * sizeof (struct sdefine));
		write(fh, sysconf_buf, sysconf_len);
		close(fh);
		//must use rename, because we will use mmap on imgfile -- ylsdd
		rename(tmpfile, imgfile);
		close(fh);
	}
	free(menuitem);
	free(sysvar);
	free(sysconf_buf);
	menuitem = old_menuitem;
	sysconf_menu = old_menu;
	sysvar = old_sysvar;
	sysconf_key = old_key;
	sysconf_buf = old_buf;
	sysconf_len = old_len;
}

/*static int
ytht_smth_reload_badwords(bwfile, tobuild)
char *bwfile;
char *tobuild;
{
	FILE *fr;
	int fw;
	char tmpfile[STRLEN];
	char badword[40], *ptr;
	sprintf(tmpfile, "tmp/badword.%s.%05d", currentuser.userid, uinfo.pid);
	if ((fr = fopen(bwfile, "r")) == 0) {
		return -1;
	}
	if ((fw = open(tmpfile, O_WRONLY | O_CREAT, 0644)) == 0) {
		fclose(fr);
		return -2;
	}
	while (fgets(badword, 40, fr)) {
		if ((badword[0] == '\n') || (badword[0] == '\0'))
			continue;
		if ((ptr = strrchr(badword, '\n')) != NULL)
			*ptr = 0;
		write(fw, badword, 40);
	}
	close(fw);
	fclose(fr);
	crossfs_rename(tmpfile, tobuild);
	return 0;
}*/

static void
load_sysconf_image(imgfile)
char *imgfile;
{
	struct sysheader *shead;
	char *ptr;

	if (mmapfile(imgfile, &sysconf_mf) >= 0) {
		ptr = sysconf_mf.ptr;
		shead = (void *) ptr;
		ptr += sizeof (struct sysheader);
		menuitem = (void *) ptr;
		ptr += shead->menu * sizeof (struct smenuitem);
		sysvar = (void *) ptr;
		ptr += shead->key * sizeof (struct sdefine);
		sysconf_buf = (void *) ptr;
		sysconf_menu = shead->menu;
		sysconf_key = shead->key;
		sysconf_len = shead->len;
	}
}

void
load_sysconf()
{
	if (dashf("etc/rebuild.sysconf") || !dashf("sysconf.img2")) {
		newtrace("system reload sysconf.img2");
		build_sysconf("etc/sysconf.ini", "sysconf.img2");
	}
	load_sysconf_image("sysconf.img2");
}

static int
domenu_allocpos(struct smenuitem *pm, struct menupos **pptr)
{
	int count = 0;
	while (pm->level != -1) {
		count++;
		pm++;
	}
	*pptr = malloc(count * sizeof (struct menupos));
	return count;
}

static int
domenu_screen(pm, cmdprompt, pos)
struct smenuitem *pm;
char *cmdprompt;
struct menupos *pos;
{
	char *str;
	int line = 3, col = 0, num = 0;

/*    if(!DEFINE(DEF_NORMALSCR))  */
	clear();
	while (1) {
		switch (pm->level) {
		case -1:
			return (num);
		case -2:
			if (strcmp(sysconf_ptr(pm->name_off), "title") == 0) {
				docmdtitle(sysconf_ptr(pm->desc_off), cmdprompt);
			} else if (strcmp(sysconf_ptr(pm->name_off), "screen") == 0) {
				if ((str = sysconf_str(sysconf_ptr(pm->desc_off)))) {
					move(pm->line, pm->col);
					decodestr(str);
				}
			}
			pos[num].line = -1;
			break;
		default:
			if (pm->line >= 0 && HAS_PERM(pm->level, currentuser)) {
				if (pm->line != 0) {
					line = pm->line;
					col = pm->col;
				}
				pos[num].line = line;
				pos[num].col = col;
				move(line, col);
				prints("  %s", sysconf_ptr(pm->desc_off));
				line++;
			} else {
				if (pm->line > 0) {
					line = pm->line;
					col = pm->col;
				}
				pos[num].line = -1;
			}
		}
		num++;
		pm++;
	}
}

int
domenu(menu_name)
char *menu_name;
{
	extern int refscreen;
	struct smenuitem *pm;
	struct menupos *pos;
	char *cmdprompt = "Ä¿Ç°Ñ¡Ôñ£º";
	int size, now;
	int cmdplen, cmd, i;

	if (sysconf_menu <= 0) {
		return -1;
	}
	pm = &menuitem[sysconf_eval(menu_name)];
	size = domenu_allocpos(pm, &pos);
	size = domenu_screen(pm, cmdprompt, pos);
	cmdplen = strlen(cmdprompt);
	now = 0;
	if (strcmp(menu_name, "TOPMENU") == 0) {
		char ch = 'F';
		if (chkmail())
			ch = 'M';
		for (i = 0; i < size; i++)
			if (pos[i].line > 0 && *sysconf_ptr(pm[i].name_off) == ch)
				now = i;
	}
	modify_user_mode(MMENU);
	/* added by netty  */
	if (nettyNN == 1) {
		R_monitor();
	}
	while (1) {
		//printacbar();  by bjgyt
		while (pm[now].level < 0 || !HAS_PERM(pm[now].level, currentuser)) {
			now++;
			if (now >= size)
				now = 0;
		}
		move(pos[now].line, pos[now].col);
		prints("¡ô");
		move(1, cmdplen);
		prints("[%-12s]", sysconf_ptr(pm[now].name_off));
		clrtoeol();
		cmd = egetch();
		move(pos[now].line, pos[now].col);
		prints("  ");
		switch (cmd) {
		case EOF:
			if (!refscreen) {
				abort_bbs();
			}
			domenu_screen(pm, cmdprompt, pos);
			modify_user_mode(MMENU);
/*Modify to showout ActiveBoard After talking*/
			if (nettyNN == 1) {
				R_monitor();
			}
			break;
		case KEY_RIGHT:
			for (i = 0; i < size; i++) {
				if (pos[i].line == pos[now].line
						&& pm[i].level >= 0
						&& pos[i].col > pos[now].col
						&& HAS_PERM(pm[i].level, currentuser))
					break;
			}
			if (i < size) {
				now = i;
				break;
			}
		case '\n':
		case '\r':
			if (strcmp(sysconf_ptr(pm[now].arg_off), "..") == 0) {
				return 0;
			}
			if (pm[now].func_off != 0) {
				int (*fptr) () = pm2fptr(pm[now]);
				move(1, cmdplen);
				clrtoeol();
				fptr(sysconf_ptr(pm[now].arg_off));
				if (fptr == (void *) Select) {
					now++;
				}
				domenu_screen(pm, cmdprompt, pos);
				modify_user_mode(MMENU);
				if (nettyNN) {
					R_monitor();
				}
			}
			break;
		case KEY_LEFT:
			for (i = size - 1; i >= 0; i--) {
				if (pos[i].line == pos[now].line && pos[i].col < pos[now].col)
					break;
				if (pm2fptr(pm[i]) == Goodbye)
					break;
			}
			if (i >= 0) {
				now = i;
				break;
			}
			free(pos);
			return 0;
		case KEY_DOWN:
			now++;
			break;
		case KEY_UP:
			now--;
			while (pos[now].line < 0) {
				if (now > 0)
					now--;
				else
					now = size - 1;
			}
			break;
			/* add by bluetent 2003.5.9 */
		case KEY_PGUP:
			now = 0;
			break;
		case KEY_PGDN:
			now = size - 1;
			while (pos[now].line < 0)
				now--;
			break;
			/* add end */
		case '~':
			if (!HAS_PERM(PERM_SYSOP, currentuser)) {
				break;
			}
			newtrace("system reload sysconf.img2");
			build_sysconf("etc/sysconf.ini", "sysconf.img2");
			load_sysconf_image("sysconf.img2");
			pm = &menuitem[sysconf_eval(menu_name)];
			free(pos);
			domenu_allocpos(pm, &pos);
			size = domenu_screen(pm, cmdprompt, pos);
			now = 0;
			fill_shmfile(5, "etc/endline", ENDLINE1_SHMKEY);
			ytht_smth_reload_badwords("etc/badwords", "etc/.badwords_new");
			ytht_smth_reload_badwords("etc/sbadwords", "etc/.sbadwords_new");
			ytht_smth_reload_badwords("etc/pbadwords", "etc/.pbadwords_new");
			ytht_smth_reload_badwords("etc/filtertitle", "etc/.filtertitle_new");
			gensecm("etc/secmlist");
			break;
		case '!':	/* youzi leave */
			if (strcmp("TOPMENU", menu_name) == 0)
				break;
			else {
				free(pos);
				return Goodbye();
			}
		default:
			cmd = toupper(cmd);
			for (i = 0; i < size; i++) {
				if (pos[i].line > 0 && cmd == *sysconf_ptr(pm[i].name_off)) {
					now = i;
					break;
				}

			}
		}
	}
}
