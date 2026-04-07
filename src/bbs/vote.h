#ifndef BBS_VOTE_H
#define BBS_VOTE_H
#include "vote_defs.h"

void makevdir(char *bname);
void setvfile(char *buf, char *bname, char *filename);
int b_closepolls(void);
int vote_maintain(char *bname);
int b_vote_maintain(int, void *, char *);
int m_voter(int, void *, char *);
int b_vote(int, void *, char *);
int b_results(int, void *, char *);
int m_vote(const char *s);
#endif
