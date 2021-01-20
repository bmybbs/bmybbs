#include "bbslib.h"

int
bbsbdel_main()
{
	int i, lockfd;
	char userid[80];
	struct ythtbbs_override b[MAXREJECTS];
	struct userec *lookupuser;

	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("您尚未登录，请先登录");
	changemode(GMENU);
	printf("<center>%s -- 黑名单 [使用者: %s]<hr>\n", BBSNAME, currentuser.userid);
	ytht_strsncpy(userid, getparm("userid"), 13);
	if (userid[0] == 0) {
		printf("<form action=bbsbdel>\n");
		printf("请输入欲删除的黑名单帐号: <input type=text><br>\n");
		printf("<input type=submit>\n");
		printf("</form>");
		http_quit();
	}

	lockfd = ythtbbs_override_lock(currentuser.userid, YTHTBBS_OVERRIDE_REJECTS);
	if (lockfd < 0)
		http_fatal("内部错误，请重试");
	badnum = ythtbbs_override_count(currentuser.userid, YTHTBBS_OVERRIDE_REJECTS);
	if (badnum <= 0) {
		ythtbbs_override_unlock(lockfd);
		http_fatal("您没有设定任何黑名单");
	}

	lookupuser = getuser(userid);
	if (!lookupuser || !ythtbbs_override_included(currentuser.userid, YTHTBBS_OVERRIDE_REJECTS, lookupuser->userid)) {
		ythtbbs_override_unlock(lockfd);
		http_fatal("此人本来就不在你的黑名单里");
	}

	ythtbbs_override_get_records(currentuser.userid, b, badnum, YTHTBBS_OVERRIDE_REJECTS);
	for (i = 0; i < badnum; i++) {
		if (strcasecmp(b[i].id, userid) == 0) {
			b[i].id[0] = '\0';
			badnum--;
			break; // found!
		}
	}
	ythtbbs_override_set_records(currentuser.userid, b, badnum, YTHTBBS_OVERRIDE_REJECTS);
	ythtbbs_override_unlock(lockfd);
	printf("[%s]已从您的黑名单中删除.<br>\n <a href=bbsball>返回黑名单</a>", userid);
	http_quit();
	return 0;
}
