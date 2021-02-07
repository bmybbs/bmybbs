#include "bbslib.h"

int
bbsnewmail_main()
{	//modify by mintbaggio 20040829 for new www
	FILE *fp;
	struct fileheader x;
	int total = 0, total2 = 0;
	char dir[80];
	if (!loginok || isguest)
		http_fatal("您尚未登录, 请先登录");
	sprintf(dir, "mail/%c/%s/.DIR", mytoupper(currentuser.userid[0]),
		currentuser.userid);

	if(cache_header(file_time(dir),1))
		return 0;
	html_header(1);
	check_msg();
	changemode(RMAIL);
	printf("<body><center>\n");
	printf("<div class=rhead>%s -- 新邮件列表 [使用者: <span class=h11>%s</span>]<br>[信箱容量: <span class=h11>%d</span>k, 已用空间: <span class=h11>%dk</span>]</div><hr>\n",
			BBSNAME, currentuser.userid, max_mail_size(), get_mail_size());
	fp = fopen(dir, "r");
	if (fp == 0)
		http_fatal("目前您的信箱没有任何信件");
	printf("<table border=1>\n");
	printf("<tr><td>序号</td><td>状态</td><td>发信人</td><td>日期</td><td>信件标题</td></tr>\n");
	while (1) {
		if (fread(&x, sizeof (x), 1, fp) <= 0)
			break;
		total++;
		if (x.accessed & FH_READ)
			continue;
		printf("<tr><td>%d</td><td>N</td>", total);
		printf("<td>%s</td>", userid_str(fh2owner(&x)));
		printf("<td>%6.6s</td>", ytht_ctime(x.filetime) + 4);
		printf("<td><a href=bbsmailcon?file=%s&num=%d>", fh2fname(&x), total - 1);
		x.title[sizeof(x.title) - 1] = 0;
		if (strncmp("Re: ", x.title, 4))
			printf("★ ");
		hprintf("%42.42s", void1(x.title));
		printf(" </a></td></tr>\n");
		total2++;
	}
	fclose(fp);
	printf("</table><hr>\n");
	printf("您的信箱共有%d封信件, 其中新信%d封.", total, total2);
	printf("</center></body>");
	http_quit();
	return 0;
}
