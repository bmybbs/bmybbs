#include "bbs.h"
#include "displayip.h"
#include "stdlib.h"
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

int
getformarg(char *buf, char *type, char *ip, int len)
{
	char *t2, *t3;
	type[0] = 0;
	ip[0] = 0;
	t2 = strtok(buf, "&");
	while (t2) {
		t3 = strchr(t2, '=');
		if (t3 != 0) {
			t3[0] = 0;
			t3++;
			__unhcode(t3);
			if (!strcmp(t2, "type")) {
				strsncpy(type, t3, len);
			}
			if (!strcmp(t2, "ip")) {
				strsncpy(ip, t3, len);
			}
		}
		t2 = strtok(NULL, "&");
	}
	if (!ip[0])
		return -1;
	while (ip[0] == ' ')
		memmove(ip, ip + 1, strlen(ip));
	if ((t2 = strchr(ip, ' ')))
		*t2 = 0;
	return 0;
}

void
printheader()
{
	static int r = 0;
	if (r)
		return;
	printf("Content-type: text/html\n\n<HTML><HEAD>"
	       "<META HTTP-EQUIV='content-type' CONTENT='text/html; charset=GB2312'>"
	       "</HEAD><BODY bgcolor=#f0f4f0><br>%s IP地址查询<hr>",
	       MY_BBS_NAME);
	r = 1;
}

static void
get_load(load)
double load[];
{
        FILE *fp;
        fp = fopen("/proc/loadavg", "r");
        if (!fp)
                load[0] = load[1] = load[2] = 0;
        else {
                float av[3];
                fscanf(fp, "%g %g %g", av, av + 1, av + 2);
                fclose(fp);
                load[0] = av[0];
                load[1] = av[1];
                load[2] = av[2];
        }
}

int
main()
{
	int type;
	char ip[80] = "", typestr[80];

	char buf[1024];
	double load[3];
	printheader();
	get_load(load);
	if (load[0] > 2.5) {
		printf("系统负载过高，请稍后再试\n");
		return 0;
	}
	strsncpy(buf, getsenv("QUERY_STRING"), sizeof (buf));
	type = 0;
	if (getformarg(buf, typestr, ip, 80) == 0) {
		type = atoi(typestr);
		ip[50] = 0;
	}
	if (type) {
		printf("<pre>\n");
		display_ip(ip);
		printf("</pre>\n");
	}
	printf("<form action='cgifreeip'>\n");
	printf("<input type=hidden name=type value=1>");
	printf
	    ("域名或IP地址: <input type=text name=ip maxlength=50 value='%s'>\n",
	     nohtml(ip));
	printf("<input type=submit>");
	printf("</form>\n");
	printf("</boyd></html>");
}

