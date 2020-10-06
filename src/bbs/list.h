#ifndef BMYBBS_LIST_H
#define BMYBBS_LIST_H
#include "ythtbbs/cache.h"

int myfriend(unsigned uid);
int hisfriend(struct user_info *uentp);
int isreject(struct user_info *uentp);
int allusers(void);
void setlistrange(int i);
void Users(void);
int t_friends(void);
int t_users(void);
int choose(int update, int defaultn, void (*title_show)(void), int (*key_deal)(int, int, int), int (*list_show)(void), int (*read)(int, int));
#endif //BMYBBS_LIST_H
