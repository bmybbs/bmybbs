#include "bbs.h"
#include "ythtlib.h"

int
main()
{
	DIR *dirp;
	struct dirent *direntp;
	struct stat sbuf;
	time_t n;
	int day;
	int i = 0;
	char buf[256];

	chdir(ATTACHCACHE);
	dirp = opendir(ATTACHCACHE);
	if (dirp == NULL)
		return -1;
	time(&n);
	while ((direntp = readdir(dirp)) != NULL) {
		if (direntp->d_name[0] == '.')
			continue;
		if (lstat(direntp->d_name, &sbuf))
			continue;
		//只保留最近不到2天有人访问过的附件.
		if (n - sbuf.st_atime > 2.7 * 3600 * 24) {
			if (S_ISREG(sbuf.st_mode))
				unlink(direntp->d_name);
			else {
				errlog("strange error:%s", direntp->d_name);
				continue;
			}
		}

	}
	closedir(dirp);
	return 0;
}
