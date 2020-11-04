#include "bbslib.h"
// bbsgetmsg
int print_emote_table(char *form, char *input);
static int checkmsgbuf(char *msg);

int
bbssendmsg_main()
{
	int i;
	int mode, destpid = 0;
	int direct_reply;
	char destid[20], msg[MAX_MSG_SIZE];
	struct userec *u;
	int offline = 0;
	html_header(1);
	changemode(MSG);
	if (!loginok || isguest)
		http_fatal("匆匆过客不能发讯息, 请先登录！");
	//if (!((currentuser.userlevel )& (PERM_CHAT|PERM_PAGE)))
	if (!((currentuser.userlevel )& (PERM_PAGE)))
		http_fatal("您没有权限发讯息");
	ytht_strsncpy(destid, getparm("destid"), 13);
	ytht_strsncpy(msg, getparm("msg"), MAX_MSG_SIZE);
	direct_reply = atoi(getparm("dr"));
	destpid = atoi(getparm("destpid"));
	if (destid[0] == 0 || msg[0] == 0) {
		char buf3[256];
		strcpy(buf3, "<body onload='document.form0.msg.focus()'>");
		if (destid[0] == 0)
			strcpy(buf3, "<body onload='document.form0.destid.focus()'>");
		printf("%s\n", buf3);
		printf("<div class=rhead>%s -- 发送信息</div><hr>\n", BBSNAME);
		printf("<form name=form0 action=bbssendmsg method=post>"
				"<input type=hidden name=destpid value=%d>"
				"送讯息给: <input name=destid maxlength=12 value='%s' size=12><br>"
				"讯息内容:\n<br>", destpid, destid);
		printf("<table><tr><td><textarea name=msg rows=5 cols=76>"
				"%s" "</textarea></td><td>", nohtml(void1(msg)));
		print_emote_table("form0", "msg");
		printf("</td></tr></table>"
				"<input type=submit value=确认 width=6><input type=button value=取消 width=6 onclick=\"window.location.href='bbsgetmsg'\"></form>");
		http_quit();
	}
	if (checkmsgbuf(msg))
		http_fatal("消息太长了？最多11行哦");
	u = getuser(destid);
	if (u == 0)
		http_fatal("错误的帐号");
	strcpy(destid, u->userid);
	if (!strcasecmp(destid, currentuser.userid))
		http_fatal("你不能给自己发讯息！");
	if (!strcasecmp(destid, "guest"))
		http_fatal("无法发讯息给这个人 1");
	if (!((u->userdefine & DEF_ALLMSG)
		|| ((u->userdefine & DEF_FRIENDMSG) && ythtbbs_override_included(destid, YTHTBBS_OVERRIDE_FRIENDS, currentuser.userid))))
		http_fatal("无法发讯息给这个人 2");
	if (!strcmp(destid, "SYSOP"))
		http_fatal("无法发讯息给这个人 3");
	if (ythtbbs_override_included(destid, YTHTBBS_OVERRIDE_REJECTS, currentuser.userid))
		http_fatal("无法发讯息给这个人 4");
	if (get_unreadcount(destid) > MAXMESSAGE)
		http_fatal("对方尚有一些讯息未处理，请稍候再发或给他(她)写信...");
	printf("<body>\n");
	for (i = 0; i < MAXACTIVE; i++)
		if (shm_utmp->uinfo[i].active)
			if (!strcasecmp(shm_utmp->uinfo[i].userid, destid)) {
				if (destpid != 0 && shm_utmp->uinfo[i].pid != destpid)
					continue;
				destpid = shm_utmp->uinfo[i].pid;
				mode = shm_utmp->uinfo[i].mode;
				if (mode == BBSNET || mode == PAGE || mode == LOCKSCREEN) offline = 1;
				if (send_msg(currentuser.userid, i, destid, destpid, msg, offline) == 1)
					printf("已经帮你送出%s消息", offline ? "离线" : "");
				else
					printf("发送消息失败");
				printf("<script>top.fmsg.location='bbsgetmsg'</script>\n");
				if (!direct_reply) {
					printf("<br><form name=form1><input name=b1 type=button onclick='history.go(-2)' value='[返回]'>");
					printf("</form>");
				}
				http_quit();
			}

	if (send_msg(currentuser.userid, i, destid, destpid, msg, 1) == 1)
		printf("已经帮你送出离线消息");
	else
		printf("发送消息失败");
	printf("<script>top.fmsg.location='bbsgetmsg'</script>\n");
	if (!direct_reply) {
		printf("<br><form name=form1><input name=b1 type=button onclick='history.go(-2)' value='[返回]'>");
		printf("</form>");
	}
	http_quit();
	return 0;
}

static int checkmsgbuf(char *msg) {
	char *tmp2, *tmp1;
	int line = 0;
	tmp2 = msg;
	while (1) {
		tmp1 = strchr(tmp2, '\n');
		if (tmp1 == NULL)
			break;
		if (tmp1 - tmp2 >= 80)
			return -1;
		tmp2 = tmp1 + 1;
		line++;
	}
	if (tmp1 != NULL)
		if (*tmp1 != 0)
			line++;
	if (strlen(msg) - (tmp2 - msg) >= 80)
		return -1;
	if (line > 11)
		return -2;
	return 0;
}
