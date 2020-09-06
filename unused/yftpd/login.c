#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include "dirlist.h"
#include "mystring.h"
#include "login.h"
#include "logging.h"
#include "../include/bbs.h"
#include "yftpdutmp.h"
#include "options.h"
#include "main.h"

struct userec currentuser;
int clubrights[4];
int isanonymous = 0;
int checkuser(char *id, char *pw);

int
yftpd_login(char *password)
{
	struct stat buf;

	if (stat(PATH_DENY_LOGIN, &buf) == 0) {
		prints("421-Server disabled.\r\n");
		print_file(421, PATH_DENY_LOGIN);
		yftpd_log
		    ("Login as user '%s' failed: Server disabled.\n", user);
		exit(0);
	}
	if (!strcasecmp(user, "anonymous") || !strcasecmp(user, "guest")) {
		strcpy(user, "guest");
		isanonymous = 1;
	}
	if (!checkuser(user, password))
		return 1;
	if (seek_in_file(PATH_ADMINISTRATOR, user))
		currentuser.userlevel |= PERM_SYSOP;
	readclubrights();
	if (chdir(MY_FTP_ROOT)) {
		prints("421 Couldn't change cwd to ftp root: %s.",
		       strerror(errno));
		exit(1);
	}
	switch (yftpdutmp_log(1)) {
	case -1:
		prints("421 Internel error, sorry...");
		exit(1);
	case -2:
		prints("421 Multilogin is not allowed.");
		exit(1);
	case -3:
		prints("421 MAXUSER %d reached. Couldn't login", MAX_FTPUSER);
		exit(1);
	default:
		break;
	}
	prints("230 User %s logged in.", currentuser.userid);
	yftpd_log("Successfully logged in as user '%s'.\n", user);
	state = STATE_AUTHENTICATED;
	return 0;
}

int
getusernum(char *id)
{
	int i;
	struct UCACHE *shm_ucache;
	if (id[0] == 0)
		return -1;
	shm_ucache =
	    (struct UCACHE *) get_old_shm(UCACHE_SHMKEY,
					  sizeof (struct UCACHE));
	if (shm_ucache == NULL)
		return -1;
	for (i = 0; i < MAXUSERS; i++) {
		if (!strcasecmp(shm_ucache->userid[i], id)) {
			shmdt(shm_ucache);
			return i;
		}
	}
	shmdt(shm_ucache);
	return -1;
}

int
getuser(int uid, struct userec *userec1)
{
	int fd;
	if (uid < 0)
		return -1;
	fd = open(MY_BBS_HOME "/" PASSFILE, O_RDONLY);
	if (fd < 0)
		return -1;
	if ((off_t) - 1 == lseek(fd, uid * sizeof (struct userec), SEEK_SET)) {
		close(fd);
		return -1;
	}
	bzero(userec1, sizeof (struct userec));
	if (sizeof (struct userec) != read(fd, userec1, sizeof (struct userec))) {
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int
checkuser(char *id, char *pw)
{
	int currentuid;
	if ((currentuid = getusernum(id)) < 0)
		return 0;
	if (getuser(currentuid, &currentuser) < 0)
		return 0;
	if (strcasecmp(id, currentuser.userid))
		return 0;
	if (userbansite(currentuser.userid, remotehostname))
		return 0;
	if (isanonymous)
		return 1;
	if (!HAS_PERM(PERM_BASIC))
		return 0;
	if (!checkpasswd(currentuser.passwd, pw)) {
		time_t t = time(NULL);
		logattempt(currentuser.userid, remotehostname, "FTP", t);
		return 0;
	}
	return 1;
}

void
readclubrights()
{
	FILE *fp;
	int tempnum;
	char buf[STRLEN];
	sethomefile(buf, currentuser.userid, "clubrights");
	if ((fp = fopen(buf, "r")) == NULL) {
		memset(&clubrights, 0, 4 * sizeof (int));
	} else {
		memset(&clubrights, 0, 4 * sizeof (int));
		while (fgets(buf, STRLEN, fp) != NULL) {
			tempnum = atoi(buf);
			clubrights[tempnum / 32] |= (1 << tempnum % 32);
		}
		fclose(fp);
	}

}

int
hasreadperm(int clubnum, char flag, int level)
{
	if (clubnum != 0)
		return ((HAS_CLUBRIGHT(clubnum, clubrights))
			|| (flag & CLUBTYPE_FLAG));
	return level & PERM_POSTMASK || HAS_PERM(level)
	    || (level & PERM_NOZAP);
}

int
seek_in_file(const char *filename, const char *seekstr)
{
	FILE *fp;
	char buf[STRLEN];
	char *namep;
	if ((fp = fopen(filename, "r")) == NULL)
		return 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		namep = (char *) strtok(buf, ": \n\r\t");
		if (namep != NULL && strcasecmp(namep, seekstr) == 0) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}
