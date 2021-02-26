#ifndef BMYBBS_BOARDS_H
#define BMYBBS_BOARDS_H
#include "ythtbbs/board.h"

extern struct onebrc brc;

int GoodBrds(const char *cmd);
int EGroup(const char *cmd);
int Boards(const char *s);
int New(const char *s);
int unread_position(char *dirfile, struct boardmem *bptr);
int brc_initial(char *boardname, int keep);
void clear_new_flag_quick(int t);
int clear_all_new_flag(const char *s);
int Read(const char *s);
int showhell(const char *s);
int showprison(const char *s);
int clubtest(char *board);
#endif //BMYBBS_BOARDS_H
