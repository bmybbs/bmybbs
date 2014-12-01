#include "bbslib.h"

int
bbssndmail_main()
{
	char mymaildir[80], userid[80], filename[80], title[80], title2[80],
	    *content;
	int i, sig, backup, allfriend, mark = 0;
	struct userec *u;
	html_header(1);
	strsncpy(userid, getparm("userid"), 40);
	if (!loginok || (isguest && strcmp(userid, "SYSOP")))
		http_fatal("�Ҵҹ��Ͳ���д�ţ����ȵ�¼");
	if (HAS_PERM(PERM_DENYMAIL))
		http_fatal("�����������Ȩ");
	sprintf(mymaildir, "mail/%c/%s/.DIR", mytoupper(currentuser.userid[0]),currentuser.userid);
	if (check_maxmail(mymaildir))
		http_fatal("���ĸ����ż����࣬������");
	changemode(SMAIL);
	strsncpy(title, getparm("title"), 50);
	backup = strlen(getparm("backup"));
	allfriend = strlen(getparm("allfriend"));
	if (!strstr(userid, "@") && !allfriend) {
		u = getuser(userid);
		if (u == 0)
			http_fatal("������������ʺ�");
		strcpy(userid, u->userid);
		if (inoverride(currentuser.userid, userid, "rejects"))
			http_fatal("�޷����Ÿ������");
	}
	for (i = 0; i < strlen(title); i++)
		if (title[i] <= 27 && title[i] >= -1)
			title[i] = ' ';
	sig = atoi(getparm("signature"));
	content = getparm("text");
	if (title[0] == 0)
		strcpy(title, "û����");
	sprintf(filename, "bbstmpfs/tmp/%d.tmp", thispid);
	f_write(filename, content);
	if (insertattachments(filename, content, currentuser.userid) > 0)
		mark |= FH_ATTACHED;
	if (!allfriend) {
		snprintf(title2, sizeof (title2), "{%s} %s", userid, title);
		post_mail(userid, title, filename, currentuser.userid,
			  currentuser.username, fromhost, sig - 1, mark);
	} else {
		loadfriend(currentuser.userid);
		snprintf(title2, sizeof (title2), "[Ⱥ���ż�] %.60s", title);
		for (i = 0; i < friendnum; i++) {
			u = getuser(fff[i].id);
			if (u == 0)
				continue;
			if (inoverride
			    (currentuser.userid, fff[i].id, "rejects"))
				continue;
			post_mail(fff[i].id, title2, filename,
				  currentuser.userid, currentuser.username,
				  fromhost, sig - 1, mark);
		}
	}
    if (backup) {
        post_mail_to_sent_box(currentuser.userid, title2, filename,
			  currentuser.userid, currentuser.username, fromhost,
			  sig - 1, mark);
    }
	unlink(filename);
	printf("�ż��Ѽĸ�%s.<br>\n", allfriend ? "���к���" : userid);
	if (backup)
		printf("�ż��Ѿ�����.<br>\n");
	printf("<a href='javascript:history.go(-2)'>����</a>");
	http_quit();
	return 0;
}
