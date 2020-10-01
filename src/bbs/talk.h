#ifndef BMYBBS_TALK_H
#define BMYBBS_TALK_H
#include "struct.h"

int t_pager(void);
int t_query(char q_id[12 + 2]);
void num_alcounter(void);
int num_useshell(void);
int num_active_users(void);
int t_cmpuids(int uid, struct user_info *up);
int t_talk(void);
int ttt_talk(struct user_info *userinfo);
int servicepage(int line, char *mesg);
int talkreply(void);
void endmsg(void);
int listfilecontent(char *fname);
int addtooverride(char *uident);
int deleteoverride(char *uident, char *filename);
int override_add(void);
void t_friend(void);
void t_reject(void);
struct user_info *t_search(char *sid, int pid, int invisible_check);
int getfriendstr(void);
int getrejectstr(void);
int wait_friend(void);
#endif //BMYBBS_TALK_H
