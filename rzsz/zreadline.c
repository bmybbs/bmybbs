/*
  zreadline.c - line reading stuff for lrzsz
  Copyright (C) until 1998 Chuck Forsberg (OMEN Technology Inc)
  Copyright (C) 1994 Matt Porter
  Copyright (C) 1996, 1997 Uwe Ohse

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
  02111-1307, USA.

  originally written by Chuck Forsberg
*/
/* once part of lrz.c, taken out to be useful to lsz.c too */

#include "zglobal.h"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>

/* Ward Christensen / CP/M parameters - Don't change these! */
#define TIMEOUT (-2)

static int readline_readnum;
static int readline_fd;
static char *readline_buffer;
int readline_left = 0;
char *readline_ptr;

jmp_buf zmodemjmp;

static void
zreadline_alarm_handler(int dummy)
{
	/* doesn't need to do anything */
	longjmp(zmodemjmp, 1);
}

/*
 * This version of readline is reasonably well suited for
 * reading many characters.
 *
 * timeout is in tenths of seconds
 */
int
readline_internal(unsigned int timeout, int *func_read(int, char *, int))
{
	if (!no_timeout) {
		unsigned int n;

		n = timeout / 10;
		if (n < 2 && timeout != 1)
			n = 3;
		else if (n == 0)
			n = 1;
		signal(SIGALRM, zreadline_alarm_handler);
		alarm(n);
	}

	readline_ptr = readline_buffer;
	readline_left = (int)((*func_read)(readline_fd, readline_ptr, readline_readnum));
	if (!no_timeout)
		alarm(0);
	if (readline_left > 0 && bytes_per_error) {
		static long ct = 0;
		static int mod = 1;

		ct += readline_left;
		while (ct > bytes_per_error) {
			readline_ptr[ct % bytes_per_error] ^= mod;
			ct -= bytes_per_error;
			mod++;
			if (mod == 256)
				mod = 1;
		}
	}
	if (readline_left < 1)
		return TIMEOUT;
	--readline_left;
	return (*readline_ptr++ & 0377);
}

void
readline_setup(int fd, size_t readnum, size_t bufsize)
{
	readline_fd = fd;
	readline_readnum = readnum;
	readline_buffer = malloc(bufsize > readnum ? bufsize : readnum);
	if (!readline_buffer)
		zmodem_error(1, 0, "out of memory");
}

void
readline_clean(void)
{
	if (readline_buffer)
		free(readline_buffer);
	readline_buffer = NULL;
}

void
readline_purge(void)
{
	readline_left = 0;
	return;
}
