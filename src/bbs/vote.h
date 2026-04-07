#ifndef BBS_VOTE_H
#define BBS_VOTE_H
#include "vote_defs.h"

void makevdir(char *bname);
void setvfile(char *buf, char *bname, char *filename);
int b_closepolls(void);
int vote_maintain(char *bname);
int b_vote_maintain(void);
int m_voter(void);
int b_vote(void);
int b_results(void);
int m_vote(const char *s);
#endif
