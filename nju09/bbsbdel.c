#include "bbslib.h"

int
bbsbdel_main()
{
	FILE *fp;
	int i, total = 0;
	char path[80], userid[80];
	struct override b[MAXREJECTS];
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("您尚未登录，请先登录");
	changemode(GMENU);
	sethomefile(path, currentuser.userid, "rejects");
	printf("<center>%s -- 黑名单 [使用者: %s]<hr>\n", BBSNAME,
	       currentuser.userid);
	strsncpy(userid, getparm("userid"), 13);
	if (userid[0] == 0) {
		printf("<form action=bbsbdel>\n");
		printf("请输入欲删除的黑名单帐号: <input type=text><br>\n");
		printf("<input type=submit>\n");
		printf("</form>");
		http_quit();
	}
	loadbad(currentuser.userid);
	if (badnum <= 0)
		http_fatal("您没有设定任何黑名单");
	if (!isbad(userid))
		http_fatal("此人本来就不在你的黑名单里");
	for (i = 0; i < badnum; i++) {
		if (strcasecmp(bbb[i].id, userid)) {
			memcpy(&b[total], &bbb[i], sizeof (struct override));
			total++;
		}
	}
	fp = fopen(path, "w");
	fwrite(b, sizeof (struct override), total, fp);
	fclose(fp);
	printf
	    ("[%s]已从您的黑名单中删除.<br>\n <a href=bbsball>返回黑名单</a>",
	     userid);
	http_quit();
	return 0;
}
