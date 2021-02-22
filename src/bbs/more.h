#ifndef BMYBBS_MORE_H
#define BMYBBS_MORE_H

int NNread_init(void);
int setcalltime(const char *s);
int countln(char *fname);
void check_calltime(void);
void R_monitor(void);
int ansimore(char *filename, int promptend);
int ansimore_withzmodem(char *filename, int promptend, char *title);
int ansimore2(char *filename, int promptend, int row, int numlines);
int ansimore2stuff(char *filename, int promptend, int row, int numlines);
#endif //BMYBBS_MORE_H
