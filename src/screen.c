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
#include "screen.h"
#include "edit.h"

extern char clearbuf[];
extern char cleolbuf[];
extern char strtstandout[];
extern char endstandout[];
extern int iscolor;
extern int clearbuflen;
extern int cleolbuflen;
extern int strtstandoutlen;
extern int endstandoutlen;
#ifndef VEDITOR
#endif
extern int editansi;

extern int automargins;

#define o_clear()     output(clearbuf,clearbuflen)
#define o_cleol()     output(cleolbuf,cleolbuflen)
#define o_standup()   output(strtstandout,strtstandoutlen)
#define o_standdown() output(endstandout,endstandoutlen)

int scr_lns, scr_cols;
unsigned char cur_ln = 0, cur_col = 0;
int roll, scrollcnt = 0;
unsigned char docls;
unsigned char downfrom;
int standing = NA;
int inansi = NA;
int disable_move = 0;
struct screenline *big_picture = NULL;

static void
init_screen(slns, scols)
int slns, scols;
{
	register struct screenline *slp;

	scr_lns = slns;
	scr_cols = min(scols, LINELEN);
	if (big_picture != NULL) {
		free(big_picture);
		big_picture = (struct screenline *) calloc(scr_lns,
							   sizeof (struct
								   screenline));
		return;
	}
	big_picture =
	    (struct screenline *) calloc(scr_lns, sizeof (struct screenline));
	for (slns = 0; slns < scr_lns; slns++) {
		slp = &big_picture[slns];
		slp->mode = 0;
		slp->len = 0;
		slp->oldlen = 0;
	}
	docls = YEA;
	downfrom = 0;
	roll = 0;
}

void
initscr()
{
//      if (!big_picture) {
	t_columns = WRAPMARGIN;
	init_screen(t_lines, WRAPMARGIN);
//      }
}

int tc_col, tc_line;

static void
rel_move(was_col, was_ln, new_col, new_ln)
int was_col, was_ln, new_col, new_ln;
{
	if (new_ln >= t_lines || new_col >= t_columns)
		return;
	tc_col = new_col;
	tc_line = new_ln;
	if ((new_col == 0) && (new_ln == was_ln + 1)) {
		ochar('\n');
		if (was_col != 0)
			ochar('\r');
		return;
	}
	if ((new_col == 0) && (new_ln == was_ln)) {
		if (was_col != 0)
			ochar('\r');
		return;
	}
	if (was_col == new_col && was_ln == new_ln)
		return;
	if (new_col == was_col - 1 && new_ln == was_ln) {
		ochar(Ctrl('H'));
		return;
	}

	do_move(new_col, new_ln, ochar);
}

void
standoutput(buf, ds, de, sso, eso)
char *buf;
int ds, de, sso, eso;
{
	int st_start, st_end;

	if (eso <= ds || sso >= de) {
		output(buf + ds, de - ds);
		return;
	}
	st_start = max(sso, ds);
	st_end = min(eso, de);
	if (sso > ds)
		output(buf + ds, sso - ds);
	o_standup();
	output(buf + st_start, st_end - st_start);
	o_standdown();
	if (de > eso)
		output(buf + eso, de - eso);
}

void
redoscr()
{
	register int i, j;
	int ochar();
	register struct screenline *bp = big_picture;

	o_clear();
	tc_col = 0;
	tc_line = 0;
	for (i = 0; i < scr_lns; i++) {
		j = i + roll;
		while (j >= scr_lns)
			j -= scr_lns;
		if (bp[j].len == 0)
			continue;
		rel_move(tc_col, tc_line, 0, i);
		if (bp[j].mode & STANDOUT)
			standoutput(bp[j].data, 0, bp[j].len, bp[j].sso,
				    bp[j].eso);
		else
			output(bp[j].data, bp[j].len);
		tc_col += bp[j].len;
		if (tc_col >= t_columns) {
			if (!automargins) {
				tc_col -= t_columns;
				tc_line++;
				if (tc_line >= t_lines)
					tc_line = t_lines - 1;
			} else
				tc_col = t_columns - 1;
		}
		bp[j].mode &= ~(MODIFIED);
		bp[j].oldlen = bp[j].len;
	}
	rel_move(tc_col, tc_line, cur_col, cur_ln);
	docls = NA;
	scrollcnt = 0;
	oflush();
}

void
refresh()
{
	register int i, j;
	register struct screenline *bp = big_picture;
	extern int automargins;

	if (num_in_buf() != 0)
		return;
	if ((docls) || (abs(scrollcnt) >= (scr_lns - 3))) {
		redoscr();
		return;
	}
	if (scrollcnt < 0) {
		char buf[10];
		//rel_move(tc_col,tc_line,0,0) ;
		do_move(0, -1, ochar);
		sprintf(buf, "\033[%dL", -scrollcnt);
		output(buf, strlen(buf));
		do_move(tc_col, tc_line, ochar);
		scrollcnt = 0;
	}
	if (scrollcnt > 0) {
//        rel_move(tc_col,tc_line,0,t_lines-1) ;
		do_move(0, t_lines, ochar);
		while (scrollcnt > 0) {
			j = scr_lns - scrollcnt + roll - 1;
			while (j >= scr_lns)
				j -= scr_lns;
			bp[j].mode |= MODIFIED;
			bp[j].smod = 0;
			bp[j].emod = bp[j].len - 1;
			bp[j].oldlen = 255;
			ochar('\n');
			scrollcnt--;
		}
		//rel_move(tc_col, tc_line, 0, t_lines-1);
		do_move(tc_col, tc_line, ochar);
	}
	for (i = 0; i < scr_lns; i++) {
		j = i + roll;
		while (j >= scr_lns)
			j -= scr_lns;
		if (bp[j].mode & MODIFIED && bp[j].smod < bp[j].len) {
			bp[j].mode &= ~(MODIFIED);
			if (bp[j].emod >= bp[j].len)
				bp[j].emod = bp[j].len - 1;
			rel_move(tc_col, tc_line, bp[j].smod, i);
			if (bp[j].mode & STANDOUT)
				standoutput(bp[j].data, bp[j].smod,
					    bp[j].emod + 1, bp[j].sso,
					    bp[j].eso);
			else
				output(&bp[j].data[bp[j].smod],
				       bp[j].emod - bp[j].smod + 1);
			tc_col = bp[j].emod + 1;
			if (tc_col >= t_columns) {
				if (automargins) {
					tc_col -= t_columns;
					tc_line++;
					if (tc_line >= t_lines)
						tc_line = t_lines - 1;
				} else
					tc_col = t_columns - 1;
			}
		}
		if (bp[j].oldlen > bp[j].len) {
			rel_move(tc_col, tc_line, bp[j].len, i);
			o_cleol();
		}
		bp[j].oldlen = bp[j].len;
	}
	rel_move(tc_col, tc_line, cur_col, cur_ln);
	oflush();
}

void
move(y, x)
int y, x;
{
	cur_col = x /*+c_shift(y,x) */ ;
	cur_ln = y;
}

void
getyx(y, x)
int *y, *x;
{
	*y = cur_ln;
	*x = cur_col /*-c_shift(y,x)*/ ;
}

void
clear()
{
	register int i;
	register struct screenline *slp;

	roll = 0;
	docls = YEA;
	downfrom = 0;
	for (i = 0; i < scr_lns; i++) {
		slp = &big_picture[i];
		slp->mode = 0;
		slp->len = 0;
		slp->oldlen = 0;
	}
	move(0, 0);
}

void
clear_whole_line(i)
int i;
{
	register struct screenline *slp = &big_picture[i];
	slp->mode = slp->len = 0;
	slp->oldlen = 79;
}

void
clrtoeol()
{
	register struct screenline *slp;
	register int ln;

	standing = NA;
	ln = cur_ln + roll;
	while (ln >= scr_lns)
		ln -= scr_lns;
	slp = &big_picture[ln];
	if (cur_col <= slp->sso)
		slp->mode &= ~STANDOUT;
	if (cur_col > slp->oldlen) {
		register int i;
		for (i = slp->len; i <= cur_col; i++)
			slp->data[i] = 0;
	}
	slp->len = cur_col;
}

void
clrtobot()
{
	register struct screenline *slp;
	register int i, j;

	for (i = cur_ln; i < scr_lns; i++) {
		j = i + roll;
		while (j >= scr_lns)
			j -= scr_lns;
		slp = &big_picture[j];
		slp->mode = 0;
		slp->len = 0;
		if (slp->oldlen > 0)
			slp->oldlen = 255;
	}
}

void
clrstandout()
{
	register int i;
	for (i = 0; i < scr_lns; i++)
		big_picture[i].mode &= ~(STANDOUT);
}

#define nullstr "(null)"

void
outc(c)
unsigned char c;
{
	struct screenline *slp;

#ifndef BIT8
	c &= 0x7f;
#endif
	if (inansi == 1) {
		if (c == 'm')
			inansi = 0;
		return;
	}
	if (c == KEY_ESC && iscolor == NA) {
		inansi = 1;
		return;
	}
	if (c == '\r')
		return;
	slp = &big_picture[(cur_ln + roll) % scr_lns];
	/* deal with non-printables */
	if (!isprint2(c)) {
		if (c == '\n') {	/* do the newline thing */
			if (standing) {
				slp->eso = max(slp->eso, cur_col);
				standing = NA;
			}
			if (cur_col > slp->len) {
				int i;
				for (i = slp->len; i <= cur_col; i++)
					slp->data[i] = ' ';
			}
			slp->len = cur_col;
			cur_col = 0;	/* reset cur_col */
			if (cur_ln < scr_lns)
				cur_ln++;
			return;
		} else if ((c != KEY_TAB && c != KEY_ESC) || !showansi) {
			c = '*';	/* else substitute a '*' for non-printable */
		}
	}
	if (cur_col >= slp->len) {
		int i;
		for (i = slp->len; i < cur_col; i++)
			slp->data[i] = ' ';
		slp->data[cur_col] = '\0';
		slp->len = cur_col + 1;
	}
	if (slp->data[cur_col] != c) {
		if ((slp->mode & MODIFIED) != MODIFIED)
			slp->smod = (slp->emod = cur_col);
		else {
			if (cur_col > slp->emod)
				slp->emod = cur_col;
			if (cur_col < slp->smod)
				slp->smod = cur_col;
		}
		slp->mode |= MODIFIED;
		slp->data[cur_col] = c;
	}
	cur_col++;
	if (cur_col >= scr_cols) {
		if (standing && slp->mode & STANDOUT) {
			standing = NA;
			slp->eso = max(slp->eso, cur_col);
		}
		cur_col = 0;
		if (cur_ln < scr_lns)
			cur_ln++;
	}
}

void
outs(str)
char *str;
{
	register int i;
	while (*str) {
		if (*str != '\t')
			outc(*str++);
		else {
			i = (cur_col + 8) / 8 * 8 - cur_col;
			while (i > 0) {
				outc(' ');
				i--;
			}
			str++;
		}
	}
}

void
outns(str, n)
char *str;
int n;
{
	register int i;
	for (; n > 0; n--) {
		if (*str != '\t')
			outc(*str++);
		else {
			i = (cur_col + 8) / 8 * 8 - cur_col;
			while (i > 0) {
				outc(' ');
				i--;
			}
			str++;
		}
	}
}

const int dec[] =
    { 1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10,
	1
};

void
prints(char *format, ...)
{
	va_list ap;
	register char *fmt;
	char *bp;
	register int i, count, hd, indx;

	va_start(ap, format);
	fmt = format;
	while (*fmt != '\0') {
#ifndef VEDITOR
		if (*fmt == '' && !iscolor) {
			while (*fmt != 'm' && *fmt != 'H')
				fmt++;
			fmt++;
			continue;
		}
#endif
		if (*fmt == '%') {
			int sgn = 1;
			int val = 0;
			int len, negi;

			fmt++;
			while (*fmt == '-') {
				sgn *= -1;
				fmt++;
			}
			while (isdigit(*fmt)) {
				val *= 10;
				val += *fmt - '0';
				fmt++;
			}
			switch (*fmt) {
			case 's':
				bp = va_arg(ap, char *);
				if (bp == NULL)
					bp = nullstr;
				if (val) {
					register int slen = strlen(bp);
					if (val <= slen)
						outns(bp, val);
					else if (sgn > 0) {
						for (slen = val - slen;
						     slen > 0; slen--)
							outc(' ');
						outs(bp);
					} else {
						outs(bp);
						for (slen = val - slen;
						     slen > 0; slen--)
							outc(' ');
					}
				} else
					outs(bp);
				break;
			case 'd':
				i = va_arg(ap, int);

				negi = NA;
				if (i < 0) {
					negi = YEA;
					i *= -1;
				}
				for (indx = 0; indx < 10; indx++)
					if (i >= dec[indx])
						break;
				if (i == 0)
					len = 1;
				else
					len = 10 - indx;
				if (negi)
					len++;
				if (val >= len && sgn > 0) {
					register int slen;
					for (slen = val - len; slen > 0; slen--)
						outc(' ');
				}
				if (negi)
					outc('-');
				hd = 1, indx = 0;
				while (indx < 10) {
					count = 0;
					while (i >= dec[indx]) {
						count++;
						i -= dec[indx];
					}
					indx++;
					if (indx == 10)
						hd = 0;
					if (hd && !count)
						continue;
					hd = 0;
					outc('0' + count);
				}
				if (val >= len && sgn < 0) {
					register int slen;
					for (slen = val - len; slen > 0; slen--)
						outc(' ');
				}
				break;
			case 'c':
				i = va_arg(ap, int);
				outc(i);
				break;
			case '\0':
				goto endprint;
			default:
				outns(fmt, 1);
				break;
			}
			fmt++;
			continue;
		}

		outns(fmt, 1);
		fmt++;
	}
      endprint:
	return;
}

void
addch(ch)
int ch;
{
	outc(ch);
}

void
scroll()
{
	scrollcnt++;
	roll++;
	if (roll >= scr_lns)
		roll -= scr_lns;
	move(scr_lns - 1, 0);
	clrtoeol();
}

void
rscroll()
{
	scrollcnt--;
	if (roll > 0)
		roll--;
	else
		roll = scr_lns - 1;
	move(scr_lns - 1, 0);
	clrtoeol();
	move(0, 0);
	clrtoeol();
}

void
standout()
{
	register struct screenline *slp;
	register int ln;

	if (!strtstandoutlen)
		return;
	if (!standing) {
		ln = cur_ln + roll;
		while (ln >= scr_lns)
			ln -= scr_lns;
		slp = &big_picture[ln];
		standing = YEA;
		slp->sso = cur_col;
		slp->eso = cur_col;
		slp->mode |= STANDOUT;
	}
}

void
standend()
{
	register struct screenline *slp;
	register int ln;

	if (!strtstandoutlen)
		return;
	if (standing) {
		ln = cur_ln + roll;
		while (ln >= scr_lns)
			ln -= scr_lns;
		slp = &big_picture[ln];
		standing = NA;
		slp->eso = max(slp->eso, cur_col);
	}
}

void saveline(int line, int mode, char* buffer) /* 0 : save, 1 : restore */
{
    register struct screenline *bp = big_picture;
    static char tmpbuffer[LINELEN];
    char *tmp = tmpbuffer;
    int x,y;

    if (buffer)
        tmp = buffer;
    switch (mode) {
        case 0:
	    strsncpy(tmp, bp[line].data, LINELEN);
	    tmp[bp[line].len] = '\0';
	    break;
        case 1:
            getyx(&x, &y);
	    move(line, 0);
            clrtoeol();
	    prints("%s", tmp);
            move(x,y);
	break;
	}
};

void
do_naws(int ln, int col)
{
	t_lines = ln;
	t_realcols = col;
	if (t_lines < 24 || t_lines > 100)
		t_lines = 24;
	if (t_realcols < 80 || t_realcols > 240)
		t_realcols = 80;
	initscr();
}
