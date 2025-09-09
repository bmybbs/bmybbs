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
#include <sys/mman.h>
#include "bbs.h"
#include "bbs_global_vars.h"
#include "smth_screen.h"
#include "maintain.h"
#include "bcache.h"
#include "stuff.h"
#include "io.h"
#include "edit.h"
#include "sendmsg.h"
#include "talk.h"
#include "namecomplete.h"
#include "mail.h"
#include "more.h"
#include "bbsinc.h"
#include "convcode.h"
#include "userinfo.h"
#include "main.h"
#include "bbs-internal.h"

#ifdef SSHBBS
extern int ssh_write(int fd, const void *buf, size_t count);
extern int ssh_read(int fd, void *buf, size_t count);
#endif

pid_t childpid;
static int loadkeys(struct one_key *key, char *name);
static int savekeys(struct one_key *key, char *name);
static void keyprint(char *buf, int key);
static int showkeyinfo(struct one_key *akey, int i);
static unsigned int setkeys(struct one_key *key);
static void myexec_cmd(int umode, int pager, const char *cmdfile, const char *param);
static void datapipefd(int fds, int fdn);
static int sendGoodWish(char *userid);
static void childreturn(int i);
static void escape_filename(char *fn);
static void bbs_zsendfile(char *filename);
static void get_load(double load[]);

static const char * const g_permstrings[] = {
	"\xBB\xF9\xB1\xBE\xC8\xA8\xC1\xA6",									/* PERM_BASIC     ����Ȩ�� */
	"\xBD\xF8\xC8\xEB\xC1\xC4\xCC\xEC\xCA\xD2",							/* PERM_CHAT      ���������� */
	"\xBA\xF4\xBD\xD0\xCB\xFB\xC8\xCB\xC1\xC4\xCC\xEC",					/* PERM_PAGE      ������������ */
	"\xB7\xA2\xB1\xED\xCE\xC4\xD5\xC2",									/* PERM_POST      �������� */
	"\xCA\xB9\xD3\xC3\xD5\xDF\xD7\xCA\xC1\xCF\xD5\xFD\xC8\xB7",			/* PERM_LOGINOK   ʹ����������ȷ */
	"\xBD\xFB\xD6\xB9\xCA\xB9\xD3\xC3\xC7\xA9\xC3\xFB\xB5\xB5",			/* PERM_DENYSIG   ��ֹʹ��ǩ���� */
	"\xD2\xFE\xC9\xED\xCA\xF5",											/* PERM_CLOAK     ������ */
	"\xBF\xB4\xB4\xA9\xD2\xFE\xC9\xED\xCA\xF5",							/* PERM_SEECLOAK  ���������� */
	"\xD5\xCA\xBA\xC5\xD3\xC0\xBE\xC3\xB1\xA3\xC1\xF4",					/* PERM_XEMPT     �ʺ����ñ��� */
	"\xB1\xE0\xBC\xAD\xBD\xF8\xD5\xBE\xBB\xAD\xC3\xE6",					/* PERM_WELCOME   �༭��վ���� */
	"\xB0\xE5\xD6\xF7",													/* PERM_BOARDS    ���� */
	"\xD5\xCA\xBA\xC5\xB9\xDC\xC0\xED\xD4\xB1",							/* PERM_ACCOUNTS  �ʺŹ���Ա */
	"\xB1\xBE\xD5\xBE\xD6\xD9\xB2\xC3",									/* PERM_ARBITRATE ��վ�ٲ� */
	"\xCD\xB6\xC6\xB1\xB9\xDC\xC0\xED\xD4\xB1",							/* PERM_OVOTE     ͶƱ����Ա */
	"\xCF\xB5\xCD\xB3\xCE\xAC\xBB\xA4\xB9\xDC\xC0\xED\xD4\xB1",			/* PERM_SYSOP     ϵͳά������Ա */
	"Read/Post \xCF\xDE\xD6\xC6",										/* PERM_POSTMASK  Read/Post ���� */
	"\xBE\xAB\xBB\xAA\xC7\xF8\xD7\xDC\xB9\xDC",							/* PERM_ANNOUNCE  �������ܹ� */
	"\xCC\xD6\xC2\xDB\xC7\xF8\xD7\xDC\xB9\xDC",							/* PERM_OBOARDS   �������ܹ� */
	"\xBB\xEE\xB6\xAF\xBF\xB4\xB0\xE6\xD7\xDC\xB9\xDC",					/* PERM_ACBOARD   ������ܹ� */
	"\xB2\xBB\xC4\xDC ZAP(\xCC\xD6\xC2\xDB\xC7\xF8\xD7\xA8\xD3\xC3)",	/* PERM_NOZAP     ����ZAP(������ר��) */
	"\xC7\xBF\xD6\xC6\xBA\xF4\xBD\xD0",									/* PERM_FORCEPAGE ǿ�ƺ��� */
	"\xD1\xD3\xB3\xA4\xB7\xA2\xB4\xF4\xCA\xB1\xBC\xE4",					/* PERM_EXT_IDLE  �ӳ�����ʱ�� */
	"\xB4\xF3\xD0\xC5\xCF\xE4",											/* PERM_SPECIAL1  ������ */
	"\xCC\xD8\xCA\xE2\xC8\xA8\xCF\xDE 2",								/* PERM_SPECIAL2  ����Ȩ�� 2 */
	"\xCC\xD8\xCA\xE2\xC8\xA8\xCF\xDE 3",								/* PERM_SPECIAL3  ����Ȩ�� 3 */
	"\xC7\xF8\xB3\xA4",													/* PERM_SPECIAL4  ���� */
	"\xB1\xBE\xD5\xBE\xBC\xE0\xB2\xEC\xD7\xE9",							/* PERM_SPECIAL5  ��վ����� */
	"\xB1\xBE\xD5\xBE\xC1\xA2\xB7\xA8\xBB\xE1",							/* PERM_SPECIAL6  ��վ������ */
	"\xCC\xD8\xCA\xE2\xC8\xA8\xCF\xDE 7",								/* PERM_SPECIAL7  ����Ȩ�� 7 */
	"\xB8\xF6\xC8\xCB\xCE\xC4\xBC\xAF",									/* PERM_SPECIAL8  �����ļ� */
	"\xBD\xFB\xD6\xB9\xB7\xA2\xD0\xC5\xC8\xA8",							/* PERM_DENYMAIL  ��ֹ����Ȩ */
};

static const char *const g_user_definestr[NUMDEFINES] = {
	/* DEF_FRIENDCALL         �������ر�ʱ���ú��Ѻ��� */
	"\xBA\xF4\xBD\xD0\xC6\xF7\xB9\xD8\xB1\xD5\xCA\xB1\xBF\xC9\xC8\xC3\xBA\xC3\xD3\xD1\xBA\xF4\xBD\xD0",
	/* DEF_ALLMSG             ���������˵�ѶϢ */
	"\xBD\xD3\xCA\xDC\xCB\xF9\xD3\xD0\xC8\xCB\xB5\xC4\xD1\xB6\xCF\xA2",
	/* DEF_FRIENDMSG          ���ܺ��ѵ�ѶϢ */
	"\xBD\xD3\xCA\xDC\xBA\xC3\xD3\xD1\xB5\xC4\xD1\xB6\xCF\xA2",
	/* DEF_SOUNDMSG           �յ�ѶϢ�������� */
	"\xCA\xD5\xB5\xBD\xD1\xB6\xCF\xA2\xB7\xA2\xB3\xF6\xC9\xF9\xD2\xF4",
	/* DEF_COLOR              ʹ�ò�ɫ */
	"\xCA\xB9\xD3\xC3\xB2\xCA\xC9\xAB",
	/* DEF_ACBOARD            ��ʾ����� */
	"\xCF\xD4\xCA\xBE\xBB\xEE\xB6\xAF\xBF\xB4\xB0\xE6",
	/* DEF_ENDLINE            ��ʾѡ����ѶϢ�� */
	"\xCF\xD4\xCA\xBE\xD1\xA1\xB5\xA5\xB5\xC4\xD1\xB6\xCF\xA2\xC0\xB8",
	/* DEF_EDITMSG            �༭ʱ��ʾ״̬�� */
	"\xB1\xE0\xBC\xAD\xCA\xB1\xCF\xD4\xCA\xBE\xD7\xB4\xCC\xAC\xC0\xB8",
	/* DEF_NOTMSGFRIEND       ѶϢ������һ��/����ģʽ */
	"\xD1\xB6\xCF\xA2\xC0\xB8\xB2\xC9\xD3\xC3\xD2\xBB\xB0\xE3/\xBE\xAB\xBC\xF2\xC4\xA3\xCA\xBD",
	/* DEF_NORMALSCR          ѡ������һ��/����ģʽ */
	"\xD1\xA1\xB5\xA5\xB2\xC9\xD3\xC3\xD2\xBB\xB0\xE3/\xBE\xAB\xBC\xF2\xC4\xA3\xCA\xBD",
	/* DEF_NEWPOST            ������������ New ��ʾ */
	"\xB7\xD6\xC0\xE0\xCC\xD6\xC2\xDB\xC7\xF8\xD2\xD4 New \xCF\xD4\xCA\xBE",
	/* DEF_CIRCLE             �Ķ������Ƿ�ʹ���ƾ�ѡ�� */
	"\xD4\xC4\xB6\xC1\xCE\xC4\xD5\xC2\xCA\xC7\xB7\xF1\xCA\xB9\xD3\xC3\xC8\xC6\xBE\xED\xD1\xA1\xD4\xF1",
	/* DEF_FIRSTNEW           �Ķ������α�ͣ�ڵ�һƪδ�� */
	"\xD4\xC4\xB6\xC1\xCE\xC4\xD5\xC2\xD3\xCE\xB1\xEA\xCD\xA3\xD3\xDA\xB5\xDA\xD2\xBB\xC6\xAA\xCE\xB4\xB6\xC1",
	/* DEF_LOGFRIEND          ��վʱ��ʾ�������� */
	"\xBD\xF8\xD5\xBE\xCA\xB1\xCF\xD4\xCA\xBE\xBA\xC3\xD3\xD1\xC3\xFB\xB5\xA5",
	/* DEF_INNOTE             ��վʱ��ʾ����¼ */
	"\xBD\xF8\xD5\xBE\xCA\xB1\xCF\xD4\xCA\xBE\xB1\xB8\xCD\xFC\xC2\xBC",
	/* DEF_OUTNOTE            ��վʱ��ʾ����¼ */
	"\xC0\xEB\xD5\xBE\xCA\xB1\xCF\xD4\xCA\xBE\xB1\xB8\xCD\xFC\xC2\xBC",
	/* DEF_MAILMSG            ��վʱѯ�ʼĻ�����ѶϢ */
	"\xC0\xEB\xD5\xBE\xCA\xB1\xD1\xAF\xCE\xCA\xBC\xC4\xBB\xD8\xCB\xF9\xD3\xD0\xD1\xB6\xCF\xA2",
	/* DEF_LOGOUT             ʹ���Լ�����վ���� */
	"\xCA\xB9\xD3\xC3\xD7\xD4\xBC\xBA\xB5\xC4\xC0\xEB\xD5\xBE\xBB\xAD\xC3\xE6",
	/* DEF_SEEWELC1           ���������֯�ĳ�Ա */
	"\xCE\xD2\xCA\xC7\xD5\xE2\xB8\xF6\xD7\xE9\xD6\xAF\xB5\xC4\xB3\xC9\xD4\xB1",
	/* DEF_LOGINFROM          ������վ֪ͨ */
	"\xBA\xC3\xD3\xD1\xC9\xCF\xD5\xBE\xCD\xA8\xD6\xAA",
	/* DEF_NOTEPAD            �ۿ����԰� */
	"\xB9\xDB\xBF\xB4\xC1\xF4\xD1\xD4\xB0\xE6",
	/* DEF_NOLOGINSEND        ��Ҫ�ͳ���վ֪ͨ������ */
	"\xB2\xBB\xD2\xAA\xCB\xCD\xB3\xF6\xC9\xCF\xD5\xBE\xCD\xA8\xD6\xAA\xB8\xF8\xBA\xC3\xD3\xD1",
	/* DEF_THESIS             ����ʽ���� */
	"\xD6\xF7\xCC\xE2\xCA\xBD\xBF\xB4\xB0\xE6",
	/* DEF_MSGGETKEY          �յ�ѶϢ�Ⱥ��Ӧ����� */
	"\xCA\xD5\xB5\xBD\xD1\xB6\xCF\xA2\xB5\xC8\xBA\xF2\xBB\xD8\xD3\xA6\xBB\xF2\xC7\xE5\xB3\xFD",
	/* DEF_DELDBLCHAR         �������ִ��� */
	"\xBA\xBA\xD7\xD6\xD5\xFB\xD7\xD6\xB4\xA6\xC0\xED",
	/* DEF_USEGB KCN 99.09.03 ʹ��GB���Ķ� */
	"\xCA\xB9\xD3\xC3\x47\x42\xC2\xEB\xD4\xC4\xB6\xC1",
	/* DEF_ANIENDLINE         ʹ�ö�̬���� */
	"\xCA\xB9\xD3\xC3\xB6\xAF\xCC\xAC\xB5\xD7\xCF\xDF",
	/* DEF_INTOANN            ���η��ʰ�����ʾ���뾫���� */
	"\xB3\xF5\xB4\xCE\xB7\xC3\xCE\xCA\xB0\xE6\xC3\xE6\xCC\xE1\xCA\xBE\xBD\xF8\xC8\xEB\xBE\xAB\xBB\xAA\xC7\xF8",
	/* DEF_POSTNOMSG          ��������ʱ��ʱ����MSG */
	"\xB7\xA2\xB1\xED\xCE\xC4\xD5\xC2\xCA\xB1\xD4\xDD\xCA\xB1\xC6\xC1\xB1\xCEMSG",
	/* DEF_SEESTATINLOG       ��վʱ�ۿ�ͳ����Ϣ */
	"\xBD\xF8\xD5\xBE\xCA\xB1\xB9\xDB\xBF\xB4\xCD\xB3\xBC\xC6\xD0\xC5\xCF\xA2",
	/* DEF_FILTERXXX          ���˿������˷�����Ϣ */
	"\xB9\xFD\xC2\xCB\xBF\xC9\xC4\xDC\xC1\xEE\xC8\xCB\xB7\xB4\xB8\xD0\xD0\xC5\xCF\xA2",
	/* DEF_INTERNETMAIL       ��ȡվ���ż� */
	//"\xCA\xD5\xC8\xA1\xD5\xBE\xCD\xE2\xD0\xC5\xBC\xFE",
	/* DEF_NEWSTOP10          ��վʱ�ۿ�ȫ��ʮ�����а� */
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
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "readkey");
	loadkeys(read_comms, tempname);
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "mailkey");
	loadkeys(mail_comms, tempname);
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "friendkey");
	loadkeys(friend_list, tempname);
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "rejectkey");
	loadkeys(reject_list, tempname);
}

int modify_user_mode(int mode) {
	if (uinfo.mode == mode)
		return 0;
	uinfo.mode = mode;
	update_ulist(&uinfo, utmpent);
	return 0;
}

int
showperminfo(unsigned int pbits, int i, int use_define)
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
	prints("�밴����Ҫ�Ĵ������趨���̣��� Enter ����.\n");
	i = 0;
	while (key[i].fptr != NULL && i < 40) {
		showkeyinfo(key, i);
		i++;
	}
	while (!done) {
		getdata(t_lines - 1, 0, "ѡ��(ENTER ����): ", choice, 2, DOECHO,
			YEA);
		*choice = toupper(*choice);
		if (*choice == '\n' || *choice == '\0')
			done = YEA;
		else if (*choice < '0' || *choice > '0' + i - 1)
			bell();
		else {
			j = *choice - '0';
			move(t_lines - 1, 0);
			prints("�붨��[\033[35m%s\033[m]�Ĺ��ܼ�:", key[j].func);
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

int x_copykeys(const char *s) {
	(void) s;
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	char ans[3];
	clear();
	move(0, 0);
	clrtoeol();

	prints("��ѡ����Ҫ�ָ���Ĭ�ϼ�λ,�� Enter ����.\n");
	getdata(1, 0, "[0]�뿪 [1]����1 [2]����2 [3]���� [4]���� [5]���� [6]ȫ�� (Ĭ��[0]):", ans, 2, DOECHO, YEA);

	if (ans[0] == '0' || ans[0] == '\n' || ans[0] == '\0')
			return 0;

	if (ans[0] == '3' || ans[0] == '6')
	{
		int i = 0;
		while (mail_default_comms[i].fptr != NULL) {
			mail_comms[i].key= mail_default_comms[i].key;
			i++;
		}
		sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "mailkey");
		savekeys(&mail_comms[0], tempname);

	}

	prints("�������");
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
	prints("�밴����Ҫ�Ĵ������趨��������,�� Enter ����.\n");
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
	getdata(t_lines - 1, 0, "ѡ��(ENTER ����): ", choice, 2, DOECHO, YEA);
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
				"�Ƿ�ѵ�ǰ���Լ��ļ��̶�������Ϊϵͳ����(Y/N):N ",
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
		prints("�������");
	}
	pressreturn();
	return 0;
}
*/

int x_setkeys(const char *s) {
	(void) s;
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	clear();
	move(0, 0);
	clrtoeol();
	setkeys(&read_comms[0]);
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "readkey");
	savekeys(&read_comms[0], tempname);
	return 0;
}

int x_setkeys2(const char *s) {
	(void) s;
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	clear();
	move(0, 0);
	clrtoeol();
	setkeys(&read_comms[40]);
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "readkey");
	savekeys(&read_comms[0], tempname);
	return 0;
}

int x_setkeys3(const char *s) {
	(void) s;
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	clear();
	move(0, 0);
	clrtoeol();
	setkeys(&mail_comms[0]);
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "mailkey");
	savekeys(&mail_comms[0], tempname);
	return 0;
}

int x_setkeys4(const char *s) {
	(void) s;
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	clear();
	move(0, 0);
	clrtoeol();
	setkeys(&friend_list[0]);
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "friendkey");
	savekeys(&friend_list[0], tempname);
	return 0;
}

int x_setkeys5(const char *s) {
	(void) s;
	char tempname[STRLEN];
	modify_user_mode(USERDEF);
	clear();
	move(0, 0);
	clrtoeol();
	setkeys(&reject_list[0]);
	sethomefile_s(tempname, sizeof(tempname), currentuser.userid, "rejectdkey");
	savekeys(&reject_list[0], tempname);
	return 0;
}

unsigned int
setperms(unsigned int pbits, char *prompt, int numbers, int (*showfunc) (unsigned int, int, int), int param)
{
	int lastperm = numbers - 1;
	int i, done = NA;
	char choice[3];

	move(4, 0);
	prints("�밴����Ҫ�Ĵ������趨%s���� Enter ����.\n", prompt);
	move(6, 0);
	clrtobot();
/*    pbits &= (1 << numbers) - 1;*/
	for (i = 0; i <= lastperm; i++) {
		(*showfunc) (pbits, i, param);
	}
	while (!done) {
		getdata(t_lines - 1, 0, "ѡ��(ENTER ����): ", choice, 2, DOECHO,
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

int x_level(const char *s) {
	(void) s;
	int id, oldlevel;
	unsigned int newlevel;
	char content[2048];

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
	newlevel = setperms(lookupuser.userlevel, "Ȩ��", NUMPERMS, showperminfo, 0);
	move(2, 0);
	if (newlevel == lookupuser.userlevel)
		prints("User '%s' level NOT changed\n", lookupuser.userid);
	else {
		oldlevel = lookupuser.userlevel;
		lookupuser.userlevel = newlevel;
		{
			char secu[STRLEN];
			sprintf(secu, "�޸� %s ��Ȩ��", lookupuser.userid);
			permtostr(oldlevel, genbuf);
			sprintf(content, "�޸�ǰ��Ȩ�ޣ�%s\n�޸ĺ��Ȩ�ޣ�",
				genbuf);
			permtostr(lookupuser.userlevel, genbuf);
			strcat(content, genbuf);
			securityreport(secu, content);
		}
		substitute_record(PASSFILE, &lookupuser, sizeof (struct userec), id);
		prints("User '%s' level changed\n", lookupuser.userid);
	}
	pressreturn();
	clear();
	return 0;
}

int x_userdefine(const char *s) {
	(void) s;
	int id;
	unsigned int newlevel;
	extern int nettyNN;

	modify_user_mode(USERDEF);
	if (!(id = getuser(currentuser.userid))) {
		move(3, 0);
		prints("�����ʹ���� ID...");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	move(1, 0);
	clrtobot();
	move(2, 0);
	newlevel = setperms(lookupuser.userdefine, "����", NUMDEFINES, showperminfo, 1);
	move(2, 0);
	if (newlevel == lookupuser.userdefine)
		prints("����û���޸�...\n");
	else {
		lookupuser.userdefine = newlevel;
		currentuser.userdefine = newlevel;
		if ((!g_convcode && !(newlevel & DEF_USEGB)) || (g_convcode && (newlevel & DEF_USEGB)))
			switch_code();
		substitute_record(PASSFILE, &lookupuser, sizeof (struct userec), id);
		uinfo.pager |= FRIEND_PAGER;
		if (!(uinfo.pager & ALL_PAGER)) {
			if (!DEFINE(DEF_FRIENDCALL, currentuser))
				uinfo.pager &= ~FRIEND_PAGER;
		}
		uinfo.pager &= ~ALLMSG_PAGER;
		uinfo.pager &= ~FRIENDMSG_PAGER;
		if (DEFINE(DEF_DELDBLCHAR, currentuser))
			enabledbchar = 1;
		else
			enabledbchar = 0;
		if (DEFINE(DEF_FRIENDMSG, currentuser)) {
			uinfo.pager |= FRIENDMSG_PAGER;
		}
		if (DEFINE(DEF_ALLMSG, currentuser)) {
			uinfo.pager |= ALLMSG_PAGER;
			uinfo.pager |= FRIENDMSG_PAGER;
		}
		update_utmp();
		if (DEFINE(DEF_ACBOARD, currentuser))
			nettyNN = NNread_init();
		prints("�µĲ����趨���...\n\n");
	}
	iscolor = (DEFINE(DEF_COLOR, currentuser)) ? 1 : 0;
	pressreturn();
	clear();
	return 0;
}

int x_cloak(const char *s) {
	(void) s;
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
		else
			currentuser./*pseudo_*/lastlogout = 0;
		substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
		prints("������ (cloak) �Ѿ�%s��!", (uinfo.invisible) ? "����" : "ֹͣ");
		pressreturn();
	}
	return 0;
}

int x_edits(const char *s) {
	(void) s;
	int aborted;
	char ans[7], buf[STRLEN];
	int ch, num, confirm;
	extern int WishNum;
	static char *const e_file[] = {
		"plans", "signatures", "notes", "logout", "GoodWish", "bansite",
		NULL
	};
	static char *const explain_file[] = {
		"����˵����", "ǩ����", "�Լ��ı���¼", "��վ�Ļ���",
		"�ײ�������Ϣ", "��ֹ��ID��¼��IP(ÿ��IPռһ��)",
		NULL
	};

	modify_user_mode(GMENU);
	clear();
	move(1, 0);
	prints("���޸��˵���\n\n");
	for (num = 0; e_file[num] != NULL && explain_file[num] != NULL; num++) {
		prints("[\033[1;32m%d\033[m] %s\n", num + 1, explain_file[num]);
	}
	prints("[\033[1;32m%d\033[m] �������\n", num + 1);

	getdata(num + 5, 0, "��Ҫ������һ����˵���: ", ans, 2, DOECHO, YEA);
	if (ans[0] - '0' <= 0 || ans[0] - '0' > num || ans[0] == '\n' || ans[0] == '\0')
		return 0;

	ch = ans[0] - '0' - 1;
	sethomefile_s(genbuf, sizeof(genbuf), currentuser.userid, e_file[ch]);
	move(3, 0);
	clrtobot();
	sprintf(buf, "(E)�༭ (D)ɾ�� %s? [E]: ", explain_file[ch]);
	getdata(3, 0, buf, ans, 2, DOECHO, YEA);
	if (ans[0] == 'D' || ans[0] == 'd') {
		confirm = askyn("��ȷ��Ҫɾ���������", NA, NA);
		if (confirm != 1) {
			move(5, 0);
			prints("ȡ��ɾ���ж�\n");
			pressreturn();
			clear();
			return 0;
		}
		unlink(genbuf);
		move(5, 0);
		prints("%s ��ɾ��\n", explain_file[ch]);
		pressreturn();
		if (ch == 4)
			WishNum = 9999;
		clear();
		return 0;
	}
	modify_user_mode(EDITUFILE);
	aborted = vedit(genbuf, NA, YEA);
	clear();
	if (!aborted) {
		prints("%s ���¹�\n", explain_file[ch]);
		sprintf(buf, "edit %s", explain_file[ch]);
		if (!strcmp(e_file[ch], "signatures")) {
			set_numofsig();
			prints("ϵͳ�����趨�Լ��������ǩ����...");
		}
	} else
		prints("%s ȡ���޸�\n", explain_file[ch]);
	pressreturn();
	if (ch == 4)
		WishNum = 9999;
	return 0;
}

int a_edits(const char *s) {
	(void) s;
	int aborted;
	char ans[7], buf[STRLEN], buf2[STRLEN];
	int ch, num, confirm;
	/*modified by pzhg 070412 for voteing for valid ids in spacific boards*/
	static char *const e_file[] = {
		"../Welcome", "../Welcome2", "issue", "logout", "movie",
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
	static char *const explain_file[] = {
		"�����վ������", "��վ����", "��վ��ӭ��", "��վ����",
		"�����", "��Ļ����", "���ñ���¼", "menu.ini",
		"����ע��� ID", "ID �в��ܰ������ִ�", "����ȷ��֮E-Mail",
		"������վ֮λַ",
		"����E-mail������", "ÿ���Զ����ŵ�", "����POST���İ�",
		"����������", "������Ա����", "������IP�б�",
		"������������", "��������", "������Ҫ���˵ı���",
		"FTP����Ա����", "���˴ʻ�", "������˴ʻ�", "�����ʻ�",
		"ת�Ű���������Ӧ", "����IP���ƴ���", "����վ��", "id��ʶ",
		"�������趨", "�Ƽ�����", "�����Ŷ�����",
		"����ע��mail.xjtu.edu.cn","����ע��stu.xjtu.edu.cn",
		"��������10�����","����ͶƱȨ����","�¿�����","�Ƽ�����","��ֹguest���ĵ�ip",NULL
	};//modify by wjbta@bmy  ����id��ʶ����ʾ, modify by mintbaggio 040406 for front page commend

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return 0;
	}
	clear();
	move(0, 0);
	prints("����ϵͳ����\n\n");
	for (num = 0;
			HAS_PERM(PERM_SYSOP, currentuser) ? e_file[num] != NULL
			&& explain_file[num] != NULL : strcasecmp(explain_file[num], "menu.ini") != 0;
			num++) {
		if (num >= 20)
			move(num - 20 + 2, 39);
		prints("[%2d] %s\n", num + 1, explain_file[num]);
	}
	if (num >= 20)
		move(num - 20 + 2, 39);
	prints("[%2d] �������\n", num + 1);

	getdata(t_lines - 1, 0, "��Ҫ������һ��ϵͳ����: ", ans, 3, DOECHO,
		YEA);
	ch = atoi(ans);
	if (!isdigit(ans[0]) || ch <= 0 || ch > num || ans[0] == '\n' || ans[0] == '\0')
		return 0;
	ch -= 1;
	sprintf(buf2, "etc/%s", e_file[ch]);
	move(3, 0);
	clrtobot();
	sprintf(buf, "(E)�༭ (D)ɾ�� %s? [E]: ", explain_file[ch]);
	getdata(3, 0, buf, ans, 2, DOECHO, YEA);
	if (ans[0] == 'D' || ans[0] == 'd') {
		confirm = askyn("��ȷ��Ҫɾ�����ϵͳ��", NA, NA);
		if (confirm != 1) {
			move(5, 0);
			prints("ȡ��ɾ���ж�\n");
			pressreturn();
			clear();
			return 0;
		}
		{
			char secu[STRLEN];
			sprintf(secu, "ɾ��ϵͳ������%s", explain_file[ch]);
			securityreport(secu, secu);
		}
		unlink(buf2);
		move(5, 0);
		prints("%s ��ɾ��\n", explain_file[ch]);
		pressreturn();
		clear();
		return 0;
	}
	modify_user_mode(EDITSFILE);
	aborted = vedit(buf2, NA, YEA);
	clear();
	if (aborted != -1) {
		prints("%s ���¹�", explain_file[ch]);
		{
			char secu[STRLEN];
			sprintf(secu, "�޸�ϵͳ������%s", explain_file[ch]);
			securityreport(secu, secu);
		}

		if (!strcmp(e_file[ch], "../Welcome")) {
			unlink("Welcome.rec");
			prints("\nWelcome ��¼������");
		}
	}
	pressreturn();
	return 0;
}

//added by pzhg for SysFiles2
int a_edits2(const char *s) {
	(void) s;
	int aborted;
	char ans[7], buf[STRLEN], buf2[STRLEN];
	int ch, num, confirm;
	static char *const e_file[] = {
		"birthday","sysboards","adpost","ad_banner","ad_left", "secorder",
		NULL
	};
	static char *const explain_file[] = {
		"���ջ�ӭ����","վ��������","�������","Banner", "Left Ads","WWW������˳��",
		NULL
	};

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return 0;
	}
	clear();
	move(0, 0);
	prints("����ϵͳ����2\n\n");
	for (num = 0;
			HAS_PERM(PERM_SYSOP, currentuser) ? e_file[num] != NULL
			&& explain_file[num] != NULL : strcasecmp(explain_file[num], "menu.ini") != 0;
			num++) {
		if (num >= 20)
			move(num - 20 + 2, 39);
		prints("[%2d] %s\n", num + 1, explain_file[num]);
	}
	if (num >= 20)
		move(num - 20 + 2, 39);
	prints("[%2d] �������\n", num + 1);

	getdata(t_lines - 1, 0, "��Ҫ������һ��ϵͳ����: ", ans, 3, DOECHO,
		YEA);
	ch = atoi(ans);
	if (!isdigit(ans[0]) || ch <= 0 || ch > num || ans[0] == '\n' || ans[0] == '\0')
		return 0;
	ch = ch % 6; // items of e_file
	sprintf(buf2, "etc/%s", e_file[ch]);
	move(3, 0);
	clrtobot();
	sprintf(buf, "(E)�༭ (D)ɾ�� %s? [E]: ", explain_file[ch]);
	getdata(3, 0, buf, ans, 2, DOECHO, YEA);
	if (ans[0] == 'D' || ans[0] == 'd') {
		confirm = askyn("��ȷ��Ҫɾ�����ϵͳ��", NA, NA);
		if (confirm != 1) {
			move(5, 0);
			prints("ȡ��ɾ���ж�\n");
			pressreturn();
			clear();
			return 0;
		}
		{
			char secu[STRLEN];
			sprintf(secu, "ɾ��ϵͳ������%s", explain_file[ch]);
			securityreport(secu, secu);
		}
		unlink(buf2);
		move(5, 0);
		prints("%s ��ɾ��\n", explain_file[ch]);
		pressreturn();
		clear();
		return 0;
	}
	modify_user_mode(EDITSFILE);
	aborted = vedit(buf2, NA, YEA);
	clear();
	if (aborted != -1) {
		prints("%s ���¹�", explain_file[ch]);
		{
			char secu[STRLEN];
			sprintf(secu, "�޸�ϵͳ������%s", explain_file[ch]);
			securityreport(secu, secu);
		}
	}
	pressreturn();
	return 0;
}

int x_lockscreen(const char *s) {
	(void) s;
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
	prints(
			"\n\033[1;37m       _       _____   ___     _   _   ___     ___       __"
			"\n      ( )     (  _  ) (  _`\\  ( ) ( ) (  _`\\  (  _`\\    |  |"
			"\n      | |     | ( ) | | ( (_) | |/'/' | (_(_) | | ) |   |  |"
			"\n      | |  _  | | | | | |  _  | , <   |  _)_  | | | )   |  |"
			"\n      | |_( ) | (_) | | (_( ) | |\\`\\  | (_( ) | |_) |   |==|"
			"\n      (____/' (_____) (____/' (_) (_) (____/' (____/'   |__|\033[m\n");
	move(17,0);
	getdata(17,0,"��������? (1)�Է�ȥ�� (2)��MM���� (3)�������� (4)û���� (5)�Զ���:",ubuf,3,DOECHO,YEA); //add by landefeng@BMY for ��������
	switch(ubuf[0]){
		case '1':
			modify_user_mode(USERDF1);break;
		case '2':
			modify_user_mode(USERDF2);break;
		case '3':
			modify_user_mode(USERDF3);break;
		case '5':						//add by leoncom@bmy �Զ�����������
			move(17,0);
			clrtobot();
			update_endline();
			if(HAS_PERM(PERM_SELFLOCK, currentuser)) {
				uinfo.user_state_temp[0]='\0';                  //����ϴμ�¼
				update_ulist(&uinfo,utmpent);
				getdata(17,0,"�������Զ�������:",user_self,9,DOECHO,YEA);
				int i=0,flag=0;
				for(i=0;i<=7;i++){
					if(user_self[i]==' ')
					{
						flag=1;
						break;
					}
				}
				if ((stringfilter(user_self,0) == YTHT_SMTH_FILTER_RESULT_SAFE) && !flag)
				{
					strcpy(uinfo.user_state_temp,user_self);
					update_ulist(&uinfo, utmpent);
					move(17,0);
					clrtobot();
					prints("���ĵ�ǰ��������Ϊ:%s",user_self);
					modify_user_mode(USERDF4);break;
				}
				else
				{
					move(17,0);
					clrtobot();
					prints("��������Զ������ɺ��в����ʴʻ�������ַ�������Ĭ�Ϸ�ʽ����");
					modify_user_mode(LOCKSCREEN);
					break;
				}
			} else {
				move(17,0);
				clrtobot();
				prints("�㱻����Աȡ���Զ���������Ȩ�ޣ�����Ĭ�Ϸ�ʽ����");
				modify_user_mode(LOCKSCREEN);
				break;
			}
		default:
			prints("Ĭ������");
			modify_user_mode(LOCKSCREEN);
	}
	move(18,0);
	clrtobot();
	prints("\033[1;36mӫĻ����\033[33m %19s\033[36m ʱ��\033[32m %-12s \033[36m��ʱ��ס��...\033[m", ctime(&now), currentuser.userid);
	while (*buf == '\0' || !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
		move(19, 0);
		clrtobot();
		update_endline();
		getdata(19, 0, buf[0] == '\0' ? "��������������Խ���: " :
			"���������������������������������Խ���: ", buf,
			PASSLEN, NOECHO, YEA);
	}
	unblock_msg();
	return 0;
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
myexec_cmd(int umode, int pager, const char *cmdfile, const char *param)
{
	char buf[STRLEN * 8], param1[256];
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

	if (!HAS_PERM(PERM_SYSOP, currentuser) && heavyload(0)) {
		clear();
		prints("��Ǹ��Ŀǰϵͳ���ɹ��أ��˹�����ʱ����ִ��...");
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
			execl(cmdfile, cmdfile, param1, currentuser.userid, uinfo.from, pidstr, NULL);
		} else {
			snprintf(buf, sizeof (buf), "%s exec %s %s %s %d",
				currentuser.userid, cmdfile,
				currentuser.userid, uinfo.from, getppid());
			newtrace(buf);
			execl(cmdfile, cmdfile, currentuser.userid, uinfo.from, pidstr, NULL);
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
				if ((unsigned long) now_t - (unsigned long) old > 60)
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

int sendgoodwish(const char *uid) {
	(void) uid;
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
	prints("\033[0;1;32m���Ա�\033[m\n�����������������������ȥ����ף����");
	prints("\nҲ����Ϊ������/������һ�����Ļ���");
	move(6, 0);

	if (userid == NULL) {
		getdata(3, 0,
			"(1)��������ȥף�� (2)��һȺ����ȥף�� (0) ȡ�� [1]",
			ans, 2, DOECHO, YEA);
		if (ans[0] == '0') {
			clear();
			return 0;
		}
		if (ans[0] != '2') {
			usercomplete("���������� ID: ", uid);
			if (uid[0] == '\0') {
				clear();
				return 0;
			}
		} else {
			clear();
			sethomefile_s(wishlists, sizeof(wishlists), currentuser.userid, "wishlist");
			cnt = listfilecontent(wishlists);
			while (1) {
				getdata(0, 0,
					"(A)���� (D)ɾ�� (I)������� (C)���Ŀǰ���� (E)���� (S)�ͳ�?[S]�� ",
					tmp, 2, DOECHO, YEA);
				if (tmp[0] == '\n' || tmp[0] == '\0' || tmp[0] == 's' || tmp[0] == 'S') {
					clear();
					break;
				}
				if (tmp[0] == 'a' || tmp[0] == 'd' || tmp[0] == 'A' || tmp[0] == 'D') {
					move(1, 0);
					if (tmp[0] == 'a' || tmp[0] == 'A')
						usercomplete("����������ʹ���ߴ���(ֻ�� ENTER ��������): ", uident);
					else
						namecomplete("����������ʹ���ߴ���(ֻ�� ENTER ��������): ", uident);
					move(1, 0);
					clrtoeol();
					if (uident[0] == '\0')
						continue;
					if (!getuser(uident)) {
						move(2, 0);
						prints("���ʹ���ߴ����Ǵ����.\n");
					}
				}
				switch (tmp[0]) {
				case 'A':
				case 'a':
					if (seek_in_file(wishlists, uident)) {
						move(2, 0);
						prints("�Ѿ���Ϊ��ף����֮һ \n");
						break;
					}
					ytht_add_to_file(wishlists, uident);
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
						ytht_del_from_file(wishlists, uident, true);
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
						ythtbbs_cache_UserTable_getuserid(uinfo.friend[n], uident, sizeof(uident));
						prints("%s\n", uident);
						move(3, 0);
						n++;
						prints("(A)ȫ������ (Y)���� (N)������ (Q)����? [Y]:");
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
						if (key == '\0' || key == '\n' || key == 'y' || key == 'Y') {
							if (!getuser(uident)) {
								move(4, 0);
								prints("���ʹ���ߴ����Ǵ����.\n");
								i--;
								continue;
							} else if (seek_in_file(wishlists, uident)) {
								i--;
								continue;
							}
							ytht_add_to_file(wishlists, uident);
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
			prints("\033[m���������������ԡ�       ");
			move(6, 0);
			tmpbuf[0] = '\0';
			prints("��������[ֱ�Ӱ� ENTER �������ԣ���� 5 �䣬ÿ��� 50 �ַ�]:");
			for (count = 0; count < 5; count++) {
				getdata(7 + count, 0, ": ", tmpbuf, 51, DOECHO, YEA);
				if (tmpbuf[0] == '\0')
					break;;
				strcpy(buf[count], tmpbuf);
				tmpbuf[0] = '\0';
			}
			if (count == 0)
				return 0;
			sprintf(genbuf, "��ȷ��Ҫ��������������");
			move(9 + count, 0);
			if (askyn(genbuf, YEA, NA) == NA) {
				clear();
				return 0;
			}
			sethomefile_s(wishlists, sizeof(wishlists), currentuser.userid, "wishlist");
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
				sethomefile_s(genbuf, sizeof(genbuf), uid, "GoodWish");
				if ((fp = fopen(genbuf, "a")) == NULL) {
					prints("�޷��������û��ĵײ�������Ϣ�ļ�����֪ͨվ��...\n");
					pressanykey();
					fclose(mp);
					return NA;
				}
				now = time(0);
				timestr = ctime(&now) + 11;
				*(timestr + 5) = '\0';
				for (i = 0; i < count; i++) {
					fprintf(fp, "%s(%s)[%d/%d]��%s\n",
						currentuser.userid, timestr,
						i + 1, count, buf[i]);
				}
				fclose(fp);
				sethomefile_s(genbuf, sizeof(genbuf), uid, "HaveNewWish");
				if ((fp = fopen(genbuf, "w+")) != NULL) {
					fputs("Have New Wish", fp);
					fclose(fp);
				}
				move(11 + count, 0);
				//prints("�Ѿ������ͳ����������ˡ�\n");
				sprintf(genbuf, "%s sendgoodwish %s",
					currentuser.userid, uid);
				newtrace(genbuf);
			}	//for loop
			fclose(mp);
			return 0;
		}
	} else
		strcpy(uid, userid);
	if (!(tuid = getuser(uid))) {
		move(7, 0);
		prints("\033[1m�������ʹ���ߴ���( ID )�����ڣ�\033[m\n");
		pressanykey();
		clear();
		return -1;
	}
	move(5, 0);
	clrtoeol();
	prints("\033[m���� \033[1m%s\033[m ���ԡ�       ", uid);
	move(6, 0);
	tmpbuf[0] = '\0';
	prints("��������[ֱ�Ӱ� ENTER �������ԣ���� 5 �䣬ÿ��� 50 �ַ�]:");
	for (count = 0; count < 5; count++) {
		getdata(7 + count, 0, ": ", tmpbuf, 51, DOECHO, YEA);
		if (tmpbuf[0] == '\0')
			break;;
		strcpy(buf[count], tmpbuf);
		tmpbuf[0] = '\0';
	}
	if (count == 0)
		return 0;

	sprintf(genbuf, "��ȷ��Ҫ�����������Ը� \033[1m%s\033[m ��", uid);
	move(9 + count, 0);
	if (askyn(genbuf, YEA, NA) == NA) {
		clear();
		return 0;
	}
	sethomefile_s(genbuf, sizeof(genbuf), uid, "GoodWish");
	if ((fp = fopen(genbuf, "a")) == NULL) {
		prints("�޷��������û��ĵײ�������Ϣ�ļ�����֪ͨվ��...\n");
		pressanykey();
		return NA;
	}
	now = time(0);
	timestr = ctime(&now) + 11;
	*(timestr + 5) = '\0';
	for (i = 0; i < count; i++) {
		fprintf(fp, "%s(%s)[%d/%d]��%s\n",
			currentuser.userid, timestr, i + 1, count, buf[i]);
	}
	fclose(fp);
	sethomefile_s(genbuf, sizeof(genbuf), uid, "HaveNewWish");
	if ((fp = fopen(genbuf, "w+")) != NULL) {
		fputs("Have New Wish", fp);
		fclose(fp);
	}
	move(11 + count, 0);
	prints("�Ѿ������ͳ����������ˡ�\n");
	sprintf(genbuf, "%s sendgoodwish %s", currentuser.userid, uid);
	newtrace(genbuf);
	/*sprintf(genbuf,"������ף�����͸�������ô?");
	if(askyn(genbuf,YEA,NA)==YEA){
		usercomplete("���������� ID: ", uid);
		if (uid[0] == '\0') {
			clear();
			return 0;
		}
		if (!(tuid = getuser(uid))) {
			move(7, 0);
			prints("\x1b[1m�������ʹ���ߴ���( ID )�����ڣ�\x1b[m\n");
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

static void
childreturn(int i)
{
	(void) i;
	int retv;
	while ((retv = waitpid(-1, NULL, WNOHANG | WUNTRACED)) > 0)
		if (childpid > 0 && retv == childpid)
			childpid = 0;
}

int ent_bnet(const char *cmd) {
	int p[2];
#ifdef SSHBBS
	move(9, 0);
	clrtobot();
	prints("��Ŀǰ��ʹ�� ssh ��ʽ���� %s.\n"
			"ssh ����������ݴ�����м���,�����������������˽����Ϣ.\n"
			"������֪��,���紩����ͨ����վ���ӵ�����bbs,��Ȼ�����Ļ���\n"
			"����վ������ݴ����Ǽ��ܵ�,���Ǵӱ�վ������� BBS վ����\n"
			"���ݴ��䲢û�м���,�������������˽����Ϣ�б�������������\n����.\n",
			MY_BBS_NAME);
	if (askyn("��ȷ��Ҫʹ�����紩����", NA, NA) != 1)
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

int x_denylevel(const char *s) {
	(void) s;
	int id;
	char ans[7], content[2048];
	int oldlevel;
	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();
	move(0, 0);
	prints("����ʹ���߻���Ȩ��\n");
	clrtoeol();
	move(1, 0);
	usercomplete("���������ĵ�ʹ�����ʺ�: ", genbuf);
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
	prints("�趨ʹ���� '%s' �Ļ���Ȩ�� \n\n", genbuf);
	prints("(1) �����������Ȩ��       (A) �ָ���������Ȩ��\n");
	prints("(2) ȡ��������վȨ��       (B) �ָ�������վȨ��\n");
	prints("(3) ��ֹ����������         (C) �ָ�����������Ȩ��\n");
	prints("(4) ��ֹ������������       (D) �ָ�������������Ȩ��\n");
	prints("(5) ������˾�����         (E) ����������˾�����\n");
	prints("(6) ��ֹ�����ż�           (F) �ָ�����Ȩ��\n");
	prints("(7) ��ֹʹ��ǩ����         (G) �ָ�ʹ��ǩ����Ȩ��\n");
	prints("(8) ��ֹʹ���Զ�������     (H) �ָ�ʹ���Զ�������\n");
	getdata(13, 0, "��������Ĵ���: ", ans, 3, DOECHO, YEA);
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
			getdata(13, 0, "��ֹʹ��ǩ������ԭ��", genbuf, 40,
				DOECHO, YEA);
			sprintf(content,
				"������ֹʹ��ǩ������ԭ���ǣ�\n    %s\n\n"
				"(�������Ϊǩ����ͼƬ��С��������� Announce "
				"��Ĺ���<<����ͼƬǩ�����Ĵ�С����>>\n"
				"http://ytht.net/Ytht.Net/bbscon?B=Announce&F=M.1047666649.A )",
				genbuf);
			mail_buf(content, lookupuser.userid, "������ֹʹ��ǩ����");
		}
		break;
	case 'g':
	case 'G':
		lookupuser.userlevel &= ~PERM_DENYSIG;
		break;
	case '8':
		lookupuser.userlevel &= ~PERM_SELFLOCK;
		{
			getdata(13, 0, "��ֹʹ���Զ���������ԭ��", genbuf, 40,
				DOECHO, YEA);
			sprintf(content,
				"������ֹʹ���Զ���������ԭ������:\n\n   %s\n",
				genbuf);
			mail_buf(content, lookupuser.userid, "������ֹʹ���Զ�������");
		}
		break;
	case 'h':
	case 'H':
		lookupuser.userlevel |= PERM_SELFLOCK;
		sprintf(content,"�����Զ�����������Ѿ�����������µ�¼����Լ���ʹ��");
		mail_buf(content,lookupuser.userid,"������ʹ���Զ�������");
		break;
	default:
		prints("\n ʹ���� '%s' ����Ȩ��û�б��  %d\n", lookupuser.userid);
		pressreturn();
		clear();
		return 0;
	}
	{
		char secu[STRLEN];
		sprintf(secu, "�޸� %s �Ļ���Ȩ��", lookupuser.userid);
		permtostr(oldlevel, genbuf);
		sprintf(content, "�޸�ǰ��Ȩ�ޣ�%s\n�޸ĺ��Ȩ�ޣ�", genbuf);
		permtostr(lookupuser.userlevel, genbuf);
		strcat(content, genbuf);
		securityreport(secu, content);
	}

	substitute_record(PASSFILE, &lookupuser, sizeof (struct userec), id);
	clear();
	return 0;
}

int
s_checkid(const char *s)
{
	(void) s;
	char buf[256];
	char checkuser[20];
	int day, id;
	modify_user_mode(GMENU);
	clear();
	stand_title("����ID�������\n");
	clrtoeol();
	move(2, 0);
	usercomplete("�����������ʹ�����ʺ�: ", genbuf);
	if (genbuf[0] == '\0') {
	clear();
		return 0;
	}
	strcpy(checkuser, genbuf);
	if (!(id = getuser(genbuf))) {
		move(4, 0);
		prints("��Ч��ʹ�����ʺ�");
		clrtoeol();
		pressreturn();
	clear();
		return 0;
	}
	getdata(5, 0, "��������(0-����ʱ��): ", buf, 7, DOECHO, YEA);
	day = atoi(buf);
	sprintf(buf,
		 "/usr/bin/nice " MY_BBS_HOME "/bin/finddf %d %d %s > " MY_BBS_HOME
		 "/bbstmpfs/tmp/checkid.%s 2>/dev/null", currentuser.userlevel,
		 day, checkuser, currentuser.userid);
	if ((HAS_PERM(PERM_SYSOP, currentuser) && heavyload(2.5))
		|| (!HAS_PERM(PERM_SYSOP, currentuser) && heavyload(1.5))) {
		prints("ϵͳ���ع���, �޷�ִ�б�ָ��");
		pressreturn();
		return 1;
	}
	system(buf);
	sprintf(buf, "%s finddf %s %d", currentuser.userid, checkuser, day);
	newtrace(buf);
	sprintf(buf, MY_BBS_HOME "/bbstmpfs/tmp/checkid.%s",
		 currentuser.userid);
	mail_file(buf, currentuser.userid, "\"System Report\"");
	prints("���");
	clrtoeol();
	pressreturn();
	clear();
	return 1;
}

char *directfile(char *fpath, char *direct, char *filename) {
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
	static const char invalid[] = {
		'/', '\\', '!', '&', '|', '*', '?', '`', '\'', '\"', ';', '<',
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
	char to[512];
	char buf[512], *fn = NULL;
	char attachfile[512];
	char attach_to_send[1024];
	int  isa, base64;
	size_t len;

	ansimore("etc/zmodem", 0);
	move(14, 0);
	len = file_size(from);

	prints("�˴δ��乲 %d bytes, ��Լ��ʱ %d �루�� 5k/s ���㣩", len, len / ZMODEM_RATE);
	move(t_lines - 1, 0);
	clrtoeol();
	strcpy(name, "N");

	getdata(t_lines - 1, 0,
		"��ȷ��Ҫʹ��Zmodem�����ļ�ô?[y/N]", name, 2, DOECHO, YEA);
	if (toupper(name[0]) != 'Y')
		return FULLUPDATE;
	strncpy(name, title, 76);
	name[80] = '\0';
	escape_filename(name);
	move(t_lines - 2, 0);
	clrtoeol();
	prints("�������ļ�����Ϊ�������");
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
	snprintf(path, sizeof (path), PATHZMODEM "/%s.%d", currentuser.userid, uinfo.pid);
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
			snprintf(attachfile, sizeof(attachfile), "%s-attach-%s",  name, fn);
			if (getattach(fr, attachfile, path, base64, len, 0)) {
				fprintf(fw, "����%s����\n", fn);
			} else {
				sprintf(attach_to_send, "%s/%s", path, attachfile);
				bbs_zsendfile(attach_to_send);
				fprintf(fw, "����%s\n", fn);
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
get_load(double load[])
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
