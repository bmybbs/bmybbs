#ifndef BMYBBS_BCACHE_H
#define BMYBBS_BCACHE_H
#include "ythtbbs/board.h"
#include "struct.h"

extern struct boardmem *bcache;
extern struct UCACHE *uidshm;
extern struct UCACHEHASH *uidhashshm;

void attach_err(int shmkey, char *name);
void *attach_shm(int shmkey, int shmsize);
void reload_boards(void);
void resolve_boards(void);
int apply_boards(int (*func)(struct boardmem *));
struct boardmem *getbcache(char *bname);
int updatelastpost(char *board);
int hasreadperm(struct boardheader *bh);
int hasreadperm_ext(char *username, char *boardname);
int getbnum(char *bname);
int canberead(char *bname);
int noadm4political(char *bname);
int haspostperm(char *bname);
int posttest(char *uid, char *bname);
int hideboard(char *bname);
int normal_board(char *bname);
int innd_board(char *bname);
int is1984_board(char *bname);
int political_board(char *bname);
int club_board(char *bname);
int clubsync(char *boardname);
void resolve_ucache(void);
void setuserid(int num, char *userid);
int searchnewuser(void);
void getuserid(char *userid, unsigned int uid);
int searchuser(char *userid);
int getuser(char *userid);
char *u_namearray(char buf[][12 + 1], int *pnum, char *tag);
void resolve_utmp(void);
int getnewutmpent(struct user_info *up);
int apply_ulist(int (*fptr)(struct user_info *));
int search_ulist(struct user_info *uentp, int (*fptr)(int, struct user_info *), int farg);
int search_ulistn(struct user_info *uentp, int (*fptr)(int, struct user_info *), int farg, int unum);
int count_logins(struct user_info *uentp, int (*fptr)(int, struct user_info *), int farg, int show);
int t_search_ulist(int (*fptr)(int, struct user_info *), int farg);
int user_isonline(char *userid);
void update_ulist(struct user_info *uentp, int uent);
void update_utmp(void);
int get_utmp(void);
void update_utmp2(void);
int who_callme(struct user_info *uentp, int (*fptr)(int, struct user_info *), int farg, int me);
int insertuseridhash(struct useridhashitem *ptr, int size, char *userid, int num);
int getbmnum(char *userid);
struct user_info *query_uindex(int uid, int dotest);
int count_uindex(int uid);
int count_uindex_telnet(int uid);
char *get_temp_sessionid(char *temp_sessionid);
void show_small_bm(char *board);
int setbmstatus(int online);
#endif //BMYBBS_BCACHE_H
