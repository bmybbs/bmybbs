//查找和删除过期的board目录下的.DIR衍生产物     yuhuan  2002/7/11

#include "bbs.h"
#define MAXFILE 5000
#define MINAGE 50000		//at least 50000 sec old
#ifdef NUMBUFFER
#undef NUMBUFFER
#endif
#define NUMBUFFER 100
#define HASHSIZE 40

char *(spcname[]) = {
	".DIR",
	    ".JUNK",
	    ".DELETED",
	    ".DIGEST", ".deleted", ".deleteD", ".deleteJ", ".", "..", NULL};

int
isfakedir(char *file)
{
	int i;
	for (i = 0; spcname[i] != NULL; i++)
		if (!strcmp(file, spcname[i]))
			return 0;
	return 1;
}

int
rm_fakedir(char *path)
{
	DIR *dirp;
	struct dirent *direntp;
	int h;
	dirp = opendir(path);
	if (dirp == NULL)
		return -1;
	while ((direntp = readdir(dirp)) != NULL) {
		if (direntp->d_name[0] != '.')
			continue;
		if (isfakedir(direntp->d_name)) {
			printf("unlink %s\n", direntp->d_name);
			unlink(direntp->d_name);
		}
	}
	closedir(dirp);
	return 0;
}

int
auto_rm_fakedir(struct boardheader *bhp)
{
	char buf[200];
	int i;
	snprintf(buf, sizeof buf, MY_BBS_HOME "/boards/%s", bhp->filename);
	if (rm_fakedir(buf) < 0)
		return -1;
	return 0;
}

main()
{
	int b_fd;
	struct boardheader bh;
	int size;
	time_t nowtime;

	size = sizeof (bh);

	chdir(MY_BBS_HOME);
	nowtime = time(NULL);
	printf("auto_rm_fakedir is running~\n");
	printf("\033[1mbbs home=%s now time = %s\033[0m\n", MY_BBS_HOME,
	       ctime(&nowtime));
	if ((b_fd = open(MY_BBS_HOME "/.BOARDS", O_RDONLY)) == -1)
		return -1;
	flock(b_fd, LOCK_EX);
	while (read(b_fd, &bh, size) == size) {
		if (!bh.filename[0])
			continue;
		printf("processing %s\n", bh.filename);
		fflush(stdout);
		if (auto_rm_fakedir(&bh) < 0) {
			time(&nowtime);
			printf(" FAILED! %s", ctime(&nowtime));
		} else {
			time(&nowtime);
			printf(" ....OK. %s", ctime(&nowtime));
		}
	}
	flock(b_fd, LOCK_UN);
	close(b_fd);
}
