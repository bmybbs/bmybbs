#ifndef BMYBBS_DELETE_H
#define BMYBBS_DELETE_H
#include "ythtbbs/cache.h"

int offline(const char *s);
int online(const char *s);
int kick_user(const struct user_info *userinfo, int mode);
#endif //BMYBBS_DELETE_H
