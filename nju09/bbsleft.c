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

	if(!strcmp(str, "����������")){
		//modify: \"_bbsall.htm\" to: bbssecfly by: flyinsea
		printf("<a href=bbssecfly target=f3 style=\"font-size:8px;\"> &gt;&gt;</a> ");
	}
	printf("</div></td></tr>\n");
	if(!strcmp(str, "Ԥ��������"))
	        printf("<tr><td colspan=2> <DIV class=s id=div%d>", (*n)++);
	else	printf("<tr><td align=right></td><td> <DIV class=s id=div%d>", (*n)++);
	/*	printf("<div id=div%da class=r><A href='javascript:changemn(\"%d\");'>",
	       *n, *n);
	printf("<img border=0 id=img%d src=/folder.gif>%s</A></div>\n", *n,
	       str);
	printf("<div id=div%d class=s>\n", (*n)++);*/
}

void
printsectree(const struct sectree *sec)
{
	int i;
	for (i = 0; i < sec->nsubsec; i++) {
#if 0
		if (sec->subsec[i]->nsubsec)
			continue;
#endif
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
#if 0
	{
		char *ptr;
		char buf[256];
		ptr = getsenv("HTTP_USER_AGENT");
		sprintf(buf, "%-14.14s %.100s", currentuser.userid, ptr);
		addtofile(MY_BBS_HOME "/browser.log", buf);
	}
#endif

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
		//" onLoad='MM_preloadImages(\"/images/minus.gif\");document.loginform.id.focus();'>\n");
	printf("<table width=100%% border=0 cellpadding=0 cellspacing=0>\n"
                        "<tr><td width=100%% height=14></td></tr>\n"
                        "<tr><td height=16 class=\"level2\">&nbsp;</td></tr>\n"
                        "<tr><td height=70 class=\"level3\">\n");
        printf("<table width=100%% border=0 cellpadding=0 cellspacing=1>\n<tr><td><div align=center>\n");
	if (!loginok || isguest) {
		printf
		    ("<form action=bbslogin method=post target=_top name=loginform><tr><td>\n"
		     "<div style=\"text-align:center\">�û���¼</div>\n"
		     "�ʺ�<input type=text name=id maxlength=12 size=8 class=inputuser><br>\n"
		     "����<input type=password name=pw maxlength=12 size=8 class=inputpwd><br>\n"
		     //"��Χ<select name=ipmask class=1100>\n"
		     //"<option value=0 selected>��IP</option>\n"
		     //"<option value=1>2 IP</option>\n"
		     //"<option value=2>4 IP</option>\n"
		     //"<option value=3>8 IP</option>\n"
		     //"<option value=4>16 IP</option>\n"
		     //"<option value=5>32 IP</option>\n"
		     //"<option value=6>64 IP</option>\n"
		     //"<option value=7>128 IP</option>\n"
		     //"<option value=8>256 IP</option></select><br>"
		     //"<a href=/ipmask.html target=_blank class=1100>����ʲô?</a>\n<br>"
		     "<input type=submit class=sumbitshort value=��¼>&nbsp;"
		     "<input type=submit class=resetshort value=ע�� onclick=\"{openreg();return false}\">"
		     "&nbsp&nbsp<a target=f3 href='bbsfindpass' target='_blank' class=linkindex><br>�һ��ʺŻ�����</a><br>\n"
		     "</form>\n");
	} else {
		char buf[256] = "δע���û�";
		printf("<a class=1100>�û�: <a href=bbsqry?userid=%s target=f3>%s</a><br>",
		       currentuser.userid, currentuser.userid);
		if (currentuser.userlevel & PERM_LOGINOK)
			strcpy(buf, charexp(countexp(&currentuser)));
		if (currentuser.userlevel & PERM_BOARDS)
			strcpy(buf, "����");
		if (currentuser.userlevel & PERM_XEMPT)
			strcpy(buf, "�����ʺ�");
		if (currentuser.userlevel & PERM_SYSOP)
			strcpy(buf, "��վվ��");
		printf("<a class=1100>����: %s<br>", buf);
		printf("<a href=bbslogout target=_top class=1100>ע�����ε�¼</a><br>\n");
	}
	printf("</div></td></tr></table></td> </tr></table>\n");
//	printf("<hr>");
	//printf("&nbsp;&nbsp;<a href=http://162.105.153.249 target=_blank><font color=red>���ﶩվ��</font></a><br>");
	printf("<table width=100%% border=0 cellpadding=0 cellspacing=0>\n");
	printf("<tr><td width=27 align=right><img src=\"/images/list2.gif\"></td>\n"
		"<td width=107><a href=\"boa?secstr=?\" target=f3 class=linkleft>" MY_BBS_ID "����</a></td></tr>\n");
	printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
		"<td><a href=bbs0an target=f3 class=linkleft>����������</a></td></tr>\n");
	printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
		"<td><a href=\"bbstop10\" target=f3 class=linkleft><b>���Ż���ҳ</b></a></td></tr>\n");
	//by bjgyt printf
	/*printf("<tr><td align=right valign=top> <img src=\"/images/bmy_arrowblank.gif\" width=6 height=5></td>\n
		<td><a target=f3 href=\"bbsshownav?a1=class&a2=all\" class=1100>���վ��ʻ���</a></td></tr>\n");*/
	//Add by liuche 20121119 order by oOIOo ^_^
	printdiv(&div, "BMY��ʾǽ");
	printf("&nbsp;&nbsp;<a target=f3 href=gdoc?B=AcademicClub class=linkleft>������Ϣ</a><br>\n");
	printf("&nbsp;&nbsp;<a target=f3 href=gdoc?B=Activity class=linkleft>У԰�</a><br>\n");
	printf("&nbsp;&nbsp;<a class=linkleft href=\"%sLost_Found\" target=f3>ʧ������</a><br>\n"
		, showByDefMode() );
	printf("&nbsp;&nbsp;<a class=linkleft href=\"tdoc?board=BoardHome\" target=f3>��Ҫ������</a><br>\n");
	printf("&nbsp;&nbsp;<a class=linkleft href=\"%sBMY_Dev\" target=f3>�����뱨��</a><br>\n"
		, showByDefMode() );
	printf("&nbsp;&nbsp;<a class=linkleft href=\"%sArtDesign\" target=f3>�������վ</a><br>\n"
		, showByDefMode() );	
	printf("</div></td></tr>\n");
	
	if (loginok && !isguest) {
		char *ptr, buf[10];
		struct boardmem *x1;
		int mybrdmode;
		readuservalue(currentuser.userid, "mybrdmode", buf,
			      sizeof (buf));
		mybrdmode = atoi(buf);
		printdiv(&div, "Ԥ��������");
//		printf("<tr><td rowspan=2 align=right valign=top><img id=img0 src=\"/images/bmy_arrow_black.gif\" width=6 height=5></td>\n"
//			"<td><DIV class=r id=div0a><a href=\"javascript:;\" onClick=\"changemn('0')\" class=1100>Ԥ��������</a>
//			\n");
		readmybrd(currentuser.userid);
		//printf("<tr><td><DIV class=s id=div0>\n");
		for (i = 0; i < mybrdnum; i++) {
			ptr = mybrd[i];
			if (!mybrdmode) {
				x1 = getboard(mybrd[i]);
				if (x1)
					ptr =
					    nohtml(titlestr(x1->header.title));
			}
			printf
			    ("&nbsp;&nbsp;<a target=f3 href=%s%s class=linkleft>%s</a><br>\n",
			     showByDefMode(), mybrd[i], ptr);
		}
		printf
		    ("&nbsp;&nbsp;<a target=f3 href=bbsboa?secstr=* class=linkleft>Ԥ��������</a><br>\n");
		printf
		    ("&nbsp;&nbsp;<a target=f3 href=bbsmybrd?mode=1 class=linkleft>Ԥ������</a><br>\n");
		printf("</div></td></tr>\n");
	}
//	printf("<tr><td align=right valign=top><img id=img1 src=\"images/bmy_arrow_black.gif\" width=6 height=5></td>\n
//		<td><DIV class=r id=div1a><a href=\"javascript:;\" target=f3 class=1100 onClick=\"changemn('1');\">����������</a>
//		<a href=\"_bbsall.htm\" target=f3 style=\"font-size:8px;\"> &gt;&gt;</a> </div></td></tr>\n");
	printdiv(&div, "����������");
	printsectree(&sectree);
	//printf("</div>\n");
	//printf("<div class=s>");
#if 0
	for (i = 0; i < sectree.nsubsec; i++) {
		const struct sectree *sec = sectree.subsec[i];
		if (!sec->nsubsec)
			continue;
		printf("&nbsp;&nbsp;<a target=f3 href=bbsboa?secstr=%s class=linkleft>%s</a><br>\n",
		       sec->basestr, sec->title);
	}
#endif
	printf("</div></td></tr>\n");
//	printf("<tr><td align=right valign=top><img id=img2 src=\"images/bmy_arrow_black.gif\" width=6 height=5></td>\n
//		<td><DIV class=r id=div2a><a href="javascript:;" target=f3 class=1100 onClick=\"changemn('2');\"≯��˵����</a>
//		</div></td></tr>\n");
	printdiv(&div, "̸��˵��");
//	printf("<tr><td align=right></td><td> <DIV class=s id=div2>\n");
	if (loginok && !isguest) {
		printf
		    ("&nbsp;&nbsp;<a href=bbsfriend target=f3 class=linkleft>���ߺ���</a><br>\n");
	}
	printf
	    ("&nbsp;&nbsp;<a href=bbsufind?search=A&limit=20 target=f3 class=linkleft>�����ķ�</a><br>\n");
	printf("&nbsp;&nbsp;<a href=bbsqry target=f3 class=linkleft>��ѯ����</a><br>\n");
//      printf("&nbsp;&nbsp;<a target=f3 href=bbsalluser>����ʹ����</a><br>\n");
	if (currentuser.userlevel & PERM_PAGE) {
		printf
		    ("&nbsp;&nbsp;<a href=bbssendmsg target=f3 class=linkleft>����ѶϢ</a><br>\n");
		printf
		    ("&nbsp;&nbsp;<a href=bbsmsg target=f3 class=linkleft>�鿴����ѶϢ</a><br>\n");
	}
	printf("</div></td></tr>\n");
	if (loginok && !isguest) {
		printdiv(&div, "���˹�����");
//		printf("<tr><td align=right valign=top><img id=img3 src=\"images/bmy_arrow_black.gif\" width=6 height=5></td>\n
///			<td><DIV class=r id=div3a><a class=1100 href='javascript:;' onClick=\"changemn('3');\">���˹�����</a>
//			</div></td></tr>\n");
		printf("&nbsp;&nbsp;<a target=f3 href=bbsinfo class=linkleft>��������</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbsplan class=linkleft>��˵����</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbssig class=linkleft>��ǩ����</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbspwd?mode=1 class=linkleft>�޸�����</a><br>"
//		       "&nbsp;&nbsp;<a target=f3 href=bbspwd?mode=2 class=linkleft>�һ���������</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbsparm class=linkleft>�޸ĸ��˲���</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbsmywww class=linkleft>www���˶���</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbsnick class=linkleft>��ʱ���ǳ�</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbsstat class=linkleft>����ͳ��</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbsfall class=linkleft>�趨����</a><br>"
			   "&nbsp;&nbsp;<a target=f3 href=bbsball class=linkleft>�趨������</a><br>"
			   "&nbsp;&nbsp;<a target=f3 href=bbsmybrd?mode=2 class=linkleft>RSS���Ĺ���</a><br>");
		if (currentuser.userlevel & PERM_CLOAK)
			printf("&nbsp;&nbsp;<a target=f3 "
			       "onclick='return confirm(\"ȷʵ�л�����״̬��?\")' "
			       "href=bbscloak class=linkleft>�л�����</a><br>\n");
		printf("</div></td></tr>");
		printdiv(&div, "�����ż�");
		printf("&nbsp;&nbsp;<a target=f3 href=bbsnewmail class=linkleft>���ʼ�</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbsmail class=linkleft>�ռ���</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbsmail?box_type=1 class=linkleft>�ѷ����ʼ�</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbspstmail class=linkleft>�����ʼ�</a><br>"
		       "</div></td></tr>");
	}
	printdiv(&div, "�ر����");
	//printf("&nbsp;&nbsp;<a target=f3 href=bbssechand>�����г�</a><br>\n");
	printf("&nbsp;&nbsp;<a target=f3 href=/wnl.html class=linkleft>������</a><br>\n");
	//printf("&nbsp;&nbsp;<a target=f3 href=/cgi-bin/cgincce class=1100>�Ƽ��ʵ�</a><br>\n");
	printf("&nbsp;&nbsp;<a target=f3 href=/scicalc.html class=linkleft>��ѧ������</a><br>\n");
	//printf("&nbsp;&nbsp;<a target=f3 href=/periodic/periodic.html class=1100>Ԫ�����ڱ�</a><br>\n");
	//printf("&nbsp;&nbsp;<a target=f3 href=/cgi-bin/cgiman class=1100>Linux�ֲ��ѯ</a><br>\n");
	printf("&nbsp;&nbsp;<a href=bbsfind target=f3 class=linkleft>���²�ѯ</a><br>\n");
	
	//printf("&nbsp;&nbsp;<a target=f3 href=/cgi-bin/cgifreeip class=1100>IP��ַ��ѯ</a><br>\n");
	printf("&nbsp;&nbsp;<a target=f3 href=bbsx?chm=0 class=linkleft>���ؾ�����</a><br>\n");

	
	//printf("&nbsp;&nbsp;<a target=f3 href=home/pub/index.html class=linkleft>��������</a><br>\n");
//	printf("&nbsp;&nbsp;<a target=f3 href=\"/xiaoli.htm\" class=linkleft>У��</a><br>\n");

	printf("</div></td></tr>\n");
	//printf("<div class=r>");
	printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
		"<td><a class=linkleft href=\"bbs0an?path=/groups/GROUP_0/PersonalCorpus\" target=f3>�����ļ���</a>\n"
		"</td></tr>\n");
//	printf("&nbsp;&nbsp;<a target=f3 href=bbs0an?path=/groups/GROUP_0/PersonalCorpus>�����ļ���</a><br>\n");
	printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
		"<td><a class=linkleft href=\"bbsall\" target=f3>����������</a></td></tr>\n");
	//printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
       //         "<td><a class=linkleft href=\"/search.htm\" target=f3>����������</a>"
       //         "</td></tr>\n");
	printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
               "<td><a class=linkleft href=\"bbsselstyle\" target=f3>��������</a>"
                "</td></tr>\n");
//	printf("&nbsp;&nbsp;<a target=f3 href=bbsall>����������</a><br>\n");
//	printf("<hr>");
	printf("<tr><form action=bbssbs target=f3><td colspan=2>\n"
	       "&nbsp;&nbsp;&nbsp;&nbsp;<input type=text name=keyword maxlength=20 "
	       "size=9 onclick=\"this.select()\" value=ѡ��������><input type=submit class=sumbitgrey value=go></td></form></tr>\n");
//	printf("&nbsp;&nbsp;<a href='telnet:%s'>Telnet��¼</a>\n", BBSHOST);
/*	if (!loginok || isguest)
		printf
		    ("<br>&nbsp;&nbsp;<a href=\"javascript: openreg()\" class=1100>���û�ע��</a>\n");
*/	if (loginok && !isguest && !(currentuser.userlevel & PERM_LOGINOK)
	    && !has_fill_form())
		printf
		    ("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
                "<td><a class=linkleft href=\"bbsform\" target=f3>��дע�ᵥ</a></td></tr>\n");
/*	if (loginok && !isguest && HAS_PERM(PERM_ACCOUNTS))
		printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
                "<td><a class=linkleft href=\"bbsscanreg\" target=f3>SCANREG</a></td></tr>\n");
*/
	if (loginok && !isguest && HAS_PERM(PERM_SYSOP))        //add by mintbaggio@BMY for www SYSOP kick www user
                printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
                "<td><a class=linkleft href=\"kick\" target=f3>��www��վ</a></td></tr>\n");
	//if(loginok && !isguest) printf("<br>&nbsp;&nbsp;<a href='javascript:openchat()'>bbs���</a>");
//	printf("<br>&nbsp;&nbsp;<a href=bbsselstyle target=f3>�������濴��</a>");
	//printf ("<br>&nbsp;&nbsp;<a href='http://bug.ytht.org/' target=_BLANK>���� Bug</a>");
//	printf("<tr><td align=right valign=top><img src=/coco.gif></td>\n");
	printdiv(&div, "��������");
	printf("&nbsp;&nbsp;<a target=_BLANK href='http://www.xjtu.edu.cn/' class=linkleft>������ҳ</a><br>\n");
	printf("&nbsp;&nbsp;<a target=_BLANK href='http://nic.xjtu.edu.cn/' class=linkleft>��������</a><br>\n");
	printf("</div></td></tr>\n");
//lanboy add ads here
//        printf("<br><a href='http://cn.rd.yahoo.com/auct/promo/bmy/200501/hp/evt=29631/*http://cn.auctions.yahoo.com/?refcode=bmy-hp' target=_BLANK>��<IMG src=/pic80x60.gif width=90 height=180 border=0></a>");
	printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
		"<td><a class=linkleft href=\"%sBBShelp\" target=f3>�û�����</a>\n"
		"</td></tr>\n", showByDefMode() );
	printf("<tr><td align=right> <img src=\"/images/list2.gif\"></td>\n"
		"<td><a class=linkleft href=\"bbspstmail?userid=SYSOP\" target=f3>���Ÿ�վ��</a>\n"
		"</td></tr>\n");
	printf("</table>\n");
	printf("<table width=124>"
	       "<tr><form><td>&nbsp;&nbsp;<input type=button style='width:90px' value='���ز˵�' "
	       "onclick=\"{if(switchAutoHide()==true) {this.value='ֹͣ�Զ�����';}"
	       "else this.value='���ز˵�';return false;}\">"
	       "</td></form></tr></table>\n");

// add by interma announce log picture
//        printf("<table width=124>"             "<tr><td>"                  "<a href='http://kxfz.xjtu.edu.cn/'target='_blank'><img src='/images/bbs-link.jpg' width=120 height=44 border=0 /></a>"          "</td></tr></table>\n");

       // printf("<table width=124>"             "<tr><td>"                  "<a href='http://meeting.xjtu.edu.cn/'target='_blank'><img src='/images/xueshu.jpg' width=120 height=44 border=0 /></a>"            "</td></tr></table>\n");


      // printf("<table width=124>"             "<tr><td>"                  "<a href='http://t.qq.com/k/%%25E5%%258F%%2591%%25E7%%258E%%25B0%%25E6%%2590%%259C%%25E6%%2590%%259C%%25EF%%25BC%%258C%%25E5%%258F%%2591%%25E7%%258E%%25B0%%25E7%%25B2%%25BE%%25E5%%25BD%%25A9' target='_blank'> <img src='/images/jstw.jpg' width=120 height=44 border=0 /></a>"           "</td></tr></table>\n");

//      printf("<table width=124>"             "<tr><td>"                  "<a href='http://bbs.xjtu.edu.cn/BMY/con?B=job&F=M.1318923353.A&N=50453&T=0' target='_blank'> <img src='/images/libai.jpg' width=120 height=44 border=0 /></a>"           "</td></tr></table>\n");

//      printf("<table width=124>"             "<tr><td>"                  "<a href='http://bbs.xjtu.edu.cn/BMY/con?B=job&F=M.1319087095.A&N=47877&T=0' target='_blank'> <img src='/images/harman.gif' width=120 height=44 border=0 /></a>"           "</td></tr></table>\n");

//      printf("<table width=124>"             "<tr><td>"                  "<a href='http://bbs.xjtu.edu.cn/BMY/con?B=job&F=M.1319526683.A&N=49268&T=0' target='_blank'> <img src='/images/nokia.jpg' width=120 height=44 border=0 /></a>"           "</td></tr></table>\n");

//      printf("<table width=124>"             "<tr><td>"                  "<a href='http://bbs.xjtu.edu.cn/BMY/con?B=job&F=M.1319184956.A&N=48175&T=0' target='_blank'> <img src='/images/rskd.jpg' width=120 height=44 border=0 /></a>"           "</td></tr></table>\n");


     //   printf("<table width=124>"             "<tr><td>"                  "<a href='http://kxfz.xjtu.edu.cn/'target='_blank'><img src='/images/bbs-link.jpg' width=120 height=44 border=0 /></a>"          "</td></tr></table>\n");

	fp = fopen("etc/ad_left", "r");
	if(!fp){
		//printf("fail to open\n");
		goto endleft;
	}
	bzero(buf, 512);
	while(fgets(buf, 512, fp)){
		strltrim(strrtrim(buf));
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
			"function window.onbeforeunload(){\n"
			"  if(event.clientX>document.body.clientWidth&&event.clientY<0||event.altKey){\n"
			"return 'ֱ�ӹر��������������վʱ�䣬ǿ�ҽ������㡰ע�����ε�¼����'}}\n"
			"</script>\n");
//add by macintosh 20051216, end
		if (HAS_PERM(PERM_LOGINOK) && !HAS_PERM(PERM_POST))
			printf
			    ("<script>alert('���������ȫվ�������µ�Ȩ��, ��ο�sysop�湫��, ��������sysop��������. ��������, ����committee���������.')</script>\n");
		mails(currentuser.userid, &i);
		if (i > 0)
			printf("<script>alert('�������ż�!')</script>\n");
	}
	// if(loginok&&currentuser.userdefine&DEF_ACBOARD)
	//              printf("<script>window.open('bbsmovie','','left=200,top=200,width=600,height=240');</script>"); 
	//virusalert();
	if (isguest && 0)
		printf
		    ("<script>setTimeout('open(\"regreq\", \"winREGREQ\", \"width=600,height=460\")', 1800000);</script>");
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
//		       inet_ntoa(wwwcache->accel_addr), wwwcache->accel_port);
	printf("</body></html>");



/*	printf("<script src=\"inc/func.js\"></script>\n"
		"<script language="JavaScript" type="text/JavaScript">\n
		<!--
		function MM_preloadImages() { //v3.0\n
			var d=document; if(d.images){ if(!d.MM_p) d.MM_p=new Array();\n
			var i,j=d.MM_p.length,a=MM_preloadImages.arguments; for(i=0; i<a.length; i++)\n
			if (a[i].indexOf("#")!=0){ d.MM_p[j]=new Image; d.MM_p[j++].src=a[i];}}\n
		}
		//-->
		</script>\n"
	       "<body leftmargin=1 topmargin=1 MARGINHEIGHT=1 MARGINWIDTH=1 background=%s>\n",
	       currstyle->lbg);
	printf("<nobr>");
	if (!loginok || isguest) {
		printf("<table>\n");
		printf
		    ("<form action=bbslogin method=post target=_top><tr><td><center>"
		     "�û���¼<br>"
		     "�ʺ�<input type=text name=id maxlength=12 size=8><br>"
		     "����<input type=password name=pw maxlength=12 size=8><br>"
		     "��Χ<select name=ipmask>\n"
		     "<option value=0 selected>��IP</option>\n"
		     "<option value=1>2 IP</option>\n"
		     "<option value=2>4 IP</option>\n"
		     "<option value=3>8 IP</option>\n"
		     "<option value=4>16 IP</option>\n"
		     "<option value=5>32 IP</option>\n"
		     "<option value=6>64 IP</option>\n"
		     "<option value=7>128 IP</option>\n"
		     "<option value=8>256 IP</option></select><br>"
		     "<a href=/ipmask.html target=_blank>����ʲô?</a>\n<br>"
		     "<input type=submit value=��¼>&nbsp;"
		     "<input type=submit value=ע�� onclick=\"{openreg();return false}\">\n"
		     "</center></td></tr></form></table>");
	} else {
		char buf[256] = "δע���û�";
		printf("�û�: <a href=bbsqry?userid=%s target=f3>%s</a><br>",
		       currentuser.userid, currentuser.userid);
		if (currentuser.userlevel & PERM_LOGINOK)
			strcpy(buf, charexp(countexp(&currentuser)));
		if (currentuser.userlevel & PERM_BOARDS)
			strcpy(buf, "����");
		if (currentuser.userlevel & PERM_XEMPT)
			strcpy(buf, "�����ʺ�");
		if (currentuser.userlevel & PERM_SYSOP)
			strcpy(buf, "��վվ��");
		printf("����: %s<br>", buf);
		printf("<a href=bbslogout target=_top>ע�����ε�¼</a><br>\n");
	}
	printf("<hr>");
	//printf("&nbsp;&nbsp;<a href=http://162.105.153.249 target=_blank><font color=red>���ﶩվ��</font></a><br>");

	printf("&nbsp;&nbsp;<a target=f3 href=boa?secstr=?>" MY_BBS_ID
	       "����</a><br>\n");
	printf("&nbsp;&nbsp;<a target=f3 href=bbs0an>����������</a><br>\n");
	printf("&nbsp;&nbsp;<a target=f3 href=bbstop10>ʮ�����Ż���</a><br>\n");
	//by bjgyt printf
	    ("&nbsp;&nbsp;<a target=f3 href=bbsshownav?a1=class&a2=all>���վ��ʻ���</a><br>\n");
	if (loginok && !isguest) {
		char *ptr, buf[10];
		struct boardmem *x1;
		int mybrdmode;
		readuservalue(currentuser.userid, "mybrdmode", buf,
			      sizeof (buf));
		mybrdmode = atoi(buf);
		printdiv(&div, "Ԥ��������");
		readmybrd(currentuser.userid);
		for (i = 0; i < mybrdnum; i++) {
			ptr = mybrd[i];
			if (!mybrdmode) {
				x1 = getboard(mybrd[i]);
				if (x1)
					ptr =
					    nohtml(titlestr(x1->header.title));
			}
			printf
			    ("&nbsp;&nbsp;<a target=f3 href=tdoc?B=%s>%s</a><br>\n",
			     mybrd[i], ptr);
		}
		printf
		    ("&nbsp;&nbsp;<a target=f3 href=bbsboa?secstr=*>Ԥ��������</a><br>\n");
		printf
		    ("&nbsp;&nbsp;<a target=f3 href=bbsmybrd?mode=1>Ԥ������</a><br>\n");
		printf("</div>\n");
	}
	printdiv(&div, "����������");
	printsectree(&sectree);
	printf("</div>\n");
	printf("<div class=r>");
	for (i = 0; i < sectree.nsubsec; i++) {
		const struct sectree *sec = sectree.subsec[i];
		if (!sec->nsubsec)
			continue;
		printf("--<a target=f3 href=bbsboa?secstr=%s>%s</a><br>\n",
		       sec->basestr, sec->title);
	}
	printf("</div>\n");
	printdiv(&div, "̸��˵��");
	if (loginok && !isguest) {
		printf
		    ("&nbsp;&nbsp;<a href=bbsfriend target=f3>���ߺ���</a><br>\n");
	}
	printf
	    ("&nbsp;&nbsp;<a href=bbsufind?search=A&limit=20 target=f3>�����ķ�</a><br>\n");
	printf("&nbsp;&nbsp;<a href=bbsqry target=f3>��ѯ����</a><br>\n");
//      printf("&nbsp;&nbsp;<a target=f3 href=bbsalluser>����ʹ����</a><br>\n");
	if (currentuser.userlevel & PERM_PAGE) {
		printf
		    ("&nbsp;&nbsp;<a href=bbssendmsg target=f3>����ѶϢ</a><br>\n");
		printf
		    ("&nbsp;&nbsp;<a href=bbsmsg target=f3>�鿴����ѶϢ</a><br>\n");
	}
	printf("</div>\n");
	if (loginok && !isguest) {
		printdiv(&div, "���˹�����");
		printf("&nbsp;&nbsp;<a target=f3 href=bbsinfo>��������</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbsplan>��˵����</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbssig>��ǩ����</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbspwd>�޸�����</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbsparm>�޸ĸ��˲���</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbsmywww>WWW���˶���</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbsnick>��ʱ���ǳ�</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbsstat>����ͳ��</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbsfall>�趨����</a><br>");
		if (currentuser.userlevel & PERM_CLOAK)
			printf("&nbsp;&nbsp;<a target=f3 "
			       "onclick='return confirm(\"ȷʵ�л�����״̬��?\")' "
			       "href=bbscloak>�л�����</a><br>\n");
		printf("</div>");
		printdiv(&div, "�����ż�");
		printf("&nbsp;&nbsp;<a target=f3 href=bbsnewmail>���ʼ�</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbsmail>�����ʼ�</a><br>"
		       "&nbsp;&nbsp;<a target=f3 href=bbspstmail>�����ʼ�</a><br>"
		       "</div>");
	}
	printdiv(&div, "�ر����");
	//printf("&nbsp;&nbsp;<a target=f3 href=bbssechand>�����г�</a><br>\n");
	printf("&nbsp;&nbsp;<a target=f3 href=/wnl.html>������</a><br>\n");
	//printf
	    ("&nbsp;&nbsp;<a target=f3 href=/cgi-bin/cgincce>�Ƽ��ʵ�</a><br>\n");
	printf
	    ("&nbsp;&nbsp;<a target=f3 href=/scicalc.html>��ѧ������</a><br>\n");
	//printf
	    ("&nbsp;&nbsp;<a target=f3 href=/periodic/periodic.html>Ԫ�����ڱ�</a><br>\n");
	//printf
	    ("&nbsp;&nbsp;<a target=f3 href=/cgi-bin/cgiman>Linux�ֲ��ѯ</a><br>\n");
	printf("&nbsp;&nbsp;<a href=bbsfind target=f3>���²�ѯ</a><br>\n");
	//printf
	    ("&nbsp;&nbsp;<a target=f3 href=/cgi-bin/cgifreeip>IP��ַ��ѯ</a><br>\n");
	printf("&nbsp;&nbsp;<a target=f3 href=bbsx?chm=1>���ؾ�����</a><br>\n");
	printf
	    ("&nbsp;&nbsp;<a target=f3 href=home/pub/index.html>��������</a><br>\n");
	printf("</div>\n");
	printf("<div class=r>");
	printf
	    ("&nbsp;&nbsp;<a target=f3 href=bbs0an?path=/groups/GROUP_0/PersonalCorpus>�����ļ���</a><br>\n");
	printf("&nbsp;&nbsp;<a target=f3 href=bbsall>����������</a><br>\n");
	printf("<hr>");
	printf("<table><tr><form action=home target=f3><td>&nbsp;&nbsp;"
	       "<input type=text name=board maxlength=20 "
	       "size=9 value=ѡ�������� onclick=\"this.select()\"></td></form></tr></table>\n");
	printf("&nbsp;&nbsp;<a href='telnet:%s'>Telnet��¼</a>\n", BBSHOST);
	if (!loginok || isguest)
		printf
		    ("<br>&nbsp;&nbsp;<a href=\"javascript: openreg()\">���û�ע��</a>\n");
	if (loginok && !isguest && !(currentuser.userlevel & PERM_LOGINOK)
	    && !has_fill_form())
		printf
		    ("<br>&nbsp;&nbsp;<a target=f3 href=bbsform><font color=red>��дע�ᵥ</font></a>\n");
	if (loginok && !isguest && HAS_PERM(PERM_ACCOUNTS))
		printf
		    ("<br>&nbsp;&nbsp;<a href=bbsscanreg target=f3>SCANREG</a>");
	if (loginok && !isguest && HAS_PERM(PERM_SYSOP))	//add by mintbaggio@BMY for www SYSOP kick www user
		printf("<br>&nbsp;&nbsp;<a href=kick target=f3>��www��վ</a>");
	//if(loginok && !isguest) printf("<br>&nbsp;&nbsp;<a href='javascript:openchat()'>bbs���</a>");
	printf
	    ("<br>&nbsp;&nbsp;<a href=bbsselstyle target=f3>�������濴��</a>");
	//printf ("<br>&nbsp;&nbsp;<a href='http://bug.ytht.org/' target=_BLANK>���� Bug</a>");
	printf("<br><br><center><img src=/coco.gif>");
	printf("<br><br><b>��������</b>");
	printf("<br><br><a href='http://bbs.xsyu.edu.cn/' target=_BLANK>����ʯ��ѧԺbbs</a>");
	printf("</div>");
	printf("<script>if(isNS4) arrange();if(isOP)alarrangeO();</script>");
	if (loginok && !isguest) {
		if (HAS_PERM(PERM_LOGINOK) && !HAS_PERM(PERM_POST))
			printf
			    ("<script>alert('���������ȫվ�������µ�Ȩ��, ��ο�sysop�湫��, ��������sysop��������. ��������, ����Appeal���������.')</script>\n");
		mails(currentuser.userid, &i);
		if (i > 0)
			printf("<script>alert('�������ż�!')</script>\n");
	}
	// if(loginok&&currentuser.userdefine&DEF_ACBOARD)
	//              printf("<script>window.open('bbsmovie','','left=200,top=200,width=600,height=240');</script>"); 
	//virusalert();
	if (isguest && 0)
		printf
		    ("<script>setTimeout('open(\"regreq\", \"winREGREQ\", \"width=600,height=460\")', 1800000);</script>");
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
		printf("<script src=/testdoc.js></script>",
		       inet_ntoa(wwwcache->accel_addr), wwwcache->accel_port);
*/	printf("</body></html>");
	return 0;
}

/*
 * void
virusalert()
{
	if (file_has_word("virusalert.txt", fromhost)) {
		printf
		    ("<script>window.open('/virusalert.html','','left=200,top=200,width=250,height=80');</script>");
	}
}
*/
