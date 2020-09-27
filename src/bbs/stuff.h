#ifndef BMYBBS_STUFF_H
#define BMYBBS_STUFF_H
int dashf(const char *fname);
int dashd(const char *fname);
int dashl(const char *fname);
int pressreturn(void);
int pressanykey(void);
int askyn(char *str, int defa, int gobottom);
void printdash(char *mesg);
void bell(void);
int deltree(char *dst);
int do_exec(char *com, char *wd);
#endif //BMYBBS_STUFF_H
