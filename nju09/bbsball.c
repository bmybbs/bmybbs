#include "bbslib.h"

int
bbsball_main()
{	//modify by mintbaggio 20040829 for new www
	int i;
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("您尚未登录, 请先登录");
	changemode(GMENU);
	loadbad(currentuser.userid);
	printf("<body><center>\n");
	printf("<div class=rhead>%s -- 黑名单 [使用者: <span class=h11>%s</span>]</div><hr><br>\n", BBSNAME,
	       currentuser.userid);
	printf("您共设定了 %d 位黑名单<br>", badnum);
	printf
	    ("<table border=1><tr><td>序号</td><td>黑名单代号</td><td>黑名单说明</td><td>从黑名单删除</td></tr>");
	for (i = 0; i < badnum; i++) {
		printf("<tr><td>%d</td>", i + 1);
		printf("<td><a href=bbsqry?userid=%s>%s</a></td>", bbb[i].id,
		       bbb[i].id);
		printf("<td>%s</td>\n", nohtml(bbb[i].exp));
		printf
		    ("<td>[<a onclick='return confirm(\"确实删除吗?\")' href=bbsbdel?userid=%s>删除</a>]</td></tr>",
		     bbb[i].id);
	}
	printf("</table><hr>\n");
	printf("[<a href=bbsbadd>添加新的黑名单</a>]</center></body>\n");
	http_quit();
	return 0;
}
