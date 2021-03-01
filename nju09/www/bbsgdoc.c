#include "bbslib.h"

extern void nosuchboard(char *board, char *cginame);
extern void printboardtop(struct boardmem *x);
extern int getdocstart(int total, int lines);

int
bbsgdoc_main()
{	//modity by macintosh 050516 for new www
	FILE *fp;
	char board[80], buf[128];
	struct boardmem *x1;
	struct fileheader x;
	int i, start, total;
	html_header(1);
	check_msg();
	printf("<script src=/function.js></script>\n");
	changemode(READING);
	ytht_strsncpy(board, getparm2("B", "board"), 32);
	x1 = getboard(board);
	if (x1 == 0)
		nosuchboard(board, "bbsgdoc");
	updateinboard(x1);
	strcpy(board, x1->header.filename);
	sprintf(buf, "boards/%s/.DIGEST", board);
	fp = fopen(buf, "r");
	if (fp == 0)
		http_fatal("错误的讨论区目录");
	total = file_size(buf) / sizeof (struct fileheader);
	start = getdocstart(total, w_info->t_lines);
/*	printf("<body topmargin=0>");
	printf("<nobr><center>\n"); */
	printf("<body topmargin=0 leftmargin=0>\n");
	printf("<table width=\"100%%\" border=0 cellpadding=0 cellspacing=0>\n"
		"<td><form name=form1 action=bbsgdoc>\n");
	printboardtop(x1);

/*	printf("文章数[%d] ", total);
	printf("<a href=bbspst?board=%s>发表文章</a> ", board); */
	printf("<tr><td><a href=\"pst?B=%s\" class=\"btnsubmittheme\" title=\"发表文章 accesskey: p\" accesskey=\"p\">发表文章</a>\n", board);
	printf("文章数&lt;%d&gt; 在线&lt;%d&gt;</td>", total, x1->inboard);
	sprintf(buf, "bbsgdoc?board=%s", board);
//	bbsdoc_helper(buf, start, total, w_info->t_lines);
	printf("<td align=right><a href=\"gdoc?B=%s&S=%d\" title=\"第一页 accesskey: 1\" accesskey=\"1\">第一页</a>\n", board, 1);
	if(start > w_info->t_lines+1) printf("<a href=\"gdoc?B=%s&S=%d\" title=\"上一页 accesskey: f\" accesskey=\"f\">上一页</a>\n", board, (start-w_info->t_lines));
	if(start < total-w_info->t_lines+1) printf("<a href=\"gdoc?B=%s&S=%d\" title=\"下一页 accesskey: n\" accesskey=\"n\">下一页</a>\n", board, (start+w_info->t_lines));
	printf("<a href=\"gdoc?B=%s&S=%d\" title=\"最后一页 accesskey: l\" accesskey=\"l\">最后一页</a>\n", board, (total-w_info->t_lines+1));
	//add by macintosh 050519 for func "Go"
	printf("<input type=hidden name=B value=%s>", board);
	printf("<input name=Submit1 type=Submit class=sumbitgrey value=Go>\n"
		"<input name=S type=text style=\"font-size:11px;font-family:verdana;\" size=4></td>\n"
		"</tr></table></td></tr>\n");
	printf("</form></td>");
	if (NULL == fp)
		http_fatal("本讨论区目前没有文章");
	if (total <= 0) {
		fclose(fp);
		http_fatal("本讨论区文摘目前没有文章");
	}

/*	printhr();
	printf("<table border=0>\n");
	printf("<tr><td>序号</td><td>状态</td><td>作者</td><td>日期</td><td>标题</td><td>星级</td><td>评价</td></tr>\n");
*/
	printf("%s", "<tr><td width=40 class=\"level1\">&nbsp;</td>\n"
		"<td class=\"level1\"><TABLE width=\"95%\" cellpadding=2 cellspacing=0>\n"
		"<TBODY>\n");
	printf("%s", "<TR>\n"
		"<TD class=tdtitle>序号</TD>\n"
		"<TD class=tdtitle>状态</TD>\n"
		"<TD class=tduser>作者</TD>\n"
		"<TD align=center class=tdtitle>日期</TD>\n"
		"<TD align=center class=tdtitle>标题</TD>\n"
		"<TD class=tdtitle>星级</TD>\n"
		"<TD class=tdtitle>评价</TD>\n"
		"</TR>\n");

	fseek(fp, (start - 1) * sizeof (struct fileheader), SEEK_SET);
	for (i = 0; i < w_info->t_lines; i++) {
		if (fread(&x, sizeof (x), 1, fp) <= 0)
			break;
		x.owner[sizeof(x.owner) - 1] = 0;
		printf("<tr><td class=tdborder>%d</td><td class=tdborder>%s</td><td class=tduser>%s</td>", start + i, flag_str(x.accessed), userid_str(x.owner));
		printf("<td align=center class=tdborder>%12.12s</td>", ytht_ctime(x.filetime) + 4);
		printf("<td class=tdborder><a href=bbsgcon?board=%s&file=%s&num=%d>%s%s</a></td><td class=tdborder>%d</td><td class=tdborder>%d人</td></tr>\n", board, fh2fname(&x), start + i - 1, strncmp(x.title, "Re: ", 4) ? "● " : "", void1(titlestr(x.title)), x.staravg50 / 50, x.hasvoted);
	}
	printf("</TR> </TBODY></TABLE></td></tr>\n");
/*	printhr();
	printf("文章数[%d] ", total);
	printf("<a href=bbspst?board=%s>发表文章</a> ", board);
	bbsdoc_helper(buf, start, total, w_info->t_lines);
*/
	printf("<tr><td height=40 class=\"level1\">&nbsp;</td>\n"
		"<td height=40 class=\"level1\">\n"
		"<table width=\"95%%\" height=\"100%%\"  border=0 cellpadding=0 cellspacing=0 class=\"level1\">\n"
		"<td><form name=form2 action=bbsgdoc>\n");
	printf("<tr><td><a href=\"pst?B=%s\" class=\"btnsubmittheme\" title=\"发表文章 accesskey: p\" accesskey=\"p\">发表文章</a>\n", board);
	printf("文章数&lt;%d&gt; 在线&lt;%d&gt;</td>", total, x1->inboard);
	sprintf(buf, "bbsgdoc?board=%s", board);
	printf("<td align=right><a href=\"gdoc?B=%s&S=%d\" title=\"第一页 accesskey: 1\" accesskey=\"1\">第一页</a>\n", board, 1);
	if(start > w_info->t_lines+1) printf("<a href=\"gdoc?B=%s&S=%d\" title=\"上一页 accesskey: f\" accesskey=\"f\">上一页</a>\n", board, (start-w_info->t_lines));
	if(start < total-w_info->t_lines+1) printf("<a href=\"gdoc?B=%s&S=%d\" title=\"下一页 accesskey: n\" accesskey=\"n\">下一页</a>\n", board, (start+w_info->t_lines));
	printf("<a href=\"gdoc?B=%s&S=%d\" title=\"最后一页 accesskey: l\" accesskey=\"l\">最后一页</a>\n", board, (total-w_info->t_lines+1));
	//add by macintosh 050519 for func "Go"
	printf("<input type=hidden name=B value=%s>", board);
	printf("<input name=Submit2 type=Submit class=sumbitgrey value=Go>\n"
		"<input name=S type=text style=\"font-size:11px;font-family:verdana;\" size=4></td>\n"
		"</tr></table></td></tr>\n");
	printf("</form></td>");
	fclose(fp);
//	printdocform("bbsgdoc", board);
	printf("</table></td></tr></table></body>\n");
	http_quit();
	return 0;
}
