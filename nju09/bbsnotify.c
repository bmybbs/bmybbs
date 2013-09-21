#include "bbslib.h"

int bbsnotify_main() {

	NotifyItemList allNotifyItems;

	if(!loginok || isguest)
		http_fatal("����δ��¼�����ȵ�¼");

	allNotifyItems = parse_notification(currentuser.userid);

	html_header(1);
	check_msg();
	changemode(READING);

	// style ��ֱ��д��������
	printf("<style type=\"text/css\">\n"
			"body { padding: 40px }\n"
			"div#notify-content { width: 80% }\n"
			"div#notify-content h1 { width: 100%; border-bottom: 1px #AAAAAA solid; padding-bottom: 12px }\n"
			"div.notify-item { border-bottom: 1px #AAAAAA solid; padding: 12px 15px; }\n"
			"div.notify-item a { display: inline-block; margin: 0 5px; }\n"
			"</style>");

	printf("<body><div id=\"notify-content\"><h1>�ҵ�����</h1>");

	if(allNotifyItems == NULL) {
		printf("<div class=\"notify-item\">��ǰû�����ѣ���<a href=\"bbsboa?secstr=*\">Ԥ��������</a>����ˮ�� :-)</div></div></body>");
		http_quit();
		return 0;
	}

	struct NotifyItem * currItem;
	for(currItem = (struct NotifyItem *)allNotifyItems; currItem != NULL; currItem = currItem->next) {
		switch(currItem->type) {
			case NOTIFY_TYPE_POST:
				printf("<div class=\"notify-item\">[<a href=\"bbsdelnotify?type=0&amp;B=%s&amp;ID=%d\">����</a>] "
					   "����<a href=\"qry?U=%s\">%s</a>%s��<a href=\"bbscon?B=%s&amp;F=M.%d.A\">%s</a>�лظ�����</div>",
					    currItem->board, (int)currItem->noti_time,
					    currItem->from_userid, currItem->from_userid, Difftime(currItem->noti_time),
						currItem->board, (int)currItem->noti_time, currItem->title_gbk);
				break;
			default : break;
		}
	}
	free_notification(allNotifyItems);
	printf("<div><a href=\"bbsdelnotify?type=%d\">����ȫ������</a></div>", NOTIFY_TYPE_NONSPECIFIED);
	printf("</div></body>");
	http_quit();
	return 0;
}
