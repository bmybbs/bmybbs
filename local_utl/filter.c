#include "stdio.h"
#include "string.h"
#define MAX 100
int
main(int argn, char **argv)
{
	char array[MAX][60], line[1024], *p;
	int i, n;
	FILE *fp;
	if (argn < 2)
		return -1;
	fp = fopen(argv[1], "r");
	if (fp == NULL)
		return -1;
	for (i = 0; i < MAX; i++) {
		if (fgets(array[i], 60, fp) == NULL)
			break;
		if ((p = strchr(array[i], '\n')) != NULL)
			*p = 0;
		if ((p = strchr(array[i], '\r')) != NULL)
			*p = 0;
		while (array[i][0] == ' ')
			memmove(&array[i][0], &array[i][1], strlen(array[i]));
		while (array[i][0] != 0
		       && array[i][strlen(array[i]) - 1] ==
		       ' ') array[i][strlen(array[i]) - 1] = 0;
		if (!array[i][0])
			i--;
	}
	n = i;
	while (fgets(line, 1024, stdin) != NULL) {
		for (i = 0; i < n; i++) {
			if (strstr(line, array[i]))
				break;
		}
		if (i == n || n == 0)
			fputs(line, stdout);
	}
	fclose(fp);
	return 0;
}
