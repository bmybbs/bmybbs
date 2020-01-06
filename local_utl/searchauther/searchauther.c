#include "bbs.h"
struct {
	char author[21];
	int score;
	int n;
} alist[500];
int num = 0;

int
addscore(char *author, int sum, int n)
{
	int i;
	for (i = 0; i < num; i++) {
		if (strcmp(alist[i].author, author))
			continue;
		alist[i].score += sum;
		alist[i].n += n;
		return;
	}
	if (num < 500) {
		strcpy(alist[num].author, author);
		alist[num].score = sum;
		alist[num].n = n;
		num++;
	}
}

int
printscore()
{
	int i;
	for (i = 0; i < num; i++) {
		printf("%10d %10d %s\n", alist[i].score, alist[i].n,
		       alist[i].author);
	}
}

char *
searchauthor(char *path)
{
	static char author[100];
	int fd, retv;
	char buf[100];
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return NULL;
	retv = read(fd, buf, 99);
	close(fd);
	if (retv < 30)
		return NULL;
	buf[retv] = 0;
	if (strncmp(buf, "发信人: ", 8))
		return NULL;
	if (1 != sscanf(buf, "发信人: %s", author))
		return NULL;
	author[20] = 0;
	if (strchr(author, '.') || strchr(author, '@'))
		return NULL;
	return author;
}

main(int argn, char **argv)
{
	int sum, n;
	float avg;
	char board[100], fname[100], buf[100], path[300], *ptr;
	while (NULL != fgets(buf, sizeof (buf), stdin)) {
		if (sscanf(buf, "%d%f%d%s%s", &sum, &avg, &n, board, fname) !=
		    5) continue;
		sprintf(path, MY_BBS_HOME "/boards/%s/%s", board, fname);
		ptr = searchauthor(path);
		if (ptr == NULL)
			continue;
		addscore(ptr, sum, n);
	}
	printscore();
}
