#include "bbslib.h"

int
bbsbrdadd_main()
{
	FILE *fp;
	char file[200], board[200];
	int i;
	html_header(1);
	changemode(ZAP);
	ytht_strsncpy(board, getparm("B"), 32);
	if (!board[0])
		ytht_strsncpy(board, getparm("board"), 32);
	if (!loginok)
		http_fatal("超时或未登录，请重新login");
	readmybrd(currentuser.userid);
	if (mybrdnum >= GOOD_BRC_NUM)
		http_fatal("您预定讨论区数目已达上限，不能增加预定");
	if (ismybrd(board))
		http_fatal("你已经预定了这个讨论区");
	if (!getboard(board))
		http_fatal("此讨论区不存在");
	strcpy(mybrd[mybrdnum], board);
	mybrdnum++;
	sethomefile(file, currentuser.userid, ".goodbrd");
	fp = fopen(file, "w");
	if (fp) {
		for (i = 0; i < mybrdnum; i++)
			fprintf(fp, "%s\n", mybrd[i]);
		fclose(fp);
	} else
		http_fatal("Can't save");
	//printf("<script>top.f2.location='bbsleft?t=%d'</script>\n", now_t);
	printf("<script>top.f2.location.reload();</script>\n");
	printf
	    ("预定讨论区成功<br><a href='javascript:history.go(-1)'>快速返回</a>");
	http_quit();
	return 0;
}
