#ifndef BMYBBS_IO_H
#define BMYBBS_IO_H
#include <time.h>
extern time_t now_t, old;

void oflush(void);
void init_alarm(void);
void ochar(int c);
void io_output(const char *s, int len);
void add_io(int fd, int timeout);
void add_flush(void (*flushfunc)(void));
int num_in_buf(void);
int igetkey(void);
int ask(char *prompt);
int getdata(int line, int col, char *prompt, char *buf, int len, int echo, int clearlabel);
int multi_getdata(int line, int col, int maxcol, char *prompt, char *buf, int len, int maxline, int clearlabel);
void getfield(int line, char *info, char *desc, char *buf, int len);
#endif //BMYBBS_IO_H
