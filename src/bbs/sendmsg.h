#ifndef BMYBBS_SENDMSG_H
#define BMYBBS_SENDMSG_H
#include "struct.h"

int canmsg(struct user_info *uin);
int s_msg(void);
int do_sendmsg(char *uid, struct user_info *uentp, char msgstr[256], int mode, int userpid);
int wall(void);
int wall_telnet(void);
int friend_wall(void);
void r_msg2(void);
void r_msg(void);
void block_msg(void);
void unblock_msg(void);
int friend_login_wall(struct user_info *pageinfo);
int show_allmsgs(void);
#endif //BMYBBS_SENDMSG_H
