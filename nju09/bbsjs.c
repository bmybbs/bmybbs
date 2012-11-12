#include "bbslib.h"

int
bbsjs_main()
{
	FILE *fp;
	static char s[300];
	html_header(1);
	printf("<font style='font-size:12px'>\n");
	printf
	    ("<center>欢迎访问[%s], 目前在线人数(www/all) [<font color=green>%d/%d</font>]",
	     MY_BBS_NAME, count_www(), count_online());
	printf("</font>");
}

int
count_www()
{
	int i, total = 0;
	for (i = 0; i < MAXACTIVE; i++)
		if (shm_utmp->uinfo[i].pid == 1)
			total++;
	return total;
}
