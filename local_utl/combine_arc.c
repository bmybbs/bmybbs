#include "bbs.h"

void
add_content(char *board, struct fileheader *fh)
{
	char fn[256];
	FILE *fp;
	snprintf(fn, sizeof (fn), MY_BBS_HOME "/boards/%s/%s", board,
		 fh2fname(fh));
	fp = fopen(fn, "r");
	if (NULL == fp)
		return;
	keepoldheader(fp, SKIPHEADER);
	printf("\nÊ±¼ä: %s", ctime((time_t *) & (fh->filetime)));
	while (fgets(fn, sizeof (fn), fp)) {
		if (!strcmp(fn, "--\n") || !strcmp(fn, "\n"))
			break;
		fputs(fn, stdout);
	}
	fclose(fp);
}

int
main(int argc, char *argv[])
{
	struct mmapfile mf = { ptr:NULL };
	int i = 0, fc, day, start;
	char buf[256];
	struct fileheader *fh;
	if (argc < 4) {
		printf("not enough argments!\n");
		exit(1);
	}
	if (strchr(argv[1], '/')) {
		printf("invalid board name!\n");
		exit(2);
	}
	snprintf(buf, sizeof (buf), MY_BBS_HOME "/boards/%s/.DIR", argv[1]);
	MMAP_TRY {
	if (mmapfile(buf, &mf)) {
		printf("can not mmap .DIR file!\n");
		MMAP_RETURN(3);
	}
	fc = mf.size / sizeof (struct fileheader);
	day = atoi(argv[3]);
	if (!day)
		day = 1;
	start = time(NULL) - day * 86400;
	fh = (struct fileheader *) (mf.ptr) + fc - 1;
	for (fh = (struct fileheader *) (mf.ptr) + fc - 1;
	     fh >= (struct fileheader *) mf.ptr; fh--) {
		if (fh->filetime >= start) {
			if (!strncmp(fh->title, argv[2], strlen(argv[2])))
				add_content(argv[1], fh);
		} else
			i++;
		if (i > 10)
			break;
	}
	}
	MMAP_CATCH {
		printf("mmap .DIR changed! \n");
		mmapfile(NULL, &mf);
		MMAP_RETURN(3);
	}
	MMAP_END mmapfile(NULL, &mf);
	return 0;
}
