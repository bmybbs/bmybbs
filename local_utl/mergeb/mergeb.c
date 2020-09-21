#include <time.h>
#include <stdio.h>
#include "bbs.h"
#define MAXINT ((int)((((unsigned int)~(int)0))>>1))

long get_num_records(char *filename, int size);

int main(int argc, char *argv[])
{
	char dir1[256], dir2[256], tmpfile[256];
	struct fileheader fh1, fh2;
	int ssize;
	int id1, id2;
	int num1, num2;
	int time1, time2;
	ssize = sizeof (struct fileheader);

	if (argc < 3) {
		printf("Usage:%s board1 board2\n", argv[0]);
		exit(0);
	}
	strcpy(tmpfile, ".TMPDIR");
	strncpy(dir1, argv[1], 255);
	strcat(dir1, "/.DIR");
	dir1[255] = 0;
	strncpy(dir2, argv[2], 255);
	strcat(dir2, "/.DIR");
	dir2[255] = 0;

	id1 = 1;
	id2 = 1;
	printf("get num\n");
	num1 = get_num_records(dir1, sizeof (fh1));
	num2 = get_num_records(dir2, sizeof (fh2));
	printf("num1=%d,num2=%d\n", num1, num2);
	if (get_record(dir1, &fh1, ssize, id1) == -1)
		time1 = MAXINT;
	else
		time1 = fh1.filetime;

	if (get_record(dir2, &fh2, ssize, id2) == -1)
		time2 = MAXINT;
	else
		time2 = fh2.filetime;
	while ((id1 <= num1) || (id2 <= num2)) {
		printf("b\n");
		if ((time1 < time2) || (id2 > num2)) {
			append_record(tmpfile, &fh1, ssize);
			if (get_record(dir1, &fh1, ssize, id1) == -1) {
				time1 = MAXINT;
			} else
				time1 = fh1.filetime;
			id1++;
		} else {
			if (append_record(tmpfile, &fh2, ssize))
				return 0;
			if (get_record(dir2, &fh2, ssize, id2) == -1) {
				time2 = MAXINT;
			} else
				time2 = fh2.filetime;
			id2++;
		}
		printf("%d-%s,", id1, fh2fname(&fh1));
		printf("%d-%s\n", id2, fh2fname(&fh2));
	}
}

long
get_num_records(char *filename, int size)
{
	struct stat st;

	if (stat(filename, &st) == -1)
		return 0;
	return (st.st_size / size);
}
