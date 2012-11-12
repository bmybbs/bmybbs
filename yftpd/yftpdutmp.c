#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/file.h>
#include <signal.h>

#include "main.h"
#include "yftpdutmp.h"
#include "options.h"
#include "logging.h"
#include "login.h"

FILE *yftpdutmp = NULL;
long yftpdutmp_offset = 0xFFFFFFFF;

void
yftpdutmp_init()
{
	/* First we have to create the file if it doesn't exist */
	yftpdutmp = fopen(PATH_YFTPDUTMP, "a");
	if (yftpdutmp)
		fclose(yftpdutmp);
	/* Then we can open it for reading and writing */
	if (!(yftpdutmp = fopen(PATH_YFTPDUTMP, "r+"))) {
		prints("421-Could not open UTMPFILE\r\n"
		       "421 Server disabled for security reasons.");
		exit(1);
	}
	rewind(yftpdutmp);
}

void
yftpdutmp_end()
{
	if (yftpdutmp) {
		if (yftpdutmp_offset != -1)
			yftpdutmp_log(0);
		fclose(yftpdutmp);
		yftpdutmp = NULL;
	}
}

int
yftpdutmp_log(char type)
{
	struct yftpdutmp ut, tmp;
	long i;
	int fd, nlogin = 0;

	if (!yftpdutmp)
		return -1;
	memset((void *) &ut, 0, sizeof (ut));
	ut.bu_pid = getpid();
	fd = open(PATH_UTMPLOCK, O_CREAT | O_RDONLY, 0600);

	if (flock(fd, LOCK_EX) < 0) {
		flock(fd, LOCK_UN);
		close(fd);
		return -1;
	}

	if (type) {
		ut.bu_type = 1;
		strncpy(ut.bu_name, user, sizeof (ut.bu_name));
		strncpy(ut.bu_host, remotehostname, sizeof (ut.bu_host));
		/* Determine offset of first user marked dead */
		rewind(yftpdutmp);
		i = 0;
		while (i < MAX_FTPUSER
		       && fread((void *) &tmp, sizeof (tmp), 1, yftpdutmp)) {
			if (!tmp.bu_type || kill(tmp.bu_pid, 0) < 0)
				continue;
			if (!isanonymous && !strncmp
			    (tmp.bu_name, ut.bu_name, sizeof (ut.bu_name)))
				nlogin++;
			else
			    if (!strncmp
				(tmp.bu_host, ut.bu_host,
				 sizeof (ut.bu_host))) nlogin++;
			if (nlogin >= 2) {
				flock(fd, LOCK_UN);
				close(fd);
				return -2;
			}
			i++;
		}
		rewind(yftpdutmp);
		i = 0;
		while (i < MAX_FTPUSER
		       && fread((void *) &tmp, sizeof (tmp), 1, yftpdutmp)) {
			if (!tmp.bu_type || kill(tmp.bu_pid, 0) < 0)
				break;
			i++;
		}
		if (i >= MAX_FTPUSER) {
			flock(fd, LOCK_UN);
			close(fd);
			return -3;
		}
		yftpdutmp_offset = i * sizeof (tmp);
	} else
		ut.bu_type = 0;
	time(&(ut.bu_time));
	fseek(yftpdutmp, yftpdutmp_offset, SEEK_SET);
	fwrite((void *) &ut, sizeof (ut), 1, yftpdutmp);
	fflush(yftpdutmp);
	flock(fd, LOCK_UN);
	close(fd);
	return 0;
}

char
yftpdutmp_pidexists(pid_t pid)
{
	struct yftpdutmp tmp;
	rewind(yftpdutmp);
	while (fread((void *) &tmp, sizeof (tmp), 1, yftpdutmp)) {
		if (tmp.bu_pid == pid)
			return 1;
	}
	return 0;
}

char
yftpdutmp_userexists(char *username)
{
	struct yftpdutmp tmp;
	rewind(yftpdutmp);
	while (fread((void *) &tmp, sizeof (tmp), 1, yftpdutmp)) {
		if (!strcmp(tmp.bu_name, username))
			return 1;
	}
	return 0;
}
