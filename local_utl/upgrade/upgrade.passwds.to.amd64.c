/************************************************************************
 *
 * @file upgrade.passwds.to.amd64.c
 * @author IronBlood
 *
 * 由于 time_t 在不同架构下的长度不同，导致对应的 .PASSWDS 文件格式产生差异。
 * 本程序仅供从 32位 Linux 下的 .PASSWDS 文件升级到 64位 Linux 程序所需要的
 * 格式。
 *
 * 此处 struct userec_32 与系统使用的 userec 唯一差异在于，使用 int 类型指代
 * 了 time_t，确保结构体长度一致。
 *
 * 对应的，fileheader 结构体也可以使用这样的方法升级。
 *
 ************************************************************************/

#include "bbs.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


struct userec_32 {
	char userid[IDLEN + 2];
	int firstlogin;
	char lasthost[16];
	unsigned int numlogins;
	unsigned int numposts;
	char flags[2];
	char passwd[PASSLEN];   //!<加密后的密码
	char username[NAMELEN];
	unsigned short numdays;	//!<曾经登录的天数
	char unuse[30];
	int dietime;
	int lastlogout;
	char ip[16];
	char realmail[STRLEN - 16];
	unsigned userlevel;
	int lastlogin;
	int stay;
	char realname[NAMELEN];
	char address[STRLEN];
	char email[STRLEN];
	int signature;
	unsigned int userdefine;
	int notedate_nouse;
	int noteline_nouse;
	int notemode_nouse;
};

int main() {
	int fd_32;
	struct stat st_32;
	void * ummap_ptr_32=NULL;
	int ummap_size=0;

	fd_32 = open(".PASSWDS.32", O_RDONLY);

	fstat(fd_32, &st_32);

	ummap_ptr_32 = mmap(NULL, st_32.st_size, PROT_READ, MAP_SHARED, fd_32, 0);


	ummap_size=st_32.st_size;
	int user_number = ummap_size / sizeof(struct userec_32);

	struct userec_32 ibu;
	struct userec ue;

	int i;
	for(i=0; i<user_number; i++) {
		memset(&ue, 0, sizeof(ue));

		memcpy(&ibu, ummap_ptr_32 + sizeof(ibu) * i, sizeof(ibu));

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

		append_record(".PASSWDS.64", &ue, sizeof(ue));

		fprintf(stdout, "added user %s\n", ue.userid);
	}

	return 0;
}
