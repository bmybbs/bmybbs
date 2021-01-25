#ifndef BMYBBS_MAIL_H
#define BMYBBS_MAIL_H
#include "ythtbbs/article.h"

extern int in_mail;

char *email_domain(void);
void filter(char *line);
int chkmail(void);
int check_query_mail(char qry_mail_dir[80]);
int mailall(void);
void m_internet(void);
void m_init(void);
int m_send(const char *userid);
int M_send(void);
int m_new(void);
int mail_reply(int ent, struct fileheader *fileinfo, char *direct);
int mail_forward(int ent, struct fileheader *fileinfo, char *direct);
int mail_u_forward(int ent, struct fileheader *fileinfo, char *direct);
int m_read(void);
int invalidaddr(char *addr);
int g_send(void);
time_t mail_file(char tmpfile[80], char userid[80], char title[80]);
int mail_buf(char *buf, char userid[], char title[]);
int ov_send(void);
int voter_send(char *fname);
int club_send(void);
int doforward(char *filepath, char *oldtitle, int mode);
int m_cancel(char userid[]);
int post_reply(int ent, struct fileheader *fileinfo, char *direct);
#endif //BMYBBS_MAIL_H
