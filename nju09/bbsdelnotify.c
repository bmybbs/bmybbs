#include "bbslib.h"

/* ɾ�����ѵ�ʵ�� by IronBlood
 * ɾ���������ѣ��������
 * 		http://....../bbsdelnotify?type=0&B=sysop&ID=1234567890
 * 										^   ^^^^^    ^^^^^^^^^^
 * ɾ���������ѣ�
 * 		http://....../bbsdelnotify?type=-1
 */
int bbsdelnotify_main() {
	char board[80];
	int type, id;

	html_header(1);
	if(!loginok || isguest)
		http_fatal("����δ��¼");

	type=atoi(getparm("type")); // -1 ���� ȫ��ɾ��
	switch(type) {
		case NOTIFY_TYPE_NONSPECIFIED: // ɾ��ȫ������
			del_all_notification(currentuser.userid);
			break;
		case NOTIFY_TYPE_POST:
			strsncpy(board, getparm("B"), 32);
			id = atoi(getparm("ID"));
			del_post_notification(currentuser.userid, board, id);
			break;
		default: break;
	}

	// ���������б�
	printf("������ɾ��<br /><a href=\"bbsnotify\">�������������б�</a>");
	http_quit();
	return 0;
}
