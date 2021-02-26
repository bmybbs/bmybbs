#include "bbslib.h"

int
bbsfdel_main()
{
	unsigned int i;
	int lockfd;
	char userid[80];
	struct ythtbbs_override f[MAXFRIENDS], EMPTY;
	struct userec *lookupuser;

	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("您尚未登录，请先登录");
	changemode(GMENU);
	printf("<center>%s -- 好友名单 [使用者: %s]<hr>\n", BBSNAME, currentuser.userid);
	ytht_strsncpy(userid, getparm("userid"), 13);
	if (userid[0] == 0) {
		printf("<form action=bbsfdel>\n");
		printf("请输入欲删除的好友帐号: <input type=text><br>\n");
		printf("<input type=submit>\n");
		printf("</form>");
		http_quit();
	}

	lockfd = ythtbbs_override_lock(currentuser.userid, YTHTBBS_OVERRIDE_FRIENDS);
	friendnum = ythtbbs_override_count(currentuser.userid, YTHTBBS_OVERRIDE_FRIENDS);
	if (friendnum <= 0) {
		ythtbbs_override_unlock(lockfd);
		http_fatal("您没有设定任何好友");
	}

	lookupuser = getuser(userid);
	if (!lookupuser || !ythtbbs_override_included(currentuser.userid, YTHTBBS_OVERRIDE_FRIENDS, lookupuser->userid)) {
		ythtbbs_override_unlock(lockfd);
		http_fatal("此人本来就不在你的好友名单里");
	}

	ythtbbs_override_get_records(currentuser.userid, f, friendnum, YTHTBBS_OVERRIDE_FRIENDS);
	for (i = 0; i < friendnum; i++) {
		f[i].id[sizeof(EMPTY.id) - 1] = 0;
		if (strcasecmp(f[i].id, userid) == 0) {
			f[i].id[0] = '\0';
			friendnum--;
			break; // found
		}
	}
	ythtbbs_override_set_records(currentuser.userid, f, friendnum, YTHTBBS_OVERRIDE_FRIENDS);
	ythtbbs_override_unlock(lockfd);

	printf("[%s]已从您的好友名单中删除.<br>\n <a href=bbsfall>返回好友名单</a>", userid);
	http_quit();
	return 0;
}

