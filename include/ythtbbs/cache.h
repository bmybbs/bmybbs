#ifndef BMYBBS_CACHE_H
#define BMYBBS_CACHE_H
#include <stdbool.h>
#include <stdarg.h>
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

// 最长发呆时间，3天
#define MAX_IDEL_TIME (3 * 24 * 3600)
// 最长会话时间，7天强制登出
#define MAX_SESS_TIME (7 * 24 * 3600)

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
	int  login_type;             ///< 登录类型，参考 ythtbbs_user_login_type
	char userid[20];
	char realname[20];
	char username[NAMELEN];
	unsigned int unreadmsg;
	short curboard;
	int clubrights[4];           ///< 俱乐部权限 add by ylsdd
	unsigned friend[MAXFRIENDS]; ///< 用于存放好友的 uid 列表
	unsigned reject[MAXREJECTS]; ///< 用于存放黑名单的 uid 列表
	struct wwwsession wwwinfo;
	struct onebrc brc;           ///< 用于存放某个版面未读信息的数据，在 bbs / www 中使用
	struct allbrc allbrc;        ///< 用于加载全部未读信息（压缩后的），在 api 中避免反复申请内存
	char user_state_temp[16];  //add by leoncom
};

#define USHM_SIZE       (MAXACTIVE + 10)
#define UCACHE_HASH_SIZE (MAXUSERS*2)
#define MAX_LOGIN_PER_USER 10
// 分段存放会话
#define LOGIN_END_SLOT_NUMBER_TELNET 2
#define LOGIN_END_SLOT_NUMBER_SSH    4
#define LOGIN_END_SLOT_NUMBER_WEB    8
#define LOGIN_END_SLOT_NUMBER_OAUTH  MAX_LOGIN_PER_USER
/**
 * 原 UCACHE / UINDEX 两个 item 的合并
 */
struct ythtbbs_cache_User {
	char userid[IDLEN+1];
	int  utmp_indices[MAX_LOGIN_PER_USER]; /* position in the UTFPFILE */
};

struct ythtbbs_cache_UserIDHashItem {
	int  user_num;          /* index in the .PASSWDS, STARTING from 1 */
	char userid[IDLEN + 1];
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

// 如果返回 QUIT 则终止
typedef int (*ythtbbs_cache_UserTable_foreach_callback)(const struct ythtbbs_cache_User *user, int curr_idx, va_list ap);
void ythtbbs_cache_UserTable_foreach_v(ythtbbs_cache_UserTable_foreach_callback callback, ...);

int ythtbbs_cache_UserTable_get_user_online_friends(const char *userid, bool has_see_cloak_perm, struct user_info *user_list, size_t user_list_size);

/**
 * @brief 依据 uid 查找用户是否在线
 * 参考了 src/bbs/bcache.c::query_uindex 的实现。
 * (nju09/bbsfoot.c query_f 为此版本的简化)
 * @param userid
 * @param has_see_cloak_perm 是否具有查看隐身的权限
 * @param search_uid 目标用户uid（从1索引）
 */
const struct user_info *ythtbbs_cache_UserTable_query_user_by_uid(const char *userid, bool has_see_cloak_perm, unsigned search_uid, bool dotest);

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

/**
 * @brief 依据缓存寻找首个空的用户位
 *
 * 本方法在 term / www 下均被使用，但非多线程安全，需要进一步重构。
 * @warning 多线程不安全
 * @return 用户id（从 1 索引），当已达到最大用户数时，返回0
 * TODO
 */
int ythtbbs_cache_UserTable_searchnewuser();

/**
 * @brief 给用户表设置 userid
 * 来自于 src/bbs/bcache.c::setuserid，不安全的实现，需要进一步重构
 * @warning 多线程不安全
 */
void ythtbbs_cache_UserTable_setuserid(int usernum, char *userid);

/**
 * @brief 从用户缓存表中复制用户名到缓冲区
 * 来自 src/bbs/bcache.c::getuserid
 */
void ythtbbs_cache_UserTable_getuserid(int usernum, char *userid, size_t len);

/**
 * @brief 查找 usernum
 * 来自 src/bbs/bcache.c::searchuser
 * 从缓存用户表中先依据散列表查找，如果没有命中，再遍历用户表。
 */
int ythtbbs_cache_UserTable_search_usernum(const char *userid);

/**
 * @brief 获取用户数
 * 这个 API 目前仅供 src/bbs/main.c 调用
 */
int ythtbbs_cache_UserTable_get_usersum(void);

/**
 * @brief 设置用户数
 * 这个 API 目前仅供 src/bbs/main.c 调用
 */
void ythtbbs_cache_UserTable_set_usersum(int usersum);

int ythtbbs_cache_UserTable_get_number(void);

/**
 * @brief 获取用户名
 * 这个函数来自 src/bbs/bcache.c::u_namearray，在 src/bbs/namecomplete.c::usercomplete 函数中使用。实现中没有校验缓冲区长度。
 * 在原实现中，当 tag 为空时，直接返回缓存 ucache 中的 userid 数组，在新的实现中，不管 tag 是否为空，都将 ythtbbs_cache_UserTable 中的 userid 复制到缓冲区 buf 中。
 */
char *ythtbbs_cache_UserTable_get_namearray(char buf[][IDLEN + 1], int *pnum, char *tag, int (*fptr)(char *, char *, char *));

/**
 * @brief
 * 参考 src/bbs/bcache.c::count_uindex 的实现（其实属于内部实现的调用）。
 */
int ythtbbs_cache_UserTable_count(int uid);

/**
 * @brief
 * 参考 src/bbs/bcache.c::count_uindex_telnet 的实现（其实属于内部实现的调用）。
 */
int ythtbbs_cache_UserTable_count_telnet(int uid);

/**
 * @brief 依据 userid 查找用户索引
 * 用户索引指的是在 PASSFILE 中的位置，从 0 开始索引。在 BMYBBS 环境中，遇到 uid 的地方通常是从 1 开始索引，因此需要注意相互转换。
 * 当匹配到后返回索引值。出错返回 -1。
 */
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
 * @brief 移除原有会话并更新为新会话
 * 重构原 src/bbs/bcache.c::update_utmp2 实现。在 src/bbs/main.c::u_exit 中被调用，调用后全局变量 utmpent 被置为 -1。
 */
void ythtbbs_cache_utmp_update(int utmp_idx, const struct user_info *ptr_info);

/**
 * @brief 统计在线人数
 * 来自 src/bbs/talk.c::num_active_users 实现。在原实现中，wwwguest 统计值
 * 直接赋给 utmpshm->wwwguest (具体逻辑在 count_active 中)。随着缓存不再生
 * 成 guest 会话，暂时不做统计。
 */
int ythtbbs_cache_utmp_count_active(void);

/**
 * @brief 向 shm_utmp 中插入 user_info
 *
 * 对应于 src/bbs/bcache::getnewutmpent 的重构。
 * @warning 和 getnewutmpent 所不同的是返回的是索引，从0开始计数
 * @param ptr_user_info
 * @return -1 表示插入失败，否则返回索引值
 */
int ythtbbs_cache_utmp_insert(struct user_info *ptr_user_info);

/**
 * @brief 从 shm_utmp 中移除索引
 * 参考了 nju09 bbslogout 的实现
 * @param utmp_idx 索引值
 */
void ythtbbs_cache_utmp_remove(int utmp_idx);

/**
 * @brief 更新未读消息记录
 */
void ythtbbs_cache_utmp_increase_unreadmsg(const struct user_info *ptr_info);
typedef int (*ythtbbs_cache_utmp_apply_callback)(const struct user_info *, void *x_param);

/**
 * @brief 提供通用的遍历 utmp 会话表的接口
 * 来自 src/bbs/bcache.c::apply_ulist 实现。但是多了一个传入参数，可以给回调函数更大的灵活性。
 */
int ythtbbs_cache_utmp_apply(ythtbbs_cache_utmp_apply_callback fptr, void *x_param);

int ythtbbs_cache_utmp_check_active_by_idx(int idx);

int ythtbbs_cache_utmp_check_uid_by_idx(int idx, int uid);

unsigned short ythtbbs_cache_utmp_get_activeuser(void);

time_t ythtbbs_cache_utmp_get_watchman(void);
void ythtbbs_cache_utmp_set_watchman(time_t t);

unsigned int ythtbbs_cache_utmp_get_unlock(void);
void ythtbbs_cache_utmp_set_unlock(void);

unsigned short ythtbbs_cache_utmp_get_maxtoday(void);
void ythtbbs_cache_utmp_set_maxtoday(unsigned short m);

unsigned short ythtbbs_cache_utmp_get_maxuser(void);
void ythtbbs_cache_utmp_set_maxuser(unsigned short m);

unsigned short ythtbbs_cache_utmp_get_wwwguest(void);

void ythtbbs_cache_utmp_set_www_kicked(int utmp_idx);

/**
 * @brief 依据 utmp_idx 获取缓存中的结构体（只读）
 * @param idx 在 user_info 数组中的索引，从 0 开始计数
 * @return 结构体指针
 */
struct user_info *ythtbbs_cache_utmp_get_by_idx(int idx);

void ythtbbs_cache_Board_resolve();
struct boardmem *ythtbbs_cache_Board_get_board_by_idx(int idx);
struct boardmem *ythtbbs_cache_Board_get_board_by_name(const char *bname);
int ythtbbs_cache_Board_get_idx_by_ptr(const struct boardmem *ptr);
int ythtbbs_cache_Board_get_idx_by_name(const char *bname);
int ythtbbs_cache_Board_set_bm_hat_v(void *b, va_list ap);
int ythtbbs_cache_Board_updatelastpost(const char *board);
int ythtbbs_cache_Board_updatelastpost_x(struct boardmem *ptr);

time_t ythtbbs_cache_Board_get_pollvote(void);
void ythtbbs_cache_Board_set_pollvote(time_t t);

int ythtbbs_cache_Board_get_number(void);
time_t ythtbbs_cache_Board_get_uptime(void);

typedef int (*ythtbbs_cache_Board_foreach_callback)(struct boardmem *board, int curr_idx, va_list ap);
void ythtbbs_cache_Board_foreach_v(ythtbbs_cache_Board_foreach_callback callback, ...);
#endif //BMYBBS_CACHE_H
