#include "bbslib.h"

extern bool nju09_mybrd_has_read_perm(const char *userid, const char *boardname);

int
bbsbrddel_main()
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
	if (g_GoodBrd.num == 0)
		http_fatal("您没有预定任何讨论区");
	if (!ythtbbs_mybrd_exists(&g_GoodBrd, board))
		http_fatal("你并没有预定这个讨论区");
	ythtbbs_mybrd_remove(&g_GoodBrd, board);
	ythtbbs_mybrd_save(currentuser.userid, &g_GoodBrd, nju09_mybrd_has_read_perm);
	printf("<script>top.f2.location.reload();</script>\n");
	printf("移除预定讨论区成功<br><a href='javascript:history.go(-1)'>快速返回</a>");
	http_quit();
	return 0;
}
