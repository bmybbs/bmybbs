#ifndef BMYBBS_READ_H
#define BMYBBS_READ_H
#include "ythtbbs/article.h"

struct keeploc *getkeep(char *s, int def_topline, int def_cursline);
void fixkeep(char *s, int first, int last);
void i_read(int cmdmode, char *direct, int (*dotitle)(void), char *(*doentry)(int, void *, char *), const struct one_key *rcmdlist, int ssize);
int auth_search_down(int ent, struct fileheader *fileinfo, char *direct);
int auth_search_up(int ent, struct fileheader *fileinfo, char *direct);
int post_search_down(int ent, struct fileheader *fileinfo, char *direct);
int post_search_up(int ent, struct fileheader *fileinfo, char *direct);
int show_author(int ent, struct fileheader *fileinfo, char *direct);
int friend_author(int ent, struct fileheader *fileinfo, char *direct);
int SR_BMfunc(int ent, struct fileheader *fileinfo, char *direct);
int SR_first_new(int ent, struct fileheader *fileinfo, char *direct);
int SR_last(int ent, struct fileheader *fileinfo, char *direct);
int SR_first(int ent, struct fileheader *fileinfo, char *direct);
int SR_read(int ent, struct fileheader *fileinfo, char *direct);
int SR_author(int ent, struct fileheader *fileinfo, char *direct);
int t_search_down(int ent, struct fileheader *fileinfo, char *direct);
int t_search_up(int ent, struct fileheader *fileinfo, char *direct);
int thread_up(int ent, struct fileheader *fileinfo, char *direct);
int thread_down(int ent, struct fileheader *fileinfo, char *direct);
int sread(int passonly, int readfirst, int pnum, int auser, struct fileheader *ptitle);
int searchpattern(char *filename, char *query);
#endif //BMYBBS_READ_H
