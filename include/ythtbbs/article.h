/* article.c */
#ifndef __ARTICLE_H
#define __ARTICLE_H

struct fileheader {
	time_t filetime;
	time_t edittime;
	time_t thread;
	unsigned int accessed;
	char title[60];
	char owner[14];				//如果是本站的, 就用id, 如果时email, 就取第一个词并
								//且加'.'. 如果是本站匿名, 则第一个为'\0', 后面跟id
	unsigned short viewtime;
	unsigned char sizebyte;
	unsigned char staravg50;	//staravg 最大为5, staravg50 = staravg * 50
								//i.e. staravg50 = totalstar * 50 / hasvoted
	unsigned char hasvoted;
	char deltime;				//记录回收站和纸篓里面的文章时什么时间删除的
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
	time_t filetime;
	char boardname[20];
	char title[60];
	char unused[44];
};

struct boardtop {
	char title[60];
	int unum;
	time_t thread;
	char firstowner[14];
	time_t lasttime;
	char board[24];
};

/**
 * @brief 依据日期、标记位转换为文件名
 * @warning 多线程不安全
 */
char *fh2fname(struct fileheader *fh);

/**
 * @brief 生成过刊文件名
 * @warning 多线程不安全
 */
char *bknh2bknname(struct bknheader *bknh);

/**
 * @brief 获取文章作者
 * 匿名模式返回 "Anonymous"
 * @see fh2realauthor
 */
char *fh2owner(struct fileheader *fh);

/**
 * @brief 获取真实文章作者
 * 匿名模式特别处理
 * @see fh2owner
 */
char *fh2realauthor(struct fileheader *fh);

/**
 * @brief 获取文章的修改时间
 */
time_t fh2modifytime(struct fileheader *fh);

/**
 * @brief 设置文章作者
 */
void fh_setowner(struct fileheader *fh, char *owner, int anony);
int change_dir(char *, struct fileheader *, void *func(void *, void *), int, int, int);
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
 * @warn 该过程中将字符串直接转为 time_t，可能有些平台上不支持。同时该方法使用过程中未校验 filename 格式是否正确。谨慎使用。
 * @param filename 形如 M.1376120232.A
 * @return 时间戳
 */
time_t fn2timestamp(char * filename);

/** 处理文章中 @id 这种关键字
 *
 * @param content 发帖的正文
 * @param userids 字符串数组，需要预先声明为 char [MAX_MENTION_ID][14]
 * @param from 用于调用时候声明来源，1 表示 nju09 以及其他不附带 QMD 的场景下，跳过内部 QMD 校验。
 * @return 处理成功返回0
 */
int parse_mentions(char *content, char **userids, int from);
#endif
