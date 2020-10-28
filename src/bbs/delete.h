#ifndef BMYBBS_DELETE_H
#define BMYBBS_DELETE_H
#include "ythtbbs/cache.h"

void offline(void);
int online(void);
int kick_user(struct user_info *userinfo, int mode);
#endif //BMYBBS_DELETE_H
