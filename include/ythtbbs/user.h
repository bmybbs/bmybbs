/* user.c */
#ifndef __USER_H
#define __USER_H
#include <time.h>
#include "config.h"
#include "board.h"
#include "cache.h"

/* these are flags in userec.flags[0] */
#define PAGER_FLAG     0x1   /* true if pager was OFF last session */
#define CLOAK_FLAG     0x2   /* true if cloak was ON last session */
#define CLOSECLUB_FLAG 0x4   /* 表示版面是否可见, 1 for close club, 0 for open club, add by macintosh 050530 for semi-closed club*/
#define SIG_FLAG       0x8   /* true if sig was turned OFF last session */
#define BRDSORT_FLAG2 0x10   /* true if the boards sorted by score */
//#define BRDSORT_FLAG 0x20  /* true if the boards sorted alphabetical, available only if FLAG2 is false */
#define BRDSORT_MASK  0x30
#define CURSOR_FLAG   0x80   /* true if the cursor mode open */
#define ACTIVE_BOARD 0x200   /* true if user toggled active movie board on */
#define CLUB_FLAG (CLOSECLUB_FLAG | CLUBTYPE_FLAG) /*俱乐部*/

/** Structure used to hold information in PASSFILE
 *
 */
struct userec {
	char userid[IDLEN + 2];
	time_t firstlogin;
	char lasthost[BMY_IPV6_LEN];
	unsigned int numlogins;
	unsigned int numposts;
	char flags[2];
	char passwd[PASSLEN];   //!<加密后的密码
	char username[NAMELEN];
	unsigned short numdays;	//!<曾经登录的天数
	char unuse[30];
	time_t dietime;
	time_t lastlogout;
	char ip[BMY_IPV6_LEN];
	char realmail[STRLEN - 16];
	unsigned userlevel;
	time_t lastlogin;
	time_t stay;
	char realname[NAMELEN];
	char address[STRLEN];
	char email[STRLEN];
	int signature;
	unsigned int userdefine;
	time_t notedate_nouse;
	int noteline_nouse;
	int notemode_nouse;
};

char mytoupper(unsigned char ch);

char *sethomepath_s(char *buf, size_t buf_size, const char *userid);
char *sethomefile_s(char *buf, size_t buf_size, const char *userid, const char *filename);
char *setmailfile_s(char *buf, size_t buf_size, const char *userid, const char *filename);
/* for sent mail */
char *setsentmailfile_s(char *buf, size_t buf_size, const char *userid, const char *filename);
int saveuservalue(char *userid, char *key, char *value);
int readuservalue(char *userid, char *key, char *value, int size);
char *charexp(int);
char *cperf(int);
int countexp(struct userec *);
int countperf(struct userec *);
int countlife(struct userec *);
int userlock(const char *userid, int locktype);
int userunlock(const char *userid, int fd);
int checkbansite(const char *addr);

/** 检查用户权限
 * 改方法从 nju09 移植，参见 int user_perm(struct userec *x, int level)。
 * @param x
 * @param level
 * @return
 */
int check_user_perm(struct userec *x, int level);

/**
 * @brief 检查用户的阅读权限
 * 该方法从 nju09 移植，用于检查用户读取版面的权限。
 * @param user
 * @param board 版面名称
 * @return
 * @see int has_read_perm(struct userec *user, char *board)
 * @see int has_read_perm_x(struct userec *user, struct boardmem *x)
 */
int check_user_read_perm(const struct user_info *user, const char *board);

/**
 * @brief 检查 guest 阅读权限
 * 参考 check_user_read_perm_x 的实现，便于相关接口调用
 * @return 有阅读权限返回 true
 */
bool check_guest_read_perm_x(const struct boardmem *board);

/**
 * @brief 检查用户的阅读权限
 * 该方法从 nju09 移植，用于检查用户读取版面的权限。
 * @param user
 * @param board boardmem 指针
 * @return 有权限返回 1，无权限返回 0。
 * @see int has_read_perm(struct userec *user, char *board)
 * @see int has_read_perm_x(struct userec *user, struct boardmem *x)
 */
int check_user_read_perm_x(const struct user_info *user, const struct boardmem *board);

/**
 * @brief 检查用户的发帖权限
 * 该方法从 nju09 移植，会同时判断版面阅读权限。
 * @param user
 * @param board
 * @return 有权限返回 1，无权限返回 0。
 */
int check_user_post_perm_x(const struct user_info *user, const struct boardmem *board);
int userbansite(const char *userid, const char *fromhost);
void logattempt(const char *user, const char *from, const char *zone, time_t time);
int inoverride(char *who, char *owner, char *file);

/**
 * @brief 检查 id 是否包含数字
 * 该方法在 telnet/nju09 中均有使用，移动到库中，方法名有待改进。
 * @param userid 字符串
 * @return 包含则返回
 */
int id_with_num(const char *userid);

int chk_BM(struct userec *, struct boardheader *bh, int isbig);
int chk_BM_id(char *, struct boardheader *);
int bmfilesync(struct userec *);

enum ythtbbs_user_login_type {
	YTHTBBS_LOGIN_TELNET,
	YTHTBBS_LOGIN_SSH,
	YTHTBBS_LOGIN_NJU09,
	YTHTBBS_LOGIN_API,
	YTHTBBS_LOGIN_OAUTH,
};

enum ythtbbs_user_login_status {
	YTHTBBS_USER_LOGIN_OK       = 0,
	YTHTBBS_USER_NOT_EXIST      = 1,
	YTHTBBS_USER_WRONG_PASSWORD = 2,
	YTHTBBS_USER_SUSPENDED      = 3,
	YTHTBBS_USER_TOO_FREQUENT   = 4,
	YTHTBBS_USER_SITE_BAN       = 5,
	YTHTBBS_USER_USER_BAN       = 6,
	YTHTBBS_USER_IN_PRISON      = 7,
	YTHTBBS_USER_ONLINE_FULL    = 8,
	YTHTBBS_USER_SESSION_ERROR  = 9,
};

enum ythtbbs_register_status {
	YTHTBBS_REGISTER_OK,
	YTHTBBS_REGISTER_FILE_ERROR,
	YTHTBBS_REGISTER_FULL,
	YTHTBBS_REGISTER_CANNOT_SEEK,
};

int ythtbbs_user_login(const char *userid, const char *passwd, const char *fromhost, const enum ythtbbs_user_login_type login_type, struct user_info *out_info, struct userec *out_userec, int *out_utmp_idx);

int ythtbbs_user_logout(const char *userid, const int utmp_idx);

/**
 * @brief 清理 PASSWDS 文件，暂定这个函数形式
 * 这个方法在实现的过程中判断上一次的调用时间，如果1h内已被调用过，则中断调用。
 * 遍历 PASSFILE，生命力为负的用户依次清除版主信息（如果存在）、收件箱、
 * 用户目录，然后使用 0 填充原数据结构空间。
 *
 * TODO 并未更新缓存
 * @note 参考了 src/bbs/register.c::getnewuserid() 的实现。但是没有调用 is_bad_id() 以及 touchnew()。时间相关的函数替换了多线程安全的版本。
 */
void ythtbbs_user_clean(void);

enum ythtbbs_register_status ythtbbs_user_create(const struct userec *user, int *usernum, int *time_interval);

const char *ythtbbs_user_get_login_type_str(enum ythtbbs_user_login_type type);
#endif
