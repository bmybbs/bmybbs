#include "bbslib.h"

int bbsnotify_main() {

	NotifyItemList allNotifyItems;

	if(!loginok || isguest)
		http_fatal("您尚未登录，请先登录");

	allNotifyItems = parse_notification(currentuser.userid);

	html_header(1);
	check_msg();
	changemode(READING);

	if(allNotifyItems == NULL) {
		printf("<body>没有提醒</body>");
		http_quit();
		return 0;
	}

	printf("<body><ul>");

	struct NotifyItem * currItem;
	for(currItem = (struct NotifyItem *)allNotifyItems; currItem != NULL; currItem = currItem->next) {
		switch(currItem->type) {
			case NOTIFY_TYPE_POST:
				printf("<li>[<a href=\"bbsdelnotify?type=0&amp;B=%s&amp;ID=%d\">删除</a>] [%s]"
					   "网友<a href=\"qry?U=%s\">%s</a>在<a href=\"bbscon?B=%s&amp;F=M.%d.A\">%s</a>中回复了您</li>",
					    currItem->board, (int)currItem->noti_time, ctime(&currItem->noti_time),
					    currItem->from_userid, currItem->from_userid,
						currItem->board, (int)currItem->noti_time, currItem->title_gbk);
				break;
			default : break;
		}
	}
	free_notification(allNotifyItems);

	printf("</ul></body>");
	http_quit();
	return 0;
}
