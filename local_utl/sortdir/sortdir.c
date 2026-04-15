#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "bbs.h"
#include "errno.h"

struct fileheader *data;

int
cmpfile(const void *p1, const void *p2)
{
	const struct fileheader *f1, *f2;

	f1 = p1;
	f2 = p2;
	return f1->filetime - f2->filetime;
}

int
main(int argc, char *argv[])
{
	FILE *dr;
	int file;
	size_t n;

	if (argc < 2) {
		printf("no input file!\n");
		exit(1);
	}
	n = file_size(argv[1]) / sizeof (struct fileheader);
	if (n == 0) {
		printf("no context!\n");
		exit(2);
	}
	data = malloc(n * sizeof (struct fileheader));
	if (NULL == data) {
		printf("out of memory\n");
		exit(3);
	}
	dr = fopen(argv[1], "r");
	if (NULL == dr) {
		printf("can't open file to read\n");
		exit(4);
	}
	if (fread(data, sizeof (struct fileheader), n, dr) != n) {
		if (ferror(dr)) {
			perror("read error");
		} else {
			printf("unexpected EOF\n");
		}
		fclose(dr);
		exit(4);
	}
	qsort(data, n, sizeof (struct fileheader), cmpfile);
	printf("end.len=%ld %ld", n, n * sizeof (struct fileheader));
	file = open("tmpfile", O_CREAT | O_TRUNC | O_WRONLY, 0770);
	if (file < 0) {
		printf("can't open file to write\n");
		exit(5);
	}
	if (write(file, data, n * sizeof (struct fileheader)) == 0)
		perror("write error");
	if (close(file))
		printf("close error=%d\n", errno);

	return 0;
}
