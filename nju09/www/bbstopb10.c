#include "bbslib.h"

int
bbstopb10_main()
{
	FILE *fp;
	char buf[256], tmp[256], name[256], cname[256], cc[256];
	int i, r;
	html_header(1);
	check_msg();
	fp = fopen("0Announce/bbslist/board2", "r");
	if (fp == 0)
		http_fatal("error 1");
	printf("<body><center>%s -- 热门讨论区<hr>", BBSNAME);
	printf("<table border=1>\n");
	printf("<tr><td>名次<td>版名<td>中文版名<td>人气\n");
	for (i = 0; i <= 15; i++) {
		if (fgets(buf, 150, fp) == 0)
			break;
		if (i == 0)
			continue;
		r = sscanf(buf, "%s %s %s %s %s", tmp, tmp, name, cname, cc);
		if (r == 5) {
			printf
			    ("<tr><td>%d<td><a href=%s%s>%s</a><td width=200><a href=%s%s>%s</a><td>%s\n",
			     i, showByDefMode(), name, name, showByDefMode(), name, cname, cc);
		}
	}
	printf("</table>\n</center></body>\n");
	fclose(fp);
	return 0;
}
