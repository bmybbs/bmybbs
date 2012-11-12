#include "ythtlib.h"

int
eff_size(char *file)
{
	FILE *fp;
	char buf[1000];
	int i, size, size2 = 0;
	size = file_size(file);
	if (size > 3000 || size == 0)
		goto E;
	size = 0;
	fp = fopen(file, "r");
	if (fp == 0)
		return 0;
	for (i = 0; i < 10; i++) {
		char *ptr;
		if (fgets(buf, sizeof (buf), fp) == 0)
			break;
		if ((ptr = strchr(buf, '\r')))
			*ptr = 0;
		if ((ptr = strchr(buf, '\n')))
			*ptr = 0;
		if (!strlen(strtrim(buf)))
			break;
	}
	while (1) {
		if (fgets(buf, sizeof (buf), fp) == 0)
			break;
		if (!strcmp(buf, "--\n"))
			break;
		if (!strncmp(buf, ": ", 2))
			continue;
		if (!strncmp(buf, "¡¾ ÔÚ ", 4))
			continue;
		if (strstr(buf, "¡ù À´Ô´:£®"))
			continue;
		for (i = 0; buf[i]; i++)
			if (buf[i] < 0)
				size2++;
		size += strlen(strtrim(buf));
	}
	fclose(fp);
      E:
	size = size - size2 / 2;
	if (size == 0)
		size = 1;
	return size;
}
