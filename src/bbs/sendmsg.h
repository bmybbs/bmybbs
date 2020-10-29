#ifndef BMYBBS_SENDMSG_H
#define BMYBBS_SENDMSG_H
#include "ythtbbs/cache.h"

int canmsg(struct user_info *uin);
int s_msg(void);
int do_sendmsg(const char *uid, const struct user_info *uentp, char *msgstr, int mode, int userpid);
int wall(void);
int wall_telnet(void);
int friend_wall(void);
void r_msg2(void);
void r_msg(void);
void block_msg(void);
void unblock_msg(void);
int friend_login_wall(const struct user_info *pageinfo, void *x_param);
int show_allmsgs(void);
#endif //BMYBBS_SENDMSG_H
