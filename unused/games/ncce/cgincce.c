//ecnegrevid 2001.7.20
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/param.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <newt.h>
#include <stdlib.h>
#include "ncce.h"
#define CHARSET "gb2312"
char *
getsenv(char *s)
{
	char *t = getenv(s);
	if (t)
		return t;
	return "";
}

int
__to16(char c)
{
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= '0' && c <= '9')
		return c - '0';
	return 0;
}

void
__unhcode(char *s)
{
	int m, n;
	for (m = 0, n = 0; s[m]; m++, n++) {
		if (s[m] == '+') {
			s[n] = ' ';
			continue;
		}
		if (s[m] == '%') {
			s[n] = __to16(s[m + 1]) * 16 + __to16(s[m + 2]);
			m += 2;
			continue;
		}
		s[n] = s[m];
	}
	s[n] = 0;
}

void
strsncpy(char *s1, char *s2, int n)
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
nohtml(char *s)
{
	static char buf[5000];
	int i = 0;

	while (*s && i < sizeof (buf) - 10) {
		if (*s == '<') {
			strcpy(buf + i, "&lt;");
			i += 4;
		} else if (*s == '>') {
			strcpy(buf + i, "&gt;");
			i += 4;
		} else {
			buf[i] = *s;
			i++;
		}
		s++;
	}
	buf[i] = 0;
	return buf;
}

main(int argn, char **argv)
{
	char w[30], *list, buf[1024], *t2, *t3;
	strsncpy(buf, getsenv("QUERY_STRING"), sizeof (buf));
	w[0] = 0;
	t2 = strtok(buf, "&");
	while (t2) {
		t3 = strchr(t2, '=');
		if (t3 != 0) {
			t3[0] = 0;
			t3++;
			__unhcode(t3);
			if (!strcmp(t2, "w")) {
				strsncpy(w, t3, sizeof (w));
				break;
			}
		}
		t2 = strtok(0, "&");
	}
	printf("Content-type: text/html; charset=%s\n\n\n", CHARSET);
	printf("<HTML><head>");
	printf
	    ("<meta http-equiv='Content-Type' content='text/html; charset=%s'>\n",
	     CHARSET);
	printf("<script>function sf(){document.l.w.focus();}</script>");
	printf("</head><body><form action=cgincce method=get name=l>"
	       "<label for='w'>¿Æ¼¼´Êµä: </label>"
	       "<input name=w type=input value=%s> "
	       "<input type=submit value=Search></form>"
	       "<script>sf();</script>", nohtml(w));
	if (w[0]) {
		list = search_dict(w);
		printf("<pre><hr>%s</pre>", nohtml(list));
	}
	printf("</body></html>");
}
