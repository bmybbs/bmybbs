/* article.c */
#ifndef __ARTICLE_H
#define __ARTICLE_H

struct fileheader {
	int filetime;
	int edittime;
	int thread;
	unsigned int accessed;
	char title[60];
	char owner[14];		//如果是本站的, 就用id, 如果时email, 就取第一个词并
	//且加'.'. 如果是本站匿名, 则第一个为'\0', 后面跟id
	unsigned short viewtime;
	unsigned char sizebyte;
	unsigned char staravg50;	//staravg 最大为5, staravg50 = staravg * 50
	//i.e. staravg50 = totalstar * 50 / hasvoted
	unsigned char hasvoted;
	char deltime;		//记录回收站和纸篓里面的文章时什么时间删除的
	//now_t / (3600 * 24) % 100
	//用于自动清除垃圾
	char unused[32];
};

//fileheader - accessed values
#define FH_READ 		0x00000001	//whether the file has been viewed if it is a mail
#define FH_HIDE 		0x00000002	//whether the file has been set as hidden in backnumber
#define FH_MARKED 		0x00000004
#define FH_DIGEST 		0x00000008	//Has been put into digest
#define FH_NOREPLY 		0x00000010
#define FH_ATTACHED		0x00000020	//Has attachments
#define FH_DEL			0x00000040	//Marked to be deleted
#define FH_SPEC 		0x00000080	//Will be put to 0Announce, and this flag will be clear then
#define FH_INND 		0x00000100	//write into innd/out.bntp
#define FH_ANNOUNCE		0x00000200	//have been put into 0Announce
#define FH_1984 		0x00000400	//have been checked to see if there is any ...
#define FH_ISDIGEST		0x00000800	//whether it is a digest file, i.e., filename is start with G., but not M.
#define FH_REPLIED		0x00001000	//this mail has been replied
#define FH_ALLREPLY		0x00002000	//this article can be re by all...
#define FH_MATH 		0x00004000	//this article contains itex math functions.
#define FH_DANGEROUS	0x00008000
#define FILE_TOP1 		0x00010000	//hace
#define FILE_ISTOP1		0x00020000	//slowaction
#define FH_MINUSDEL 	0x00040000	//add by mintbaggio for minus-postnums delte
#define FH_MAILREPLY 	0x00080000	//add by macintosh for reply mail to author
#define FH_ISWATER		0x00100000	//判断是否被标注为水文

struct bknheader {
	int filetime;
	char boardname[20];
	char title[60];
	char unused[44];
};

struct boardtop {
        char title[60];
        int unum;
        int thread;
        char firstowner[14];
        int lasttime;
	char board[24];
};

char *fh2fname(struct fileheader *fh);
char *bknh2bknname(struct bknheader *bknh);
char *fh2owner(struct fileheader *fh);
char *fh2realauthor(struct fileheader *fh);
void fh_setowner(struct fileheader *fh, char *owner, int anony);
int fh2modifytime(struct fileheader *fh);
int change_dir(char *, struct fileheader *,
	       void *func(void *, void *), int, int, int);
void DIR_do_mark(struct fileheader *, struct fileheader *);
void DIR_do_digest(struct fileheader *, struct fileheader *);
void DIR_do_underline(struct fileheader *, struct fileheader *);
void DIR_do_allcanre(struct fileheader *, struct fileheader *);
void DIR_do_attach(struct fileheader *, struct fileheader *);
void DIR_clear_dangerous(struct fileheader *, struct fileheader *);
void DIR_do_dangerous(struct fileheader *, struct fileheader *);
void DIR_do_markdel(struct fileheader *, struct fileheader *);
void DIR_do_mark_minus_del(struct fileheader *, struct fileheader *);	//add by mintbaggio@BMY 040321 for postnums delete
void DIR_do_edit(struct fileheader *, struct fileheader *);
void DIR_do_changetitle(struct fileheader *, struct fileheader *);
void DIR_do_evaluate(struct fileheader *, struct fileheader *);
void DIR_do_spec(struct fileheader *, struct fileheader *);
void DIR_do_import(struct fileheader *, struct fileheader *);
void DIR_do_suremarkdel(struct fileheader *, struct fileheader *);
void DIR_do_top(struct fileheader *, struct fileheader *);

/**
 * @brief 给文章增加或者解除水文标记。
 * @param
 * @param
 */
void DIR_do_water(struct fileheader *, struct fileheader *);
int outgo_post(struct fileheader *, char *, char *, char *);
void cancelpost(char *, char *, struct fileheader *, int);
int cmp_title(char *title, struct fileheader *fh1);
int fh_find_thread(struct fileheader *fh, char *board);
int Search_Bin(char *ptr, int key, int start, int end);	
int add_edit_mark(char *fname, char *userid, time_t now_t, char *fromhost);
int is_article_area_top(char *boardname, int thread);
int update_article_area_top_link(char *boardname, int oldthread, int newfiletime, char *newtitle);
int is_article_site_top(char *boardname, int thread);
int update_article_site_top_link(char *boardname, int oldthread, int newfiletime, char *newtitle);

/** 将文件名转为时间戳
 * @warn 该过程中将 int 直接转为 time_t，可能有些平台上不支持。同时该方法使用过程中未校验 filename 格式是否正确。谨慎使用。
 * @param filename 形如 M.1376120232.A
 * @return 时间戳
 */
time_t fn2timestamp(char * filename);
#endif
