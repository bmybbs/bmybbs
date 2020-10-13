#include "bbslib.h"

void
printdiv(int *n, char *str)
{//modify by mintbaggio 040411 for new www, modify 041225
	printf("<tr><td align=right><img id=img%d src=\"/images/plus.gif\"></td>\n"
			"<td><DIV class=r id=div%da> <a class=linkleft href=\"javascript:;\" id=menu onClick=\"changemn('%d');\">%s</a>",
			*n, *n, *n, str);
	printf("<script type=\"text/javascript\">"
			"function clickRadio()"
			"  {"
			"document.getElementById('menu').click()\n"
			"  }"
			"</script>");

	if(!strcmp(str, "分类讨论区")){
		//modify: \"_bbsall.htm\" to: bbssecfly by: flyinsea
		printf("<a href=bbssecfly target=f3 style=\"font-size:8px;\"> &gt;&gt;</a> ");
	}
	printf("</div></td></tr>\n");
	if(!strcmp(str, "预定讨论区"))
		printf("<tr><td colspan=2> <DIV class=s id=div%d>", (*n)++);
	else
		printf("<tr><td align=right></td><td> <DIV class=s id=div%d>", (*n)++);
}

void
printsectree(const struct sectree *sec)
{
	int i;
	for (i = 0; i < sec->nsubsec; i++) {
		printf("&nbsp;&nbsp;<a target=f3 href=boa?secstr=%s class=linkleft>"
				"%s%c</a><br>\n", sec->subsec[i]->basestr,
				nohtml(sec->subsec[i]->title),
				sec->subsec[i]->nsubsec ? '+' : ' ');
	}
}

int
bbsleft_main()
{
	int i;
	int div = 0;
	char buf[512];
	FILE* fp;
	changemode(MMENU);
	html_header(2);

	//modify by mintbaggio 040411 for new www
	printf("<script src=\"/inc/func.js\"></script>"
			"<script language=\"JavaScript\" type=\"text/JavaScript\">"
			"<!--\n"
			"\t	function MM_preloadImages() { //v3.0\n"
			"\t\t		var d=document; if(d.images){ if(!d.MM_p) d.MM_p=new Array();\n"
			"\t\t		var i,j=d.MM_p.length,a=MM_preloadImages.arguments; for(i=0; i<a.length; i++)\n"
			"\t\t		if (a[i].indexOf(\"#\")!=0){ d.MM_p[j]=new Image; d.MM_p[j++].src=a[i];}}\n"
			"\t	}\n"
			"-->\n"
			"</script>\n</head>\n"
			"<body class=\"level2\" leftmargin=0 topmargin=0 onMouseOver='doMouseOver()' onMouseEnter='doMouseOver()' onMouseOut='doMouseOut()' onload=\"clickRadio()\">\n");
	printf("<table width=100%% border=0 cellpadding=0 cellspacing=0>\n"
			"<tr><td width=100%% height=14></td></tr>\n"
			"<tr><td height=16 class=\"level2\">&nbsp;</td></tr>\n"
			"<tr><td height=70 class=\"level3\">\n");
	printf("<table width=100%% border=0 cellpadding=0 cellspacing=1>\n<tr><td><div align=center>\n");
	if (!loginok || isguest) {
		printf("<form action=bbslogin method=post target=_top name=loginform><tr><td>\n"
			 "<div style=\"text-align:center\">用户登录</div>\n"
			 "帐号<input type=text name=id maxlength=12 size=8 class=inputuser><br>\n"
			 "密码<input type=password name=pw maxlength=12 size=8 class=inputpwd><br>\n"
			 "<input type=submit class=sumbitshort value=登录>&nbsp;"
			 "<input type=submit class=resetshort value=注册 onclick=\"{openreg();return false}\">"
			 "&nbsp&nbsp<a target=f3 href='bbsfindpass' target='_blank' class=linkindex><br>找回帐号或密码</a><br>\n"
			 "</form>\n");
	} else {
		char buf[256] = "未注册用户";
		printf("<a class=1100>用户: <a href=bbsqry?userid=%s target=f3>%s</a><br>",
				currentuser.userid, currentuser.userid);
		if (currentuser.userlevel & PERM_LOGINOK)
			strcpy(buf, charexp(countexp(&currentuser)));
		if (currentuser.userlevel & PERM_BOARDS)
			strcpy(buf, "版主");
		if (currentuser.userlevel & PERM_XEMPT)
			strcpy(buf, "永久帐号");
		if (currentuser.userlevel & PERM_SYSOP)
			strcpy(buf, "本站站长");
		printf("<a class=1100>级别: %s<br>", buf);
		printf("<a href=bbslogout target=_top class=1100>注销本次登录</a><br>\n");
	}
	printf("</div></td></tr></table></td> </tr></table>\n");
	printf("<table width=100%% border=0 cellpadding=0 cellspacing=0>\n");
	printf("<tr><td width=27 align=right><img src=\"/images/list2.gif\"></td>\n"
			"<td width=107><a href=\"boa?secstr=?\" target=f3 class=linkleft>" MY_BBS_ID "导读</a></td></tr>\n");
	printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
			"<td><a href=bbs0an target=f3 class=linkleft>精华公布栏</a></td></tr>\n");
	printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
			"<td><a href=\"bbstop10\" target=f3 class=linkleft><b>热门话题页</b></a></td></tr>\n");
	//Add by liuche 20121119 order by oOIOo ^_^
	printdiv(&div, "BMY告示墙");
	printf("&nbsp;&nbsp;<a target=f3 href=gdoc?B=AcademicClub class=linkleft>讲座信息</a><br>\n");
	printf("&nbsp;&nbsp;<a target=f3 href=gdoc?B=Activity class=linkleft>校园活动</a><br>\n");
	printf("&nbsp;&nbsp;<a class=linkleft href=\"%sLost_Found\" target=f3>失物招领</a><br>\n", showByDefMode() );
	printf("&nbsp;&nbsp;<a class=linkleft href=\"tdoc?board=BoardHome\" target=f3>我要当版主</a><br>\n");
	printf("&nbsp;&nbsp;<a class=linkleft href=\"%sBMY_Dev\" target=f3>程序与报错</a><br>\n", showByDefMode() );
	printf("&nbsp;&nbsp;<a class=linkleft href=\"%sArtDesign\" target=f3>美工与进站</a><br>\n", showByDefMode() );
	printf("</div></td></tr>\n");

	if (loginok && !isguest) {
		char *ptr, buf[10];
		struct boardmem *x1;
		int mybrdmode;
		readuservalue(currentuser.userid, "mybrdmode", buf, sizeof (buf));
		mybrdmode = atoi(buf);
		printdiv(&div, "预定讨论区");
		readmybrd(currentuser.userid);
		for (i = 0; i < mybrdnum; i++) {
			ptr = mybrd[i];
			if (!mybrdmode) {
				x1 = getboard(mybrd[i]);
				if (x1)
					ptr = nohtml(titlestr(x1->header.title));
			}
			printf("&nbsp;&nbsp;<a target=f3 href=%s%s class=linkleft>%s</a><br>\n", showByDefMode(), mybrd[i], ptr);
		}
		printf("&nbsp;&nbsp;<a target=f3 href=bbsboa?secstr=* class=linkleft>预定区总览</a><br>\n");
		printf("&nbsp;&nbsp;<a target=f3 href=bbsmybrd?mode=1 class=linkleft>预定管理</a><br>\n");
		printf("</div></td></tr>\n");
	}
	printdiv(&div, "分类讨论区");
	printsectree(&sectree);
	printf("</div></td></tr>\n");
	printdiv(&div, "谈天说地");
	if (loginok && !isguest) {
		printf("&nbsp;&nbsp;<a href=bbsfriend target=f3 class=linkleft>在线好友</a><br>\n");
	}
	printf("&nbsp;&nbsp;<a href=bbsufind?search=A&limit=20 target=f3 class=linkleft>环顾四方</a><br>\n");
	printf("&nbsp;&nbsp;<a href=bbsqry target=f3 class=linkleft>查询网友</a><br>\n");
	if (currentuser.userlevel & PERM_PAGE) {
		printf("&nbsp;&nbsp;<a href=bbssendmsg target=f3 class=linkleft>发送讯息</a><br>\n");
		printf("&nbsp;&nbsp;<a href=bbsmsg target=f3 class=linkleft>查看所有讯息</a><br>\n");
	}
	printf("</div></td></tr>\n");
	if (loginok && !isguest) {
		printdiv(&div, "个人工具箱");
		printf("&nbsp;&nbsp;<a target=f3 href=bbsinfo class=linkleft>个人资料</a><br>"
				"&nbsp;&nbsp;<a target=f3 href=bbsplan class=linkleft>改说明档</a><br>"
				"&nbsp;&nbsp;<a target=f3 href=bbssig class=linkleft>改签名档</a><br>"
				"&nbsp;&nbsp;<a target=f3 href=bbspwd?mode=1 class=linkleft>修改密码</a><br>"
				"&nbsp;&nbsp;<a target=f3 href=bbsparm class=linkleft>修改个人参数</a><br>"
				"&nbsp;&nbsp;<a target=f3 href=bbsmywww class=linkleft>www个人定制</a><br>"
				"&nbsp;&nbsp;<a target=f3 href=bbsnick class=linkleft>临时改昵称</a><br>"
				"&nbsp;&nbsp;<a target=f3 href=bbsstat class=linkleft>排名统计</a><br>"
				"&nbsp;&nbsp;<a target=f3 href=bbsfall class=linkleft>设定好友</a><br>"
				"&nbsp;&nbsp;<a target=f3 href=bbsball class=linkleft>设定黑名单</a><br>"
				"&nbsp;&nbsp;<a target=f3 href=bbsmybrd?mode=2 class=linkleft>RSS订阅管理</a><br>");
		if (currentuser.userlevel & PERM_CLOAK)
			printf("&nbsp;&nbsp;<a target=f3 "
					"onclick='return confirm(\"确实切换隐身状态吗?\")' "
					"href=bbscloak class=linkleft>切换隐身</a><br>\n");
		printf("</div></td></tr>");
		printdiv(&div, "处理信件");
		printf("&nbsp;&nbsp;<a target=f3 href=bbsnewmail class=linkleft>新邮件</a><br>"
				"&nbsp;&nbsp;<a target=f3 href=bbsmail class=linkleft>收件箱</a><br>"
				"&nbsp;&nbsp;<a target=f3 href=bbsmail?box_type=1 class=linkleft>发件箱</a><br>"
				"&nbsp;&nbsp;<a target=f3 href=bbspstmail class=linkleft>发送邮件</a><br>"
				"</div></td></tr>");
	}
	printdiv(&div, "特别服务");
	printf("&nbsp;&nbsp;<a target=f3 href=/wnl.html class=linkleft>万年历</a><br>\n");
	printf("&nbsp;&nbsp;<a target=f3 href=/scicalc.html class=linkleft>科学计算器</a><br>\n");
	printf("&nbsp;&nbsp;<a href=bbsfind target=f3 class=linkleft>文章查询</a><br>\n");

	printf("&nbsp;&nbsp;<a target=f3 href=bbsx?chm=0 class=linkleft>下载精华区</a><br>\n");

	printf("</div></td></tr>\n");
	printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
			"<td><a class=linkleft href=\"bbs0an?path=/groups/GROUP_0/PersonalCorpus\" target=f3>个人文集区</a>\n"
			"</td></tr>\n");
	printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
			"<td><a class=linkleft href=\"bbsall\" target=f3>所有讨论区</a></td></tr>\n");
	printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
			"<td><a class=linkleft href=\"bbsselstyle\" target=f3>更换界面</a>"
			"</td></tr>\n");
	printf("<tr><form action=bbssbs target=f3><td colspan=2>\n"
			"&nbsp;&nbsp;&nbsp;&nbsp;<input type=text name=keyword maxlength=20 "
			"size=9 onclick=\"this.select()\" value=选择讨论区><input type=submit class=sumbitgrey value=go></td></form></tr>\n");
			if (loginok && !isguest && !(currentuser.userlevel & PERM_LOGINOK) && !has_fill_form())
			printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
			 "<td><a class=linkleft href=\"bbsform\" target=f3>填写注册单</a></td></tr>\n");
		if (loginok && !isguest && HAS_PERM(PERM_SYSOP, currentuser))        //add by mintbaggio@BMY for www SYSOP kick www user
			printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
					"<td><a class=linkleft href=\"kick\" target=f3>踢www下站</a></td></tr>\n");
		printdiv(&div, "友情链接");
		printf("&nbsp;&nbsp;<a target=_BLANK href='http://www.xjtu.edu.cn/' class=linkleft>交大主页</a><br>\n");
		printf("&nbsp;&nbsp;<a target=_BLANK href='http://nic.xjtu.edu.cn/' class=linkleft>网络中心</a><br>\n");
		printf("</div></td></tr>\n");
		printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
				"<td><a class=linkleft href=\"%sBBShelp\" target=f3>用户帮助</a>\n"
				"</td></tr>\n", showByDefMode() );
		printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
				"<td><a class=linkleft href=\"bbspstmail?userid=SYSOP\" target=f3>发信给站长</a>\n"
				"</td></tr>\n");
		printf("</table>\n");
		printf("<table width=124>"
				"<tr><form><td>&nbsp;&nbsp;<input type=button style='width:90px' value='隐藏菜单' "
				"onclick=\"{if(switchAutoHide()==true) {this.value='停止自动隐藏';}"
				"else this.value='隐藏菜单';return false;}\">"
				"</td></form></tr></table>\n");

		fp = fopen("etc/ad_left", "r");
		if(!fp){
			//printf("fail to open\n");
			goto endleft;
		}
		bzero(buf, 512);
		while(fgets(buf, 512, fp)){
			ytht_strtrim(buf);
			if (strlen(buf) <= 1)
				continue;
			char *p = strchr(buf, ' ');
			if (p == NULL)
				continue;
			*p = '\0';

			printf("<table width=124>"
					"<tr><td>"
					"<a href='%s' target='_blank'><img src='%s' width=120 height=44 border=0 /></a>"
					"</td></tr></table>\n", p+1, buf);
		}
		fclose(fp);

endleft:

		// end of interma announce log picture

		printf("<script>if(isNS4) arrange();if(isOP)alarrangeO();</script>");
		//add by macintosh 20051216
		if (loginok && !isguest) {
			printf("<script>\n"
					"window.onbeforeunload = function(){\n"
					"  if(event.clientX>document.body.clientWidth&&event.clientY<0||event.altKey){\n"
					"return '直接关闭浏览器将不计上站时间，强烈建议您点“注销本次登录”。'}}\n"
					"</script>\n");
			//add by macintosh 20051216, end
			if (HAS_PERM(PERM_LOGINOK, currentuser) && !HAS_PERM(PERM_POST, currentuser))
				printf("<script>alert('您被封禁了全站发表文章的权限, 请参看sysop版公告, 期满后在sysop版申请解封. 如有异议, 可在committee版提出申诉.')</script>\n");
			mails(currentuser.userid, &i);
			if (i > 0)
				printf("<script>alert('您有新信件!')</script>\n");
		}
		if (isguest && 0)
			printf("<script>setTimeout('open(\"regreq\", \"winREGREQ\", \"width=600,height=460\")', 1800000);</script>");
		if (loginok && !isguest) {
			char filename[80];
			sethomepath(filename, currentuser.userid);
			mkdir(filename, 0755);
			sethomefile(filename, currentuser.userid, BADLOGINFILE);
			if (file_exist(filename)) {
				printf("<script>"
						"window.open('bbsbadlogins', 'badlogins', 'toolbar=0, scrollbars=1, location=0, statusbar=1, menubar=0, resizable=1, width=450, height=300');"
						"</script>");
			}
		}
		if (!via_proxy && wwwcache->accel_port && wwwcache->accel_ip)
			printf("<script src=http://%s:%d/testdoc.js></script>",
					inet_ntoa(wwwcache->accel_addr), wwwcache->accel_port);
		else if (via_proxy)
			printf("<script src=/testdoc.js></script>");
		printf("</body></html>");


	printf("</body></html>");
return 0;
}

