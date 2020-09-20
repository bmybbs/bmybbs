#include <string.h>
#include <stdio.h>
#include "ytht/strop.h"

char *
encode_url(unsigned char *s)
{
	int i, j, half = 0;
	static char buf[1024];
	char a[4];
	j = 0;
	for (i = 0; s[i]; i++) {
		if ((!half
		     && strchr("~`!@#$%%^&*()-_=+[{]}\\|;:'\",<.>/? ", s[i]))
		    || (s[i + 1] == 0 && !half && (unsigned char) s[i] >= 128)) {
			buf[j++] = '%';
			sprintf(a, "%02X", s[i]);
			buf[j++] = a[0];
			buf[j++] = a[1];
		} else
			buf[j++] = s[i];
		if (half)
			half = 0;
		else if ((unsigned char) s[i] >= 128)
			half = 1;
	}
	buf[j] = 0;
	return buf;
}

char *
nohtml(char *s)
{
	static char buf[1024];
	int i = 0;

	while (*s && i < 1000) {
		if (*s == '<') {
			strcpy(buf + i, "&lt;");
			i += 4;
		} else if (*s == '>') {
			strcpy(buf + i, "&gt;");
			i += 4;
		} else if (*s == ' ' && i && buf[i - 1] == ' ') {
			strcpy(buf + i, "&nbsp;");
			i += 6;
		} else {
			buf[i] = *s;
			i++;
		}
		s++;
	}
	buf[i] = 0;
	return buf;
}

char *
void1(char *s)
{
	int i;
	int flag = 0;
	for (i = 0; s[i]; i++) {
		if (flag == 0) {
			if ((unsigned char)s[i] >= 128)
				flag = 1;
			continue;
		}
		flag = 0;
		if ((unsigned char)s[i] < 32)
			s[i - 1] = 32;
	}
	if (flag)
		s[strlen(s) - 1] = 0;
	return s;
}

char *
userid_str(char *s)
{
	static char buf[512];
	char buf2[256], tmp[256], *ptr, *ptr2;
	strsncpy(tmp, s, 255);
	buf[0] = 0;
	ptr = strtok(tmp, " ,();\r\n\t");
	while (ptr && strlen(buf) < 400) {
		if ((ptr2 = strchr(ptr, '.'))) {
			ptr2[1] = 0;
			strcat(buf, ptr);
		} else {
			ptr = nohtml(ptr);
			sprintf(buf2, "<a href=qry?U=%s>%s</a>", ptr, ptr);
			strcat(buf, buf2);
		}
		ptr = strtok(0, " ,();\r\n\t");
		if (ptr)
			strcat(buf, " ");
	}
	return buf;
}
