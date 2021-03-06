#include "bbslib.h"

int
bbssndmail_main()
{
	char mymaildir[80], userid[IDLEN + 2], filename[80], title[80], title2[80 * 2], *content;
	int sig, backup, allfriend, mark = 0;
	int lockfd;
	size_t i;
	struct userec *u;
	html_header(1);
	ytht_strsncpy(userid, getparm("userid"), sizeof(userid));
	if (!loginok || (isguest && strcmp(userid, "SYSOP")))
		http_fatal("匆匆过客不能写信，请先登录");
	if (HAS_PERM(PERM_DENYMAIL, currentuser))
		http_fatal("您被封禁发信权");
	sprintf(mymaildir, "mail/%c/%s/.DIR", mytoupper(currentuser.userid[0]),currentuser.userid);
	if (check_maxmail())
		http_fatal("您的个人信件过多，请整理");
	changemode(SMAIL);
	ytht_strsncpy(title, getparm("title"), 50);
	backup = strlen(getparm("backup"));
	allfriend = strlen(getparm("allfriend"));
	if (!strstr(userid, "@") && !allfriend) {
		u = getuser(userid);
		if (u == 0)
			http_fatal("错误的收信人帐号");
		strcpy(userid, u->userid);
		if (ythtbbs_override_included(userid, YTHTBBS_OVERRIDE_REJECTS, currentuser.userid))
			http_fatal("无法发信给这个人");
	}
	for (i = 0; i < strlen(title); i++)
		if (title[i] <= 27 && title[i] >= -1)
			title[i] = ' ';
	sig = atoi(getparm("signature"));
	content = getparm("text");
	if (title[0] == 0)
		strcpy(title, "没主题");
	sprintf(filename, "bbstmpfs/tmp/%d.tmp", thispid);
	f_write(filename, content);
	if (insertattachments(filename, content, currentuser.userid) > 0)
		mark |= FH_ATTACHED;
	if (!allfriend) {
		snprintf(title2, sizeof (title2), "{%s} %s", userid, title);
		post_mail(userid, title, filename, currentuser.userid,
				currentuser.username, fromhost, sig - 1, mark);
	} else {
		lockfd = ythtbbs_override_lock(currentuser.userid, YTHTBBS_OVERRIDE_FRIENDS);
		ythtbbs_override_get_records(currentuser.userid, fff, MAXFRIENDS, YTHTBBS_OVERRIDE_FRIENDS);
		ythtbbs_override_unlock(lockfd);
		snprintf(title2, sizeof (title2), "[群体信件] %.60s", title);
		struct ythtbbs_override EMPTY;
		for (i = 0; i < friendnum; i++) {
			fff[i].id[sizeof(EMPTY.id) - 1] = 0;
			u = getuser(fff[i].id);
			if (u == 0)
				continue;
			if (ythtbbs_override_included(fff[i].id, YTHTBBS_OVERRIDE_REJECTS, currentuser.userid))
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
	printf("信件已寄给%s.<br>\n", allfriend ? "所有好友" : userid);
	if (backup)
		printf("信件已经备份.<br>\n");
	printf("<a href='javascript:history.go(-2)'>返回</a>");
	http_quit();
	return 0;
}
