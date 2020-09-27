/**
 * 本文件为临时文件，用于从 include/bbs.h 中剥离出仅适
 * 用于 src/bbs 的宏、变量声明等，后续需要进一步重构。
 * TODO
 */

#ifndef BMYBBS_BBSINC_H
#define BMYBBS_BBSINC_H
#include <setjmp.h>
extern int nettyNN;
extern int selboard;           /* THis flag is true if above is active */
extern int showansi;
extern int in_mail;
extern jmp_buf byebye ;        /* Used for exception condition like I/O error*/

void setqtitle(char *stitle);
char *setbfile(char *buf, char *boardname, char *filename);
void setbdir(char *buf, char *boardname, int Digestmode);

int postfile(char *filename, char *nboard, char *posttitle, int mode);
#endif //BMYBBS_BBSINC_H
