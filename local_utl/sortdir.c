#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "bbs.h"
#include "ythtlib.h"
#include "ythtbbs.h"
#include "errno.h"

struct fileheader *data;

int
cmpfile(f1, f2)
struct fileheader *f1, *f2;
{
	return f1->filetime - f2->filetime;
}

int
main(int argc, char *argv[])
{
	FILE *dr;
	int file, n;

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
	fread(data, sizeof (struct fileheader), n, dr);
	qsort(data, n, sizeof (struct fileheader), cmpfile);
	printf("end.len=%d %d", n, n * sizeof (struct fileheader));
	file = open("tmpfile", O_CREAT | O_TRUNC | O_WRONLY, 0770);
	if (file < 0) {
		printf("can't open file to write\n");
		exit(5);
	}
	if (write(file, data, n * sizeof (struct fileheader)) == 0)
		perror("write error");
	if (close(file))
		printf("close error=%d\n", errno);
}
