#include "bbslib.h"

//modify by mintbaggio 20040829 for new www
int
bbsmsg_main()
{
	char buf[MAX_MSG_SIZE];
	char msgbuf[MAX_MSG_SIZE*2];
	int count, i;
	struct msghead head;
	html_header(1);
	check_msg();
	printf("<body>");
	printf("<div class=rhead>%s -- �鿴��Ϣ</div><hr>\n", BBSNAME);
	if (!loginok || isguest)
		http_fatal("�Ҵҹ����޷��鿴ѶϢ, ���ȵ�¼");
	changemode(LOOKMSGS);
	count =  get_msgcount(0, currentuser.userid);
	if (count == 0)
		http_fatal("û���κ�ѶϢ");
	for (i=0; i<count; i++) {
		load_msghead(0, currentuser.userid, &head, i);
		load_msgtext(currentuser.userid, &head, buf);
		translate_msg(buf, &head, msgbuf, 0);
		hprintf("%s", msgbuf);
	}
	u_info->unreadmsg = 0;
        printf("<a onclick='return confirm(\"�����Ҫ�������ѶϢ��?\")' href=bbsdelmsg>�������ѶϢ</a> ");
	printf("<a href=bbsmailmsg>�Ļ�������Ϣ</a>");
	http_quit();
	return 0;
}
