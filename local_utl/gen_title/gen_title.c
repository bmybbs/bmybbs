#include "bbs.h"
#include <sys/types.h>
#include <sys/mman.h>

static int
simple_digest(char *str, int maxlen)
{
	char x[sizeof (int)];
	char *p;
	bzero(x, sizeof (int));
	for (p = str; *p && ((p - str) < maxlen); p++)
		x[(p - str) % sizeof (int)] += *p;
	return (*(int *) &x[0]);
}

int
generate_board_title_fh(char *DOTDIR)
{
	int size = sizeof (struct fileheader), total, i;
	char *t, *t2;
	struct fileheader *ptr1;
	int fd;
	struct search_temp {
		int has_pre;
		int digest;
		int thread_id;
		int id;
	} *index;
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
	index = (struct search_temp *) malloc(sizeof (*index) * total);
	ptr1 = (struct fileheader *) (ptr);
	for (i = 0; i < total; i++, ptr1++) {
		int j;

		t2 = ptr1->title;
		if (!strncmp(t2, "Re:", 3)) {
			index[i].has_pre = 1;
			t2 += 4;
		} else
			index[i].has_pre = 0;
		index[i].thread_id = 0;
		index[i].digest = simple_digest(t2, STRLEN);
		for (j = i - 1; j >= 0; j--) {
			struct fileheader *tmppost;

			if (index[j].digest != index[i].digest)
				continue;
			tmppost = ((struct fileheader *) (ptr + j * size));
			t = tmppost->title;
			if (index[j].has_pre)
				t += 4;
			if (!strncmp(t, t2, 50)) {
				index[i].thread_id = index[j].thread_id;
				break;
			}
		}
		if (index[i].thread_id == 0) {
			index[i].thread_id = ptr1->filetime;
			index[i].id = ptr1->filetime;
		} else {
			index[i].id = ptr1->filetime;
		}
	}
	ptr1 = (struct fileheader *) (ptr);
	for (i = 0; i < total; i++, ptr1++) {
		if (ptr1->thread == 0)
			ptr1->thread = index[i].thread_id;
	}
	}
	MMAP_CATCH {
		close(fd);
	}
	MMAP_END munmap(ptr, buf.st_size);
	return 0;
}

int
generate_board_title(struct boardheader *bh)
{
	char DOTDIR[80];
	sprintf(DOTDIR, "boards/%s/.DIR", bh->filename);
	return generate_board_title_fh(DOTDIR);
}
int
generate_all_title()
{
	//apply_boards(generate_board_title);
	return new_apply_record(".BOARDS", sizeof (struct boardheader),
				generate_board_title, NULL);
}

int
generate_bknum()
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
                        generate_board_title_fh(buf);
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
			    ("%s [-a|boardname]\n  generatate board thread index\n",
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
		generate_board_title(&bh);
	}
	if (allflag) {
		generate_all_title();
	}
	if (bknum) {
       		generate_bknum(); 
	}
	return 0;
}
