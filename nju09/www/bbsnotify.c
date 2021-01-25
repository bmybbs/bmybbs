#include "bbslib.h"

int bbsnotify_main() {

	NotifyItemList allNotifyItems;

	if(!loginok || isguest)
		http_fatal("您尚未登录，请先登录");

	allNotifyItems = parse_notification(currentuser.userid);

	html_header(1);
	check_msg();
	changemode(READING);

	// style 就直接写在这里了
	printf("<style type=\"text/css\">\n"
			"body { padding: 40px }\n"
			"div#notify-content { width: 80%% }\n"
			"div#notify-content h1 { width: 100%%; border-bottom: 1px #AAAAAA solid; padding-bottom: 12px }\n"
			"div.notify-item { border-bottom: 1px #AAAAAA solid; padding: 12px 15px; }\n"
			"div.notify-item a { display: inline-block; margin: 0 5px; }\n"
			"div.mention-item { background-color: #F8F8F8 }\n"
			"</style>");

	printf("<body><div id=\"notify-content\"><h1>我的提醒</h1>");

	if(allNotifyItems == NULL) {
		printf("<div class=\"notify-item\">当前没有提醒，到<a href=\"bbsboa?secstr=*\">预定讨论区</a>多多灌水吧 :-)</div></div></body>");
		http_quit();
		return 0;
	}

	struct NotifyItem * currItem;
	for(currItem = (struct NotifyItem *)allNotifyItems; currItem != NULL; currItem = currItem->next) {
		switch(currItem->type) {
			case NOTIFY_TYPE_POST:
				printf("<div class=\"notify-item\">[<a href=\"bbsdelnotify?type=0&amp;B=%s&amp;ID=%ld\">忽略</a>] "
					   "网友<a href=\"qry?U=%s\">%s</a>%s在<a href=\"bbscon?B=%s&amp;F=M.%ld.A\">%s</a>中回复了您</div>",
					    currItem->board, currItem->noti_time,
					    currItem->from_userid, currItem->from_userid, ytht_Difftime(currItem->noti_time),
						currItem->board, currItem->noti_time, currItem->title_gbk);
				break;
			case NOTIFY_TYPE_MENTION:
				printf("<div class=\"notify-item mention-item\">[<a href=\"bbsdelnotify?type=0&amp;B=%s&amp;ID=%d\">忽略</a>] "
					   "网友<a href=\"qry?U=%s\">%s</a>%s在<a href=\"bbscon?B=%s&amp;F=M.%d.A\">%s</a>中&nbsp;<strong>@</strong>&nbsp;了您</div>",
						currItem->board, (int)currItem->noti_time,
						currItem->from_userid, currItem->from_userid, ytht_Difftime(currItem->noti_time),
						currItem->board, (int)currItem->noti_time, currItem->title_gbk);
				break;
			default : break;
		}
	}
	free_notification(allNotifyItems);
	printf("<div><a href=\"bbsdelnotify?type=%d\">忽略全部提醒</a></div>", NOTIFY_TYPE_NONSPECIFIED);
	printf("</div></body>");
	http_quit();
	return 0;
}
