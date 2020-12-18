#include "bbslib.h"
#include "bmy/article.h"

static const int COUNT_PER_PAGE = 20;

int bbssub_main() {
	int start;
	char start_buf[80];
	size_t i;
	struct bmy_articles *articles;

	if (isguest || !loginok)
		http_fatal("请先登录");

	changemode(READING);
	ytht_strsncpy(start_buf, getparm("start"), sizeof(start_buf));
	start = atoi(start_buf);
	if (start < 0)
		start = 0;

	articles = bmy_article_list_subscription(currentuser.userid, COUNT_PER_PAGE, start);
	if (articles == NULL || articles->count == 0) {
		bmy_article_list_free(articles);
		http_fatal("没有记录");
	}

	// 输出，UTF8
	printf("Content-type: text/html; charset=utf-8\n\n");
	printf("<!doctype html>"
			"<html lang='zh'>"
			"<head><meta charset='UTF-8'><link rel='stylesheet' type='text/css' href='%s'></head>"
			"<body>", currstyle->cssfile);

	if (start == 0) {
		printf("<a href='sub?start=%d'>下一页</a>", COUNT_PER_PAGE);
	} else {
		printf("<a href='sub?start=%d'>上一页</a> / <a href='sub?start=%d'>下一页</a>", start - COUNT_PER_PAGE, start + COUNT_PER_PAGE);
	}

	printf("<table>");
	printf("<thead><tr><th>版面名称</th><th>时间</th><th>标题</th><th>作者</th><th>讨论数</th></tr></thead>");

	printf("<tbody>");
	for (i = 0; i < articles->count; i++) {
		printf("<tr>");
		printf("<td>%s</td>", articles->articles[i].boardname_zh);
		printf("<td>%12.12s</td>", ytht_ctime(articles->articles[i].thread));
		printf("<td>%s</td>", articles->articles[i].title);
		printf("<td>%s</td>", articles->articles[i].owner);
		printf("<td>%d</td>", articles->articles[i].count);
		printf("</tr>");
	}

	printf("</tbody></table></body></html>");
	bmy_article_list_free(articles);
	return 0;
}

