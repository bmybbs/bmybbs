#include "bbslib.h"

int
bbsfall_main()
{	//modify by mintbaggio 20040829 for new www
	unsigned int i;
	int lockfd;
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("您尚未登录, 请先登录");
	changemode(GMENU);

	lockfd = ythtbbs_override_lock(currentuser.userid, YTHTBBS_OVERRIDE_FRIENDS);
	friendnum = ythtbbs_override_count(currentuser.userid, YTHTBBS_OVERRIDE_FRIENDS);
	ythtbbs_override_get_records(currentuser.userid, fff, friendnum, YTHTBBS_OVERRIDE_FRIENDS);
	ythtbbs_override_unlock(lockfd);

	printf("<body><center>\n");
	printf("<div class=rhead>%s -- 好友名单 [使用者: <span class=h11>%s</span>]</div><hr><br>\n", BBSNAME, currentuser.userid);
	printf("您共设定了 %ld 位好友<br>", friendnum);
	printf("<table border=1><tr><td>序号</td><td>好友代号</td><td>好友说明</td><td>删除好友</td></tr>");
	for (i = 0; i < friendnum; i++) {
		printf("<tr><td>%d</td>", i + 1);
		printf("<td><a href=bbsqry?userid=%s>%s</a></td>", fff[i].id, fff[i].id);
		printf("<td>%s</td>\n", nohtml(fff[i].exp));
		printf("<td>[<a onclick='return confirm(\"确实删除吗?\")' href=bbsfdel?userid=%s>删除</a>]</td></tr>", fff[i].id);
	}
	printf("</table><hr>\n");
	printf("[<a href=bbsfadd>添加新的好友</a>]</center></body>\n");
	http_quit();
	return 0;
}

