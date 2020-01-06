//ylsdd 2002.10.30                                                                                    
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

void
printsection()
{
	printf("各章节介绍<br>");
	printf("<LI><A HREF='cgiman?1+intro'>USER COMMANDS</A>"
	       "<LI><A HREF='cgiman?2+intro'>SYSTEM CALLS</A>"
	       "<LI><A HREF='cgiman?3+intro'>C LIBRARY FUNCTIONS</A>"
	       "<LI><A HREF='cgiman?4+intro'>DEVICES AND NETWORK INTERFACES</A>"
	       "<LI><A HREF='cgiman?5+intro'>FILE FORMATS</A>"
	       "<LI><A HREF='cgiman?6+intro'>GAMES AND DEMOS</A>"
	       "<LI><A HREF='cgiman?7+intro'>ENVIRONMENTS, TABLES, AND TROFF MACROS</A>"
	       "<LI><A HREF='cgiman?8+intro'>MAINTENANCE COMMANDS</A>"
	       //"<LI><A HREF='cgiman?n+'>...</A>"
	       //"<LI><A HREF='cgiman?1+X'>X WINDOW SYSTEM</A>"
	    );
}

void
printform()
{
	int i;
	printf
	    ("<hr><form action=cgiman>要查询的关键词: <INPUT NAME=qry SIZE=24>"
	     " <INPUT TYPE=submit VALUE='查找'>"
	     "<P>选择章节: <SELECT NAME=sec>" "<OPTION VALUE='ANY'> 任意");
	for (i = 1; i < 9; i++)
		printf("<OPTION VALUE='%d'> %d", i, i);
	printf("<OPTION VALUE='n' > n <OPTION VALUE='x' > x  </SELECT></form>");
}

void
printheader()
{
	static int r = 0;
	if (r)
		return;
	printf("Content-type: text/html\n\n<HTML><HEAD>"
	       "<META HTTP-EQUIV='content-type' CONTENT='text/html; charset=GB2312'>"
	       "</HEAD><BODY bgcolor=#f0f4f0><br>一塌糊涂 BBS Linux 在线手册<hr>");
	r = 1;
}

int
getformarg(char *buf, char *sec, char *qry, int len)
{
	char *t2, *t3;
	qry[0] = 0;
	sec[0] = 0;
	t2 = strtok(buf, "&");
	while (t2) {
		t3 = strchr(t2, '=');
		if (t3 != 0) {
			t3[0] = 0;
			t3++;
			__unhcode(t3);
			if (!strcmp(t2, "sec")) {
				strsncpy(sec, t3, len);
			}
			if (!strcmp(t2, "qry")) {
				strsncpy(qry, t3, len);
			}
		}
		t2 = strtok(NULL, "&");
	}
	if (!qry[0])
		return -1;
	while (qry[0] == ' ')
		memmove(qry, qry + 1, strlen(qry));
	if ((t2 = strchr(qry, ' ')))
		*t2 = 0;
	return 0;
}

int
geturlarg(char *buf, char *sec, char *qry, int len)
{
	char *t2;
	qry[0] = 0;
	sec[0] = 0;
	t2 = strtok(buf, "+");
	if (!t2)
		return -1;
	__unhcode(t2);
	strsncpy(sec, t2, len);
	t2 = strtok(NULL, "+");
	if (!t2)
		return -1;
	__unhcode(t2);
	strsncpy(qry, t2, len);
	return 0;
}

int
teststr(char *str)
{
	char rejectstr[] = "\"'\\`~,:;!@#$%^&*|/? <>()[]{}\t\n\r";
	if (strpbrk(str, rejectstr))
		return -1;
	return 0;
}

int
testarg(char *sec, char *qry)
{
	if (teststr(sec) || teststr(qry))
		return -1;
	if (!strcasecmp(sec, "any"))
		sec[0] = 0;
	return 0;
}

int
getmanfile(char *sec, char *qry, char *manfile, int len)
{
	char cmd[1024], *ptr;
	FILE *p;
	if (sec[0])
		snprintf(cmd, sizeof (cmd), "man -w -c '%s' '%s'", sec, qry);
	else
		snprintf(cmd, sizeof (cmd), "man -w -c '%s'", qry);
	p = popen(cmd, "r");
	if (!p)
		return -1;
	if (!fgets(manfile, len, p)) {
		pclose(p);
		return -1;
	}
	pclose(p);
	ptr = strchr(manfile, '\r');
	if (ptr)
		*ptr = 0;
	ptr = strchr(manfile, '\n');
	if (ptr)
		*ptr = 0;
	if (access(manfile, F_OK))
		return -1;
	return 0;
}

int
runman2html(char *manfile)
{
	char cmd[2048];
	if (strstr(manfile, ".gz"))
		snprintf(cmd, sizeof (cmd),
			 "zcat '%s' | man2html | sed -es/'http:\\/\\/localhost\\/cgi-bin\\/man\\/man2html'/'cgiman'/",
			 manfile);
	else
		snprintf(cmd, sizeof (cmd),
			 "man2html %s | sed -es/'http:\\/\\/localhost\\/cgi-bin\\/man\\/man2html'/'cgiman'/",
			 manfile);
	system(cmd);
	return 0;
}

int
main(int argn, char **argv)
{
	char buf[1024], sec[100], qry[100], manfile[100];
	int retv;
	strsncpy(buf, getsenv("QUERY_STRING"), sizeof (buf));
	if (strchr(buf, '&'))
		retv = getformarg(buf, sec, qry, 100);
	else if (strchr(buf, '+'))
		retv = geturlarg(buf, sec, qry, 100);
	else
		retv = -1;
	if (!retv)
		retv = testarg(sec, qry);
	if (!retv) {
		retv = getmanfile(sec, qry, manfile, sizeof (manfile));
		if (retv) {
			printheader();
			printf
			    ("找不到与<font color=red>%s</font>相关的文档<br><br>\n",
			     nohtml(qry));
		}
	}
	if (retv) {
		printheader();
		printsection();
		printform();
		return 0;
	}
	runman2html(manfile);
	return 0;
}
