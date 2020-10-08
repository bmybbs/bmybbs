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

    Copyright (C) 1998, Zhou lin, kcn@cic.tsinghua.edu.cn

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#ifndef BBS_CONFIG
#define BBS_CONFIG

/* FIREBIRD BBS 3.0K CONFIGURATION VARIABLES */

/*
   Define this to enable wide character support for Chinese or other
   languages not using the ANSI character set. Comment out if this isn't
   needed.
*/
#define BIT8

/*
   You may define a default board here, already selected when a user logs in.
   If this is set to the empty string, the bbs will require users to (S)elect
   a board before reading or posting. Don't comment this out.
*/
#define DEFAULTBOARD    "sysop"

/*
   You get this many chances to give a valid userid/password combination
   before the bbs squawks at you and closes the connection.
   Don't comment out.
*/
#define LOGINATTEMPTS 3

/*
   Turn this on to allow users to create their own accounts by typing 'new'
   at the "Enter userid:" prompt. Comment out to restrict access to accounts
   created by the Sysop (see important note in README.install).
*/
#define LOGINASNEW 1

/* Defined means when BM delete a post, the posting record of the owner will
   decrease by 1 */
#define BMDEL_DECREASE 1

/*
   Defined means user may log as as many times as they wish, simultaneously.
   On a larger bbs with many window-users you probably want this off.
   Comment out to disable.
#define MULTI_LOGINS 1
*/

/*
Comment this out to disable the Voting stuff. Dunno why, it's not a resource
hog or anything, but if you don't want it...
*/
#define VOTE    1

/*
   Define this if you want the Internet Post/Mail Forwarding features
   enabled. You MUST have the sendmail(8) program on your system and
   a cron job to run the bbs mail queue every so often. The bbs does not
   invoke sendmail, it simply creates the queue and data files for it.
*/
#define INTERNET_EMAIL 1

#ifdef INTERNET_EMAIL
/* If you defined INTERNET_EMAIL, you have the option of telling the bbs
   an address where failed forwarded messages can go to (people will
   invariably set their addresses wrong!). If this is commented out bounces
   will go to the bbs mailbox in the usual mail spool.
*/
#define ERRORS_TO "root"
#endif  /* INTERNET_EMAIL */

/*
   Define DOTIMEOUT to set a timer to log out users who sit idle on the system.
   Then decide how long to let them stay: MONITOR_TIMEOUT is the time in
   seconds a user can sit idle in Monitor mode; IDLE_TIMEOUT applies to all
   other modes.
*/
#define DOTIMEOUT 1

/*
   These are moot if DOTIMEOUT is commented; leave them defined anyway.
*/
#define IDLE_TIMEOUT    (60*50)
#define LOGIN_TIMEOUT   (60*1)

/*
   By default, users with post permission can reply to a post right after
   they read it. This can lead to lots of silly replies, hence the reply
   feature is a little controversial. Uncomment this to disable Reply.
*/
/*
#define NOREPLY 1
*/
/*
   The Eagle's Nest has problems with apparently random corruption of the
   board and mail directories. Other boards don't have this problem. Anyway,
   this ifdef enables some code that decreases this problem. you probably
   won't need it, but...
*/
/* #define POSTBUG      1 */

/*
   Define this if you are going to install an Internet Relay Chat client
   and/or server. Either the one with this distribution or a newer one.
*/
#define IRC   1

/****************************************************************************/

#define MAXUSERS  100000  /* Maximum number of users,780 aliament 1k*/
/* modified by clearboy@BMY@20050511*/
#define MAXBOARD  800  /* Maximum number of boards */
#define MAXACTIVE 5000
                          /* Max users allowed on the system at once. Set this
                             to a reasonable value based on the resources of
                             your system. */
#define MAXACTIVERUN 5000
			  /* */

#define MAXSIGLINES    6  /* max. # of lines appended for post signature */
#define MAXQUERYLINES 16  /* max. # of lines shown by the Query function */
#define MAX_MAIL_HOLD (100)
#define MAX_SYSOPMAIL_HOLD (10000)

#define POP_CHECK 1
/* Once you set this, do not change it unless you are restarting your
   bbs from scratch! The PASSWDS file will not work if this is changed.
   If commented out, no real name/address info can be kept in the passwd
   file. Pretty useless to have this if LOGINASNEW is defined. */

/*
Define this to show real names to SYSOP in the Config Menu user list.
*/
#define ACTS_REALNAMES       1

/*
Define this for Query to show real names.
*/
#define QUERY_REALNAMES      1

/*
Define this to use Notepad.
*/
#define USE_NOTEPAD      1

/*
Define this to stamp mark on edited post
*/
#define ADD_EDITMARK     1

/*
Define this if you have Sendmail 8.8.x that supports MIME autoconvert
You must also set "EightBitMode=pass8" in /etc/sendmail.cf
*/
#define SENDMAIL_MIME_AUTOCONVERT    1
#define NOREMOTECHAT

#define MY_BBS_HOME   "/home/bbs"
#define MY_BBS_NAME   "\xB1\xF8\xC2\xED\xD9\xB8\x42\x42\x53"  /* 兵马俑BBS */
#define MY_BBS_DOMAIN "bbs.xjtu.edu.cn"
#define MY_BBS_IP     "202.117.1.8"
#define MY_BBS_ID     "BMY"
#define MY_BBS_LOC    "\xCE\xF7\xB0\xB2\xBD\xBB\xCD\xA8\xB4\xF3\xD1\xA7" /* 西安交通大学 */
#define BBSUSER	      "bbs"
#define BBSGID	      999
#define BBSUID	      999
#define BBS_PORT      23
#define BBS_BIG5_PORT 2300
#define SMAGIC	      "BMY"

#define CONVTABLE_SHMKEY   3101
#define BCACHE_SHMKEY      7813
#define UCACHE_SHMKEY      7912
#define UCACHE_HASH_SHMKEY 7911
#define UTMP_SHMKEY        3785
#define UTMP_HASH_SHMKEY   3786
#define UINDEX_SHMKEY      3787
#define WWWCACHE_SHMKEY   37788
#define ACBOARD_SHMKEY     9014
#define GOODBYE_SHMKEY     5003
#define WELCOME_SHMKEY     5004
#define ENDLINE1_SHMKEY    5006
#define STAT_SHMKEY        5100

#define BBSLOG_MSQKEY 3333
#define BBSEVA_MSQKEY 3334

#define PASSFILE    ".PASSWDS"              /* Name of file User records stored in */
#define ULIST_BASE  ".UTMP"                 /* Names of users currently on line */
#define FLUSH       ".PASSFLUSH"            /* Stores date for user cache flushing */
#define BOARDS      ".BOARDS"               /* File containing list of boards */
#define DOT_DIR     ".DIR"                  /* Name of Directory file info */
#define THREAD_DIR  ".THREAD"               /* Name of Thread file info */
#define DIGEST_DIR  ".DIGEST"               /* Name of Digest file info */
#define TOPFILE_DIR ".TOPFILE"              //hace
#define BADWORDS    "etc/.badwords_new"     /* word list to filter */
#define SBADWORDS   "etc/.sbadwords_new"
#define PBADWORDS   "etc/.pbadwords_new"
#define APPS		".APPS"                 /* File containing list of apps */

/*
 * 有关 app 的限制
 */
#define MAXSECRETKEYLENGTH	8
#define APPKEYLENGTH		36
#define APPNAMELENGTH		20
#define APPPID				-1

#ifndef ATTACHCACHE
#define ATTACHCACHE "/home/bbsattach/cache"
#endif  /* ATTACHCACHE */

#define STRLEN               80	/* Length of most string data */
#define NAMELEN              40	/* Length of username/realname */
#define IDLEN                12	/* Length of userids */
#define PASSLEN              14	/* Length of encrypted passwd field */
#define MAXMESSAGE            5
#define BADLOGINFILE "logins.bad"
#define PATHTMPBRC MY_BBS_HOME "/bbstmpfs/brc"
#define PATHUSERATTACH MY_BBS_HOME "/bbstmpfs/userattach"
#define PATHZMODEM MY_BBS_HOME "/bbstmpfs/zmodem"

/**
 * INET6_ADDRSTRLEN 46 longest: 0fff:1fff:2fff:3fff:4fff:5fff:255.255.255.255
 * BMY_IPV6 either "0fff:1fff:2fff:3fff:4fff:5fff:6fff:7fff" or "255.255.255.255"
 */
#define BMY_IPV6_LEN (40)
#define MAXFRIENDS (200)
#define MAXREJECTS (32)
#define TOKENLENGTH 8
#define MAXATTACHSIZE (10000000)
#define MAXPICSIZE (1500000) // 图片最大为500kb
#define MAX_MENTION_ID (20)  /* 最大允许 @ 的 id 个数 */
#define BM_LEN 60

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

#endif  /* BBS_CONFIG */
