#ifndef BMYBBS_BBS_MAIN
#define BMYBBS_BBS_MAIN
#include "ythtbbs/user.h"
extern int showansi;
extern int RMSG;
extern jmp_buf byebye ;        /* Used for exception condition like I/O error*/

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
