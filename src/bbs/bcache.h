#ifndef BMYBBS_BCACHE_H
#define BMYBBS_BCACHE_H
#include "ythtbbs/board.h"
#include "ythtbbs/cache.h"

void attach_err(int shmkey, char *name);
void *attach_shm(int shmkey, int shmsize);
int hasreadperm(struct boardheader *bh);
int hasreadperm_ext(char *username, char *boardname);
int getbnum(const char *bname);
int canberead(const char *bname);
int noadm4political(const char *bname);
int haspostperm(char *bname);
int posttest(char *uid, char *bname);
int normal_board(char *bname);
int innd_board(char *bname);
int is1984_board(char *bname);
int club_board(char *bname);
int clubsync(char *boardname);
int getuser(const char *userid);
int search_ulist(struct user_info *uentp, int (*fptr)(int, struct user_info *), int farg);
int search_ulistn(struct user_info *uentp, int (*fptr)(int, struct user_info *), int farg, int unum);
int count_logins(struct user_info *uentp, int (*fptr)(int, struct user_info *), int farg, int show);
int t_search_ulist(int (*fptr)(int, struct user_info *), int farg);
int user_isonline(char *userid);
void update_ulist(struct user_info *uentp, int uent);
void update_utmp(void);
int get_utmp(void);
int who_callme(struct user_info *uentp, int (*fptr)(int, struct user_info *), int farg, int me);
int getbmnum(char *userid);
char *get_temp_sessionid(char *temp_sessionid, size_t len);
void show_small_bm(char *board);
int setbmstatus(int online);
#endif //BMYBBS_BCACHE_H
