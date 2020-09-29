#ifndef BMYBBS_BOARDS_H
#define BMYBBS_BOARDS_H
#include "ythtbbs/boardrc.h"

extern struct onebrc brc;

void GoodBrds(void);
void EGroup(char *cmd);
void Boards(void);
void New(void);
int unread_position(char *dirfile, struct boardmem *bptr);
void update_postboards(void);
int brc_initial(char *boardname, int keep);
void clear_new_flag_quick(int t);
void clear_all_new_flag(void);
int Read(void);
int showhell(void);
int showprison(void);
int clubtest(char *board);
#endif //BMYBBS_BOARDS_H
