#include "bbslib.h"
#include "check_server.h"

char *get_login_pic_link (char *picname, char *linkback);

int
checkfile(char *fn, int maxsz)
{
	char path[456];
	int sz;
	sprintf(path, HTMPATH "%s", fn);
	sz = file_size(path);
	if (sz < 100 || sz > maxsz)
		return -1;
	return 0;
}

int
showannounce()
{
	static struct mmapfile mf = { .ptr = NULL };
	if (mmapfile("0Announce/announce", &mf) < 0 || mf.size <= 10)
		return -1;
	printf("<table width=85%% border=1><tr><td>");
	fwrite(mf.ptr, mf.size, 1, stdout);
	printf("</table>");
	return 0;
}

void loginwindow()
{
	html_header(4);
	char loginpics[512];

	get_no_more_than_four_login_pics(loginpics, sizeof(loginpics));
	printf("<script>function openreg(){open('" SMAGIC
			"/bbsreg', 'winREG', 'width=600,height=460,resizable=yes,scrollbars=yes');}\n"
			"function sf(){document.l.id.focus();}\n"
			"function st(){document.l.t.value=(new Date()).valueOf();}\n"
			"function lg(){self.location.href='/" SMAGIC
			"/bbslogin?id=guest&t='+(new Date()).valueOf();}\n"
			"</script>\n");
	printf("<link href=\"/images/oras.css\" rel=stylesheet type=text/css>\n");
	// 欢迎光临
	printf("<title>\xBB\xB6\xD3\xAD\xB9\xE2\xC1\xD9 "MY_BBS_NAME"</title>");
	printf("<script type=\"text/javascript\" src=\"jquery-1.6.4.min.js\"></script>");
	printf("<script type=\"text/javascript\" src=\"showloginpics.min.js\"></script>");
	printf("<link type=\"text/css\" rel=\"Stylesheet\" href=\"showloginpics.css\" />");
	printf("<body bgcolor=#efefef leftmargin=0 topmargin=0 onload='document.l.id.focus();'>\n"
		"<table width=100%% height=100%% border=0 cellpadding=0 cellspacing=0>\n"
		"<tr>\n <td rowspan=4>&nbsp;</td>\n <td width=650 height=47> </td>\n"
		"<td rowspan=4>&nbsp;</td>\n  </tr>\n  <tr> \n"
		"<td height=264 valign=top bgcolor=#FFFFFF>\n"
		"<table width=780 border=0 align=center cellpadding=0 cellspacing=0 bgcolor=#FFFFFF>\n"
		// 匿名登录
		"<tr>\n<td height=35 align=right class=0004><a href='javascript:lg();' class=linkindex>\xC4\xE4\xC3\xFB\xB5\xC7\xC2\xBC</a>"
		// telnet登录
		"/ <a href='telnet://" MY_BBS_DOMAIN "' class=linkindex>telnet\xB5\xC7\xC2\xBC</a> /"
		// CTerm工具下载
		"<a href='CTerm.rar' class=linkindex>CTerm\xB9\xA4\xBE\xDF\xCF\xC2\xD4\xD8</a> /"
		// 新用户注册
		"<a href='javascript: openreg();' class=linkindex>\xD0\xC2\xD3\xC3\xBB\xA7\xD7\xA2\xB2\xE1</a><font color=#FFFFFF> \n"
		"/</font></font> </td>\n"
		"</tr>\n <tr> \n"
		"<td width=770 height=400><div id=\"container\"></div></td>" /* modified by IronBlood 11.09.08 */
		//"<td width=770 height=400 bgcolor=\"#ff6600\"><a href=%s><img src=%s alt=\"bmybbs\" border=0 width=770 height=400></img></a></td>"  /* modified by linux 05.9.11 */
		"</tr>\n"
		"<tr>\n"
		"<td height=20><div id=\"nav\"></div></td>\n" /* modified by IronBlood 11.12.25 */
		"</tr>\n"
		"<tr>\n"
		"<td height=35> <table border=0 align=center cellpadding=0 cellspacing=0>\n"
		"<tr>\n"
		"<form name=l action=/" SMAGIC "/bbslogin method=post><td>\n"
		// 帐号
		"<td>\xD5\xCA\xBA\xC5&nbsp&nbsp&nbsp</td>\n"
		"<td><input name=id type=text id=usrname style=\"font-size:11px;font-family:verdana\" size=10></td>\n"
		// 密码
		"<td>&nbsp&nbsp&nbsp\xC3\xDC\xC2\xEB&nbsp&nbsp&nbsp</td>\n"
		"<td><input name=pw type=password id=pwd style=\"font-size:11px;font-family:verdana\" size=10></td>\n"
		"<td>&nbsp&nbsp&nbsp<input name=login_btn type=image id=login_btn src=\"images/index_log.gif\" width=40 height=18 border=0 onMouseOver=\"this.src='images/index_log2.gif'\" onMouseOut=\"this.src='images/index_log.gif'\"></td>\n"
	//add by liuche@BMY 20120205
		// 找回用户名或密码？
		"<td>&nbsp&nbsp&nbsp<a href='/findpass.html' target='_blank' class=linkindex>\xD5\xD2\xBB\xD8\xD3\xC3\xBB\xA7\xC3\xFB\xBB\xF2\xC3\xDC\xC2\xEB\xA3\xBF</a></td>\n"
		"</form></tr></table>\n"
		"</tr>\n </td>\n"
		"</tr>\n"
		"</table></td>\n"
		"</tr>\n"
		"<tr>\n"
		"<td align=center bgcolor=#FFFFFF><img src=\"images/index_line.gif\" name=Image1 width=650 height=20 id=Image1></td>\n"
		// 陕ICP备 06008037号-5
		"<tr><td align=center><a href='https://beian.miit.gov.cn/' target='_blank'>\xC9\xC2ICP\xB1\xB8 06008037\xBA\xC5-5</a><br />"
		// 开发维护：西安交通大学网络中心  BBS程序组
		"\xBF\xAA\xB7\xA2\xCE\xAC\xBB\xA4\xA3\xBA\xCE\xF7\xB0\xB2\xBD\xBB\xCD\xA8\xB4\xF3\xD1\xA7\xCD\xF8\xC2\xE7\xD6\xD0\xD0\xC4  BBS\xB3\xCC\xD0\xF2\xD7\xE9</td></tr>"
	"</tr>\n"
"</table>");/* modified by linux 05.9.11 */
	showannounce();
	printf("<script>showloginpics(\"%s\")</script>", loginpics);
	printf("</body>\n</html>");
}

void
shownologin()
{
	static struct mmapfile mf = {
		.ptr = NULL
	};
	html_header(4);
	// 使用 bmy 图片取代 ytht 的配置
	printf("<STYLE type=text/css>A{COLOR: #99ccff; text-decoration: none;}</STYLE>"
			"</head><BODY text=#99ccff bgColor=#ffffff leftmargin=1 MARGINWIDTH=1><br>"
			"<CENTER>");
	printf("<IMG src=cai.jpg border=0 alt='' width=70%%><BR>");
	// 停站通知
	printf("<b>\xCD\xA3\xD5\xBE\xCD\xA8\xD6\xAA</b><br>");
	if (!mmapfile("NOLOGIN", &mf))
		fwrite(mf.ptr, mf.size, 1, stdout);
	printf("</CENTER></BODY></HTML>");
}

int
bbsindex_main()
{
	char str[20]; //, redbuf[50], main_page[STRLEN];
	if (nologin) {
		shownologin();
		http_quit();
		return 0;
	}
	if (strcmp(FIRST_PAGE, (g_is_nginx ? g_url : getsenv("SCRIPT_URL"))) == 0) {
		loginwindow();
		http_quit();
	}

	if (cache_header(1000000000, 86400))
		return 0;
	if (strcmp("/" SMAGIC "/", (g_is_nginx ? g_url : getsenv("SCRIPT_URL"))) == 0) {
		html_header(1);
		if (!isguest && (readuservalue(currentuser.userid, "wwwstyle", str, sizeof (str)) || atoi(str) != wwwstylenum)) {
			sprintf(str, "%d", wwwstylenum);
			saveuservalue(currentuser.userid, "wwwstyle", str);
		}
		//add by mintbaggio 040411 for new www
		// 欢迎光临
		printf("<title>\xBB\xB6\xD3\xAD\xB9\xE2\xC1\xD9 %s</title>"
				"<frameset cols=135,* frameSpacing=0 frameborder=no id=fs0>\n"
				"<frame src=bbsleft?t=%ld name=f2 frameborder=no scrolling=auto>\n"
				"<frameset id=fs1 rows=0,*,20 frameSpacing=0 frameborder=no border=0>\n"
				"<frame scrolling=no name=fmsg src=bbsgetmsg>\n"
				"<frame name=f3 src=\"bbsboa?secstr=?\">\n"
				"<frame scrolling=no name=f5 src=bbsfoot>\n"
				"</frameset>\n"
				"</frameset>\n"
				"<noframes>\n"
				"<body>\n"
				"</body>\n"
				"</noframes>\n", MY_BBS_NAME, now_t);
		http_quit();
	}

	// 不符合的情况
	html_header(3);
	redirect(FIRST_PAGE);
	http_quit();
	return 0;
}

