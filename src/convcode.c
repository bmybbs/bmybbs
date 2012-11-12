/*
    Firebird Bulletin Board System
    Copyright (C) 1999, KCN,Zhou Lin, kcn@cic.tsinghua.edu.cn
    
    get some function from hztty.:)
    
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

#define BtoGtablefile "etc/b2g_table"
#define GtoBtablefile "etc/g2b_table"

#define	BtoG_bad1 0xa1
#define	BtoG_bad2 0xf5
#define	GtoB_bad1 0xa1
#define	GtoB_bad2 0xbc

unsigned char *GtoB, *BtoG;
#define GtoB_count 7614
#define BtoG_count 13973
char gb2big_savec[2];
char big2gb_savec[2];

#ifndef PTY_EXEC
extern int convcode;

static void resolv_file(char *buf);
static void *attach_shm2(int shmkey, int shmsize, void (*resolv_file) (char *));
static void g2b(register unsigned char *s);
static void b2g(register unsigned char *s);
static char *hzconvert(char *s, int *plen, char *psaved, void (*dbcvrt) (void));

void
switch_code()
{
	convcode = !convcode;
	redoscr();
}
#endif

static void
resolv_file(char *buf)
{
	int fd;
	int i;
	fd = open(BtoGtablefile, O_RDONLY);
	if (fd == -1)
		for (i = 0; i < BtoG_count; i++) {
			buf[i * 2] = BtoG_bad1;
			buf[i * 2 + 1] = BtoG_bad2;
	} else {
		read(fd, buf, BtoG_count * 2);
		close(fd);
	}
	fd = open(GtoBtablefile, O_RDONLY);
	if (fd == -1)
		for (i = 0; i < GtoB_count; i++) {
			buf[BtoG_count * 2 + i * 2] = GtoB_bad1;
			buf[BtoG_count * 2 + i * 2 + 1] = GtoB_bad2;
	} else {
		read(fd, buf + BtoG_count * 2, GtoB_count * 2);
		close(fd);
	}
}

#ifdef PTY_EXEC
int
attach_err(key, str)
int key;
char *str;
{
}
#endif

static void *
attach_shm2(shmkey, shmsize, resolv_file)
int shmkey;
int shmsize;
void (*resolv_file) (char *);
{
	void *shmptr;
	int shmid;

	shmid = shmget(shmkey, shmsize, 0);
	if (shmid < 0) {
		shmid = shmget(shmkey, shmsize, IPC_CREAT | 0600);
		if (shmid < 0)
			attach_err(shmkey, "shmget");
		shmptr = (void *) shmat(shmid, NULL, 0);
		if (shmptr == (void *) -1)
			attach_err(shmkey, "shmat");
		resolv_file(shmptr);
	} else {
		shmptr = (void *) shmat(shmid, NULL, 0);
		if (shmptr == (void *) -1)
			attach_err(shmkey, "shmat");
	}
	return shmptr;
}

void
conv_init()
{
	BtoG =
	    attach_shm2(CONVTABLE_SHMKEY, GtoB_count * 2 + BtoG_count * 2,
			resolv_file);
	GtoB = BtoG + BtoG_count * 2;
	gb2big_savec[0] = 0;
	big2gb_savec[0] = 0;
	gb2big_savec[1] = 0;
	big2gb_savec[1] = 0;
}

#define	c1	(unsigned char)(s[0])
#define	c2	(unsigned char)(s[1])

static void
g2b(s)
register unsigned char *s;
{
	register unsigned int i;

	if ((c2 >= 0xa1) && (c2 <= 0xfe)) {
		if ((c1 >= 0xa1) && (c1 <= 0xa9)) {
			i = ((c1 - 0xa1) * 94 + (c2 - 0xa1)) * 2;
			s[0] = GtoB[i++];
			s[1] = GtoB[i];
			return;
		} else if ((c1 >= 0xb0) && (c1 <= 0xf7)) {
			i = ((c1 - 0xb0 + 9) * 94 + (c2 - 0xa1)) * 2;
			s[0] = GtoB[i++];
			s[1] = GtoB[i];
			return;
		}
	}
	s[0] = GtoB_bad1;
	s[1] = GtoB_bad2;
}

static void
b2g(s)
register unsigned char *s;
{
	register int i;

	if ((c1 >= 0xa1) && (c1 <= 0xf9)) {
		if ((c2 >= 0x40) && (c2 <= 0x7e)) {
			i = ((c1 - 0xa1) * 157 + (c2 - 0x40)) * 2;
			s[0] = BtoG[i++];
			s[1] = BtoG[i];
			return;
		} else if ((c2 >= 0xa1) && (c2 <= 0xfe)) {
			i = ((c1 - 0xa1) * 157 + (c2 - 0xa1) + 63) * 2;
			s[0] = BtoG[i++];
			s[1] = BtoG[i];
			return;
		}
	}
	s[0] = BtoG_bad1;
	s[1] = BtoG_bad2;
}

#undef c1
#undef c2

static char *
hzconvert(s, plen, psaved, dbcvrt)
char *s;
int *plen;
char *psaved;			/* unprocessed char buffer pointer */
void (*dbcvrt) ();		/* 2-byte conversion func for a hanzi */
{
	char *p, *pend;

	if (*plen == 0)
		return (s);
	if (*psaved) {		/* previous buffered char */
		*(--s) = *psaved;	/* put the unprocessed char down */
		(*plen)++;
		*psaved = 0;	/* clean this char buffer */
	}
	p = s;
	pend = s + (*plen);	/* begin/end of the buffer string */
	while (p < pend) {
		if ((*p) & 0x80) {	/* hi-bit on: hanzi */
			if (p < pend - 1)	/* not the last one */
				dbcvrt(p++);
			else {	/* the end of string */
				*psaved = *p;	/* save the unprocessed char */
				(*plen)--;
				break;
			}
		}
		p++;
	}
	return (s);
}

char *
gb2big(s, plen, inst)
char *s;
int *plen;
int inst;
{
	return (hzconvert(s, plen, &gb2big_savec[inst], (void *) g2b));
}

char *
big2gb(s, plen, inst)
char *s;
int *plen;
int inst;
{
	return (hzconvert(s, plen, &big2gb_savec[inst], (void *) b2g));
}
