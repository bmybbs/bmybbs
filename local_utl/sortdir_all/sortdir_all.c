#include "bbs.h"
#include <sys/types.h>
#include <sys/mman.h>

int
cmpfh(struct fileheader *a, struct fileheader *b)
{
	return (a->filetime - b->filetime);
}

int
_sort_dir(char *DOTDIR)
{
	int size = sizeof (struct fileheader), total, i;
	char *t, *t2;
	struct fileheader *ptr1;
	int fd;
	struct stat buf;
	char *ptr;
	printf("thread %s\n", DOTDIR);
	if ((fd = open(DOTDIR, O_RDWR))== -1) {
		printf("faint, %s\n", DOTDIR);
		return -1;
	}
	fstat(fd, &buf);
	MMAP_TRY {
	ptr = mmap(0, buf.st_size, PROT_READ| PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	total = buf.st_size / size;
	qsort(ptr, total, size, cmpfh);
	}
	MMAP_CATCH {
	}
	MMAP_END munmap(ptr, buf.st_size);
	return 0;
}

int
sortdir(struct boardheader *bh)
{
	char DOTDIR[80];
	sprintf(DOTDIR, "boards/%s/.DIR", bh->filename);
	return _sort_dir(DOTDIR);
}
int
sort_all_dir()
{
	//apply_boards(generate_board_title);
	return new_apply_record(".BOARDS", sizeof (struct boardheader),
				sortdir, NULL);
}

int
sortdir_bknum()
{
        char buf[1024], buf2[1024], *ptr;
        FILE *fp;
        fp = fopen("filelist.txt", "rt");
        if (!fp)
                return 1;
        while (fgets(buf, sizeof (buf), fp)) {
                if ((ptr = strchr(buf, '\n')))
                        *ptr = 0;
                if (!file_exist(buf)) {
                        printf("error f_exist %s", buf);
                        return 1;
                }
                strcpy(buf2, buf);
                strcat(buf2, ".org");
                if (strstr(buf, "backnumbers/") && strstr(buf, "/B.")) {
                        _sort_dir(buf);
                }
                fflush(stdout);
        }
        return 0;
}

int
main(int argc, char **argv)
{
	int allflag = 0;
	int bknum = 0;
	struct boardheader bh;
	char *name;
	while (1) {
		int c;
		c = getopt(argc, argv, "abh");
		if (c == -1)
			break;
		switch (c) {
		case 'a':
			allflag = 1;
			break;
		case 'h':
			printf
			    ("%s [-a|boardname]\n  sort board .DIR by filetime\n",
			     argv[0]);
			return 0;
		case 'b':
			bknum = 1;
			break;
		case '?':
			printf
			    ("%s:Unknown argument.\nTry `%s -h' for more information.\n",
			     argv[0], argv[0]);
			return 0;
		}
	}
	chdir(MY_BBS_HOME);
	if (optind < argc) {
		name = argv[optind++];
		if (optind < argc) {
			printf
			    ("%s:Too many arguments.\nTry `%s -h' for more information.\n",
			     argv[0], argv[0]);
			return 0;
		}
		strncpy(bh.filename, name, STRLEN);
		sortdir(&bh);
	}
	if (allflag) {
		sort_all_dir();
	}
	if (bknum) {
       		sortdir_bknum(); 
	}
	return 0;
}
