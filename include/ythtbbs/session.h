#ifndef BMYBBS_SESSION_H
#define BMYBBS_SESSION_H
#include <stddef.h>

void ythtbbs_session_generate_id(char *buf, size_t len);
int ythtbbs_session_set(const char *sessionid, const char *userid, const int utmp_idx);
int ythtbbs_session_del(const char *sessionid);
int ythtbbs_session_get_utmp_idx(const char *sessionid, const char *userid);
#endif
