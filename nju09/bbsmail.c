#include "bbslib.h"

int
bbsmail_main()
{	//modify by mintbaggio 20040829 for new www
	FILE *fp;
	int i, start, total;
	char buf[512], dir[80];
	struct fileheader x;
    /* type of mail box
     * 0 : in box
     * 1 : out box
     */
    int box_type = 0; 
	if (!loginok || isguest)
		http_fatal("您尚未登录, 请先登录");
    strsncpy(buf, getparm("type"), 10);
    if(buf[0] != 0) {
        box_type = atoi(buf);
    }
    if(box_type == 1) {
	    sprintf(dir, "mail/%c/%s/send_mail/.DIR", mytoupper(currentuser.userid[0]),
		    currentuser.userid);
    } else {
    	sprintf(dir, "mail/%c/%s/.DIR", mytoupper(currentuser.userid[0]),
    		currentuser.userid);
    }
	if(cache_header(file_time(dir),10))
		return 0;
	html_header(1);
	check_msg();
	changemode(RMAIL);
	strsncpy(buf, getparm("start"), 10);
	start = atoi(buf);
	if (buf[0] == 0)
		start = 999999;
	printf("<script language=javascript>function SelectAll()\
	{\
	for (i=0;i<document.maillist.length;i++)\
	{\
		if (document.maillist.elements[i].type==\"checkbox\")\
			if (! document.maillist.elements[i].checked)\
			{\
				document.maillist.elements[i].checked=true;\
			}\
	}\
}</script>\n");
	printf("<body leftmargin=0 topmargin=0>\n");
	printf("%s", "<table width=100%% border=0 cellpadding=0 cellspacing=0>\n"
		"<tr><td height=30 colspan=2></td>\n</tr>\n"
		"<tr>\n<td height=70 colspan=2>\n"
		"<table width=100%% height=100%% border=0 cellpadding=0 cellspacing=0 class=level2>\n"
		"<tr>\n<td width=40 rowspan=2>&nbsp; </td>\n"
		);
	printf("<td height=35>%s &gt;  <span id=topmenu_b>信件列表</span> [使用者: <span class=themetext>%s</span>] &#187; <b>信箱容量:</b> <span class=smalltext>%dk</span> &#187; <b>已用空间</b><spanclass=smalltext>%dk</span></td></tr>\n", BBSNAME,currentuser.userid,max_mail_size(),get_mail_size());
	printf("%s", "<tr>\n"
		"<td height=35 valign=top><a href='javascript:SelectAll()' class=\"btnsubmittheme\">全部选中</a>\n"
		"<a onclick='return confirm(\"你真的要删除这些信件吗?\")' href='javascript:document.maillist.submit()' class=\"btnsubmittheme\">删除选中的邮件</a>\n"
		"<a href='bbspstmail' class=\"btnsubmittheme\" title=\"发送信件 accesskey: m\" accesskey=\"m\">发送信件</a>\n");
	total = file_size(dir) / sizeof (struct fileheader);
	if (total < 0 || total > 30000)
		http_fatal("too many mails");
	if (!total) {
		printf("目前还没有任何信件");
		http_quit();
		return 0;
	}
	start = getdocstart(total, w_info->t_lines);

	if (total > w_info->t_lines)
		printf("<a href='mail?S=1' class=btnsubmittheme>第一页</a>\n"
			"<a href='mail?S=0' class=btnsubmittheme>最后一页</a>\n");
	//printf("start=%d, t_line=%d\n", start, w_info->t_lines);
	if(start > w_info->t_lines+1)
		printf("<a href='bbsmail?&S=%d' class=btnsubmittheme>上一页</a>\n", start - w_info->t_lines);
	if (start < total - w_info->t_lines + 1)
		printf("<a href='bbsmail?&S=%d' class=btnsubmittheme>下一页</a>\n", start + w_info->t_lines);
	printf("<a href=# onclick='javascript:{location=location;return false;}' class=btnsubmittheme>刷新</a>\n");
	
	printf("</td>\n</tr>\n");
#if 0
	total = file_size(dir) / sizeof (struct fileheader);
	if (total < 0 || total > 30000)
		http_fatal("too many mails");
	if (!total) {
		printf("目前还没有任何信件");
		http_quit();
		return 0;
	}
	start = getdocstart(total, w_info->t_lines);
#endif
	fp = fopen(dir, "r");
	if (fp == 0)
		http_fatal("无法打开目录文件, 请通知系统维护");
	//printhr();
	printf("%s", "<tr><td width=40 class=\"level1\">&nbsp;</td>\n"
		"<td class=\"level1\"><br>\n"
		);

	printf("<table width=95%% cellpadding=2 cellspacing=0><form action=bbsdelmail name=maillist>\n");
	printf
	    ("%s","<tbody><tr>\n"
		"<td class=tdtitle>选择</td>\n"
		"<td class=tdtitle>序号</td>\n"
		"<td class=tdtitle>状态</td>\n"
		"<td class=tdtitle>发信人</td>\n"
		"<td class=tdtitle>日期</td>\n"
		"<td class=tdtitle>信件标题</td></tr>\n");
	fseek(fp, (start - 1) * sizeof (struct fileheader), SEEK_SET);
	for (i = 0; i < w_info->t_lines; i++) {
		int type = 'N';
		if (fread(&x, sizeof (x), 1, fp) <= 0)
			break;
		printf("<tr><td class=tdborder><input type=checkbox name=F%d value=%s></td>",
		       i, fh2fname(&x));
		printf("<td class=tdborder>%d</td>", i + start);
		if (x.accessed & FH_READ)
			type = ' ';
		if (x.accessed & FH_MARKED)
			type = (type == 'N') ? 'M' : 'm';
		printf("<td class=tdborder>%c%c</td>", type,
		       x.accessed & FH_ATTACHED ? '@' : ' ');
		printf("<td class=tdborder>%s</td>", userid_str(fh2owner(&x)));
		printf("<td class=tdborder>%12.12s</td>", Ctime(x.filetime) + 4);
		printf("<td class=tdborder><a href=bbsmailcon?file=%s&num=%d>", fh2fname(&x),
		       i + start - 1);
		if (strncmp("Re: ", x.title, 4))
			printf("★ ");
		x.title[40] = 0;
		hprintf("%s", void1(x.title));
		printf("</a></td></tr>\n");
	}
	fclose(fp);
	printf("</tbody></form></table>\n");
	//printhr();
#if 0
	printf("[信件总数: %d] ", total);
	printf("[<a href='javascript:SelectAll()'>全部选中</a>] ");
	printf
	    ("[<a onclick='return confirm(\"你真的要删除这些信件吗?\")'"
	     " href='javascript:document.maillist.submit()'>删除选中的邮件</a>] ");
	printf("[<a href=bbspstmail>发送信件</a>] ");
	if (total > w_info->t_lines)
		printf("<a href=mail?S=1>第一页</a> <a href=mail?S=0>最后一页</a> ");
	bbsdoc_helper("bbsmail?", start, total, w_info->t_lines);
	printf
	    ("<form><input type=submit value=跳转到> 第 <input style='height:20px' type=text name=start size=3> 封</form></body>");
#endif
	printf("<form><input type=submit class=resetlong value=跳转到> 第 <input type=text name=start size=3> 封</form>\n");
	printf("</td></tr><table></td></tr></table></body>\n");
	http_quit();
	return 0;
}
