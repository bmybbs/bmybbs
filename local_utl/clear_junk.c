#include "bbs.h"
#include "ythtlib.h"

int clear_junk(char *junk, char *junk2);
int
main()
{
	int i;
	char junk[256], junk2[256];
	char *(disks[]) = {
		MY_BBS_HOME "/0Announce/",
		MY_BBS_HOME "/boards/",
		MY_BBS_HOME "/mail/",
		MY_BBS_HOME "/",
		NULL
	};
	for (i = 0; disks[i]; i++) {
		sprintf(junk, "%s.junk", disks[i]);
		sprintf(junk2, "%s.junk2", disks[i]);
		//printf("%s\n%s\n", junk, junk2);
		clear_junk(junk, junk2);
	}
	return 0;
}

int
clear_junk(char *junk, char *junk2)
{
	DIR *dirp;
	struct dirent *direntp;
	struct stat sbuf;
	time_t n;
	int day;
	int i = 0;
	char buf[PATH_MAX];

	chdir(junk);
	dirp = opendir(junk);
	if (dirp == NULL)
		return -1;
	time(&n);
	day = (n - 937669132) / 86400 + 1;
	while ((direntp = readdir(dirp)) != NULL) {
		if (direntp->d_name[0] == '.')
			continue;
		if (lstat(direntp->d_name, &sbuf))
			continue;
		//垃圾只保留75小时,比3天多一点.
		if (n - sbuf.st_ctime > 75 * 3600) {
			if (S_ISREG(sbuf.st_mode))
				unlink(direntp->d_name);
			else if (S_ISDIR(sbuf.st_mode)) {
				snprintf(buf, sizeof (buf), "%s/%s.%d.%d",
					 junk2, direntp->d_name, day, i++);
				rename(direntp->d_name, buf);
			} else {
				errlog("strange error:%s", direntp->d_name);
				continue;
			}
		}

	}
	closedir(dirp);
	sync();
	return 0;
}
