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

void getsalt(char *salt);	//生成密码加密用两字节salt
int junkboard(char *board);

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
struct boardmem *getbcache(char *board);	//返回shm中这个 board 的指针
struct boardmem *getboard(char *board);	//返回shm中这个board的指针, 同时把shm中的版名拷贝到board, 如果没有权限则返回NULL
struct userec *getuser(char *id);
char *ModeType(int mode);
char *anno_path_of(char *board);
int file_has_word(char *file, char *word);
int f_append(char *file, char *buf);
int put_record(void *buf, int size, int num, char *file);
int del_record(char *file, int size, int num);
char *noansi(char *s);		//return a static buf
char *strright(char *s, int len);
/*add by macintosh 050619 for Tex Math Equ*/
extern int usedMath;
extern int usingMath;
extern int withinMath; 

#define redirect(x)	printf("<meta http-equiv='Refresh' content='0; url=%s'>\n", x)
#define refreshto(x, t)	printf("<meta http-equiv='Refresh' content='%d; url=%s'>\n", t, x)
#define cgi_head()	printf("Content-type: text/html; charset=%s\n\n", CHARSET)

char *getsenv(char *s);
int http_quit();
char *titlestr(char *str);
int hprintf(char *fmt, ...);
int hhprintf(char *fmt, ...);

extern char parm_name[256][80], *parm_val[256];
extern int parm_num;

char *getparm(char *var);
int shm_init(void);
int post_mail_to_sent_box(char *userid, char *title, char *file, char *id,
        char *nickname, char *ip, int sig, int mark);
int post_mail(char *userid, char *title, char *file, char *id, char *nickname,
	      char *ip, int sig, int mark);
int post_imail(char *userid, char *title, char *file, char *id, char *nickname,
	       char *ip, int sig);
int post_article(char *board, char *title, char *file, char *id, char *nickname,
		 char *ip, int sig, int mark, int outgoing, char *realauthor,
		 int thread);
int has_BM_perm(struct userec *user, struct boardmem *x);
int hideboard(char *bname);
int has_post_perm(struct userec *user, struct boardmem *x);
int count_mails(char *id, int *total, int *unread);
int send_msg(char *myuserid, int i, char *touserid, int topid, char *msg,
	     int offline);
int count_life_value(struct userec *urec);
int countexp(struct userec *x);
int countperf(struct userec *x);
int save_user_data(struct userec *x);
int is_bansite(char *ip);
int user_perm(struct userec *x, int level);
int getusernum(char *id);
int checkuser(char *id, char *pw);
int count_id_num(char *id);
int count_online(void);
int count_online2(void);

extern struct override fff[200];
extern size_t friendnum;

int loadfriend(char *id);
int isfriend(char *id);

extern struct override bbb[MAXREJECTS];
extern int badnum;

int loadbad(char *);
int isbad(char *);
char *void1(char *);
char *sec(char);
char *flag_str(int);
char *flag_str2(int, int);
char *userid_str(char *s);
int fprintf2(FILE * fp, char *s);
char *getbfroma(char *path);
int set_my_cookie();
int has_fill_form();
void getrandomstr(unsigned char *s);
int whatch(unsigned char ch);
int goodq(unsigned char ch);
int goodgbid(char *userid);

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

int cmpuser(struct user_info *a, struct user_info *b);
void display_attach(FILE * fp, char *currline, char *nowfile);
extern char mybrd[GOOD_BRC_NUM][80];
extern int mybrdnum;

extern char *encode_url(char *);
extern char *noquote_html(char *);
extern char *utf8_decode(char *str);
int initfriends(struct user_info *);
void newreport(char *board, char *s);

void printhr();
char *bbsred(char *);
char *wwwlogin(struct userec *user, int ipmask);
void html_header(int);

struct emotion {
	char *(smilename[4]);
	char *filename;
};
#define feditmark(x)  ((x).edittime?((x).filetime-(x).edittime):0)

#ifndef MAKE_PROTO
#include "proto.h"
#endif
void json_header();
void sstrcat(char *s, const char *format, ...);

char* showByDefMode();
//not high light Add by liuche 20121112
void NHsprintf(char *s, char *s0);
