#include "bbslib.h"

int denynum = 0;
struct deny denyuser[256];

void
loaddenyuser(char *board)
{
	FILE *fp;
	char path[80], buf[256], *ptr;
	denynum = 0;
	sprintf(path, "boards/%s/deny_users", board);
	fp = fopen(path, "r");
	if (fp == 0)
		return;
	while (denynum < 100) {
		if (fgets(buf, 200, fp) == 0)
			break;
		if (2 > sscanf(buf, "%s %s", denyuser[denynum].id, denyuser[denynum].exp))
			continue;
		ptr = strrchr(buf, '[');
		if (ptr == NULL)
			continue;	//http_fatal("无法辨识的数据,请通知系统维护");
		sscanf(ptr + 1, "%d", &denyuser[denynum].free_time);
		denynum++;
	}
	fclose(fp);
}

void
savedenyuser(char *board)
{
	FILE *fp;
	int i;
	char path[80], *exp;
	sprintf(path, "boards/%s/deny_users", board);
	fp = fopen(path, "w");
	if (fp == 0)
		return;
	for (i = 0; i < denynum; i++) {
		int m;
		struct tm *tmtime;
		time_t daytime = denyuser[i].free_time + 24 * 60 * 60;
		tmtime = gmtime(&daytime);
		exp = denyuser[i].exp;
		if (denyuser[i].id[0] == 0)
			continue;
		for (m = 0; exp[m]; m++) {
			if (exp[m] <= 32 && exp[m] > 0)
				exp[m] = '.';
		}
		fprintf(fp, "%-12s %-40s %2d月%2d日解 \x1b[%um\n",
			denyuser[i].id, denyuser[i].exp,
			tmtime->tm_mon + 1, tmtime->tm_mday,
			denyuser[i].free_time);
	}
	fclose(fp);
}
