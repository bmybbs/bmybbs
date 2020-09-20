/* uudecode utility.
   Copyright (C) 1994, 1995 Free Software Foundation, Inc.

   This product is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This product is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this product; see the file COPYING.  If not, write to
   the Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/* Copyright (c) 1983 Regents of the University of California.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. All advertising materials mentioning features or use of this software
      must display the following acknowledgement:
	 This product includes software developed by the University of
	 California, Berkeley and its contributors.
   4. Neither the name of the University nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE.  */

/* Reworked to GNU style by Ian Lance Taylor, ian@airs.com, August 93.  */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

#include "config.h"
#include "ytht/uudecode.h"
#include "ytht/common.h"

#define	DEC(Char) (((Char) - ' ') & 077)

int
uudecode(fp, outname)
FILE *fp;
char *outname;
{
	int fd;
	size_t nw = 0;
	FILE *fw;
	char buf[2 * BUFSIZ], wbuf[1024 * 16 + 2];

	if (strstr(outname, "..") != NULL) {
		errlog("uudecode: 非法文件名!%s", outname);
		return -999;
	}

	fd = open(outname, O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IRGRP);
	if (fd < 0) {
		fakedecode(fp);
		return -1;
	}
	fw = fdopen(fd, "w");
	if (fw == NULL) {
		close(fd);
		unlink(outname);
		fakedecode(fp);
		return -2;
	}
	while (1) {
		int n;
		char *p;

		if (fgets(buf, sizeof (buf), fp) == NULL) {
			errlog("uudecode: not finish %s", outname);
			fclose(fw);
			close(fd);
			return -3;
		}
		p = buf;

		/* N is used to avoid writing out all the characters at the end of
		   the file.  */

		n = DEC(*p);
		if (n <= 0) {
			if (nw)
				fwrite(wbuf, 1, nw, fw);
			break;
		}
		for (++p; n > 0; p += 4, n -= 3) {
			if (n >= 3) {
				wbuf[nw++] = DEC(p[0]) << 2 | DEC(p[1]) >> 4;
				wbuf[nw++] = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
				wbuf[nw++] = DEC(p[2]) << 6 | DEC(p[3]);
			} else {
				if (n >= 1) {
					wbuf[nw++] =
					    DEC(p[0]) << 2 | DEC(p[1]) >> 4;
				}
				if (n >= 2) {
					wbuf[nw++] =
					    DEC(p[1]) << 4 | DEC(p[2]) >> 2;
				}
			}
			if (nw >= sizeof (wbuf) - 2) {
				fwrite(wbuf, nw, 1, fw);
				nw = 0;
			}
		}
	}
	fclose(fw);
	if (fgets(buf, sizeof (buf), fp) == NULL
	    || (strcmp(buf, "end\n") && strcmp(buf, "end\r\n"))) {
		errlog("uudecode: No end line.%s", outname);
		close(fd);
		return -4;
	}
	close(fd);
	return 0;
}

int
fakedecode(FILE * fp)
{
	char buf[256];
	while (fgets(buf, sizeof (buf), fp) != NULL) {
		if (buf[0] != 'e')
			continue;
		if (!strcmp(buf, "end\n") || !strcmp(buf, "end\r\n"))
			return 0;
	}
	return -1;
}

char *
attachdecode(FILE * fp, char *articlename, char *filename)
{
	char *ano;
	char decodefile[NAME_MAX + 1];
	static char decodepath[PATH_MAX + 1];
	if (strlen(articlename) > 2)
		ano = articlename + 2;
	else
		ano = articlename;
	snprintf(decodefile, NAME_MAX, "%s:%s", ano, filename);
	snprintf(decodepath, PATH_MAX, "%s/%s", ATTACHCACHE, decodefile);
	if (access(decodepath, F_OK)) {
		if (uudecode(fp, decodepath))
			return NULL;
	} else
		fakedecode(fp);
	return decodepath;
}
