#include "bbslib.h"

/**
 * 判断是否为合法的 guest 链接
 * 样式后缀允许为 _[A-H]
 * @param url
 * @return 0 不合法 1 合法
 */
static int is_valid_guest_url(const char* url);

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

	char *fourpics=get_no_more_than_four_login_pics();
	printf("<script>function openreg(){open('" SMAGIC
			"/bbsreg', 'winREG', 'width=600,height=460,resizable=yes,scrollbars=yes');}\n"
			"function sf(){document.l.id.focus();}\n"
			"function st(){document.l.t.value=(new Date()).valueOf();}\n"
			"function lg(){self.location.href='/" SMAGIC
			"/bbslogin?id=guest&t='+(new Date()).valueOf();}\n"
			"</script>\n");
	printf("<link href=\"/images/oras.css\" rel=stylesheet type=text/css>\n");
	printf("<title>欢迎光临 "MY_BBS_NAME"</title>");
	printf("<script type=\"text/javascript\" src=\"jquery-1.6.4.min.js\"></script>");
	printf("<script type=\"text/javascript\" src=\"showloginpics.min.js\"></script>");
	printf("<link type=\"text/css\" rel=\"Stylesheet\" href=\"showloginpics.css\" />");
	printf("<body bgcolor=#efefef leftmargin=0 topmargin=0 onload='document.l.id.focus();'>\n"
		"<table width=100%% height=100%% border=0 cellpadding=0 cellspacing=0>\n"
		"<tr>\n <td rowspan=4>&nbsp;</td>\n <td width=650 height=47> </td>\n"
		"<td rowspan=4>&nbsp;</td>\n  </tr>\n  <tr> \n"
		"<td height=264 valign=top bgcolor=#FFFFFF>\n"
		"<table width=780 border=0 align=center cellpadding=0 cellspacing=0 bgcolor=#FFFFFF>\n"
		"<tr>\n<td height=35 align=right class=0004><a href='javascript:lg();' class=linkindex>匿名登录</a>"
		"/ <a href='telnet://" MY_BBS_DOMAIN "' class=linkindex>telnet登录</a> /"
		"<a href='CTerm.rar' class=linkindex>CTerm工具下载</a> /"
		"<a href='javascript: openreg();' class=linkindex>新用户注册</a><font color=#FFFFFF> \n"
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
		"<td>帐号&nbsp&nbsp&nbsp</td>\n"
		"<td><input name=id type=text id=usrname style=\"font-size:11px;font-family:verdana\" size=10></td>\n"
		"<td>&nbsp&nbsp&nbsp密码&nbsp&nbsp&nbsp</td>\n"
		"<td><input name=pw type=password id=pwd style=\"font-size:11px;font-family:verdana\" size=10></td>\n"
		"<td>&nbsp&nbsp&nbsp<input name=login_btn type=image id=login_btn src=\"images/index_log.gif\" width=40 height=18 border=0 onMouseOver=\"this.src='images/index_log2.gif'\" onMouseOut=\"this.src='images/index_log.gif'\"></td>\n"
	//add by liuche@BMY 20120205
		"<td>&nbsp&nbsp&nbsp<a href='/findpass.html' target='_blank' class=linkindex>找回用户名或密码？</a></td>\n"
		"</form></tr></table>\n"
		"</tr>\n </td>\n"
		"</tr>\n"
		"</table></td>\n"
		"</tr>\n"
		"<tr>\n"
		"<td align=center bgcolor=#FFFFFF><img src=\"images/index_line.gif\" name=Image1 width=650 height=20 id=Image1></td>\n"
		"<tr><td align=center>陕ICP备 05001571号<br />"
		//"本BBS隶属于：西安交通大学网络中心／中国教育科研网西北中心<br />"
		"开发维护：西安交通大学网络中心  BBS程序组</td></tr>"
	"</tr>\n"
"</table>");/* modified by linux 05.9.11 */
	showannounce();
	printf("<script>showloginpics(\"%s\")</script>",fourpics);
	free(fourpics);		// 释放资源 by IronBlood 2014.10.22 其实此处快退出了，会自动释放的 :-)
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
	printf("<b>停站通知</b><br>");
	if (!mmapfile("NOLOGIN", &mf))
		fwrite(mf.ptr, mf.size, 1, stdout);
	printf("</CENTER></BODY></HTML>");
}

int
bbsindex_main()
{
	char str[20], redbuf[50]; //, main_page[STRLEN];
	if (nologin) {
		shownologin();
		http_quit();
		return 0;
	}
	if (!(loginok || is_valid_guest_url(getsenv("SCRIPT_URL"))) && (rframe[0] == 0)) {
		if (strcasecmp(FIRST_PAGE, getsenv("SCRIPT_URL")) != 0) {
			html_header(3);
			redirect(FIRST_PAGE);
			http_quit();
		}
		loginwindow();
		http_quit();
	}
	if (!(loginok || is_valid_guest_url(getsenv("SCRIPT_URL")))) {
		sprintf(redbuf, "/" SMAGIC "/bbslogin?id=guest&t=%d", (int) now_t);
		html_header(3);
		redirect(redbuf);
		http_quit();
	}
	if (cache_header(1000000000, 86400))
		return 0;
	html_header(1);
#if 0
	printf("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Frameset//EN\"\n"
		"\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-frameset.dtd\">\n"
		"<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"gb2312\">\n"
		"<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />\n"
		"<meta http-equiv=\"Content-Language\" content=\"gb2312\" />\n"
		"<meta content=\"all\" name=\"robots\" />\n"
		"<meta name=\"author\" content=\"flyinsea@bmy,Inversun@bmy, minbtaggio@bmy\" />\n"
		"<meta name=\"Copyright\" content=\"bbs.xjtu.edu.cn,西安交大,兵马俑BBS\" />\n"
		"<meta name=\"description\" content=\"bbs.xjtu.edu.cn,西安交大,兵马俑BBS\" />\n"
		"<meta content=\"兵马俑BBS,bmy,bbs.xjtu.edu.cn,西安交大\" name=\"keywords\" />\n"
		"<!--<link rel=\"icon\" href=\"/images/favicon.ico\" type=\"/image/x-icon\" />\n"
		"<link rel=\"shortcut icon\" href=\"/images/favicon.ico\" type=\"/image/x-icon\" />-->\n"
		"<link href=\"/images/oras.css\" rel=\"stylesheet\" type=\"text/css\" />\n");
#endif
	if (!isguest && (readuservalue(currentuser.userid, "wwwstyle", str, sizeof (str)) || atoi(str) != wwwstylenum)) {
		sprintf(str, "%d", wwwstylenum);
		saveuservalue(currentuser.userid, "wwwstyle", str);
	}
	//add by mintbaggio 040411 for new www
	printf("<title>欢迎光临 %s</title>"
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
	return 0;
}

static int is_valid_guest_url(const char* url) {
	const char* patt = "/" SMAGIC "_?/";
	size_t i, l;
	l = strlen(patt);

	if (strcmp("/" SMAGIC "/", url) == 0)
		return 1;

	if (strlen(url) != l)
		return 0;

	for(i=0; i<l; i++) {
		if (patt[i] == '?') {
			if(url[i] < 'A' || url[i] > 'H')
				return 0;
		} else if (patt[i] != url[i])
			return 0;
	}

	return 1;
}
