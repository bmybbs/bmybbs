 /*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
    Guy Vega, gtvega@seabass.st.usm.edu
    Dominic Tynes, dbtynes@seabass.st.usm.edu
    Copyright (C) 1999, KCN,Zhou Lin, kcn@cic.tsinghua.edu.cn

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  */

#include "bbs.h"
#include "bbstelnet.h"
#include "edit.h"

#define OBUFSIZE  (1024*4)
#define IBUFSIZE  (256)

#define INPUT_ACTIVE 0
#define INPUT_IDLE 1

unsigned char outbuffer[OBUFSIZE + 1];
unsigned char *outbuf = outbuffer + 1;
int obufsize = 0;

unsigned char inbuffer[IBUFSIZE + 1];
unsigned char *inbuf = inbuffer + 1;
int ibufsize = 0;
int icurrchar = 0;
unsigned char inbuffer2[21];
int ibufsize2 = 0;
int icurrchar2 = 0;

int KEY_ESC_arg;

static int i_mode = INPUT_ACTIVE;
time_t now_t, old;

static void hit_alarm_clock(void);
static int telnet_machine(int ch);
static int filter_telnet(unsigned char *s, int *len);
static int igetch2(void);
static int trans0(char *pinyin, char *trans0pinyin);
static int dochinput(char *remstr, char *chstr, int ch);
static int igetch(void);
static int igetch_org(void);
static void top_show(char *prompt);

void
oflush()
{
	if (obufsize) {
		if (convcode) {
			char *out;
			out = gb2big(outbuf, &obufsize, 0);
#ifdef SSHBBS
			if (ssh_write(0, out, obufsize) <= 0)
#else
			if (write(0, out, obufsize) <= 0)
#endif
				abort_bbs();
#ifdef SSHBBS
		} else if (ssh_write(0, outbuf, obufsize) <= 0)
#else
		} else if (write(0, outbuf, obufsize) <= 0)
#endif
			abort_bbs();
	}
	obufsize = 0;
}

static void
hit_alarm_clock()
{
	if (HAS_PERM(PERM_NOTIMEOUT))
		return;
	if (i_mode == INPUT_IDLE) {
		clear();
		prints("Idle timeout exceeded! Booting...\n");
		oflush();
		kill(getpid(), SIGHUP);
	}
	i_mode = INPUT_IDLE;
	alarm(IDLE_TIMEOUT);
}

void
init_alarm()
{
	i_mode = INPUT_IDLE;
	signal(SIGALRM, (void *) hit_alarm_clock);
	alarm(IDLE_TIMEOUT);
}

void
ochar(c)
int c;
{
	if (obufsize > OBUFSIZE - 1) {	/* doin a oflush */
		oflush();
	}
	outbuf[obufsize++] = c;
	/* need to change IAC to IAC IAC */
	if (c == IAC) {
		if (obufsize > OBUFSIZE - 1) {	/* doin a oflush */
			oflush();
		}
		outbuf[obufsize++] = c;
	}
}

void
output(const char *s, int len)
{
	int l, n;
	char *p0, *p1;
	p0 = s;
	l = len;
	while ((p1 = memchr(p0, IAC, l)) != NULL) {
		n = p1 - p0 + 1;
		if (obufsize + n + 1 > OBUFSIZE) {
#ifdef SSHBBS
			ssh_write(0, outbuf, obufsize);
#else
			write(0, outbuf, obufsize);
#endif
			obufsize = 0;
		}
		memcpy(outbuf + obufsize, p0, n);
		obufsize += n;
		p0 += n;
		l -= n;
		outbuf[obufsize++] = IAC;
	}
	if (obufsize + l > OBUFSIZE) {
#ifdef SSHBBS
		ssh_write(0, outbuf, obufsize);
#else
		write(0, outbuf, obufsize);
#endif
		obufsize = 0;
	}
	memcpy(outbuf + obufsize, p0, l);
	obufsize += l;
}

void inline
outputstr(s)
char *s;
{
	output(s, strlen(s));
}

int i_newfd = 0;
struct timeval i_to, *i_top = NULL;
void (*flushf) () = NULL;

void
add_io(fd, timeout)
int fd;
int timeout;
{
	i_newfd = fd;
	if (timeout) {
		i_to.tv_sec = timeout;
		i_to.tv_usec = 0;
		i_top = &i_to;
	} else
		i_top = NULL;
}

void
add_flush(flushfunc)
void (*flushfunc) ();
{
	flushf = flushfunc;
}

int
num_in_buf()
{
	return icurrchar - ibufsize;
}

char lastch;
int telnet_state = 0;
int naw_col, naw_ln, naw_changed = 0;
static int
telnet_machine(ch)
unsigned char ch;
{
	switch (telnet_state) {
	case 255:		/* after the first IAC */
		switch (ch) {
		case DO:
		case DONT:
		case WILL:
		case WONT:
			telnet_state = 1;
			break;
		case SB:	/* loop forever looking for the SE */
			telnet_state = 2;
			break;
		case IAC:
			return IAC;
		default:
			telnet_state = 0;	/* other telnet command */
		}
		break;
	case 1:		/* Get the DO,DONT,WILL,WONT */
		telnet_state = 0;	/* the ch is the telnet option */
		break;
	case 2:		/* the telnet suboption */
		if (ch == 31)	/* 改变行列 */
			telnet_state = 4;
		if (ch == IAC)
			telnet_state = 3;	/* wait for SE */
		break;
	case 3:		/* wait for se */
		if (ch == SE)
			telnet_state = 0;
		if (naw_changed) {
			naw_changed = 0;
			do_naws(naw_ln, naw_col);
		} else
			telnet_state = 2;
		break;
	case 4:
		if (ch != 0)
			telnet_state = 3;
		else
			telnet_state = 5;
		break;
	case 5:
		naw_col = ch;
		telnet_state = 6;
		break;
	case 6:
		if (ch != 0)
			telnet_state = 3;
		else {
			naw_changed = 1;
			telnet_state = 7;
		}
		break;
	case 7:
		naw_ln = ch;
		telnet_state = 2;
		break;
	}
	return 0;
}

static int
filter_telnet(unsigned char *s, int *len)
{
	unsigned char *p1, *p2, *pend;
	int newlen;
	newlen = 0;
	for (p1 = s, p2 = s, pend = s + (*len); p1 != pend; p1++) {
		if (telnet_state) {
			int ch = 0;
			ch = telnet_machine(*p1);
			if (ch == IAC) {	/* 两个IAC */
				*p2 = IAC;
				p2++;
				newlen++;
			}
		} else {
			if (*p1 == IAC)
				telnet_state = 255;
			else {
				*p2 = *p1;
				p2++;
				newlen++;
			}
		}
	}
	return (*len = newlen);
}

static int
igetch2()
{
	char c;

      igetagain2:
	while ((icurrchar2 < ibufsize2) && (inbuf[icurrchar2] == 0))
		icurrchar2++;

	if (ibufsize2 <= icurrchar2) {
		fd_set readfds;
		struct timeval to;
		int sr, hifd;
		if (i_top)
			to = *i_top;
		else {
			to.tv_sec = IDLE_TIMEOUT * 60 * 3;
			to.tv_usec = 0;;
		}
		hifd = 1;
		FD_ZERO(&readfds);

		while (1) {
			FD_SET(0, &readfds);
			sr = select(hifd, &readfds, NULL, NULL, &to);
			if (sr >= 0)
				break;
			if (errno == EINTR)
				continue;
			else
				abort_bbs();
		}
		if ((sr == 0) && (!i_top))
			abort_bbs();
		if (sr == 0)
			return I_TIMEOUT;
		now_t = time(0);
		uinfo.lasttime = now_t;
		if ((unsigned long) now_t - (unsigned long) old > 60) {
			update_utmp();
		}
#ifdef SSHBBS
		while ((ibufsize2 = ssh_read(0, inbuffer2 + 1, 20)) <= 0) {
#else
		while ((ibufsize2 = read(0, inbuffer2 + 1, 20)) <= 0) {
#endif
			if (ibufsize2 == 0)
				longjmp(byebye, -1);
			if (ibufsize2 < 0 && errno != EINTR)
				longjmp(byebye, -1);
		}
		if (!filter_telnet(inbuffer2 + 1, &ibufsize2)) {
			icurrchar2 = 0;
			ibufsize2 = 0;
			goto igetagain2;
		}
		/* add by KCN for GB/BIG5 encode */
		if (convcode) {
			inbuf = big2gb(inbuffer2 + 1, &ibufsize2, 0);
			if (ibufsize2 == 0) {
				icurrchar2 = 0;
				goto igetagain2;
			}
		} else
			inbuf = inbuffer2 + 1;
		/* end */
		icurrchar2 = 0;
	}

	if (icurrchar2 >= ibufsize2)
		goto igetagain2;

	if (((inbuf[icurrchar2] == '\n') && (lastch == '\r'))
	    || ((inbuf[icurrchar2] == '\r') && (lastch == '\n'))) {
		lastch = 0;
		icurrchar2++;
		goto igetagain2;
	}
	lastch = inbuf[icurrchar2];

	i_mode = INPUT_ACTIVE;
	c = inbuf[icurrchar2];
	switch (c) {
	case Ctrl('L'):
		redoscr();
		icurrchar2++;
		goto igetagain2;
	default:
		break;
	}
	icurrchar2++;
	while ((icurrchar2 != ibufsize2) && (inbuf[icurrchar2] == 0))
		icurrchar2++;
	return c;
}

static char *const (sh[]) = {
	"zh", "z", "y", "x", "t", "sh",
	    "s", "r", "q", "p", "n",
	    "m", "l", "k", "j", "h", "g", "f", "d", "ch", "c", "b", NULL};
static char *const sht = "azyxtosrqpnmlkjhgfdecb";
static char *const (yun[]) = {
	"uo", "un", "ui", "uang", "uan", "uai", "ua", "u", "ou", "ong",
	    "o", "iu", "ing", "in", "ie", "iao", "iang", "ian", "ia",
	    "i", "er", "eng", "en", "ei", "e", "ao", "ang", "an", "ai",
	    "a", NULL};
const static char yunt[] = "qwrtypsudfoghjklzxcivbnme1234a";
static char *const (yunsp[]) = {
"o", "er", "en", "ei", "e", "ao", "ang", "an", "ai", "a", NULL};
const static char yunspt[] = "ovnme1234a";

static int
trans0(char *pinyin, char *trans0pinyin)
{
	int n = strlen(pinyin), t0 = 0, i, j;
	for (i = 0, t0 = 0; i < n && t0 < 12;) {
		while (pinyin[i] == '\'')
			i++;
		for (j = 0; sh[j] != NULL; j++)
			if (strncmp(pinyin + i, sh[j], strlen(sh[j])) == 0)
				break;
		if (sh[j] != NULL) {
			trans0pinyin[t0++] = sht[j];
			i += strlen(sh[j]);
			if (pinyin[i] == '\'') {
				trans0pinyin[t0++] = '?';
				continue;
			}
			for (j = 0; yun[j] != NULL; j++)
				if (strncmp(pinyin + i, yun[j], strlen(yun[j]))
				    == 0)
					break;
			if (yun[j] == NULL)
				trans0pinyin[t0++] = '?';
			else {
				trans0pinyin[t0++] = yunt[j];
				i += strlen(yun[j]);
			}
		} else {
			trans0pinyin[t0++] = '-';
			for (j = 0; yunsp[j] != NULL; j++)
				if (strncmp
				    (pinyin + i, yunsp[j],
				     strlen(yunsp[j])) == 0) break;
			if (yunsp[j] == NULL)
				break;
			else {
				trans0pinyin[t0++] = yunspt[j];
				i += strlen(yunsp[j]);
			}
		}
	}
	trans0pinyin[t0] = 0;
	if (i == n)
		return 1;
	else
		return 0;
}

static int
dochinput(char *remstr, char *chstr, int ch)
{
	static char pinyin[33] = "";
	char trans0pinyin[33];
	static int n = 0;
	int hasdeal = 0;
	if (remstr == NULL) {
		n = 0;
		return 0;
	}
	if (n > 0 && (ch == 8 || ch == 127)) {
		n--;
		pinyin[n] = 0;
		hasdeal = 1;
	} else if (isalpha(ch) || ch == '\'') {
		if (n < 32)
			pinyin[n++] = ch;
		pinyin[n] = 0;
		hasdeal = 1;
	}
	if (hasdeal) {
		sprintf(remstr, "%s#", pinyin);
		if (trans0(pinyin, trans0pinyin)) {
			strcat(remstr, trans0pinyin);
			strcat(remstr, "#");
		}
		chstr[0] = 0;
		return 0;
	}
	strcpy(chstr, pinyin);
	if (n == 0 || strchr("\n\t\r ", (char) ch) == NULL)
		chstr[n++] = ch;
	chstr[n] = 0;
	remstr[0] = 0;
	pinyin[0] = 0;
	if (n == 0)
		return 0;
	n = 0;
	return 1;
}

static int
igetch()
{
	extern int cur_ln;
	static int lp = 0;
	static int chinput = 0;
	static char chinputbuf[40] = "";
	static char remchinputbuf[120] = "";
	static char *ptr = chinputbuf;
	static int mode = 0;
	int ch;

	return igetch_org();

	if (mode) {
		ch = igetch_org();
		if (mode == 1) {	/* Escape sequence */
			if (ch == '[' || ch == 'O')
				mode = 2;
			else if (ch == '1' || ch == '4')
				mode = 3;
			else
				mode = 0;
		} else if (mode == 2) {	/* Cursor key */
			if (ch >= '1' && ch <= '6')
				mode = 3;
			else
				mode = 0;
		} else {	/*if( mode == 3 ) */
			/* Ins Del Home End PgUp PgDn */
			mode = 0;
		}
		return ch;
	}

      CHINPUT:
	if (chinput) {
		if (*ptr != 0)
			return *(ptr++);
		if ((cur_ln > 15 && !lp) || (cur_ln < 10 && lp)) {
			lp = !lp;
			redoscr();
		}
	      AGAIN:
		if (lp) {
			outputstr("\033[s\033[8;0H ");
			//outputstr("\033[8;0H ");
		} else {
			outputstr("\033[s\033[19;0H ");
			//outputstr("\033[19;0H ");
		}
		if (remchinputbuf[0])
			outputstr(remchinputbuf);
		else
			outputstr("糊涂输入法  ^O 切换回普通模式");

		{		//extern unsigned char cur_ln, cur_col;
			//char buf[23];
			//sprintf(buf,"\033[K\033[%d;%dH",(int)cur_ln+1,(int)cur_col+1);
			//outputstr(buf);
			outputstr("\033[K\033[u");
		}

		ch = igetch_org();
		if (ch == KEY_ESC) {
			mode = 1;
			return ch;
		}
		if (ch > 127 || ch < 0)
			return ch;
		if (ch == Ctrl('O')) {
			chinput = 0;
			redoscr();
		} else if (dochinput(remchinputbuf, chinputbuf, ch)) {
			ptr = chinputbuf;
			goto CHINPUT;
		} else
			goto AGAIN;
	}
	ch = igetch_org();
	if (ch == KEY_ESC) {
		mode = 1;
		return ch;
	}
	if (ch == Ctrl('O')) {
		dochinput(NULL, NULL, 0);
		remchinputbuf[0] = 0;
		chinput = 1;
		goto CHINPUT;
	}
	return ch;
}

static int
igetch_org()
{
	char c;
	extern int RMSG;

	if (RMSG == YEA && ((uinfo.mode == CHAT1) || (uinfo.mode == CHAT2)
			    || (uinfo.mode == CHAT3)
			    || (uinfo.mode == TALK) || (uinfo.mode == PAGE)
			    || (uinfo.mode == FIVE)
			    || (uinfo.mode == PAGE_FIVE)))
		return igetch2();

      igetagain:
	while ((icurrchar < ibufsize) && (inbuf[icurrchar] == 0))
		icurrchar++;
	if (ibufsize <= icurrchar) {
		fd_set readfds;
		struct timeval to;
		int sr, hifd = 1;

		if (flushf && RMSG == NA)
			(*flushf) ();
		refresh();
		hifd = 1;
		if (i_newfd && RMSG == NA) {
			hifd = i_newfd + 1;
		}
		if (i_top && RMSG == NA)
			to = *i_top;
		else {
			to.tv_sec = IDLE_TIMEOUT * 60 * 3;
			to.tv_usec = 0;
		}
		FD_ZERO(&readfds);

		while (1) {
			FD_SET(0, &readfds);
			if (i_newfd) {
				FD_SET(i_newfd, &readfds);
			}
			sr = select(hifd, &readfds, NULL, NULL, &to);
			if (sr >= 0)
				break;
			if (errno == EINTR)
				continue;
			else
				abort_bbs();
		}
		if ((sr == 0) && (!i_top))
			abort_bbs();
		if (sr == 0)
			return I_TIMEOUT;
		if (i_newfd && FD_ISSET(i_newfd, &readfds))
			return I_OTHERDATA;
		now_t = time(0);
		uinfo.lasttime = now_t;
		if ((unsigned long) now_t - (unsigned long) old > 60) {
			update_utmp();
		}
#ifdef SSHBBS
		while ((ibufsize = ssh_read(0, inbuffer + 1, IBUFSIZE)) <= 0) {
#else
		while ((ibufsize = read(0, inbuffer + 1, IBUFSIZE)) <= 0) {
#endif
			if (ibufsize == 0)
				longjmp(byebye, -1);
			if (ibufsize < 0 && errno != EINTR)
				longjmp(byebye, -1);
		}
		if (!filter_telnet(inbuffer + 1, &ibufsize)) {
			icurrchar = 0;
			ibufsize = 0;
			goto igetagain;
		}
		/* add by KCN for GB/BIG5 encode */
		if (!convcode) {
			inbuf = inbuffer + 1;
		} else {
			inbuf = big2gb(inbuffer + 1, &ibufsize, 0);
			if (ibufsize == 0) {
				icurrchar = 0;
				goto igetagain;
			}
		}
		/* end */
		icurrchar = 0;
	}

	if (icurrchar >= ibufsize)
		goto igetagain;

	if (((inbuf[icurrchar] == '\n') && (lastch == '\r'))
	    || ((inbuf[icurrchar] == '\r') && (lastch == '\n'))) {
		lastch = 0;
		icurrchar++;
		goto igetagain;
	}

	lastch = inbuf[icurrchar];

	i_mode = INPUT_ACTIVE;
	c = inbuf[icurrchar];
	switch (c) {
	case Ctrl('L'):
		redoscr();
		icurrchar++;
		goto igetagain;
	default:
		break;
	}
	icurrchar++;
	while ((icurrchar != ibufsize) && (inbuf[icurrchar] == 0))
		icurrchar++;
	return c;
}

int
igetkey()
{
	int mode;
	int ch, last;
	int oldblock;
	extern int RMSG;
	extern int have_msg_unread;
	extern int msg_blocked;
	mode = last = 0;
	while (1) {
		ch = igetch();
		if ((ch == Ctrl('Z')) && (RMSG == NA)
		    && (uinfo.mode != LOCKSCREEN)) {
			if (have_msg_unread) {
				oldblock = msg_blocked;
				msg_blocked = 0;
				r_msg();
				msg_blocked = oldblock;
			} else
				r_msg2();
			return 0;
		}
		if (mode == 0) {
			if (ch == KEY_ESC)
				mode = 1;
			else
				return ch;	/* Normal Key */
		} else if (mode == 1) {	/* Escape sequence */
			if (ch == '[' || ch == 'O')
				mode = 2;
			else if (ch == '1' || ch == '4')
				mode = 3;
			else {
				KEY_ESC_arg = ch;
				return KEY_ESC;
			}
		} else if (mode == 2) {	/* Cursor key */
			if (ch >= 'A' && ch <= 'D')
				return KEY_UP + (ch - 'A');
			else if (ch >= '1' && ch <= '6')
				mode = 3;
			else
				return ch;
		} else if (mode == 3) {	/* Ins Del Home End PgUp PgDn */
			if (ch == '~')
				return KEY_HOME + (last - '1');
			else
				return ch;
		}
		last = ch;
	}
}

static void
top_show(prompt)
char *prompt;
{
	if (editansi) {
		prints(ANSI_RESET);
	}
	move(0, 0);
	clrtoeol();
	//standout();
	prints("%s", prompt);
	//standend();
}

int
ask(prompt)
char *prompt;
{
	int ch;

	top_show(prompt);
	ch = igetkey();
	move(0, 0);
	clrtoeol();
	return (ch);
}

int
getdata(line, col, prompt, buf, len, echo, clearlabel)
int line, col, len, echo, clearlabel;
char *prompt, *buf;
{
	int ch, clen = 0, curr = 0, x, y;
	char tmp[STRLEN];
	int dbchar, i;
	extern int scr_cols;
	extern int RMSG;
	extern int have_msg_unread;

	if (clearlabel == YEA) {
		//memset(buf,0, sizeof(buf));
		buf[0] = 0;
	}
	move(line, col);
	if (prompt) {
		prints("%s", prompt);
	}
	getyx(&y, &x);
	clen = strlen(buf);
	curr = (clen >= len) ? len - 1 : clen;
	buf[curr] = '\0';
	prints("%s", buf);
	if (echo == NA) {
		while ((ch = igetkey()) != '\r') {
			if (RMSG == YEA && have_msg_unread == 0) {
				if (ch == Ctrl('Z') || ch == KEY_UP) {
					buf[0] = Ctrl('Z');
					clen = 1;
					break;
				}
				if (ch == Ctrl('A') || ch == KEY_DOWN) {
					buf[0] = Ctrl('A');
					clen = 1;
					break;
				}
			}
			if (ch == '\n')
				break;
			if (ch == '\177' || ch == Ctrl('H')) {
				if (clen == 0) {
					continue;
				}
				clen--;
				getyx(&y, &x);
				move(y, x - 1);
				outc(' ');
				move(y, x - 1);
				continue;
			}
			if (!isprint2(ch)) {
				continue;
			}
			if (clen >= len - 1) {
				continue;
			}
			buf[clen++] = ch;
			if (echo)
				outc(ch);
			else
				outc('*');
		}
		buf[clen] = '\0';
		outc('\n');
		return clen;
	}
	clrtoeol();
	while (1) {
		ch = igetkey();
		if ((RMSG == YEA) && have_msg_unread == 0) {
			if (ch == Ctrl('Z') || ch == KEY_UP) {
				buf[0] = Ctrl('Z');
				clen = 1;
				break;
			}
			if (ch == Ctrl('A') || ch == KEY_DOWN) {
				buf[0] = Ctrl('A');
				clen = 1;
				break;
			}
		}
		if (ch == '\n' || ch == '\r')
			break;
		if (ch == Ctrl('R')) {
			enabledbchar = ~enabledbchar & 1;
			continue;
		}
		if (ch == '\177' || ch == Ctrl('H')) {
			if (curr == 0) {
				continue;
			}
			strcpy(tmp, &buf[curr]);
			if (enabledbchar) {
				dbchar = 0;
				for (i = 0; i < curr - 1; i++)
					if (dbchar)
						dbchar = 0;
					else if (buf[i] & 0x80)
						dbchar = 1;
				if (dbchar) {
					curr--;
					clen--;
				}
			}
			buf[--curr] = '\0';
			(void) strcat(buf, tmp);
			clen--;
			move(y, x);
			prints("%s", buf);
			clrtoeol();
			move(y, x + curr);
			continue;
		}
		if (ch == KEY_DEL) {
			if (curr >= clen) {
				curr = clen;
				continue;
			}
			strcpy(tmp, &buf[curr + 1]);
			buf[curr] = '\0';
			(void) strcat(buf, tmp);
			clen--;
			move(y, x);
			prints("%s", buf);
			clrtoeol();
			move(y, x + curr);
			continue;
		}
		if (ch == KEY_LEFT) {
			if (curr == 0) {
				continue;
			}
			curr--;
			if (enabledbchar) {
				int i, j = 0;
				for (i = 0; i < curr; i++)
					if (j)
						j = 0;
					else if (buf[i] < 0)
						j = 1;
				if (j) {
					curr--;
				}
			}
			move(y, x + curr);
			continue;
		}
		if (ch == Ctrl('E') || ch == KEY_END) {
			curr = clen;
			move(y, x + curr);
			continue;
		}
		if (ch == Ctrl('A') || ch == KEY_HOME) {
			curr = 0;
			move(y, x + curr);
			continue;
		}
		if (ch == KEY_RIGHT) {
			if (curr >= clen) {
				curr = clen;
				continue;
			}
			curr++;
			if (enabledbchar) {
				int i, j = 0;
				for (i = 0; i < curr; i++)
					if (j)
						j = 0;
					else if (buf[i] < 0)
						j = 1;
				if (j) {
					curr++;
				}
			}

			move(y, x + curr);
			continue;
		}
		if (!isprint2(ch)) {
			continue;
		}

		if (x + clen >= scr_cols || clen >= len - 1) {
			continue;
		}

		if (!buf[curr]) {
			buf[curr + 1] = '\0';
			buf[curr] = ch;
		} else {
			strncpy(tmp, &buf[curr], len);
			buf[curr] = ch;
			buf[curr + 1] = '\0';
			strncat(buf, tmp, len - curr);
		}
		curr++;
		clen++;
		move(y, x);
		prints("%s", buf);
		move(y, x + curr);
	}
	buf[clen] = '\0';
	if (echo) {
		move(y, x);
		prints("%s", buf);
	}
	outc('\n');
	return clen;
}

int ZmodemRateLimit = 0;
int
raw_write(int fd, char *buf, int len)
{
	static int lastcounter = 0;
	int nowcounter, i;
	static int bufcounter;
	int retlen = 0;
	int mylen;
	char *wb;
	if (ZmodemRateLimit) {
		nowcounter = time(0);
		if (lastcounter == nowcounter) {
			if (bufcounter >= ZMODEM_RATE) {
				sleep(1);
				nowcounter = time(0);
				bufcounter = len;
			} else
				bufcounter += len;
		} else {
			/*
			 * time clocked, clear bufcounter
			 */
			bufcounter = len;
		}
		lastcounter = nowcounter;
	}
#ifdef SSHBBS
	return ssh_write(fd, buf, len);
#else
	wb = malloc(len * 2);
	if (wb == NULL)
		return -1;
	for (i = 0; i < len; i++) {
		if ((unsigned char) buf[i] == 0xff) {
			wb[retlen++] = '\xff';
			wb[retlen++] = '\xff';
		} else if (buf[i] == 13) {
			wb[retlen++] = '\x1d';
			wb[retlen++] = '\x00';
		} else
			wb[retlen] = buf[i];
	}
	mylen = write(fd, wb, retlen);
	free(wb);
	return mylen;

#endif
}

int
raw_read(int fd, char *buf, int len)
{
#ifdef SSHBBS
	return ssh_read(fd, buf, len);
#else
	return read(fd, buf, len);
#endif
}

int
multi_getdata(int line, int col, int maxcol, char *prompt, char *buf, int len,
	      int maxline, int clearlabel)
{
	int ch, x, y, startx, starty, j, k, i0, cursorx = 0, cursory = 0;
	size_t i, chk, now;
	char savebuffer[25][LINELEN * 3];
	extern int RMSG;

	if (clearlabel == 1) {
		buf[0] = 0;
	}
	move(line, col);
	if (prompt)
		prints("%s", prompt);
	getyx(&starty, &startx);
	now = strlen(buf);
	for (i = 0; i < 24; i++)
		saveline(i, 0, savebuffer[i]);

	while (1) {
		y = starty;
		x = startx;
		move(y, x);
		chk = 0;
		if (now == 0) {
			cursory = y;
			cursorx = x;
		}
		for (i = 0; i < strlen(buf); i++) {
			if (chk)
				chk = 0;
			else if (buf[i] < 0)
				chk = 1;
			if (chk && x >= maxcol)
				x++;
			if (buf[i] != 13 && buf[i] != 10) {
				if (x > maxcol) {
					clrtoeol();
					x = col;
					y++;
					move(y, x);
				}
				prints("%c", buf[i]);
				x++;
			} else {
				clrtoeol();
				x = col;
				y++;
				move(y, x);
			}
			if (i == now - 1) {
				cursory = y;
				cursorx = x;
			}
		}
		clrtoeol();
		move(cursory, cursorx);
		ch = igetkey();
		if ((ch == '\n' || ch == '\r'))	// && num_in_buf()==0)
			break;
		for (i = starty; i <= y; i++)
			saveline(i, 1, savebuffer[i]);
		if (1 == RMSG
		    && (KEY_UP == ch || KEY_DOWN == ch || Ctrl('Z') == ch
			|| Ctrl('A') == ch)
		    && (!buf[0])) {
			return -ch;
		}
		switch (ch) {
		case Ctrl('Q'):
//              case '\n':
//              case '\r':
			if (y - starty + 1 < maxline) {
				for (i = strlen(buf) + 1; i > now; i--)
					buf[i] = buf[i - 1];
				buf[now++] = '\n';
			}
			break;
		case KEY_UP:
			if (cursory > starty) {
				y = starty;
				x = startx;
				chk = 0;
				if (y == cursory - 1 && x <= cursorx)
					now = 0;
				for (i = 0; i < strlen(buf); i++) {
					if (chk)
						chk = 0;
					else if (buf[i] < 0)
						chk = 1;
					if (chk && x >= maxcol)
						x++;
					if (buf[i] != 13 && buf[i] != 10) {
						if (x > maxcol) {
							x = col;
							y++;
						}
						x++;
					} else {
						x = col;
						y++;
					}
					if (!enabledbchar || !chk)
						if (y == cursory - 1
						    && x <= cursorx)
							    now = i + 1;
				}
			}
			break;
		case KEY_DOWN:
			if (cursory < y) {
				y = starty;
				x = startx;
				chk = 0;
				if (y == cursory + 1 && x <= cursorx)
					now = 0;
				for (i = 0; i < strlen(buf); i++) {
					if (chk)
						chk = 0;
					else if (buf[i] < 0)
						chk = 1;
					if (chk && x >= maxcol)
						x++;
					if (buf[i] != 13 && buf[i] != 10) {
						if (x > maxcol) {
							x = col;
							y++;
						}
						x++;
					} else {
						x = col;
						y++;
					}
					if (!enabledbchar || !chk)
						if (y == cursory + 1
						    && x <= cursorx)
							    now = i + 1;
				}
			}
			break;
		case '\177':
		case Ctrl('H'):
			if (now > 0) {
				for (i = now - 1; i < strlen(buf); i++)
					buf[i] = buf[i + 1];
				now--;
				if (enabledbchar) {
					chk = 0;
					for (i = 0; i < now; i++) {
						if (chk)
							chk = 0;
						else if (buf[i] < 0)
							chk = 1;
					}
					if (chk) {
						for (i = now - 1;
						     i < strlen(buf); i++)
							buf[i] = buf[i + 1];
						now--;
					}
				}
			}
			break;
		case KEY_DEL:
			if (now < strlen(buf)) {
				if (enabledbchar) {
					chk = 0;
					for (i = 0; i < now + 1; i++) {
						if (chk)
							chk = 0;
						else if (buf[i] < 0)
							chk = 1;
					}
					if (chk)
						for (i = now; i < strlen(buf);
						     i++)
							buf[i] = buf[i + 1];
				}
				for (i = now; i < strlen(buf); i++)
					buf[i] = buf[i + 1];
			}
			break;
		case KEY_LEFT:
			if (now > 0) {
				now--;
				if (enabledbchar) {
					chk = 0;
					for (i = 0; i < now; i++) {
						if (chk)
							chk = 0;
						else if (buf[i] < 0)
							chk = 1;
					}
					if (chk)
						now--;
				}
			}
			break;
		case KEY_RIGHT:
			if (now < strlen(buf)) {
				now++;
				if (enabledbchar) {
					chk = 0;
					for (i = 0; i < now; i++) {
						if (chk)
							chk = 0;
						else if (buf[i] < 0)
							chk = 1;
					}
					if (chk)
						now++;
				}
			}
			break;
		case KEY_HOME:
		case Ctrl('A'):
			now--;
			while (now >= 0 && buf[now] != '\n' && buf[now] != '\r')
				now--;
			now++;
			break;
		case KEY_END:
		case Ctrl('E'):
			while (now < strlen(buf) && buf[now] != '\n'
			       && buf[now] != '\r')
				now++;
			break;
		case KEY_PGUP:
			now = 0;
			break;
		case KEY_PGDN:
			now = strlen(buf);
			break;
		case Ctrl('Y'):
			i0 = strlen(buf);
			i = now - 1;
			while (i >= 0 && buf[i] != '\n' && buf[i] != '\r')
				i--;
			i++;
			if (!buf[i])
				break;
			j = now;
			while (j < i0 - 1 && buf[j] != '\n' && buf[j] != '\r')
				j++;
			if (j >= i0 - 1)
				j = i0 - 1;
			j = j - i + 1;
			if (j < 0)
				j = 0;
			for (k = 0; k < i0 - i - j + 1; k++)
				buf[i + k] = buf[i + j + k];

			y = starty;
			x = startx;
			chk = 0;
			if (y == cursory && x <= cursorx)
				now = 0;
			for (i = 0; i < strlen(buf); i++) {
				if (chk)
					chk = 0;
				else if (buf[i] < 0)
					chk = 1;
				if (chk && x >= maxcol)
					x++;
				if (buf[i] != 13 && buf[i] != 10) {
					if (x > maxcol) {
						x = col;
						y++;
					}
					x++;
				} else {
					x = col;
					y++;
				}
				if (!enabledbchar || !chk)
					if (y == cursory && x <= cursorx)
						now = i + 1;
			}

			if (now > strlen(buf))
				now = strlen(buf);
			break;
		default:
			if (isprint2(ch) && strlen(buf) < len - 1) {
				for (i = strlen(buf) + 1; i > now; i--)
					buf[i] = buf[i - 1];
				buf[now++] = ch;
				y = starty;
				x = startx;
				chk = 0;
				for (i = 0; i < strlen(buf); i++) {
					if (chk)
						chk = 0;
					else if (buf[i] < 0)
						chk = 1;
					if (chk && x >= maxcol)
						x++;
					if (buf[i] != 13 && buf[i] != 10) {
						if (x > maxcol) {
							x = col;
							y++;
						}
						x++;
					} else {
						x = col;
						y++;
					}
				}
				if (y - starty + 1 > maxline) {
					for (i = now - 1; i < strlen(buf); i++)
						buf[i] = buf[i + 1];
					now--;
				}
			}
			break;
		}
	}

	return y - starty + 1;
}
