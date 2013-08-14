#include "bbslib.h"

int
bbsfdel_main()
{
	FILE *fp;
	int i, total = 0;
	char path[80], userid[80];
	struct override f[200];
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("您尚未登录，请先登录");
	changemode(GMENU);
	sethomefile(path, currentuser.userid, "friends");
	printf("<center>%s -- 好友名单 [使用者: %s]<hr>\n", BBSNAME,
	       currentuser.userid);
	strsncpy(userid, getparm("userid"), 13);
	if (userid[0] == 0) {
		printf("<form action=bbsfdel>\n");
		printf("请输入欲删除的好友帐号: <input type=text><br>\n");
		printf("<input type=submit>\n");
		printf("</form>");
		http_quit();
	}
	loadfriend(currentuser.userid);
	if (friendnum <= 0)
		http_fatal("您没有设定任何好友");
	if (!isfriend(userid))
		http_fatal("此人本来就不在你的好友名单里");
	for (i = 0; i < friendnum; i++) {
		if (strcasecmp(fff[i].id, userid)) {
			memcpy(&f[total], &fff[i], sizeof (struct override));
			total++;
		}
	}
	fp = fopen(path, "w");
	fwrite(f, sizeof (struct override), total, fp);
	fclose(fp);
	printf
	    ("[%s]已从您的好友名单中删除.<br>\n <a href=bbsfall>返回好友名单</a>",
	     userid);
	http_quit();
	return 0;
}
