#ifndef BMYBBS_LIST_H
#define BMYBBS_LIST_H
#include "ythtbbs/cache.h"

int myfriend(unsigned uid);
int hisfriend(const struct user_info *uentp);
int isreject(const struct user_info *uentp);
int allusers(void);
void setlistrange(int i);
void Users(void);
int t_friends(const char *s);
int t_users(const char *s);
int choose(int update, int defaultn, void (*title_show)(void), int (*key_deal)(int, int, int), int (*list_show)(void), int (*read)(int, int));
#endif //BMYBBS_LIST_H
