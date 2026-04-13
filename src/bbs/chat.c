/*
	modify test by leoncom 2009.3.4(nothing but add this comment)
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
#include "chat.h"
#include "smth_screen.h"
#include "xyz.h"
#include "term.h"
#include "io.h"
#include "mail.h"
#include "bcache.h"
#include "sendmsg.h"
#include "stuff.h"
#include "record.h"
#include "talk.h"
#include "list.h"
#include "main.h"
#include "bbsinc.h"
#include "bbs_global_vars.h"
#include "bbs-internal.h"

char chatroom[IDLEN];		/* Chat-Room Name */
int chatline;			/* Where to display message now */
int stop_line;			/* next line of bottom of message window area */
FILE *rec;
int recflag = 0;
char buftopic[STRLEN];
char chat_station[19];

#define b_lines t_lines-1
#define cuser currentuser
// ———————————————————————————————————————
char *msg_seperator = "\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA\xA1\xAA";
// \033[1;33;44m 使用者代号    目前状态  │ 使用者代号    目前状态  │ 使用者代号    目前状态 \033[m
char *msg_shortulist = "\033[1;33;44m \xCA\xB9\xD3\xC3\xD5\xDF\xB4\xFA\xBA\xC5    \xC4\xBF\xC7\xB0\xD7\xB4\xCC\xAC  \xA9\xA6 \xCA\xB9\xD3\xC3\xD5\xDF\xB4\xFA\xBA\xC5    \xC4\xBF\xC7\xB0\xD7\xB4\xCC\xAC  \xA9\xA6 \xCA\xB9\xD3\xC3\xD5\xDF\xB4\xFA\xBA\xC5    \xC4\xBF\xC7\xB0\xD7\xB4\xCC\xAC \033[m";

struct chat_command {
	char *cmdname;		/* Char-room command length */
	void (*cmdfunc) (char *);	/* Pointer to function */
};

struct chatalias {
	char cmd[9];
	char action[81];
};

struct chatalias *chat_aliases;
int chat_alias_count;

static void chat_load_alias(void);
static int chat_send(int fd, char *buf);
static int chat_recv(int fd, char *chatid);
static void fixchatid(char *chatid);
static int printuserent(const struct user_info *uentp, void *);
static void chat_help(char *arg);
static void query_user(char *arg);
static void call_user(char *arg);
static void chat_date(char *arg);
static void chat_users(char *arg);
static void set_rec(char *arg);
static void define_alias(char *arg);
static int use_alias(char *arg, int cfd);
static int print_friend_ent(const struct user_info *uentp, void *);
static void chat_friends(char *arg);
static void chat_sendmsg(char *arg);
static int chat_cmd_match(char *buf, char *str);
static int chat_cmd(char *buf, int cfd);
static void setpager(char *);

static void
chat_load_alias()
{
	char buf[256];
	int i;
	chat_alias_count = 0;
	chat_aliases = (struct chatalias *) malloc(sizeof (struct chatalias) * MAXDEFINEALIAS);
	for (i = 0; i < MAXDEFINEALIAS; i++)
		chat_aliases[i].cmd[0] = 0;
	sethomefile_s(buf, sizeof(buf), currentuser.userid, "chatalias");
	chat_alias_count = get_num_records(buf, sizeof (struct chatalias));
	if (chat_alias_count > MAXDEFINEALIAS)
		chat_alias_count = MAXDEFINEALIAS;
	if (chat_alias_count != 0)
		get_records(buf, chat_aliases, sizeof (struct chatalias), 1, chat_alias_count);
	for (i = 0; i < chat_alias_count; i++) {
		if (chat_aliases[i].cmd[0] == 0) {
			chat_alias_count = i;
			break;
		}
		chat_aliases[i].cmd[8] = 0;
		chat_aliases[i].action[80] = 0;
	}
}

void printchatline(const char *str)
{
	const char *ptr1, *ptr2;
	move(chatline, 0);
	clrtoeol();
	ptr1 = str;
	ptr2 = index(str, '\n');
	while (1) {
		if (ptr2 != NULL) {
			outns(ptr1, ptr2 - ptr1 + 1);
		} else {
			outs(ptr1);
			outc('\n');
		}
		if (++chatline == stop_line)
			chatline = 2;
		move(chatline, 0);
		clrtoeol();
		if (ptr2 == NULL)
			break;
		ptr1 = ptr2 + 1;
		ptr2 = index(ptr1, '\n');
	}
	if (recflag == 1)
		fprintf(rec, "%s\n", str);
	// →
	outs("\xA1\xFA");
}

static void chat_clear() {
	for (chatline = 2; chatline < stop_line; chatline++) {
		move(chatline, 0);
		clrtoeol();
	}
	chatline = stop_line - 1;
	printchatline("");
}

static void print_chatid(char *chatid) {
	move(b_lines, 0);
	outs(chatid);
	outc(':');
}

static int chat_send(int fd, char *buf)
{
	size_t len;
	ssize_t size;

	snprintf(genbuf, sizeof(genbuf), "%s\n", buf);
	len = strlen(genbuf);
	size = send(fd, genbuf, len, 0);
	return ((size < 0) ? 0 : ((unsigned) size == len));
}

static int chat_recv(int fd, char *chatid)
{
	static char buf[512];
	static size_t bufstart = 0;
	ssize_t c;
	size_t len;
	char *bptr;

	len = sizeof (buf) - bufstart - 1;
	if ((c = recv(fd, buf + bufstart, len, 0)) <= 0)
		return -1;
	c += bufstart;
	bptr = buf;
	while (c > 0) {
		len = strlen(bptr) + 1;
		if (len > (unsigned long) c && len < (sizeof (buf) / 2))
			break;

		if (*bptr == '/') {
			switch (bptr[1]) {
			case 'c':
				chat_clear();
				break;
			case 'n':
				strncpy(chatid, bptr + 2, 8);
				print_chatid(chatid);
				clrtoeol();
				break;
			case 'r':
				strncpy(chatroom, bptr + 2, IDLEN - 1);
				break;
			case 't':
				move(0, 0);
				clrtoeol();
				// 房间： \033[32m%s
				snprintf(genbuf, sizeof genbuf, "\xB7\xBF\xBC\xE4\xA3\xBA \033[32m%s", chatroom);
				strncpy(buftopic, bptr + 2, STRLEN - 1);
				// \033[1;44;33m %-21s  \033[33m话题：\033[36m%-47s\033[5;31m%6s\033[m
				// 录音中
				prints("\033[1;44;33m %-21s  \033[33m\xBB\xB0\xCC\xE2\xA3\xBA\033[36m%-47s\033[5;31m%6s\033[m", genbuf, bptr + 2, (recflag == 1) ? "\xC2\xBC\xD2\xF4\xD6\xD0" : "      ");
			}
		} else {
			printchatline(bptr);
		}

		c -= len;
		bptr += len;
	}

	if (c > 0) {
		strcpy(genbuf, bptr);
		ytht_strsncpy(buf, genbuf, sizeof (buf));
		bufstart = len - 1;
	} else
		bufstart = 0;
	return 0;
}

static void fixchatid(char *chatid)
{
	chatid[CHAT_IDLEN] = '\0';
	while (*chatid != '\0' && *chatid != '\n') {
		if (strchr(BADCIDCHARS, *chatid))
			*chatid = '_';
		chatid++;
	}
}

int ent_chat(const char *chatbuf) {
	char inbuf[80], chatid[CHAT_IDLEN], lastcmd[MAXLASTCMD][80];
	struct sockaddr_in sin;
	struct hostent *h;
	int cfd, cmdpos, ch;
	int chatroom, chatport;
	int currchar;
	int newmail;
	extern int talkidletime;
	int page_pending = NA;
	int chatting = YEA;
	int i, j;
	char runchatbuf[STRLEN];
	char stnname[12][20];
	char stnaddr[12][30];
	char temp[10];
	int portnum[12];
	FILE *stationrec;
	chatroom = atoi(chatbuf);
	switch (chatroom) {
	case 4:
		strcpy(chat_station, CHATNAME4);
		modify_user_mode(CHAT4);
		chatport = CHATPORT4;
		break;
	case 3:
		strcpy(chat_station, CHATNAME3);
		modify_user_mode(CHAT3);
		chatport = CHATPORT3;
		break;
	case 2:
		strcpy(chat_station, CHATNAME2);
		modify_user_mode(CHAT2);
		chatport = CHATPORT2;
		break;
	case 1:
	default:
		strcpy(chat_station, CHATNAME1);
		modify_user_mode(CHAT1);
		chatport = CHATPORT1;
		break;
	}

	if ((chatroom == 1) && (stationrec = fopen("etc/chatstation", "r")) != NULL) {
		i = 0;
		while (fgets(inbuf, STRLEN, stationrec) != NULL && i <= 11) {
			strncpy(stnname[i], (char *) strtok(inbuf, " \n\r\t"), 19);
			strncpy(stnaddr[i], (char *) strtok(NULL, " \n\r\t"), 29);
			strncpy(temp, (char *) strtok(NULL, " \n\r\t"), 9);
			portnum[i] = atoi(temp);
			i++;
		}
		fclose(stationrec);
		move(1, 0);
		clrtobot();
		// \n\n序 连线站名称             连线站位址\n
		prints("\n\n\xD0\xF2 \xC1\xAC\xCF\xDF\xD5\xBE\xC3\xFB\xB3\xC6             \xC1\xAC\xCF\xDF\xD5\xBE\xCE\xBB\xD6\xB7\n");
		prints("== =====================  ==============================\n");
		for (j = 0; j <= i - 1; j++) {
			move(5 + j, 0);
			prints("%2d %-22s %-32s", j, stnname[j], stnaddr[j]);
		}
		// 请输入站台序号：
		getdata(23, 0, "\xC7\xEB\xCA\xE4\xC8\xEB\xD5\xBE\xCC\xA8\xD0\xF2\xBA\xC5\xA3\xBA", temp, 3, DOECHO, YEA);
		i = atoi(temp);
		if ((i > j - 1) || (i < 0))
			i = 0;
		clear();
		// 连往『%s』，请稍候...\n
		prints("\xC1\xAC\xCD\xF9\xA1\xBA%s\xA1\xBB\xA3\xAC\xC7\xEB\xC9\xD4\xBA\xF2...\n", stnname[i]);
		if (!(h = gethostbyname(stnaddr[i]))) {
			perror("gethostbyname");
			return -1;
		}
		memset(&sin, 0, sizeof sin);
		sin.sin_family = PF_INET;
		memcpy(&sin.sin_addr, h->h_addr, h->h_length);
		sin.sin_port = htons(portnum[i]);
		cfd = socket(sin.sin_family, SOCK_STREAM, 0);

		if (connect(cfd, (struct sockaddr *) &sin, sizeof (sin))) {
			gethostname(inbuf, STRLEN);
			if (!(h = gethostbyname(inbuf))) {
				prints("gethostbyname");
				close(cfd);
				return -1;
			}
			memset(&sin, 0, sizeof sin);
			sin.sin_family = PF_INET;
			memcpy(&sin.sin_addr, h->h_addr, h->h_length);
			chatport = CHATPORT1;
			sin.sin_port = htons(chatport);
			close(cfd);
			move(1, 0);
			clrtoeol();
			// 对方聊天室没开，连进本站的国际会议厅...
			prints("\xB6\xD4\xB7\xBD\xC1\xC4\xCC\xEC\xCA\xD2\xC3\xBB\xBF\xAA\xA3\xAC\xC1\xAC\xBD\xF8\xB1\xBE\xD5\xBE\xB5\xC4\xB9\xFA\xBC\xCA\xBB\xE1\xD2\xE9\xCC\xFC...");
			sprintf(runchatbuf, "%d", chatroom);
#ifdef IRIX
			if (fork() == 0) {
#else
			if (vfork() == 0) {
#endif
				close(0);
				execl("bin/chatd", "chatd", runchatbuf, NULL);
				exit(-1);
			}
			cfd = socket(sin.sin_family, SOCK_STREAM, 0);
			if ((connect(cfd, (struct sockaddr *) &sin, sizeof (sin)))) {
				perror("connect failed");
				close(cfd);
				return -1;
			}
		}
	} else {
		gethostname(inbuf, STRLEN);
		if (!(h = gethostbyname(inbuf))) {
			perror("gethostbyname");
			return -1;
		}
		memset(&sin, 0, sizeof (sin));
		sin.sin_family = PF_INET;
		memcpy(&sin.sin_addr, h->h_addr, h->h_length);
		sin.sin_port = htons(chatport);
		cfd = socket(sin.sin_family, SOCK_STREAM, 0);

		if (connect(cfd, (struct sockaddr *) &sin, sizeof (sin))) {
			close(cfd);
			move(1, 0);
			clrtoeol();
			// 开启聊天室...
			prints("\xBF\xAA\xC6\xF4\xC1\xC4\xCC\xEC\xCA\xD2...");
			sprintf(runchatbuf, "%d", chatroom);
#ifdef IRIX
			if (fork() == 0) {
#else
			if (vfork() == 0) {
#endif
				close(0);
				execl("bin/chatd", "chatd", runchatbuf, NULL);
				exit(-1);
			}
			sleep(1);
			cfd = socket(sin.sin_family, SOCK_STREAM, 0);
			if ((connect(cfd, (struct sockaddr *) &sin, sizeof (sin)))) {
				perror("connect failed");
				close(cfd);
				return -1;
			}
		}
	}
	while (1) {
		// 请输入聊天代号(输入'*'退出)：
		getdata(2, 0, "\xC7\xEB\xCA\xE4\xC8\xEB\xC1\xC4\xCC\xEC\xB4\xFA\xBA\xC5(\xCA\xE4\xC8\xEB'*'\xCD\xCB\xB3\xF6)\xA3\xBA", inbuf,
			CHAT_IDLEN, DOECHO, YEA);
		if (!strcmp(inbuf, "*") || stringfilter(inbuf, YTHT_SMTH_FILTER_OPTION_SIMPLE) != YTHT_SMTH_FILTER_RESULT_SAFE) {
			if (strcmp(inbuf, "*")) {
				// 在聊天室起过滤名
				strcpy(genbuf, "\xD4\xDA\xC1\xC4\xCC\xEC\xCA\xD2\xC6\xF0\xB9\xFD\xC2\xCB\xC3\xFB");
				mail_buf(inbuf, "delete", genbuf);
				ythtbbs_cache_Board_updatelastpost("deleterequest");
			}
			close(cfd);
			clear();
			uinfo.in_chat = NA;
			uinfo.chatid[0] = '\0';
			update_utmp();
			return 0;
		}
		if (stringfilter(inbuf, YTHT_SMTH_FILTER_OPTION_PLTCAL) != YTHT_SMTH_FILTER_RESULT_SAFE) {
			// 在聊天室起过滤名
			strcpy(genbuf, "\xD4\xDA\xC1\xC4\xCC\xEC\xCA\xD2\xC6\xF0\xB9\xFD\xC2\xCB\xC3\xFB");
			mail_buf(inbuf, "delete", genbuf);
			ythtbbs_cache_Board_updatelastpost("deleterequest");
		}
		snprintf(chatid, sizeof chatid, "%.8s",
			((inbuf[0] != '\0' && inbuf[0] != '\n') ? inbuf : cuser.userid));
		fixchatid(chatid);
		snprintf(inbuf, sizeof inbuf, "/! %d %d %s %s 0", uinfo.uid,
			cuser.userlevel, cuser.userid, chatid);
		chat_send(cfd, inbuf);
		if (recv(cfd, inbuf, 3, 0) != 3) {
			close(cfd);
			return 0;
		}
		inbuf[3] = 0;
		move(3, 0);
		if (!strcmp(inbuf, CHAT_LOGIN_OK))
			break;
		else if (!strcmp(inbuf, CHAT_LOGIN_EXISTS))
			// 这个代号已经有人用了
			prints("\xD5\xE2\xB8\xF6\xB4\xFA\xBA\xC5\xD2\xD1\xBE\xAD\xD3\xD0\xC8\xCB\xD3\xC3\xC1\xCB");
		else if (!strcmp(inbuf, CHAT_LOGIN_INVALID))
			// 这个代号是错误的
			prints("\xD5\xE2\xB8\xF6\xB4\xFA\xBA\xC5\xCA\xC7\xB4\xED\xCE\xF3\xB5\xC4");
		else
			// 你已经有另一个视窗进入此聊天室。
			prints("\xC4\xE3\xD2\xD1\xBE\xAD\xD3\xD0\xC1\xED\xD2\xBB\xB8\xF6\xCA\xD3\xB4\xB0\xBD\xF8\xC8\xEB\xB4\xCB\xC1\xC4\xCC\xEC\xCA\xD2\xA1\xA3");
		clrtoeol();
		refresh();
		sleep(1);
		bell();
	}

	endmsg();
	add_io(cfd, 0);

	newmail = cmdpos = currchar = 0;
	memset(lastcmd, 0, MAXLASTCMD * 80);

	uinfo.in_chat = YEA;
	strcpy(uinfo.chatid, chatid);
	update_utmp();

	clear();
	chatline = 2;
	strcpy(inbuf, chatid);
	stop_line = t_lines - 2;
	move(stop_line, 0);
	outs(msg_seperator);
	move(1, 0);
	outs(msg_seperator);
	print_chatid(chatid);
	memset(inbuf, 0, 80);

	chat_load_alias();
	while (chatting) {
		move(b_lines, currchar + 10);
		ch = igetkey();
		talkidletime = 0;
		if (talkrequest)
			page_pending = YEA;
		if (page_pending)
			page_pending = servicepage(0, NULL);

		switch (ch) {
		case KEY_UP:
			cmdpos += MAXLASTCMD - 2;
			__attribute__((fallthrough));
		case KEY_DOWN:
			cmdpos++;
			cmdpos %= MAXLASTCMD;
			strcpy(inbuf, lastcmd[cmdpos]);
			move(b_lines, 10);
			clrtoeol();
			outs(inbuf);
			currchar = strlen(inbuf);
			continue;

		case KEY_LEFT:
			if (currchar)
				--currchar;
			continue;

		case KEY_RIGHT:
			if (inbuf[currchar])
				++currchar;
			continue;
		}

		if (!newmail && chkmail()) {
			newmail = 1;
			// \033[1;32m◆ \033[31m当！邮差送信来了...\033[m
			printchatline("\033[1;32m\xA1\xF4 \033[31m\xB5\xB1\xA3\xA1\xD3\xCA\xB2\xEE\xCB\xCD\xD0\xC5\xC0\xB4\xC1\xCB...\033[m");
		}

		if (ch == I_OTHERDATA) {	/* incoming */
			if (chat_recv(cfd, chatid) == -1)
				break;
			continue;
		}
#ifdef BIT8
		if (isprint2(ch))
#else
		if (isprint(ch))
#endif

		{
			if (currchar < 68) {
				if (inbuf[currchar]) {	/* insert */
					int i;
					for (i = currchar; inbuf[i] && i < 68; i++) ;
					inbuf[i + 1] = '\0';
					for (; i > currchar; i--)
						inbuf[i] = inbuf[i - 1];
				} else {	/* append */
					inbuf[currchar + 1] = '\0';
				}
				inbuf[currchar] = ch;
				move(b_lines, currchar + 10);
				outs(&inbuf[currchar++]);
			}
			continue;
		}

		if (ch == '\n' || ch == '\r') {
			if (currchar) {
				if (stringfilter(inbuf, YTHT_SMTH_FILTER_OPTION_SIMPLE) != YTHT_SMTH_FILTER_RESULT_SAFE) {
					// 在聊天室说过滤词
					strcpy(genbuf, "\xD4\xDA\xC1\xC4\xCC\xEC\xCA\xD2\xCB\xB5\xB9\xFD\xC2\xCB\xB4\xCA");
					mail_buf(inbuf, "delete", genbuf);
					ythtbbs_cache_Board_updatelastpost("deleterequest");
					// \033[1;32m聊天室禁止污言秽语哦\033[m
					printchatline("\033[1;32m\xC1\xC4\xCC\xEC\xCA\xD2\xBD\xFB\xD6\xB9\xCE\xDB\xD1\xD4\xBB\xE0\xD3\xEF\xC5\xB6\033[m");
					inbuf[0] = '\0';
					currchar = 0;
					move(b_lines, 10);
					clrtoeol();
					chatting = 1;
					continue;
				}
				if (stringfilter(inbuf, YTHT_SMTH_FILTER_OPTION_PLTCAL) != YTHT_SMTH_FILTER_RESULT_SAFE) {
					// 在聊天室说过滤词
					sprintf(genbuf, "\xD4\xDA\xC1\xC4\xCC\xEC\xCA\xD2\xCB\xB5\xB9\xFD\xC2\xCB\xB4\xCA");
					mail_buf(inbuf, "delete", genbuf);
					ythtbbs_cache_Board_updatelastpost("deleterequest");
				}
				chatting = chat_cmd(inbuf, cfd);
				if (chatting == 0)
					chatting = chat_send(cfd, inbuf);
				if (!strncmp(inbuf, "/b", 2))
					break;
				for (cmdpos = MAXLASTCMD - 1; cmdpos; cmdpos--)
					strcpy(lastcmd[cmdpos], lastcmd[cmdpos - 1]);
				strcpy(lastcmd[0], inbuf);

				inbuf[0] = '\0';
				currchar = cmdpos = 0;
				move(b_lines, 10);
				clrtoeol();
			}
			continue;
		}

		if (ch == Ctrl('H') || ch == '\177') {
			if (currchar) {
				currchar--;
				inbuf[69] = '\0';
				memcpy(&inbuf[currchar], &inbuf[currchar + 1], 69 - currchar);
				move(b_lines, currchar + 10);
				clrtoeol();
				outs(&inbuf[currchar]);
			}
			continue;
		}
		if (ch == Ctrl('Q')) {
			inbuf[0] = '\0';
			currchar = 0;
			move(b_lines, 10);
			clrtoeol();
			continue;
		}

		if (ch == Ctrl('C') || ch == Ctrl('D')) {
			chat_send(cfd, "/b");
			if (recflag == 1) {
				set_rec(NULL);
			}
			break;
		}
	}
	close(cfd);
	add_io(0, 0);
	uinfo.in_chat = NA;
	uinfo.chatid[0] = '\0';
	update_utmp();
	clear();
	free(chat_aliases);
	return 0;
}

static int printuserent(const struct user_info *uentp, void *x_param) {
	(void) x_param;
	static char uline[256];
	static int cnt;
	char pline[50];

	if (!uentp) {
		if (cnt)
			printchatline(uline);
		bzero(uline, 256);
		cnt = 0;
		return 0;
	}
	if (!uentp->active || !uentp->pid)
		return 0;
	if (!(HAS_PERM(PERM_SYSOP, currentuser) || HAS_PERM(PERM_SEECLOAK, currentuser)) && uentp->invisible)
		return 0;

	snprintf(pline, sizeof pline, " %s%-13s\x1b[m%c%s%-10.10s\x1b[m",
		myfriend(uentp->uid) ? "\x1b[1;32m" : "",
		uentp->userid, uentp->invisible ? '#' : ' ',
		ModeColor(uentp->mode), ModeType(uentp->mode));
	if (cnt < 2)
		// │
		strcat(pline, "\xA9\xA6");
	strcat(uline, pline);
	if (++cnt == 3) {
		printchatline(uline);
		memset(uline, 0, 256);
		cnt = 0;
	}
	return 0;
}

static void chat_help(char *arg)
{
	char *ptr;
	char buf[256];
	FILE *fp;

	if (strstr(arg, " op")) {
		if ((fp = fopen("help/chatophelp", "r")) == NULL)
			return;
		while (fgets(buf, 255, fp) != NULL) {
			if ((ptr = strchr(buf, '\n')) != NULL)
				*ptr = '\0';
			buf[sizeof buf - 1] = '\0';
			printchatline(buf);
		}
		fclose(fp);
	} else {
		if ((fp = fopen("help/chathelp", "r")) == NULL)
			return;
		while (fgets(buf, 255, fp) != NULL) {
			if ((ptr = strchr(buf, '\n')) != NULL)
				*ptr = '\0';
			buf[sizeof buf - 1] = '\0';
			printchatline(buf);
		}
		fclose(fp);
	}
}

/* youzi 1997.7.25 */
static void query_user(char *arg)
{
	char *userid, msg[STRLEN * 2];
	char qry_mail_dir[STRLEN];
	char time_buf[STRLEN];

	userid = strrchr(arg, ' ');

	if (userid == NULL) {
		// \033[1;37m★ \033[33m请输入您要查寻的 ID \033[37m★\033[m
		printchatline("\033[1;37m\xA1\xEF \033[33m\xC7\xEB\xCA\xE4\xC8\xEB\xC4\xFA\xD2\xAA\xB2\xE9\xD1\xB0\xB5\xC4 ID \033[37m\xA1\xEF\033[m");
		return;
	}
	userid++;
	if (!getuser(userid)) {
		// \033[1;31m不正确的使用者代号\033[m
		printchatline("\033[1;31m\xB2\xBB\xD5\xFD\xC8\xB7\xB5\xC4\xCA\xB9\xD3\xC3\xD5\xDF\xB4\xFA\xBA\xC5\033[m");
		return;
	}
	snprintf(qry_mail_dir, sizeof qry_mail_dir, "mail/%c/%s/%s", mytoupper(lookupuser.userid[0]),
		lookupuser.userid, DOT_DIR);

	// \033[1;32m%s\033[m (\033[1;37m%s\033[m) 共上站 \033[1;37m%d\033[m 次, 发表
	sprintf(msg, "\033[1;32m%s\033[m (\033[1;37m%s\033[m) \xB9\xB2\xC9\xCF\xD5\xBE \033[1;37m%d\033[m \xB4\xCE, \xB7\xA2\xB1\xED"
		// 过 \033[1;37m%d\033[m 篇文章
		"\xB9\xFD \033[1;37m%d\033[m \xC6\xAA\xCE\xC4\xD5\xC2", lookupuser.userid,
		lookupuser.username, lookupuser.numlogins, lookupuser.numposts);

	printchatline(msg);

	ytht_strsncpy(time_buf, lookupuser.dietime ? ytht_ctime(lookupuser.dietime) : ytht_ctime(lookupuser.lastlogin), sizeof(time_buf));
	if (ifinprison(lookupuser.userid)) {
		ytht_strsncpy(time_buf, ytht_ctime(lookupuser.lastlogin), sizeof(time_buf));
		// 在监狱服刑，入狱时间[\033[1m%s\033[m]
		snprintf(msg, sizeof msg, "\xD4\xDA\xBC\xE0\xD3\xFC\xB7\xFE\xD0\xCC\xA3\xAC\xC8\xEB\xD3\xFC\xCA\xB1\xBC\xE4[\033[1m%s\033[m]", time_buf);
	} else if (lookupuser.dietime) {
		snprintf(msg, sizeof msg,
			// 已经死亡，还有 [\033[1m%d\033[m] 天就要转世投胎了
			"\xD2\xD1\xBE\xAD\xCB\xC0\xCD\xF6\xA3\xAC\xBB\xB9\xD3\xD0 [\033[1m%d\033[m] \xCC\xEC\xBE\xCD\xD2\xAA\xD7\xAA\xCA\xC0\xCD\xB6\xCC\xA5\xC1\xCB",
			countlife(&lookupuser));
	} else {
		snprintf(msg, sizeof msg,
			// 上次在 [\033[1;37m%-24.24s\033[m] 由 [\033[1;37m%s\033[m] 到本站一游
			"\xC9\xCF\xB4\xCE\xD4\xDA [\033[1;37m%-24.24s\033[m] \xD3\xC9 [\033[1;37m%s\033[m] \xB5\xBD\xB1\xBE\xD5\xBE\xD2\xBB\xD3\xCE",
			// (不详)
			time_buf, (lookupuser.lasthost[0] == '\0' ? "(\xB2\xBB\xCF\xEA)" : lookupuser.lasthost));
	}

	printchatline(msg);

	// 信箱：[\033[1;5;37m%2s\033[m]，生命力：[\033[1;37m%d\033[m]
	snprintf(msg, sizeof msg, "\xD0\xC5\xCF\xE4\xA3\xBA[\033[1;5;37m%2s\033[m]\xA3\xAC\xC9\xFA\xC3\xFC\xC1\xA6\xA3\xBA[\033[1;37m%d\033[m]",
		// ⊙
		(check_query_mail(qry_mail_dir) == 1) ? "\xA1\xD1" : "  ",
		countlife(&lookupuser));
	printchatline(msg);
	// \033[1;37m目前%s站上\033[m
	snprintf(msg, sizeof msg, "\033[1;37m\xC4\xBF\xC7\xB0%s\xD5\xBE\xC9\xCF\033[m", (t_search(userid, NA, 1) != NULL) ?
		// 在
		// 不在
		"\xD4\xDA" : "\xB2\xBB\xD4\xDA");
	printchatline(msg);
}

static void call_user(char *arg)
{
	char *userid, msg[STRLEN * 2];
	struct user_info *uin = NULL;
	int good_id;

	userid = strrchr(arg, ' ');
	if (userid == NULL) {
		// \033[1;37m★ \033[32m请输入你要邀请的 ID\033[37m ★\033[m
		printchatline("\033[1;37m\xA1\xEF \033[32m\xC7\xEB\xCA\xE4\xC8\xEB\xC4\xE3\xD2\xAA\xD1\xFB\xC7\xEB\xB5\xC4 ID\033[37m \xA1\xEF\033[m");
		return;
	} else
		userid += 1;
	if (!strcasecmp(userid, currentuser.userid))
		good_id = NA;
	else {
		uin = t_search(userid, NA, 1);
		if (uin == NULL)
			good_id = NA;
		else
			good_id = YEA;
	}
	if (good_id == YEA && canmsg(uin)) {
		// 到 %s 的 %s 包厢聊聊天
		snprintf(msg, sizeof msg, "\xB5\xBD %s \xB5\xC4 %s \xB0\xFC\xCF\xE1\xC1\xC4\xC1\xC4\xCC\xEC", chat_station, chatroom);
		do_sendmsg(userid, uin, msg, 1, uin->pid);
		// \033[1;37m已经帮你邀请 \033[32m%s\033[37m 了\033[m
		snprintf(msg, sizeof msg, "\033[1;37m\xD2\xD1\xBE\xAD\xB0\xEF\xC4\xE3\xD1\xFB\xC7\xEB \033[32m%s\033[37m \xC1\xCB\033[m",
			uin->userid);
	} else
		snprintf(msg, sizeof msg, "\033[1;32m%s\033[37m %s\033[m", userid,
			// 无法呼叫
			// 并没有上站
			uin ? "\xCE\xDE\xB7\xA8\xBA\xF4\xBD\xD0" : "\xB2\xA2\xC3\xBB\xD3\xD0\xC9\xCF\xD5\xBE");
	printchatline(msg);
}

static void
chat_date(char *arg)
{
	(void) arg;
	time_t thetime;

	time(&thetime);
	// \033[1m %s标准时间: \033[32m%s\033[0m
	snprintf(genbuf, sizeof genbuf, "\033[1m %s\xB1\xEA\xD7\xBC\xCA\xB1\xBC\xE4: \033[32m%s\033[0m", MY_BBS_NAME, ctime(&thetime));
	printchatline(genbuf);
}

static void
chat_users(char *arg)
{
	(void) arg;
	printchatline("");
	// \033[1m【 \033[36m%s \033[37m的访客列表 】\033[m
	snprintf(genbuf, sizeof genbuf, "\033[1m\xA1\xBE \033[36m%s \033[37m\xB5\xC4\xB7\xC3\xBF\xCD\xC1\xD0\xB1\xED \xA1\xBF\033[m", MY_BBS_NAME);
	printchatline(genbuf);
	printchatline(msg_shortulist);

	if (ythtbbs_cache_utmp_apply(printuserent, NULL) == -1) {
		// \033[1m空无一人\033[m
		printchatline("\033[1m\xBF\xD5\xCE\xDE\xD2\xBB\xC8\xCB\033[m");
	}
	printuserent(NULL, NULL);
}

static void
set_rec(char *arg)
{
	(void) arg;
	char fname[STRLEN];
	time_t now;
	int savemode;

	now = time(0);

	snprintf(fname, sizeof fname, "tmp/chat.%s", currentuser.userid);
	if (recflag == 0) {
		if ((rec = fopen(fname, "w")) == NULL)
			return;

		// \033[1;5;32m开始录音...\033[m
		printchatline("\033[1;5;32m\xBF\xAA\xCA\xBC\xC2\xBC\xD2\xF4...\033[m");
		recflag = 1;
		move(0, 0);
		clrtoeol();
		// 房间： \033[32m%s
		snprintf(genbuf, sizeof genbuf, "\xB7\xBF\xBC\xE4\xA3\xBA \033[32m%s", chatroom);
		// \033[1;44;33m %-21s  \033[33m话题：\033[36m%-47s\033[5;31m%6s\033[m
		// 录音中
		prints("\033[1;44;33m %-21s  \033[33m\xBB\xB0\xCC\xE2\xA3\xBA\033[36m%-47s\033[5;31m%6s\033[m", genbuf, buftopic, (recflag == 1) ? "\xC2\xBC\xD2\xF4\xD6\xD0" : "      ");

		// 本段由 %s
		fprintf(rec, "\xB1\xBE\xB6\xCE\xD3\xC9 %s", currentuser.userid);
		// 所录下，时间： %s
		fprintf(rec, "\xCB\xF9\xC2\xBC\xCF\xC2\xA3\xAC\xCA\xB1\xBC\xE4\xA3\xBA %s", ctime(&now));
	} else {
		savemode = uinfo.mode;
		recflag = 0;
		move(0, 0);
		clrtoeol();
		// 房间： \033[32m%s
		snprintf(genbuf, sizeof genbuf, "\xB7\xBF\xBC\xE4\xA3\xBA \033[32m%s", chatroom);
		// \033[1;44;33m %-21s  \033[33m话题：\033[36m%-47s\033[5;31m%6s\033[m
		// 录音中
		prints("\033[1;44;33m %-21s  \033[33m\xBB\xB0\xCC\xE2\xA3\xBA\033[36m%-47s\033[5;31m%6s\033[m", genbuf, buftopic, (recflag == 1) ? "\xC2\xBC\xD2\xF4\xD6\xD0" : "      ");

		// \033[1;5;32m录音结束...\033[m
		printchatline("\033[1;5;32m\xC2\xBC\xD2\xF4\xBD\xE1\xCA\xF8...\033[m");
		// 结束时间：%s\n
		fprintf(rec, "\xBD\xE1\xCA\xF8\xCA\xB1\xBC\xE4\xA3\xBA%s\n", ctime(&now));
		fclose(rec);
		// 录音结果
		mail_file(fname, currentuser.userid, "\xC2\xBC\xD2\xF4\xBD\xE1\xB9\xFB");
		/*postfile(fname,"syssecurity","录音结果",2); */
		unlink(fname);
		modify_user_mode(savemode);
	}
}

void
setpager(char *arg)
{
	(void) arg;
	char buf[STRLEN];

	t_pager(NULL);
	// \033[1;32m◆ \033[31m呼叫器 %s 了\033[m
	snprintf(buf, sizeof buf, "\033[1;32m\xA1\xF4 \033[31m\xBA\xF4\xBD\xD0\xC6\xF7 %s \xC1\xCB\033[m",
		// 打开
		// 关闭
		(uinfo.pager & ALL_PAGER) ? "\xB4\xF2\xBF\xAA" : "\xB9\xD8\xB1\xD5");
	printchatline(buf);

}

static void define_alias(char *arg)
{
	int i;
	int del = 0;
	char buf[256];
	char *action = NULL;

	for (i = 0; (i < 255) && (arg[i] != 0) && !isspace(arg[i]); i++) ;

	if (arg[i] == 0) {
		if (chat_alias_count != 0) {
			// 已定义的alias:
			printchatline("\xD2\xD1\xB6\xA8\xD2\xE5\xB5\xC4" "alias:");
			for (i = 0; i < chat_alias_count; i++) {
				if (chat_aliases[i].cmd[0] != 0) {
					snprintf(buf, sizeof buf, "%-9s %s\n",
						chat_aliases[i].cmd,
						chat_aliases[i].action);
					printchatline(buf);
				};
			};
			return;
		} else
			// 未定义alias\n
			printchatline("\xCE\xB4\xB6\xA8\xD2\xE5" "alias\n");
	};

	arg = &arg[i + 1];
	for (i = 0; (i < 9) && (arg[i] != 0) && !isspace(arg[i]); i++) ;
	if (i >= 9) {
		// alias太长!\n
		printchatline("alias\xCC\xAB\xB3\xA4!\n");
		return;
	};

	if (arg[i] == 0)
		del = 1;
	else
		action = &arg[i + 1];

	arg[i] = 0;

	if (!del)
		if (chat_alias_count == MAXDEFINEALIAS) {
			// 自定义alias已经满了\n
			printchatline("\xD7\xD4\xB6\xA8\xD2\xE5" "alias\xD2\xD1\xBE\xAD\xC2\xFA\xC1\xCB\n");
			return;
		}

	for (i = 0; i < chat_alias_count; i++)
		if (!strncasecmp(chat_aliases[i].cmd, arg, 8)) {
			if (del) {
				chat_alias_count--;
				sethomefile_s(buf, sizeof(buf), currentuser.userid, "chatalias");
				if (chat_alias_count != 0) {
					memcpy(&chat_aliases[i], &chat_aliases[chat_alias_count], sizeof (struct chatalias));
					chat_aliases[chat_alias_count].cmd[0] = 0;
					substitute_record(buf, &chat_aliases [chat_alias_count], sizeof (chat_aliases [chat_alias_count]), chat_alias_count + 1);
				} else {
					chat_aliases[i].cmd[0] = 0;
				}
				substitute_record(buf, &chat_aliases[i], sizeof (chat_aliases[i]), i + 1);
				// 自定义alias已经删除\n
				snprintf(buf, sizeof buf, "\xD7\xD4\xB6\xA8\xD2\xE5" "alias\xD2\xD1\xBE\xAD\xC9\xBE\xB3\xFD\n");
				printchatline(buf);
				return;
			} else {
				// 自定义alias-%s已经存在\n
				snprintf(buf, sizeof buf, "\xD7\xD4\xB6\xA8\xD2\xE5" "alias-%s\xD2\xD1\xBE\xAD\xB4\xE6\xD4\xDA\n", chat_aliases[i].cmd);
				printchatline(buf);
				return;
			}
		}
	if (!del) {
		struct chatalias EMPTY;
		strncpy(chat_aliases[chat_alias_count].cmd, arg, 8);
		chat_aliases[chat_alias_count].cmd[sizeof(EMPTY.cmd) - 1] = 0;
		strncpy(chat_aliases[chat_alias_count].action, action, 80);
		chat_aliases[chat_alias_count].action[sizeof(EMPTY.cmd) - 1] = 0;
		// 自定义alias-%s已经创建\n
		snprintf(buf, sizeof buf, "\xD7\xD4\xB6\xA8\xD2\xE5" "alias-%s\xD2\xD1\xBE\xAD\xB4\xB4\xBD\xA8\n", arg);
		printchatline(buf);
		i = chat_alias_count;
		chat_alias_count++;
	} else {
		// 没找到自定义alias\n
		printchatline("\xC3\xBB\xD5\xD2\xB5\xBD\xD7\xD4\xB6\xA8\xD2\xE5" "alias\n");
		return;
	}
	sethomefile_s(buf, sizeof(buf), currentuser.userid, "chatalias");
	substitute_record(buf, &chat_aliases[i], sizeof (chat_aliases[i]), i + 1);
}

static int use_alias(char *arg, int cfd)
{
	char buf[256];
	char buf1[256];
	char *args[10];
	int arg_count;
	int i;
	int slen;
	register char *fmt;

	strncpy(buf, arg, 255);
	arg_count = 0;
	args[0] = buf;
	args[1] = buf;
	while (*args[arg_count + 1] != 0)
		if (isspace(*args[arg_count + 1])) {
			arg_count++;
			if (arg_count == 9)
				break;
			*args[arg_count] = 0;
			args[arg_count]++;
			args[arg_count + 1] = args[arg_count];
		} else
			args[arg_count + 1]++;
	for (i = arg_count + 1; i < 10; i++)
		// 大家
		args[i] = "\xB4\xF3\xBC\xD2";

	for (i = 0; i < chat_alias_count; i++) {
		if (!strncasecmp(chat_aliases[i].cmd, args[0], 8)) {
			strcpy(buf1, "/a ");
			slen = 3;
			fmt = chat_aliases[i].action;
			while (*fmt != 0) {
				if (*fmt == '$') {
					fmt++;
					if (isdigit(*fmt)) {
						int index = *fmt - '0';
						if (slen + strlen(args[index]) > 255 - 8)
							break;
						buf1[slen] = 0;
						strcat(buf1, "\033[1m");
						strcat(buf1, args[index]);
						strcat(buf1, "\033[m");
						slen += strlen(args[index]) + 7;
					} else if (*fmt == 's') {
						if (slen + strlen(args[1]) > 255 - 8)
							break;
						buf1[slen] = 0;
						strcat(buf1, "\033[1m");
						strcat(buf1, args[1]);
						strcat(buf1, "\033[m");
						slen += strlen(args[1]) + 7;
					} else if (slen < 253) {
						buf1[slen++] = '$';
						buf1[slen++] = *fmt;
					} else
						break;
				} else
					buf1[slen++] = *fmt;
				fmt++;
			}
			buf1[slen] = 0;
			chat_send(cfd, buf1);
			return 1;
		}
	}
	return 0;
}

/* add from SMTH BBS source */
/* print one user & status if he is a friend */
static int print_friend_ent(const struct user_info *uentp, void *x_param) {
	(void) x_param;
	static char uline[256];
	static int cnt;
	char pline[50];

	if (!uentp) {
		if (cnt)
			printchatline(uline);
		bzero(uline, 256);
		cnt = 0;
		return 0;
	}
	if (!uentp->active || !uentp->pid)
		return 0;
	if (!HAS_PERM(PERM_SEECLOAK, currentuser) && uentp->invisible)
		return 0;
#if 0
	if (kill(uentp->pid, 0) == -1)
		return 0;
#endif

	/*if (!myfriend(uentp->userid)) Leeward 99.02.01 */
	if (!myfriend(uentp->uid))
		return 0;

	snprintf(pline, sizeof pline, " %-13s%c%-10s",
		uentp->userid, uentp->invisible ? '#' : ' ',
		ModeType(uentp->mode));
	if (cnt < 2)
		// │
		strcat(pline, "\xA9\xA6");
	strcat(uline, pline);
	if (++cnt == 3) {
		printchatline(uline);
		memset(uline, 0, 256);
		cnt = 0;
	}
	return 0;
}

static void
chat_friends(char *arg)
{
	(void) arg;
	printchatline("");
	// \033[1m【 当前线上的好友列表 】\033[m
	snprintf(genbuf, sizeof genbuf, "\033[1m\xA1\xBE \xB5\xB1\xC7\xB0\xCF\xDF\xC9\xCF\xB5\xC4\xBA\xC3\xD3\xD1\xC1\xD0\xB1\xED \xA1\xBF\033[m");
	printchatline(genbuf);
	printchatline(msg_shortulist);

	if (ythtbbs_cache_utmp_apply(print_friend_ent, NULL) == -1) {
		// \033[1m没有朋友在线上\033[m
		printchatline("\033[1m\xC3\xBB\xD3\xD0\xC5\xF3\xD3\xD1\xD4\xDA\xCF\xDF\xC9\xCF\033[m");
	}
	print_friend_ent(NULL, NULL);
}

static void chat_sendmsg(char *arg)
{
	struct user_info *uentp;
	char *uident, *msgstr, showstr[80];

	uident = strchr(arg, ' ');
	if (uident == NULL) {
		// \033[1;32m请输入你要发消息的 ID\033[m
		printchatline("\033[1;32m\xC7\xEB\xCA\xE4\xC8\xEB\xC4\xE3\xD2\xAA\xB7\xA2\xCF\xFB\xCF\xA2\xB5\xC4 ID\033[m");
		return;
	} else
		uident += 1;

	msgstr = strchr(uident, ' ');
	if (msgstr == NULL) {
		// \033[1;32m请输入你要发的消息\033[m
		printchatline("\033[1;32m\xC7\xEB\xCA\xE4\xC8\xEB\xC4\xE3\xD2\xAA\xB7\xA2\xB5\xC4\xCF\xFB\xCF\xA2\033[m");
		return;
	}
	*msgstr = 0;
	msgstr++;

	uentp = t_search(uident, NA, 1);
	if (uentp == NULL) {
		// \033[1m线上没有这个ID\033[m
		printchatline("\033[1m\xCF\xDF\xC9\xCF\xC3\xBB\xD3\xD0\xD5\xE2\xB8\xF6ID\033[m");
		return;
	}

	if (do_sendmsg(uentp->userid, uentp, msgstr, 2, 0) != 1) {
		// \033[1m无法发消息给 %s \033[m
		snprintf(showstr, sizeof showstr, "\033[1m\xCE\xDE\xB7\xA8\xB7\xA2\xCF\xFB\xCF\xA2\xB8\xF8 %s \033[m", uentp->userid);
		printchatline(showstr);
		return;
	}
	// \033[1m已经给 %s 发出消息\033[m
	snprintf(showstr, sizeof showstr, "\033[1m\xD2\xD1\xBE\xAD\xB8\xF8 %s \xB7\xA2\xB3\xF6\xCF\xFB\xCF\xA2\033[m", uentp->userid);
	printchatline(showstr);
}

static const struct chat_command chat_cmdtbl[] = {
	{"alias", define_alias},	/* 1998.12.6 KCN */
	{"pager", setpager},
	{"help", chat_help},
	{"clear", chat_clear},
	{"date", chat_date},
	{"users", chat_users},
	{"set", set_rec},
	{"call", call_user},
	{"query", query_user},	/* 1997.7.25 youzi */
	{"g", chat_friends},
	{NULL, NULL}
};

static int chat_cmd_match(char *buf, char *str)
{
	while (*str && *buf && !isspace(*buf)) {
		if (tolower(*buf++) != *str++)
			return 0;
	}
	return 1;
}

static int chat_cmd(char *buf, int cfd)
{
	int i;

	if (*buf++ != '/')
		return 0;

	if (*buf == '/') {
		if (!use_alias(buf + 1, cfd))
			return 0;
		else
			return 1;
	}
	if ((tolower(*buf) == 'm') && isspace(*(buf + 1)))
		return 0;
	if ((tolower(*buf) == 'a') && isspace(*(buf + 1)))
		return 0;
	if ((tolower(*buf) == 's') && isspace(*(buf + 1))) {
		chat_sendmsg(buf);
		return 1;
	} else
		for (i = 0; chat_cmdtbl[i].cmdname; i++) {
			if (*buf != '\0' && chat_cmd_match(buf, chat_cmdtbl[i].cmdname)) {
				chat_cmdtbl[i].cmdfunc(buf);
				return 1;
			}
		}
	return 0;
}
