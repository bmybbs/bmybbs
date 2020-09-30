#ifndef NJULIB_H
#define NJULIB_H
#ifndef ENABLE_FASTCGI
#define FCGI_ToFILE(x) (x)
//#define ENABLE_FASTCGI 1
#define FCGI_FILE FILE
#endif
//#include "fcgi_stdio.h"
#include <sys/mman.h>
#include "bbs.h"
#include "ythtbbs/ythtbbs.h"

#define FIRST_PAGE	"/"
#define CSS_FILE 	"/bbs.css"
#define CHARSET		"gb2312"
#define NAVFILE		"nav.txt"
#define MAXWWWCLIENT MAXACTIVE
#define CACHE_ABLE	0x100

#define MAX_PROXY_NUM 4
#define DEFAULT_PROXY_PORT 8080
#define HTMPATH "/home/apache/htdocs/bbs/"
#define CSSPATH		"/images/"

struct WWWCACHE {
	time_t www_version;
	unsigned int www_visit;
	unsigned int home_visit;
	union {
		unsigned int accel_ip;
		struct in_addr accel_addr;
	};
	unsigned int accel_port;
	unsigned int validproxy[MAX_PROXY_NUM];
	int nouse[27 - MAX_PROXY_NUM];
};

struct wwwstyle {
	char *name;
	char *cssfile;
	//char *leftcssfile;	//omit by macintosh 20060112
	//char *lbg;
	char *colortb1;
	char *colortb2;
	char *colorstar;
};

#define NWWWSTYLE (9)
extern struct wwwstyle *currstyle, wwwstyle[];
extern int wwwstylenum;
//extern int no_cache_header;
//extern int has_smagic;
// extern int go_to_first_page;

#define SECNUM 13
#define BBSNAME MY_BBS_NAME
#define BBSHOME MY_BBS_HOME
#define BBSHOST MY_BBS_DOMAIN
#define LDEB if(!strcmp(currentuser.userid,"lepton"))
#define PATHLEN         1024

extern const char seccodes[SECNUM];
extern const char secname[SECNUM][2][20];
extern char needcgi[STRLEN];
extern char rframe[STRLEN];
extern time_t thisversion;

extern int loginok;
extern int isguest;
extern int tempuser;
extern int utmpent;
extern volatile int incgiloop;
extern int thispid;
extern time_t now_t;
extern time_t starttime;
extern jmp_buf cgi_start;
extern struct userec currentuser;
extern struct user_info *u_info;
extern struct wwwsession *w_info;
extern struct UTMPFILE *shm_utmp;
extern struct BCACHE *shm_bcache;
extern struct UCACHE *shm_ucache;
extern struct UCACHEHASH *uidhashshm;
extern struct WWWCACHE *wwwcache;
extern struct UINDEX *uindexshm;
extern char fromhost[BMY_IPV6_LEN];
extern struct in6_addr from_addr; //ipv6 by leoncom
extern int via_proxy;
extern int quote_quote;
extern char *ummap_ptr;
extern int ummap_size;
extern char ATT_ADDRESS[20];
extern char ATT_PORT[10];
/*add by macintosh 050619 for Tex Math Equ*/
extern int usedMath;
extern int usingMath;
extern int withinMath;

#define redirect(x)	printf("<meta http-equiv='Refresh' content='0; url=%s'>\n", x)
#define refreshto(x, t)	printf("<meta http-equiv='Refresh' content='%d; url=%s'>\n", t, x)
#define cgi_head()	printf("Content-type: text/html; charset=%s\n\n", CHARSET)

extern char parm_name[256][80], *parm_val[256];
extern int parm_num;

int count_mails(char *id, int *total, int *unread);

extern struct override fff[200];
extern size_t friendnum;

extern struct override bbb[MAXREJECTS];
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

void display_attach(FILE * fp, char *currline, char *nowfile);
extern char mybrd[GOOD_BRC_NUM][80];
extern int mybrdnum;

void newreport(char *board, char *s);

char *wwwlogin(struct userec *user, int ipmask);
struct emotion {
	char *(smilename[4]);
	char *filename;
};
#define feditmark(x)  ((x).edittime?((x).filetime-(x).edittime):0)


// copy from proto.h
int junkboard(char *board);
int file_has_word(char *file, char *word);
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
int isaword(char *dic[], char *buf);
void get_session_string(char *name);
void print_session_string(char *value);
int cache_header(time_t t, int age);
void html_header(int mode);
void json_header(void);
void xml_header(void);
char *getparm(char *var);
char *getparm2(char *v1, char *v2);
int shm_init(void);
int ummap(void);
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
struct boardmem *getbcache(char *board);	//返回shm中这个 board 的指针
struct boardmem *getboard(char *board);	//返回shm中这个board的指针, 同时把shm中的版名拷贝到board, 如果没有权限则返回NULL
int send_msg(char *myuserid, int i, char *touserid, int topid, char *msg, int offline);
int count_life_value(struct userec *urec);
int save_user_data(struct userec *x);
int user_perm(struct userec *x, int level);
int insertuseridhash(struct useridhashitem *ptr, int size, char *userid, int num);
int finduseridhash(struct useridhashitem *ptr, int size, char *userid);
int getusernum(char *id);
struct userec *getuser(char *id);
int count_online(void);
int loadfriend(char *id);
int initfriends(struct user_info *u);
int isfriend(char *id);
int loadbad(char *id);
int isbad(char *id);
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
int getlastpost(char *board, int *lastpost, int *total);
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
char *bbsred(char *command);
int max_mail_size(void);
int get_mail_size(void);
int check_maxmail(char *currmaildir);
int countln(char *fname);
double *system_load(void);
int setbmstatus(struct userec *u, int online);
void add_uindex(int uid, int utmpent);
void remove_uindex(int uid, int utmpent);
int count_uindex(int uid);
int cachelevel(int filetime, int attached);
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
#endif
