#ifndef BMYBBS_CACHE_H
#define BMYBBS_CACHE_H
#include <time.h>
#include "config.h"
#include "struct.h"
#include "board.h"

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
	struct moneyCenter mc;
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
	char userid[IDLEN + 1]; // TODO replace with ptr?
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
unsigned int ythtbbs_cache_User_hash(char *userid);
#endif //BMYBBS_CACHE_H
