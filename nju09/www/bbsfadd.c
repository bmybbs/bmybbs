#include "bbslib.h"

int
bbsfadd_main()
{
	struct userec *lookupuser;
	int lockfd;
	struct ythtbbs_override of;

	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("您尚未登录，请先登录");
	changemode(GMENU);
	printf("<body><center>%s -- 好友名单 [使用者: %s]<hr>\n", BBSNAME, currentuser.userid);
	ytht_strsncpy(of.id, getparm("userid"), 13);
	ytht_strsncpy(of.exp, getparm("exp"), 32);
	if (of.id[0] == 0 || of.exp[0] == 0) {
		if (of.exp[0])
			printf("<font color=red>请输入好友说明</font>");
		printf("<form action=bbsfadd>\n");
		printf("请输入欲加入的好友帐号: <input type=text name=userid value='%s'><br>\n", (of.id[0] == 0) ? "" : of.id);
		printf("请输入对这个好友的说明: <input type=text name=exp value='%s'>\n", (of.exp[0] == 0) ? "" : of.exp);
		printf("<br><input type=submit value=确定></form>\n");
		http_quit();
	}

	lookupuser = getuser(of.id);
	if (!lookupuser)
		http_fatal("错误的使用者帐号");

	lockfd = ythtbbs_override_lock(currentuser.userid, YTHTBBS_OVERRIDE_FRIENDS);
	friendnum = ythtbbs_override_count(currentuser.userid, YTHTBBS_OVERRIDE_FRIENDS);
	if (friendnum >= MAXFRIENDS-1) {
		ythtbbs_override_unlock(lockfd);
		http_fatal("您的好友名单已达到上限, 不能添加新的好友");
	}

	if (ythtbbs_override_included(currentuser.userid, YTHTBBS_OVERRIDE_FRIENDS, lookupuser->userid)) {
		ythtbbs_override_unlock(lockfd);
		http_fatal("此人已经在你的好友名单里了");
	}

	ytht_strsncpy(of.id, lookupuser->userid, sizeof(of.id));
	ythtbbs_override_add(currentuser.userid, &of, YTHTBBS_OVERRIDE_FRIENDS);
	ythtbbs_override_unlock(lockfd);

	printf("[%s]已加入您的好友名单.<br>\n <a href=bbsfall>返回好友名单</a>", of.id);
	http_quit();
	return 0;
}

