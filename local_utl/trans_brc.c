//by ylsdd
//translate old .boardrc to brc, should be modified according to diferent system
//and, for ytht brc, one should add save_brc to crontab, which save user's brc
//from tmpfs to ~bbs/home/?/userid/

#include "bbs.h"
#include "ythtlib.h"
#include "ythtbbs.h"
main()
{
	char path[1024], ent[1024], newent[1024];
	time_t nowtime;
	struct allbrc allbrc;
	int fd, t1, t2;
	DIR *dirp;
	struct dirent *direntp;
	char ch;

	chdir(MY_BBS_HOME);
	nowtime = time(NULL);
	printf("trans_brc is running~\n");
	printf("\033[1mbbs home=%s now time = %s\033[0m\n", MY_BBS_HOME,
	       ctime(&nowtime));
	for (ch = 'A'; ch <= 'Z'; ch++) {
		sprintf(path, MY_BBS_HOME "/home/%c", ch);
		printf("processing %s\n", path);
		dirp = opendir(path);
		if (dirp == NULL)
			continue;
		while ((direntp = readdir(dirp)) != NULL) {
			if (direntp->d_name[0] == '.')
				continue;
			//printf("%s\n", direntp->d_name);
			sprintf(ent, "%s/%s/.boardrc", path, direntp->d_name);
			sprintf(newent, "%s/%s/brc", path, direntp->d_name);
			if (!file_exist(ent) || !file_isfile(ent))
				continue;
			t1 = file_time(ent);
			t2 = file_time(newent);
			if (file_exist(newent) && t2 > t1)
				continue;
			printf("%s\n", direntp->d_name);
			brc_init_old(&allbrc, ent);
			if (!allbrc.size)
				continue;
			sprintf(ent, "%s/%s/brc.tmp", path, direntp->d_name);
			fd = open(ent, O_WRONLY | O_CREAT, 0660);
			if (fd < 0)
				continue;
			write(fd, allbrc.brc_c, allbrc.size);
			close(fd);
			rename(ent, newent);
		}
		closedir(dirp);
	}
}
