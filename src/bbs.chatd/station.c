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
#include "chat.h"

#ifdef LINUX
#include <unistd.h>
#endif

#ifdef AIX
#include <sys/select.h>
#endif

#if !RELIABLE_SELECT_FOR_WRITE
#include <fcntl.h>
#endif

#if USES_SYS_SELECT_H
#include <sys/select.h>
#endif

#if NO_SETPGID
#define setpgid setpgrp
#endif

#ifndef L_XTND
#define L_XTND          2	/* relative to end of file */
#endif

#define RESTRICTED(u)   (users[(u)].flags == 0)	/* guest */
#define SYSOP(u)        (users[(u)].flags & PERM_SYSOP)
#define ROOMOP(u)       (users[(u)].flags & PERM_CHATROOM)
#define OUTSIDER(u)     (users[(u)].flags & PERM_DENYSIG)
#define WWW_USER(u)     (users[(u)].utent == -2)
#define CHATOP(u)       (users[(u)].flags & PERM_NOZAP)

/* add by cityhunter on 3.14 *//* rm by ylsdd */
//#define CHATOP(u)       (users[(u)].flags & PERM_SPECIAL1)

#define ROOM_LOCKED     0x1
#define ROOM_SECRET     0x2
#define ROOM_NOEMOTE    0x4
#define CLOAK(u)        (users[(u)].flags & PERM_SEECLOAK)
#define LOCKED(r)       (rooms[(r)].flags & ROOM_LOCKED)
#define SECRET(r)       (rooms[(r)].flags & ROOM_SECRET)
#define NOEMOTE(r)      (rooms[(r)].flags & ROOM_NOEMOTE)

#define ROOM_ALL        (-2)
#define PERM_CHATROOM PERM_CHAT

char *CHATROOM_TOPIC[4] = {
	"今天我们要讨论的是...",
	"彩虹桥上好谈心...",
	"最近会有什么活动呢?",
	"又到群龙聚首的日子了..."
};

struct chatuser {
	int sockfd;		/* socket to bbs server */
	int utent;		/* utable entry for this user */
	int room;		/* room: -1 means none, 0 means main */
	int flags;
	char cloak;
	char userid[IDLEN + 2];	/* real userid */
	char chatid[CHAT_IDLEN];	/* chat id */
	char ibuf[128];		/* buffer for sending/receiving */
	int ibufsize;		/* current size of ibuf */
	char host[30];
} users[MAXACTIVE];

struct chatroom {
	char name[IDLEN];	/* name of room; room 0 is "main" */
	short occupants;	/* number of users in room */
	short flags;		/* ROOM_LOCKED, ROOM_SECRET */
	char invites[MAXACTIVE];	/* Keep track of invites to rooms */
	char topic[48];		/* Let the room op to define room topic */
} rooms[MAXROOM];

struct chatcmd {
	char *cmdstr;
	void (*cmdfunc) (int, char *);
	int exact;
};

int chatroom, chatport;
int sock = -1;			/* the socket for listening */
int nfds;			/* number of sockets to select on */
int num_conns;			/* current number of connections */
fd_set allfds;			/* fd set for selecting */
struct timeval zerotv;		/* timeval for selecting */
char chatbuf[256];		/* general purpose buffer */
char chatname[19];

/* name of the main room (always exists) */

int chat_get_op(int rnum);
int chat_unset_op(int unum, int recunum);

char mainroom[] = "main";
char *maintopic = "今天，我们要讨论的是.....";

char *msg_not_op     = "\033[1;37m★\033[32m您不是这厢房的老大\033[37m ★\033[m";
char *msg_no_such_id = "\033[1;37m★\033[32m [\033[36m%s\033[32m] 不在这间厢房里\033[37m ★\033[m";
char *msg_not_here   = "\033[1;37m★\033[32m [\033[36m%s\033[32m] 并没有前来本会议厅\033[37m ★\033[m";

#define HAVE_REPORT

#ifdef  HAVE_REPORT
void
report(s)
char *s;
{
	static int disable = NA;
	int fd;

	if (disable)
		return;
	if ((fd = open("trace.chatd", O_WRONLY, 0644)) != -1) {
		char buf[160];
		flock(fd, LOCK_EX);
		lseek(fd, 0, L_XTND);
		sprintf(buf, "%s\n", s);
		write(fd, buf, strlen(buf));
		flock(fd, LOCK_UN);
		close(fd);
		return;
	}
	disable = YEA;
	return;
}
#else
#define report(s)       ;
#endif

#ifdef WWW_CHAT
/* define for WWW Chat password attempt */
#define BADLOGINFILE    "logins.bad"
char *
sethomefile(char *buf, char *userid, char *filename)
{
	sprintf(buf, MY_BBS_HOME "/home/%c/%s/%s", mytoupper(userid[0]), userid, filename);
	return buf;
}

void
logattempt(char *uid, char *frm)
{
	char fname[STRLEN];
	int fd, len;
	char genbuf[255];
	time_t now = time(0);
	sprintf(genbuf, "%-12.12s  %-30s %s  WWW-CHAT\n", uid, ctime(&now), frm);
	len = strlen(genbuf);
	if ((fd = open(MY_BBS_HOME "/" BADLOGINFILE, O_WRONLY | O_CREAT | O_APPEND, 0644)) > 0) {
		write(fd, genbuf, len);
		close(fd);
	}
	sethomefile(fname, uid, BADLOGINFILE);
	if ((fd = open(fname, O_WRONLY | O_CREAT | O_APPEND, 0644)) > 0) {
		write(fd, genbuf, len);
		close(fd);
	}
}

char *crypt();

int
checkpasswd(char *passwd, char *test)
{
	static char pwbuf[14];
	char *pw;

	strncpy(pwbuf, test, 14);
	pw = crypt(pwbuf, passwd);
	return (!strcmp(pw, passwd));
}

int
check_userpasswd(char *user, char *passwd)
{
	FILE *rec;
	int found = 0;
	char buf[256];
	struct userec currentuser;
	if ((rec = fopen(MY_BBS_HOME "/" PASSFILE, "rb")) == NULL)
		return -1;
	while (1) {
		if (fread(&currentuser, sizeof (currentuser), 1, rec) <= 0)
			break;
		if (currentuser.numlogins <= 0)
			continue;
		if (strcasecmp(user, currentuser.userid))
			continue;
		else {
			found = 1;
			break;
		}
	}
	fclose(rec);
	if (!found)
		return -1;
	if (checkpasswd(currentuser.passwd, passwd))
		return 1;
	else
		return -2;
}
#endif
int
is_valid_chatid(char *id)
{
	int i;
	if (*id == '\0')
		return 0;

	for (i = 0; i < CHAT_IDLEN && *id; i++, id++)
		if (strchr(BADCIDCHARS, *id))
			return 0;

	return 1;
}

long Isspace(char ch)
{
	return (long) strchr(" \t\n\r", ch);
}

char *
nextword(char **str)
{
	char *head, *tail, ch;

	head = *str;
	for (;;) {
		ch = *head;
		if (!ch) {
			*str = head;
			return head;
		}
		if (!Isspace(ch))
			break;
		head++;
	}

	tail = head + 1;
	while ((ch = *tail) != 0) {
		if (Isspace(ch)) {
			*tail++ = '\0';
			break;
		}
		tail++;
	}
	*str = tail;

	return head;
}

int
chatid_to_indx(int unum, char *chatid)
{
	register int i;
	for (i = 0; i < MAXACTIVE; i++) {
		if (users[i].sockfd == -1)
			continue;
		if (!strcasecmp(chatid, users[i].chatid)) {
			if (users[i].cloak == 0 || !CLOAK(unum))
				return i;
		}
	}
	return -1;
}

int
fuzzy_chatid_to_indx(int unum, char *chatid)
{
	register int i, indx = -1;
	unsigned int len = strlen(chatid);
	for (i = 0; i < MAXACTIVE; i++) {
		if (users[i].sockfd == -1)
			continue;
		if (!strncasecmp(chatid, users[i].chatid, len) || !strncasecmp(chatid, users[i].userid, len)) {
			if (len == strlen(users[i].chatid) || len == strlen(users[i].userid)) {
				indx = i;
				break;
			}
			if (indx == -1)
				indx = i;
			else
				return -2;
		}
	}
	if (users[indx].cloak == 0 || CLOAK(unum))
		return indx;
	else
		return -1;
}

int
roomid_to_indx(char *roomid)
{
	register int i;
	for (i = 0; i < MAXROOM; i++) {
		if (i && rooms[i].occupants == 0)
			continue;
		report(roomid);
		report(rooms[i].name);
		if (!strcasecmp(roomid, rooms[i].name))
			return i;
	}
	return -1;
}

void
do_send(fd_set *writefds, char *str)
{
	register int i;
	int len = strlen(str);

	if (select(nfds, NULL, writefds, NULL, &zerotv) > 0) {
		for (i = 0; i < nfds; i++)
			if (FD_ISSET(i, writefds))
				send(i, str, len + 1, 0);
	}
}

void
send_to_room(int room, char *str)
{
	int i;
	fd_set writefds;

	FD_ZERO(&writefds);
	for (i = 0; i < MAXACTIVE; i++) {
		if (users[i].sockfd == -1)
			continue;
		if (room == ROOM_ALL || room == users[i].room)
			/* write(users[i].sockfd, str, strlen(str) + 1); */
			FD_SET(users[i].sockfd, &writefds);
	}
	do_send(&writefds, str);
}

void
send_to_unum(int unum, char *str)
{
	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(users[unum].sockfd, &writefds);
	do_send(&writefds, str);
}

#ifdef WWW_CHAT
void
send_www_user(int room, char *str)
{
	int i;
	fd_set writefds;
	FD_ZERO(&writefds);
	for (i = 0; i < MAXACTIVE; i++) {
		if (users[i].sockfd == -1)
			continue;
		if (!WWW_USER(i))
			continue;
		if (room == ROOM_ALL || room == users[i].room)
			/* write(users[i].sockfd, str, strlen(str) + 1); */
			FD_SET(users[i].sockfd, &writefds);
	}
	do_send(&writefds, str);
}

void
www_user_list(int unum, int rnum)
{
	int i, c, myroom, urnum;
	if (unum == -1)
		send_www_user(rnum, "/az");
	else
		send_to_unum(unum, "/az");
	for (i = 0, c = 0; i < MAXACTIVE; i++) {
		urnum = users[i].room;
		if (users[i].sockfd != -1 && rnum != -1 && !(users[i].cloak == 1)) {
			if (rnum == urnum) {
				sprintf(chatbuf, "/a+%c%-8s  %-12s", (ROOMOP(i)) ? '*' : ' ', users[i].chatid, users[i].userid);
				if (unum == -1)
					send_www_user(rnum, chatbuf);
				else
					send_to_unum(unum, chatbuf);
			}
		}
	}
}

void
www_room_list(int unum)
{
	int i, occupants;
	if (unum == -1)
		send_www_user(ROOM_ALL, "/bz");
	else
		send_to_unum(unum, "/bz");
	for (i = 0; i < MAXROOM; i++) {
		occupants = rooms[i].occupants;
		if (occupants > 0) {
			if ((rooms[i].flags & ROOM_SECRET) && (unum != -1) && (users[unum].room != i))
				continue;
			sprintf(chatbuf, "/b+%-12s%4d  %s", rooms[i].name, occupants, rooms[i].topic);
			if (rooms[i].flags & ROOM_LOCKED)
				strcat(chatbuf, " [锁住]");
			if (rooms[i].flags & ROOM_SECRET)
				strcat(chatbuf, " [机密]");
			if (rooms[i].flags & ROOM_NOEMOTE)
				strcat(chatbuf, " [禁止动作]");
			if (unum == -1)
				send_www_user(ROOM_ALL, chatbuf);
			else
				send_to_unum(unum, chatbuf);
		}
	}
}
#endif

void
exit_room(int unum, int disp, char *msg)
{
	int oldrnum = users[unum].room;

	if (oldrnum != -1) {
		if (--rooms[oldrnum].occupants) {
			switch (disp) {
			case EXIT_LOGOUT:
				sprintf(chatbuf, "\033[1;37m★ \033[32m[\033[36m%s\033[32m] 慢慢离开了 \033[37m★\033[m", users[unum].chatid);
				if (msg && *msg) {
					strcat(chatbuf, ": ");
					strncat(chatbuf, msg, 80);
				}
				break;

			case EXIT_LOSTCONN:
				sprintf(chatbuf, "\033[1;37m★ \033[32m[\033[36m%s\033[32m] 缓缓的离开了 \033[37m★\033[m", users[unum].chatid);
				break;

			case EXIT_KICK:
				sprintf(chatbuf, "\033[1;37m★ \033[32m[\033[36m%s\033[32m] 被老大赶出去了 \033[37m★\033[m", users[unum].chatid);
				break;
			}
			if (users[unum].cloak == 0)
				send_to_room(oldrnum, chatbuf);
		}
	}
	users[unum].flags &= ~PERM_CHATROOM;
	users[unum].room = -1;
#ifdef WWW_CHAT
	if (rooms[oldrnum].occupants == 0)
		www_room_list(-1);
	www_user_list(-1, oldrnum);
#endif
}

void
chat_topic(int unum, char *msg)
{
	int rnum;
	rnum = users[unum].room;

	if (!ROOMOP(unum) && !SYSOP(unum) && !CHATOP(unum)) {
		send_to_unum(unum, msg_not_op);
		return;
	}
	if (*msg == '\0') {
		send_to_unum(unum, "\033[1;31m◎ \033[37m请指定话题 \033[31m◎\033[m");
		return;
	}

	strncpy(rooms[rnum].topic, msg, 48);
	rooms[rnum].topic[47] = '\0';
	sprintf(chatbuf, "/t%.47s", msg);
	send_to_room(rnum, chatbuf);
	sprintf(chatbuf, "\033[1;37m★ \033[32m[\033[36m%s\033[32m] 将话题改为 \033[1;33m%s \033[37m★\033[m", users[unum].chatid, msg);
	send_to_room(rnum, chatbuf);
}

int
enter_room(int unum, char *room, char *msg)
{
	int rnum;
#ifdef WWW_CHAT
	int oldrnum;
#endif
	int op = 0;
	register int i;

	rnum = roomid_to_indx(room);
	if (rnum == -1) {
		/* new room */
/*    if (OUTSIDER(unum))
	{
		send_to_unum(unum, "\033[1;31m◎ \033[37m抱歉，您不能开新包厢 \033[31m◎\033[m");
		return 0;
	}*/
		for (i = 1; i < MAXROOM; i++) {
			if (rooms[i].occupants == 0) {
				report("new room");
				rnum = i;
				memset(rooms[rnum].invites, 0, MAXACTIVE);
				strcpy(rooms[rnum].topic, maintopic);
				strncpy(rooms[rnum].name, room, IDLEN - 1);
				rooms[rnum].name[IDLEN - 1] = '\0';
				rooms[rnum].flags = 0;
				op++;
				break;
			}
		}
		if (rnum == -1) {
			send_to_unum(unum, "\033[1;31m◎ \033[37m我们的房间满了喔 \033[31m◎\033[m");
			return 0;
		}
	}
	if (!SYSOP(unum) && !CHATOP(unum)) {
		if (LOCKED(rnum) && rooms[rnum].invites[unum] == 0) {
			send_to_unum(unum, "\033[1;31m◎ \033[37m本房商讨机密中，非请勿入 \033[31m◎\033[m");
			return 0;
		}
	}

#ifdef WWW_CHAT
	oldrnum = users[unum].room;
#endif
	exit_room(unum, EXIT_LOGOUT, msg);
	users[unum].room = rnum;
	if (op)
		users[unum].flags |= PERM_CHATROOM;
	rooms[rnum].occupants++;
	rooms[rnum].invites[unum] = 0;
	if (users[unum].cloak == 0) {
		sprintf(chatbuf, "\033[1;31m□ \033[37m[\033[36;1m%s \033[34;1m(\033[33;1m %s \033[34;1m)\033[37m] 进入 \033[35m%s \033[37m包厢 \033[31m□\033[m", users[unum].chatid, users[unum].userid, rooms[rnum].name);
		send_to_room(rnum, chatbuf);
	}
	sprintf(chatbuf, "/r%s", room);
	send_to_unum(unum, chatbuf);
	sprintf(chatbuf, "/t%s", rooms[rnum].topic);
	send_to_unum(unum, chatbuf);
#ifdef WWW_CHAT
	if ((rnum == oldrnum) || ((rnum == 0) && (rooms[0].occupants == 1)) || (op) || (rooms[oldrnum].occupants == 0))
		www_room_list(-1);
	www_user_list(-1, oldrnum);
	www_user_list(-1, rnum);
#endif
	return 0;
}

void
logout_user(int unum)
{
	int i, sockfd = users[unum].sockfd;

	close(sockfd);
	FD_CLR(sockfd, &allfds);
	memset(&users[unum], 0, sizeof (users[unum]));
	users[unum].sockfd = users[unum].utent = users[unum].room = -1;
	for (i = 0; i < MAXROOM; i++) {
		if (rooms[i].invites[unum] != 0)
			rooms[i].invites[unum] = 0;
	}
	num_conns--;
}

int
print_user_counts(int unum)
{
	int i, c, userc = 0, suserc = 0, roomc = 0;
	for (i = 0; i < MAXROOM; i++) {
		c = rooms[i].occupants;
		if (i > 0 && c > 0) {
			if (!SECRET(i) || SYSOP(unum) || CHATOP(unum))
				roomc++;
		}
		c = users[i].room;
		if (users[i].sockfd != -1 && c != -1 && users[i].cloak == 0) {
			if (SECRET(c) && !SYSOP(unum) && !CHATOP(unum))
				suserc++;
			else
				userc++;
		}
	}
	sprintf(chatbuf, "\033[1;31m□\033[37m 欢迎A光临『\033[32m%s\033[37m』的【\033[36m%s\033[37m】\033[31m□\033[m", MY_BBS_NAME, chatname);
	send_to_unum(unum, chatbuf);
	sprintf(chatbuf, "\033[1;31m□\033[37m 目前已经有 \033[1;33m%d \033[37m间会议室有客人 \033[31m□\033[m", roomc + 1);
	send_to_unum(unum, chatbuf);
	sprintf(chatbuf, "\033[1;31m□ \033[37m本会议厅内共有 \033[36m%d\033[37m 人 ", userc + 1);
	if (suserc)
		sprintf(chatbuf + strlen(chatbuf), "[\033[36m%d\033[37m 人在高机密讨论室]", suserc);
	sprintf(chatbuf + strlen(chatbuf), "\033[31m□\033[m");
	send_to_unum(unum, chatbuf);
	return 0;
}

int
login_user(int unum, char *msg)
{
	int i, utent;		//, fd = users[unum].sockfd;
	char *utentstr;
	char *level;
	char *userid;
	char *chatid;
	char *cloak;

	utentstr = nextword(&msg);
	level = nextword(&msg);
	userid = nextword(&msg);
	chatid = nextword(&msg);
	cloak = nextword(&msg);

	utent = atoi(utentstr);
#ifdef WWW_CHAT
	if (utent <= -2) {
		utent = -2;
		passwd = nextword(&msg);
		if ((i = check_userpasswd(userid, passwd)) != 1) {
			if (i == -2)
				logattempt(userid, users[unum].host);
			send_to_unum(unum, CHAT_LOGIN_BADPASS);
			return -1;
		}
	}
#endif

	for (i = 0; i < MAXACTIVE; i++) {
		if (users[i].sockfd == -1)
			continue;
/*    if(utent == -2)      only CHAT1 allows remote user */
		{
			if (!strcasecmp(userid, users[i].userid)) {
				send_to_unum(unum, CHAT_LOGIN_BOGUS);
				return -1;
			}
		}
/*    else if (users[i].utent == utent) {
		send_to_unum(unum, CHAT_LOGIN_BOGUS);
		return -1;
	}*/
	}
	if (!is_valid_chatid(chatid)) {
		send_to_unum(unum, CHAT_LOGIN_INVALID);
		return 0;
	}
	if (chatid_to_indx(0, chatid) != -1) {
		/* userid in use */
		send_to_unum(unum, CHAT_LOGIN_EXISTS);
		return 0;
	}
	if (!strncasecmp("localhost" /*MY_BBS_DOMAIN */ , users[unum].host, 30)) {
		users[unum].flags = atoi(level) & ~(PERM_DENYSIG);
		users[unum].cloak = atoi(cloak);
	} else {
		if (chatport != CHATPORT1) {	/* only CHAT1 allows remote user */
			send_to_unum(unum, CHAT_LOGIN_BOGUS);
			return -1;
		} else {
			if (!(atoi(level) & PERM_LOGINOK) && !strncasecmp(chatid, "guest", 8)) {
				send_to_unum(unum, CHAT_LOGIN_INVALID);
				return 0;
			}
			users[unum].flags = PERM_DENYSIG;
			users[unum].cloak = 0;
		}
	}
	report(level);
	report(users[unum].host);

	if (utent == -2)
		users[unum].flags |= PERM_SPECIAL8;
	users[unum].utent = utent;
	strcpy(users[unum].userid, userid);
	strncpy(users[unum].chatid, chatid, CHAT_IDLEN - 1);
	users[unum].chatid[CHAT_IDLEN - 1] = '\0';
	send_to_unum(unum, CHAT_LOGIN_OK);
	print_user_counts(unum);
	enter_room(unum, mainroom, (char *) NULL);
	return 0;
}

void
chat_list_rooms(int unum, char *msg)
{
	(void) msg;
	int i, occupants;
	if (RESTRICTED(unum)) {
		send_to_unum(unum, "\033[1;31m◎ \033[37m抱歉！老大不让你看有哪些房间有客人 \033[31m◎\033[m");
		return;
	}
	send_to_unum(unum, "\033[1;33;44m 谈天室名称  │人数│话题        \033[m");
	for (i = 0; i < MAXROOM; i++) {
		occupants = rooms[i].occupants;
		if (occupants > 0) {
			if (!SYSOP(unum) && !CHATOP(unum))
				if ((rooms[i].flags & ROOM_SECRET) && (users[unum].room != i))
					continue;
			sprintf(chatbuf, " \033[1;32m%-12s\033[37m│\033[36m%4d\033[37m│\033[33m%s\033[m", rooms[i].name, occupants, rooms[i].topic);
			if (rooms[i].flags & ROOM_LOCKED)
				strcat(chatbuf, " [锁住]");
			if (rooms[i].flags & ROOM_SECRET)
				strcat(chatbuf, " [机密]");
			if (rooms[i].flags & ROOM_NOEMOTE)
				strcat(chatbuf, " [禁止动作]");
			send_to_unum(unum, chatbuf);
		}
	}
}

int
chat_do_user_list(int unum, char *msg, int whichroom)
{
	int start, stop, curr = 0;
	int i, rnum, myroom = users[unum].room;
	while (*msg && Isspace(*msg))
		msg++;
	start = atoi(msg);
	while (*msg && isdigit(*msg))
		msg++;
	while (*msg && !isdigit(*msg))
		msg++;
	stop = atoi(msg);
	send_to_unum(unum, "\033[1;33;44m 聊天代号│使用者代号  │聊    天    室┃Op┃来自                          \033[m");
	for (i = 0; i < MAXACTIVE; i++) {
		rnum = users[i].room;
		if (users[i].sockfd != -1 && rnum != -1 && !(users[i].cloak == 1 && !CLOAK(unum))) {
			if (whichroom != -1 && whichroom != rnum)
				continue;
			if (myroom != rnum) {
				if (RESTRICTED(unum))
					continue;
				if ((rooms[rnum].flags & ROOM_SECRET) && !SYSOP(unum) && !CHATOP(unum))
					continue;
			}
			curr++;
			if (curr < start)
				continue;
			else if (stop && (curr > stop))
				break;
			sprintf(chatbuf,
				"\033[1;5m%c\033[0;1;37m%-8s│\033[31m%s%-12s\033[37m│\033[32m%-14s\033[37m┃\033[34m%-2s\033[37m┃\033[35m%-30s\033[m",
				(users[i].cloak == 1) ? 'C' : ' ',
				users[i].chatid,
				WWW_USER(i) ? "\033[1;35m" : "\033[1;36m",
				users[i].userid, rooms[rnum].name,
				ROOMOP(i) ? "是" : "否", users[i].host);
			send_to_unum(unum, chatbuf);
		}
	}
	return 0;
}

void
chat_list_by_room(int unum, char *msg)
{
	int whichroom;
	char *roomstr;

	roomstr = nextword(&msg);
	if (*roomstr == '\0')
		whichroom = users[unum].room;
	else {
		if ((whichroom = roomid_to_indx(roomstr)) == -1) {
			sprintf(chatbuf, "\033[1;31m◎ \033[37m没 %s 这个房间喔 \033[31m◎\033[m", roomstr);
			send_to_unum(unum, chatbuf);
			return;
		}
		if ((rooms[whichroom].flags & ROOM_SECRET) && !SYSOP(unum) && !CHATOP(unum)) {
			send_to_unum(unum, "\033[1;31m◎ \033[37m本会议厅的房间皆公开的，没有秘密 \033[31m◎\033[m");
			return;
		}
	}
	chat_do_user_list(unum, msg, whichroom);
}

void
chat_list_users(int unum, char *msg)
{
	chat_do_user_list(unum, msg, -1);
}

void
chat_map_chatids(int unum, int whichroom)
{
	int i, c, myroom, rnum;
	myroom = users[unum].room;
	send_to_unum(unum, "\033[1;33;44m 聊天代号 使用者代号  │ 聊天代号 使用者代号  │ 聊天代号 使用者代号 \033[m");
	for (i = 0, c = 0; i < MAXACTIVE; i++) {
		rnum = users[i].room;
		if (users[i].sockfd != -1 && rnum != -1 && !(users[i].cloak == 1 && !CLOAK(unum))) {
			if (whichroom != -1 && whichroom != rnum)
				continue;
			if (myroom != rnum) {
				if (RESTRICTED(unum))
					continue;
				if ((rooms[rnum].flags & ROOM_SECRET) && !SYSOP(unum) && !CHATOP(unum))
					continue;
			}
			sprintf(chatbuf + (c * 50),
				"\033[1;34;5m%c\033[m\033[1m%-8s%c%s%-12s%s\033[m",
				(users[i].cloak == 1) ? 'C' : ' ',
				users[i].chatid, (ROOMOP(i)) ? '*' : ' ',
				OUTSIDER(i) ? "\033[1;35m" : "\033[1;36m",
				users[i].userid, (c < 2 ? "│" : "  "));
			if (++c == 3) {
				send_to_unum(unum, chatbuf);
				c = 0;
			}
		}
	}
	if (c > 0)
		send_to_unum(unum, chatbuf);
}

void
chat_map_chatids_thisroom(int unum, char *msg)
{
	(void) msg;
	chat_map_chatids(unum, users[unum].room);
}

void
chat_setroom(int unum, char *msg)
{
	char *modestr;
	int rnum = users[unum].room;
	int sign = 1;
	int flag;
	char *fstr;

	modestr = nextword(&msg);
	if (!ROOMOP(unum) && !SYSOP(unum) && !CHATOP(unum)) {
		send_to_unum(unum, msg_not_op);
		return;
	}
	if (*modestr == '+')
		modestr++;
	else if (*modestr == '-') {
		modestr++;
		sign = 0;
	}
	if (*modestr == '\0') {
		send_to_unum(unum, "\033[1;31m⊙ \033[37m请告诉柜台您要的房间是: {[\033[32m+\033[37m(设定)][\033[32m-\033[37m(取消)]}{[\033[32ml\033[37m(锁住)][\033[32ms\033[37m(秘密)]}\033[m");
		return;
	}
	while (*modestr) {
		flag = 0;
		switch (*modestr) {
		case 'l':
		case 'L':
			flag = ROOM_LOCKED;
			fstr = "锁住";
			break;
		case 's':
		case 'S':
			flag = ROOM_SECRET;
			fstr = "机密";
			break;
		case 'e':
		case 'E':
			flag = ROOM_NOEMOTE;
			fstr = "禁止动作";
			break;
		default:
			sprintf(chatbuf, "\033[1;31m◎ \033[37m抱歉，看不懂你的意思：[\033[36m%c\033[37m] \033[31m◎\033[m", *modestr);
			send_to_unum(unum, chatbuf);
			return;
		}
		if (flag && ((rooms[rnum].flags & flag) != sign * flag)) {
			rooms[rnum].flags ^= flag;
			sprintf(chatbuf,
				"\033[1;37m★\033[32m 这房间被 %s %s%s的形式 \033[37m★\033[m",
				users[unum].chatid, sign ? "设定为" : "取消",
				fstr);
			send_to_room(rnum, chatbuf);
		}
		modestr++;
	}
}

void
chat_nick(int unum, char *msg)
{
	char *chatid;
	int othernum;

	chatid = nextword(&msg);
	chatid[CHAT_IDLEN - 1] = '\0';
	if (!is_valid_chatid(chatid)) {
		send_to_unum(unum, "\033[1;31m◎ \033[37m您的名字是不是写错了\033[31m ◎\033[m");
		return;
	}
	othernum = chatid_to_indx(0, chatid);
	if (othernum != -1 && othernum != unum) {
		send_to_unum(unum, "\033[1;31m◎ \033[37m抱歉！有人跟你同名，所以你不能进来 \033[31m◎\033[m");
		return;
	}
	sprintf(chatbuf, "\033[1;31m◎ \033[36m%s \033[0;37m已经改名为 \033[1;33m%s \033[31m◎\033[m", users[unum].chatid, chatid);
	send_to_room(users[unum].room, chatbuf);
	strcpy(users[unum].chatid, chatid);
	sprintf(chatbuf, "/n%s", users[unum].chatid);
	send_to_unum(unum, chatbuf);
#ifdef WWW_CHAT
	www_user_list(-1, users[unum].room);
#endif
}

void
chat_private(int unum, char *msg)
{
	char *recipient;
	int recunum;

	recipient = nextword(&msg);
	recunum = fuzzy_chatid_to_indx(unum, recipient);
	if (recunum < 0) {
		/* no such user, or ambiguous */
		if (recunum == -1)
			sprintf(chatbuf, msg_no_such_id, recipient);
		else
			sprintf(chatbuf, "\033[1;31m ◎\033[37m 那位参与者叫什么名字 \033[31m◎\033[m");
		send_to_unum(unum, chatbuf);
		return;
	}
	if (*msg) {
		sprintf(chatbuf, "\033[1;32m ※ \033[36m%s \033[37m传纸条小秘书来到\033[m: ", users[unum].chatid);
		strncat(chatbuf, msg, 80);
		send_to_unum(recunum, chatbuf);
		sprintf(chatbuf, "\033[1;32m ※ \033[37m纸条已经交给了 \033[36m%s\033[m: ", users[recunum].chatid);
		strncat(chatbuf, msg, 80);
		send_to_unum(unum, chatbuf);
	} else {
		sprintf(chatbuf, "\033[1;31m ◎\033[37m 你要跟对方说些什么呀？\033[31m◎\033[m");
		send_to_unum(unum, chatbuf);
	}
}

void
put_chatid(int unum, char *str)
{
	int i;
	char *chatid = users[unum].chatid;
	memset(str, ' ', 10);
	for (i = 0; chatid[i]; i++)
		str[i] = chatid[i];
	str[i] = ':';
	str[10] = '\0';
}

int
chat_allmsg(int unum, char *msg)
{
	register int i;
	char c;
	int color;
	char colorstr[10];
	if (*msg) {
		put_chatid(unum, chatbuf);
		i = strlen(chatbuf);
		while ((*msg != 0) && (i < 252)) {
			if (*msg == '%') {
				msg++;
				c = tolower(*msg);
				if (isdigit(*msg) || ((c >= 'a') && (c <= 'f'))) {
					if ((c >= 'a') && (c <= 'f'))
						color = c - 'a' + 10;
					else
						color = c - '0';
					if (color > 7)
						sprintf(colorstr, "\x1b[1;%dm", color + 30);
					else
						sprintf(colorstr, "\x1b[%dm", color + 30);
					if (strlen(colorstr + i) < 252) {
						chatbuf[i] = 0;
						strcat(chatbuf, colorstr);
						i += strlen(colorstr);
					} else
						break;
				} else {
					chatbuf[i++] = '%';
					if (i == 252)
						break;
					chatbuf[i++] = *msg;
				};
			} else
				chatbuf[i++] = *msg;
			msg++;
		};
		chatbuf[i] = 0;
		strcat(chatbuf, "\x1b[m");
/*    strcat(chatbuf, msg);*/
		send_to_room(users[unum].room, chatbuf);
	}
	return 0;
}

void
chat_act(int unum, char *msg)
{
	register int i;
	char c;
	int color;
	char colorstr[10];
	if (NOEMOTE(users[unum].room)) {
		send_to_unum(unum, "本聊天室禁止动作");
		return;
	};
	if (*msg) {
/*    sprintf(chatbuf, "\033[1;36m%s \033[37m%s\033[m", users[unum].chatid, msg); */
		sprintf(chatbuf, "\x1b[1;36m%s \x1b[1;33m\x1b[m", users[unum].chatid);
		i = strlen(chatbuf);
		while ((*msg != 0) && (i < 252)) {
			if (*msg == '%') {
				msg++;
				c = tolower(*msg);
				if (isdigit(*msg) || ((c >= 'a') && (c <= 'f'))) {
					if ((c >= 'a') && (c <= 'f'))
						color = c - 'a' + 10;
					else
						color = c - '0';
					if (color > 7)
						sprintf(colorstr, "\x1b[1;%dm", color + 30);
					else
						sprintf(colorstr, "\x1b[%dm", color + 30);
					if (strlen(colorstr + i) < 252) {
						chatbuf[i] = 0;
						strcat(chatbuf, colorstr);
						i += strlen(colorstr);
					} else
						break;
				} else {
					chatbuf[i++] = '%';
					if (i == 252)
						break;
					chatbuf[i++] = *msg;
				};
			} else
				chatbuf[i++] = *msg;
			msg++;
		};
		chatbuf[i] = 0;
		strcat(chatbuf, "\x1b[m");
		send_to_room(users[unum].room, chatbuf);
	}
}

void
chat_cloak(int unum, char *msg)
{
	(void) msg;
	if (CLOAK(unum)) {
		if (users[unum].cloak == 1)
			users[unum].cloak = 0;
		else
			users[unum].cloak = 1;
		sprintf(chatbuf, "\033[1;36m%s \033[37m%s 隐身状态...\033[m",
			users[unum].chatid,
			(users[unum].cloak == 1) ? "进入" : "停止");
		send_to_unum(unum, chatbuf);
	}
}

void
chat_join(int unum, char *msg)
{
	char *roomid;

	roomid = nextword(&msg);
	if (RESTRICTED(unum)) {
		send_to_unum(unum, "\033[1;31m◎ \033[37m你只能在这里聊天 \033[31m◎\033[m");
		return;
	}
	if (*roomid == '\0') {
		send_to_unum(unum, "\033[1;31m◎ \033[37m请问哪个房间 \033[31m◎\033[m");
		return;
	}
	enter_room(unum, roomid, msg);
	return;
}

void
chat_kick(int unum, char *msg)
{
	char *twit;
	int rnum = users[unum].room;
	int recunum;

	twit = nextword(&msg);
	if (!ROOMOP(unum) && !SYSOP(unum) && !CHATOP(unum)) {
		send_to_unum(unum, msg_not_op);
		return;
	}
	if ((recunum = chatid_to_indx(unum, twit)) == -1) {
		sprintf(chatbuf, msg_no_such_id, twit);
		send_to_unum(unum, chatbuf);
		return;
	}
	if (rnum != users[recunum].room) {
		sprintf(chatbuf, msg_not_here, users[recunum].chatid);
		send_to_unum(unum, chatbuf);
		return;
	}
	exit_room(recunum, EXIT_KICK, (char *) NULL);

	if (rnum == 0)
		logout_user(recunum);
	else
		enter_room(recunum, mainroom, (char *) NULL);
}

void
chat_makeop(int unum, char *msg)
{
	char *newop = nextword(&msg);
	int rnum = users[unum].room;
	int recunum;

	if (!ROOMOP(unum) && !SYSOP(unum) && !CHATOP(unum)) {
		send_to_unum(unum, msg_not_op);
		return;
	}
	if ((recunum = chatid_to_indx(unum, newop)) == -1) {
		/* no such user */
		sprintf(chatbuf, msg_no_such_id, newop);
		send_to_unum(unum, chatbuf);
		return;
	}
	if (unum == recunum) {
		sprintf(chatbuf, "\033[1;37m★ \033[32m你忘了你本来就是老大喔 \033[37m★\033[m");
		send_to_unum(unum, chatbuf);
		return;
	}
	if (rnum != users[recunum].room) {
		sprintf(chatbuf, msg_not_here, users[recunum].chatid);
		send_to_unum(unum, chatbuf);
		return;
	}
	users[unum].flags &= ~PERM_CHATROOM;
	users[recunum].flags |= PERM_CHATROOM;
	sprintf(chatbuf, "\033[1;37m★ \033[36m %s\033[32m决定让 \033[35m%s \033[32m当老大 \033[37m★\033[m",
		users[unum].chatid, users[recunum].chatid);
	send_to_room(rnum, chatbuf);
#ifdef WWW_CHAT
	www_user_list(-1, rnum);
#endif
}

/*---   Add by Zhangwei 1999-08-14              ---*/
void
chat_clear_ops(int unum, char *s)
{
	(void) s;
	int rnum, recnum;
	if (!SYSOP(unum))
		return;			/*--- Only SYSOPs can do this!!! ---*/
	rnum = users[unum].room;
	while (1) {
		recnum = chat_get_op(rnum);
		if (-1 == recnum)
			break;
		if (1 != chat_unset_op(unum, recnum))
			break;
	}
	send_to_unum(unum, "\033[1;37m★ \033[33m群龙无首啊！\033[37m ★\033[m");
}

int
chat_unset_op(int unum, int recunum)
{
	int act, rnum;
	rnum = users[unum].room;
	if (rnum != users[recunum].room)
		act = 0;
	else if (!ROOMOP(recunum))
		act = 0;
	else {
		users[recunum].flags &= ~PERM_CHATROOM;
		act = 1;
	}
	return act;
}

int
chat_get_op(int rnum)
{
	int i, urnum;
	for (i = 0; i < MAXACTIVE; i++) {
		urnum = users[i].room;
		if (users[i].sockfd != -1 && urnum != -1) {
/*                              && !(users[i].cloak==1 && !CLOAK(unum) ) ) { */
			/*---   对SYSOP操作会有问题！！！     1999-09-04 ---*/
			if (!strcmp(users[i].userid, "SYSOP"))
				continue;
			if (rnum == -1 || rnum != urnum)
				continue;	/* && */
			if (ROOMOP(i))
				return i;
		}
	}
	return -1;
}

void
chat_knock_room(int unum, char *msg)
{
	char *roomid;
	const char *outd = "里面";
	const char *ind = "外面";
	int rnum;

	roomid = nextword(&msg);
	if (RESTRICTED(unum)) {
		send_to_unum(unum, "\033[1;31m◎ \033[37m你只能在这里聊天 \033[31m◎\033[m");
		return;
	}
	if (*roomid == '\0') {
		send_to_unum(unum, "\033[1;31m◎ \033[37m请问哪个房间 \033[31m◎\033[m");
		return;
	}
	rnum = roomid_to_indx(roomid);
	if (rnum == -1) {
		send_to_unum(unum, "\033[1;31m◎ \033[37m 没有这个房间 \033[31m◎\033[m");
		return;
	}
	if ((SECRET(rnum) || NOEMOTE(rnum)) && !SYSOP(unum) && !CHATOP(unum)) {
		send_to_unum(unum, "\033[1;31m◎ \033[37m 请勿打扰，谢谢合作！ 记得要乖哦~~:PP \033[31m◎\033[m");
		return;
	}
	sprintf(chatbuf, "\033[1;37m◎ \033[31m当当当...  \033[33m%s [%s] \033[37m在%s敲门: \033[32m%s \033[37m◎\033[m", users[unum].chatid, users[unum].userid, (rnum == users[unum].room) ? (outd) : (ind), (msg));
	send_to_room(rnum, chatbuf);
	if (rnum != users[unum].room)
		send_to_unum(unum, chatbuf);
	return;
}

/* ----- End of Add by Zhangwei ----- */

/* ----- Add by LiuQixin 1999.11.2 ----- */
void
chat_clear_op(int unum, char *msg)
{
	char *newop = nextword(&msg);
	int rnum = users[unum].room;
	int recunum;

	if (!SYSOP(unum) && !CHATOP(unum))
		return;		/* only SYSOP can do this */
	if ((recunum = chatid_to_indx(unum, newop)) == -1) {
		/* no such user */
		sprintf(chatbuf, msg_no_such_id, newop);
		send_to_unum(unum, chatbuf);
		return;
	}
	if (unum == recunum) {
		sprintf(chatbuf, "\033[1;37m★ \033[32m要自裁？何必呢？？何苦呢？？ \033[37m★\033[m");
		send_to_unum(unum, chatbuf);
		return;
	}
	if (rnum != users[recunum].room) {
		sprintf(chatbuf, msg_not_here, users[recunum].chatid);
		send_to_unum(unum, chatbuf);
		return;
	}
	if ((users[recunum].flags & PERM_CHATROOM) != PERM_CHATROOM) {
		sprintf(chatbuf, "\033[1;37m★ \033[32m奇怪，\033[35m%s \033[32m不是老大呀！\033[37m★\033[m",
			users[recunum].chatid);
	}
	users[recunum].flags &= ~PERM_CHATROOM;
	sprintf(chatbuf, "\033[1;37m★ \033[36m %s\033[32m决定撤了 \033[35m%s \033[32m的职 \033[37m★\033[m", users[unum].chatid, users[recunum].chatid);
	send_to_room(rnum, chatbuf);
}

/* ----- End of Add by LiuQixin ----- */

void
chat_invite(int unum, char *msg)
{
	char *invitee = nextword(&msg);
	int rnum = users[unum].room;
	int recunum;

	if (!ROOMOP(unum) && !SYSOP(unum)) {
		send_to_unum(unum, msg_not_op);
		return;
	}
	if ((recunum = chatid_to_indx(unum, invitee)) == -1) {
		sprintf(chatbuf, msg_not_here, invitee);
		send_to_unum(unum, chatbuf);
		return;
	}
	if (rooms[rnum].invites[recunum] == 1) {
		sprintf(chatbuf, "\033[1;37m★ \033[36m%s \033[32m等一下就来 \033[37m★\033[m", users[recunum].chatid);
		send_to_unum(unum, chatbuf);
		return;
	}
	rooms[rnum].invites[recunum] = 1;
	sprintf(chatbuf, "\033[1;37m★ \033[36m%s \033[32m邀请您到 [\033[33m%s\033[32m] 房间聊天\033[37m ★\033[m", users[unum].chatid, rooms[rnum].name);
	send_to_unum(recunum, chatbuf);
	sprintf(chatbuf, "\033[1;37m★ \033[36m%s \033[32m等一下就来 \033[37m★\033[m", users[recunum].chatid);
	send_to_unum(unum, chatbuf);
}

void
chat_broadcast(int unum, char *msg)
{
	if (!SYSOP(unum)) {
		send_to_unum(unum, "\033[1;31m◎ \033[37m你不可以在会议厅内大声喧哗 \033[31m◎\033[m");
		return;
	}
	if (*msg == '\0') {
		send_to_unum(unum, "\033[1;37m★ \033[32m广播内容是什么 \033[37m★\033[m");
		return;
	}
	sprintf(chatbuf, "\033[1m Ding Dong!! 传达室报告： \033[36m%s\033[37m 有话对大家宣布：\033[m", users[unum].chatid);
	send_to_room(ROOM_ALL, chatbuf);
	sprintf(chatbuf, "\033[1;34m【\033[33m%s\033[34m】\033[m", msg);
	send_to_room(ROOM_ALL, chatbuf);
}

void
chat_goodbye(int unum, char *msg)
{
	exit_room(unum, EXIT_LOGOUT, msg);
}

/* -------------------------------------------- */
/* MUD-like social commands : action             */
/* -------------------------------------------- */

struct action {
	char *verb;		/* 动词 */
	char *part1_msg;	/* 介词 */
	char *part2_msg;	/* 动作 */
};

struct action party_data[] = {
	{"?", "很疑惑的看着", ""},
	{"admire", "对", "的敬仰如滔滔江水，连绵不绝"},
	{"agree", "完全同意", "的看法"},
	{"bearhug", "热情的拥抱", ""},

	{"beg", "抱住", "的两腿，一把鼻涕一把泪地说：“行行好，救救俺这没钱的吧！”"},
	{"bless", "祝福", "心想事成"},
	{"bow", "毕躬毕敬的向", "鞠躬"},
	{"caress", "抚摸", ""},
	{"cat", "像只小猫般地依偎在", "的怀里撒娇。"},
	{"cringe", "向", "卑躬屈膝，摇尾乞怜"},
	{"cry", "向", "嚎啕大哭"},
	{"comfort", "温言安慰", ""},
	{"clap", "向", "热烈鼓掌"},
	{"dance", "拉了", "的手翩翩起舞"},
	{"drivel", "对著", "流口水"},
	{"dunno", "瞪大眼睛，天真地问：", "，你说什麽我不懂耶... :("},
	{"farewell", "含泪依依不舍地向", "道别"},
	{"faint", "晕倒在", "的怀里"},
	{"fear", "对", "露出怕怕的表情"},
	{"forgive", "大度的对", "说：算了，原谅你了"},
	{"giggle", "对著", "傻傻的呆笑"},
	{"grin", "对", "露出邪恶的笑容"},
	{"growl", "对", "咆哮不已"},
	{"hand", "跟", "握手"},

	{"heng", "看都不看", "一眼， 哼了一声，高高的把头扬起来了,不屑一顾的样子..."},
	{"hug", "轻轻地拥抱", ""},
	{"idiot", "无情地耻笑", "的痴呆。"},
	{"kick", "把", "踢的死去活来"},
	{"kiss", "轻吻", "的脸颊"},
	{"laugh", "大声嘲笑", ""},
	{"nod", "向", "点头称是"},
	{"nudge", "用手肘顶", "的肥肚子"},
	{"pad", "轻拍", "的肩膀"},
	{"papaya", "敲了敲", "的木瓜脑袋"},
	{"pinch", "用力的把", "拧的黑青"},
	{"punch", "狠狠揍了", "一顿"},
	{"pure", "对", "露出纯真的笑容"},
	{"report", "偷偷地对", "说：“报告我好吗？”"},
	{"shrug", "无奈地向", "耸了耸肩膀"},
	{"sigh", "对", "叹了一口气"},
	{"slap", "啪啪的巴了", "一顿耳光"},
	{"smooch", "拥吻著", ""},
	{"snicker", "嘿嘿嘿..的对", "窃笑"},
	{"sniff", "对", "嗤之以鼻"},
	{"spank", "用巴掌打", "的臀部"},
	{"squeeze", "紧紧地拥抱著", ""},
	{"thank", "向", "道谢"},
	{"tickle", "咕叽!咕叽!搔", "的痒"},
	{"waiting", "深情地对", "说：一年三百六十五天，每分每秒我都在这里等着你"},
	{"wave", "对著", "拼命的摇手"},
	{"welcome", "热烈欢迎", "的到来"},
	{"wink", "对", "神秘的眨眨眼睛"},
	{"xixi", "嘻嘻地对", "笑了几声"},
	{"zap", "对", "疯狂的攻击"},
	{NULL, NULL, NULL}
};

int
party_action(int unum, char *cmd, char *party)
{
	int i;
	for (i = 0; party_data[i].verb; i++) {
		if (!strcmp(cmd, party_data[i].verb)) {
			if (*party == '\0') {
				party = "大家";
			} else {
				int recunum = fuzzy_chatid_to_indx(unum, party);
				if (recunum < 0) {
					/* no such user, or ambiguous */
					if (recunum == -1)
						sprintf(chatbuf, msg_no_such_id, party);
					else
						sprintf(chatbuf, "\033[1;31m◎ \033[37m请问哪间房间 \033[31m◎\033[m");
					send_to_unum(unum, chatbuf);
					return 0;
				}
				party = users[recunum].chatid;
			}
			sprintf(chatbuf, "\033[1;36m%s \033[32m%s\033[33m %s \033[32m%s\033[37;0m", users[unum].chatid, party_data[i].part1_msg, party, party_data[i].part2_msg);
			send_to_room(users[unum].room, chatbuf);
			return 0;
		}
	}
	return 1;
}

/* -------------------------------------------- */
/* MUD-like social commands : speak              */
/* -------------------------------------------- */

struct action speak_data[] = {
	{"ask", "询问", NULL},
	{"chant", "歌颂", NULL},
	{"cheer", "喝采", NULL},
	{"chuckle", "轻笑", NULL},
	{"curse", "咒骂", NULL},
	{"demand", "要求", NULL},
	{"frown", "蹙眉", NULL},
	{"groan", "呻吟", NULL},
	{"grumble", "发牢骚", NULL},
	{"hum", "喃喃自语", NULL},
	{"moan", "悲叹", NULL},
	{"notice", "注意", NULL},
	{"order", "命令", NULL},
	{"ponder", "沈思", NULL},
	{"pout", "噘著嘴说", NULL},
	{"pray", "祈祷", NULL},
	{"request", "恳求", NULL},
	{"shout", "大叫", NULL},
	{"sing", "唱歌", NULL},
	{"smile", "微笑", NULL},
	{"smirk", "假笑", NULL},
	{"swear", "发誓", NULL},
	{"tease", "嘲笑", NULL},
	{"whimper", "呜咽的说", NULL},
	{"yawn", "哈欠连天", NULL},
	{"yell", "大喊", NULL},
	{NULL, NULL, NULL}
};

int
speak_action(int unum, char *cmd, char *msg)
{
	int i;

	for (i = 0; speak_data[i].verb; i++) {
		if (!strcmp(cmd, speak_data[i].verb)) {
			sprintf(chatbuf, "\033[1;36m%s \033[32m%s：\033[33m %s\033[37;0m", users[unum].chatid, speak_data[i].part1_msg, msg);
			send_to_room(users[unum].room, chatbuf);
			return 0;
		}
	}
	return 1;
}

/* -------------------------------------------- */
/* MUD-like social commands : condition          */
/* -------------------------------------------- */

struct action condition_data[] = {
	{":D", "乐的合不拢嘴", NULL},
	{"applaud", "啪啪啪啪啪啪啪....", NULL},
	{"blush", "脸都红了", NULL},
	{"cough", "咳了几声", NULL},
	{"faint", "咣当一声，晕倒在地上", NULL},
	{"happy", "r-o-O-m....听了真爽！", NULL},
	{"jump", "高兴的蹦蹦跳跳", NULL},
	{"luck", "哇！福气啦！", NULL},
	{"puke", "真恶心，我听了都想吐", NULL},
	{"shake", "摇了摇头", NULL},
	{"sleep", "Zzzzzzzzzz，真无聊，都快睡著了", NULL},
	{"slow", "快要慢死了。气得不住骂自己的计算机。", NULL},
	{"so", "就酱子!!", NULL},
	{"strut", "大摇大摆地走", NULL},
	{"tongue", "吐了吐舌头", NULL},
	{"think", "歪著头想了一下", NULL},
	{"wawl", "惊天动地的哭", NULL},
	{NULL, NULL, NULL}
};

int
condition_action(int unum, char *cmd)
{
	int i;

	for (i = 0; condition_data[i].verb; i++) {
		if (!strcmp(cmd, condition_data[i].verb)) {
			sprintf(chatbuf, "\033[1;36m%s \033[33m%s\033[37;0m", users[unum].chatid, condition_data[i].part1_msg);
			send_to_room(users[unum].room, chatbuf);
			return 1;
		}
	}
	return 0;
}

/* -------------------------------------------- */
/* MUD-like social commands : help               */
/* -------------------------------------------- */

char *dscrb[] = {
	"\033[1m【 Verb + Nick：   动词 + 对方名字 】\033[36m   例：//kick piggy\033[m",
	"\033[1m【 Verb + Message：动词 + 要说的话 】\033[36m   例：//sing 天天天蓝\033[m",
	"\033[1m【 Verb：动词 】    ↑↓：旧话重提\033[m", NULL
};
struct action *verbs[] = { party_data, speak_data, condition_data, NULL };

#define SCREEN_WIDTH    80
#define MAX_VERB_LEN    10
#define VERB_NO         8

void
view_action_verb(int unum)
{
	int i, j;
	char *p;

	send_to_unum(unum, "/c");
	for (i = 0; dscrb[i]; i++) {
		send_to_unum(unum, dscrb[i]);
		chatbuf[0] = '\0';
		j = 0;
		while ((p = verbs[i][j++].verb) != NULL) {
			strcat(chatbuf, p);
			if ((j % VERB_NO) == 0) {
				send_to_unum(unum, chatbuf);
				chatbuf[0] = '\0';
			} else {
				strncat(chatbuf, "        ",
					MAX_VERB_LEN - strlen(p));
			}
		}
		if (j % VERB_NO)
			send_to_unum(unum, chatbuf);
		send_to_unum(unum, " ");
	}
}

struct chatcmd chatcmdlist[] = {
	{"act", chat_act, 0},
	{"bye", chat_goodbye, 0},
	{"exit", chat_goodbye, 0},
	{"flags", chat_setroom, 0},
	{"invite", chat_invite, 0},
	{"join", chat_join, 0},
	{"kick", chat_kick, 0},
	{"msg", chat_private, 0},
	{"nick", chat_nick, 0},
	{"operator", chat_makeop, 0},
	{"rooms", chat_list_rooms, 0},
	{"whoin", chat_list_by_room, 1},
	{"wall", chat_broadcast, 1},
	{"cloak", chat_cloak, 1},
	{"who", chat_map_chatids_thisroom, 0},
	{"list", chat_list_users, 0},
	{"topic", chat_topic, 0},
	{"noops", chat_clear_ops, 0},
	{"knock", chat_knock_room, 0},
	{"disop", chat_clear_op, 0},

	{NULL, NULL, 0}
};

int
command_execute(int unum)
{
	char *msg = users[unum].ibuf;
	char *cmd;
	struct chatcmd *cmdrec;
	int match = 0;

	/* Validation routine */
	if (users[unum].room == -1) {
		/* MUST give special /! command if not in the room yet */
		if (msg[0] != '/' || msg[1] != '!')
			return -1;
		else
			return (login_user(unum, msg + 2));
	}

	/* If not a /-command, it goes to the room. */
	if (msg[0] != '/') {
		chat_allmsg(unum, msg);
		return 0;
	}

	msg++;
	cmd = nextword(&msg);

	if (cmd[0] == '/') {
		if (!strcmp(cmd + 1, "help") || (cmd[1] == '\0')) {
			view_action_verb(unum);
			match = 1;
		} else if (NOEMOTE(users[unum].room)) {
			send_to_unum(unum, "本聊天室禁止动作");
			match = 1;
		} else if (party_action(unum, cmd + 1, msg) == 0)
			match = 1;
		else if (speak_action(unum, cmd + 1, msg) == 0)
			match = 1;
		else
			match = condition_action(unum, cmd + 1);
	} else {
		for (cmdrec = chatcmdlist; !match && cmdrec->cmdstr; cmdrec++) {
			if (cmdrec->exact)
				match = !strcasecmp(cmd, cmdrec->cmdstr);
			else
				match = !strncasecmp(cmd, cmdrec->cmdstr, strlen(cmd));
			if (match)
				cmdrec->cmdfunc(unum, msg);
		}
	}

	if (!match) {
		sprintf(chatbuf, "\033[1;31m ◎ \033[37m抱歉，看不懂你的意思：\033[36m/%s \033[31m◎\033[m", cmd);
		send_to_unum(unum, chatbuf);
	}
	memset(users[unum].ibuf, 0, sizeof (users[unum].ibuf));
	return 0;
}

int
process_chat_command(int unum)
{
	register int i;
	int rc, ibufsize;

	if ((rc = recv(users[unum].sockfd, chatbuf, sizeof (chatbuf), 0)) <= 0) {
		/* disconnected */
		exit_room(unum, EXIT_LOSTCONN, (char *) NULL);
		return -1;
	}
	ibufsize = users[unum].ibufsize;
	for (i = 0; i < rc; i++) {
		/* if newline is two characters, throw out the first */
		if (chatbuf[i] == '\r')
			continue;

		/* carriage return signals end of line */
		else if (chatbuf[i] == '\n') {
			users[unum].ibuf[ibufsize] = '\0';
			if (command_execute(unum) == -1)
				return -1;
			ibufsize = 0;
		}

		/* add other chars to input buffer unless size limit exceeded */
		else {
			if (ibufsize < 127)
				users[unum].ibuf[ibufsize++] = chatbuf[i];
		}
	}
	users[unum].ibufsize = ibufsize;

	return 0;
}

int
main(int argc, char *argv[])
{
	struct sockaddr_in sin;
	register int i;
	int sr, newsock;
	socklen_t sinsize;
	fd_set readfds;
	struct timeval tv;
	struct hostent *local;
	char buf[100];

	umask(007);
	/* ----------------------------- */
	/* init variable : rooms & users */
	/* ----------------------------- */

	if (argc <= 1) {
		strcpy(chatname, CHATNAME1);
		chatroom = 1;
		chatport = CHATPORT1;
	} else {
		chatroom = atoi(argv[1]);
	}
	switch (chatroom) {
	case 4:
		strcpy(chatname, CHATNAME4);
		chatport = CHATPORT4;
		break;
	case 3:
		strcpy(chatname, CHATNAME3);
		chatport = CHATPORT3;
		break;
	case 2:
		strcpy(chatname, CHATNAME2);
		chatport = CHATPORT2;
		break;
	case 1:
	default:
		strcpy(chatname, CHATNAME1);
		chatport = CHATPORT1;
		break;
	}
	maintopic = CHATROOM_TOPIC[chatroom - 1];
	strcpy(rooms[0].name, mainroom);
	strcpy(rooms[0].topic, maintopic);

	if (chatport <= 1000) {
		strcpy(chatname, CHATNAME1);
		chatroom = 1;
	}
	for (i = 0; i < MAXACTIVE; i++) {
		users[i].chatid[0] = '\0';
		users[i].sockfd = users[i].utent = -1;
	}

	/* ------------------------------ */
	/* bind chat server to port       */
	/* ------------------------------ */

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return -1;
	}
	sin.sin_family = AF_INET;
	sin.sin_port = htons(chatport);
	sin.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (struct sockaddr *) &sin, sizeof (sin)) < 0) {
		printf("%d", chatport);
		return -1;
	}
	sinsize = sizeof (sin);
	if (getsockname(sock, (struct sockaddr *) &sin, &sinsize) == -1) {
		perror("getsockname");
		exit(1);
	}

	if (listen(sock, 5) == -1) {
		perror("listen");
		exit(1);
	}

	gethostname(buf, 100);
	local = gethostbyname(buf);

	if (fork()) {
		return (0);
	}
	setpgid(0, 0);

	/* ------------------------------ */
	/* trap signals                   */
	/* ------------------------------ */

	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGURG, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);

	FD_ZERO(&allfds);
	FD_SET(sock, &allfds);
	nfds = sock + 1;

	while (1) {
		memcpy(&readfds, &allfds, sizeof (readfds));

		tv.tv_sec = 60 * 30;
		tv.tv_usec = 0;
		if ((sr = select(nfds, &readfds, NULL, NULL, &tv)) < 0) {
			if (errno == EINTR)
				sleep(50);
			continue;
		} else if (!sr)
			continue;

#if 0
		if (sr == 0) {
			exit(0);	/* normal chat server shutdown */
		}
#endif

		if (FD_ISSET(sock, &readfds)) {
			sinsize = sizeof (sin);
			newsock = accept(sock, (struct sockaddr *) &sin, &sinsize);
			if (newsock == -1) {
				continue;
			}
			for (i = 0; i < MAXACTIVE; i++) {
				if (users[i].sockfd == -1) {
					struct hostent *hp;
					char *s = users[i].host;
					struct in_addr in;
					int j;

					if (local) {
						for (j = 0; local->h_addr_list[j] != 0; j++) {
							memcpy(&in, local->h_addr_list[j], local->h_length);
							if (!memcmp(&sin.sin_addr, &in, local->h_length))
								break;
						}
						if ((local->h_addr_list[j]) || (sin.sin_addr.s_addr == 0x100007F))
							strcpy(s, "localhost");
						else {
							hp = gethostbyaddr((char *) &sin.sin_addr, sizeof(struct in_addr), sin.sin_family);
							strncpy(s, hp ? hp->h_name : (char *)inet_ntoa(sin.sin_addr), 30);
						}
					} else {
						hp = gethostbyaddr((char *) &sin.sin_addr, sizeof(struct in_addr), sin.sin_family);
						strncpy(s, hp ? hp->h_name : (char *)inet_ntoa(sin.sin_addr), 30);
					}
					s[29] = 0;

					users[i].sockfd = newsock;
					users[i].room = -1;
					break;
				}
			}

			if (i >= MAXACTIVE) {
				/* full -- no more chat users */
				close(newsock);
			} else {

#if !RELIABLE_SELECT_FOR_WRITE
				int flags = fcntl(newsock, F_GETFL, 0);
				flags |= O_NDELAY;
				fcntl(newsock, F_SETFL, flags);
#endif

				FD_SET(newsock, &allfds);
				if (newsock >= nfds)
					nfds = newsock + 1;
				num_conns++;
			}
		}

		for (i = 0; i < MAXACTIVE; i++) {
			/* we are done with newsock, so re-use the variable */
			newsock = users[i].sockfd;
			if (newsock != -1 && FD_ISSET(newsock, &readfds)) {
				if (process_chat_command(i) == -1) {
					logout_user(i);
				}
			}
		}
#if 0
		if (num_conns <= 0) {
			/* one more pass at select, then we go bye-bye */
			tv = zerotv;
		}
#endif
	}
	/* NOTREACHED */
}

