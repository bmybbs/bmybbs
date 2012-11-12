#include <stdio.h>
#define NUM 200
struct {
	char cginame[30];
	int count;
	double ut, st;
} sum[NUM];
main()
{
	char cginame[30];
	double ut, st, totalt;
	int i, count;
	bzero(sum, sizeof (sum));
	while (scanf("%s%d%lf%lf", cginame, &count, &ut, &st) == 4) {
		for (i = 0; i < NUM; i++) {
			if (!strcmp(cginame, sum[i].cginame)) {
				sum[i].count += count;
				sum[i].ut += ut;
				sum[i].st += st;
				i = NUM;
			}
			if (sum[i].cginame[0] == 0) {
				strcpy(sum[i].cginame, cginame);
				sum[i].count += count;
				sum[i].ut += ut;
				sum[i].st += st;
				i = NUM;
			}
		}
	}
	for (i = 0; i < NUM && sum[i].cginame[0]; i++)
		totalt += sum[i].ut + sum[i].st;
	for (i = 0; i < NUM && sum[i].cginame[0]; i++) {
		if (!sum[i].count)
			continue;
		printf("%12.12s\t%d\t%3.1f%%\t%f\t%e\t%e\t", sum[i].cginame,
		       sum[i].count,
		       (sum[i].ut + sum[i].st) * 100 / totalt,
		       sum[i].ut + sum[i].st, sum[i].ut, sum[i].st);
		printf("%d\t%e\t%e\n",
		       (int) (100 * (sum[i].ut + sum[i].st) * 1. /
			      sum[i].count), sum[i].ut * 1. / sum[i].count,
		       sum[i].st * 1. / sum[i].count);
	}
}
