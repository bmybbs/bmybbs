#ifndef BMYBBS_BM_H
#define BMYBBS_BM_H
#include "ythtbbs/article.h"
int deny_user(void);
int addclubmember(char *uident, int clubnum);
int clubmember(void);
int deny_from_article(int ent, struct fileheader *fileinfo, char *direct);
int mail_buf_slow(char *userid, char *title, char *content, char *sender);
#endif //BMYBBS_BM_H
