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
#include "bbstelnet.h"
#include "chat.h"

char chatroom[IDLEN];		/* Chat-Room Name */
int chatline;			/* Where to display message now */
int stop_line;			/* next line of bottom of message window area */
FILE *rec;
int recflag = 0;
char buftopic[STRLEN];
char chat_station[19];

#define b_lines t_lines-1
#define cuser currentuser
char *msg_seperator = "\
¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª";
char *msg_shortulist = "[1;33;44m\
 Ê¹ÓÃÕß´úºÅ    Ä¿Ç°×´Ì¬  ©¦ Ê¹ÓÃÕß´úºÅ    Ä¿Ç°×´Ì¬  ©¦ Ê¹ÓÃÕß´úºÅ    Ä¿Ç°×´Ì¬ [m";

struct chat_command {
	char *cmdname;		/* Char-room command length */
	void (*cmdfunc) ();	/* Pointer to function */
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
static int printuserent(struct user_info *uentp);
static void chat_help(char *arg);
static void query_user(char *arg);
static void call_user(char *arg);
static void chat_date(void);
static void chat_users(void);
static void set_rec(void);
static void define_alias(char *arg);
static int use_alias(char *arg, int cfd);
static int print_friend_ent(struct user_info *uentp);
static void chat_friends(void);
static void chat_sendmsg(char *arg);
static int chat_cmd_match(char *buf, char *str);
static int chat_cmd(char *buf, int cfd);

static void
chat_load_alias()
{
	char buf[256];
	int i;
	chat_alias_count = 0;
	chat_aliases =
	    (struct chatalias *) malloc(sizeof (struct chatalias) *
					MAXDEFINEALIAS);
	for (i = 0; i < MAXDEFINEALIAS; i++)
		chat_aliases[i].cmd[0] = 0;
	setuserfile(buf, "chatalias");
	chat_alias_count = get_num_records(buf, sizeof (struct chatalias));
	if (chat_alias_count > MAXDEFINEALIAS)
		chat_alias_count = MAXDEFINEALIAS;
	if (chat_alias_count != 0)
		get_records(buf, chat_aliases, sizeof (struct chatalias), 1,
			    chat_alias_count);
	for (i = 0; i < chat_alias_count; i++) {
		if (chat_aliases[i].cmd[0] == 0) {
			chat_alias_count = i;
			break;
		}
		chat_aliases[i].cmd[8] = 0;
		chat_aliases[i].action[80] = 0;
	}
}

void
printchatline(str)
const char *str;
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
	outs("¡ú");
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

static int
chat_send(fd, buf)
int fd;
char *buf;
{
	size_t len;

	sprintf(genbuf, "%s\n", buf);
	len = strlen(genbuf);
	return (send(fd, genbuf, len, 0) == len);
}

static int
chat_recv(fd, chatid)
int fd;
char *chatid;
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
				sprintf(genbuf, "·¿¼ä£º [32m%s", chatroom);
				strncpy(buftopic, bptr + 2, STRLEN - 1);
				prints
				    ("[1;44;33m %-21s  [33m»°Ìâ£º[36m%-47s[5;31m%6s[m",
				     genbuf, bptr + 2,
				     (recflag == 1) ? "Â¼ÒôÖÐ" : "      ");
			}
		} else {
			printchatline(bptr);
		}

		c -= len;
		bptr += len;
	}

	if (c > 0) {
		strcpy(genbuf, bptr);
		strncpy(buf, genbuf, sizeof (buf));
		bufstart = len - 1;
	} else
		bufstart = 0;
	return 0;
}

static void
fixchatid(chatid)
char *chatid;
{
	chatid[CHAT_IDLEN] = '\0';
	while (*chatid != '\0' && *chatid != '\n') {
		if (strchr(BADCIDCHARS, *chatid))
			*chatid = '_';
		chatid++;
	}
}

int
ent_chat(chatbuf)
char *chatbuf;
{
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

	if ((chatroom == 1)
	    && (stationrec = fopen("etc/chatstation", "r")) != NULL) {
		i = 0;
		while (fgets(inbuf, STRLEN, stationrec) != NULL && i <= 11) {
			strncpy(stnname[i], (char *) strtok(inbuf, " \n\r\t"),
				19);
			strncpy(stnaddr[i], (char *) strtok(NULL, " \n\r\t"),
				29);
			strncpy(temp, (char *) strtok(NULL, " \n\r\t"), 9);
			portnum[i] = atoi(temp);
			i++;
		}
		fclose(stationrec);
		move(1, 0);
		clrtobot();
		prints("\n\nÐò Á¬ÏßÕ¾Ãû³Æ             Á¬ÏßÕ¾Î»Ö·\n");
		prints
		    ("== =====================  ==============================\n");
		for (j = 0; j <= i - 1; j++) {
			move(5 + j, 0);
			prints("%2d %-22s %-32s", j, stnname[j], stnaddr[j]);
		}
		getdata(23, 0, "ÇëÊäÈëÕ¾Ì¨ÐòºÅ£º", temp, 3, DOECHO, YEA);
		i = atoi(temp);
		if ((i > j - 1) || (i < 0))
			i = 0;
		clear();
		prints("Á¬Íù¡º%s¡»£¬ÇëÉÔºò...\n", stnname[i]);
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
			prints("¶Ô·½ÁÄÌìÊÒÃ»¿ª£¬Á¬½ø±¾Õ¾µÄ¹ú¼Ê»áÒéÌü...");
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
			if ((connect
			     (cfd, (struct sockaddr *) &sin, sizeof (sin)))) {
				perror("connect failed");
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
			prints("¿ªÆôÁÄÌìÊÒ...");
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
			if ((connect
			     (cfd, (struct sockaddr *) &sin, sizeof (sin)))) {
				perror("connect failed");
				return -1;
			}
		}
	}
	while (1) {
		getdata(2, 0, "ÇëÊäÈëÁÄÌì´úºÅ(ÊäÈë'*'ÍË³ö)£º", inbuf,
			CHAT_IDLEN, DOECHO, YEA);
		if (!strcmp(inbuf, "*") || stringfilter(inbuf, 0)) {
			if (strcmp(inbuf, "*")) {
				strcpy(genbuf, "ÔÚÁÄÌìÊÒÆð¹ýÂËÃû");
				mail_buf(inbuf, "delete", genbuf);
				updatelastpost("deleterequest");
			}
			close(cfd);
			clear();
			uinfo.in_chat = NA;
			uinfo.chatid[0] = '\0';
			update_utmp();
			return 0;
		}
		if (stringfilter(inbuf, 2)) {
			strcpy(genbuf, "ÔÚÁÄÌìÊÒÆð¹ýÂËÃû");
			mail_buf(inbuf, "delete", genbuf);
			updatelastpost("deleterequest");
		}
		sprintf(chatid, "%.8s",
			((inbuf[0] != '\0' && inbuf[0] != '\n') ? inbuf :
			 cuser.userid));
		fixchatid(chatid);
		sprintf(inbuf, "/! %d %d %s %s 0", uinfo.uid,
			cuser.userlevel, cuser.userid, chatid);
		chat_send(cfd, inbuf);
		if (recv(cfd, inbuf, 3, 0) != 3) {
			return 0;
		}
		move(3, 0);
		if (!strcmp(inbuf, CHAT_LOGIN_OK))
			break;
		else if (!strcmp(inbuf, CHAT_LOGIN_EXISTS))
			prints("Õâ¸ö´úºÅÒÑ¾­ÓÐÈËÓÃÁË");
		else if (!strcmp(inbuf, CHAT_LOGIN_INVALID))
			prints("Õâ¸ö´úºÅÊÇ´íÎóµÄ");
		else
			prints("ÄãÒÑ¾­ÓÐÁíÒ»¸öÊÓ´°½øÈë´ËÁÄÌìÊÒ¡£");
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
			printchatline("[1;32m¡ô [31mµ±£¡ÓÊ²îËÍÐÅÀ´ÁË...[m");
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
					for (i = currchar; inbuf[i] && i < 68;
					     i++) ;
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
				if (stringfilter(inbuf, 0)) {
					strcpy(genbuf, "ÔÚÁÄÌìÊÒËµ¹ýÂË´Ê");
					mail_buf(inbuf, "delete", genbuf);
					updatelastpost("deleterequest");
					printchatline
					    ("\033[1;32mÁÄÌìÊÒ½ûÖ¹ÎÛÑÔ»àÓïÅ¶\033[m");
					inbuf[0] = '\0';
					currchar = 0;
					move(b_lines, 10);
					clrtoeol();
					chatting = 1;
					continue;
				}
				if (stringfilter(inbuf, 2)) {
					sprintf(genbuf, "ÔÚÁÄÌìÊÒËµ¹ýÂË´Ê");
					mail_buf(inbuf, "delete", genbuf);
					updatelastpost("deleterequest");
				}
				chatting = chat_cmd(inbuf, cfd);
				if (chatting == 0)
					chatting = chat_send(cfd, inbuf);
				if (!strncmp(inbuf, "/b", 2))
					break;
				for (cmdpos = MAXLASTCMD - 1; cmdpos; cmdpos--)
					strcpy(lastcmd[cmdpos],
					       lastcmd[cmdpos - 1]);
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
				memcpy(&inbuf[currchar], &inbuf[currchar + 1],
				       69 - currchar);
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
				set_rec();
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

static int
printuserent(uentp)
struct user_info *uentp;
{
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
	if (!(HAS_PERM(PERM_SYSOP) || HAS_PERM(PERM_SEECLOAK))
	    && uentp->invisible)
		return 0;

	sprintf(pline, " %s%-13s\x1b[m%c%s%-10.10s\x1b[m",
		myfriend(uentp->uid) ? "\x1b[1;32m" : "",
		uentp->userid, uentp->invisible ? '#' : ' ',
		ModeColor(uentp->mode), ModeType(uentp->mode));
	if (cnt < 2)
		strcat(pline, "©¦");
	strcat(uline, pline);
	if (++cnt == 3) {
		printchatline(uline);
		memset(uline, 0, 256);
		cnt = 0;
	}
	return 0;
}

static void
chat_help(arg)
char *arg;
{
	char *ptr;
	char buf[256];
	FILE *fp;

	if (strstr(arg, " op")) {
		if ((fp = fopen("help/chatophelp", "r")) == NULL)
			return;
		while (fgets(buf, 255, fp) != NULL) {
			ptr = strstr(buf, "\n");
			*ptr = '\0';
			printchatline(buf);
		}
		fclose(fp);
	} else {
		if ((fp = fopen("help/chathelp", "r")) == NULL)
			return;
		while (fgets(buf, 255, fp) != NULL) {
			ptr = strstr(buf, "\n");
			*ptr = '\0';
			printchatline(buf);
		}
		fclose(fp);
	}
}

/* youzi 1997.7.25 */
static void
query_user(arg)
char *arg;
{
	char *userid, msg[STRLEN * 2];
	int exp, perf;
	char qry_mail_dir[STRLEN];

	userid = strrchr(arg, ' ');

	if (userid == NULL) {
		printchatline("[1;37m¡ï [33mÇëÊäÈëÄúÒª²éÑ°µÄ ID [37m¡ï[m");
		return;
	}
	userid++;
	if (!getuser(userid)) {
		printchatline("[1;31m²»ÕýÈ·µÄÊ¹ÓÃÕß´úºÅ[m");
		return;
	}
	sprintf(qry_mail_dir, "mail/%c/%s/%s", mytoupper(lookupuser.userid[0]),
		lookupuser.userid, DOT_DIR);
	exp = countexp(&lookupuser);
	perf = countperf(&lookupuser);

	sprintf(msg, "[1;32m%s[m ([1;37m%s[m) ¹²ÉÏÕ¾ [1;37m%d[m ´Î, ·¢±í"
		"¹ý [1;37m%d[m ÆªÎÄÕÂ", lookupuser.userid,
		lookupuser.username, lookupuser.numlogins, lookupuser.numposts);

	printchatline(msg);

	strcpy(genbuf,
		   lookupuser.dietime ? ytht_ctime(lookupuser.dietime) :
		   ytht_ctime(lookupuser.lastlogin));
	if (ifinprison(lookupuser.userid)) {
		strcpy(genbuf, ytht_ctime(lookupuser.lastlogin));
		sprintf(msg, "ÔÚ¼àÓü·þÐÌ£¬ÈëÓüÊ±¼ä[\033[1m%s\033[m]", genbuf);
	} else if (lookupuser.dietime) {
		sprintf(msg,
			"ÒÑ¾­ËÀÍö£¬»¹ÓÐ [\033[1m%d\033[m] Ìì¾ÍÒª×ªÊÀÍ¶Ì¥ÁË",
			countlife(&lookupuser));
	} else {
		sprintf(msg,
			"ÉÏ´ÎÔÚ [[1;37m%-24.24s[m] ÓÉ [[1;37m%s[m] µ½±¾Õ¾Ò»ÓÎ",
			genbuf,
			(lookupuser.lasthost[0] ==
			 '\0' ? "(²»Ïê)" : lookupuser.lasthost));
	}

	printchatline(msg);

	sprintf(msg, "ÐÅÏä£º[[1;5;37m%2s[m]£¬ÉúÃüÁ¦£º[[1;37m%d[m]",
		(check_query_mail(qry_mail_dir) == 1) ? "¡Ñ" : "  ",
		countlife(&lookupuser));
	printchatline(msg);
	sprintf(msg, "[1;37mÄ¿Ç°%sÕ¾ÉÏ[m", (t_search(userid, NA, 1) != NULL) ?
		"ÔÚ" : "²»ÔÚ");
	printchatline(msg);
}

static void
call_user(arg)
char *arg;
{
	char *userid, msg[STRLEN * 2];
	struct user_info *uin = NULL;
	int good_id;

	userid = strrchr(arg, ' ');
	if (userid == NULL) {
		printchatline("[1;37m¡ï [32mÇëÊäÈëÄãÒªÑûÇëµÄ ID[37m ¡ï[m");
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
		sprintf(msg, "µ½ %s µÄ %s °üÏáÁÄÁÄÌì", chat_station, chatroom);
		do_sendmsg(userid, uin, msg, 1, uin->pid);
		sprintf(msg, "[1;37mÒÑ¾­°ïÄãÑûÇë [32m%s[37m ÁË[m",
			uin->userid);
	} else
		sprintf(msg, "[1;32m%s[37m %s[m", userid,
			uin ? "ÎÞ·¨ºô½Ð" : "²¢Ã»ÓÐÉÏÕ¾");
	printchatline(msg);
}

static void
chat_date()
{
	time_t thetime;

	time(&thetime);
	sprintf(genbuf, "\033[1m %s±ê×¼Ê±¼ä: \033[32m%s\033[0m",
		MY_BBS_NAME, ctime(&thetime));
				       /*---by ylsdd: because of Y2K--*/
	printchatline(genbuf);
}

static void
chat_users()
{
	printchatline("");
	sprintf(genbuf, "[1m¡¾ [36m%s [37mµÄ·Ã¿ÍÁÐ±í ¡¿[m", MY_BBS_NAME);
	printchatline(genbuf);
	printchatline(msg_shortulist);

	if (apply_ulist(printuserent) == -1) {
		printchatline("[1m¿ÕÎÞÒ»ÈË[m");
	}
	printuserent(NULL);
}

static void
set_rec()
{
	char fname[STRLEN];
	time_t now;
	int savemode;

	now = time(0);
/*        if(!(HAS_PERM(PERM_SYSOP)||HAS_PERM(PERM_SEECLOAK)))
                        return;                        */

	sprintf(fname, "tmp/chat.%s", currentuser.userid);
	if (recflag == 0) {
		if ((rec = fopen(fname, "w")) == NULL)
			return;

		printchatline("[1;5;32m¿ªÊ¼Â¼Òô...[m");
		recflag = 1;
		move(0, 0);
		clrtoeol();
		sprintf(genbuf, "·¿¼ä£º [32m%s", chatroom);
		prints("[1;44;33m %-21s  [33m»°Ìâ£º[36m%-47s[5;31m%6s[m",
		       genbuf, buftopic, (recflag == 1) ? "Â¼ÒôÖÐ" : "      ");

		fprintf(rec, "±¾¶ÎÓÉ %s", currentuser.userid);
		fprintf(rec, "ËùÂ¼ÏÂ£¬Ê±¼ä£º %s", ctime(&now));
	} else {
		savemode = uinfo.mode;
		recflag = 0;
		move(0, 0);
		clrtoeol();
		sprintf(genbuf, "·¿¼ä£º [32m%s", chatroom);
		prints("[1;44;33m %-21s  [33m»°Ìâ£º[36m%-47s[5;31m%6s[m",
		       genbuf, buftopic, (recflag == 1) ? "Â¼ÒôÖÐ" : "      ");

		printchatline("[1;5;32mÂ¼Òô½áÊø...[m");
		fprintf(rec, "½áÊøÊ±¼ä£º%s\n", ctime(&now));
		fclose(rec);
		mail_file(fname, currentuser.userid, "Â¼Òô½á¹û");
		/*postfile(fname,"syssecurity","Â¼Òô½á¹û",2); */
		unlink(fname);
		modify_user_mode(savemode);
	}
}

void
setpager()
{
	char buf[STRLEN];

	t_pager();
	sprintf(buf, "[1;32m¡ô [31mºô½ÐÆ÷ %s ÁË[m",
		(uinfo.pager & ALL_PAGER) ? "´ò¿ª" : "¹Ø±Õ");
	printchatline(buf);

}

static void
define_alias(arg)
char *arg;
{
	int i;
	int del = 0;
	char buf[256];
	char *action = NULL;

	for (i = 0; (i < 255) && (arg[i] != 0) && !isspace(arg[i]); i++) ;

	if (arg[i] == 0) {
		if (chat_alias_count != 0) {
			printchatline("ÒÑ¶¨ÒåµÄalias:");
			for (i = 0; i < chat_alias_count; i++) {
				if (chat_aliases[i].cmd[0] != 0) {
					sprintf(buf, "%-9s %s\n",
						chat_aliases[i].cmd,
						chat_aliases[i].action);
					printchatline(buf);
				};
			};
			return;
		} else
			printchatline("Î´¶¨Òåalias\n");
	};

	arg = &arg[i + 1];
	for (i = 0; (i < 9) && (arg[i] != 0) && !isspace(arg[i]); i++) ;
	if (i >= 9) {
		printchatline("aliasÌ«³¤!\n");
		return;
	};

	if (arg[i] == 0)
		del = 1;
	else
		action = &arg[i + 1];

	arg[i] = 0;

	if (!del)
		if (chat_alias_count == MAXDEFINEALIAS) {
			printchatline("×Ô¶¨ÒåaliasÒÑ¾­ÂúÁË\n");
			return;
		}

	for (i = 0; i < chat_alias_count; i++)
		if (!strncasecmp(chat_aliases[i].cmd, arg, 8)) {
			if (del) {
				chat_alias_count--;
				setuserfile(buf, "chatalias");
				if (chat_alias_count != 0) {
					memcpy(&chat_aliases[i],
					       &chat_aliases[chat_alias_count],
					       sizeof (struct chatalias));
					chat_aliases[chat_alias_count].cmd[0] =
					    0;
					substitute_record(buf,
							  &chat_aliases
							  [chat_alias_count],
							  sizeof (chat_aliases
								  [chat_alias_count]),
							  chat_alias_count + 1);
				} else {
					chat_aliases[i].cmd[0] = 0;
				}
				substitute_record(buf, &chat_aliases[i],
						  sizeof (chat_aliases[i]),
						  i + 1);
				sprintf(buf, "×Ô¶¨ÒåaliasÒÑ¾­É¾³ý\n");
				printchatline(buf);
				return;
			} else {
				sprintf(buf, "×Ô¶¨Òåalias-%sÒÑ¾­´æÔÚ\n",
					chat_aliases[i].cmd);
				printchatline(buf);
				return;
			}
		}
	if (!del) {
		strncpy(chat_aliases[chat_alias_count].cmd, arg, 8);
		chat_aliases[chat_alias_count].cmd[8] = 0;
		strncpy(chat_aliases[chat_alias_count].action, action, 80);
		chat_aliases[chat_alias_count].action[81] = 0;
		sprintf(buf, "×Ô¶¨Òåalias-%sÒÑ¾­´´½¨\n", arg);
		printchatline(buf);
		i = chat_alias_count;
		chat_alias_count++;
	} else {
		printchatline("Ã»ÕÒµ½×Ô¶¨Òåalias\n");
		return;
	}
	setuserfile(buf, "chatalias");
	substitute_record(buf, &chat_aliases[i], sizeof (chat_aliases[i]),
			  i + 1);
}

static int
use_alias(arg, cfd)
char *arg;
int cfd;
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
		args[i] = "´ó¼Ò";

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
						if (slen + strlen(args[index]) >
						    255 - 8)
							break;
						buf1[slen] = 0;
						strcat(buf1, "[1m");
						strcat(buf1, args[index]);
						strcat(buf1, "[m");
						slen += strlen(args[index]) + 7;
					} else if (*fmt == 's') {
						if (slen + strlen(args[1]) >
						    255 - 8)
							break;
						buf1[slen] = 0;
						strcat(buf1, "[1m");
						strcat(buf1, args[1]);
						strcat(buf1, "[m");
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
static int
print_friend_ent(uentp)		/* print one user & status if he is a friend */
struct user_info *uentp;
{
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
	if (!HAS_PERM(PERM_SEECLOAK) && uentp->invisible)
		return 0;
#if 0
	if (kill(uentp->pid, 0) == -1)
		return 0;
#endif

	/*if (!myfriend(uentp->userid)) Leeward 99.02.01 */
	if (!myfriend(uentp->uid))
		return 0;

	sprintf(pline, " %-13s%c%-10s",
		uentp->userid, uentp->invisible ? '#' : ' ',
		ModeType(uentp->mode));
	if (cnt < 2)
		strcat(pline, "©¦");
	strcat(uline, pline);
	if (++cnt == 3) {
		printchatline(uline);
		memset(uline, 0, 256);
		cnt = 0;
	}
	return 0;
}

static void
chat_friends()
{
	printchatline("");
	sprintf(genbuf, "[1m¡¾ µ±Ç°ÏßÉÏµÄºÃÓÑÁÐ±í ¡¿[m");
	printchatline(genbuf);
	printchatline(msg_shortulist);

	if (apply_ulist(print_friend_ent) == -1) {
		printchatline("[1mÃ»ÓÐÅóÓÑÔÚÏßÉÏ[m");
	}
	print_friend_ent(NULL);
}

static void
chat_sendmsg(arg)
char *arg;
{
	struct user_info *uentp;
	char *uident, *msgstr, showstr[80];

	uident = strchr(arg, ' ');
	if (uident == NULL) {
		printchatline("[1;32mÇëÊäÈëÄãÒª·¢ÏûÏ¢µÄ ID[m");
		return;
	} else
		uident += 1;

	msgstr = strchr(uident, ' ');
	if (msgstr == NULL) {
		printchatline("[1;32mÇëÊäÈëÄãÒª·¢µÄÏûÏ¢[m");
		return;
	}
	*msgstr = 0;
	msgstr++;

	uentp = t_search(uident, NA, 1);
	if (uentp == NULL) {
		printchatline("[1mÏßÉÏÃ»ÓÐÕâ¸öID[m");
		return;
	}

	if (do_sendmsg(uentp->userid, uentp, msgstr, 2, 0) != 1) {
		sprintf(showstr, "[1mÎÞ·¨·¢ÏûÏ¢¸ø %s [m", uentp->userid);
		printchatline(showstr);
		return;
	}
	sprintf(showstr, "[1mÒÑ¾­¸ø %s ·¢³öÏûÏ¢[m", uentp->userid);
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

static int
chat_cmd_match(buf, str)
char *buf;
char *str;
{
	while (*str && *buf && !isspace(*buf)) {
		if (tolower(*buf++) != *str++)
			return 0;
	}
	return 1;
}

static int
chat_cmd(buf, cfd)
char *buf;
int cfd;
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
			if (*buf != '\0'
			    && chat_cmd_match(buf, chat_cmdtbl[i].cmdname)) {
				chat_cmdtbl[i].cmdfunc(buf);
				return 1;
			}
		}
	return 0;
}
