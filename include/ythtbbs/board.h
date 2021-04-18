/* board.c */
#ifndef __BOARD_H
#define __BOARD_H
#include <stddef.h>
#include <stdbool.h>
#include "config.h"

#define BMNUM 16

/**
 * This structure is used to hold data in the BOARDS files
 */
struct boardheader {
	char filename[24];
	char title[24];
	int clubnum;
	unsigned level;
	char flag;
	char secnumber1;  ///< 这个就是此版面所在的区号，例如：'2','G'等等
	char secnumber2;
	char type[5];
	char bm[BMNUM][IDLEN + 1];
	int hiretime[BMNUM];
	int board_ctime;
	int board_mtime;
	char sec1[4];
	char sec2[4];
	char keyword[64];
	char unused[96];
};

/**
 * used for caching files and boards
 */
struct boardmem {
	struct boardheader header;
	int lastpost;
	int total;
	short inboard;
	short bmonline;
	short bmcloak;
	int stocknum;
	int score;
	int unused[10];
};

/**
 * record in user directionary
 */
struct boardmanager {
	char board[24];
	char bmpos;
	char unused;
	short bid;
};

char *bm2str(char *buf, struct boardheader *bh);
char *sbm2str(char *buf, struct boardheader *bh);

/**
 * @brief 依据版面名称获取 boardmem 对象
 * 从 nju09/BBSLIB.c 复制而来，理应属于 libythtbbs 库的一部分。该方法将从 shm_bcache
 * 中递归的查找比对 board_name，若相同则返回 boardmem 地址。
 * 从 nju09 移植，by IronBlood 20130805
 * 使用 ythtbbs_cache_Board_get_board_by_name，变更于 20201114
 * @warning 该方法中不包含用户权限的校验。调用结束后不需要释放 boardmem 地址。
 * @param board_name 版面的英文名称
 * @see struct boardmem * getbcache(char *board)
 * @see struct boardmem * getboard(char *board)
 */
struct boardmem *getboardbyname(const char *board_name) __attribute__((deprecated("use ythtbbs_cache_Board_get_board_by_name instead")));

/**
 * @brief 依据版面名称检查是否是存在于 etc/junkboards 文件中
 * @warning 该方法移植自 nju09，和 term 下略有不同，需要进一步检查。
 * @param board_name 版面名称
 * @return
 */
int board_is_junkboard(char *board_name);

enum YTHTBBS_BOARD_INFO_STATUS {
	YTHTBBS_BOARD_INFO_NOT_FOUND = 0,
	YTHTBBS_BOARD_INFO_FOUND     = 1,
};

char *ythtbbs_board_set_board_file(char *buf, size_t len, const char *boardname, const char *filename);

enum YTHTBBS_BOARD_INFO_STATUS ythtbbs_board_load_intro(char *buf, size_t len, const struct boardheader *bh);
enum YTHTBBS_BOARD_INFO_STATUS ythtbbs_board_load_note(char *buf, size_t len, const struct boardheader *bh);
#endif

bool ythtbbs_board_is_political(const char *bname);
bool ythtbbs_board_is_political_x(const struct boardmem *x);
bool ythtbbs_board_is_hidden(const char *bname);
bool ythtbbs_board_is_hidden_x(const struct boardmem *x);

