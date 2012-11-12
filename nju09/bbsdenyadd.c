#include "bbslib.h"

static int show_form(char *board);
static int inform(char *board, char *user, char *exp, int dt);

int
bbsdenyadd_main()
{
	int i;
	char buf[256], exp[80], board[80], *userid;
	int dt;
	struct userec *x;
	struct boardmem *x1;
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("您尚未登录, 请先登录");
	changemode(READING);
	strsncpy(board, getparm("board"), 30);
	strsncpy(exp, getparm("exp"), 30);
	dt = atoi(getparm("dt"));
	if (!(x1 = getboard(board)))
		http_fatal("错误的讨论区");
	if (!has_BM_perm(&currentuser, x1))
		http_fatal("你无权进行本操作");
	loaddenyuser(board);
	userid = getparm("userid");
	if (userid[0] == 0)
		return show_form(board);
	if ((x = getuser(userid)) == 0)
		http_fatal("错误的使用者帐号");
	if (!has_post_perm(x, x1))
		http_fatal("这个人本来就没有post权");
	strcpy(userid, x->userid);
	if (!(currentuser.userlevel & PERM_SYSOP) && (dt > 20))
		http_fatal("封禁时间大于20天,超过了权限,若需要,请联系站长");
	if (dt < 1 || dt > 99)
		http_fatal("请输入被封天数(1-99)");
	if (exp[0] == 0)
		http_fatal("请输入封人原因");
	for (i = 0; i < denynum; i++)
		if (!strcasecmp(denyuser[i].id, userid))
			http_fatal("此用户已经被封");
	if (denynum > 40)
		http_fatal("太多人被封了");
	strsncpy(denyuser[denynum].id, userid, 13);
	strsncpy(denyuser[denynum].exp, exp, 30);
	denyuser[denynum].free_time = now_t + dt * 86400;
	denynum++;
	savedenyuser(board);
	printf("封禁 %s 成功<br>\n", userid);
	snprintf(buf, 256, "%s deny %s %s", currentuser.userid, board, userid);
	newtrace(buf);
	inform(board, userid, exp, dt);
	printf("[<a href=bbsdenyall?board=%s>返回被封帐号名单</a>]", board);
	http_quit();
	return 0;
}

static int
show_form(char *board)
{
	printf("<center>%s -- 版务管理 [讨论区: %s]<hr>\n", BBSNAME, board);
	printf
	    ("<form action=bbsdenyadd><input type=hidden name=board value='%s'>",
	     board);
	printf
	    ("封禁使用者<input name=userid size=12> 本版POST权 <input name=dt size=2> 天, 原因<input name=exp size=20>\n");
	printf("<input type=submit value=确认></form>");
	return 0;
}

static int
inform(char *board, char *user, char *exp, int dt)
{
	FILE *fp;
	char path[80], title[80], buf[256];
	struct tm *tmtime;
	time_t daytime = now_t + dt * 24 * 60 * 60;//by bjgyt
	tmtime = gmtime(&daytime);
	sprintf(title, "%s被%s取消在%s的POST权利", user,
		currentuser.userid, board);
	sprintf(path, "bbstmpfs/tmp/%d.tmp", thispid);
	fp = fopen(path, "w");
	fprintf(fp, "【此篇文章是由自动发信系统所张贴】\n\n");
	snprintf(buf, sizeof (buf),
		 "封人原因: %s\n"
		 "被封天数: %d\n"
		 "解封日期: %d月%d日\n"
		 "如有异议，可向版主提出，或到Appeal版投诉\n",
		 exp, dt, tmtime->tm_mon + 1, tmtime->tm_mday);
	fputs(buf, fp);
	fclose(fp);
	securityreport(title, buf);
	post_article(board, title, path, "XJTU-XANET", "自动发信系统",
		     "自动发信系统", -1, 4, 0, "XJTU-XANET", -1);//8 bit 0->4 bjgyt
	post_mail(user, title, path, currentuser.userid, currentuser.username,
		  fromhost, -1, 0);
	unlink(path);
	printf("系统已经发信通知了%s.<br>\n", user);
	return 0;
}
