#include "ythtlib.h"
#include <string.h>

void
strsncpy(char *s1, const char *s2, int n)
{
	int l = strlen(s2);
	if (n < 0)
		return;
	if (n > l + 1)
		n = l + 1;
	strncpy(s1, s2, n - 1);
	s1[n - 1] = 0;
}

char *
strltrim(char *s)
{
	char *s2 = s;
	if (s[0] == 0)
		return s;
	while (s2[0] && strchr(" \t\r\n", s2[0]))
		s2++;
	return s2;
}

char *
strrtrim(char *s)
{
	static char t[1024], *t2;
	if (s[0] == 0)
		return s;
	strsncpy(t, s, 1024);
	t2 = t + strlen(s) - 1;
	while (strchr(" \t\r\n", t2[0]) && t2 > t)
		t2--;
	t2[1] = 0;
	return t;
}

void
normalize(char *buf)
{
	int i = 0;
	while (buf[i]) {
		if (buf[i] == '/')
			buf[i] = ':';
		i++;
	}
}
