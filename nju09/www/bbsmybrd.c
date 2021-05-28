#include "bbslib.h"

static int read_submit();
//secstr == NULL: all boards
//secstr == "": boards that doesnn't belong to any group
static int showlist_callback(struct boardmem *board, int curr_idx, va_list ap) {
	(void) curr_idx;
	struct boardmem **data = va_arg(ap, struct boardmem **);
	int *total = va_arg(ap, int *);
	const char *secstr = va_arg(ap, const char *);

	int len = 0;
	if (secstr) {
		len = strlen(secstr);
		if (len == 0)
			len = 1;
	}

	if (secstr
			&& strncmp(board->header.sec1, secstr, len)
			&& strncmp(board->header.sec2, secstr, len))
		return 0;

	if (has_read_perm_x(&currentuser, board)) {
		data[*total] = board;
		*total = *total + 1;
	}

	return 0;
}

static void
showlist_alphabetical(const char *secstr, int needrss)
{
	struct boardmem *(data[MAXBOARD]);
	int i, total = 0;

	ythtbbs_cache_Board_foreach_v(showlist_callback, data, &total, secstr);

	if (!total)
		return;

	qsort(data, total, sizeof (struct boardmem *), (void *) cmpboard);
	printf("<table>\n");

	if (needrss) {
		for (i = 0; i < total; i++) {
			if (i % 3 == 0)
				printf("\n<tr>");
			printf("<td class=tdborder><a href=%s%s>%s(%s)</a>&nbsp;<a href=\"http://bbs.xjtu.edu.cn/BMYGXTBBUYRMZQTLAGFIFGNMJULDMBECHGGV_B/rss?board=%s\" target=\"blank\"><img  src=\"/images/rss.gif\" border=\"0\" />\n", showByDefMode(), data[i]->header.filename, data[i]->header.filename, data[i]->header.title, data[i]->header.filename);
		}
		printf("</table><br>\n");
	} else {
		for (i = 0; i < total; i++) {
		char *buf3 = "";
		if (ythtbbs_mybrd_exists(&g_GoodBrd, data[i]->header.filename))
			buf3 = " checked";
		if (i % 3 == 0)
			printf("\n<tr>");
		printf("<td class=tdborder><input type=checkbox name=%s %s><a href=%s%s>%s(%s)</a>&nbsp;<a href=\"http://bbs.xjtu.edu.cn/BMYGXTBBUYRMZQTLAGFIFGNMJULDMBECHGGV_B/rss?board=%s\" target=\"blank\"><img  src=\"/images/rss.gif\" border=\"0\" />\n",
				data[i]->header.filename, buf3, showByDefMode(), data[i]->header.filename,
				data[i]->header.filename, data[i]->header.title, data[i]->header.filename);
		}
		printf("</table><br>\n");
	}
}

static void
showlist_grouped(int needrss)
{
	int i;
	for (i = 0; i < sectree.nsubsec; i++) {
		printf("<b>%s</b>", nohtml(sectree.subsec[i]->title));
		printf("<hr>\n");
		showlist_alphabetical(sectree.subsec[i]->basestr, needrss);
	}
}

int
bbsmybrd_main()
{	//modify by mintbaggio 20040829 for new www
	int type, mode, mybrdmode;
	char buf[80];
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("尚未登录或者超时");
	changemode(ZAP);
	type = atoi(getparm("type"));
	if (type != 0) {
		read_submit();
		http_quit();
	}
	readmybrd(currentuser.userid);
	//printf("<style type=text/css>A {color: 000080} </style>\n");
	//printf("<body><center>\n");

	mode = atoi(getparm("mode"));
	if (mode == 2) {
		printf("<body leftmargin=0 topmargin=0>\n");
		printf("<table width=100%% border=0 cellpadding=0 cellspacing=0>\n"
		"<tr><td height=30 colspan=2></td>\n"
		"</tr><tr><td height=70 colspan=2>\n"
		"<table width=100%% height=100%% border=0 cellpadding=0 cellspacing=0 class=\"level2\">\n");
		printf("<tr>\n<td width=40 rowspan=2>&nbsp; </td>\n");
		printf("<td height=35> %s &gt; <span id=\"topmenu_b\">个人RSS订阅管理</span></td>\n", MY_BBS_NAME);
		printf("<tr>\n<td width=40 class=\"level1\">&nbsp;</td>\n"
		"<td class=\"level1\"><br>\n");
		showlist_grouped(1);
		printf("</td></tr></table></td></tr></table></body>\n");
		http_quit();
		return 0;
	}

	printf("<body leftmargin=0 topmargin=0>\n");
	printf("<table width=100%% border=0 cellpadding=0 cellspacing=0>\n"
		"<tr><td height=30 colspan=2></td>\n"
		"</tr><tr><td height=70 colspan=2>\n"
		"<table width=100%% height=100%% border=0 cellpadding=0 cellspacing=0 class=\"level2\">\n");
	printf("<tr>\n<td width=40 rowspan=2>&nbsp; </td>\n");
	//printf("<div class=rhead>个人预定讨论区管理(您目前预定了<span class=h11>%d</span>个讨论区，最多可预定<span class=h11>%d</span>个)</div><br>\n", mybrdnum, GOOD_BRC_NUM);
	printf("<td height=35> %s &gt; <span id=\"topmenu_b\">个人预定讨论区管理</span>\n", MY_BBS_NAME);
	printf("[ 目前预定: <span class=\"smalltext\">%d</span>个 | 最多预定:</b> <span class=\"smalltext\">%d</span>个 ]</td>\n" , g_GoodBrd.num, GOOD_BRD_NUM);
	printf("</tr>\n <td height=35 valign=top><a href=\"bbsmybrd?mode=0\" class=\"btnsubmittheme\">按字母顺序排列</a>\n"
		"<a href=\"bbsmybrd?mode=1\" class=\"btnsubmittheme\">按分类排列</a></td></tr>\n");
	printf("<tr>\n<td width=40 class=\"level1\">&nbsp;</td>\n"
		"<td class=\"level1\"><br>\n"
		"<form action=bbsmybrd?type=1&confirm1=1 method=post>\n");
	printf("<input type=hidden name=confirm1 value=1>\n");

	if (mode == 0)
		showlist_alphabetical(NULL, 0);
	else
		showlist_grouped(0);

	readuservalue(currentuser.userid, "mybrdmode", buf, sizeof (buf));
	mybrdmode = atoi(buf);
	printf("<br>\n<input type=radio name=mybrdmode value='0' %s>预定讨论区显示中文描述 "
			"<input type=radio name=mybrdmode value='1' %s>预定讨论区显示英文名称<br>",
			mybrdmode ? "" : "checked", mybrdmode ? "checked" : "");
	printf("<input type=submit class=resetlong value=确认预定> <input type=reset class=sumbitlong value=复原>\n");
	printf("</form></td></tr></table></td></tr></table></body>\n");
	http_quit();
	return 0;
}

bool nju09_mybrd_has_read_perm(const char *userid, const char *boardname) {
	char buf[32]; // boardheader.filename char[24];
	(void) userid;
	strcpy(buf, boardname);
	return (getboard(buf) != NULL);
}

int readmybrd(char *userid) {
	ythtbbs_mybrd_load(userid, &g_GoodBrd, nju09_mybrd_has_read_perm);
	return 0;
}

static int read_submit() {
	int i;
	struct boardmem *x;
	g_GoodBrd.num = 0;
	int count = 0;
	if (!strcmp(getparm("confirm1"), ""))
		http_fatal("参数错误");
	for (i = 0; i < parm_num; i++) {
		if (!strcasecmp(parm_val[i], "on")) {
			if (ythtbbs_mybrd_exists(&g_GoodBrd, parm_name[i]))
				continue;
			if (g_GoodBrd.num >= GOOD_BRD_NUM)
				http_fatal("您试图预定超过%d个讨论区", GOOD_BRD_NUM);
			x = getboard(parm_name[i]);
			if (!x) {
				printf("警告: 无法预定'%s'讨论区<br>\n", nohtml(parm_name[i]));
				continue;
			}
			ythtbbs_mybrd_append(&g_GoodBrd, x->header.filename);
		}
	}
	count = ythtbbs_mybrd_save(currentuser.userid, &g_GoodBrd, nju09_mybrd_has_read_perm);
	char *mybrdmode = getparm("mybrdmode");
	saveuservalue(currentuser.userid, "mybrdmode", (mybrdmode != NULL && mybrdmode[0] == '0') ? "0" : "1");
	printf("<script>top.f2.location='bbsleft?t=%ld'</script>修改预定讨论区成功，您现在一共预定了%d个讨论区:<hr>\n", now_t, count);
	printf("[<a href='javascript:history.go(-2)'>返回</a>]");
	return 0;
}

