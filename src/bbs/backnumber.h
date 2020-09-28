#ifndef BMYBBS_BACKNUMBER_H
#define BMYBBS_BACKNUMBER_H
#include <time.h>

void setbacknumberfile(char *path, char *filename);
int new_backnumber(void);
int selectbacknumber(void);
int do_intobacknumber(char *filename, time_t t);
#endif //BMYBBS_BACKNUMBER_H
