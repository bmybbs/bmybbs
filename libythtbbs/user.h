/* user.c */
#ifndef __USER_H
#define __USER_H

#include "struct.h"
#include "board.h"
/** Structure used to hold information in PASSFILE
 *
 */
struct userec {
	char userid[IDLEN + 2];
	time_t firstlogin;
	char lasthost[16];
	unsigned int numlogins;
	unsigned int numposts;
	char flags[2];
	char passwd[PASSLEN];   //!<加密后的密码
	char username[NAMELEN];
	unsigned short numdays;	//!<曾经登录的天数
	char unuse[30];
	time_t dietime;
	time_t lastlogout;
	char ip[16];
	char realmail[STRLEN - 16];
	unsigned userlevel;
	time_t lastlogin;
	time_t stay;
	char realname[NAMELEN];
	char address[STRLEN];
	char email[STRLEN];
	int signature;
	unsigned int userdefine;
	time_t notedate_nouse;
	int noteline_nouse;
	int notemode_nouse;
};

struct override {
	char id[13];
	char exp[40];
};

char mytoupper(unsigned char ch);
char *sethomepath(char *buf, const char *userid)  __attribute__((deprecated));
char *sethomefile(char *buf, const char *userid, const char *filename)  __attribute__((deprecated));
char *setmailfile(char *buf, const char *userid, const char *filename)  __attribute__((deprecated));

char *sethomepath_s(char *buf, size_t buf_size, const char *userid);
char *sethomefile_s(char *buf, size_t buf_size, const char *userid, const char *filename);
char *setmailfile_s(char *buf, size_t buf_size, const char *userid, const char *filename);
/* for sent mail */
char *setsentmailfile(char *buf, const char *userid, const char *filename);
int saveuservalue(char *userid, char *key, char *value);
int readuservalue(char *userid, char *key, char *value, int size);
char *charexp(int);
char *cperf(int);
int countexp(struct userec *);
int countperf(struct userec *);
int countlife(struct userec *);
int userlock(char *userid, int locktype);
int userunlock(char *userid, int fd);
int checkbansite(const char *addr);

/** 检查用户权限
 * 改方法从 nju09 移植，参见 int user_perm(struct userec *x, int level)。
 * @param x
 * @param level
 * @return
 */
int check_user_perm(struct userec *x, int level);

/**
 * @brief 检查用户的阅读权限
 * 该方法从 nju09 移植，用于检查用户读取版面的权限。
 * @param user
 * @param board 版面名称
 * @return
 * @see int has_read_perm(struct userec *user, char *board)
 * @see int has_read_perm_x(struct userec *user, struct boardmem *x)
 */
int check_user_read_perm(struct user_info *user, char *board);

/**
 * @brief 检查用户的阅读权限
 * 该方法从 nju09 移植，用于检查用户读取版面的权限。
 * @param user
 * @param board boardmem 指针
 * @return 有权限返回 1，无权限返回 0。
 * @see int has_read_perm(struct userec *user, char *board)
 * @see int has_read_perm_x(struct userec *user, struct boardmem *x)
 */
int check_user_read_perm_x(struct user_info *user, struct boardmem *board);

/**
 * @brief 检查用户的发帖权限
 * 该方法从 nju09 移植，会同时判断版面阅读权限。
 * @param user
 * @param board
 * @return 有权限返回 1，无权限返回 0。
 */
int check_user_post_perm_x(struct user_info *user, struct boardmem *board);
int userbansite(const char *userid, const char *fromhost);
void logattempt(char *user,char *from,char *zone,time_t time);
int inoverride(char *who, char *owner, char *file);

/**
 * @brief 检查 id 是否包含数字
 * 该方法在 telnet/nju09 中均有使用，移动到库中，方法名有待改进。
 * @param userid 字符串
 * @return 包含则返回
 */
int id_with_num(char *userid);

int chk_BM(struct userec *, struct boardheader *bh, int isbig);
int chk_BM_id(char *, struct boardheader *);
int bmfilesync(struct userec *);
#endif
