#include "bbslib.h"

int bbsnotify_main() {

	NotifyItemList allNotifyItems;

	if(!loginok || isguest)
		http_fatal("ÄúÉÐÎ´µÇÂ¼£¬ÇëÏÈµÇÂ¼");

	allNotifyItems = parse_notification(currentuser.userid);

	html_header(1);
	check_msg();
	changemode(READING);

	if(allNotifyItems == NULL) {
		printf("<body>Ã»ÓÐÌáÐÑ</body>");
		http_quit();
		return 0;
	}

	printf("<body><ul>");

	struct NotifyItem * currItem;
	for(currItem = (struct NotifyItem *)allNotifyItems; currItem != NULL; currItem = currItem->next) {
		switch(currItem->type) {
			case NOTIFY_TYPE_POST:
				printf("<li><a href=\"bbscon?B=%s&amp;F=M.%d.A\">%s</a></li>",
						currItem->board, currItem->noti_time, currItem->title_gbk);
				break;
			default : break;
		}
	}
	free_notification(allNotifyItems);

	printf("</ul></body>");
	http_quit();
	return 0;
}
