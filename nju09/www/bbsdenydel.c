#include "bbslib.h"

static int inform(char *board, char *user);

int
bbsdenydel_main()
{
	int i;
	char board[80], *userid;
	struct boardmem *x;
	html_header(1);
	check_msg();
	if (!loginok)
		http_fatal("您尚未登录, 请先登录");
	changemode(READING);
	ytht_strsncpy(board, getparm("board"), 30);
	if (!(x = getboard(board)))
		http_fatal("错误的讨论区");
	if (!has_BM_perm(&currentuser, x))
		http_fatal("你无权进行本操作");
	loaddenyuser(board);
	userid = getparm("userid");
	for (i = 0; i < denynum; i++) {
		if (!strcasecmp(denyuser[i].id, userid)) {
			denyuser[i].id[0] = 0;
			savedenyuser(board);
			printf("已经给 %s 解封. <br>\n", userid);
			inform(board, userid);
			printf("[<a href=bbsdenyall?board=%s>返回被封名单</a>]", board);
			http_quit();
		}
	}
	http_fatal("这个用户不在被封名单中");
	http_quit();
	return 0;
}

static int
inform(char *board, char *user)
{
	FILE *fp;
	char path[80], title[80], buf[256];
	sprintf(title, "恢复 %s 在 %s 的POST权", user, board);
	sprintf(path, "bbstmpfs/tmp/%d.tmp", thispid);
	fp = fopen(path, "w");
	fprintf(fp, "【此篇文章是由自动发信系统所张贴】\n\n");
	snprintf(buf, sizeof (buf), "%s 恢复了 %s %s版POST权.\n"
			"请理解版务管理工作,谢谢!\n", currentuser.userid, user, board);
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
