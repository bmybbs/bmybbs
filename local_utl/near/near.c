#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bbs.h"

int
boardcount(char *bn)
{
	static char bname[MAXBOARD][20];
	static int count[MAXBOARD];
	static int nboard = 0;
	int i;
	for (i = 0; i < nboard; i++) {
		if (!strcmp(bname[i], bn)) {
			return count[i]++;
		}
	}
	if (nboard >= MAXBOARD)
		return 0;
	strncpy(bname[nboard], bn, 19);
	bname[nboard][19] = 0;
	count[nboard] = 1;
	nboard++;
	return 0;
}

main(int argn, char **argv)
{
	char buf[1024], *ptr, bn[30], s[30];
	int i, n = time(NULL), ntr = 0, maxntr = 4, count = 0, maxcount =
	    100, maxoneboard = 5;
	float d = 2;

	if (argn >= 2)
		d = atof(argv[1]);
	if (d <= 0)
		d = 1;

	if (argn >= 3)
		maxcount = atoi(argv[2]);

	if (argn >= 4)
		maxntr = atoi(argv[3]);

	if (argn >= 5)
		maxoneboard = atoi(argv[4]);

	gets(buf);
	puts(buf);
	while (gets(buf) != NULL) {
		ptr = strrchr(buf, 'M');
		if (ptr == NULL)
			ptr = strrchr(buf, 'G');
		if (ptr == NULL) {
			//printf("NULL! %s\n", buf);
			continue;
		}
		ptr += 2;
		i = atoi(ptr);
		if (n - i > d * 24 * 3600)
			continue;
		sscanf(buf, "%s%s%s%s", s, s, s, bn);
		if (!strcmp(bn, "triangle")) {
			if (n - i > 2 * 24 * 3600)
				continue;
			ntr++;
			if (ntr > maxntr)
				continue;
		} else if (boardcount(bn) >= maxoneboard) {
			continue;
		}
		puts(buf);
		count++;
		if (count > maxcount)
			break;
	}
}
