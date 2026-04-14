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
#include "bbs_global_vars.h"
#include "tmpl.h"
#include "smth_screen.h"
#include "io.h"
#include "more.h"
#include "stuff.h"
#include "bbsinc.h"
#include "mail.h"
#include "edit.h"
#include "postheader.h"
#include "bbs-internal.h"

extern int numofsig;

static int
getdata_ctrl(line, col, prompt, buf, len, echo, clearlabel)
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
			ytht_strsncpy(tmp, &buf[curr + 1], sizeof tmp);
			buf[curr] = '\0';
			strcat(buf, tmp);
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

 		/*pzhg's modification, to deal with ^p*/
		if ((!in_mail) && (ch == Ctrl('P'))){
			buf[0]=ch;
			clen = 1;
			break;
		}
		/*end*/

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


int
post_header(header)
struct postheader *header;
{
	int anonyboard = 0;
	char r_prompt[256], mybuf[256], ans[5], genbuf[STRLEN];
	char titlebuf[STRLEN];
	struct boardmem *bp;

	header->canreply = 1;
	if (currentuser.signature > numofsig || currentuser.signature < -1)
		currentuser.signature = 1;
	if (header->reply_mode) {
		strcpy(titlebuf, header->title);
		header->include_mode = 'S';
	} else
		titlebuf[0] = '\0';
	bp = ythtbbs_cache_Board_get_board_by_name(currboard);
	if (bp == NULL)
		return NA;
	if (header->postboard)
		anonyboard = bp->header.flag & ANONY_FLAG;
	header->chk_anony = 0;
	/*add by macintosh 060329, whether there're templets*/
	int hastmpl = 0;
	if (!in_mail){
		char tmplfile[STRLEN];
		setbfile(tmplfile, sizeof(tmplfile), currboard, ".tmpl");
		if (file_exist(tmplfile))
			hastmpl = file_size(tmplfile) / sizeof (struct a_template);
	}
	/*end*/
	while (1) {
		if (header->reply_mode)
			sprintf(r_prompt, "ÒýÑÔÄ£Ê½ [[1m%c[m] "
			"°´[1;32mS[0;37m/[1;32mY[0;37m/[1;32mN[0;37m/[1;32mR[0;37m/[1;32mA[0;37m¸ÄÒýÑÔÄ£Ê½[m",
				header->include_mode);
		move(t_lines - 4, 1);
		clrtobot();
		prints("\033[0m©±©¥©¥©¥©¥©¥©·[m%s [1m%s[m  %s %s %s\n",
		       (header->postboard) ? "·¢±íÎÄÕÂÓÚ" : "ÊÕÐÅÈË£º",
		       header->ds,
			(!in_mail) ? ((header->canreply) ? "±¾ÎÄ[1m¿É[m»Ø¸´\033[1;32mU\033[m" :
					"±¾ÎÄ[1m²»¿É[m»Ø¸´\033[1;32mU\033[m"):"",
			(!in_mail) ? ((header->mailreply) ? "[1mÒª[m»Ø¸´µ½ÐÅÏä\033[1;32mB\033[m" :
					"[1m²»[m»Ø¸´µ½ÐÅÏä\033[1;32mB\033[m"):"",
		       (anonyboard) ? (header->chk_anony ==
				       1 ? "[1mÒª[mÊ¹ÓÃÄäÃû\033[1;32mC\033[m" :
				       "[1m²»[mÊ¹ÓÃÄäÃû\033[1;32mC\033[m") : "");
		move(t_lines - 3, 1);
		prints("\033[0m©¦ ÎÄÔð×Ô¸º ©§Ê¹ÓÃ±êÌâ: [1m%-50s[m\n",
		       (header->title[0] ==
			'\0') ? "[ÕýÔÚÉè¶¨Ö÷Ìâ]" : header->title);
		move(t_lines - 2, 1);
		sprintf(genbuf, "µÚ [1m%d[m ¸ö", currentuser.signature);
		prints("\033[0m©¸©¤©¤©¤©¤©¤©¾Ê¹ÓÃ%sÇ©Ãûµµ     %s",
		       ((currentuser.signature == -1) ? "Ëæ»ú" : genbuf)
		       , (header->reply_mode) ? r_prompt :
		       ((hastmpl)? "°´Ctrl+P½øÈëÄ£°æ·¢ÎÄ":""));
		if (titlebuf[0] == '\0') {
			move(t_lines - 1, 0);
			if (header->postboard == YEA
			    || strcmp(header->title, "Ã»Ö÷Ìâ"))
				strcpy(titlebuf, header->title);
			getdata_ctrl(t_lines - 1, 0, "±êÌâ: ", titlebuf, 50, DOECHO,
				NA);
			{
				int i = strlen(titlebuf) - 1;
				while (i > 0 && isspace(titlebuf[i]))
					titlebuf[i--] = 0;
			}
			if (titlebuf[0] == '\0') {
				if (header->title[0] != '\0') {
					titlebuf[0] = ' ';
					continue;
				} else
					return NA;
			}

			/*pzhg's modification*/
			/*enter the tmpl's chose derectly*/
			if ((!in_mail) && (titlebuf[0]==Ctrl('P'))){
				choose_tmpl();
				return NA;
			}
			/*end*/

			strcpy(header->title, titlebuf);
			continue;
		}
		move(t_lines - 1, 0);
		sprintf(mybuf,
			"Çë°´\033[1;32m0\033[m~\033[1;32m%d/V/L\033[mÑ¡/¿´/Ëæ»úÇ©Ãûµµ%s,"
			"\033[1;32mT\033[m¸Ä±êÌâ%s,%s%s\033[1;32mEnter\033[mÈ·¶¨:",
			numofsig,
			(header->reply_mode) ?
			""//",\033[1;32mS\033[m/\033[1;32mY\033[m/\033[1;32mN\033[m/\033[1;32mR\033[m/\033[1;32mA\033[m¸ÄÒýÑÔÄ£Ê½"
			: ((hastmpl)?",\033[1;32mP\033[mÄ£°å·¢ÎÄ":""), (anonyboard) ? ",\033[1;32mC\033[mÄäÃû" : "",
			"",//in_mail ? "" : "\033[1;32mU\033[mÊôÐÔ,"
			in_mail ? "" : "\033[1;32mQ\033[m·ÅÆú,");
		if (strlen(mybuf)>79+10*11)
			sprintf(mybuf,
				"Çë°´\033[1;32m0\033[m~\033[1;32m%d/V/L\033[mÑ¡/¿´/Ëæ»úÇ©Ãûµµ%s,"
				"\033[1;32mT\033[m¸Ä±êÌâ%s,%s%s\033[1;32mEnter\033[mÈ·¶¨:",
				numofsig,
				(header->reply_mode) ?
				",\033[1;32mS\033[m/\033[1;32mY\033[m/\033[1;32mN\033[m/\033[1;32mR\033[m/\033[1;32mA\033[m¸ÄÒýÑÔÄ£Ê½"
				: ((hastmpl)?",\033[1;32mP\033[mÄ£°å·¢ÎÄ":""), (anonyboard) ? ",\033[1;32mC\033[mÄäÃû" : "",
				"","");//in_mail ? "" : "\033[1;32mU\033[mÊôÐÔ,"
		getdata(t_lines - 1, 0, mybuf, ans, 3, DOECHO, YEA);
		ans[0] = toupper(ans[0]);
		if ((ans[0] - '0') >= 0 && ans[0] - '0' <= 9) {
			if (atoi(ans) <= numofsig)
				currentuser.signature = atoi(ans);
		}else if (ans[0] == 'Q' && !in_mail)
			return NA;
		else if (ans[0] == 'P' && !(header->reply_mode) && (!in_mail)){
			choose_tmpl();
			return NA;
		}else if ((header->reply_mode &&
			    (ans[0] == 'Y' || ans[0] == 'N' || ans[0] == 'A'
			     || ans[0] == 'R')) || ans[0] == 'S') {
			header->include_mode = ans[0];
		}
		else if (ans[0]=='U' && !in_mail){
			if (header->canreply == 0)
				header->canreply =1;
			else
				header->canreply =0;
		} else if (ans[0]=='B' && !in_mail){
			if (header->mailreply == 0)
				header->mailreply =1;
			else
				header->mailreply =0;
		} else if (ans[0] == 'T') {
			titlebuf[0] = '\0';
		} else if (ans[0] == 'C' && anonyboard) {
			header->chk_anony = (header->chk_anony == 1) ? 0 : 1;
		} else if (ans[0] == 'V') {
			sethomefile_s(mybuf, sizeof(mybuf), currentuser.userid, "signatures");
			if (askyn("Ô¤ÉèÏÔÊ¾Ç°Èý¸öÇ©Ãûµµ, ÒªÏÔÊ¾È«²¿Âð", NA, YEA)
			    == YEA)
				ansimore(mybuf, YEA);
			else {
				clear();
				ansimore2(mybuf, YEA, 0, 18);
			}
		} else if (ans[0] == 'L') {
			currentuser.signature = -1;
		} else {
			if (header->title[0] == '\0')
				return NA;
			else
				return YEA;
		}
	}
}
