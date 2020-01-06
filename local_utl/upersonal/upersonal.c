#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "bbs.h"
#include "ythtbbs.h"
#define PersonalPATH MY_BBS_HOME"/0Announce/groups/GROUP_0/Personal_Corpus"
struct PInfo {
	char name[14];
	long tstay;
	int ntime;
} pinfotopntime[100], apinfo, temppinfo0, temppinfo1;

main()
{
	DIR *dirp;
	char ch, path[500], fn[500];
	struct dirent *direntp;
	int i, fd;

	bzero(pinfotopntime, sizeof (struct PInfo) * 100);
	for (ch = 'A'; ch <= 'Z'; ch++) {
		sprintf(path, "%s/%c", PersonalPATH, ch);
		dirp = opendir(path);
		if (dirp == NULL)
			continue;
		while ((direntp = readdir(dirp)) != NULL) {
			if (direntp->d_name[0] == '.')
				continue;
			sprintf(fn, "%s/%s/.logvisit", path, direntp->d_name);
			sprintf(apinfo.name, "%.12s", direntp->d_name);
			fd = open(fn, O_RDONLY);
			if (fd < 0)
				continue;
/*         unlink(fn); */
			getpinfo(fd);
			close(fd);
		}
	}
	closedir(dirp);
	printf("个人文集\t访问次数\t累计时间\n");
	for (i = 0; i < 100; i++) {
		if (pinfotopntime[i].ntime == 0)
			break;
		printf("%-14.12s\t%8d\t%8ld\n", pinfotopntime[i].name,
		       pinfotopntime[i].ntime, pinfotopntime[i].tstay);
	}
}

getpinfo(int fd)
{
	int retv, i, j;
	retv = read(fd, &apinfo.ntime, sizeof (int));
	read(fd, &apinfo.tstay, sizeof (int));
	if (retv < 0) {
		apinfo.ntime = 0;
		apinfo.tstay = 0;
	}
	for (i = 0; i < 100; i++) {
		if (apinfo.ntime > pinfotopntime[i].ntime)
			break;
		if (apinfo.ntime == pinfotopntime[i].ntime &&
		    apinfo.tstay > pinfotopntime[i].tstay)
			break;
	}
	for (j = i, temppinfo1 = apinfo; j < 100; j++) {
		temppinfo0 = pinfotopntime[j];
		pinfotopntime[j] = temppinfo1;
		temppinfo1 = temppinfo0;
	}
}
