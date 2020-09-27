/***********************************************
	term.c for BBS without tty use
	
    Firebird Bulletin Board System
    Copyright (C) 1999	KCN,Zhou lin,kcn@cic.tsinghua.edu.cn
    
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

#include <arpa/telnet.h>

int clearbuflen = 6;
char clearbuf[6];

int cleolbuflen = 3;
char cleolbuf[3];

int strtstandoutlen = 4;
char strtstandout[4];

int endstandoutlen = 3;
char endstandout[3];

int t_lines = 24;
int t_columns = 80;

int automargins = 1;

#ifdef CAN_EXEC
int term_convert;
queue_tl qneti, qneto;
#endif

void init_tty(void) {
#ifdef CAN_EXEC
	max_timeout = 0;
	tmachine_init(0);
#endif
}

void term_init(void) {
	strncpy(clearbuf, "\033[H\033[J", clearbuflen);
	strncpy(cleolbuf, "\033[K", cleolbuflen);
	strncpy(strtstandout, "\033[7m", strtstandoutlen);
	strncpy(endstandout, "\033[m", endstandoutlen);
}

void do_move(int destcol, int destline, int (*outc) ()) {
	char buf[30];
	char *p;
	sprintf(buf, "\033[%d;%dH", destline + 1, destcol + 1);
	for (p = buf; *p != 0; p++)
		(*outc) (*p);
}
