//
// Created by ironblood on 2020/8/10.
//

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
#endif //BMYBBS_CACHE_H
