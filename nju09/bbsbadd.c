#include "bbslib.h"

int
bbsbadd_main()
{
	FILE *fp;
	char path[80], userid[80], exp[80];
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("您尚未登录，请先登录");
	changemode(GMENU);
	sethomefile(path, currentuser.userid, "rejects");
	printf("<body><center>%s -- 黑名单 [使用者: %s]<hr>\n", BBSNAME,
	       currentuser.userid);
	strsncpy(userid, getparm("userid"), 13);
	strsncpy(exp, getparm("exp"), 32);
	loadbad(currentuser.userid);
	if (userid[0] == 0 || exp[0] == 0) {
		if (userid[0])
			printf("<font color=red>请输入黑名单说明</font>");
		printf("<form action=bbsbadd>\n");
		printf
		    ("请输入欲加入的黑名单帐号: <input type=text name=userid value='%s'><br>\n",
		     userid);
		printf
		    ("请输入对这个黑名单账号的说明: <input type=text name=exp value='%s'>\n",
		     exp);
		printf("<br><input type=submit value=确定></form>\n");
		http_quit();
	}
	if (!getuser(userid))
		http_fatal("错误的使用者帐号");
	if (badnum >= MAXREJECTS-1)
		http_fatal("您的黑名单已达到上限, 不能添加新的黑名单");
	if (isfriend(userid))
		http_fatal("此人已经在你的黑名单里了");
	strcpy(bbb[badnum].id, getuser(userid)->userid);
	strcpy(bbb[badnum].exp, exp);
	badnum++;
	fp = fopen(path, "w");
	fwrite(bbb, sizeof (struct override), badnum, fp);
	fclose(fp);
	printf("[%s]已加入您的黑名单.<br>\n <a href=bbsball>返回黑名单</a>",
	       userid);
	http_quit();
	return 0;
}
