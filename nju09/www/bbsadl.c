#include "bbslib.h"

static int bbsadl_callback(struct boardmem *board, int curr_idx, va_list ap) {
	int *no = va_arg(ap, int *);
	char buf[256];
	snprintf(buf, sizeof(buf), "www/an/%s.tgz", board->header.filename);
	if (!file_exist(buf))
		return 0;
	*no = *no + 1;
	printf("<tr><td>%d<td><a href=/an/%s.tgz>%s.tgz</a><td>%ld<td>%s\n",
			*no, board->header.filename, board->header.filename, file_size(buf), ytht_ctime(file_time(buf)) + 4);
	return 0;
}

int
bbsadl_main()
{
	int no = 0;
	html_header(1);
	check_msg();
	printf("<nobr><center>%s -- 精华区下载服务<hr>\n", BBSNAME);
	printf("<table>\n");
	printf("<tr><td>序号<td>名称<td>大小(字节)<td>更新时间\n");
	ythtbbs_cache_Board_foreach_v(bbsadl_callback, no);
	printf("</table>");
	return 0;
}

