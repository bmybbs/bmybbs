#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_DIRENT_H
#  include <dirent.h>
#else
#    define dirent direct
#    define NAMLEN(dirent) (dirent)->d_namlen
#  ifdef HAVE_SYS_NDIR_H
#    include <sys/ndir.h>
#  endif
#  ifdef HAVE_SYS_DIR_H
#    include <sys/dir.h>
#  endif
#  ifdef HAVE_NDIR_H
#    include <ndir.h>
#  endif
#endif

#include <unistd.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <stdarg.h>
#include <string.h>
#include "mystring.h"
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <errno.h>
#include <glob.h>

#include "main.h"
#include "login.h"
#include "commands.h"
#include "pathop.h"

void
yftpd_stat(char *name, FILE * client, char verbose)
{
	struct stat statbuf;
	char temp[MAXCMD + 3], linktarget[MAXCMD + 5], perm[11], timestr[17];
	struct tm filetime;
	time_t t;
	//maybe some directory should be hiden. if so, return
	if (my_lstat(name, (struct stat *) &statbuf) < 0)
		return;
	if (!verbose) {
		fprintf(client, "%s\r\n", name);
		return;
	}
#ifdef S_ISLNK
	if (S_ISLNK(statbuf.st_mode)) {
		strcpy(perm, "lrwxrwxrwx");
		temp[readlink(name, temp, sizeof (temp) - 1)] = '\0';
		sprintf(linktarget, " -> %s", temp);
	} else {
#endif
		strcpy(perm, "----------");
		if (S_ISDIR(statbuf.st_mode))
			perm[0] = 'd';
		if (statbuf.st_mode & S_IRUSR)
			perm[1] = 'r';
		if (statbuf.st_mode & S_IWUSR)
			perm[2] = 'w';
		if (statbuf.st_mode & S_IXUSR)
			perm[3] = 'x';
		if (statbuf.st_mode & S_IRGRP)
			perm[4] = 'r';
		if (statbuf.st_mode & S_IWGRP)
			perm[5] = 'w';
		if (statbuf.st_mode & S_IXGRP)
			perm[6] = 'x';
		if (statbuf.st_mode & S_IROTH)
			perm[7] = 'r';
		if (statbuf.st_mode & S_IWOTH)
			perm[8] = 'w';
		if (statbuf.st_mode & S_IXOTH)
			perm[9] = 'x';
		linktarget[0] = '\0';
#ifdef S_ISLNK
	}
#endif
	memcpy(&filetime, localtime(&(statbuf.st_mtime)), sizeof (struct tm));
	time(&t);
	if (filetime.tm_year == localtime(&t)->tm_year)
		mystrncpy(timestr, ctime(&(statbuf.st_mtime)) + 4, 12);
	else
		strftime(timestr, sizeof (timestr), "%b %d  %G", &filetime);
	fprintf(client, "%s %3i %-8s %-8s %8lu %s %s%s\r\n", perm,
		(int) statbuf.st_nlink, "bbs", "nobody",
		(unsigned long) statbuf.st_size, timestr, name, linktarget);
}

void
dirlist(char *name, FILE * client, char verbose)
{
	DIR *directory;
	char cwd[256];
	int i;
	glob_t globbuf;
	if ((strstr(name, "/.")) && strchr(name, '*'))
		return;		/* DoS protection */
	if ((directory = my_opendir(name))) {
		closedir(directory);
		my_getcwd(cwd, sizeof (cwd) - 1);
		my_chdir(name);
		glob("*", 0, NULL, &globbuf);
	} else
		glob(name, 0, NULL, &globbuf);
	for (i = 0; i < globbuf.gl_pathc; i++)
		yftpd_stat(globbuf.gl_pathv[i], client, verbose);
	my_chdir(cwd);
	globfree(&globbuf);
}
