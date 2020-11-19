#ifndef BMYBBS_TALK_H
#define BMYBBS_TALK_H
#include "ythtbbs/cache.h"

int t_pager(void);
int t_query(const char *q_id);
void num_alcounter(void);
int num_useshell(void);
int t_cmpuids(int uid, struct user_info *up);
int t_talk(void);
int ttt_talk(const struct user_info *userinfo);
int servicepage(int line, char *mesg);
int talkreply(void);
void endmsg(void);
int listfilecontent(char *fname);
int addtooverride(const char *uident);
int deleteoverride(const char *uident, const char *filename);
int override_add(void);
void t_friend(void);
void t_reject(void);
struct user_info *t_search(char *sid, int pid, int invisible_check);
int getfriendstr(void);
int getrejectstr(void);
int wait_friend(void);
#endif //BMYBBS_TALK_H
