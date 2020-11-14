#include "bbslib.h"

static void show_xbanner(void);
static void show_xsec(const struct sectree *sec);
static void show_xboards(const char *secstr);
static void show_xsec_boards(struct boardmem *(data[]), int total);
int chm=0;

static void
show_xbanner()
{
	printf("&nbsp;</td></tr></table></td></tr></table>\n");
	printf("%s", "<table width=\"100%\" border=0 cellpadding=0 cellspacing=0>\n"
	"<tr><td height=30></td></tr>\n"
	"<tr><td height=70>\n"
	"<table width=\"100%\" height=\"100%\" border=0 cellpadding=0 cellspacing=0 class=\"level2\">\n"
	"<tr><td><td> <div id=\"bmy\"><span class=\"hidden\">兵马俑</span></div> </td></td>\n"
	"<td width=290><table border=0 cellpadding=0 cellspacing=0>\n"
	"<tr>");
	printf("<form action=bbsx target=f3><td colspan=2><input name=chm type=hidden value=%d>\n"
	"<input name=Submit type=submit class=sumbitgrey value=\"下载%s格式\" title=\"点击切换下载文件格式\">\n"
	"</td>\n</form>", chm?0:1, chm?"tgz":"chm");
	printf("</tr></table></td>\n"
	"<td align=right>&nbsp;</td></tr></table></td></tr></table>\n");
	return;
}

static void
show_xsec(const struct sectree *sec)
{
	int i;
	for (i = 0; i < sec->nsubsec; i++) {
		printf("<tr>");
		printf("<td><div class=\"linediv\"><a href=boa?secstr=%s class=linkboardtheme>"
				"%s</a></div></td>\n", sec->subsec[i]->basestr,
				nohtml(sec->subsec[i]->title));
		printf("<td rowspan=2 align=right valign=bottom><a href=boa?secstr=%s class=linkbigtheme>%s</a></td></tr>\n",
			sec->subsec[i]->basestr, sec->subsec[i]->basestr);
		show_xboards(sec->subsec[i]->basestr);
	}
	return;
}

static int show_xboards_callback(struct boardmem *board, int curr_idx, va_list ap) {
	struct boardmem **data = va_arg(ap, struct boardmem **);
	int *total = va_arg(ap, int *);
	int hasintro = va_arg(ap, int);
	const char *secstr = va_arg(ap, const char *);

	int len = strlen(secstr);
	if (board->header.filename[0] <= 32 || board->header.filename[0] > 'z')
		return 0;

	if (hasintro) {
		if (strcmp(secstr, board->header.sec1) && strcmp(secstr, board->header.sec2))
			return 0;
	} else {
		if (strncmp(secstr, board->header.sec1, len) && strncmp(secstr, board->header.sec2, len))
			return 0;
	}

	if (!has_read_perm_x(&currentuser, board))
		return 0;

	data[*total] = board;
	*total = *total + 1;
	return 0;
}

static void
show_xboards(const char *secstr)
{
	struct boardmem *(data[MAXBOARD]);
	int hasintro = 0;
	int total = 0;
	const struct sectree *sec;

	sec = getsectree(secstr);
	if (sec->introstr[0])
		hasintro = 1;
	ythtbbs_cache_Board_foreach_v(show_xboards_callback, data, &total, hasintro, secstr);
	show_xsec_boards(data, total);
}

static void
show_xsec_boards(struct boardmem *(data[]), int total)
{
	int i;
	printf("<tr><td>\n");
	for(i=0; i<total; i++){
		printf("<a href=bbsx/%s.%s>%s</a> ",
			data[i]->header.filename,
			chm ? "chm":"tgz",
			data[i]->header.title);
	}
	printf("</td></tr>\n");
}

int
bbsx_main()
{

	char path[256];
	struct boardmem *x1;
	char *ptr, *board;

	board = getsenv("SCRIPT_URL");
	board = strrchr(board , '/')+1;

	if (strstr(board, ".tgz") && !strstr(board, "bbsx"))
	{
		if (!loginok || isguest) {
			http_fatal("请先登陆再下载");
			return 0;
		}
		ptr=strstr(board, ".tgz");
		*ptr=0;
		if (!board[0]){
			http_fatal("错误的参数");
			return -1;
		}
		x1 = getboard(board);
		if (!x1){
			http_fatal("错误的文件名");
			return -1;
		}
		sprintf(path, MY_BBS_HOME "/0Announce%s/%s.tgz", anno_path_of(board), board);
		if (!file_exist(path)){
			http_fatal("文件不存在");
			return -1;
		}
		printf("Content-type: application/tgz\n\n");
		showfile(path);
		return 0;
	}else if (!strstr(board, ".tgz") && !strstr(board, "bbsx")){
		http_fatal("错误的文件名");
		return -1;
	}

	chm = atoi(getparm("chm"));

	const struct sectree *sec;
	sec = getsectree("?");
	html_header(1);
	check_msg();
	changemode(SELECT);
	printf("<script src=\"/inc/tog.js\"></script></head><body leftmargin=0 topmargin=0>\n");
	show_xbanner();
	printf("%s", "<table width=75% border=0 cellpadding=0 cellspacing=0>\n");
	show_xsec(sec);
	printf("</table>\n<br></td>\n");
	printf("</body></html>");
	return 0;
}

