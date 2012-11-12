/*
  lsz - send files with x/y/zmodem
  Copyright (C) until 1988 Chuck Forsberg (Omen Technology INC)
  Copyright (C) 1994 Matt Porter, Michael D. Black
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
#include "zglobal.h"
#include "rzsz.h"
/* char *getenv(); */

#define SS_NORMAL 0
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <errno.h>

#ifndef R_OK
#  define R_OK 4
#endif

#if defined(HAVE_SYS_MMAN_H) && defined(HAVE_MMAP)
#  include <sys/mman.h>
size_t mm_size;
void *mm_addr = NULL;
#else
#  undef HAVE_MMAP
#endif

#ifndef HAVE_ERRNO_DECLARATION
extern int errno;
#endif

unsigned Baudrate = 2400;	/* Default, should be set by first mode() call */
unsigned Txwindow;		/* Control the size of the transmitted window */
unsigned Txwspac;		/* Spacing between zcrcq requests */
unsigned Txwcnt;		/* Counter used to space ack requests */
size_t Lrxpos;			/* Receiver's last reported offset */
int errors;
enum zm_type_enum protocol;
extern int turbo_escape;
static int no_unixmode;

static int zsendfile(struct zm_fileinfo *zi, const char *buf, size_t blen,
		     int *func_write(int, char *, int),
		     void *func_oflush(void), int *func_read(int, char *,
							      int));
static int getnak(int *func_write(int, char *, int), void *func_oflush(void),
		  int *func_read(int, char *, int));
static int wctxpn(struct zm_fileinfo *, int *func_write(int, char *, int),
		  void *func_oflush(void), int *func_read(int, char *, int));
static int wcs(const char *oname, const char *remotename,
	       int *func_write(int, char *, int), void *func_oflush(void),
	       int *fnc_read(int, char *, int));
static size_t zfilbuf(struct zm_fileinfo *zi);
static size_t filbuf(char *buf, size_t count);
static int getzrxinit(int *func_write(int, char *, int),
		      void *func_oflush(void), int *func_read(int, char *,
							       int));
static int calc_blklen(long total_sent);
static int sendzsinit(int *func_write(int, char *, int),
		      void *func_oflush(void), int *func_read(int, char *,
							       int));
static int wctx(struct zm_fileinfo *, int *func_write(int, char *, int),
		void *func_oflush(void), int *func_read(int, char *, int));
static int zsendfdata(struct zm_fileinfo *, int *func_write(int, char *, int),
		      void *func_oflush(void), int *func_read(int, char *,
							       int));
static int getinsync(struct zm_fileinfo *, int flag,
		     int *func_write(int, char *, int),
		     void *func_oflush(void), int *func_read(int, char *,
							      int));
static void saybibi(int *func_write(int, char *, int), void *func_oflush(void),
		    int *func_read(int, char *, int));
static int wcputsec(char *buf, int sectnum, size_t cseclen,
		    int *func_write(int, char *, int), void *func_oflush(void),
		    int *func_read(int, char *, int));

#define ZSDATA(x,y,z,w,u) \
	do { if (Crc32t) {zsda32(x,y,z,w,u); } else {zsdata(x,y,z,w,u);}} while(0)
#ifdef HAVE_MMAP
#define DATAADR (mm_addr ? ((char *)mm_addr)+zi->bytes_sent : txbuf)
#else
#define DATAADR (txbuf)
#endif

int Filesleft;
long Totalleft;
size_t buffersize = 16384;

#ifdef HAVE_MMAP
int use_mmap = 1;
#endif

/*
 * Attention string to be executed by receiver to interrupt streaming data
 *  when an error is detected.  A pause (0336) may be needed before the
 *  ^C (03) or after it.
 */
#ifdef READCHECK
char Myattn[] = { 0 };
#else
char Myattn[] = { 03, 0336, 0 };
#endif

FILE *input_f;

#define MAX_BLOCK 8192
char txbuf[MAX_BLOCK];

long vpos = 0;			/* Number of bytes read from file */

char Lastrx;
char Crcflg;
int Restricted = 0;		/* restricted; no /.. or ../ in filenames */
int Quiet = 0;			/* overrides logic that would otherwise set verbose */
int Ascii = 0;			/* Add CR's for brain damaged programs */
int Fullname = 0;		/* transmit full pathname */
int Unlinkafter = 0;		/* Unlink file after it is sent */
int Dottoslash = 0;		/* Change foo.bar.baz to foo/bar/baz */
int firstsec;
int errcnt = 0;			/* number of files unreadable */
size_t blklen = 128;		/* length of transmitted records */
int Optiong;			/* Let it rip no wait for sector ACK's */
int Totsecs;			/* total number of sectors this file */
int Filcnt = 0;			/* count of number of files opened */
int Lfseen = 0;
unsigned Rxbuflen = 16384;	/* Receiver's max buffer length */
unsigned Tframlen = 0;		/* Override for tx frame length */
unsigned blkopt = 0;		/* Override value for zmodem blklen */
int Rxflags = 0;
int Rxflags2 = 0;
size_t bytcnt;
int Wantfcs32 = TRUE;		/* want to send 32 bit FCS */
char Lzconv;			/* Local ZMODEM file conversion request */
char Lzmanag;			/* Local ZMODEM file management request */
int Lskipnocor;
char Lztrans;
char zconv;			/* ZMODEM file conversion request */
char zmanag;			/* ZMODEM file management request */
char ztrans;			/* ZMODEM file transport request */
int Cmdtries = 11;
int Cmdack1;			/* Rx ACKs command, then do it */
int Exitcode;
size_t Lastsync;		/* Last offset to which we got a ZRPOS */
int Beenhereb4;			/* How many times we've been ZRPOS'd same place */

int no_timeout = FALSE;
size_t max_blklen = 1024;
size_t start_blklen = 0;
int zmodem_requested;
time_t stop_time = 0;

int error_count;

#define OVERHEAD 18
#define OVER_ERR 20

#define MK_STRING(x) #x

jmp_buf intrjmp;		/* For the interrupt on RX CAN */

static int io_mode_fd = 0;
static int zrqinits_sent = 0;
static int play_with_sigint = 0;

/* called by signal interrupt or terminate to clean things up */
void
bibi(int n, int *func_write(int, char *, int), void *func_oflush(void))
{
	canit(0, func_write);
	(*func_oflush) ();
	io_mode(io_mode_fd, 0);
	if (n == SIGQUIT)
		abort();
	exit(128 + n);
}

int Zctlesc;			/* Encode control characters */
int Zrwindow = 1400;		/* RX window size (controls garbage count) */

extern jmp_buf zmodemjmp;
int
bbs_zsendfile(char *filename, char *remote, int *func_write(int, char *, int),
	      void *func_oflush(void), int *func_read(int, char *, int))
{
	struct stat f;

	if (stat(filename, &f) != 0)
		return ERROR;
	Totalleft = f.st_size;
	Filesleft = 1;
	calc_blklen(Totalleft);
	protocol = ZM_ZMODEM;
	io_mode_fd = 0;
	blklen = start_blklen = 1024;
	if (setjmp(zmodemjmp) == 0) {
		zsendline_init();
		io_mode(io_mode_fd, 1);
		readline_setup(io_mode_fd, 128, 256);
		(*func_write) (0, "rz\r", 3);

		/*   TODO : throw away received input */

		purgeline(io_mode_fd);
		stohdr(0L);
		zshhdr(ZRQINIT, Txhdr, func_write, func_oflush);
		zrqinits_sent++;
		(*func_oflush) ();

		Crcflg = FALSE;
		firstsec = TRUE;
		bytcnt = (size_t) - 1;
		Totsecs = 0;

		if (wcs(filename, remote, func_write, func_oflush, func_read) ==
		    ERROR) {
			readline_clean();
			return ERROR;
		}
		if (zmodem_requested)
			saybibi(func_write, func_oflush, func_read);
		else if (protocol != ZM_XMODEM) {
			struct zm_fileinfo zi;
			char *pa;

			pa = (char *) alloca(PATH_MAX + 1);
			*pa = '\0';
			zi.fname = pa;
			zi.modtime = 0;
			zi.mode = 0;
			zi.bytes_total = 0;
			zi.bytes_sent = 0;
			zi.bytes_received = 0;
			zi.bytes_skipped = 0;
			wctxpn(&zi, func_write, func_oflush, func_read);
		}

		(*func_oflush) ();
		/* here needs a oflush */
		/* eat avalible input */
		/* better to eat some input here */
		io_mode(io_mode_fd, 0);
		readline_clean();
	} else {
		(*func_oflush) ();
		signal(SIGALRM, SIG_IGN);
		alarm(0);
		return ERROR;
	}
	return OK;
}

static int
wcs(const char *oname, const char *remotename,
    int *func_write(int, char *, int), void *func_oflush(void),
    int *func_read(int, char *, int))
{
#if !defined(S_ISDIR)
	int c;
#endif
	struct stat f;
	char *name;
	struct zm_fileinfo zi;

#ifdef HAVE_MMAP
	int dont_mmap_this = 0;
#endif

	if (Restricted) {
		/* restrict pathnames to current tree or uucppublic */
		if (strstr(oname, "../")
#ifdef PUBDIR
		    || (oname[0] == '/'
			&& strncmp(oname, MK_STRING(PUBDIR),
				   strlen(MK_STRING(PUBDIR))))
#endif
		    ) {
			canit(0, func_write);
			zmodem_error(1, 0,
				     "security violation: not allowed to upload from %s",
				     oname);
		}
	}

	if (0 == strcmp(oname, "-")) {
		char *p = getenv("ONAME");

		name = (char *) alloca(PATH_MAX + 1);
		if (p) {
			strcpy(name, p);
		} else {
			sprintf(name, "s%lu.lsz", (unsigned long) getpid());
		}
		input_f = stdin;
#ifdef HAVE_MMAP
		dont_mmap_this = 1;
#endif
	} else if ((input_f = fopen(oname, "r")) == NULL) {
		++errcnt;
		return OK;	/* pass over it, there may be others */
	} else {
		name = (char *) alloca(PATH_MAX + 1);
		strcpy(name, oname);
	}
#ifdef HAVE_MMAP
	if (!use_mmap || dont_mmap_this)
#endif
	{
		static char *s = NULL;
		static size_t last_length = 0;
		struct stat st;

		if (fstat(fileno(input_f), &st) == -1)
			st.st_size = 1024 * 1024;
		if (buffersize == (size_t) - 1 && s) {
			if ((size_t) st.st_size > last_length) {
				free(s);
				s = NULL;
				last_length = 0;
			}
		}
		if (!s && buffersize) {
			last_length = 16384;
			if (buffersize == (size_t) - 1) {
				if (st.st_size > 0)
					last_length = st.st_size;
			} else
				last_length = buffersize;
			/* buffer whole pages */
			last_length = (last_length + 4095) & 0xfffff000;
			s = malloc(last_length);
			if (!s) {
				exit(1);
			}
		}
		if (s) {
#ifdef SETVBUF_REVERSED
			setvbuf(input_f, _IOFBF, s, last_length);
#else
			setvbuf(input_f, s, _IOFBF, last_length);
#endif
		}
	}
	vpos = 0;
	/* Check for directory or block special files */
	fstat(fileno(input_f), &f);
#if defined(S_ISDIR)
	if (S_ISDIR(f.st_mode) || S_ISBLK(f.st_mode)) {
#else
	c = f.st_mode & S_IFMT;
	if (c == S_IFDIR || c == S_IFBLK) {
#endif
		fclose(input_f);
		return OK;
	}

	if (remotename) {
		/* disqualify const */
		union {
			const char *c;
			char *s;
		} cheat;

		cheat.c = remotename;
		zi.fname = cheat.s;
	} else
		zi.fname = name;
	zi.modtime = f.st_mtime;
	zi.mode = f.st_mode;
#if defined(S_ISFIFO)
	zi.bytes_total = (S_ISFIFO(f.st_mode)) ? DEFBYTL : f.st_size;
#else
	zi.bytes_total = c == S_IFIFO ? DEFBYTL : f.st_size;
#endif
	zi.bytes_sent = 0;
	zi.bytes_received = 0;
	zi.bytes_skipped = 0;
	zi.eof_seen = 0;

	++Filcnt;
	switch (wctxpn(&zi, func_write, func_oflush, func_read)) {
	case ERROR:
		return ERROR;
	case ZSKIP:
		return OK;
	}
	if (!zmodem_requested
	    && wctx(&zi, func_write, func_oflush, func_read) == ERROR) {
		return ERROR;
	}
	if (Unlinkafter)
		unlink(oname);

	return 0;
}

/*
 * generate and transmit pathname block consisting of
 *  pathname (null terminated),
 *  file length, mode time and file mode in octal
 *  as provided by the Unix fstat call.
 *  N.B.: modifies the passed name, may extend it!
 */
static int
wctxpn(struct zm_fileinfo *zi, int *func_write(int, char *, int),
       void *func_oflush(void), int *func_read(int, char *, int))
{
	register char *p, *q;
	char *name2;
	struct stat f;

	name2 = (char *) alloca(PATH_MAX + 1);

	if (protocol == ZM_XMODEM) {
		return OK;
	}
	if (!zmodem_requested)
		if (getnak(func_write, func_oflush, func_read)) {
			return ERROR;
		}

	q = (char *) 0;
#if 0
	if (Dottoslash) {	/* change . to . */
		for (p = zi->fname; *p; ++p) {
			if (*p == '/')
				q = p;
			else if (*p == '.')
				*(q = p) = '/';
		}
		if (q && strlen(++q) > 8) {	/* If name>8 chars */
			q += 8;	/*   make it .ext */
			strcpy(name2, q);	/* save excess of name */
			*q = '.';
			strcpy(++q, name2);	/* add it back */
		}
	}
#endif
	for (p = zi->fname, q = txbuf; *p;)
		if ((*q++ = *p++) == '/' && !Fullname)
			q = txbuf;
	*q++ = 0;
	p = q;
	while (q < (txbuf + MAX_BLOCK))
		*q++ = 0;
	/* note that we may lose some information here in case mode_t is wider than an 
	 * int. But i believe sending %lo instead of %o _could_ break compatability
	 */
	if (!Ascii && (input_f != stdin) && *zi->fname
	    && fstat(fileno(input_f), &f) != -1)
		sprintf(p, "%lu %lo %o 0 %d %ld", (long) f.st_size, f.st_mtime,
			(unsigned int) ((no_unixmode) ? 0 : f.st_mode),
			Filesleft, Totalleft);
	Totalleft -= f.st_size;
	if (--Filesleft <= 0)
		Totalleft = 0;
	if (Totalleft < 0)
		Totalleft = 0;

	/* force 1k blocks if name won't fit in 128 byte block */
	if (txbuf[125])
		blklen = 1024;
	else {			/* A little goodie for IMP/KMD */
		txbuf[127] = (f.st_size + 127) >> 7;
		txbuf[126] = (f.st_size + 127) >> 15;
	}
	if (zmodem_requested)
		return zsendfile(zi, txbuf, 1 + strlen(p) + (p - txbuf),
				 func_write, func_oflush, func_read);
	if (wcputsec(txbuf, 0, 128, func_write, func_oflush, func_read) ==
	    ERROR) {
		return ERROR;
	}
	return OK;
}

static int
getnak(int *func_write(int, char *, int), void *func_oflush(void),
       int *func_read(int, char *, int))
{
	int firstch;
	int tries = 0;

	Lastrx = 0;
	for (;;) {
		tries++;
		switch (firstch = READLINE_PF(100, func_read)) {
		case ZPAD:
			if (getzrxinit(func_write, func_oflush, func_read))
				return ERROR;
			Ascii = 0;	/* Receiver does the conversion */
			return FALSE;
		case TIMEOUT:
			/* 30 seconds are enough */
			if (tries == 3) {
				return TRUE;
			}
			/* don't send a second ZRQINIT _directly_ after the
			 * first one. Never send more then 4 ZRQINIT, because
			 * omen rz stops if it saw 5 of them */
			if ((zrqinits_sent > 1 || tries > 1)
			    && zrqinits_sent < 4) {
				/* if we already sent a ZRQINIT we are using zmodem
				 * protocol and may send further ZRQINITs 
				 */
				stohdr(0L);
				zshhdr(ZRQINIT, Txhdr, func_write, func_oflush);
				zrqinits_sent++;
			}
			continue;
		case WANTG:
			io_mode(io_mode_fd, 2);	/* Set cbreak, XON/XOFF, etc. */
			Optiong = TRUE;
			blklen = 1024;
		case WANTCRC:
			Crcflg = TRUE;
		case NAK:
			return FALSE;
		case CAN:
			if ((firstch = READLINE_PF(20, func_read)) == CAN
			    && Lastrx == CAN)
				return TRUE;
		default:
			break;
		}
		Lastrx = firstch;
	}
}

static int
wctx(struct zm_fileinfo *zi, int *func_write(int, char *, int),
     void *func_oflush(void), int *func_read(int, char *, int))
{
	register size_t thisblklen;
	register int sectnum, attempts, firstch;

	firstsec = TRUE;
	thisblklen = blklen;

	while ((firstch = READLINE_PF(Rxtimeout, func_read)) != NAK
	       && firstch != WANTCRC && firstch != WANTG && firstch != TIMEOUT
	       && firstch != CAN) ;
	if (firstch == CAN) {
		return ERROR;
	}
	if (firstch == WANTCRC)
		Crcflg = TRUE;
	if (firstch == WANTG)
		Crcflg = TRUE;
	sectnum = 0;
	for (;;) {
		if (zi->bytes_total <= (zi->bytes_sent + 896L))
			thisblklen = 128;
		if (!filbuf(txbuf, thisblklen))
			break;
		if (wcputsec
		    (txbuf, ++sectnum, thisblklen, func_write, func_oflush,
		     func_read) == ERROR)
			return ERROR;
		zi->bytes_sent += thisblklen;
	}
	fclose(input_f);
	attempts = 0;
	do {
		purgeline(io_mode_fd);
		sendline(EOT, func_write);
		flushmo();
		++attempts;
	} while ((firstch = (READLINE_PF(Rxtimeout, func_read)) != ACK)
		 && attempts < RETRYMAX);
	if (attempts == RETRYMAX) {
		return ERROR;
	} else
		return OK;
}

static int
wcputsec(char *buf, int sectnum, size_t cseclen,
	 int *func_write(int, char *, int), void *func_oflush(void),
	 int *func_read(int, char *, int))
{
	int checksum, wcj;
	char *cp;
	unsigned oldcrc;
	int firstch;
	int attempts;

	firstch = 0;		/* part of logic to detect CAN CAN */

	for (attempts = 0; attempts <= RETRYMAX; attempts++) {
		Lastrx = firstch;
		sendline(cseclen == 1024 ? STX : SOH, func_write);
		sendline(sectnum, func_write);
		sendline(-sectnum - 1, func_write);
		oldcrc = checksum = 0;
		for (wcj = cseclen, cp = buf; --wcj >= 0;) {
			sendline(*cp, func_write);
			oldcrc = updcrc((0377 & *cp), oldcrc);
			checksum += *cp++;
		}
		if (Crcflg) {
			oldcrc = updcrc(0, updcrc(0, oldcrc));
			sendline((int) oldcrc >> 8, func_write);
			sendline((int) oldcrc, func_write);
		} else
			sendline(checksum, func_write);

		flushmo();
		if (Optiong) {
			firstsec = FALSE;
			return OK;
		}
		firstch = READLINE_PF(Rxtimeout, func_read);
	      gotnak:
		switch (firstch) {
		case CAN:
			if (Lastrx == CAN) {
			      cancan:
				return ERROR;
			}
			break;
		case TIMEOUT:
			continue;
		case WANTCRC:
			if (firstsec)
				Crcflg = TRUE;
		case NAK:
			continue;
		case ACK:
			firstsec = FALSE;
			Totsecs += (cseclen >> 7);
			return OK;
		case ERROR:
			break;
		default:
			break;
		}
		for (;;) {
			Lastrx = firstch;
			if ((firstch = READLINE_PF(Rxtimeout, func_read)) ==
			    TIMEOUT) break;
			if (firstch == NAK || firstch == WANTCRC)
				goto gotnak;
			if (firstch == CAN && Lastrx == CAN)
				goto cancan;
		}
	}
	return ERROR;
}

/* fill buf with count chars padding with ^Z for CPM */
static size_t
filbuf(char *buf, size_t count)
{
	int c;
	size_t m;

	if (!Ascii) {
		m = read(fileno(input_f), buf, count);
		if (m <= 0)
			return 0;
		while (m < count)
			buf[m++] = 032;
		return count;
	}
	m = count;
	if (Lfseen) {
		*buf++ = 012;
		--m;
		Lfseen = 0;
	}
	while ((c = getc(input_f)) != EOF) {
		if (c == 012) {
			*buf++ = 015;
			if (--m == 0) {
				Lfseen = TRUE;
				break;
			}
		}
		*buf++ = c;
		if (--m == 0)
			break;
	}
	if (m == count)
		return 0;
	else
		while (m-- != 0)
			*buf++ = CPMEOF;
	return count;
}

/* Fill buffer with blklen chars */
static size_t
zfilbuf(struct zm_fileinfo *zi)
{
	size_t n;

	n = fread(txbuf, 1, blklen, input_f);
	if (n < blklen)
		zi->eof_seen = 1;
	else {
		/* save one empty paket in case file ends ob blklen boundary */
		int c = getc(input_f);

		if (c != EOF || !feof(input_f))
			ungetc(c, input_f);
		else
			zi->eof_seen = 1;
	}
	return n;
}

/*
 * Get the receiver's init parameters
 */
static int
getzrxinit(int *func_write(int, char *, int), void *func_oflush(void),
	   int *func_read(int, char *, int))
{
	static int dont_send_zrqinit = 1;
	int old_timeout = Rxtimeout;
	int n;
	struct stat f;
	size_t rxpos;
	int timeouts = 0;

	Rxtimeout = 100;	/* 10 seconds */
	/* XXX purgeline(io_mode_fd); this makes _real_ trouble. why? -- uwe */

	for (n = 10; --n >= 0;) {
		/* we might need to send another zrqinit in case the first is 
		 * lost. But *not* if getting here for the first time - in
		 * this case we might just get a ZRINIT for our first ZRQINIT.
		 * Never send more then 4 ZRQINIT, because
		 * omen rz stops if it saw 5 of them.
		 */
		if (zrqinits_sent < 4 && n != 10 && !dont_send_zrqinit) {
			zrqinits_sent++;
			stohdr(0L);
			zshhdr(ZRQINIT, Txhdr, func_write, func_oflush);
		}
		dont_send_zrqinit = 0;

		switch (zgethdr(Rxhdr, 1, &rxpos, func_read)) {
		case ZCHALLENGE:	/* Echo receiver's challenge numbr */
			stohdr(rxpos);
			zshhdr(ZACK, Txhdr, func_write, func_oflush);
			continue;
		case ZCOMMAND:	/* They didn't see our ZRQINIT */
			/* ??? Since when does a receiver send ZCOMMAND?  -- uwe */
			continue;
		case ZRINIT:
			Rxflags = 0377 & Rxhdr[ZF0];
			Rxflags2 = 0377 & Rxhdr[ZF1];
			Txfcs32 = (Wantfcs32 && (Rxflags & CANFC32));
			{
				int old = Zctlesc;

				Zctlesc |= Rxflags & TESCCTL;
				/* update table - was initialised to not escape */
				if (Zctlesc && !old)
					zsendline_init();
			}
			Rxbuflen =
			    (0377 & Rxhdr[ZP0]) + ((0377 & Rxhdr[ZP1]) << 8);
			if (!(Rxflags & CANFDX))
				Txwindow = 0;
			if (play_with_sigint)
				signal(SIGINT, SIG_IGN);
			io_mode(io_mode_fd, 2);	/* Set cbreak, XON/XOFF, etc. */
#ifndef READCHECK
			/* Use MAX_BLOCK byte frames if no sample/interrupt */
			if (Rxbuflen < 32 || Rxbuflen > MAX_BLOCK) {
				Rxbuflen = MAX_BLOCK;
			}
#endif
			/* Override to force shorter frame length */
			if (Tframlen && Rxbuflen > Tframlen)
				Rxbuflen = Tframlen;
			if (!Rxbuflen)
				Rxbuflen = 1024;

			/* If using a pipe for testing set lower buf len */
			fstat(0, &f);
#if defined(S_ISCHR)
			if (!(S_ISCHR(f.st_mode))) {
#else
			if ((f.st_mode & S_IFMT) != S_IFCHR) {
#endif
				Rxbuflen = MAX_BLOCK;
			}
			/*
			 * If input is not a regular file, force ACK's to
			 *  prevent running beyond the buffer limits
			 */
			/* COMMAN : we only deal with regular files */
#if 0
			fstat(fileno(input_f), &f);
#if defined(S_ISREG)
			if (!(S_ISREG(f.st_mode))) {
#else
			if ((f.st_mode & S_IFMT) != S_IFREG) {
#endif
				Canseek = -1;
				/* return ERROR; */
			}
#endif
			/* Set initial subpacket length */
			if (blklen < 1024) {	/* Command line override? */
				if (Baudrate > 300)
					blklen = 256;
				if (Baudrate > 1200)
					blklen = 512;
				if (Baudrate > 2400)
					blklen = 1024;
			}
			if (Rxbuflen && blklen > Rxbuflen)
				blklen = Rxbuflen;
			if (blkopt && blklen > blkopt)
				blklen = blkopt;
			Rxtimeout = old_timeout;
			return (sendzsinit(func_write, func_oflush, func_read));
		case ZCAN:
		case TIMEOUT:
			if (timeouts++ == 0)
				continue;	/* force one other ZRQINIT to be sent */
			return ERROR;
		case ZRQINIT:
			if (Rxhdr[ZF0] == ZCOMMAND)
				continue;
		default:
			zshhdr(ZNAK, Txhdr, func_write, func_oflush);
			continue;
		}
	}
	return ERROR;
}

/* Send send-init information */
static int
sendzsinit(int *func_write(int, char *, int), void *func_oflush(void),
	   int *func_read(int, char *, int))
{
	int c;

	if (Myattn[0] == '\0' && (!Zctlesc || (Rxflags & TESCCTL)))
		return OK;
	errors = 0;
	for (;;) {
		stohdr(0L);
		if (Zctlesc) {
			Txhdr[ZF0] |= TESCCTL;
			zshhdr(ZSINIT, Txhdr, func_write, func_oflush);
		} else
			zsbhdr(ZSINIT, Txhdr, func_write, func_oflush);
		ZSDATA(Myattn, 1 + strlen(Myattn), ZCRCW, func_write,
		       func_oflush);
		c = zgethdr(Rxhdr, 1, NULL, func_read);
		switch (c) {
		case ZCAN:
			return ERROR;
		case ZACK:
			return OK;
		default:
			if (++errors > 19)
				return ERROR;
			continue;
		}
	}
}

/* Send file name and related info */
static int
zsendfile(struct zm_fileinfo *zi, const char *buf, size_t blen,
	  int *func_write(int, char *, int), void *func_oflush(void),
	  int *func_read(int, char *, int))
{
	int c;
	unsigned long crc;
	size_t rxpos;

	/* we are going to send a ZFILE. There cannot be much useful
	 * stuff in the line right now (*except* ZCAN?). 
	 */
#if 0
	purgeline(io_mode_fd);	/* might possibly fix stefan glasers problems */
#endif

	for (;;) {
		Txhdr[ZF0] = Lzconv;	/* file conversion request */
		Txhdr[ZF1] = Lzmanag;	/* file management request */
		if (Lskipnocor)
			Txhdr[ZF1] |= ZF1_ZMSKNOLOC;
		Txhdr[ZF2] = Lztrans;	/* file transport request */
		Txhdr[ZF3] = 0;
		zsbhdr(ZFILE, Txhdr, func_write, func_oflush);
		ZSDATA(buf, blen, ZCRCW, func_write, func_oflush);
	      again:
		c = zgethdr(Rxhdr, 1, &rxpos, func_read);
		switch (c) {
		case ZRINIT:
			while ((c = READLINE_PF(50, func_read)) > 0)
				if (c == ZPAD) {
					goto again;
				}
			/* **** FALL THRU TO **** */
		default:
			continue;
		case ZRQINIT:	/* remote site is sender! */
			return ERROR;
		case ZCAN:
			return ERROR;
		case TIMEOUT:
			return ERROR;
		case ZABORT:
			return ERROR;
		case ZFIN:
			return ERROR;
		case ZCRC:
			crc = 0xFFFFFFFFL;
#ifdef HAVE_MMAP
			if (use_mmap && !mm_addr) {
				struct stat st;

				if (fstat(fileno(input_f), &st) == 0) {
					mm_size = st.st_size;
					mm_addr =
					    mmap(0, mm_size, PROT_READ,
						 MAP_SHARED, fileno(input_f),
						 0);
					if ((caddr_t) mm_addr == (caddr_t) - 1)
						mm_addr = NULL;
					else {
						fclose(input_f);
						input_f = NULL;
					}
				}
			}
			if (mm_addr) {
				size_t i;
				size_t count;
				char *p = mm_addr;

				count = (rxpos < mm_size
					 && rxpos > 0) ? rxpos : mm_size;
				for (i = 0; i < count; i++, p++) {
					crc = UPDC32(*p, crc);
				}
				crc = ~crc;
			} else
#endif
			if (rxpos == 0) {
				struct stat st;

				if (0 == fstat(fileno(input_f), &st)) {
					rxpos = st.st_size;
				} else
					rxpos = -1;
			}
			while (rxpos-- && ((c = getc(input_f)) != EOF))
				crc = UPDC32(c, crc);
			crc = ~crc;
			clearerr(input_f);	/* Clear EOF */
			fseek(input_f, 0L, 0);
			stohdr(crc);
			zsbhdr(ZCRC, Txhdr, func_write, func_oflush);
			goto again;
		case ZSKIP:
			if (input_f)
				fclose(input_f);
#ifdef HAVE_MMAP
			else if (mm_addr) {
				munmap(mm_addr, mm_size);
				mm_addr = NULL;
			}
#endif

			return c;
		case ZRPOS:
			/*
			 * Suppress zcrcw request otherwise triggered by
			 * lastsync==bytcnt
			 */
#ifdef HAVE_MMAP
			if (!mm_addr)
#endif
				if (rxpos && fseek(input_f, (long) rxpos, 0)) {
					return ERROR;
				}
			if (rxpos)
				zi->bytes_skipped = rxpos;
			bytcnt = zi->bytes_sent = rxpos;
			Lastsync = rxpos - 1;
			return zsendfdata(zi, func_write, func_oflush,
					  func_read);
		}
	}
}

/* Send the data in the file */
static int
zsendfdata(struct zm_fileinfo *zi, int *func_write(int, char *, int),
	   void *func_oflush(void), int *func_read(int, char *, int))
{
	static int c;
	int newcnt;
	static int junkcount;	/* Counts garbage chars received by TX */
	static long total_sent = 0;

#ifdef HAVE_MMAP
	if (use_mmap && !mm_addr) {
		struct stat st;

		if (fstat(fileno(input_f), &st) == 0) {
			mm_size = st.st_size;
			mm_addr =
			    mmap(0, mm_size, PROT_READ, MAP_SHARED,
				 fileno(input_f), 0);
			if ((caddr_t) mm_addr == (caddr_t) - 1)
				mm_addr = NULL;
			else {
				fclose(input_f);
				input_f = NULL;
			}
		}
	}
#endif

	Lrxpos = 0;
	junkcount = 0;
	Beenhereb4 = 0;
      somemore:
	if (setjmp(intrjmp)) {
	      waitack:
		junkcount = 0;
		c = getinsync(zi, 0, func_write, func_oflush, func_read);
	      gotack:
		switch (c) {
		default:
			if (input_f)
				fclose(input_f);
			return ERROR;
		case ZCAN:
			if (input_f)
				fclose(input_f);
			return ERROR;
		case ZSKIP:
			if (input_f)
				fclose(input_f);
			return c;
		case ZACK:
		case ZRPOS:
			break;
		case ZRINIT:
			return OK;
		}
#if 0

		/* COMMAN: tcp channel is reliable, so disable this */

#ifdef READCHECK
		/*
		 * If the reverse channel can be tested for data,
		 *  this logic may be used to detect error packets
		 *  sent by the receiver, in place of setjmp/longjmp
		 *  rdchk(fdes) returns non 0 if a character is available
		 */
		while (rdchk(io_mode_fd)) {
#ifdef READCHECK_READS
			switch (checked)
#else
			switch (READLINE_PF(1))
#endif
			{
			case CAN:
			case ZPAD:
				c = getinsync(zi, 1);
				goto gotack;
			case XOFF:	/* Wait a while for an XON */
			case XOFF | 0200:
				READLINE_PF(100);
			}
		}
#endif
#endif
	}

	newcnt = Rxbuflen;
	Txwcnt = 0;
	stohdr(zi->bytes_sent);
	zsbhdr(ZDATA, Txhdr, func_write, func_oflush);

	do {
		size_t n;
		int e;

		blklen = calc_blklen(total_sent);
		total_sent += blklen + OVERHEAD;
#ifdef HAVE_MMAP
		if (mm_addr) {
			if (zi->bytes_sent + blklen < mm_size)
				n = blklen;
			else {
				n = mm_size - zi->bytes_sent;
				zi->eof_seen = 1;
			}
		} else
#endif
			n = zfilbuf(zi);
		if (zi->eof_seen) {
			e = ZCRCE;
		} else if (junkcount > 3) {
			e = ZCRCW;
		} else if (bytcnt == Lastsync) {
			e = ZCRCW;
		} else if (Txwindow && (Txwcnt += n) >= Txwspac) {
			Txwcnt = 0;
			e = ZCRCQ;
		} else {
			e = ZCRCG;
		}
		ZSDATA(DATAADR, n, e, func_write, func_oflush);
		bytcnt = zi->bytes_sent += n;
		if (e == ZCRCW)
			goto waitack;
#ifdef READCHECK
		/*
		 * If the reverse channel can be tested for data,
		 *  this logic may be used to detect error packets
		 *  sent by the receiver, in place of setjmp/longjmp
		 *  rdchk(fdes) returns non 0 if a character is available
		 */
		oflush();
		while (rdchk(io_mode_fd)) {
#ifdef READCHECK_READS
			switch (checked)
#else
			switch (READLINE_PF(1, func_read))
#endif
			{
			case CAN:
			case ZPAD:
				c = getinsync(zi, 1);
				if (c == ZACK)
					break;
				/* zcrce - dinna wanna starta ping-pong game */
				ZSDATA(txbuf, 0, ZCRCE);
				goto gotack;
			case XOFF:	/* Wait a while for an XON */
			case XOFF | 0200:
				READLINE_PF(100, func_read);
			default:
				++junkcount;
			}
		}
#endif				/* READCHECK */
		if (Txwindow) {
			size_t tcount = 0;

			while ((tcount = zi->bytes_sent - Lrxpos) >= Txwindow) {
				if (e != ZCRCQ)
					ZSDATA(txbuf, 0, e =
					       ZCRCQ, func_write, func_oflush);
				c =
				    getinsync(zi, 1, func_write, func_oflush,
					      func_read);
				if (c != ZACK) {
					ZSDATA(txbuf, 0, ZCRCE, func_write,
					       func_oflush);
					goto gotack;
				}
			}
		}
	} while (!zi->eof_seen);

	if (play_with_sigint)
		signal(SIGINT, SIG_IGN);

	for (;;) {
		stohdr(zi->bytes_sent);
		zsbhdr(ZEOF, Txhdr, func_write, func_oflush);
		switch (getinsync(zi, 0, func_write, func_oflush, func_read)) {
		case ZACK:
			continue;
		case ZRPOS:
			goto somemore;
		case ZRINIT:
			return OK;
		case ZSKIP:
			if (input_f)
				fclose(input_f);
			return c;
		default:
			if (input_f)
				fclose(input_f);
			return ERROR;
		}
	}
}

static int
calc_blklen(long total_sent)
{
	static long total_bytes = 0;
	static int calcs_done = 0;
	static long last_error_count = 0;
	static int last_blklen = 0;
	static long last_bytes_per_error = 0;
	unsigned long best_bytes = 0;
	long best_size = 0;
	long this_bytes_per_error;
	long d;
	unsigned int i;

	if (total_bytes == 0) {
		/* called from countem */
		total_bytes = total_sent;
		return 0;
	}

	/* it's not good to calc blklen too early */
	if (calcs_done++ < 5) {
		if (error_count && start_blklen > 1024)
			return last_blklen = 1024;
		else
			last_blklen /= 2;
		return last_blklen = start_blklen;
	}

	if (!error_count) {
		/* that's fine */
		if (start_blklen == max_blklen)
			return start_blklen;
		this_bytes_per_error = LONG_MAX;
		goto calcit;
	}

	if (error_count != last_error_count) {
		/* the last block was bad. shorten blocks until one block is
		 * ok. this is because very often many errors come in an
		 * short period */
		if (error_count & 2) {
			last_blklen /= 2;
			if (last_blklen < 32)
				last_blklen = 32;
			else if (last_blklen > 512)
				last_blklen = 512;
		}
		last_error_count = error_count;
		last_bytes_per_error = 0;	/* force recalc */
		return last_blklen;
	}

	this_bytes_per_error = total_sent / error_count;
	/* we do not get told about every error, because
	 * there may be more than one error per failed block.
	 * but one the other hand some errors are reported more
	 * than once: If a modem buffers more than one block we
	 * get at least two ZRPOS for the same position in case
	 * *one* block has to be resent.
	 * so don't do this:
	 * this_bytes_per_error/=2;
	 */
	/* there has to be a margin */
	if (this_bytes_per_error < 100)
		this_bytes_per_error = 100;

	/* be nice to the poor machine and do the complicated things not
	 * too often
	 */
	if (last_bytes_per_error > this_bytes_per_error)
		d = last_bytes_per_error - this_bytes_per_error;
	else
		d = this_bytes_per_error - last_bytes_per_error;
	if (d < 4) {
		return last_blklen;
	}
	last_bytes_per_error = this_bytes_per_error;

      calcit:
	for (i = 32; i <= max_blklen; i *= 2) {
		long ok;	/* some many ok blocks do we need */
		long failed;	/* and that's the number of blocks not transmitted ok */
		unsigned long transmitted;

		ok = total_bytes / i + 1;
		failed = ((long) i + OVERHEAD) * ok / this_bytes_per_error;
		transmitted =
		    total_bytes + ok * OVERHEAD + failed * ((long) i +
							    OVERHEAD +
							    OVER_ERR);
		if (transmitted < best_bytes || !best_bytes) {
			best_bytes = transmitted;
			best_size = i;
		}
	}
	if (best_size > 2 * last_blklen)
		best_size = 2 * last_blklen;
	last_blklen = best_size;
	return last_blklen;
}

/*
 * Respond to receiver's complaint, get back in sync with receiver
 */
static int
getinsync(struct zm_fileinfo *zi, int flag, int *func_write(int, char *, int),
	  void *func_oflush(void), int *func_read(int, char *, int))
{
	int c;
	size_t rxpos;

	for (;;) {
		c = zgethdr(Rxhdr, 0, &rxpos, func_read);
		switch (c) {
		case ZCAN:
		case ZABORT:
		case ZFIN:
		case TIMEOUT:
			return ERROR;
		case ZRPOS:
			/* ************************************* */
			/*  If sending to a buffered modem, you  */
			/*   might send a break at this point to */
			/*   dump the modem's buffer.            */
			if (input_f)
				clearerr(input_f);	/* In case file EOF seen */
#ifdef HAVE_MMAP
			if (!mm_addr)
#endif
				if (fseek(input_f, (long) rxpos, 0))
					return ERROR;
			zi->eof_seen = 0;
			bytcnt = Lrxpos = zi->bytes_sent = rxpos;
			if (Lastsync == rxpos) {
				error_count++;
			}
			Lastsync = rxpos;
			return c;
		case ZACK:
			Lrxpos = rxpos;
			if (flag || zi->bytes_sent == rxpos)
				return ZACK;
			continue;
		case ZRINIT:
		case ZSKIP:
			if (input_f)
				fclose(input_f);
#ifdef HAVE_MMAP
			else if (mm_addr) {
				munmap(mm_addr, mm_size);
				mm_addr = NULL;
			}
#endif
			return c;
		case ERROR:
		default:
			error_count++;
			zsbhdr(ZNAK, Txhdr, func_write, func_oflush);
			continue;
		}
	}
}

/* Say "bibi" to the receiver, try to do it cleanly */
static void
saybibi(int *func_write(int, char *, int), void *func_oflush(void),
	int *func_read(int, char *, int))
{
	for (;;) {
		stohdr(0L);	/* CAF Was zsbhdr - minor change */
		zshhdr(ZFIN, Txhdr, func_write, func_oflush);	/*  to make debugging easier */
		switch (zgethdr(Rxhdr, 0, NULL, func_read)) {
		case ZFIN:
			sendline('O', func_write);
			sendline('O', func_write);
			flushmo();
		case ZCAN:
		case TIMEOUT:
			return;
		}
	}
}

/* End of lsz.c */
