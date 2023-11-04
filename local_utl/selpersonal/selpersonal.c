#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "bbs.h"
#define MAXPERSON 1000
#define MAXSEL 5

int
getnum(long residue, int score[MAXPERSON], int count)
{
	int i;
	for (i = 0; i < count; i++) {
		residue -= score[i];
		if (residue <= 0)
			break;
	}
	return i;
}

char *
gettitle(char ch, char *author)
{
	static char retstr[100];
	char teststr[80], *ptr, buf[512];
	FILE *fp;
	snprintf(buf, sizeof buf,
		MY_BBS_HOME
		"/0Announce/groups/GROUP_0/Personal_Corpus/%c/.Names", ch);
	snprintf(retstr, sizeof retstr, "%sÎÄ¼¯", author);
	fp = fopen(buf, "r");
	if (!fp) {
		return retstr;
	}
	snprintf(teststr, sizeof teststr, "(BM: %s _Personal)", author);
	while (fgets(buf, 512, fp)) {
		if (strncmp(buf, "Name=", 5) || !strstr(buf, teststr))
			continue;
		ptr = strchr(buf, ' ');
		if (!ptr)
			break;
		*ptr = 0;
		strcpy(retstr, buf + 5);
		break;
	}
	fclose(fp);
	return retstr;
}

int main(int argc, char *argv[])
{
	char au[MAXPERSON][512], ch[MAXPERSON], buf[512], lastchar = 'A';
	int score[MAXPERSON];
	int count = 0, n1, n2, n3, n4;
	long totalscore = 0, residue = 0;
	int num[MAXSEL], i, j;
	while (fgets(buf, 512, stdin) != NULL) {
		if (isalpha(buf[0]))
			lastchar = toupper(buf[0]);
		if (5 !=
		    sscanf(buf, "%s%d%d%d%d", au[count], &n1, &n2, &n3,
			   &n4)) continue;
		sethomefile_s(buf, sizeof buf, au[count], "sucessreg");
		if (!file_exist(buf))
			continue;
		score[count] = n1 + n2 / 2 + n3 / 4 + n4 / 8;
		if (score[count] < 12)
			continue;
		score[count] = sqrt(score[count]);
		if (score[count] > 8)
			score[count] = 8;
		ch[count] = lastchar;
		totalscore += score[count];
		count++;
		if (count >= MAXPERSON)
			break;
	}
	//printf("%d %d\n", count, (int) totalscore);
	srandom(time(NULL));
	for (i = 0; i < 10; i++)
		residue = (residue + random()) % totalscore;
	n1 = 0;
	while (n1 < MAXSEL) {
		residue = (residue + random()) % totalscore;
		n2 = getnum(residue, score, count);
		if (n2 == count)
			continue;
		for (i = 0; i < n1; i++) {
			if (n2 == num[i])
				break;
		}
		if (i < n1)
			continue;
		num[n1] = n2;
		n1++;
	}
	for (i = 0; i < n1; i++)
		printf
		    ("<li><a href=bbs0an?path=/groups/GROUP_0/Personal_Corpus/%c/%s>%s</a><br>\n",
		     ch[num[i]], au[num[i]], gettitle(ch[num[i]], au[num[i]]));

	return 0;
}
