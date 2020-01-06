#include "bbslib.h"

int
bbsmdoc_main()
{
	FILE *fp;
	char board[80], dir[80];
	struct boardmem *x1;
	struct fileheader x;
	int i, start, total;
	char bmbuf[(IDLEN + 1) * 4];
	if (!loginok || isguest)
		http_fatal("请先登录");
	changemode(READING);
	strsncpy(board, getparm("B"), 32);
	if (!board[0])
		strsncpy(board, getparm("board"), 32);
	x1 = getboard(board);
	if (x1 == 0) {
		html_header(1);
		nosuchboard(board, "bbsmdoc");
	}
	updateinboard(x1);
	strcpy(board, x1->header.filename);
	if (!has_BM_perm(&currentuser, x1))
		http_fatal("您没有权限访问本页");
	sprintf(dir, "boards/%s/.DIR", board);
	if(cache_header(file_time(dir),10))
		return 0;
	html_header(1);
	check_msg();
	printf("<script src=/function.js></script>\n");
	
	fp = fopen(dir, "r");
	if (fp == 0)
		http_fatal("错误的讨论区目录");
	total = file_size(dir) / sizeof (struct fileheader);
	start = atoi(getparm("start"));
	if (strlen(getparm("start")) == 0 || start > total - 20)
		start = total - 20;
	if (start < 0)
		start = 0;
	printf("<nobr><center>\n");
	printf("%s -- [讨论区: %s] 版主[%s] 文章数[%d]<hr>\n",
	       BBSNAME, board, userid_str(bm2str(bmbuf, &(x1->header))), total);
	if (total <= 0) {
		fclose(fp);
		http_fatal("本讨论区目前没有文章");
	}
	printf("<form name=form1 method=post action=bbsman>\n");
	printf("<table>\n");
	printf("<tr><td>序号<td>管理<td>状态<td>作者<td>日期<td>标题\n");
	fseek(fp, start * sizeof (struct fileheader), SEEK_SET);
	for (i = 0; i < 20; i++) {
		char filename[80];
		if (fread(&x, sizeof (x), 1, fp) <= 0)
			break;
		sprintf(filename, "boards/%s/%s", board, fh2fname(&x));
		printf("<tr><td>%d", start + i + 1);
		printf
		    ("<td><input style='height:18px' name=box%s type=checkbox>",
		     fh2fname(&x));
		printf("<td>%s<td>%s", flag_str_bm(x.accessed),
		       userid_str(fh2owner(&x)));
		printf("<td>%12.12s", Ctime(x.filetime) + 4);
		x.title[40] = 0;
		printf("<td><a href=con?B=%s&F=%s&N=%d&T=%d>%s%s </a>\n",
		       board, fh2fname(&x), start + i + 1, feditmark(x),
		       strncmp(x.title, "Re: ", 4) ? "○ " : "",
		       void1(titlestr(x.title)));
	}
	fclose(fp);
	printf("</table>\n");
	printf("<input type=hidden name=mode value=''>\n");
	printf("<input type=hidden name=board value='%s'>\n", board);
	printf
	    ("<input type=button value=加删除标记 onclick="
	     "'if(confirm(\"请使用telnet方式进行区段删除，这里仅增加删除标记，你真的要加标记吗?\")) {document.form1.mode.value=1; document.form1.submit();}'>&nbsp;&nbsp;\n");
	printf
	    ("<input type=button value=加M onclick='document.form1.mode.value=2; document.form1.submit();'>\n");
	printf
	    ("<input type=button value=加G onclick='document.form1.mode.value=3; document.form1.submit();'>\n");
	printf
	    ("<input type=button value=不可Re onclick='document.form1.mode.value=4; document.form1.submit();'>\n");
	printf
	    ("<input type=button value=清除MG onclick='document.form1.mode.value=5; document.form1.submit();'>\n");
	printf("</form>\n");
	if (start > 0) {
		printf("<a href=bbsmdoc?board=%s&start=%d>上一页</a> ",
		       board, start < 20 ? 0 : start - 20);
	}
	if (start < total - 20) {
		printf("<a href=bbsmdoc?board=%s&start=%d>下一页</a> ",
		       board, start + 20);
	}
	printf("<a href=bbsdoc?board=%s>一般模式</a> ", board);
	printf("<a href=bbsdenyall?board=%s>封人名单</a> ", board);
	printf("<a href=bbsmnote?board=%s>编辑备忘录</a> ", board);
	printdocform("bbsmdoc", board);
	http_quit();
	return 0;
}
