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

#include <string.h>
#include <stdio.h>
#include "ytht/strlib.h"

#define clearbuflen       6
#define strtstandoutlen   4
#define endstandoutlen    3
#define cleolbuflen       3

static char cleolbuf[clearbuflen];
static char clearbuf[clearbuflen];
static char strtstandout[clearbuflen];
static char endstandout[clearbuflen];

int t_lines = 24;
int t_columns = 80;

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
	ytht_strsncpy(clearbuf, "\033[H\033[J", clearbuflen);
	ytht_strsncpy(cleolbuf, "\033[K", cleolbuflen);
	ytht_strsncpy(strtstandout, "\033[7m", strtstandoutlen);
	ytht_strsncpy(endstandout, "\033[m", endstandoutlen);
}

void do_move(int destcol, int destline, int (*outc) ()) {
	char buf[30];
	char *p;
	sprintf(buf, "\033[%d;%dH", destline + 1, destcol + 1);
	for (p = buf; *p != 0; p++)
		(*outc) (*p);
}
