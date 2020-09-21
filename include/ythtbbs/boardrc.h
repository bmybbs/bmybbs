/* boardrc.c */
#ifndef __BOADRC_H
#define __BOADRC_H
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

void brc_init(struct allbrc *allbrc, char *userid, char *filename);
void brc_fini(struct allbrc *allbrc, char *userid);
void brc_getboard(struct allbrc *allbrc, struct onebrc *brc, char *board);
void brc_putboard(struct allbrc *allbrc, struct onebrc *brc);
void brc_addlistt(struct onebrc *brc, int t);
int brc_unreadt(struct onebrc *brc, int t);
int brc_unreadt_quick(struct allbrc *allbrc, char *board, int t);
void brc_clearto(struct onebrc *brc, int t);
void brc_init_old(struct allbrc *allbrc, char *filename);
#endif
