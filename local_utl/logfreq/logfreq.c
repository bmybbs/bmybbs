#include <stdio.h>
#include <string.h>

int n[3600];

int
getlogtime(char *line)
{
	int s, m;
	char *ptr;
	//ptr=strstr(line,"10/May/2002:21");
	ptr = strstr(line, "2002:");
	if (!ptr)
		return -1;
	ptr += 8;
	if (sscanf(ptr, "%d:%d", &m, &s) != 2)
		return -1;
	return m * 60 + s;
}

main()
{
	char line[1024];
	int t, count = 0;
	while (fgets(line, 1024, stdin) != NULL) {
		t = getlogtime(line);
		if (t < 0 || t >= 3600) {
			continue;
		}
		n[t]++;
		count++;
	}
	for (t = 0; t < 3600; t += 4)
		printf("%d %d\n", t, n[t] + n[t + 1] + n[t + 2] + n[t + 3]);
}
