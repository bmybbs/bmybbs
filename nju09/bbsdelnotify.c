#include "bbslib.h"

/* 删除提醒的实现 by IronBlood
 * 删除单条提醒，例如回帖
 * 		http://....../bbsdelnotify?type=0&B=sysop&ID=1234567890
 * 										^   ^^^^^    ^^^^^^^^^^
 * 删除所有提醒，
 * 		http://....../bbsdelnotify?type=-1
 */
int bbsdelnotify_main() {
	char board[80];
	int type, id;

	html_header(1);
	if(!loginok || isguest)
		http_fatal("您尚未登录");

	type=atoi(getparm("type")); // -1 留作 全部删除
	switch(type) {
		case NOTIFY_TYPE_NONSPECIFIED: // 删除全部提醒
			del_all_notification(currentuser.userid);
			break;
		case NOTIFY_TYPE_POST:
			strsncpy(board, getparm('B'), 32);
			id = atoi(getparm('ID'));
			del_post_notification(currentuser.userid, board, id);
			break;
		default: break;
	}

	// 返回提醒列表
	printf("提醒已删除<br /><a href=\"bbsnotify\">返回所有提醒列表</a>");
	http_quit();
	return 0;
}
