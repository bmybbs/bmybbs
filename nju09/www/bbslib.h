#ifndef NJULIB_H
#define NJULIB_H
#include <sys/mman.h>
#include "bbs.h"
#include "ythtbbs/ythtbbs.h"
#include "ythtbbs/override.h"
#include "ythtbbs/mybrd.h"

#define FIRST_PAGE   "/"
#define CHARSET      "gb2312"

#define HTMPATH      "/home/apache/htdocs/bbs/"
#define CSSPATH      "/images/"

struct wwwstyle {
	char *name;
	char *cssfile;
	//char *leftcssfile;	//omit by macintosh 20060112
	//char *lbg;
	char *colortb1;
	char *colorstar;
};

#define NWWWSTYLE (9)
extern struct wwwstyle *currstyle, wwwstyle[];
extern int wwwstylenum;

#define SECNUM 13
#define BBSNAME MY_BBS_NAME
#define BBSHOME MY_BBS_HOME
#define BBSHOST MY_BBS_DOMAIN
#define PATHLEN         1024

extern const char seccodes[SECNUM];
extern const char secname[SECNUM][2][20];
extern char needcgi[STRLEN];

extern int loginok;
extern int isguest;
extern int tempuser;
extern int utmpent;
extern volatile int incgiloop;
extern int thispid;
extern time_t now_t;
extern jmp_buf cgi_start;
extern struct userec currentuser;
extern struct user_info *u_info;
extern struct wwwsession *w_info;
extern char fromhost[BMY_IPV6_LEN];
extern struct in6_addr from_addr; //ipv6 by leoncom
extern int quote_quote;
extern char *ummap_ptr;
extern int ummap_size;
/*add by macintosh 050619 for Tex Math Equ*/
extern int usedMath;
extern int usingMath;
extern int withinMath;

#define redirect(x)     printf("<meta http-equiv='Refresh' content='0; url=%s'>\n", x)
#define refreshto(x, t) printf("<meta http-equiv='Refresh' content='%d; url=%s'>\n", t, x)
#define cgi_head()      printf("Content-type: text/html; charset=%s\n\n", CHARSET)

extern char parm_name[256][80], *parm_val[256];
extern int parm_num;

int count_mails(char *id, int *total, int *unread);

extern struct ythtbbs_override fff[200];
extern size_t friendnum;

extern struct ythtbbs_override bbb[MAXREJECTS];
extern int badnum;

char *sec(char);
int whatch(unsigned char ch);
int goodq(unsigned char ch);

struct deny {
	char id[80];
	char exp[80];
	int free_time;
};

struct cgi_applet {
	int (*main) (void);
	char *(name[5]);
	double utime;
	double stime;
	int count;
};

extern struct deny denyuser[256];
extern int denynum;
extern int nologin;

extern char mybrd[GOOD_BRD_NUM][80];
extern int mybrdnum;
extern struct goodboard g_GoodBrd;

void newreport(char *board, char *s);

struct emotion {
	char *(smilename[4]);
	char *filename;
};
#define feditmark(x)  ((x).edittime?((x).filetime-(x).edittime):0)
// copy from proto.h
int junkboard(char *board);
int f_write(char *file, char *buf);
int f_append(char *file, char *buf);
int put_record(void *buf, int size, int num, char *file);
int del_record(char *file, int size, int num);
char *noansi(char *s); // return a static buf
char *nohtml(const char *s);
char *getsenv(char *s);
int http_quit(void);
void http_fatal(char *fmt, ...);
void strnncpy(char *s, int *l, char *s2);
void strnncpy2(char *s, int *l, char *s2, int len);
char *titlestr(char *str);
int hprintf(char *fmt, ...);
int fhhprintf(FILE * output, char *fmt, ...);
void parm_add(char *name, char *val);
int cache_header(time_t t, int age);
void html_header(int mode);
void json_header(void);
void xml_header(void);
char *getparm(char *var);
char *getparm2(char *v1, char *v2);
int shm_init(void);
int ummap(void);
const char *getextrparam_str(unsigned int param);
int addextraparam(char *ub, int size, int n, int param);
int mail_file(char *filename, char *userid, char *title, char *sender);
int post_mail_to_sent_box(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig, int mark);
int post_mail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig, int mark);
int post_mail_buf(char *userid, char *title, char *buf, char *id, char *nickname, char *ip, int sig, int mark);
int post_article_1984(char *board, char *title, char *file, char *id, char *nickname, char *ip, int sig, int mark, int outgoing, char *realauthor, int thread);
int post_article(char *board, char *title, char *file, char *id, char *nickname, char *ip, int sig, int mark, int outgoing, char *realauthor, int thread);
int securityreport(char *title, char *content);
char *anno_path_of(char *board);
int has_BM_perm(struct userec *user, struct boardmem *x);
int has_read_perm(struct userec *user, char *board);
int has_read_perm_x(struct userec *user, struct boardmem *x);
int has_post_perm(struct userec *user, struct boardmem *x);
int has_vote_perm(struct userec *user, struct boardmem *x);
int hideboard(char *bname);
int hideboard_x(struct boardmem *x);
int innd_board(char *bname);
int political_board(char *bname);
int anony_board(char *bname);
int noadm4political(char *bname);
struct boardmem *getboard(char *board);	//返回shm中这个board的指针, 同时把shm中的版名拷贝到board, 如果没有权限则返回NULL
int send_msg(char *myuserid, int i, char *touserid, int topid, char *msg, int offline);
int count_life_value(struct userec *urec);
int save_user_data(struct userec *x);
int user_perm(struct userec *x, int level);
int getusernum(char *id);
struct userec *getuser(char *id);
int count_online(void);
int changemode(int mode);
char *encode_url(char *s);
char *noquote_html(char *s);
char *void1(char *s);
char *flag_str_bm(int access);
char *flag_str(int access);
char *flag_str2(int access, int has_read);
char *userid_str(char *s);
char *getbfroma(char *path);
int set_my_cookie(void);
int has_fill_form(void);
int cmpboard(struct boardmem **b1, struct boardmem **b2);
int cmpboardscore(struct boardmem **b1, struct boardmem **b2);
int cmpboardinboard(struct boardmem **b1, struct boardmem **b2);
int cmpuser(struct user_info *a, struct user_info *b);
struct fileheader *findbarticle(struct mmapfile *mf, char *file, int *num, int mode);
char *utf8_decode(char *src);
void fdisplay_attach(FILE *output, FILE *fp, char *currline, char *nowfile);
void printhr(void);
void updateinboard(struct boardmem *x);
int updatelastpost(char *board);
int readuserallbrc(char *userid, int must);
void brc_update(char *userid);
int brc_initial(char *userid, char *boardname);
void brc_add_read(struct fileheader *fh);
void brc_add_readt(int t);
int brc_un_read(struct fileheader *fh);
void brc_clear(void);
int brc_un_read_time(int ftime);
void loaddenyuser(char *board);
void savedenyuser(char *board);
int max_mail_size(void);
int get_mail_size(void);
int check_maxmail(char *currmaildir);
int countln(char *fname);
double *system_load(void);
int dofilter(char *title, char *fn, int level);
int dofilter_edit(char *title, char *buf, int level);
int search_filter(char *pat1, char *pat2, char *pat3);
char *setbfile(char *buf, char *boardname, char *filename);
void sstrcat(char *s, const char *format, ...);
char *showByDefMode(void);
char *url_encode(char *str);
char to_hex(char code);
//not high light Add by liuche 20121112
void NHsprintf(char *s, char *s0);

extern void check_msg(void); // bbsgetmsg.c

extern int readmybrd(char *userid);        /* bbsmybrd.c */
extern int mails(char *id, int *unread);   /* bbsfoot.c  */
extern int mails_time(char *id);           /* bbsfoot.c  */
extern void printselsignature();           /* bbspst.c */
extern void printuploadattach();           /* bbspst.c */
extern int showcon(char *filename);        /* bbscon.c */
extern int showfile(char *fn);             /* bbsboa.c */

enum FILTER_BOARD_e {
	FILTER_BOARD_basic       = 0x00,
	FILTER_BOARD_check_mybrd = 0x01,
	FILTER_BOARD_with_secstr = 0x02,
	FILTER_BOARD_with_secnum = 0x04,
};

/**
 * @brief 用于缓存中 Board foreach 获取版面的回调函数
 * 本函数属于位于多个代码片段中相似功能的合并。各个版本略有差别，通过 FILTER_BOARD_e 枚举控制代码逻辑。枚举采用位运算。
 */
int filter_board_v(struct boardmem *board, int curr_idx, va_list ap);
#endif
