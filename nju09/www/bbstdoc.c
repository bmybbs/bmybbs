#include "bbslib.h"
#include "tmpl.h"
static char *stat1(struct fileheader *data, int from, int total);

/* bbsdoc.c */
extern void nosuchboard(char *board, char *cginame);
extern int getdocstart(int total, int lines);
extern void printboardtop(struct boardmem *x, int num);
extern void printboardhot(struct boardmem *x);
extern int top_file(const char *call_type);
extern int printkeywords(char* keywordstr);
extern void printrelationboards(char *buf);

int
bbstdoc_main()
{
	char board[80], buf[128],only_for_b[80],genbuf[STRLEN];
	FILE *fp;
	struct boardmem *x1;
	struct fileheader *data = NULL;
	int i, start = 0, total2 = 0, total = 0, sum = 0, fd, size;
	changemode(READING);
	ytht_strsncpy(board, getparm("B"), 32);
	if (!board[0])
		ytht_strsncpy(board, getparm("board"), 32);
	x1 = getboard(board);
	if (x1 == 0) {
		html_header(1);
		nosuchboard(board, "bbstdoc");
	}
	updateinboard(x1);
	strcpy(board, x1->header.filename);
	sprintf(buf, "boards/%s/.DIR", board);
	int hastmpl;
	sprintf(genbuf, "boards/%s/%s", board, ".tmpl");
	if ((fp=fopen(genbuf, "r")) == 0)
		hastmpl = 0;
	else{
		hastmpl = file_size(genbuf) / sizeof (struct a_template);
		fclose(fp);
	}
//	if(cache_header(file_time(buf),10))
//			return 0;
	html_header(1);
	check_msg();
	size = file_size(buf);
	if (!size)
		http_fatal("本讨论区目前没有文章");
	fd = open(buf, O_RDONLY);
	if (fd < 0)
		http_fatal("本讨论区目前没有文章");
	MMAP_TRY {
		data = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
		close(fd);
		if (data == (void *) -1) {
			MMAP_UNTRY;
			http_fatal("无法读取文章列表");
		}
		total = size / sizeof (struct fileheader);
		for (i = 0; i < total; i++)
			if (data[i].thread == data[i].filetime)
				total2++;
		start = getdocstart(total2, w_info->t_lines);
		//printf("<nobr><center>\n");
// 	lanboy add next line
		printf("<body topmargin=0 leftmargin=0>\n");
		printf("<table width=\"100%%\" border=0 cellpadding=0 cellspacing=0>\n"
				"<form name=form1 action=bbstdoc>");
// 	lanboy
		printboardtop(x1, 3);
		printf("<tr><td><a href=\"pst?B=%s\" class=\"btnsubmittheme\" title=\"我要发表文章 accesskey: p\" accesskey=\"p\">我要发表文章</a>\n", board);
		if (hastmpl > 0)
				printf("<a href=bbstmpl?action=show&board=%s class=\"btnsubmittheme\" title=\"模板发文 accesskey: t\" accesskey=\"t\">模板发文</a>\n", board);
		printf("文章数[%d] 主题数[%d] 在线[%d] </td>", total, total2, x1->inboard);
		printf("<td align=right><a href=bbsdoc?board=%s>一般模式</a>  ", board);
		if (has_BM_perm(&currentuser, x1))
			printf("<a href=mdoc?B=%s>管理模式</a> ", board);
		sprintf(buf, "bbstdoc?board=%s", board);
		//bbsdoc_helper(buf, start, total2, w_info->t_lines);
		printf("<a href=# onclick='javascript:{location=location;return false;}'>刷新</a> ");
		printf("<a href=\"bbstdoc?B=%s&S=%d\">第一页</a>\n", board, 1);
		if(start > 1) printf("<a href=\"bbstdoc?B=%s&S=%d\">上一页</a>\n", board, (start-w_info->t_lines));
		if(start < total2 - w_info->t_lines+1) printf("<a href=\"bbstdoc?B=%s&S=%d\">下一页</a>\n", board, (start+w_info->t_lines));
		printf("<a href=\"bbstdoc?B=%s&S=%d\">最后一页</a>\n", board, (total-w_info->t_lines+1));
		if (total <= 0) {
			munmap(data, size);
			MMAP_UNTRY;
			http_fatal("本讨论区目前没有文章");
		}
		printf("<input type=hidden name=B value=%s>", board);
		printf("<input name=Submit2 type=Submit class=sumbitgrey value=Go>\n"
				"<input name=start type=text style=\"font-size:11px;font-family:verdana;\" size=4>");
		//add by liuche 20120206 for pagenumber
		if((start-1)%w_info->t_lines==0)
		printf(" Page: %d/%d\n",(start-1)/w_info->t_lines+1,(total-1)/w_info->t_lines+1);
		else
		printf(" Page: %d/%d\n",(start-1)/w_info->t_lines+2,(total-1)/w_info->t_lines+1);

		//printhr();
		printf("</td></tr></table></td></tr></table></td></tr></form></table>\n");
		printboardhot(x1); //显示版面热门话题
		printf("<table width=\"95%%\" cellpadding=2 cellspacing=0 align=\"center\"><tr>\n"
				"<td class=tdtitle>序号</td>"
				"<td class=tdtitle>状态</td>"
				"<td class=tduser>作者</td>"
				"<td align=center class=tdtitle>日期</td>"
				"<td align=center class=tdtitle>标题</td>"
				"<td class=tdtitle>回帖/推荐度</td>\n"
				"</tr>");
		//top_file();
		for (i = 0; i < total; i++) {
			//判断是否有b标记
			if(data[i].accessed & FH_ALLREPLY)
				strcpy(only_for_b,"style='color:red;' ");
			else
				strcpy(only_for_b,"");
			//

			if (data[i].thread != data[i].filetime)
				continue;
			sum++;
			if (sum < start)
				continue;
			if(sum%2)
				printf("<tr class='d0'>");
			else
				printf("<tr>");
			printf("<td class=tdborder>%d</td>"	// 序号
					"<td class=tdborder>%s</td>"		// 状态
					"<td class=tduser>%s</td>",		// 作者
					sum, flag_str(data[i].accessed)[0] == ' ' ? "&nbsp;" : flag_str(data[i].accessed),
					userid_str(fh2owner(&data[i])));
			printf("<td align=center class=tdborder>%6.6s</td>", ytht_ctime(data[i].filetime) + 4); // 日期
			printf("<td class=tdborder><a href=bbstcon?board=%s&start=%d&th=%ld %s>○ %s </a></td><td class=tdborder>%s</td>\n",
					board, i, data[i].thread, only_for_b,void1(titlestr(data[i].title)),
					stat1(data, i, total));
			if (sum > start + w_info->t_lines - 2)
				break;
		}
		top_file("bbstdoc");
	}
	MMAP_CATCH {
		close(fd);
	}
	MMAP_END munmap(data, size);
	printf("<tr><td height=40 class=\"level1\"><TD class=level1 width=40>&nbsp;</TD>\n"
		"<table width=\"95%%\"  border=0 cellpadding=0 cellspacing=0 class=\"level1\" align=\"center\">\n"
		"<td><form name=form2 action=bbstdoc>");
	printf("<tr><td><a href=\"pst?B=%s\" class=btnsubmittheme>我要发表文章</a>\n", board);
	if (hastmpl > 0)
		printf("<a href=bbstmpl?action=show&board=%s class=btnsubmittheme>模板发文</a>\n", board);
	printf("文章数[%d] 主题数[%d] 在线[%d] </td>", total, total2, x1->inboard);
	printf("<td align=\"right\">\n"
		"<a href=\"home?B=%s\">一般模式</a>\n", board);
	if (has_BM_perm(&currentuser, x1))
		printf("<a href=mdoc?B=%s>管理模式</a> ", board);
	printf("<a href=# onclick='javascript:{location=location;return false;}'>刷新</a> ");
	printf("<a href=\"bbstdoc?B=%s&S=%d\">第一页</a>\n", board, 1);
	if(start > 1) printf("<a href=\"bbstdoc?B=%s&S=%d\">上一页</a>\n", board, (start-w_info->t_lines));
	if(start < total2 - w_info->t_lines+1) printf("<a href=\"bbstdoc?B=%s&S=%d\">下一页</a>\n", board, (start+w_info->t_lines));
	printf("<a href=\"bbstdoc?B=%s&S=%d\">最后一页</a>\n", board, (total-w_info->t_lines+1));
	printf("<input type=hidden name=B value=%s>", board);
	printf("<input name=Submit2 type=Submit class=sumbitgrey value=Go>\n"
		"<input name=start type=text style=\"font-size:11px;font-family:verdana;\" size=4>");
	if((start-1)%w_info->t_lines==0)
	printf(" Page: %d/%d\n",(start-1)/w_info->t_lines+1,(total-1)/w_info->t_lines+1);
	else
	printf(" Page: %d/%d\n",(start-1)/w_info->t_lines+2,(total-1)/w_info->t_lines+1);
	printf("</td></tr></form></table>");
	sprintf(buf, "%s", ytht_strtrim(x1->header.keyword));
	if (strlen(buf)){
		printf("<table width=\"100%%\" cellpadding=2 cellspacing=0><tr><td class=tdtitle align=center>\n");
		printf("本版关键字: ");
		printkeywords(buf);
		printf("</td></tr></table>\n");
	}

	sprintf(genbuf, "boards/%s/boardrelation", board);
	fp = fopen(genbuf, "r");
	if (fp != NULL) {
		char linebuf[128];
		fgets(linebuf, 128, fp);
		printf("<table width=\"100%%\" cellpadding=2 cellspacing=0><tr><td class=tdtitle align=center>\n");
		printf("来这个版的朋友也常去这些版面: ");
		printrelationboards(linebuf);
		printf("</td></tr></table>\n");
		fclose(fp);
	}
	//printhr();
	//printf("主题模式 文章数[%d] 主题数[%d] ", total, total2);
	//printf("<a href=bbsdoc?board=%s>一般模式</a>&nbsp;", board);
	//printf("<a href=bbspst?board=%s>发表文章</a> ", board);
	//sprintf(buf, "bbstdoc?board=%s", board);
	//bbsdoc_helper(buf, start, total2, w_info->t_lines);
	//printdocform("bbstdoc", board);
	printf("<script src=/function.js></script>\n"); // 尾部加载 js by IronBlood@bmy 20120720
	http_quit();
	return 0;
}

static char *stat1(struct fileheader *data, int from, int total) {
	static char buf[256];
	char *ptr = data[from].title;
	int i, re = 0, click = data[from].staravg50 * data[from].hasvoted / 50;
	for (i = from; i < total; i++) {
		if (!strncmp(ptr, data[i].title + 4, 40)) {
			re++;
			click += data[i].staravg50 * data[i].hasvoted / 50;
		}
	}
	sprintf(buf, "<font class=%s>%d</font>/<font class=%s>%d</font>",
		re > 9 ? "c31" : "c37", re, click ? "c31" : "c37", click);
	return buf;
}
