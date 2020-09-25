#include "bbslib.h"

int
bbsfwd_main()
{
	struct fileheader *x = NULL;
	char board[80], file[80], target[80], dir[80];
	int num;
	struct mmapfile mf = { ptr:NULL };
	html_header(1);
	check_msg();
	ytht_strsncpy(board, getparm("B"), 32);
	if (!board[0])
		ytht_strsncpy(board, getparm("board"), 32);
	ytht_strsncpy(file, getparm("F"), 30);
	if (!file[0])
		ytht_strsncpy(file, getparm("file"), 30);
	ytht_strsncpy(target, getparm("target"), 30);
	if (!loginok || isguest)
		http_fatal("匆匆过客不能进行本项操作");
	changemode(SMAIL);
	if (!getboard(board))
		http_fatal("错误的讨论区");
	sprintf(dir, "boards/%s/.DIR", board);
	MMAP_TRY {
		if (mmapfile(dir, &mf) == -1) {
			MMAP_UNTRY;
			http_fatal("错误的讨论区");
		}
		num = -1;
		x = findbarticle(&mf, file, &num, 1);
	}
	MMAP_CATCH {
		x = NULL;
	}
	MMAP_END mmapfile(NULL, &mf);
	if (x == 0)
		http_fatal("错误的文件名");
	printf("<center>%s -- 转寄/推荐给好友 [使用者: %s]<hr>\n", BBSNAME,
	       currentuser.userid);
	if (target[0]) {
		if (!strstr(target, "@")) {
			if (!getuser(target))
				http_fatal("错误的使用者帐号");
			strcpy(target, getuser(target)->userid);
		}
		return do_fwd(x, board, target);
	}
	printf("<table><tr><td>\n");
	printf("文章标题: %s<br>\n", nohtml(x->title));
	printf("文章作者: %s<br>\n", fh2owner(x));
	printf("原讨论区: %s<br>\n", board);
	printf("<form action=bbsfwd method=post>\n");
	printf("<input type=hidden name=board value=%s>", board);
	printf("<input type=hidden name=file value=%s>", file);
	printf
	    ("把文章转寄给 <input name=target size=30 maxlength=30 value=%s> (请输入对方的id或email地址). <br>\n",
	     currentuser.email);
	printf("<input type=submit value=确定转寄></form>");
	return 0;
}

int
do_fwd(struct fileheader *x, char *board, char *target)
{
	char title[512], path[200];
	sprintf(path, "boards/%s/%s", board, fh2fname(x));
	if (!file_exist(path))
		http_fatal("文件内容已丢失, 无法转寄");
	sprintf(title, "[转寄] %s", x->title);
	title[60] = 0;
	post_mail(target, title, path, currentuser.userid, currentuser.username,
		  fromhost, -1, 0);
	printf("文章已转寄给'%s'<br>\n", nohtml(target));
	printf("[<a href='javascript:history.go(-2)'>返回</a>]");
	return 0;
}
