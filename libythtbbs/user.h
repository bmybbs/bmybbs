/* user.c */
#ifndef __USER_H
#define __USER_H
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
char *sethomepath(char *buf, const char *userid);
char *sethomefile(char *buf, const char *userid, const char *filename);
char *setmailfile(char *buf, const char *userid, const char *filename);
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
int userbansite(const char *userid, const char *fromhost);
void logattempt(char *user,char *from,char *zone,time_t time);
int inoverride(char *who, char *owner, char *file);
#endif
