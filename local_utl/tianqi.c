//tianqi.c
//usage: lynx http://serve.cei.gov.cn/sl/sl01/l01index.htm -dump|/home/bbs/bin/tianqi
#include <stdio.h>
#include <string.h>
#include "../include/config.h"

main()
{
	char buf0[100], buf1[500] = "", buf2[500] =
	    "\033[1;44m[北京]", *ptr, *ptr2;
	int i = 0, len = 6;
	FILE *fp0, *fp1;

	while (fgets(buf0, sizeof (buf0), stdin) != NULL)
		if (strstr(buf0, "℃") != NULL)
			strcat(buf1, buf0);
	while ((ptr = strstr(buf1, "今天")) != NULL) {
		ptr[0] = ' ';
		ptr[1] = ' ';
		ptr[2] = ' ';
		ptr[3] = ' ';
	}
	ptr = strtok(buf1, " \r\n\t");
	while (ptr != NULL) {
		if ((ptr2 = strstr(ptr, "风")) == NULL) {
			sprintf(buf0, "\033[%sm%s", i ? "32" : "33", ptr);
			len += strlen(ptr);
		} else {
			sprintf(buf0, "\033[%sm%s", i ? "32" : "33", ptr2 + 2);
			len += strlen(ptr2 + 2);
		}
		i = !i;
		strcat(buf2, buf0);
		if (len >= 79)
			break;
		ptr = strtok(NULL, " \r\n\t");
	}
	buf2[strlen(buf2) - len + 79] = '\0';
	if (len < 79) {
		i = 79 - len;
		ptr = buf2 + strlen(buf2);
		for (i = 0; i < 79 - len; i++) {
			*(ptr + i) = ' ';
		}
		*(ptr + i) = '\0';
	}
	strcat(buf2, "\033[m");
	//printf("\n%s\n", buf2);
	fp0 = fopen(MY_BBS_HOME "/etc/endline", "r");
	if (fp0 == NULL)
		return;
	fp1 = fopen(MY_BBS_HOME "/etc/endline.new", "w");
	if (fp1 == NULL)
		return;
	i = 0;
	while (fgets(buf1, sizeof (buf1), fp0) != NULL) {
		if (strncmp
		    (buf1, "\033[1;44m[北京]", strlen("\033[1;44m[北京]")) == 0) {
			fprintf(fp1, "%s\n", buf2);
			i = 1;
		} else
			fprintf(fp1, "%s", buf1);
	}
	if (!i)
		fprintf(fp1, "%s\n", buf2);
	fclose(fp0);
	fclose(fp1);
	rename(MY_BBS_HOME "/etc/endline.new", MY_BBS_HOME "/etc/endline");
}
