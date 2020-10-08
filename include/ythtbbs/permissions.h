#ifndef __PERMISSIONS_H
#define __PERMISSIONS_H
#define PERM_BASIC      000001
#define PERM_CHAT       000002
#define PERM_PAGE       000004
#define PERM_POST       000010
#define PERM_LOGINOK    000020
#define PERM_DENYSIG    000040
#define PERM_CLOAK      000100
#define PERM_SEECLOAK   000200
#define PERM_XEMPT      000400
#define PERM_WELCOME    001000
#define PERM_BOARDS     002000
#define PERM_ACCOUNTS   004000
#define PERM_ARBITRATE  010000
#define PERM_OVOTE      020000
#define PERM_SYSOP      040000
#define PERM_POSTMASK  0100000
#define PERM_ANNOUNCE  0200000
#define PERM_OBOARDS   0400000
#define PERM_ACBOARD   01000000
#define PERM_NOZAP     02000000
#define PERM_FORCEPAGE 04000000
#define PERM_EXT_IDLE  010000000
#define PERM_SPECIAL1  020000000
#define PERM_SPECIAL2  040000000
#define PERM_SPECIAL3  0100000000
#define PERM_SPECIAL4  0200000000
#define PERM_SPECIAL5  0400000000
#define PERM_SPECIAL6  01000000000
#define PERM_SPECIAL7  02000000000
#define PERM_SPECIAL8  04000000000
#define PERM_DENYMAIL  010000000000
#define PERM_SELFLOCK  020000000000

/* This is the default permission granted to all new accounts. */
#define PERM_DEFAULT    (PERM_BASIC | PERM_CHAT | PERM_PAGE |\
                         PERM_POST | PERM_LOGINOK | PERM_SELFLOCK )

/* These permissions are bitwise ORs of the basic bits. They work that way
   too. For example, anyone with PERM_SYSOP or PERM_OBOARDS or both has
   PERM_BLEVELS. */

#define PERM_ADMINMENU  (PERM_ACCOUNTS|PERM_OVOTE|PERM_SYSOP|PERM_OBOARDS|PERM_WELCOME|PERM_ACBOARD)
#define PERM_MULTILOG   PERM_SYSOP
#define PERM_ESYSFILE   (PERM_SYSOP | PERM_WELCOME | PERM_ACBOARD)
#define PERM_LOGINCLOAK (PERM_SYSOP | PERM_ACCOUNTS | PERM_WELCOME | PERM_CLOAK)
#define PERM_SEEULEVELS (PERM_SYSOP | PERM_ARBITRATE | PERM_SEECLOAK)
#define PERM_BLEVELS    (PERM_SYSOP | PERM_OBOARDS)
#define PERM_MARKPOST   (PERM_SYSOP | PERM_BOARDS)
#define PERM_UCLEAN     (PERM_SYSOP | PERM_ACCOUNTS)
#define PERM_NOTIMEOUT  PERM_SYSOP
#define PERM_SPEC	(PERM_SYSOP | PERM_SPECIAL2 | PERM_ARBITRATE | PERM_SPECIAL2 | PERM_SPECIAL5 | PERM_SPECIAL6)

#define PERM_READMAIL   PERM_BASIC
#define PERM_VOTE       PERM_BASIC

/* These are used only in Internet Mail Forwarding */
/* You may want to be more restrictive than the default, especially for an
   open access BBS. */

#define PERM_SETADDR    PERM_POST	/* to set address for forwarding */
#define PERM_FORWARD    PERM_BASIC	/* to do the forwarding */

/* Don't mess with this. */
#define HAS_PERM(x, _currentuser)    (((x)&PERM_SPECIAL3)?(_currentuser.dietime):((x)?_currentuser.userlevel&(x):1))
#define HAS_CLUBRIGHT(x, clubrights) ((x)?(clubrights[x/32]&(1<<(x%32))):1)
#define DEFINE(x, _currentuser)     ((x)?_currentuser.userdefine&(x):1)
#define UNREAD(x, y) (((x)->edittime)?(brc_unreadt(y, (x)->edittime)):brc_unreadt(y, (x)->filetime))
#define SETREAD(x, y) (((x)->edittime)?(brc_addlistt(y, (x)->edittime)):brc_addlistt(y, (x)->filetime))

#define DEF_FRIENDCALL   000001
#define DEF_ALLMSG       000002
#define DEF_FRIENDMSG    000004
#define DEF_SOUNDMSG     000010
#define DEF_COLOR        000020
#define DEF_ACBOARD      000040
#define DEF_ENDLINE      000100
#define DEF_EDITMSG      000200
#define DEF_NOTMSGFRIEND 000400
#define DEF_NORMALSCR    001000
#define DEF_NEWPOST      002000
#define DEF_CIRCLE       004000
#define DEF_FIRSTNEW     010000
#define DEF_LOGFRIEND    020000
#define DEF_INNOTE       040000
#define DEF_OUTNOTE      0100000
#define DEF_MAILMSG      0200000
#define DEF_LOGOUT       0400000
#define DEF_SEEWELC1     01000000
#define DEF_LOGINFROM    02000000
#define DEF_NOTEPAD      04000000
#define DEF_NOLOGINSEND  010000000
#define DEF_THESIS       020000000	/* youzi */
#define DEF_MSGGETKEY    040000000
#define DEF_DELDBLCHAR   0100000000	/*  KCN */
#define DEF_USEGB        0200000000	/*  KCN  */
#define DEF_ANIENDLINE   0400000000	/*  ylsdd */
#define DEF_INTOANN      01000000000	/* ylsdd */
#define DEF_POSTNOMSG    02000000000	/* yuhuan */
#define DEF_SEESTATINLOG 04000000000
#define	DEF_FILTERXXX	 010000000000
//#define DEF_INTERNETMAIL   020000000000
#define DEF_NEWSTOP10    020000000000 //add by bjgyt
#define NUMDEFINES 32
#endif
