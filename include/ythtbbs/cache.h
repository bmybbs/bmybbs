#ifndef BMYBBS_CACHE_H
#define BMYBBS_CACHE_H
#include <stdbool.h>
#include <time.h>
#include "config.h"
#include "ythtbbs/board.h"
#include "ythtbbs/boardrc.h"

/* For All Kinds of Pagers */
#define ALL_PAGER       0x1
#define FRIEND_PAGER    0x2
#define ALLMSG_PAGER    0x4
#define FRIENDMSG_PAGER 0x8
/* END */

struct wwwsession {
	unsigned char used:1, show_reg:1, att_mode:1, doc_mode:1;
	unsigned char link_mode:1, def_mode:1, t_lines:6;
	char iskicked;
	char unused;
	time_t login_start_time;
	time_t lastposttime;
	time_t lastinboardtime;
};

/**
 * Structure used in UTMP file
 */
struct user_info {
	int active;                  ///< When allocated this field is true
	int uid;                     ///< Used to find user name in passwd file, starting from 1
	int pid;                     ///< kill() to notify user of talk request
	bool invisible;              ///< Used by cloaking function in Xyz menu
	int sockactive;              ///< Used to coordinate talk requests
	int sockaddr;                ///< ...
	int destuid;                 ///< talk uses this to identify who called
	int mode;                    ///< UL/DL, Talk Mode, Chat Mode, ...
	int pager;                   ///< pager toggle, YEA, or NA
	int in_chat;                 ///< for in_chat commands
	int fnum;                    ///< number of friends
	int rnum;                    ///< number of rejects
	short ext_idle;              ///< has extended idle time, YEA or NA
	bool isssh;                  ///< login from ssh
	time_t lasttime;             ///< time of the last action
	unsigned int userlevel;      ///< change by lepton for www
	char chatid[10];             ///< chat id, if in chat mode
	char from[BMY_IPV6_LEN];     ///< machine name the user called in from
	char sessionid[40];          ///< add by leptin for www use
	char token[TOKENLENGTH+1];   ///< 用于防范 CSRF 攻击
	char appkey[APPKEYLENGTH+1]; ///< 用于存放APP来源
	char userid[20];
	char realname[20];
	char username[NAMELEN];
	unsigned int unreadmsg;
	short curboard;
	int clubrights[4];           ///< 俱乐部权限 add by ylsdd
	unsigned friend[MAXFRIENDS]; ///< 用于存放好友的 uid 列表
	unsigned reject[MAXREJECTS]; ///< 用于存放黑名单的 uid 列表
	struct wwwsession wwwinfo;
	struct onebrc brc;
	char user_state_temp[16];  //add by leoncom
};

#define USHM_SIZE       (MAXACTIVE + 10)
struct UTMPFILE {
	struct user_info uinfo[USHM_SIZE];
	time_t uptime;
	unsigned short activeuser;
	unsigned short maxuser;	//add by gluon
	unsigned short maxtoday;
	unsigned short wwwguest;
	time_t activetime;	//time of updating activeuser
	int ave_score;
	int allprize;
	time_t watchman;
	unsigned int unlock;
	int nouse[5];
};

struct BCACHE {
	struct boardmem bcache[MAXBOARD];
	int number;
	time_t uptime;
	time_t pollvote;
};

struct UCACHE {
	char userid[MAXUSERS][IDLEN + 1];
	int number;
	int usersum;
	time_t uptime;
	int nouse[10];
};

#define UCACHE_HASH_SIZE (MAXUSERS*2)
struct useridhashitem {
	int num;
	char userid[IDLEN + 1];
};

struct UCACHEHASH {
	struct useridhashitem uhi[UCACHE_HASH_SIZE];
	time_t uptime;
};

struct UINDEX {
	int user[MAXUSERS][6];	//不清楚www判断多登录的机制是否使上限超出telnet中的5, 设成6
};

#define MAX_LOGIN_PER_USER 6
/**
 * 原 UCACHE / UINDEX 两个 item 的合并
 */
struct ythtbbs_cache_User {
	char userid[IDLEN+1];
	int  utmp_indices[MAX_LOGIN_PER_USER]; /* position in the UTFPFILE */
};

/**
 * 对应与原 UCACHE / UINDEX 两个表
 */
struct ythtbbs_cache_UserTable {
	struct ythtbbs_cache_User users[MAXUSERS];
	int number;
	int usersum;
	time_t update_time;
	int nouse[10];
};

struct ythtbbs_cache_UserIDHashItem {
	int  user_num;          /* index in the .PASSWDS, STARTING from 1 */
	char userid[IDLEN + 1];
};

/**
 * Refactoring the UCACHEHASH structure
 */
struct ythtbbs_cache_UserIDHashTable {
	struct ythtbbs_cache_UserIDHashItem items[UCACHE_HASH_SIZE];
	time_t update_time;
};

/**
 * @brief 对应于原 useridhash 函数
 * 将用户名散列到 26*26 个 buckets 中
 */
unsigned int ythtbbs_cache_User_hash(const char *userid);

/**
 * @brief 解析 UserTable 以及 UserIDHashTable
 *
 * UserTable 按照 PASSWDS 文件内的结构顺序将用户 ID 插入到缓存表
 * 中，同时关联着运行时该用户的会话位置。
 *
 * 更新 UserTable 的过程使用独占文件锁保护（适用于多进程/多线程），
 * 更新完毕后释放并调用内部的解析、更新 UserIDHashTable 的过程，
 * 该过程对外不可见，内部使用另一个独占文件锁保护。
 */
void ythtbbs_cache_UserTable_resolve();

void ythtbbs_cache_UserTable_add_utmp_idx(int uid, int utmp_idx);

void ythtbbs_cache_UserTable_remove_utmp_idx(int uid, int utmp_idx);

int ythtbbs_cache_UserTable_get_user_online_friends(const char *userid, bool has_see_cloak_perm, struct user_info *user_list, size_t user_list_size);

/**
 * @brief 判断用户是否在线
 * 遍历该 userid 对应的会话，如果用户存在且会话存在，则：
 *   1. 对于 telnet、ssh 会话认为在线
 *   2. 对于 www、api 会话认为上次活动在 5 分钟之内为在线
 * 目前判定时间使用的是魔数，后期重构为宏或者常量。
 */
bool ythtbbs_cache_UserTable_is_user_online(const char *userid);

/**
 * @brief 判断用户是否隐身
 * 遍历该 userid 对应的会话，如果用户存在且会话存在，则：
 * 只要有一个为隐身，显示为隐身 (返回 true)。
 */
bool ythtbbs_cache_UserTable_is_user_invisible(const char *userid);

int ythtbbs_cache_UserIDHashTable_find_idx(const char *userid);

/**
 * @brief 获取 ave_score
 * 目前给 moneycenter 使用
 */
int ythtbbs_cache_utmp_get_ave_score();

/**
 * @brief 设置 ave_score
 * 目前给 moneycenter 使用
 */
void ythtbbs_cache_utmp_set_ave_score(int value);
/**
 * @brief 解析 shm_utmp
 */
void ythtbbs_cache_utmp_resolve(void);

/**
 * @brief 向 shm_utmp 中插入 user_info
 *
 * 对应于 src/bbs/bcache::getnewutmpent 的重构。
 * @warning 和 getnewutmpent 所不同的是返回的是索引，从0开始计数
 * @param ptr_user_info
 * @return -1 表示插入失败，否则返回索引值
 */
int ythtbbs_cache_utmp_insert(struct user_info *ptr_user_info);

int ythtbbs_cache_utmp_check_active_by_idx(int idx);

int ythtbbs_cache_utmp_check_uid_by_idx(int idx, int uid);

/**
 * @brief 依据 utmp_idx 获取缓存中的结构体（只读）
 * @param idx 在 user_info 数组中的索引，从 0 开始计数
 * @return 结构体指针
 */
struct user_info *ythtbbs_cache_utmp_get_by_idx(int idx);

void ythtbbs_cache_Board_resolve();
struct boardmem *ythtbbs_cache_Board_get_bcache();
int ythtbbs_cache_Board_set_bm_hat(struct boardmanager *bm, bool *invisible, bool *online);
#endif //BMYBBS_CACHE_H
