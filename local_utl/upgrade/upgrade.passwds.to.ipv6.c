#include <sys/mman.h>
#include <stdio.h>

#include "bbs.h"

struct IPV4_userec {
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

int main(void) {
	int fd_old;
	struct stat st_old;
	void * ummap_ptr_old = NULL;
	unsigned int ummap_size = 0;
	unsigned int user_number;

	struct IPV4_userec ibu;
	struct userec ue;
	unsigned int i;

	fd_old = open(".PASSWDS.v4", O_RDONLY);
	fstat(fd_old, &st_old);
	ummap_size = st_old.st_size;
	ummap_ptr_old = mmap(NULL, ummap_size, PROT_READ, MAP_SHARED, fd_old, 0);
	user_number = ummap_size / sizeof(struct IPV4_userec);

	for(i=0; i<user_number; i++) {
		memset(&ue, 0, sizeof(ue));

		memcpy(&ibu, ummap_ptr_old + sizeof(ibu) * i, sizeof(ibu));

		// 赋予 ue

		strcpy(ue.userid, ibu.userid);
		ue.firstlogin = ibu.firstlogin;
		strcpy(ue.lasthost, ibu.lasthost);
		ue.numlogins = ibu.numlogins;
		ue.numposts = ibu.numposts;

		strcpy(ue.flags, ibu.flags);
		strcpy(ue.passwd, ibu.passwd);
		strcpy(ue.username, ibu.username);
		ue.numdays = ibu.numdays;
		strcpy(ue.unuse, ibu.unuse);

		ue.dietime = ibu.dietime;
		ue.lastlogout = ibu.lastlogout;
		strcpy(ue.ip, ibu.ip);
		strcpy(ue.realmail, ibu.realmail);
		ue.userlevel = ibu.userlevel;

		ue.lastlogin = ibu.lastlogin;
		ue.stay = ibu.stay;
		strcpy(ue.realname, ibu.realname);
		strcpy(ue.address, ibu.address);
		strcpy(ue.email, ibu.email);

		ue.signature = ibu.signature;
		ue.userdefine = ibu.userdefine;
		ue.notedate_nouse = ibu.notedate_nouse;
		ue.noteline_nouse = ibu.noteline_nouse;
		ue.notemode_nouse = ibu.notemode_nouse;

		append_record(".PASSWDS.v6", &ue, sizeof(ue));

		fprintf(stdout, "added user %s\n", ue.userid);
	}

	return 0;
}