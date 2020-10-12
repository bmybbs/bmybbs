#include "bbslib.h"

int
bbsbadd_main()
{
	struct userec *lookupuser;
	int lockfd;
	struct ythtbbs_override ob;

	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("您尚未登录，请先登录");
	changemode(GMENU);
	printf("<body><center>%s -- 黑名单 [使用者: %s]<hr>\n", BBSNAME, currentuser.userid);
	ytht_strsncpy(ob.id, getparm("userid"), 13);
	ytht_strsncpy(ob.exp, getparm("exp"), 32);
	loadbad(currentuser.userid);
	if (ob.id[0] == 0 || ob.exp[0] == 0) {
		if (ob.id[0])
			printf("<font color=red>请输入黑名单说明</font>");
		printf("<form action=bbsbadd>\n");
		printf("请输入欲加入的黑名单帐号: <input type=text name=userid><br>\n");
		printf("请输入对这个黑名单账号的说明: <input type=text name=exp>\n");
		printf("<br><input type=submit value=确定></form>\n");
		http_quit();
	}

	lookupuser = getuser(ob.id);
	if (!lookupuser)
		http_fatal("错误的使用者帐号");

	lockfd = ythtbbs_override_lock(currentuser.userid, YTHTBBS_OVERRIDE_REJECTS);
	badnum = ythtbbs_override_count(currentuser.userid, YTHTBBS_OVERRIDE_REJECTS);
	if (badnum >= MAXREJECTS-1) {
		ythtbbs_override_unlock(lockfd);
		http_fatal("您的黑名单已达到上限, 不能添加新的黑名单");
	}

	if (ythtbbs_override_included(currentuser.userid, YTHTBBS_OVERRIDE_REJECTS, lookupuser->userid)) {
		ythtbbs_override_unlock(lockfd);
		http_fatal("此人已经在你的黑名单里了");
	}

	strcpy(ob.id, lookupuser->userid);
	ythtbbs_override_add(currentuser.userid, &ob, YTHTBBS_OVERRIDE_REJECTS);
	ythtbbs_override_unlock(lockfd);

	printf("[%s]已加入您的黑名单.<br>\n <a href=bbsball>返回黑名单</a>", ob.id);
	http_quit();
	return 0;
}
