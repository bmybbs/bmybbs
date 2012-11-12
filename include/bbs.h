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

#ifndef  _BBS_H_
#define _BBS_H_

#ifndef BBSIRC

/* Global includes, needed in most every source file... */
#include "ythtbbs.h"
#include "config.h"
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

/*char *strdup() ;        External function declarations */
char *bfile() ;

extern FILE *ufp ;     /* External variable declarations */
extern long ti ;

#endif /* BBSIRC */

#define MAXFRIENDS (200)
#define MAXREJECTS (32)
#define GOOD_BRC_NUM    40      // 最多有 GOOD_BRC_NUM 个个人定制版面
#define NUMPERMS   (31)
//#define REG_EXPIRED         180    /* 重做身份确认期限 */

#define FILE_BUFSIZE        250    /* max. length of a file in SHM*/
#define FILE_MAXLINE         25    /* max. line of a file in SHM */
#define MAX_WELCOME          15    /* 欢迎画面数 */
#define MAX_GOODBYE          15    /* 离站画面数 */
#define MAX_ISSUE            15    /* 最大进站画面数 */
#define MAX_ENDLINE		15 /* 最大底线叶面数 */
#define MAX_DIGEST         1000    /* 最大文摘数 */
#define MAX_POSTRETRY       100
#define MAXITEMS        1024       /* 精华区最大条目数 */
#define CLUB_SIZE       4          /* 4 * sizeof(int) 为完全 close club数目上限 . by clearboy*/
#define MAXnettyLN            7    /* lines of  activity board  */        
#define ACBOARD_BUFSIZE     255    /* max. length of each line for activity board  */
#define ACBOARD_MAXLINE      400    /* max. lines of  activity board  160 */
#define MAXGOPHERITEMS     9999    /*max of gopher items*/
#define PASSFILE     ".PASSWDS"    /* Name of file User records stored in */
#define ULIST_BASE   ".UTMP"       /* Names of users currently on line */
extern  char ULIST[];

#ifndef BBSIRC 

#define FLUSH       ".PASSFLUSH"   /* Stores date for user cache flushing */
#define BOARDS      ".BOARDS"      /* File containing list of boards */
#define DOT_DIR     ".DIR"         /* Name of Directory file info */
#define THREAD_DIR  ".THREAD"      /* Name of Thread file info */
#define DIGEST_DIR  ".DIGEST"      /* Name of Digest file info */
#define TOPFILE_DIR  ".TOPFILE"    //hace 
#define BADWORDS    "etc/.badwords_new"     /* word list to filter */
#define SBADWORDS   "etc/.sbadwords_new"
#define PBADWORDS   "etc/.pbadwords_new"

#define QUIT 0x666               /* Return value to abort recursive functions */

#if 1
#define FILE_READ  0x1        /* Ownership flags used in fileheader structure */
#define FILE_OWND  0x2        /* accessed array */
#define FILE_VISIT 0x4
#define FILE_MARKED 0x8
#define FILE_DIGEST 0x10      /* Digest Mode*/
#define FILE_FORWARDED 0x20	/* undelete */
#define FILE_NOREPLY 0x40
#define FILE_ATTACHED 0x80   
#define FILE1_DEL 0x1        /* Marked for being deleted */ /*accessed array, the 2nd byte*/
#define FILE1_SPEC 0x2	     /* Will be put to 0Announce, and this flag would be clear then */
#define FILE1_INND 0x4	/* write into innd/out.bntp */
#define FILE1_ANNOUNCE 0x8  /* have put to 0Announce */
#define FILE1_1984 	0x10 /* have been checked to see if there is any ... */
#define MAIL_REPLY 0x20
#endif

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

#include "struct.h"
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
extern struct userec currentuser ;  /*  user structure is loaded from passwd */
extern int clubrights[4];
                                  /*  file at logon, and remains for the   */
                                  /*  entire session */

extern struct user_info uinfo ;   /* Ditto above...utmp entry is stored here
                                     and written back to the utmp file when
                                     necessary (pretty darn often). */ 
extern int usernum ;      /* Index into passwds file user record */
extern int utmpent ;      /* Index into this users utmp file entry */
extern int count_friends,count_users; /*Add by SmallPig for count users and friends*/

extern int t_lines, t_realcols,t_columns;    /* Screen size / width */
extern struct userec lookupuser ; /* Used when searching for other user info */

extern int         nettyNN;
extern char netty_path[] ; 
extern char netty_board[] ; 
extern char currboard[] ;       /* name of currently selected board */
extern struct bm * currBM[] ;          /* BM of currently selected board */

extern int selboard ;           /* THis flag is true if above is active */

extern char genbuf[1024] ;      /* generally used global buffer */

extern jmp_buf byebye ;        /* Used for exception condition like I/O error*/

extern struct commands * xyzlist[] ;   /* These are command lists for all the */
extern struct commands * talklist[] ;  /* sub-menus */
extern struct commands * maillist[] ;
extern struct commands * dellist[] ;
extern struct commands * maintlist[] ;

extern char save_title[] ;    /* These are used by the editor when inserting */
extern char save_filename[] ; /* header information */
extern int in_mail ;
extern int dumb_term ;
extern int showansi;

extern int t_big5;         //added by zhoulin from the furture
extern sigjmp_buf bus_jump;
/*#endif*/ /* !BBSIRC */

/*SREAD Define*/
#define SR_BMBASE       (10)
#define SR_BMDEL	(11)
#define SR_BMMARK       (12)
#define SR_BMDIGEST     (13)
#define SR_BMIMPORT     (14)
#define SR_BMTMP        (15)
#define SR_BMNOREPLY    (16)
#define SR_BMCOMBINE    (17)
/*SREAD Define*/

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

#ifdef  SYSV
#define bzero(tgt, len)         memset( tgt, 0, len )
#define bcopy(src, tgt, len)    memcpy( tgt, src, len)

#define usleep(usec)            {               \
    struct timeval t;                           \
    t.tv_sec = usec / 1000000;                  \
    t.tv_usec = usec % 1000000;                 \
    select( 0, NULL, NULL, NULL, &t);           \
}

#endif  /* SYSV */

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
#endif /* of _BBS_H_ */

#define COMMENDFILE       MY_BBS_HOME"/.COMMEND"
#define COMMENDFILE2       MY_BBS_HOME"/.COMMEND2"

