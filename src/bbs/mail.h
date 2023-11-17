#ifndef BMYBBS_MAIL_H
#define BMYBBS_MAIL_H
#include "ythtbbs/article.h"

extern int in_mail;

char *email_domain(void);
void filter(char *line);
int chkmail(void);
int check_query_mail(char qry_mail_dir[80]);
int mailall(const char *s);
int m_internet(const char *s);
void m_init(void);
int m_send(const char *userid);
int M_send(const char *s);
int m_new(const char *s);
int mail_reply(int ent, struct fileheader *fileinfo, char *direct);
int mail_forward(int ent, struct fileheader *fileinfo, char *direct);
int mail_u_forward(int ent, struct fileheader *fileinfo, char *direct);
int m_read(const char *s);
int invalidaddr(char *addr);
int g_send(const char *s);
time_t mail_file(const char *tmpfile, const char *userid, const char *title);
int mail_buf(char *buf, char userid[], char title[]);
int ov_send(const char *s);
int voter_send(char *fname);
int club_send(void);
int doforward(char *filepath, char *oldtitle, int mode);
int m_cancel(const char *userid);
int post_reply(int ent, struct fileheader *fileinfo, char *direct);
#endif //BMYBBS_MAIL_H
