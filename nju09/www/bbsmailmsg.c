#include "bbslib.h"

static void mail_msg(struct userec *user);

int
bbsmailmsg_main()
{
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("�Ҵҹ��Ͳ��ܴ���ѶϢ�����ȵ�¼");
	changemode(SMAIL);
	mail_msg(&currentuser);
	u_info->unreadmsg = 0;
	printf("ѶϢ�����Ѿ��Ļ���������");
	printf("<a href='javascript:history.go(-2)'>����</a>");
	http_quit();
	return 0;
}

static void mail_msg(struct userec *user) {
	char fname[30];
	char buf[MAX_MSG_SIZE], showmsg[MAX_MSG_SIZE * 2];
	int i;
	struct msghead head;
	time_t now;
	char title[STRLEN];
	FILE *fn;
	int count;

	sprintf(fname, "tmp/%s.msg", user->userid);
	fn = fopen(fname, "w");
	count = get_msgcount(0, user->userid);
	for (i = 0; i < count; i++) {
		load_msghead(0, user->userid, &head, i);
		load_msgtext(user->userid, &head, buf);
		translate_msg(buf, &head, showmsg, sizeof showmsg, 0);
		fprintf(fn, "%s", showmsg);
	}
	fclose(fn);

	now = time(0);
	sprintf(title, "[%12.12s] ����ѶϢ����", ctime(&now) + 4);
	mail_file(fname, user->userid, title, user->userid);
	unlink(fname);
	clear_msg(user->userid);
}

