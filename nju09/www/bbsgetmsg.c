#include "bbslib.h"

struct emotion my_emotion[] = {
	{{":'(", NULL}, "cry_smile"},
	{{":-D", ":D", NULL}, "teeth_smile"},
	{{":-|", NULL}, "whatchutalkingabout_smile"},	//faint the name, heihei
	{{":-O", NULL}, "omg_smile"},
	{{":-(", ":(", NULL}, "sad_smile"},
	{{":-)", ":)", NULL}, "regular_smile"},
	{{":-P", ":P", NULL}, "tounge_smile"},
	{{":-S", NULL}, "confused_smile"},
	{{";-)", ";)", NULL}, "wink_smile"},
	{{":-$", NULL}, "embaressed_smile"},
	{{NULL}, NULL}
};

int
print_emote_table(char *form, char *input)
{
	int i;
	printf("<table>\n");
	for (i = 0; my_emotion[i].filename != NULL; i++) {
		if ((i % 3) == 0)
			printf("<tr>");
		printf
			("<td><a href=# onclick=document.%s.%s.value=document.%s.%s.value+\"%s\"><img src=/%s.gif></a></td>\n",
			form, input, form, input, my_emotion[i].smilename[0],
			my_emotion[i].filename);
	}
	printf("</table>\n");
	return 0;
}

int
emotion_print(char *msg)
{
	char *sstart, *ptr;
	int i, j;
	sstart = msg;
	for (i = 0; my_emotion[i].filename != NULL; i++) {
		for (j = 0; my_emotion[i].smilename[j] != NULL; j++) {
			ptr = strstr(sstart, my_emotion[i].smilename[j]);
			if (NULL == ptr)
				continue;
			*ptr = 0;
			emotion_print(sstart);
			sstart = ptr + strlen(my_emotion[i].smilename[j]);
			printf("<img src=/%s.gif></img>", my_emotion[i].filename);
			i = 0;
			break;
			if (*sstart == 0)
				return 0;
		}
	}
	if (*sstart != 0)
		hprintf("%s", sstart);
	return 0;
}

int
bbsgetmsg_main()
{
	char buf[MAX_MSG_SIZE], msg[MAX_MSG_SIZE * 50];
	// static int r = 0;
	int count, line;
	struct msghead head;
	html_header(11);
	if (!loginok || isguest) {
		printf("<body topmargin=1 MARGINHEIGHT=1><script>top.document.getElementById('fs1').rows=\"2, *, 15\";</script>\n</html>\n");
		return 0;
	}
	if (u_info->unreadmsg > 0) {
		count = get_unreadmsg(currentuser.userid);
		if (count == -1 || count == 0)
			goto outthere;
		if (currentuser.userdefine & DEF_SOUNDMSG)
			printf("<bgsound src=/msg.wav>\n");
		load_msghead(1, currentuser.userid, &head, count);
		load_msgtext(currentuser.userid, &head, buf);
		line = translate_msg(buf, &head, msg, 0);
		printf("<body topmargin=1 MARGINHEIGHT=1 style='BACKGROUND-COLOR: #f0ffd0'>\n");
		printf("<script>top.document.getElementById('fs1').rows=\"%d, *, 15\";</script>\n",
			(line + 11) * 16 + 32);
		emotion_print(msg);
		printf("<form name=form0 action=bbssendmsg method=post>\n"
			"<input type=hidden name=dr value=1>\n"
			"<input type=hidden name=destpid value=%d>\n"
			"<input type=hidden name=destid value='%s'>\n",
			head.frompid, head.id);
		printf("在下面直接回复讯息或 \n");
		printf("<a href=bbsgetmsg>[忽略该消息]</a><br>\n");
		printf("<table><tr><td>\n");
		printf("<textarea name=msg rows=5 cols=76>"
			"</textarea><br>\n");
		printf("</td><td>\n");
		print_emote_table("form0", "msg");
		printf("</td></tr></table><br>\n");
		printf("<input type=submit value=确认 width=6></form>\n");
		u_info->unreadmsg--;
		http_quit();
	}
outthere:
	u_info->unreadmsg = 0;
	printf("<body topmargin=1 MARGINHEIGHT=1><script>top.document.getElementById('fs1').rows=\"2, *, 15\";</script>");
	// printf("<script>function t(){return (new Date()).valueOf();}</script>");
	// printf("<script>setTimeout('self.location.replace(\"bbsgetmsg?sn='+t()+'\")', %d);</script>", 235000+r * 1000);
	// r = (r + now_t + u_info->uid) % 10;
	printf("</body>");
	http_quit();
	return 0;
}

void
check_msg()
{
	if (loginok && !isguest) {
		if (u_info->unreadmsg > 0)
			printf("<script>if(!top.fmsg.form0) top.fmsg.location.reload();</script>\n");
	}
}
