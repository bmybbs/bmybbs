#ifndef BMYBBS_MAINTAIN_H
#define BMYBBS_MAINTAIN_H
int check_systempasswd(void);
void deliverreport(char *title, char *str);
void securityreport(char *str, char *content);
int get_grp(char seekstr[80]);
void stand_title(char *title);
int m_info(const char *s);
int m_newbrd(const char *s);
int m_editbrd(const char *s);
int m_register(const char *s);
int m_ordainBM(const char *s);
int do_ordainBM(const char *userid, const char *abname);
int m_retireBM(const char *s);
int do_retireBM(const char *userid, const char *abname);
int m_addpersonal(const char *s);
#endif //BMYBBS_MAINTAIN_H
