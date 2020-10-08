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
    Copyright (C) 1999, Zhou Lin, kcn@cic.tsinghua.edu.cn

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#ifndef _BBS_H_
#define _BBS_H_

#ifndef BBSIRC

/* Global includes, needed in most every source file... */
#include "ythtbbs/ythtbbs.h"
#include "config.h"
// TODO: Core 里头不应该判断是否存在 FCGI 的实现吧
#ifndef ENABLE_FASTCGI
#include <stdio.h>
#else
#include <fcgi_stdio.h>
#endif
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>
#include <signal.h>
#include <fcntl.h>
#include <utime.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <malloc.h>

#ifdef lint
#include <sys/uio.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/param.h>
#include <stdarg.h>
#include <sys/resource.h>
#include <pwd.h>

#include <sys/wait.h>
#include <netinet/tcp.h>
#include <arpa/telnet.h>

#ifdef AIX
#include <sys/select.h>
#endif

#include <sys/ipc.h>
#include <sys/shm.h>

#if defined(BSD44)
#include <stdlib.h>

#elif defined(LINUX)
/* include nothing :-) */
#else

#include <rpcsvc/rstat.h>
#endif


#include "config.h"             /* User-configurable stuff */

#ifdef BBSMAIN
#define perror prints
#endif

#define VERSION_ID "FIREBIRD 3.0K"

#ifndef LOCK_EX
#define LOCK_EX         2       /* exclusive lock */
#define LOCK_UN         8       /* unlock */
#endif

#ifdef XINU
extern int errno ;
#endif

#define randomize() srand((unsigned)time(NULL))

#define YEA (1)        /* Booleans  (Yep, for true and false) */
#define NA  (0)

#define DOECHO (1)     /* Flags to getdata input function */
#define NOECHO (0)

#endif /* BBSIRC */

#define GOOD_BRC_NUM    40      // 最多有 GOOD_BRC_NUM 个个人定制版面
#define NUMPERMS   (31)

#define MAX_DIGEST         1000    /* 最大文摘数 */
#define MAX_POSTRETRY       100

#ifndef BBSIRC

#define QUIT 0x666               /* Return value to abort recursive functions */

#define VOTE_FLAG    0x1
#define NOZAP_FLAG   0x2
#define OUT_FLAG     0x4
#define ANONY_FLAG   0x8
#define CLUBTYPE_FLAG 0x10
#define INNBBSD_FLAG 0x20
#define IS1984_FLAG    0x40
#define POLITICAL_FLAG	0x80
#define ZAPPED  0x1           /* For boards...tells if board is Zapped */

/*add by macintosh 050530 for semi-closed club */
#define CLOSECLUB_FLAG 0x4  /* 表示版面是否可见, 1 for close club , 0 for open club */

#define CLUB_FLAG (CLOSECLUB_FLAG | CLUBTYPE_FLAG) /*俱乐部*/

/* these are flags in userec.flags[0] */
#define PAGER_FLAG   0x1   /* true if pager was OFF last session */
#define CLOAK_FLAG   0x2   /* true if cloak was ON last session */
#define SIG_FLAG     0x8   /* true if sig was turned OFF last session */
#define BRDSORT_FLAG2 0x10 /* true if the boards sorted by score */
//#define BRDSORT_FLAG 0x20  /* true if the boards sorted alphabetical, */
			   /* available only if FLAG2 is false */
#define BRDSORT_MASK 0x30
#define CURSOR_FLAG  0x80  /* true if the cursor mode open */
#define ACTIVE_BOARD 0x200 /* true if user toggled active movie board on */

/* For All Kinds of Pagers */
#define ALL_PAGER       0x1
#define FRIEND_PAGER    0x2
#define ALLMSG_PAGER    0x4
#define FRIENDMSG_PAGER 0x8

#define SHIFTMODE(usernum,mode) ((usernum<MAXUSERS)?mode:mode<<4)

#define SETFILEMODE(array,usernum,mode) \
     (array[usernum%MAXUSERS] |= ((usernum<MAXUSERS)?mode:mode<<4))

#define CLRFILEMODE(array,usernum,mode) \
          (array[usernum%MAXUSERS] &= ((usernum<MAXUSERS)?~mode:~(mode<<4)))

#define CHECKFILEMODE(array,usernum,mode) \
       (array[usernum%MAXUSERS] & ((usernum<MAXUSERS)?mode:mode<<4))
#define USERIDSIZE (16)
#define USERNAMESZ (24)
#define TERMTYPESZ (10)
/* END */

#endif /* BBSIRC */
/*#ifndef BBSIRC*/

#define DONOTHING       0       /* Read menu command return states */
#define FULLUPDATE      1       /* Entire screen was destroyed in this oper*/
#define PARTUPDATE      2       /* Only the top three lines were destroyed */
#define DOQUIT          3       /* Exit read menu was executed */
#define NEWDIRECT       4       /* Directory has changed, re-read files */
#define READ_NEXT       5       /* Direct read next file */
#define READ_PREV       6       /* Direct read prev file */
#define GOTO_NEXT       7       /* Move cursor to next */
#define DIRCHANGED      8       /* Index file was changed */
#define UPDATETLINE	9	/* t_lines was changed */

#define I_TIMEOUT   (-2)         /* Used for the getchar routine select call */
#define I_OTHERDATA (-333)       /* interface, (-3) will conflict with chinese */

#define SCREEN_SIZE (23)         /* Used by read menu  */
extern int scrint ;               /* Set when screen has been initialized */
                                  /* Used by namecomplete *only* */

extern int digestmode;            /*To control Digestmode*/

/*#endif*/ /* !BBSIRC */

#ifndef EXTEND_KEY
#define EXTEND_KEY
#define KEY_TAB         9
#define KEY_ESC         27
#define KEY_UP          0x0101
#define KEY_DOWN        0x0102
#define KEY_RIGHT       0x0103
#define KEY_LEFT        0x0104
#define KEY_HOME        0x0201
#define KEY_INS         0x0202
#define KEY_DEL         0x0203
#define KEY_END         0x0204
#define KEY_PGUP        0x0205
#define KEY_PGDN        0x0206
#endif

#define Ctrl(c)         ( c & 037 )
#define isprint2(c)     ( (c & 0x80) || isprint(c) )

/* =============== ANSI EDIT ================== */
#define   ANSI_RESET    "\033[0m"
#define   ANSI_REVERSE  "\033[7m\033[4m"
extern    int  editansi;
extern    int  KEY_ESC_arg;
/* ============================================ */


/* pty exec */
#ifdef CAN_EXEC
#if defined(CONF_HAVE_OPENPTY)
#include <pty.h>
#endif
#include "tmachine.h"
#include <utmp.h>

extern int tmachine_init(int net);
extern int nread(int net,void *pbuf,int size);
extern int nwrite(int net,const void *pbuf,int size);
extern int nload(const char *file);
extern int nsave(const char *file);

extern queue_tl qneti,qneto;
void wlogin(const char *line, const char *name, const char *host);
int wlogout(const char *line);
extern int term_cols,term_lines;
extern char term_type[64];
extern int term_convert;
#endif

#define NUMBUFFER 20

#define SR_BMMINUSDEL    (18)       //add by mintbaggio@BMY for minus-numposts delete

enum {
	BMY_CHARSET_GBK,
	BMY_CHARSET_UTF8
};

#endif /* of _BBS_H_ */
