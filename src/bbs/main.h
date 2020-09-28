#ifndef BMYBBS_BBS_MAIN
#define BMYBBS_BBS_MAIN
#include "ythtbbs/user.h"

int ifinprison(char *name);
int cmpuids(char *uid, struct userec *up);
int dosearchuser(char *userid);
void abort_bbs(void);
void set_numofsig(void);
int egetch(void);
void update_endline(void);
void showtitle(char *title, char *mid);
void docmdtitle(char *title, char *prompt);
int Q_Goodbye(void);
#endif
