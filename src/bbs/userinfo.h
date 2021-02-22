#ifndef BMYBBS_USERINFO_H
#define BMYBBS_USERINFO_H
#include "ythtbbs/user.h"

void permtostr(int perm, char *str);
void disply_userinfo(struct userec *u, int real);
int uinfo_query(struct userec *u, int real, int unum);
int x_info(const char *s);
int x_fillform(const char *s);
#endif //BMYBBS_USERINFO_H
