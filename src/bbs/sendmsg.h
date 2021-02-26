#ifndef BMYBBS_SENDMSG_H
#define BMYBBS_SENDMSG_H
#include "ythtbbs/cache.h"

int canmsg(const struct user_info *uin);
int s_msg(const char *s);
int do_sendmsg(const char *uid, const struct user_info *uentp, char *msgstr, int mode, int userpid);
int wall(const char *s);
int wall_telnet(const char *s);
int friend_wall(const char *s);
void r_msg2(void);
void r_msg(void);
void block_msg(void);
void unblock_msg(void);
int friend_login_wall(const struct user_info *pageinfo, void *x_param);
int show_allmsgs(const char *s);
#endif //BMYBBS_SENDMSG_H
