#include <stdio.h>
#include <time.h>
#include "bbs.h"

struct boardheader board1;
struct fileheader *post1;

int
main(int argc, char *argv[])
{
	FILE *fpb, *fpf;
	char dirfile[100];
	time_t fbtime, nowtime, starttime;
	int userlevel, day, count = 0;
	struct mmapfile mf = { ptr:NULL };
	int start, total, i;
	if (argc != 4) {
		printf("Argument error!\n");
		return -1;
	}
	userlevel = atoi(argv[1]);
	day = atoi(argv[2]);
	nowtime = time(NULL);
	if (day == 0)
		starttime = 0;
	else
		starttime = nowtime - 86400 * day;
	if (starttime < 0)
		starttime = 0;
	if (day)
		printf("%20s 最近 %d 天的发文情况.\n\n", argv[3], day);
	else
		printf("%20s 的发文情况.\n\n", argv[3]);
	fpb = fopen(MY_BBS_HOME "/.BOARDS", "r");
	while (fread(&board1, sizeof (board1), 1, fpb)) {
		if (!(board1.level & PERM_POSTMASK)
		    && !(board1.level ? userlevel & board1.level : 1))
			continue;
		if (board1.clubnum != 0)
			continue;
		sprintf(dirfile, MY_BBS_HOME "/boards/%s/.DIR",
			board1.filename);
		MMAP_TRY {
			if (mmapfile(dirfile, &mf) < 0) {
				MMAP_UNTRY;
				continue;
			}
			total = mf.size / sizeof (struct fileheader);
			if (total == 0) {
				mmapfile(NULL, &mf);
				mf.ptr = NULL;
				MMAP_UNTRY;
				continue;
			}
			start = Search_Bin(mf.ptr, starttime, 0, total - 1);
			if (start < 0) {
				start = - (start + 1);
			}
			for (i = start; i < total && count <= 1000; i++) {
				post1 =
				    (struct fileheader *) (mf.ptr +
							   i *
							   sizeof (struct
								   fileheader));
				fbtime = post1->filetime;
				if (!strcmp(post1->owner, argv[3])) {
					count++;
					printf("%20s %4d %s \n",
					       board1.filename, count,
					       post1->title);
				}
			}
		}
		MMAP_CATCH {
		}
		MMAP_END {
			mmapfile(NULL, &mf);
			mf.ptr = NULL;
		}
		if (count > 1000) {
			printf("好多啊... 下面的不查了\n");
			break;
		}
	}
	fclose(fpb);
	printf("一共找到 %d 篇\n", count);
	return 0;
}
