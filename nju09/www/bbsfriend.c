#include "bbslib.h"

int
bbsfriend_main()
{
	int i, total = 0;
	struct user_info user[MAXFRIENDS];
	struct user_info *x;
	html_header(1);
	check_msg();
	changemode(FRIEND);
	printf("<body><center>\n");
	printf("<div class=rhead>%s -- 在线好友列表 [使用者:<span class=h11> %s</span>]</div><hr>\n", BBSNAME,
	       currentuser.userid);
	for (i = 0; i < MAXACTIVE && total < MAXFRIENDS; i++) {
		x = &(shm_utmp->uinfo[i]);
		if (x->active == 0)
			continue;
		if (x->invisible && !HAS_PERM(PERM_SEECLOAK, currentuser))
			continue;
		if (!isfriend(x->userid))
			continue;
		memcpy(&user[total], x, sizeof (struct user_info));
		total++;
	}
	printf("<table border=1>\n");
	printf
	    ("<tr><td>序号</td><td>友</td><td>使用者代号</td><td>使用者昵称</td><td>来自</td><td>动态</td><td>发呆</td></tr>\n");
	qsort(user, total, sizeof (struct user_info), (void *) cmpuser);
	for (i = 0; i < total; i++) {
		int dt = (now_t - user[i].lasttime) / 60;
		printf("<tr><td>%d</td>", i + 1);
		printf("<td>%s</td>", "√");
		printf("<td><a href=bbsqry?userid=%s>%s</a></td>",
		       user[i].userid, user[i].userid);
		printf("<td><a href=bbsqry?userid=%s>%24.24s</a></td>",
		       user[i].userid, nohtml(void1(user[i].username)));
		printf("<td><font class=c%d>%20.20s</font></td>",
		       user[i].pid == 1 ? 35 : 32, user[i].from);
		if(user[i].mode != USERDF4)
		printf("<td>%s</td>",
		       user[i].
		       invisible ? "隐身中..." : ModeType(user[i].mode));
		else
		printf("<td>%s</td>",
		       user[i].
		       invisible ? "隐身中..." : user[i].user_state_temp);
		if (dt == 0) {
			printf("<td></td></tr>\n");
		} else {
			printf("<td>%d</td></tr>\n", dt);
		}
	}
	printf("</table>\n");
	if (total == 0)
		printf("目前没有好友在线");
	printf("<hr>");
	printf("[<a href=bbsfall>全部好友名册</a>]");
	printf("</center></body>\n");
	http_quit();
	return 0;
}
