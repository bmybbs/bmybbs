#ifndef BMYBBS_READ_H
#define BMYBBS_READ_H

#include "ythtbbs/article.h"
int sread(int passonly, int readfirst, int pnum, int auser, struct fileheader *ptitle);
void i_read(int cmdmode, char *direct, int (*dotitle) (), char *(*doentry) (int, void *, char *), const struct one_key *rcmdlist, int ssize);
#endif //BMYBBS_READ_H
