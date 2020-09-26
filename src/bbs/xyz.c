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
#define EXTERN
#include "bbs.h"
#include "bbstelnet.h"
#include <sys/mman.h>

pid_t childpid;
static int loadkeys(struct one_key *key, char *name);
static int savekeys(struct one_key *key, char *name);
static void keyprint(char *buf, int key);
static int showkeyinfo(struct one_key *akey, int i);
static unsigned int setkeys(struct one_key *key);
static void myexec_cmd(int umode, int pager, const char *cmdfile,
		       const char *param);
static void datapipefd(int fds, int fdn);
static void exec_cmd(int umode, int pager, char *cmdfile, char *param1);
static int sendGoodWish(char *userid);
static void childreturn(int i);
static void escape_filename(char *fn);
static void bbs_zsendfile(char *filename);
static void get_load(double load[]);

static const char * const g_permstrings[] = {
	"\xBB\xF9\xB1\xBE\xC8\xA8\xC1\xA6",									/* PERM_BASIC     »ù±¾È¨Á¦ */
	"\xBD\xF8\xC8\xEB\xC1\xC4\xCC\xEC\xCA\xD2",							/* PERM_CHAT      ½øÈëÁÄÌìÊÒ */
	"\xBA\xF4\xBD\xD0\xCB\xFB\xC8\xCB\xC1\xC4\xCC\xEC",					/* PERM_PAGE      ºô½ĞËûÈËÁÄÌì */
	"\xB7\xA2\xB1\xED\xCE\xC4\xD5\xC2",									/* PERM_POST      ·¢±íÎÄÕÂ */
	"\xCA\xB9\xD3\xC3\xD5\xDF\xD7\xCA\xC1\xCF\xD5\xFD\xC8\xB7",			/* PERM_LOGINOK   Ê¹ÓÃÕß×ÊÁÏÕıÈ· */
	"\xBD\xFB\xD6\xB9\xCA\xB9\xD3\xC3\xC7\xA9\xC3\xFB\xB5\xB5",			/* PERM_DENYSIG   ½ûÖ¹Ê¹ÓÃÇ©Ãûµµ */
	"\xD2\xFE\xC9\xED\xCA\xF5",											/* PERM_CLOAK     ÒşÉíÊõ */
	"\xBF\xB4\xB4\xA9\xD2\xFE\xC9\xED\xCA\xF5",							/* PERM_SEECLOAK  ¿´´©ÒşÉíÊõ */
	"\xD5\xCA\xBA\xC5\xD3\xC0\xBE\xC3\xB1\xA3\xC1\xF4",					/* PERM_XEMPT     ÕÊºÅÓÀ¾Ã±£Áô */
	"\xB1\xE0\xBC\xAD\xBD\xF8\xD5\xBE\xBB\xAD\xC3\xE6",					/* PERM_WELCOME   ±à¼­½øÕ¾»­Ãæ */
	"\xB0\xE5\xD6\xF7",													/* PERM_BOARDS    °åÖ÷ */
	"\xD5\xCA\xBA\xC5\xB9\xDC\xC0\xED\xD4\xB1",							/* PERM_ACCOUNTS  ÕÊºÅ¹ÜÀíÔ± */
	"\xB1\xBE\xD5\xBE\xD6\xD9\xB2\xC3",									/* PERM_ARBITRATE ±¾Õ¾ÖÙ²Ã */
	"\xCD\xB6\xC6\xB1\xB9\xDC\xC0\xED\xD4\xB1",							/* PERM_OVOTE     Í¶Æ±¹ÜÀíÔ± */
	"\xCF\xB5\xCD\xB3\xCE\xAC\xBB\xA4\xB9\xDC\xC0\xED\xD4\xB1",			/* PERM_SYSOP     ÏµÍ³Î¬»¤¹ÜÀíÔ± */
	"Read/Post \xCF\xDE\xD6\xC6",										/* PERM_POSTMASK  Read/Post ÏŞÖÆ */
	"\xBE\xAB\xBB\xAA\xC7\xF8\xD7\xDC\xB9\xDC",							/* PERM_ANNOUNCE  ¾«»ªÇø×Ü¹Ü */
	"\xCC\xD6\xC2\xDB\xC7\xF8\xD7\xDC\xB9\xDC",							/* PERM_OBOARDS   ÌÖÂÛÇø×Ü¹Ü */
	"\xBB\xEE\xB6\xAF\xBF\xB4\xB0\xE6\xD7\xDC\xB9\xDC",					/* PERM_ACBOARD   »î¶¯¿´°æ×Ü¹Ü */
	"\xB2\xBB\xC4\xDC ZAP(\xCC\xD6\xC2\xDB\xC7\xF8\xD7\xA8\xD3\xC3)",	/* PERM_NOZAP     ²»ÄÜZAP(ÌÖÂÛÇø×¨ÓÃ) */
	"\xC7\xBF\xD6\xC6\xBA\xF4\xBD\xD0",									/* PERM_FORCEPAGE Ç¿ÖÆºô½Ğ */
	"\xD1\xD3\xB3\xA4\xB7\xA2\xB4\xF4\xCA\xB1\xBC\xE4",					/* PERM_EXT_IDLE  ÑÓ³¤·¢´ôÊ±¼ä */
	"\xB4\xF3\xD0\xC5\xCF\xE4",											/* PERM_SPECIAL1  ´óĞÅÏä */
	"\xCC\xD8\xCA\xE2\xC8\xA8\xCF\xDE 2",								/* PERM_SPECIAL2  ÌØÊâÈ¨ÏŞ 2 */
	"\xCC\xD8\xCA\xE2\xC8\xA8\xCF\xDE 3",								/* PERM_SPECIAL3  ÌØÊâÈ¨ÏŞ 3 */
	"\xC7\xF8\xB3\xA4",													/* PERM_SPECIAL4  Çø³¤ */
	"\xB1\xBE\xD5\xBE\xBC\xE0\xB2\xEC\xD7\xE9",							/* PERM_SPECIAL5  ±¾Õ¾¼à²ì×é */
	"\xB1\xBE\xD5\xBE\xC1\xA2\xB7\xA8\xBB\xE1",							/* PERM_SPECIAL6  ±¾Õ¾Á¢·¨»á */
	"\xCC\xD8\xCA\xE2\xC8\xA8\xCF\xDE 7",								/* PERM_SPECIAL7  ÌØÊâÈ¨ÏŞ 7 */
	"\xB8\xF6\xC8\xCB\xCE\xC4\xBC\xAF",									/* PERM_SPECIAL8  ¸öÈËÎÄ¼¯ */
	"\xBD\xFB\xD6\xB9\xB7\xA2\xD0\xC5\xC8\xA8",							/* PERM_DENYMAIL  ½ûÖ¹·¢ĞÅÈ¨ */
};

static const char *const g_user_definestr[NUMDEFINES] = {
	/* DEF_FRIENDCALL         ºô½ĞÆ÷¹Ø±ÕÊ±¿ÉÈÃºÃÓÑºô½Ğ */
	"\xBA\xF4\xBD\xD0\xC6\xF7\xB9\xD8\xB1\xD5\xCA\xB1\xBF\xC9\xC8\xC3\xBA\xC3\xD3\xD1\xBA\xF4\xBD\xD0",
	/* DEF_ALLMSG             ½ÓÊÜËùÓĞÈËµÄÑ¶Ï¢ */
	"\xBD\xD3\xCA\xDC\xCB\xF9\xD3\xD0\xC8\xCB\xB5\xC4\xD1\xB6\xCF\xA2",
	/* DEF_FRIENDMSG          ½ÓÊÜºÃÓÑµÄÑ¶Ï¢ */
	"\xBD\xD3\xCA\xDC\xBA\xC3\xD3\xD1\xB5\xC4\xD1\xB6\xCF\xA2",
	/* DEF_SOUNDMSG           ÊÕµ½Ñ¶Ï¢·¢³öÉùÒô */
	"\xCA\xD5\xB5\xBD\xD1\xB6\xCF\xA2\xB7\xA2\xB3\xF6\xC9\xF9\xD2\xF4",
	/* DEF_COLOR              Ê¹ÓÃ²ÊÉ« */
	"\xCA\xB9\xD3\xC3\xB2\xCA\xC9\xAB",
	/* DEF_ACBOARD            ÏÔÊ¾»î¶¯¿´°æ */
	"\xCF\xD4\xCA\xBE\xBB\xEE\xB6\xAF\xBF\xB4\xB0\xE6",
	/* DEF_ENDLINE            ÏÔÊ¾Ñ¡µ¥µÄÑ¶Ï¢À¸ */
	"\xCF\xD4\xCA\xBE\xD1\xA1\xB5\xA5\xB5\xC4\xD1\xB6\xCF\xA2\xC0\xB8",
	/* DEF_EDITMSG            ±à¼­Ê±ÏÔÊ¾×´Ì¬À¸ */
	"\xB1\xE0\xBC\xAD\xCA\xB1\xCF\xD4\xCA\xBE\xD7\xB4\xCC\xAC\xC0\xB8",
	/* DEF_NOTMSGFRIEND       Ñ¶Ï¢À¸²ÉÓÃÒ»°ã/¾«¼òÄ£Ê½ */
	"\xD1\xB6\xCF\xA2\xC0\xB8\xB2\xC9\xD3\xC3\xD2\xBB\xB0\xE3/\xBE\xAB\xBC\xF2\xC4\xA3\xCA\xBD",
	/* DEF_NORMALSCR          Ñ¡µ¥²ÉÓÃÒ»°ã/¾«¼òÄ£Ê½ */
	"\xD1\xA1\xB5\xA5\xB2\xC9\xD3\xC3\xD2\xBB\xB0\xE3/\xBE\xAB\xBC\xF2\xC4\xA3\xCA\xBD",
	/* DEF_NEWPOST            ·ÖÀàÌÖÂÛÇøÒÔ New ÏÔÊ¾ */
	"\xB7\xD6\xC0\xE0\xCC\xD6\xC2\xDB\xC7\xF8\xD2\xD4 New \xCF\xD4\xCA\xBE",
	/* DEF_CIRCLE             ÔÄ¶ÁÎÄÕÂÊÇ·ñÊ¹ÓÃÈÆ¾íÑ¡Ôñ */
	"\xD4\xC4\xB6\xC1\xCE\xC4\xD5\xC2\xCA\xC7\xB7\xF1\xCA\xB9\xD3\xC3\xC8\xC6\xBE\xED\xD1\xA1\xD4\xF1",
	/* DEF_FIRSTNEW           ÔÄ¶ÁÎÄÕÂÓÎ±êÍ£ÓÚµÚÒ»ÆªÎ´¶Á */
	"\xD4\xC4\xB6\xC1\xCE\xC4\xD5\xC2\xD3\xCE\xB1\xEA\xCD\xA3\xD3\xDA\xB5\xDA\xD2\xBB\xC6\xAA\xCE\xB4\xB6\xC1",
	/* DEF_LOGFRIEND          ½øÕ¾Ê±ÏÔÊ¾ºÃÓÑÃûµ¥ */
	"\xBD\xF8\xD5\xBE\xCA\xB1\xCF\xD4\xCA\xBE\xBA\xC3\xD3\xD1\xC3\xFB\xB5\xA5",
	/* DEF_INNOTE             ½øÕ¾Ê±ÏÔÊ¾±¸ÍüÂ¼ */
	"\xBD\xF8\xD5\xBE\xCA\xB1\xCF\xD4\xCA\xBE\xB1\xB8\xCD\xFC\xC2\xBC",
	/* DEF_OUTNOTE            ÀëÕ¾Ê±ÏÔÊ¾±¸ÍüÂ¼ */
	"\xC0\xEB\xD5\xBE\xCA\xB1\xCF\xD4\xCA\xBE\xB1\xB8\xCD\xFC\xC2\xBC",
	/* DEF_MAILMSG            ÀëÕ¾Ê±Ñ¯ÎÊ¼Ä»ØËùÓĞÑ¶Ï¢ */
	"\xC0\xEB\xD5\xBE\xCA\xB1\xD1\xAF\xCE\xCA\xBC\xC4\xBB\xD8\xCB\xF9\xD3\xD0\xD1\xB6\xCF\xA2",
	/* DEF_LOGOUT             Ê¹ÓÃ×Ô¼ºµÄÀëÕ¾»­Ãæ */
	"\xCA\xB9\xD3\xC3\xD7\xD4\xBC\xBA\xB5\xC4\xC0\xEB\xD5\xBE\xBB\xAD\xC3\xE6",
	/* DEF_SEEWELC1           ÎÒÊÇÕâ¸ö×éÖ¯µÄ³ÉÔ± */
	"\xCE\xD2\xCA\xC7\xD5\xE2\xB8\xF6\xD7\xE9\xD6\xAF\xB5\xC4\xB3\xC9\xD4\xB1",
	/* DEF_LOGINFROM          ºÃÓÑÉÏÕ¾Í¨Öª */
	"\xBA\xC3\xD3\xD1\xC9\xCF\xD5\xBE\xCD\xA8\xD6\xAA",
	/* DEF_NOTEPAD            ¹Û¿´ÁôÑÔ°æ */
	"\xB9\xDB\xBF\xB4\xC1\xF4\xD1\xD4\xB0\xE6",
	/* DEF_NOLOGINSEND        ²»ÒªËÍ³öÉÏÕ¾Í¨Öª¸øºÃÓÑ */
	"\xB2\xBB\xD2\xAA\xCB\xCD\xB3\xF6\xC9\xCF\xD5\xBE\xCD\xA8\xD6\xAA\xB8\xF8\xBA\xC3\xD3\xD1",
	/* DEF_THESIS             Ö÷ÌâÊ½¿´°æ */
	"\xD6\xF7\xCC\xE2\xCA\xBD\xBF\xB4\xB0\xE6",
	/* DEF_MSGGETKEY          ÊÕµ½Ñ¶Ï¢µÈºò»ØÓ¦»òÇå³ı */
	"\xCA\xD5\xB5\xBD\xD1\xB6\xCF\xA2\xB5\xC8\xBA\xF2\xBB\xD8\xD3\xA6\xBB\xF2\xC7\xE5\xB3\xFD",
	/* DEF_DELDBLCHAR         ºº×ÖÕû×Ö´¦Àí */
	"\xBA\xBA\xD7\xD6\xD5\xFB\xD7\xD6\xB4\xA6\xC0\xED",
	/* DEF_USEGB KCN 99.09.03 Ê¹ÓÃGBÂëÔÄ¶Á */
	"\xCA\xB9\xD3\xC3\x47\x42\xC2\xEB\xD4\xC4\xB6\xC1",
	/* DEF_ANIENDLINE         Ê¹ÓÃ¶¯Ì¬µ×Ïß */
	"\xCA\xB9\xD3\xC3\xB6\xAF\xCC\xAC\xB5\xD7\xCF\xDF",
	/* DEF_INTOANN            ³õ´Î·ÃÎÊ°æÃæÌáÊ¾½øÈë¾«»ªÇø */
	"\xB3\xF5\xB4\xCE\xB7\xC3\xCE\xCA\xB0\xE6\xC3\xE6\xCC\xE1\xCA\xBE\xBD\xF8\xC8\xEB\xBE\xAB\xBB\xAA\xC7\xF8",
	/* DEF_POSTNOMSG          ·¢±íÎÄÕÂÊ±ÔİÊ±ÆÁ±ÎMSG */
	"\xB7\xA2\xB1\xED\xCE\xC4\xD5\xC2\xCA\xB1\xD4\xDD\xCA\xB1\xC6\xC1\xB1\xCEMSG",
	/* DEF_SEESTATINLOG       ½øÕ¾Ê±¹Û¿´Í³¼ÆĞÅÏ¢ */
	"\xBD\xF8\xD5\xBE\xCA\xB1\xB9\xDB\xBF\xB4\xCD\xB3\xBC\xC6\xD0\xC5\xCF\xA2",
	/* DEF_FILTERXXX          ¹ıÂË¿ÉÄÜÁîÈË·´¸ĞĞÅÏ¢ */
	"\xB9\xFD\xC2\xCB\xBF\xC9\xC4\xDC\xC1\xEE\xC8\xCB\xB7\xB4\xB8\xD0\xD0\xC5\xCF\xA2",
	/* DEF_INTERNETMAIL       ÊÕÈ¡Õ¾ÍâĞÅ¼ş */
	//"\xCA\xD5\xC8\xA1\xD5\xBE\xCD\xE2\xD0\xC5\xBC\xFE",
	/* DEF_NEWSTOP10          ½øÕ¾Ê±¹Û¿´È«¹úÊ®´óÅÅĞĞ°ñ */
	"\xBD\xF8\xD5\xBE\xCA\xB1\xB9\xDB\xBF\xB4\xC8\xAB\xB9\xFA\xCA\xAE\xB4\xF3\xC5\xC5\xD0\xD0\xB0\xF1"
};

static int
loadkeys(struct one_key *key, char *name)
{
	int i;
	FILE *fp;
	fp = fopen(name, "r");
	if (fp == NULL)
		return 0;
	i = 0;
	while (key[i].fptr != NULL) {
		fread(&(key[i].key), sizeof (int), 1, fp);
		i++;
	}
	fclose(fp);
	return 1;
}

static int
savekeys(struct one_key *key, char *name)
{
	int i;
	FILE *fp;
	fp = fopen(name, "w");
	if (fp == NULL)
		return 0;
	i = 0;
	while (key[i].fptr != NULL) {
		fwrite(&(key[i].key), sizeof (int), 1, fp);
		i++;
	}
	fclose(fp);
	return 1;
}

void
loaduserkeys()
{
	char tempname[STRLEN];
	setuserfile(tempname, "readkey");
	loadkeys(read_comms, tempname);
	setuserfile(tempname, "mailkey");
	loadkeys(mail_comms, tempname);
	setuserfile(tempname, "friendkey");
	loadkeys(friend_list, tempname);
	setuserfile(tempname, "rejectkey");
	loadkeys(reject_list, tempname);
}

int
modify_user_mode(mode)
int mode;
{
	if (uinfo.mode == mode)
		return 0;
	uinfo.mode = mode;
	update_ulist(&uinfo, utmpent);
	return 0;
}

int
x_csh()
{
	char buf[PASSLEN];
	int save_pager;
	int magic;

	return -1;

	if (!HAS_PERM(PERM_SYSOP)) {
		return -1;
	}
	if (!check_systempasswd()) {
		return -1;
	}
	modify_user_mode(SYSINFO);
	clear();
	getdata(1, 0, "ÇëÊäÈëÍ¨ĞĞ°µºÅ: ", buf, PASSLEN, NOECHO, YEA);
	if (*buf == '\0' || !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
		prints("\n\n°µºÅ²»ÕıÈ·, ²»ÄÜÖ´ĞĞ¡£\n");
		pressreturn();
		clear();
		return -1;
	}
	randomize();
	magic = rand() % 1000;
	prints("\nMagic Key: %d", magic * 3 - 1);
	getdata(4, 0, "Your Key : ", buf, PASSLEN, NOECHO, YEA);
	if (*buf == '\0' || !(atoi(buf) == magic)) {
		securityreport("Fail to shell out", "Fail to shell out");
		prints("\n\nKey ²»ÕıÈ·, ²»ÄÜÖ´ĞĞ¡£\n");
		pressreturn();
		clear();
		return -1;
	}
	securityreport("Shell out", "Shell out");
	modify_user_mode(SYSINFO);
	clear();
	refresh();
	save_pager = uinfo.pager;
	uinfo.pager = 0;
	update_utmp();
	do_exec("csh", NULL);
	uinfo.pager = save_pager;
	update_utmp();
	clear();
	return 0;
}

int
showperminfo(pbits, i, use_define)
unsigned int pbits;
int i, use_define;
{
	char buf[STRLEN];

	sprintf(buf, "%c. %-30s %3s", 'A' + i,
		(use_define) ? g_user_definestr[i] : g_permstrings[i],
		((pbits >> i) & 1 ? "ON" : "OFF"));
	move(i + 6 - ((i > 15) ? 16 : 0), 0 + ((i > 15) ? 40 : 0));
	prints(buf);
	return YEA;
}

static void
keyprint(char *buf, int key)
{
	if (isprint(key))
		sprintf(buf, "%c", key);
	else {
		switch (key) {
		case KEY_TAB:
			strcpy(buf, "TAB");
			break;
		case KEY_ESC:
			strcpy(buf, "ESC");
			break;
		case KEY_UP:
			strcpy(buf, "UP");
			break;
		case KEY_DOWN:
			strcpy(buf, "DOWN");
			break;
		case KEY_RIGHT:
			strcpy(buf, "RIGHT");
			break;
		case KEY_LEFT:
			strcpy(buf, "LEFT");
			break;
		case KEY_HOME:
			strcpy(buf, "HOME");
			break;
		case KEY_INS:
			strcpy(buf, "INS");
			break;
		case KEY_DEL:
			strcpy(buf, "DEL");
			break;
		case KEY_END:
			strcpy(buf, "END");
			break;
		case KEY_PGUP:
			strcpy(buf, "PGUP");
			break;
		case KEY_PGDN:
			strcpy(buf, "PGDN");
			break;
		default:
			if (isprint(key | 0140))
				sprintf(buf, "Ctrl+%c", key | 0140);
			else
				sprintf(buf, "%x", key);
		}
	}
}

static int
showkeyinfo(struct one_key *akey, int i)
{
	char buf[STRLEN];
	char buf2[15];
	keyprint(buf2, (akey + i)->key);
	sprintf(buf, "%c. %-26s %6s", '0' + i, (akey + i)->func, buf2);
	move(i + 2 - ((i > 19) ? 20 : 0), 0 + ((i > 19) ? 40 : 0));
	prints(buf);
	return YEA;
}

static unsigned int
setkeys(struct one_key *key)
{
	int i, j, done = NA;
	char choice[3];
	prints("Çë°´ÏÂÄãÒªµÄ´úÂëÀ´Éè¶¨¼üÅÌ£¬°´ Enter ½áÊø.\n");
	i = 0;
	while (key[i].fptr != NULL && i < 40) {
		showkeyinfo(key, i);
		i++;
	}
	while (!done) {
		getdata(t_lines - 1, 0, "Ñ¡Ôñ(ENTER ½áÊø): ", choice, 2, DOECHO,
			YEA);
		*choice = toupper(*choice);
		if (*choice == '\n' || *choice == '\0')
			done = YEA;
		else if (*choice < '0' || *choice > '0' + i - 1)
			bell();
		else {
			j = *choice - '0';
			move(t_lines - 1, 0);
			prints("Çë¶¨Òå[\033[35m%s\033[m]µÄ¹¦ÄÜ¼ü:",
			       key[j].func);
			key[j].key = igetkey();
			showkeyinfo(key, j);
			/* d pbits ^= (1 << i);
			   if((*showfunc)( pbits, i ,YEA)==NA)
			   {
			   pbits ^= (1 << i);
			   } */
		}
	}
	pressreturn();
	return 0;
}

int
x_copykeys()
{
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	char ans[3];
	clear();
	move(0, 0);
	clrtoeol();

	prints("ÇëÑ¡ÔñÄãÒª»Ö¸´µÄÄ¬ÈÏ¼üÎ»,°´ Enter ½áÊø.\n");
	getdata(1, 0, "[0]Àë¿ª [1]°æÃæ1 [2]°æÃæ2 [3]ÓÊÏä [4]ºÃÓÑ [5]»µÈË [6]È«²¿ (Ä¬ÈÏ[0]):", ans, 2, DOECHO, YEA);

	if (ans[0] == '0' || ans[0] == '\n' || ans[0] == '\0')
			return 0;

	if (ans[0] == '3' || ans[0] == '6')
	{
		int i = 0;
		while (mail_default_comms[i].fptr != NULL) {
			mail_comms[i].key= mail_default_comms[i].key;
			i++;
		}
		setuserfile(tempname, "mailkey");
		savekeys(&mail_comms[0], tempname);

	}

	prints("ÉèÖÃÍê±Ï");
	pressreturn();
	return 0;
}

/*
int
x_copykeys()
{
	int i, toc = 0;
	char choice[3];
	FILE *fp;
	char buf[STRLEN];
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	clear();
	move(0, 0);
	clrtoeol();
	prints("Çë°´ÏÂÄãÒªµÄ´úÂëÀ´Éè¶¨¼üÅÌÀàĞÍ,°´ Enter ½áÊø.\n");
	fp = fopen(MY_BBS_HOME "/etc/keys", "r");
	if (fp == NULL)
		return -1;
	i = 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		sprintf(tempname, "%c. %-26s", 'A' + i, buf);
		move(i + 2 - ((i > 19) ? 20 : 0), 0 + ((i > 19) ? 40 : 0));
		prints(tempname);
		i++;
	}
	getdata(t_lines - 1, 0, "Ñ¡Ôñ(ENTER ½áÊø): ", choice, 2, DOECHO, YEA);
	*choice = toupper(*choice);
	if (*choice == '\n' || *choice == '\0')
		return 0;
	else if (*choice < 'A' || *choice > 'A' + i - 1)
		bell();
	else {
		toc = 0;
		i = *choice - 'A';
		if (currentuser.userlevel & PERM_SYSOP) {
			getdata(t_lines - 1, 0,
				"ÊÇ·ñ°Ñµ±Ç°Äã×Ô¼ºµÄ¼üÅÌ¶¨ÒåÉèÖÃÎªÏµÍ³¶¨Òå(Y/N):N ",
				choice, 2, DOECHO, YEA);
			if (*choice == 'Y')
				toc = 1;
		}
		if (toc) {
			sprintf(buf, MY_BBS_HOME "/etc/readkey.%d", i);
			savekeys(&read_comms[0], buf);
			sprintf(buf, MY_BBS_HOME "/etc/mailkey.%d", i);
			savekeys(&mail_comms[0], buf);
			sprintf(buf, MY_BBS_HOME "/etc/friendkey.%d", i);
			savekeys(&friend_list[0], buf);
			sprintf(buf, MY_BBS_HOME "/etc/rejectkey.%d", i);
			savekeys(&reject_list[0], buf);
		} else {
			sprintf(buf, MY_BBS_HOME "/etc/readkey.%d", i);
			loadkeys(&read_comms[0], buf);
			setuserfile(buf, "readkey");
			savekeys(&read_comms[0], buf);
			sprintf(buf, MY_BBS_HOME "/etc/mailkey.%d", i);
			loadkeys(&mail_comms[0], buf);
			setuserfile(buf, "mailkey");
			savekeys(&mail_comms[0], buf);
			sprintf(buf, MY_BBS_HOME "/etc/friendkey.%d", i);
			loadkeys(&friend_list[0], buf);
			setuserfile(buf, "friendkey");
			savekeys(&friend_list[0], buf);
			sprintf(buf, MY_BBS_HOME "/etc/rejectkey.%d", i);
			loadkeys(&reject_list[0], buf);
			setuserfile(buf, "rejectkey");
			savekeys(&reject_list[0], buf);
		}
		move(t_lines - 1, 0);
		prints("ÉèÖÃÍê±Ï");
	}
	pressreturn();
	return 0;
}
*/

int
x_setkeys()
{
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	clear();
	move(0, 0);
	clrtoeol();
	setkeys(&read_comms[0]);
	setuserfile(tempname, "readkey");
	savekeys(&read_comms[0], tempname);
	return 0;
}

int
x_setkeys2()
{
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	clear();
	move(0, 0);
	clrtoeol();
	setkeys(&read_comms[40]);
	setuserfile(tempname, "readkey");
	savekeys(&read_comms[0], tempname);
	return 0;
}

int
x_setkeys3()
{
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	clear();
	move(0, 0);
	clrtoeol();
	setkeys(&mail_comms[0]);
	setuserfile(tempname, "mailkey");
	savekeys(&mail_comms[0], tempname);
	return 0;
}

int
x_setkeys4()
{
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	clear();
	move(0, 0);
	clrtoeol();
	setkeys(&friend_list[0]);
	setuserfile(tempname, "friendkey");
	savekeys(&friend_list[0], tempname);
	return 0;
}

int
x_setkeys5()
{
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	clear();
	move(0, 0);
	clrtoeol();
	setkeys(&reject_list[0]);
	setuserfile(tempname, "rejectdkey");
	savekeys(&reject_list[0], tempname);
	return 0;
}

unsigned int
setperms(pbits, prompt, numbers, showfunc, param)
unsigned int pbits;
char *prompt;
int numbers;
int (*showfunc) (unsigned int, int, int);
int param;
{
	int lastperm = numbers - 1;
	int i, done = NA;
	char choice[3];

	move(4, 0);
	prints("Çë°´ÏÂÄãÒªµÄ´úÂëÀ´Éè¶¨%s£¬°´ Enter ½áÊø.\n", prompt);
	move(6, 0);
	clrtobot();
/*    pbits &= (1 << numbers) - 1;*/
	for (i = 0; i <= lastperm; i++) {
		(*showfunc) (pbits, i, param);
	}
	while (!done) {
		getdata(t_lines - 1, 0, "Ñ¡Ôñ(ENTER ½áÊø): ", choice, 2, DOECHO,
			YEA);
		*choice = toupper(*choice);
		if (*choice == '\n' || *choice == '\0')
			done = YEA;
		else if (*choice < 'A' || *choice > 'A' + lastperm)
			bell();
		else {
			i = *choice - 'A';
			pbits ^= (1 << i);
			if ((*showfunc) (pbits, i, param) == NA) {
				pbits ^= (1 << i);
			}
		}
	}
	return (pbits);
}

int
x_level()
{
	int id, oldlevel;
	unsigned int newlevel;
	char content[1024];

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();
	move(0, 0);
	prints("Change User Priority\n");
	clrtoeol();
	move(1, 0);
	usercomplete("Enter userid to be changed: ", genbuf);
	if (genbuf[0] == '\0') {
		clear();
		return 0;
	}
	if (!(id = getuser(genbuf))) {
		move(3, 0);
		prints("Invalid User Id");
		clrtoeol();
		pressreturn();
		clear();
		return -1;
	}
	move(1, 0);
	clrtobot();
	move(2, 0);
	prints("Set the desired permissions for user '%s'\n", genbuf);
	newlevel =
	    setperms(lookupuser.userlevel, "È¨ÏŞ", NUMPERMS, showperminfo, 0);
	move(2, 0);
	if (newlevel == lookupuser.userlevel)
		prints("User '%s' level NOT changed\n", lookupuser.userid);
	else {
		oldlevel = lookupuser.userlevel;
		lookupuser.userlevel = newlevel;
		{
			char secu[STRLEN];
			sprintf(secu, "ĞŞ¸Ä %s µÄÈ¨ÏŞ", lookupuser.userid);
			permtostr(oldlevel, genbuf);
			sprintf(content, "ĞŞ¸ÄÇ°µÄÈ¨ÏŞ£º%s\nĞŞ¸ÄºóµÄÈ¨ÏŞ£º",
				genbuf);
			permtostr(lookupuser.userlevel, genbuf);
			strcat(content, genbuf);
			securityreport(secu, content);
		}
		substitute_record(PASSFILE, &lookupuser, sizeof (struct userec),
				  id);
		prints("User '%s' level changed\n", lookupuser.userid);
	}
	pressreturn();
	clear();
	return 0;
}

int
x_userdefine()
{
	int id;
	unsigned int newlevel;
	extern int nettyNN;

	modify_user_mode(USERDEF);
	if (!(id = getuser(currentuser.userid))) {
		move(3, 0);
		prints("´íÎóµÄÊ¹ÓÃÕß ID...");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	move(1, 0);
	clrtobot();
	move(2, 0);
	newlevel =
	    setperms(lookupuser.userdefine, "²ÎÊı", NUMDEFINES, showperminfo,
		     1);
	move(2, 0);
	if (newlevel == lookupuser.userdefine)
		prints("²ÎÊıÃ»ÓĞĞŞ¸Ä...\n");
	else {
		lookupuser.userdefine = newlevel;
		currentuser.userdefine = newlevel;
		if ((!g_convcode && !(newlevel & DEF_USEGB))
		    || (g_convcode && (newlevel & DEF_USEGB)))
			switch_code();
		substitute_record(PASSFILE, &lookupuser, sizeof (struct userec),
				  id);
		uinfo.pager |= FRIEND_PAGER;
		if (!(uinfo.pager & ALL_PAGER)) {
			if (!DEFINE(DEF_FRIENDCALL))
				uinfo.pager &= ~FRIEND_PAGER;
		}
		uinfo.pager &= ~ALLMSG_PAGER;
		uinfo.pager &= ~FRIENDMSG_PAGER;
		if (DEFINE(DEF_DELDBLCHAR))
			enabledbchar = 1;
		else
			enabledbchar = 0;
		if (DEFINE(DEF_FRIENDMSG)) {
			uinfo.pager |= FRIENDMSG_PAGER;
		}
		if (DEFINE(DEF_ALLMSG)) {
			uinfo.pager |= ALLMSG_PAGER;
			uinfo.pager |= FRIENDMSG_PAGER;
		}
		update_utmp();
		if (DEFINE(DEF_ACBOARD))
			nettyNN = NNread_init();
		prints("ĞÂµÄ²ÎÊıÉè¶¨Íê³É...\n\n");
	}
	iscolor = (DEFINE(DEF_COLOR)) ? 1 : 0;
	pressreturn();
	clear();
	return 0;
}

int
x_cloak()
{
	modify_user_mode(GMENU);
	uinfo.invisible = (uinfo.invisible) ? NA : YEA;
	update_utmp();
	if ((currentuser.userlevel & PERM_BOARDS))
		setbmstatus(1);
	if (!uinfo.in_chat) {
		move(1, 0);
		clrtoeol();
	        if(uinfo.invisible)				//add by mintbaggio@BMY for normal cloak
			currentuser./*pseudo_*/lastlogout = time(NULL);
		else	currentuser./*pseudo_*/lastlogout = 0;
		substitute_record(PASSFILE, &currentuser, sizeof (currentuser),
                                  usernum);
		prints("ÒşÉíÊõ (cloak) ÒÑ¾­%sÁË!",
		       (uinfo.invisible) ? "Æô¶¯" : "Í£Ö¹");
		pressreturn();
	}
	return 0;
}

void
x_edits()
{
	int aborted;
	char ans[7], buf[STRLEN];
	int ch, num, confirm;
	extern int WishNum;
	static char *const e_file[] =
	    { "plans", "signatures", "notes", "logout", "GoodWish", "bansite",
		NULL
	};
	static char *const explain_file[] =
	    { "¸öÈËËµÃ÷µµ", "Ç©Ãûµµ", "×Ô¼ºµÄ±¸ÍüÂ¼", "ÀëÕ¾µÄ»­Ãæ",
		"µ×²¿Á÷¶¯ĞÅÏ¢", "½ûÖ¹±¾IDµÇÂ¼µÄIP(Ã¿¸öIPÕ¼Ò»ĞĞ)",
		NULL
	};

	modify_user_mode(GMENU);
	clear();
	move(1, 0);
	prints("±àĞŞ¸öÈËµµ°¸\n\n");
	for (num = 0; e_file[num] != NULL && explain_file[num] != NULL; num++) {
		prints("[[1;32m%d[m] %s\n", num + 1, explain_file[num]);
	}
	prints("[[1;32m%d[m] ¶¼²»Ïë¸Ä\n", num + 1);

	getdata(num + 5, 0, "ÄãÒª±àĞŞÄÄÒ»Ïî¸öÈËµµ°¸: ", ans, 2, DOECHO, YEA);
	if (ans[0] - '0' <= 0 || ans[0] - '0' > num || ans[0] == '\n'
	    || ans[0] == '\0')
		return;

	ch = ans[0] - '0' - 1;
	setuserfile(genbuf, e_file[ch]);
	move(3, 0);
	clrtobot();
	sprintf(buf, "(E)±à¼­ (D)É¾³ı %s? [E]: ", explain_file[ch]);
	getdata(3, 0, buf, ans, 2, DOECHO, YEA);
	if (ans[0] == 'D' || ans[0] == 'd') {
		confirm = askyn("ÄãÈ·¶¨ÒªÉ¾³ıÕâ¸öµµ°¸", NA, NA);
		if (confirm != 1) {
			move(5, 0);
			prints("È¡ÏûÉ¾³ıĞĞ¶¯\n");
			pressreturn();
			clear();
			return;
		}
		unlink(genbuf);
		move(5, 0);
		prints("%s ÒÑÉ¾³ı\n", explain_file[ch]);
		pressreturn();
		if (ch == 4)
			WishNum = 9999;
		clear();
		return;
	}
	modify_user_mode(EDITUFILE);
	aborted = vedit(genbuf, NA, YEA);
	clear();
	if (!aborted) {
		prints("%s ¸üĞÂ¹ı\n", explain_file[ch]);
		sprintf(buf, "edit %s", explain_file[ch]);
		if (!strcmp(e_file[ch], "signatures")) {
			set_numofsig();
			prints("ÏµÍ³ÖØĞÂÉè¶¨ÒÔ¼°¶ÁÈëÄãµÄÇ©Ãûµµ...");
		}
	} else
		prints("%s È¡ÏûĞŞ¸Ä\n", explain_file[ch]);
	pressreturn();
	if (ch == 4)
		WishNum = 9999;
}

void
a_edits()
{
	int aborted;
	char ans[7], buf[STRLEN], buf2[STRLEN];
	int ch, num, confirm;
	/*modified by pzhg 070412 for voteing for valid ids in spacific boards*/
	static char *const e_file[] =
	    { "../Welcome", "../Welcome2", "issue", "logout", "movie",
		"endline", "../vote/notes",
		"menu.ini", "badname0", "badname", "../.bad_email",
		"../.bansite",
		"../.blockmail",
		"autopost", "junkboards", "sysops", "prisonor", "untrust",
		"bbsnetA.ini", "bbsnet.ini", "filtertitle",
		"../ftphome/ftp_adm", "badwords", "sbadwords", "pbadwords",
		"../inndlog/newsfeeds.bbs", "spec_site", "secmlist","special","life",
		"commendlist", "manager_team","./pop_register/mail.xjtu.edu.cn","./pop_register/stu.xjtu.edu.cn",
		"top10forbid","voteidboards","newboard","recommboard","guestbanip",NULL
	};
	static char *const explain_file[] =
	    { "ÌØÊâ½øÕ¾¹«²¼À¸", "½øÕ¾»­Ãæ", "½øÕ¾»¶Ó­µµ", "ÀëÕ¾»­Ãæ",
		"»î¶¯¿´°æ", "ÆÁÄ»µ×Ïß", "¹«ÓÃ±¸ÍüÂ¼", "menu.ini",
		"²»¿É×¢²áµÄ ID", "ID ÖĞ²»ÄÜ°üº¬µÄ×Ö´®", "²»¿ÉÈ·ÈÏÖ®E-Mail",
		"²»¿ÉÉÏÕ¾Ö®Î»Ö·",
		"¾ÜÊÕE-mailºÚÃûµ¥", "Ã¿ÈÕ×Ô¶¯ËÍĞÅµµ", "²»ËãPOSTÊıµÄ°æ",
		"¹ÜÀíÕßÃûµ¥", "·şĞÌÈËÔ±Ãûµ¥", "²»ĞÅÈÎIPÁĞ±í",
		"±¾µØÍøÂçÁ¬Ïß", "ÍøÂçÁ¬Ïß", "µ×ÏßĞèÒª¹ıÂËµÄ±êÌâ",
		"FTP¹ÜÀíÔ±Ãûµ¥", "¹ıÂË´Ê»ã", "¾«¼ò¹ıÂË´Ê»ã", "±¨¾¯´Ê»ã",
		"×ªĞÅ°æºÍĞÂÎÅ×é¶ÔÓ¦", "´©ËóIPÏŞÖÆ´ÎÊı", "Ö÷¹ÜÕ¾³¤", "id±êÊ¶",
		"ÉúÃüÁ¦Éè¶¨", "ÍÆ¼öÎÄÕÂ", "¹ÜÀíÍÅ¶ÓÃûµ¥",
		"ĞÅÏä×¢²ámail.xjtu.edu.cn","ĞÅÏä×¢²ástu.xjtu.edu.cn",
		"²»ÔÊĞíÉÏ10´ó°æÃæ","ÏŞÖÆÍ¶Æ±È¨°æÃæ","ĞÂ¿ª°æÃæ","ÍÆ¼ö°æÃæ","½ûÖ¹guest·¢ÎÄµÄip",NULL
	};//modify by wjbta@bmy  Ôö¼Óid±êÊ¶µÄÏÔÊ¾, modify by mintbaggio 040406 for front page commend

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	move(0, 0);
	prints("±àĞŞÏµÍ³µµ°¸\n\n");
	for (num = 0;
	     HAS_PERM(PERM_SYSOP) ? e_file[num] != NULL
	     && explain_file[num] != NULL : strcasecmp(explain_file[num], "menu.ini") != 0;
	     num++) {
		if (num >= 20)
			move(num - 20 + 2, 39);
		prints("[%2d] %s\n", num + 1, explain_file[num]);
	}
	if (num >= 20)
		move(num - 20 + 2, 39);
	prints("[%2d] ¶¼²»Ïë¸Ä\n", num + 1);

	getdata(t_lines - 1, 0, "ÄãÒª±àĞŞÄÄÒ»ÏîÏµÍ³µµ°¸: ", ans, 3, DOECHO,
		YEA);
	ch = atoi(ans);
	if (!isdigit(ans[0]) || ch <= 0 || ch > num || ans[0] == '\n'
	    || ans[0] == '\0')
		return;
	ch -= 1;
	sprintf(buf2, "etc/%s", e_file[ch]);
	move(3, 0);
	clrtobot();
	sprintf(buf, "(E)±à¼­ (D)É¾³ı %s? [E]: ", explain_file[ch]);
	getdata(3, 0, buf, ans, 2, DOECHO, YEA);
	if (ans[0] == 'D' || ans[0] == 'd') {
		confirm = askyn("ÄãÈ·¶¨ÒªÉ¾³ıÕâ¸öÏµÍ³µµ", NA, NA);
		if (confirm != 1) {
			move(5, 0);
			prints("È¡ÏûÉ¾³ıĞĞ¶¯\n");
			pressreturn();
			clear();
			return;
		}
		{
			char secu[STRLEN];
			sprintf(secu, "É¾³ıÏµÍ³µµ°¸£º%s", explain_file[ch]);
			securityreport(secu, secu);
		}
		unlink(buf2);
		move(5, 0);
		prints("%s ÒÑÉ¾³ı\n", explain_file[ch]);
		pressreturn();
		clear();
		return;
	}
	modify_user_mode(EDITSFILE);
	aborted = vedit(buf2, NA, YEA);
	clear();
	if (aborted != -1) {
		prints("%s ¸üĞÂ¹ı", explain_file[ch]);
		{
			char secu[STRLEN];
			sprintf(secu, "ĞŞ¸ÄÏµÍ³µµ°¸£º%s", explain_file[ch]);
			securityreport(secu, secu);
		}

		if (!strcmp(e_file[ch], "../Welcome")) {
			unlink("Welcome.rec");
			prints("\nWelcome ¼ÇÂ¼µµ¸üĞÂ");
		}
	}
	pressreturn();
}

//added by pzhg for SysFiles2
void
a_edits2()
{
	int aborted;
	char ans[7], buf[STRLEN], buf2[STRLEN];
	int ch, num, confirm;
	static char *const e_file[] =
	    { "birthday","sysboards","adpost","ad_banner","ad_left", "secorder",NULL
	};
	static char *const explain_file[] =
	    { "ÉúÈÕ»¶Ó­»­Ãæ","Õ¾Îñ¹ÜÀí°æÃæ","¹ö¶¯¹ã¸æ","Banner", "Left Ads","WWWÌÖÂÛÇøË³Ğò",NULL
	};

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return;
	}
	clear();
	move(0, 0);
	prints("±àĞŞÏµÍ³µµ°¸2\n\n");
	for (num = 0;
	     HAS_PERM(PERM_SYSOP) ? e_file[num] != NULL
	     && explain_file[num] != NULL : strcasecmp(explain_file[num], "menu.ini") != 0;
	     num++) {
		if (num >= 20)
			move(num - 20 + 2, 39);
		prints("[%2d] %s\n", num + 1, explain_file[num]);
	}
	if (num >= 20)
		move(num - 20 + 2, 39);
	prints("[%2d] ¶¼²»Ïë¸Ä\n", num + 1);

	getdata(t_lines - 1, 0, "ÄãÒª±àĞŞÄÄÒ»ÏîÏµÍ³µµ°¸: ", ans, 3, DOECHO,
		YEA);
	ch = atoi(ans);
	if (!isdigit(ans[0]) || ch <= 0 || ch > num || ans[0] == '\n'
	    || ans[0] == '\0')
		return;
	ch -= 1;
	sprintf(buf2, "etc/%s", e_file[ch]);
	move(3, 0);
	clrtobot();
	sprintf(buf, "(E)±à¼­ (D)É¾³ı %s? [E]: ", explain_file[ch]);
	getdata(3, 0, buf, ans, 2, DOECHO, YEA);
	if (ans[0] == 'D' || ans[0] == 'd') {
		confirm = askyn("ÄãÈ·¶¨ÒªÉ¾³ıÕâ¸öÏµÍ³µµ", NA, NA);
		if (confirm != 1) {
			move(5, 0);
			prints("È¡ÏûÉ¾³ıĞĞ¶¯\n");
			pressreturn();
			clear();
			return;
		}
		{
			char secu[STRLEN];
			sprintf(secu, "É¾³ıÏµÍ³µµ°¸£º%s", explain_file[ch]);
			securityreport(secu, secu);
		}
		unlink(buf2);
		move(5, 0);
		prints("%s ÒÑÉ¾³ı\n", explain_file[ch]);
		pressreturn();
		clear();
		return;
	}
	modify_user_mode(EDITSFILE);
	aborted = vedit(buf2, NA, YEA);
	clear();
	if (aborted != -1) {
		prints("%s ¸üĞÂ¹ı", explain_file[ch]);
		{
			char secu[STRLEN];
			sprintf(secu, "ĞŞ¸ÄÏµÍ³µµ°¸£º%s", explain_file[ch]);
			securityreport(secu, secu);
		}
	}
	pressreturn();
}

void
x_lockscreen()
{
	char buf[PASSLEN + 1];
	time_t now;
	modify_user_mode(LOCKSCREEN);
	block_msg();
	move(9, 0);
	clrtobot();
	update_endline();
	buf[0] = '\0';
	char ubuf[3];
	char user_self[16];
	now = time(0);
	move(9, 0);
	prints
	    ("\n[1;37m       _       _____   ___     _   _   ___     ___       __"
	     "\n      ( )     (  _  ) (  _`\\  ( ) ( ) (  _`\\  (  _`\\    |  |"
	     "\n      | |     | ( ) | | ( (_) | |/'/' | (_(_) | | ) |   |  |"
	     "\n      | |  _  | | | | | |  _  | , <   |  _)_  | | | )   |  |"
	     "\n      | |_( ) | (_) | | (_( ) | |\\`\\  | (_( ) | |_) |   |==|"
	     "\n      (____/' (_____) (____/' (_) (_) (____/' (____/'   |__|[m\n");
	move(17,0);
	getdata(17,0,"ËøÆÁÀíÓÉ? (1)³Ô·¹È¥ÁË (2)ºÍMMÁÄÌì (3)±ğÀ´·³ÎÒ (4)Ã»ÀíÓÉ (5)×Ô¶¨Òå:",ubuf,3,DOECHO,YEA); //add by landefeng@BMY for ËøÆÁÀíÓÉ
	switch(ubuf[0]){
		case '1':
			modify_user_mode(USERDF1);break;
		case '2':
			modify_user_mode(USERDF2);break;
		case '3':
			modify_user_mode(USERDF3);break;
		case '5':						//add by leoncom@bmy ×Ô¶¨ÒåËøÆÁÀíÓÉ
			move(17,0);
			clrtobot();
			update_endline();
			if(HAS_PERM(PERM_SELFLOCK))
            {
			uinfo.user_state_temp[0]='\0';                  //Çå³ıÉÏ´Î¼ÇÂ¼
			update_ulist(&uinfo,utmpent);
	                getdata(17,0,"ÇëÊäÈë×Ô¶¨ÒåÀíÓÉ:",user_self,9,DOECHO,YEA);
		        int i=0,flag=0;
			for(i=0;i<=7;i++){
			if(user_self[i]==' ')
			{
				flag=1;
				break;
			}
			}
			if(!stringfilter(user_self,0)&&!flag)
			{
                         strcpy(uinfo.user_state_temp,user_self);
	                 update_ulist(&uinfo, utmpent);
			 move(17,0);
 			 clrtobot();
			 prints("ÄúµÄµ±Ç°ËøÆÁÀíÓÉÎª:%s",user_self);
			 modify_user_mode(USERDF4);break;
             }
			else
			{
				move(17,0);
				clrtobot();
				prints("ÄúÊäÈëµÄ×Ô¶¨ÒåÀíÓÉº¬ÓĞ²»ºÏÊÊ´Ê»ã»òÌØÊâ×Ö·û£¬½«ÒÔÄ¬ÈÏ·½Ê½ËøÆÁ");
				modify_user_mode(LOCKSCREEN);
				break;
			}
		   }
			else
			{
				move(17,0);
				clrtobot();
				prints("Äã±»¹ÜÀíÔ±È¡Ïû×Ô¶¨ÒåËøÆÁµÄÈ¨ÏŞ£¬½«ÒÔÄ¬ÈÏ·½Ê½Ëø¶¨");
				modify_user_mode(LOCKSCREEN);
				break;
			}
		default:
			prints("Ä¬ÈÏËøÆÁ");
			modify_user_mode(LOCKSCREEN);
	}
	move(18,0);
	clrtobot();
	prints("[1;36mÓ«Ä»ÒÑÔÚ[33m %19s[36m Ê±±»[32m %-12s [36mÔİÊ±Ëø×¡ÁË...[m",
	     ctime(&now), currentuser.userid);
	while (*buf == '\0' || !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
		move(19, 0);
		clrtobot();
		update_endline();
		getdata(19, 0, buf[0] == '\0' ? "ÇëÊäÈëÄãµÄÃÜÂëÒÔ½âËø: " :
			"ÄãÊäÈëµÄÃÜÂëÓĞÎó£¬ÇëÖØĞÂÊäÈëÄãµÄÃÜÂëÒÔ½âËø: ", buf,
			PASSLEN, NOECHO, YEA);
	}
	unblock_msg();
}

int
heavyload(float maxload)
{
	double cpu_load[3];
	if (maxload == 0)
		maxload = 15;
	get_load(cpu_load);
	if (cpu_load[0] > maxload)
		return 1;
	else
		return 0;
}

static void
myexec_cmd(umode, pager, cmdfile, param)
int umode, pager;
const char *cmdfile, *param;
{
	char buf[STRLEN * 2], param1[256];
	int save_pager;
	pid_t childpid;
	int p[2];
	param1[0] = 0;
	if (param != NULL) {
		char *avoid = "&;!`'\"|?~<>^()[]{}$\n\r\\", *ptr;
		int n = strlen(avoid);
		ytht_strsncpy(param1, param, sizeof(param1));
		while (n > 0) {
			n--;
			ptr = strchr(param1, avoid[n]);
			if (ptr != NULL)
				*ptr = 0;
		}
	}

	if (!HAS_PERM(PERM_SYSOP) && heavyload(0)) {
		clear();
		prints("±§Ç¸£¬Ä¿Ç°ÏµÍ³¸ººÉ¹ıÖØ£¬´Ë¹¦ÄÜÔİÊ±²»ÄÜÖ´ĞĞ...");
		pressanykey();
		return;
	}

	if (!dashf(cmdfile)) {
		move(2, 0);
		prints("no %s\n", cmdfile);
		pressreturn();
		return;
	}

	save_pager = uinfo.pager;
	if (pager == NA) {
		uinfo.pager = 0;
	}

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, p) < 0)
		return;

	modify_user_mode(umode);
	refresh();
	signal(SIGALRM, SIG_IGN);
	signal(SIGCHLD, SIG_DFL);
	childpid = fork();
	if (childpid == 0) {
		char pidstr[20];
		sprintf(pidstr, "%d", getppid());
		close(p[0]);
		if (p[1] != 0)
			dup2(p[1], 0);
		dup2(0, 1);
		dup2(0, 2);
		if (param1[0]) {
			snprintf(buf, sizeof (buf),
				 "%s exec %s \"%s\" %s %s %d",
				 currentuser.userid, cmdfile, param1,
				 currentuser.userid, uinfo.from, getppid());
			newtrace(buf);
			execl(cmdfile, cmdfile, param1, currentuser.userid,
			      uinfo.from, pidstr, NULL);
		} else {
			snprintf(buf, sizeof (buf), "%s exec %s %s %s %d",
				 currentuser.userid, cmdfile,
				 currentuser.userid, uinfo.from, getppid());
			newtrace(buf);
			execl(cmdfile, cmdfile, currentuser.userid, uinfo.from,
			      pidstr, NULL);
		}
		exit(0);
	} else if (childpid > 0) {
		close(p[1]);
		datapipefd(0, p[0]);
		close(p[0]);
		while (wait(NULL) != childpid)
			sleep(1);
	} else {
		close(p[0]);
		close(p[1]);
	}
	uinfo.pager = save_pager;
	signal(SIGCHLD, SIG_IGN);
	return;
}

static void
datapipefd(int fds, int fdn)
{
	fd_set rs;
	int retv, max;
	char buf[1024];

	max = 1 + ((fdn > fds) ? fdn : fds);
	FD_ZERO(&rs);
	while (1) {
		FD_SET(fds, &rs);
		FD_SET(fdn, &rs);
		retv = select(max, &rs, NULL, NULL, NULL);
		if (retv < 0) {
			if (errno != EINTR)
				break;
			continue;
		}
		if (FD_ISSET(fds, &rs)) {
#ifdef SSHBBS
			retv = ssh_read(fds, buf, sizeof (buf));
#else
			retv = read(fds, buf, sizeof (buf));
#endif
			if (retv > 0) {
				time(&now_t);
				uinfo.lasttime = now_t;
				if ((unsigned long) now_t -
				    (unsigned long) old > 60)
					update_utmp();
				write(fdn, buf, retv);
			} else if (retv == 0 || (retv < 0 && errno != EINTR))
				break;
			FD_CLR(fds, &rs);
		}
		if (FD_ISSET(fdn, &rs)) {
			retv = read(fdn, buf, sizeof (buf));
			if (retv > 0) {
#ifdef SSHBBS
				ssh_write(fds, buf, retv);
#else
				write(fds, buf, retv);
#endif
			} else if (retv == 0 || (retv < 0 && errno != EINTR))
				break;
			FD_CLR(fdn, &rs);
		}
	}
}

static void
exec_cmd(umode, pager, cmdfile, param1)
int umode, pager;
char *cmdfile, *param1;
{
	char buf[STRLEN * 2];
	int save_pager;

	{
		char *ptr = strchr(param1, ';');
		if (ptr != NULL)
			*ptr = 0;
	}

	if (num_useshell() >= 20) {
		clear();
		prints("Ì«¶àÈËÊ¹ÓÃÍâ²¿³ÌÊ½ÁË£¬ÄãµÈÒ»ÏÂÔÙÓÃ°É...");
		pressanykey();
		return;
	}
	if (!HAS_PERM(PERM_SYSOP) && heavyload(0)) {
		clear();
		prints("±§Ç¸£¬Ä¿Ç°ÏµÍ³¸ººÉ¹ıÖØ£¬´Ë¹¦ÄÜÔİÊ±²»ÄÜÖ´ĞĞ...");
		pressanykey();
		return;
	}
	if (!dashf(cmdfile)) {
		move(2, 0);
		prints("no %s\n", cmdfile);
		pressreturn();
		return;
	}
	save_pager = uinfo.pager;
	if (pager == NA) {
		uinfo.pager = 0;
	}
	modify_user_mode(umode);
	sprintf(buf, "/bin/sh %s %s %s %d", cmdfile, param1, currentuser.userid,
		getpid());
	sprintf(genbuf, "%s exec %s", currentuser.userid, buf);
	newtrace(genbuf);
	do_exec(buf, NULL);
	uinfo.pager = save_pager;
	clear();
}

int
sendgoodwish(char *uid)
{
	return sendGoodWish(NULL);
}

static int
sendGoodWish(char *userid)
{
	FILE *fp, *mp;
	int tuid, i, count;
	time_t now;
	char buf[5][STRLEN], tmpbuf[STRLEN], filebuf[STRLEN];
	char uid[IDLEN + 1], *timestr, ans[8], uident[13], tmp[3];
	int cnt, n, fmode = NA;
	char wishlists[STRLEN];

	modify_user_mode(GOODWISH);
	clear();
	move(1, 0);
	prints("[0;1;32mÁôÑÔ±¾[m\nÄú¿ÉÒÔÔÚÕâÀï¸øÄúµÄÅóÓÑËÍÈ¥ÄúµÄ×£¸££¬");
	prints("\nÒ²¿ÉÒÔÎªÄú¸øËû/ËıÉÓÉÏÒ»¾äÇÄÇÄ»°¡£");
	move(6, 0);

	if (userid == NULL) {
		getdata(3, 0,
			"(1)¸ø¸öÈËËÍÈ¥×£¸£ (2)¸øÒ»ÈºÈËËÍÈ¥×£¸£ (0) È¡Ïû [1]",
			ans, 2, DOECHO, YEA);
		if (ans[0] == '0') {
			clear();
			return 0;
		}
		if (ans[0] != '2') {
			usercomplete("ÇëÊäÈëËûµÄ ID: ", uid);
			if (uid[0] == '\0') {
				clear();
				return 0;
			}
		} else {
			clear();
			sethomefile(wishlists, currentuser.userid, "wishlist");
			cnt = listfilecontent(wishlists);
			while (1) {
				getdata(0, 0,
					"(A)Ôö¼Ó (D)É¾³ı (I)ÒıÈëºÃÓÑ (C)Çå³ıÄ¿Ç°Ãûµ¥ (E)·ÅÆú (S)ËÍ³ö?[S]£º ",
					tmp, 2, DOECHO, YEA);
				if (tmp[0] == '\n' || tmp[0] == '\0'
				    || tmp[0] == 's' || tmp[0] == 'S') {
					clear();
					break;
				}
				if (tmp[0] == 'a' || tmp[0] == 'd'
				    || tmp[0] == 'A' || tmp[0] == 'D') {
					move(1, 0);
					if (tmp[0] == 'a' || tmp[0] == 'A')
						usercomplete
						    ("ÇëÒÀ´ÎÊäÈëÊ¹ÓÃÕß´úºÅ(Ö»°´ ENTER ½áÊøÊäÈë): ",
						     uident);
					else
						namecomplete
						    ("ÇëÒÀ´ÎÊäÈëÊ¹ÓÃÕß´úºÅ(Ö»°´ ENTER ½áÊøÊäÈë): ",
						     uident);
					move(1, 0);
					clrtoeol();
					if (uident[0] == '\0')
						continue;
					if (!getuser(uident)) {
						move(2, 0);
						prints
						    ("Õâ¸öÊ¹ÓÃÕß´úºÅÊÇ´íÎóµÄ.\n");
					}
				}
				switch (tmp[0]) {
				case 'A':
				case 'a':
					if (seek_in_file(wishlists, uident)) {
						move(2, 0);
						prints
						    ("ÒÑ¾­ÁĞÎªÊÕ×£¸£ÈËÖ®Ò» \n");
						break;
					}
					addtofile(wishlists, uident);
					cnt++;
					break;
				case 'E':
				case 'e':
				case 'Q':
				case 'q':
					cnt = 0;
					break;
				case 'D':
				case 'd':
					if (seek_in_file(wishlists, uident)) {
						del_from_file(wishlists,
							      uident);
						cnt--;
					}
					break;
				case 'I':
				case 'i':
					n = 0;
					clear();
					for (i = cnt; n < uinfo.fnum; i++) {
						int key;
						move(2, 0);
						getuserid(uident,
							  uinfo.friend[n]);
						prints("%s\n", uident);
						move(3, 0);
						n++;
						prints
						    ("(A)È«²¿¼ÓÈë (Y)¼ÓÈë (N)²»¼ÓÈë (Q)½áÊø? [Y]:");
						if (!fmode)
							key = igetkey();
						else
							key = 'Y';
						if (key == 'q' || key == 'Q')
							break;
						if (key == 'A' || key == 'a') {
							fmode = YEA;
							key = 'Y';
						}
						if (key == '\0' || key == '\n'
						    || key == 'y' || key == 'Y') {
							if (!getuser(uident)) {
								move(4, 0);
								prints
								    ("Õâ¸öÊ¹ÓÃÕß´úºÅÊÇ´íÎóµÄ.\n");
								i--;
								continue;
							} else
							    if (seek_in_file
								(wishlists,
								 uident)) {
								i--;
								continue;
							}
							addtofile(wishlists,
								  uident);
							cnt++;
						}
					}	//for loop
					fmode = NA;
					clear();
					break;
				case 'C':
				case 'c':
					unlink(wishlists);
					cnt = 0;
					break;
				}
				if (strchr("EeQq", tmp[0]))
					break;
				move(5, 0);
				clrtobot();
				move(3, 0);
				clrtobot();
				listfilecontent(wishlists);
			}
			if (cnt <= 0)
				return 0;
			move(5, 0);
			clrtoeol();
			prints("[m¡¾ÇëÊäÈëÄúµÄÁôÑÔ¡¿       ");
			move(6, 0);
			tmpbuf[0] = '\0';
			prints
			    ("ÄúµÄÁôÑÔ[Ö±½Ó°´ ENTER ½áÊøÁôÑÔ£¬×î¶à 5 ¾ä£¬Ã¿¾ä×î³¤ 50 ×Ö·û]:");
			for (count = 0; count < 5; count++) {
				getdata(7 + count, 0, ": ", tmpbuf, 51, DOECHO,
					YEA);
				if (tmpbuf[0] == '\0')
					break;;
				strcpy(buf[count], tmpbuf);
				tmpbuf[0] = '\0';
			}
			if (count == 0)
				return 0;
			sprintf(genbuf, "ÄãÈ·¶¨Òª·¢ËÍÕâÌõÁôÑÔÂğ");
			move(9 + count, 0);
			if (askyn(genbuf, YEA, NA) == NA) {
				clear();
				return 0;
			}
			setuserfile(wishlists, "wishlist");
			if ((mp = fopen(wishlists, "r")) == NULL) {
				return -3;
			}
			for (n = 0; n < cnt; n++) {
				if (fgets(filebuf, STRLEN, mp) != NULL) {
					if (strtok(filebuf, " \n\r\t") != NULL)
						strcpy(uid, filebuf);
					else
						continue;
				}
				sethomefile(genbuf, uid, "GoodWish");
				if ((fp = fopen(genbuf, "a")) == NULL) {
					prints
					    ("ÎŞ·¨¿ªÆô¸ÃÓÃ»§µÄµ×²¿Á÷¶¯ĞÅÏ¢ÎÄ¼ş£¬ÇëÍ¨ÖªÕ¾³¤...\n");
					pressanykey();
					return NA;
				}
				now = time(0);
				timestr = ctime(&now) + 11;
				*(timestr + 5) = '\0';
				for (i = 0; i < count; i++) {
					fprintf(fp, "%s(%s)[%d/%d]£º%s\n",
						currentuser.userid, timestr,
						i + 1, count, buf[i]);
				}
				fclose(fp);
				sethomefile(genbuf, uid, "HaveNewWish");
				if ((fp = fopen(genbuf, "w+")) != NULL) {
					fputs("Have New Wish", fp);
					fclose(fp);
				}
				move(11 + count, 0);
				//prints("ÒÑ¾­°ïÄúËÍ³öÄúµÄÁôÑÔÁË¡£\n");
				sprintf(genbuf, "%s sendgoodwish %s",
					currentuser.userid, uid);
				newtrace(genbuf);
			}	//for loop
			return 0;
		}
	} else
		strcpy(uid, userid);
	if (!(tuid = getuser(uid))) {
		move(7, 0);
		prints("[1mÄúÊäÈëµÄÊ¹ÓÃÕß´úºÅ( ID )²»´æÔÚ£¡[m\n");
		pressanykey();
		clear();
		return -1;
	}
	move(5, 0);
	clrtoeol();
	prints("[m¡¾¸ø [1m%s[m ÁôÑÔ¡¿       ", uid);
	move(6, 0);
	tmpbuf[0] = '\0';
	prints("ÄúµÄÁôÑÔ[Ö±½Ó°´ ENTER ½áÊøÁôÑÔ£¬×î¶à 5 ¾ä£¬Ã¿¾ä×î³¤ 50 ×Ö·û]:");
	for (count = 0; count < 5; count++) {
		getdata(7 + count, 0, ": ", tmpbuf, 51, DOECHO, YEA);
		if (tmpbuf[0] == '\0')
			break;;
		strcpy(buf[count], tmpbuf);
		tmpbuf[0] = '\0';
	}
	if (count == 0)
		return 0;

	sprintf(genbuf, "ÄãÈ·¶¨Òª·¢ËÍÕâÌõÁôÑÔ¸ø [1m%s[m Âğ", uid);
	move(9 + count, 0);
	if (askyn(genbuf, YEA, NA) == NA) {
		clear();
		return 0;
	}
	sethomefile(genbuf, uid, "GoodWish");
	if ((fp = fopen(genbuf, "a")) == NULL) {
		prints("ÎŞ·¨¿ªÆô¸ÃÓÃ»§µÄµ×²¿Á÷¶¯ĞÅÏ¢ÎÄ¼ş£¬ÇëÍ¨ÖªÕ¾³¤...\n");
		pressanykey();
		return NA;
	}
	now = time(0);
	timestr = ctime(&now) + 11;
	*(timestr + 5) = '\0';
	for (i = 0; i < count; i++) {
		fprintf(fp, "%s(%s)[%d/%d]£º%s\n",
			currentuser.userid, timestr, i + 1, count, buf[i]);
	}
	fclose(fp);
	sethomefile(genbuf, uid, "HaveNewWish");
	if ((fp = fopen(genbuf, "w+")) != NULL) {
		fputs("Have New Wish", fp);
		fclose(fp);
	}
	move(11 + count, 0);
	prints("ÒÑ¾­°ïÄúËÍ³öÄúµÄÁôÑÔÁË¡£\n");
	sprintf(genbuf, "%s sendgoodwish %s", currentuser.userid, uid);
	newtrace(genbuf);
	/*sprintf(genbuf,"°ÑÕâÌõ×£¸£·¢ËÍ¸øÆäËûÈËÃ´?");
	   if(askyn(genbuf,YEA,NA)==YEA){
	   usercomplete("ÇëÊäÈëËûµÄ ID: ", uid);
	   if (uid[0] == '\0') {
	   clear();
	   return 0;
	   }
	   if (!(tuid = getuser(uid))) {
	   move(7, 0);
	   prints("\x1b[1mÄúÊäÈëµÄÊ¹ÓÃÕß´úºÅ( ID )²»´æÔÚ£¡\x1b[m\n");
	   pressanykey();
	   clear();
	   return -1;
	   }
	   goto loop;
	   } */
	pressanykey();
	clear();
	return 0;
}

/* ppfoong */
void
x_dict()
{
	char buf[STRLEN];
	char *s;
	//int whichdict;

	if (heavyload(0)) {
		clear();
		prints("±§Ç¸£¬Ä¿Ç°ÏµÍ³¸ººÉ¹ıÖØ£¬´Ë¹¦ÄÜÔİÊ±²»ÄÜÖ´ĞĞ...");
		pressanykey();
		return;
	}
	modify_user_mode(DICT);
	clear();
	prints("\n[1;32m     _____  __        __   __");
	prints
	    ("\n    |     \\|__|.----.|  |_|__|.-----.-----.---.-.----.--.--.");
	prints
	    ("\n    |  --  |  ||  __||   _|  ||  _  |     |  _  |   _|  |  |");
	prints
	    ("\n    |_____/|__||____||____|__||_____|__|__|___._|__| |___  |");
	prints
	    ("\n                                                     |_____|[m");
	prints("\n\n\n»¶Ó­Ê¹ÓÃ±¾Õ¾µÄ×Öµä¡£");
	prints
	    ("\n±¾×ÖµäÖ÷ÒªÎª[1;33m¡¸Ó¢ºº¡¹[m²¿·Ö, µ«Òà¿É×÷[1;33m¡¸ººÓ¢¡¹[m²éÑ¯¡£");
	prints
	    ("\n\nÏµÍ³½«¸ù¾İÄúËùÊäÈëµÄ×Ö´®, ×Ô¶¯ÅĞ¶ÏÄúËùÒª·­²éµÄÊÇÓ¢ÎÄ×Ö»¹ÊÇÖĞÎÄ×Ö¡£");
	prints("\n\n\nÇëÊäÈëÄúÓû·­²éµÄÓ¢ÎÄ×Ö»òÖĞÎÄ×Ö, »òÖ±½Ó°´ <ENTER> È¡Ïû¡£");
	getdata(15, 0, ">", buf, 30, DOECHO, YEA);
	if (buf[0] == '\0') {
		prints("\nÄú²»Ïë²éÁËà¸...");
		pressanykey();
		return;
	}
	for (s = buf; *s != '\0'; s++) {
		if (isspace(*s)) {
			prints("\nÒ»´ÎÖ»ÄÜ²éÒ»¸ö×ÖÀ², ²»ÄÜÌ«Ì°ĞÄà¸!!");
			pressanykey();
			return;
		}
	}
	myexec_cmd(DICT, YEA, "bin/cdict.sh", buf);
	sprintf(buf, "bbstmpfs/tmp/dict.%s.%d", currentuser.userid, uinfo.pid);
	if (dashf(buf)) {
		ansimore(buf, NA);
		if (askyn("Òª½«½á¹û¼Ä»ØĞÅÏäÂğ", NA, NA) == YEA)
			mail_file(buf, currentuser.userid, "×Öµä²éÑ¯½á¹û");
		unlink(buf);
	}
}

void
x_tt()
{
	myexec_cmd(TT, NA, "bin/tt", NULL);
	redoscr();
}

void
x_worker()
{
	myexec_cmd(WORKER, YEA, "bin/worker", NULL);
	redoscr();
}

void
x_tetris()
{
	myexec_cmd(TETRIS, NA, "bin/tetris", NULL);
	redoscr();
}

void
x_winmine()
{
	myexec_cmd(WINMINE, NA, "bin/winmine", NULL);
	redoscr();
}

void
x_winmine2()
{
	myexec_cmd(WINMINE2, NA, "bin/winmine2", NULL);
	redoscr();
}

void
x_recite()
{
	myexec_cmd(RECITE, NA, "bin/ptyexec", "bin/recite");
	redoscr();
}

void
x_ncce()
{
	myexec_cmd(NCCE, NA, "bin/ptyexec", "bin/ncce");
	redoscr();
}

void
x_chess()
{
	myexec_cmd(CHESS, NA, "bin/chc", NULL);
	redoscr();
}

void
x_qkmj()
{
	myexec_cmd(CHESS, NA, "bin/qkmj", NULL);
	redoscr();
}

void
x_quickcalc()
{
	clear();
	prints("\n-------Êı×ÖÔËËã, ÊäÈëhelp»ñµÃ°ïÖú------\n");
	myexec_cmd(QUICKCALC, NA, "bin/ptyexec", "bin/qc");
	redoscr();
}

void
x_freeip()
{
	clear();
	if (heavyload(2.5)) {
		prints("±§Ç¸£¬Ä¿Ç°ÏµÍ³¸ººÉ¹ıÖØ£¬´Ë¹¦ÄÜÔİÊ±²»ÄÜÖ´ĞĞ...");
		pressanykey();
		return;
	}
	myexec_cmd(FREEIP, NA, "bin/ptyexec", "bin/freeip");
	redoscr();
}

void
x_showuser()
{
	char buf[STRLEN];

	modify_user_mode(SYSINFO);
	clear();
	stand_title("±¾Õ¾Ê¹ÓÃÕß×ÊÁÏ²éÑ¯");
	ansimore("etc/showuser.msg", NA);
	getdata(20, 0, "Parameter: ", buf, 30, DOECHO, YEA);
	if ((buf[0] == '\0') || dashf("bbstmpfs/tmp/showuser.result"))
		return;
	securityreport("²éÑ¯Ê¹ÓÃÕß×ÊÁÏ", "²éÑ¯Ê¹ÓÃÕß×ÊÁÏ");
	exec_cmd(SYSINFO, YEA, "bin/showuser.sh", buf);
	sprintf(buf, "bbstmpfs/tmp/showuser.result");
	if (dashf(buf)) {
		mail_file(buf, currentuser.userid, "Ê¹ÓÃÕß×ÊÁÏ²éÑ¯½á¹û");
		unlink(buf);
	}
}

static void
childreturn(int i)
{

	int retv;
	while ((retv = waitpid(-1, NULL, WNOHANG | WUNTRACED)) > 0)
		if (childpid > 0 && retv == childpid)
			childpid = 0;
}

int
ent_bnet(char *cmd)
{
	int p[2];
#ifdef SSHBBS
	move(9, 0);
	clrtobot();
	prints("ÄúÄ¿Ç°ÊÇÊ¹ÓÃ ssh ·½Ê½Á¬½Ó %s.\n"
	       "ssh »á¶ÔÍøÂçÊı¾İ´«Êä½øĞĞ¼ÓÃÜ,±£»¤ÄúµÄÃÜÂëºÍÆäËüË½ÈËĞÅÏ¢.\n"
	       "Äú±ØĞëÖªµÀ,ÍøÂç´©ËóÊÇÍ¨¹ı±¾Õ¾Á¬½Óµ½ÆäËübbs,ËäÈ»´ÓÄúµÄ»úÆ÷\n"
	       "µ½±¾Õ¾¼äµÄÊı¾İ´«ÊäÊÇ¼ÓÃÜµÄ,µ«ÊÇ´Ó±¾Õ¾µ½ÁíÍâµÄ BBS Õ¾µã¼äµÄ\n"
	       "Êı¾İ´«Êä²¢Ã»ÓĞ¼ÓÃÜ,ÄúµÄÃÜÂëºÍÆäËüË½ÈËĞÅÏ¢ÓĞ±»µÚÈı·½ÇÔÌıµÄ\n¿ÉÄÜ.\n",
	       MY_BBS_NAME);
	if (askyn("ÄãÈ·¶¨ÒªÊ¹ÓÃÍøÂç´©ËóÂğ", NA, NA) != 1)
		return -1;
#endif
	signal(SIGALRM, SIG_IGN);
	signal(SIGCHLD, childreturn);
	modify_user_mode(BBSNET);
	do_delay(1);
	refresh();
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, p) < 0)
		return -1;
	if (p[0] <= 2 || p[1] <= 2) {
		int i = p[0] + p[1] + 1;
		dup2(p[0], i);
		dup2(p[1], i + 1);
		close(p[0]);
		close(p[1]);
		p[0] = i;
		p[1] = i + 1;
	}
	inBBSNET = 1;
	childpid = fork();
	if (childpid == 0) {
		close(p[1]);
		if (p[0] != 0)
			dup2(p[0], 0);
		dup2(0, 1);
		dup2(0, 2);
		execl("bin/bbsnet", "bbsnet",
		      (cmd[0] == 'A') ? "etc/bbsnetA.ini" : "etc/bbsnet.ini",
		      "bin/telnet", currentuser.userid, NULL);
		exit(0);
	} else if (childpid > 0) {
		close(p[0]);
		datapipefd(0, p[1]);
		close(p[1]);
	} else {
		close(p[0]);
		close(p[1]);
	}
	signal(SIGCHLD, SIG_IGN);
	inBBSNET = 0;
	redoscr();
//      r_msg();
	return 0;
}

int
x_denylevel()
{
	int id;
	char ans[7], content[1024];
	int oldlevel;
	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();
	move(0, 0);
	prints("¸ü¸ÄÊ¹ÓÃÕß»ù±¾È¨ÏŞ\n");
	clrtoeol();
	move(1, 0);
	usercomplete("ÊäÈëÓû¸ü¸ÄµÄÊ¹ÓÃÕßÕÊºÅ: ", genbuf);
	if (genbuf[0] == '\0') {
		clear();
		return 0;
	}
	if (!(id = getuser(genbuf))) {
		move(3, 0);
		prints("Invalid User Id");
		clrtoeol();
		pressreturn();
		clear();
		return -1;
	}
	move(1, 0);
	clrtobot();
	move(2, 0);
	prints("Éè¶¨Ê¹ÓÃÕß '%s' µÄ»ù±¾È¨ÏŞ \n\n", genbuf);
	prints("(1) ·â½û·¢±íÎÄÕÂÈ¨Àû       (A) »Ö¸´·¢±íÎÄÕÂÈ¨Àû\n");
	prints("(2) È¡Ïû»ù±¾ÉÏÕ¾È¨Àû       (B) »Ö¸´»ù±¾ÉÏÕ¾È¨Àû\n");
	prints("(3) ½ûÖ¹½øÈëÁÄÌìÊÒ         (C) »Ö¸´½øÈëÁÄÌìÊÒÈ¨Àû\n");
	prints("(4) ½ûÖ¹ºô½ĞËûÈËÁÄÌì       (D) »Ö¸´ºô½ĞËûÈËÁÄÌìÈ¨Àû\n");
	prints("(5) ÕûÀí¸öÈË¾«»ªÇø         (E) ²»ÄÜÕûÀí¸öÈË¾«»ªÇø\n");
	prints("(6) ½ûÖ¹·¢ËÍĞÅ¼ş           (F) »Ö¸´·¢ĞÅÈ¨Àû\n");
	prints("(7) ½ûÖ¹Ê¹ÓÃÇ©Ãûµµ         (G) »Ö¸´Ê¹ÓÃÇ©ÃûµµÈ¨Àû\n");
	prints("(8) ½ûÖ¹Ê¹ÓÃ×Ô¶¨ÒåËøÆÁ     (H) »Ö¸´Ê¹ÓÃ×Ô¶¨ÒåËøÆÁ\n");
	getdata(13, 0, "ÇëÊäÈëÄãµÄ´¦Àí: ", ans, 3, DOECHO, YEA);
	oldlevel = lookupuser.userlevel;
	switch (ans[0]) {
	case '1':
		lookupuser.userlevel &= ~PERM_POST;
		break;
	case 'a':
	case 'A':
		lookupuser.userlevel |= PERM_POST;
		break;
	case '2':
		lookupuser.userlevel &= ~PERM_BASIC;
		break;
	case 'b':
	case 'B':
		lookupuser.userlevel |= PERM_BASIC;
		break;
	case '3':
		lookupuser.userlevel &= ~PERM_CHAT;
		break;
	case 'c':
	case 'C':
		lookupuser.userlevel |= PERM_CHAT;
		break;
	case '4':
		lookupuser.userlevel &= ~PERM_PAGE;
		break;
	case 'd':
	case 'D':
		lookupuser.userlevel |= PERM_PAGE;
		break;
	case '5':
		lookupuser.userlevel |= PERM_SPECIAL8;
		break;
	case 'e':
	case 'E':
		lookupuser.userlevel &= ~PERM_SPECIAL8;
		break;
	case '6':
		lookupuser.userlevel |= PERM_DENYMAIL;
		break;
	case 'f':
	case 'F':
		lookupuser.userlevel &= ~PERM_DENYMAIL;
		break;
	case '7':
		lookupuser.userlevel |= PERM_DENYSIG;
		{
			getdata(13, 0, "½ûÖ¹Ê¹ÓÃÇ©ÃûµµµÄÔ­Òò£º", genbuf, 40,
				DOECHO, YEA);
			sprintf(content,
				"Äú±»½ûÖ¹Ê¹ÓÃÇ©Ãûµµ£¬Ô­ÒòÊÇ£º\n    %s\n\n"
				"(Èç¹ûÊÇÒòÎªÇ©ÃûµµÍ¼Æ¬´óĞ¡³¬£¬Çë²ÎÔÄ Announce "
				"°æµÄ¹«¸æ<<¹ØÓÚÍ¼Æ¬Ç©ÃûµµµÄ´óĞ¡ÏŞÖÆ>>\n"
				"http://ytht.net/Ytht.Net/bbscon?B=Announce&F=M.1047666649.A )",
				genbuf);
			mail_buf(content, lookupuser.userid,
				 "Äú±»½ûÖ¹Ê¹ÓÃÇ©Ãûµµ");
		}
		break;
	case 'g':
	case 'G':
		lookupuser.userlevel &= ~PERM_DENYSIG;
		break;
	case '8':
		lookupuser.userlevel &= ~PERM_SELFLOCK;
		{
			getdata(13, 0, "½ûÖ¹Ê¹ÓÃ×Ô¶¨ÒåËøÆÁµÄÔ­Òò£º", genbuf, 40,
				DOECHO, YEA);
			sprintf(content,
				"Äú±»½ûÖ¹Ê¹ÓÃ×Ô¶¨ÒåËøÆÁ£¬Ô­ÒòÈçÏÂ:\n\n   %s\n",
				genbuf);
			mail_buf(content, lookupuser.userid,
				 "Äú±»½ûÖ¹Ê¹ÓÃ×Ô¶¨ÒåËøÆÁ");
		}
		break;
    case 'h':
	case 'H':
		lookupuser.userlevel |= PERM_SELFLOCK;
		{
		       sprintf(content,"ÄúµÄ×Ô¶¨ÒåËøÆÁ·â½ûÒÑ¾­±»½â³ı£¬ÖØĞÂµÇÂ¼ºó¿ÉÒÔ¼ÌĞøÊ¹ÓÃ");
		       mail_buf(content,lookupuser.userid,"Äú¿ÉÒÔÊ¹ÓÃ×Ô¶¨ÒåËøÆÁ");
		}
		break;
	default:
		prints("\n Ê¹ÓÃÕß '%s' »ù±¾È¨ÀûÃ»ÓĞ±ä¸ü  %d\n", lookupuser.userid);
		pressreturn();
		clear();
		return 0;
	}
	{
		char secu[STRLEN];
		sprintf(secu, "ĞŞ¸Ä %s µÄ»ù±¾È¨ÏŞ", lookupuser.userid);
		permtostr(oldlevel, genbuf);
		sprintf(content, "ĞŞ¸ÄÇ°µÄÈ¨ÏŞ£º%s\nĞŞ¸ÄºóµÄÈ¨ÏŞ£º", genbuf);
		permtostr(lookupuser.userlevel, genbuf);
		strcat(content, genbuf);
		securityreport(secu, content);
	}

	substitute_record(PASSFILE, &lookupuser, sizeof (struct userec), id);
	clear();
	return 0;
}

int
s_checkid()
{
	char buf[256];
	char checkuser[20];
	int day, id;
	modify_user_mode(GMENU);
	clear();
	stand_title("µ÷²éID·¢ÎÄÇé¿ö\n");
	clrtoeol();
	move(2, 0);
	usercomplete("ÊäÈëÓûµ÷²éµÄÊ¹ÓÃÕßÕÊºÅ: ", genbuf);
	if (genbuf[0] == '\0') {
		clear();
		return 0;
	}
	strcpy(checkuser, genbuf);
	if (!(id = getuser(genbuf))) {
		move(4, 0);
		prints("ÎŞĞ§µÄÊ¹ÓÃÕßÕÊºÅ");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	getdata(5, 0, "ÊäÈëÌìÊı(0-ËùÓĞÊ±¼ä): ", buf, 7, DOECHO, YEA);
	day = atoi(buf);
	sprintf(buf,
		"/usr/bin/nice " MY_BBS_HOME "/bin/finddf %d %d %s > " MY_BBS_HOME
		"/bbstmpfs/tmp/checkid.%s 2>/dev/null", currentuser.userlevel,
		day, checkuser, currentuser.userid);
	if ((HAS_PERM(PERM_SYSOP) && heavyload(2.5))
	    || (!HAS_PERM(PERM_SYSOP) && heavyload(1.5))) {
		prints("ÏµÍ³¸ºÔØ¹ıÖØ, ÎŞ·¨Ö´ĞĞ±¾Ö¸Áî");
		pressreturn();
		return 1;
	}
	system(buf);
	sprintf(buf, "%s finddf %s %d", currentuser.userid, checkuser, day);
	newtrace(buf);
	sprintf(buf, MY_BBS_HOME "/bbstmpfs/tmp/checkid.%s",
		currentuser.userid);
	mail_file(buf, currentuser.userid, "\"System Report\"");
	prints("Íê±Ï");
	clrtoeol();
	pressreturn();
	clear();
	return 1;
}

char *
directfile(char *fpath, char *direct, char *filename)
{
	char *t;
	strcpy(fpath, direct);
	if ((t = strrchr(fpath, '/')) == NULL)
		exit(0);
	t++;
	strcpy(t, filename);
	return fpath;
}

static void
escape_filename(char *fn)
{
	static const char invalid[] =
	    { '/', '\\', '!', '&', '|', '*', '?', '`', '\'', '\"', ';', '<',
		'>', ':', '~', '(', ')', '[', ']', '{', '}', '$', '\n', '\r'
	};
	size_t i, j;

	for (i = 0; i < strlen(fn); i++)
		for (j = 0; j < sizeof (invalid); j++)
			if (fn[i] == invalid[j])
				fn[i] = '_';
}

int
zsend_file(char *from, char *title)
{
	char name[200], name1[200];
	char path[200];
	FILE *fr, *fw;
	char to[200];
	char buf[512], *fn = NULL;
	char attachfile[200];
	char attach_to_send[200];
	int  isa, base64;
	size_t len;

	ansimore("etc/zmodem", 0);
	move(14, 0);
	len = file_size(from);

	prints
	    ("´Ë´Î´«Êä¹² %d bytes, ´óÔ¼ºÄÊ± %d Ãë£¨ÒÔ 5k/s ¼ÆËã£©", len,
	     len / ZMODEM_RATE);
	move(t_lines - 1, 0);
	clrtoeol();
	strcpy(name, "N");

	getdata(t_lines - 1, 0,
		"ÄúÈ·¶¨ÒªÊ¹ÓÃZmodem´«ÊäÎÄ¼şÃ´?[y/N]", name, 2, DOECHO, YEA);
	if (toupper(name[0]) != 'Y')
		return FULLUPDATE;
	strncpy(name, title, 76);
	name[80] = '\0';
	escape_filename(name);
	move(t_lines - 2, 0);
	clrtoeol();
	prints("ÇëÊäÈëÎÄ¼şÃû£¬Îª¿ÕÔò·ÅÆú");
	move(t_lines - 1, 0);
	clrtoeol();
	getdata(t_lines - 1, 0, "", name, 78, DOECHO, 0);
	if (name[0] == '\0')
		return FULLUPDATE;
	name[78] = '\0';
	escape_filename(name);
	sprintf(name1, "YTHT-%s-", currboard);
	strcat(name1, name);
	strcpy(name, name1);
	strcat(name1, ".TXT");
	snprintf(path, sizeof (path), PATHZMODEM "/%s.%d", currentuser.userid,
		 uinfo.pid);
	mkdir(path, 0770);
	sprintf(to, "%s/%s", path, name1);
	fr = fopen(from, "r");
	if (fr == NULL)
		return FULLUPDATE;
	fw = fopen(to, "w");
	if (fw == NULL) {
		fclose(fr);
		return FULLUPDATE;
	}
	while (fgets(buf, sizeof (buf), fr) != NULL) {
		base64 = isa = 0;
		if (!strncmp(buf, "begin 644", 10)) {
			isa = 1;
			base64 = 1;
			fn = buf + 10;
		} else if (checkbinaryattach(buf, fr, &len)) {
			isa = 1;
			base64 = 0;
			fn = buf + 18;
		}
		if (isa) {
			sprintf(attachfile, "%s-attach-%s",  name, fn);
			if (getattach(fr, buf, attachfile, path, base64, len, 0)) {
				fprintf(fw, "¸½¼ş%s´íÎó\n", fn);
			} else {
				sprintf(attach_to_send, "%s/%s", path, attachfile);
				bbs_zsendfile(attach_to_send);
				fprintf(fw, "¸½¼ş%s\n", fn);
			}
		} else
			fputs(buf, fw);
	}
	fclose(fw);
	fclose(fr);
	bbs_zsendfile(to);
	return FULLUPDATE;
}

static void
bbs_zsendfile(char *filename)
{
	if (!dashf(filename))
		return;
	refresh();
	myexec_cmd(READING, NA, "bin/sz.sh", filename);
	unlink(filename);
}

static void
get_load(load)
double load[];
{
#if defined(LINUX)
	FILE *fp;
	fp = fopen("/proc/loadavg", "r");
	if (!fp)
		load[0] = load[1] = load[2] = 0;
	else {
		float av[3];
		fscanf(fp, "%g %g %g", av, av + 1, av + 2);
		fclose(fp);
		load[0] = av[0];
		load[1] = av[1];
		load[2] = av[2];
	}
#elif defined(BSD44)
	getloadavg(load, 3);
#else
	struct statstime rs;
	rstat("localhost", &rs);
	load[0] = rs.avenrun[0] / (double) (1 << 8);
	load[1] = rs.avenrun[1] / (double) (1 << 8);
	load[2] = rs.avenrun[2] / (double) (1 << 8);
#endif
}

void
inn_reload()
{
	char ans[4];

	getdata(t_lines - 1, 0, "ÖØ¶ÁÅäÖÃÂğ (Y/N)? [N]: ", ans, 2, DOECHO, YEA);
	if (ans[0] == 'Y' || ans[0] == 'y') {
		myexec_cmd(ADMIN, NA, "innd/ctlinnbbsd", "reload");
	}
}
