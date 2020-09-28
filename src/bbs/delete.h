#ifndef BMYBBS_DELETE_H
#define BMYBBS_DELETE_H
int d_board(void);
void offline(void);
int online(void);
int d_user(char *cid);
int kick_user(struct user_info *userinfo, int mode);
#endif //BMYBBS_DELETE_H
