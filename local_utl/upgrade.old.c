#include "bbs.h"

struct FB2000userec {		/* Structure used to hold information in */
	char userid[IDLEN + 2];	/* PASSFILE */
	time_t firstlogin;
	char lasthost[16];
	unsigned int numlogins;
	unsigned int numposts;
	unsigned int medals;	/* 奖章数 */
	unsigned int money;	/* 金钱 */
	unsigned int inbank;	/* 存款 */
	time_t banktime;	/* 存入时间 */
	char flags[2];
#ifdef ENCPASSLEN
	char passwd[ENCPASSLEN];
#else
	char passwd[PASSLEN];
#endif
	char username[NAMELEN];
	char ident[NAMELEN];
	char termtype[16];
	char reginfo[STRLEN - 16];
	unsigned int userlevel;
	time_t lastlogin;
	time_t lastlogout;	/* 最近离线时间 */
	time_t stay;
	char realname[NAMELEN];
	char address[STRLEN];
	char email[STRLEN - 12];
	unsigned int nummails;
	time_t lastjustify;
	char gender;
	unsigned char birthyear;
	unsigned char birthmonth;
	unsigned char birthday;
	int signature;
	unsigned int userdefine;
	time_t notedate;
	int noteline;
};
//copied from FB2000

void
upgradepasswd()
{
	FILE *fr, *fw;
	struct FB2000userec fb2000;
	struct userec ytht;
	fr = fopen("./.PASSWDS", "r");
	if (NULL == fr) {
		printf("Can't open passwd file for read!\n");
		exit(1);
	}
	fw = fopen("./.PASSWDS.YTHT", "w");
	if (NULL == fw) {
		printf("Can't open passwd file for write!\n");
		fclose(fr);
		exit(2);
	}
	ytht.dietime = 0;
	ytht.notemode = 0;
	bzero(ytht.ip, 16);
	bzero(ytht.unuse, 32);
	while (fread(&fb2000, sizeof (fb2000), 1, fr) == 1) {
		snprintf(ytht.userid, IDLEN + 2, "%s", fb2000.userid);
		ytht.firstlogin = fb2000.firstlogin;
		snprintf(ytht.lasthost, 16, "%s", fb2000.lasthost);
		ytht.numlogins = fb2000.numlogins;
		ytht.numposts = fb2000.numposts;
		memcpy(ytht.flags, fb2000.flags, 2);
		memcpy(ytht.passwd, fb2000.passwd, PASSLEN);
		memcpy(ytht.username, fb2000.username, NAMELEN);
		ytht.lastlogout = fb2000.lastlogout;
		ytht.lastlogin = fb2000.lastlogin;
		ytht.stay = fb2000.stay;
		memcpy(ytht.realname, fb2000.realname, NAMELEN);
		ytht.userlevel = fb2000.userlevel;
		memcpy(ytht.address, fb2000.address, STRLEN);
		memcpy(ytht.email, fb2000.email, STRLEN - 12);
		ytht.signature = fb2000.signature;
		ytht.userdefine = fb2000.userdefine;
		ytht.userlevel = fb2000.userlevel;
		ytht.notedate = fb2000.notedate;
		ytht.noteline = fb2000.noteline;
		memcpy(ytht.realmail, fb2000.reginfo, STRLEN - 16);
		fwrite(&ytht, sizeof (ytht), 1, fw);
	}
	fclose(fr);
	fclose(fw);
}

int
main()
{
	upgradepasswd();
}
