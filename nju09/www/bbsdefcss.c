#include "bbslib.h"


int
bbsdefcss_main()
{
	char *ptr, buf[256];
	int type;
	html_header(1);
	check_msg();
	printf("<body>");
	if (!loginok || isguest)
		http_fatal("匆匆过客不能定制界面");
	changemode(GMENU);
	type = atoi(getparm("type"));
	if (type > 0){
		sethomefile_s(buf, sizeof(buf), currentuser.userid, "ubbs.css");
		ptr=getparm("ucss");
		f_write(buf,ptr);
/*		sethomefile_s(buf, sizeof buf, currentuser.userid, "uleft.css");
		ptr=getparm("uleftcss");
		f_write(buf,ptr);*/
		printf("WWW用户定制样式设定成功.<br>\n");
		printf("[<a href='javascript:history.go(-2)'>返回</a>]");
		return 0;
	}
	printf("<table align=center><form action=bbsdefcss method=post>\n");
	printf("<tr><td>\n");
	printf("<input type=hidden name=type value=1>");
	printf("<span class=c31> 个人CSS模式允许用户自定义WWW的各种显示效果。<br>注意，下面是自于当前的界面[%s]所定义的CSS。<br>确定后将覆盖原有的所有自定义CSS。</span><br><br>\n",currstyle->name);
	printf("当前的CSS设置如下：<br><textarea name=ucss rows=20 cols=76>");
	if(strstr(currstyle->cssfile,"ubbs.css"))
		sethomefile_s(buf, sizeof(buf), currentuser.userid, "ubbs.css");
	else
		sprintf(buf,HTMPATH "%s",currstyle->cssfile);
	showfile(buf);
	printf("</textarea><br>");
/*	printf("</textarea><br><br>当前的左侧选单的CSS设置如下(body.foot是底行的css)：<br><textarea name=uleftcss rows=5 cols=76>");
	if(strstr(currstyle->leftcssfile,"uleft.css"))
		sethomefile_s(buf, sizeof buf, currentuser.userid, "uleft.css");
	else
		sprintf(buf,HTMPATH "%s", currstyle->leftcssfile);
	showfile(buf);
	printf("</textarea><br><br>");*/
	printf("</textarea></td></tr><tr><td><input type=submit class=resetlong value=确定>"
			" <input type=reset class=sumbitlong value=复原>\n");
	printf("</td></tr></form></table></body></html>\n");
	return 0;
}

