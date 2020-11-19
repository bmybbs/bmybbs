#ifndef BMYBBS_MAINTAIN_H
#define BMYBBS_MAINTAIN_H
int check_systempasswd(void);
void deliverreport(char *title, char *str);
void securityreport(char *str, char *content);
int get_grp(char seekstr[80]);
void stand_title(char *title);
int m_info(void);
int m_newbrd(void);
int m_editbrd(void);
int m_register(void);
int m_ordainBM(void);
int do_ordainBM(const char *userid, const char *abname);
int m_retireBM(void);
int do_retireBM(const char *userid, const char *abname);
int retireBM(char *uid, char *bname);
int retire_allBM(char *uid);
int m_addpersonal(void);
#endif //BMYBBS_MAINTAIN_H
