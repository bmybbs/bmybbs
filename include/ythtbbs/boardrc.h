/* boardrc.c */
#ifndef __BOADRC_H
#define __BOADRC_H
#include "ythtbbs/article.h"
#define BRC_MAXSIZE     25000
#define BRC_MAXNUM      60
#define BRC_STRLEN      15
struct onebrc {
	short num;
	short cur;
	short changed;
	char board[BRC_STRLEN];
	int list[BRC_MAXNUM];
	int notetime;
};

struct onebrc_c {
	short len;
	char data[280];
} __attribute__ ((__packed__));

struct allbrc {
	short size;
	short changed;
	char brc_c[BRC_MAXSIZE];
};

/**
 * @brief 初始化 brc 记录
 * 首先以只读方式打开位于 bbstmpfs 中的临时文件。
 * 如果打开失败，再只读打开 brc_file。
 * 文件打开后读取最多 BRC_MAXSIZE 的内容存放在 allbrc->brc_c 中。
 * @param allbrc 用于存放记录的缓冲区
 * @param userid
 * @param brc_file 位于用户数据目录下的 brc 文件
 */
void brc_init(struct allbrc *allbrc, const char *userid, const char *brc_file);
void brc_fini(struct allbrc *allbrc, char *userid);
void brc_getboard(struct allbrc *allbrc, struct onebrc *brc, char *board);
void brc_putboard(struct allbrc *allbrc, struct onebrc *brc);
void brc_addlistt(struct onebrc *brc, int t);
int brc_unreadt(struct onebrc *brc, int t);
int brc_unreadt_quick(struct allbrc *allbrc, char *board, int t);
void brc_clearto(struct onebrc *brc, int t);
int UNREAD(const struct fileheader *fh, struct onebrc *brc);
void SETREAD(const struct fileheader *fh, struct onebrc *brc);
#endif
