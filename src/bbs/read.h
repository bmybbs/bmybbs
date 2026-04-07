#ifndef BMYBBS_READ_H
#define BMYBBS_READ_H
#include "one_key.h"

extern char currdirect[];

struct keeploc *getkeep(char *s, int def_topline, int def_cursline);
void fixkeep(char *s, int first, int last);
void i_read(int cmdmode, char *direct, int (*dotitle)(void), char *(*doentry)(int, void *, char *), const struct one_key *rcmdlist, int ssize);
int auth_search_down(int, void *, char *);
int auth_search_up(int, void *, char *);
int post_search_down(int, void *, char *);
int post_search_up(int, void *, char *);
int show_author(int, void *, char *);
int friend_author(int, void *, char *);
int SR_BMfunc(int, void *, char *);
int SR_first_new(int, void *, char *);
int SR_last(int, void *, char *);
int SR_first(int, void *, char *);
int SR_read(int, void *, char *);
int SR_author(int, void *, char *);
int t_search_down(int, void *, char *);
int t_search_up(int, void *, char *);
int thread_up(int, void *, char *);
int thread_down(int, void *, char *);
int sread(int passonly, int readfirst, int pnum, int auser, void *record);
int searchpattern(char *filename, char *query);
#endif //BMYBBS_READ_H
