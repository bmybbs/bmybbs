#include "bbslib.h"

extern bool nju09_mybrd_has_read_perm(const char *userid, const char *boardname);

int
bbsbrdadd_main()
{
	FILE *fp;
	char file[200], board[200];
	struct boardmem *b;
	int i;
	html_header(1);
	changemode(ZAP);
	ytht_strsncpy(board, getparm("B"), 32);
	if (!board[0])
		ytht_strsncpy(board, getparm("board"), 32);
	if (!loginok)
		http_fatal("超时或未登录，请重新login");
	readmybrd(currentuser.userid);
	if (g_GoodBrd.num >= GOOD_BRD_NUM)
		http_fatal("您预定讨论区数目已达上限，不能增加预定");
	if (ythtbbs_mybrd_exists(&g_GoodBrd, board))
		http_fatal("你已经预定了这个讨论区");
	b = getboard(board);
	if (!b)
		http_fatal("此讨论区不存在");
	ythtbbs_mybrd_append(&g_GoodBrd, b->header.filename);
	ythtbbs_mybrd_save(currentuser.userid, &g_GoodBrd, nju09_mybrd_has_read_perm);
	//printf("<script>top.f2.location='bbsleft?t=%d'</script>\n", now_t);
	printf("<script>top.f2.location.reload();</script>\n");
	printf("预定讨论区成功<br><a href='javascript:history.go(-1)'>快速返回</a>");
	http_quit();
	return 0;
}
