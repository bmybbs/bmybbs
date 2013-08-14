#include "ythtbbs.h"

static int
whatch(unsigned char ch)
{
	if (isalpha(ch))
		return 1;
	if (ch > 128 + 32 && ch <= 128 + 32 + 94)
		return 2;
	return 0;
}

static int
goodq(unsigned char ch)
{
	int q = ch - 128 - 32;
	if (q >= 16 && q <= 87)
		return 1;
	return 0;
}

int
goodgbid(char *userid)		//by ylsdd
{
	int q = 0;
	char *s;
	for (s = userid; *s != '\0'; s++) {
		switch (whatch(*s)) {
		case 0:
			return 0;
		case 1:
			if (q)
				return 0;
			break;
		case 2:
			if (!q && !goodq(*s))
				return 0;
			q = !q;
			break;
		}
	}
	if (q)
		return 0;
	return 1;
}

int
is_bad_id(char *s)
{
	FILE *fp;
	char buf[80], buf2[80];

	fp = fopen(MY_BBS_HOME "/etc/badname0", "r");
	if (fp) {
		while (fgets(buf, sizeof (buf), fp) != NULL) {
			if (sscanf(buf, "%s", buf2) != 1)
				continue;
			if (!strcasecmp(s, buf2)) {
				fclose(fp);
				return 1;
			}
		}
		fclose(fp);
	}

	fp = fopen(MY_BBS_HOME "/etc/badname", "r");
	if (fp == 0)
		return 0;
	while (fgets(buf, sizeof (buf), fp)) {
		if (sscanf(buf, "%s", buf2) != 1)
			continue;
		if (strcasestr(s, buf2)) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}
