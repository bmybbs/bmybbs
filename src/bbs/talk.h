#ifndef BMYBBS_TALK_H
#define BMYBBS_TALK_H
#include "ythtbbs/cache.h"
#include "ythtbbs/override.h"

int t_pager(const char *s);
int t_query(const char *q_id);
void num_alcounter(void);
int num_useshell(void);
int t_cmpuids(int uid, struct user_info *up);
int t_talk(const char *s);
int ttt_talk(const struct user_info *userinfo);
int servicepage(int line, char *mesg);
int talkreply(void);
void endmsg(void);
int listfilecontent(char *fname);
int addtooverride(const char *uident);
int deleteoverride(const char *uident, const enum ythtbbs_override_type override_type);
int override_add(void);
int t_friend(const char *s);
int t_reject(const char *s);
struct user_info *t_search(char *sid, int pid, int invisible_check);
int getfriendstr(void);
int getrejectstr(void);
int wait_friend(const char *s);
#endif //BMYBBS_TALK_H
