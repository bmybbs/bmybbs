#include "bbslib.h"
int
bbsshownav_main()
{
	char *secstr;
	secstr=getparm("secstr");
	html_header(1);
	changemode(SELECT);
	printf("<style type=text/css>A {color: #0000f0}</style>");
	printf("<body bgcolor=#FFFFFF>");
	printf("<center><div class=rhead>\n");
	printf("%s--<span class=h11>近日精彩话题</span></div><hr>", BBSNAME);
	if (shownavpart(1, secstr))
		printf("错误的参数!");
	printf("<hr>");
	printf("</body>");
	http_quit();
	return 0;
}
int
shownavpart(int mode, const char *secstr)
{
	char buf[2048]="wwwtmp/navpart.txt";
	FILE *fp;
	int j = 0, r;
	fp = fopen(buf, "r");
	if (fp == NULL)
		return -1;
	if (mode == 0) {
		printf("<table>");	//<tr><td>评分</td><td>人数</td><td>讨论区</td><td>标题</td></tr>");
	} else
		printf
		    ("<table><tr><td>评分</td><td>人数</td><td>讨论区</td><td>作者</td><td>标题</td></tr>");
	r = now_t % 6;
	while (fgets(buf, sizeof (buf), fp) != NULL) {
		if (mode == 0 && j++ % 6 != r)
			continue;
		shownavpartline(buf, mode);
	}
	fclose(fp);
	printf("</table>");
	return 0;
}

void
shownavpartline(char *buf, int mode)
{
	char *numstr, *board, *author, *title, *boardstr, *ptr;
	int star, thread;
	struct boardmem *x1;
	star = atof(buf) + 0.5;
	numstr = strchr(buf, ' ');
	if (numstr == NULL)
		return;
	*(numstr++) = 0;
	board = strchr(numstr, ' ');
	if (board == NULL)
		return;
	*(board++) = 0;
	author = strchr(board, ' ');
	if (author == NULL)
		return;
	*(author++) = 0;
	ptr = strchr(author, ' ');
	if (ptr == NULL)
		return;
	*(ptr++) = 0;
	thread = atoi(ptr);
	title = strchr(ptr, ' ');
	if (title == NULL)
		return;
	*(title++) = 0;
	x1 = getboard(board);
	if (x1 == 0)
		boardstr = board;
	else {
		boardstr = x1->header.title;
	}
	if (mode == 0) {
		printf("<tr><td><li> <a href='tfind?B=%s&th=%s&T=%s'>",
		       board, ptr ,encode_url(title));
		if (!strncmp(title, "[转载] ", 7) && strlen(title) > 20)
			title += 7;
		if (strlen(title) > 45)
			title[45] = 0;
		printf("%s</a>", void1(titlestr(title)));
		if ((ptr = strchr(author, '.'))) {
			*++ptr = 0;
			printf(" [作者: %s]", author);
		} else {
			printf(" [作者: <a href=qry?U=%s class=blk>%s</a>]",
			       author, author);
		}
		printf(" &lt;<a href='%s%s' class=blk>%s</a>&gt;",
		       showByDefMode(), board, boardstr);
		printf("</td></tr>");
	} else {
		printf
		    ("<tr><td>%d</td><td>%s</td><td><a href='%s%s'>%s</a></td>"
		     "<td>", star, numstr,showByDefMode(), board, boardstr);
		printf("%s</td><td><a href='tfind?B=%s&th=%d&T=%s'>%s</a>"
		       "</td></tr>", userid_str(author), board, thread,
		       encode_url(title), void1(titlestr(title)));
	}
}


