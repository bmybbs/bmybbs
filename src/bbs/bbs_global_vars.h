#ifndef BMYBBS_BBS_BBSTELNET
#define BMYBBS_BBS_BBSTELNET
#include "config.h"
#include "one_key.h"
extern char save_title[STRLEN];
extern char currboard[STRLEN];
extern int currfiletime;
extern char currmaildir[STRLEN];
extern char fromhost[];
extern char fromhost[60];
extern char lookgrp[];
extern char page_requestor[];
extern char ReadPost[];
extern char ReplyPost[];
extern char tempfile[];
extern int automargins;
extern int bug_possible;
extern int can_R_endline;
extern int g_convcode;
extern int delcnt;
extern int delmsgs[];
extern int digestmode;
extern int digestmode2;
extern int editansi;
extern int effectiveline;
extern int endlineoffset;
extern int ERROR_READ_SYSTEM_FILE;
extern int friendflag;
extern int inprison;
extern int isattached;
extern int iscolor;
extern int local_article;
extern int msg_num;
extern int nettyNN;
extern int numboards;
extern int numf, friendmode;
extern int numofsig;
extern int page, range;
extern int pty;
extern int range;
extern int refscreen;
extern int started;
extern int talkidletime;
extern int t_columns;
extern int t_lines;
extern int toggle1, toggle2;
extern int WishNum;
extern int mot;
extern struct BCACHE *brdshm;
extern struct one_key friend_list[];
extern struct one_key mail_comms[];
extern const struct one_key mail_default_comms[];
extern struct one_key read_comms[];
extern struct one_key reject_list[];
extern struct postheader header;
extern struct user_info **user_record;
extern struct UTMPFILE *utmpshm;
extern time_t login_start_time;
extern int cur_ln;
extern int scr_cols;
extern int disable_move;
extern char ISdelrq;
extern int readingthread;
#define ZMODEM_RATE 5000
#endif
