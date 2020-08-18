/*    自动删文系统    KCN 1998.12.10 		  */
/*    读取~~bbs/etc/autoclear文件，将板中过期的文章删除*/
/*    文件格式:					  */
/*       版名    保留文章数    保留多少天内的文章  */
/*     如:test  1000    30                        */
/*     表示删除test板的文章，文章数减到1000篇，    */
/*       但不删除加g,m及一个月内的文章             */

#include "bbs.h"
#include "ythtbbs.h"

void securityreport(char * owner, char * str, char * title);

char information[4096];

int
del_old_post(boardname, remain_num, remain_day)
char *boardname;
int remain_num;
int remain_day;
{
	int idx1, idx2;
	char dot_file[256];
	int fd;
	struct stat st;
	int numents;
	struct fileheader fileinfo;
	int size;

	time_t posttime, now;
	now = time(0);

	size = sizeof (struct fileheader);

	sprintf(dot_file, "%s/boards/%s/%s", MY_BBS_HOME, boardname, DOT_DIR);
	idx1 = idx2 = 0;
	if ((fd = open(dot_file, O_RDWR)) == -1)
		return -1;
	flock(fd, LOCK_EX);
	fstat(fd, &st);
	numents = ((long) st.st_size) / size;
	for (; idx2 < numents; idx2++) {
		if (read(fd, &fileinfo, size) == size) {
			posttime = fileinfo.filetime;
			if (numents - idx2 + idx1 <= remain_num)
				break;
//        printf("num=%d,idx2=%d,idx1=%d,remain=%d\n",numents,idx2,idx1,remain_num);
//        printf("now=%d,posttime=%d,remain_day=%d\n",now,posttime,remain_day);
			if (now - posttime < remain_day * 24 * 60 * 60)
				break;
/*judge will del*/
			if ((fileinfo.owner[0] == '-')	//keep delete will has it
			    || (!(fileinfo.accessed & FH_DIGEST)	// digested
				&& !(fileinfo.accessed & FH_MARKED)))	// marked
//delete it!
				unlink(fh2fname(&fileinfo));
			else {
//don't delete
				if (idx1 != idx2) {
//             if(lseek(fd,idx2*size,SEEK_SET) == -1) break ;
					if (lseek(fd, idx1 * size, SEEK_SET) ==
					    -1) break;
					if (safewrite(fd, &fileinfo, size) !=
					    size) break;
					if (lseek
					    (fd, (idx2 + 1) * size,
					     SEEK_SET) == -1) break;
				}
				idx1++;
			}
		} else
			break;
	}
	sprintf(information,
		"时间：%24.24s\n自动删文系统在%s板删除%d篇文章。\n剩余文章数：%d",
		ctime(&now), boardname, idx2 - idx1, numents - idx2 + idx1);
	if (idx2 != idx1)
		for (; idx2 < numents; idx2++, idx1++) {
			if (lseek(fd, idx2 * size, SEEK_SET) == -1)
				break;
			if (read(fd, &fileinfo, size) != size)
				break;
			if (lseek(fd, idx1 * size, SEEK_SET) == -1)
				break;
			if (safewrite(fd, &fileinfo, size) != size)
				break;
	} else
		idx1 = numents;
	ftruncate(fd, (off_t) size * idx1);
	flock(fd, LOCK_UN);
	close(fd);
	return 1;
}

int
main()
{
	FILE *fp;
	char buf[STRLEN];
	char *namep;
	char *num_str;
	char *day_str;

	if ((fp = fopen(MY_BBS_HOME "/etc/autoclear", "r")) == NULL)
		return 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		namep = (char *) strtok(buf, ": \n\r\t");
		num_str = strtok(NULL, ": \n\r\t");
		day_str = strtok(NULL, ": \n\r\t");
		printf("%s %s %s\n", namep, num_str, day_str);
		del_old_post(namep, atoi(num_str), atoi(day_str));
		securityreport("AutoClean", information, "自动删文系统");
	}
	fclose(fp);
	return 0;
}
