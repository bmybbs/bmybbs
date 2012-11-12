#include "bbslib.h"
#include "tmpl.h"

static int
seek_in_file(filename, seekstr)
char filename[STRLEN], seekstr[STRLEN];
{
	FILE *fp;
	char buf[STRLEN];
	char *namep;

	if ((fp = fopen(filename, "r")) == NULL)
		return 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		namep = (char *) strtok(buf, ": \n\r\t");
		if (namep != NULL && strcasecmp(namep, seekstr) == 0) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

int
bbspst_main()
{
	FILE *fp;
	int local_article, i, num, fullquote = 0, guestre = 0, thread = -1;
	char userid[80], buf[512], path[512], file[512], board[512], title[80] =
	    "";
	struct fileheader *dirinfo = NULL;
	struct boardmem *x;
	char bmbuf[IDLEN * 4 + 4];		//add by mintbaggio 040807 for new www
	struct mmapfile mf = { ptr:NULL };
	html_header(1);
	check_msg();
	strsncpy(board, getparm("B"), 32);
	if(strcmp(board, "welcome")){	///modify by mintbaggio 040614 for guest post at board "welcome"
		if (!loginok) {
			printf("<script src=/function.js></script>\n");
			printf("匆匆过客不能发表文章，请先登录!<br><br>");
			printf("<script>openlog();</script>");
			http_quit();
		}
	}
	else if (seek_in_file(MY_BBS_HOME"/etc/guestbanip", fromhost) && !loginok)
		http_fatal("您的ip被禁止使用guest在本版发表文章!″");
	local_article = 1; // modified by linux @ 2006.6.6 for the default post status to no outgo
//	strsncpy(board, getparm("B"), 32);
	if (!board[0])
		strsncpy(board, getparm("board"), 20);
	strsncpy(file, getparm("F"), 20);
	if (!file[0])
		strsncpy(file, getparm("file"), 20);
	fullquote = atoi(getparm("fullquote"));
	if (file[0] != 'M' && file[0])
		http_fatal("错误的文件名");
	if (!(x = getboard(board)))
		http_fatal("错误的讨论区或者您无权在此讨论区发表文章");
	if (file[0]) {
		num = atoi(getparm("num"));
		sprintf(path, "boards/%s/.DIR", board);
		MMAP_TRY {
			if (mmapfile(path, &mf) == -1) {
				MMAP_UNTRY;
				http_fatal("错误的讨论区");
			}
			dirinfo = findbarticle(&mf, file, &num, 1);
		}
		MMAP_CATCH {
			dirinfo = NULL;
		}
		MMAP_END mmapfile(NULL, &mf);
		if (dirinfo) {
			thread = dirinfo->thread;
			//if (dirinfo->accessed & FH_ALLREPLY)
			//	guestre = 1;
			strsncpy(userid, fh2owner(dirinfo), 20);
			if (strncmp(dirinfo->title, "Re: ", 4))
			  {
				snprintf(title, 60, "Re: %s", dirinfo->title);
				local_article = atoi(getparm("la")); // added by linux @2006.6.6 for the post status to the status of the article before when doing a reply post
			  }
			else
				strsncpy(title, dirinfo->title, 60);
		} else
			http_fatal("错误的文件名");
		if (dirinfo->accessed & FH_NOREPLY)
			http_fatal("本文被设为不可Re模式");

	}
	if (!has_post_perm(&currentuser, x) && !isguest) 
	{
		  if (x->header.secnumber2=='C')
			http_fatal("俱乐部版面，请联系版主，申请加入俱乐部方能发文.");
		  else
		  	http_fatal("错误的讨论区或者您无权在此讨论区发表文章");
	}
	if (noadm4political(board))
		http_fatal("对不起,因为没有版面管理人员在线,本版暂时封闭.");
	if(strcmp(board, "welcome")){	//add by mintbaggio 040614 for guest post at "welcome"
		if (isguest && !guestre) {
			printf("<script src=/function.js></script>\n");
			printf("匆匆过客不能发表文章，请先登录!<br><br>");
			printf("<script>openlog();</script>");
			http_quit();
		}
	}
	else if (seek_in_file(MY_BBS_HOME"/etc/guestbanip", fromhost) && !guestre)
		http_fatal("您的ip被禁止使用guest在本版发表文章!″");
	changemode(POSTING);
//	printf("<body><center>\n");
	printf("<body leftmargin=0 topmargin=0>\n");
	printf("<table width=\"100%\" border=0 cellpadding=0 cellspacing=0>\n");
	printf("%s", "<tr>\n<td height=30 colspan=2>\n" 
		"<table width=\"100%\"  border=0 cellspacing=0 cellpadding=0>\n"
        	"<tr><td width=40><img src=\"/images/spacer.gif\" width=40 height=10 alt=\"\"></td>\n"
		"<td><table width=\"100%\" border=0 align=right cellpadding=0 cellspacing=0>\n"
		"<tr><td>\n");
	printf("<a href=\"boa?secstr=%s\">%s</a> / <a href=\"%s%s\">%s版</a> / 发表文章 </td>\n"
		"</tr></table></td>\n<td><table border=0 align=right cellpadding=0 cellspacing=0>\n"
		"<tr><td> 版主 %s \n"
		"</td></tr></table></td></tr></table></td></tr>\n", 
		x->header.sec1,nohtml(getsectree(x->header.sec1)->title), showByDefMode(), board, board, userid_str(bm2str(bmbuf, &(x->header))));
//	printf("%s -- 发表文章 [使用者: %s]<hr>\n", BBSNAME, currentuser.userid);
	if (x->header.flag & IS1984_FLAG)
		printf("<tr><td height=30 colspan=2><font color=red>请注意，本文发表后需通过审查</font></td></tr>");
 	printf("%s", "<tr><td height=70 colspan=2>\n"
 		"<table width=\"100%\" height=\"100%\" border=0 cellpadding=0 cellspacing=0 bgcolor=\"#efefef\">\n"
		"<tr><td width=40>&nbsp; </td>\n"
		"<td height=70><table width=\"95%\" height=\"100%\"  border=0 cellpadding=0 cellspacing=0>\n"
		"<tr>\n");
 	printf("<td valign=bottom>\n"
 		"<table width=\"100%\" border=0 cellpadding=0 cellspacing=0>\n"
		"<tr><td class=F0002><div class=\"menu\">\n"
		"<DIV class=btncurrent>&lt;%s&gt;</DIV>\n"
		"<DIV><A class=btnfunc href=\"%s%s\" title=\"返回讨论区 accesskey: b\" accesskey=\"b\">/ 返回讨论区</A></DIV>\n"
		"<DIV style=\"width:10px\" class=N1001></DIV>\n"
		"</div></td></tr></table></td></tr>\n", void1(titlestr(x->header.title)), showByDefMode(), board);
	printf("<tr><td width=\"100%\"><table  border=0 cellspacing=0 cellpadding=0>\n"
		"<tr><td> 发文注意事项: <br>\n"
		"发文时应慎重考虑文章内容是否适合公开场合发表，请勿肆意灌水。谢谢您的合作。 <br>"
		"本站规定同样内容的文章严禁在 4 个或 4 个以上讨论区内重复发表。违者将被封禁在本站发文的权利。<br>"
		"如需一文多发，请移步<a target=f3 href='home?B=sysop'> SYSOP </a>区申请一下。"
		"</td>\n"
		"</tr></table></td></table></td></tr>\n");
/*	printf("<table border=1>\n");
	printf("<tr><td>");
	printf("<font color=green>发文注意事项: <br>\n");
	printf
	    ("发文时应慎重考虑文章内容是否适合公开场合发表，请勿肆意灌水。谢谢您的合作。<br>\n</font>");
*/	if (file[0])
		snprintf(buf, sizeof (buf), "&ref=%s&rid=%d", file, num);
/*	printf("<tr><td><form name=form1 method=post action=bbssnd?board=%s&th=%d%s>\n",
	       board, thread, file[0] ? buf : "");
	printf
	    ("使用标题: <input type=text name=title size=40 maxlength=100 value='%s'>",
	     (void1(noquote_html(title))));
	printf(" 讨论区: [%s]\n", board);
*/
	printf("<tr><td width=40 class=\"level1\"></td>\n"
		"<td class=\"level1\"><br>\n"
		"<TABLE width=\"95%\" cellpadding=5 cellspacing=0>\n"
		"<TBODY><TR><TD class=tdtitletheme>&nbsp;</TD>\n"
		"</TR>\n");
	int hastmpl;
	char tmplfile[STRLEN];
	sprintf(tmplfile, "boards/%s/%s", board, ".tmpl");
	if (fopen(tmplfile, "r") == 0)
		hastmpl = 0;
	else
		hastmpl = file_size(tmplfile) / sizeof (struct a_template);
	if (!file[0] && hastmpl > 0) {
		printf("<tr><td class=bordertheme>");
		printf("<a target=_self href=bbstmpl?action=show&board=%s class=btnsubmittheme>", board);
		printf("模板发文</a>");
		printf("</td></tr>\n");		
	}
	printf("<TR><TD class=bordertheme>\n"
		"<form name=form1 method=post action=bbssnd?board=%s&th=%d%s>\n",
		board, thread, file[0] ? buf : "");
	printf("<table width=\"100%\"  border=0 cellspacing=0 cellpadding=0>\n"
		"<tr>\n<td><table border=0 cellpadding=0 cellspacing=0>\n"
		"<tr><td> 使用标题: </td>\n");
	
		//ArthurF修改部分开始 
		//预计实现功能 www下的标题长度限制 标题将通过js限制在45个英文和22个汉字之内
		//失去焦点的时候进行统计 超过则弹出提示框 要求修改
		if (file[0]){
			printf("<td><input name=title type=text class=inputtitle maxlength=45 size=50 value='%s'></td>\n", (void1(noquote_html(title))));
		}
		else{	
			printf("<script language=\"JavaScript\">\n"
					"function realLen(v){\n"
					"  l=0;\n"
					"  for (var i=0;i<v.length;i++){\n"
					"    if (Math.abs(v.charCodeAt(i))>255)\n"
					"      l+=2;\n"
					"    else\n"
					"      l++;\n"
					"  }\n"
					"  return l;}\n"
					"</script>\n"
					"\n"
					"<td><input id=edittitle name=title type=text class=inputtitle maxlength=45 size=50 value='%s' ></td>\n"
					, (void1(noquote_html(title))));
			printf("<script language=\"JavaScript\">\n"
			        " document.getElementById(\"edittitle\").focus(); \n"
			        "  </script>");
		}
			//修改部分结束 

	printf("<td height=20>\n"
		" 讨论区: [%s]</td>\n"
		"</tr></table></td></tr>\n", board);
	printf("%s", "<tr><td><table border=0 cellpadding=0 cellspacing=0><tr>\n");
	printf("<td> 作者：%s &nbsp</td>\n", currentuser.userid);
/*	if (innd_board(board))
		printf("转信<input type=checkbox name=outgoing %s>\n",
		       local_article ? "" : "checked");
	if (anony_board(board))
		printf("匿名<input type=checkbox name=anony>\n");
	printf("<br>作者：%s &nbsp;", currentuser.userid);
*/
	printselsignature();
	printuploadattach();
	if (innd_board(board))
                printf("转信<input type=checkbox name=outgoing %s>\n",
                       local_article ? "" : "checked");
        if (anony_board(board))
                printf("匿名<input type=checkbox name=anony>\n");
	printf("</td></tr></table></td></tr>\n");
	//printusemath(0);
	printf("<tr><td>\n");
	printf("使用Tex风格的数学公式<input type=checkbox name=usemath>\n");
	printf("设为不可回复<input type=checkbox name=nore>\n");
	printf("回复抄送至信箱<input type=checkbox name=mailback>\n");
	if (file[0])
		if (dirinfo->accessed & FH_MAILREPLY)
			printf("<input type=hidden name=replyto value=%s>\n", userid);
	printf("</td></tr>\n");
	if (file[0]) {
		printf("<tr><td>引文模式: %s ", fullquote ? "完全" : "精简");
		printf
		    ("[<a target=_self href=bbspst?inframe=1&board=%s&file=%s&num=%d&la=%d",
		     board, file, num, local_article);
		printf("&fullquote=%d>切换为%s模式</a> (将丢弃所更改内容)]",
		       !fullquote, (!fullquote) ? "完全" : "精简");
		printf("</td></tr>\n");
	}
	printf
	    ("<tr><td><textarea id=textedit onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.form1.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.form1.submit()' name=text rows=20 cols=76 wrap=virtual class=f2 >\n\n");
	if (file[0]) {
		int lines = 0;
		printf("【 在 %s 的大作中提到: 】\n", userid);
		sprintf(path, "boards/%s/%s", board, file);
		fp = fopen(path, "r");
		if (fp) {
			for (i = 0; i < 3; i++)
				if (fgets(buf, 500, fp) == 0)
					break;
			while (1) {
				if (fgets(buf, 500, fp) == 0)
					break;
				if (!strncmp(buf, ": 【", 4))
					continue;
				if (!strncmp(buf, ": : ", 4))
					continue;
				if (!strncmp(buf, "--\n", 3)
				    || !strncmp(buf, "begin 644 ", 10)
				    || !strncmp(buf, "beginbinaryattach ", 18))
					break;
				if (buf[0] == '\n')
					continue;
				if (!fullquote && lines++ >= 10) {
					printf(": (以下引言省略...)\n");
					break;
				}
				printf(": %s", nohtml(buf));
			}
			fclose(fp);
		}
	}
	printf("</textarea></td></tr>\n");
	if(file[0])
		printf("<script language=\"JavaScript\">\n"
			   " document.getElementById(\"textedit\").focus(); \n"
			   "  </script>");
	
		
/*	printf
	    ("<tr><td class=post align=center><input type=submit value=发表> ");
	printf("<input type=reset value=清除></form>\n");
	printf("</table>");
*/
	printf("%s", "<tr><td><input name=Submit2 type=submit class=resetlong value=\"发表\" "
			"onclick=\"if (realLen(value)>45){alert('文章标题长度不能超过22个汉字或45个英文长度,否则会丢失信息,请修改文章标题.');this.focus();return false}else{this.value='文章提交中，请稍候...';this.disabled=true;form1.submit()}\">\n"
			"<input name=Submit3 type=reset class=sumbitlong value=\"清除\" onclick='return confirm(\"确定要全部清除吗?\")'></td>\n"
			"</tr>\n");
	printf("%s", "</table></TD></TR></TBODY></TABLE></form></td></tr>\n"
		"<tr>\n<td height=40 bgcolor=\"#FFFFFF\">　</td>\n"
		"<td height=40 bgcolor=\"#FFFFFF\">　</td>\n"
		"</tr></table></td></tr></table>\n");
	http_quit();
	return 0;
}

void
printselsignature()
{	//modify by mintbaggio 040809 for new www
	int i, sigln, numofsig;
	char path[200];
	sprintf(path, "home/%c/%s/signatures",
		mytoupper(currentuser.userid[0]), currentuser.userid);
	sigln = countln(path);
	numofsig = (sigln + MAXSIGLINES - 1) / MAXSIGLINES;
	printf("%s", "<td>使用签名档:</td><td height=20> <select name=\"signature\" class=2015>\n");
	if (currentuser.signature == 0)
		printf("<option value=\"0\" selected>不使用签名档</option>\n");
	else
		printf("<option value=\"0\">不使用签名档</option>\n");
	if (numofsig>0) {
		if (currentuser.signature == -1)
			printf("<option value=\"-1\" selected>使用随机签名档</option>\n");
		else
			printf("<option value=\"-1\">使用随机签名档</option>\n");
	}
	for (i = 1; i <= numofsig; i++) {
		if (currentuser.signature == i)
			printf
			    ("<option value=\"%d\" selected>第 %d 个</option>\n",
			     i, i);
		else
			printf("<option value=\"%d\">第 %d 个</option>\n", i,
			       i);
	}
	printf("</select></td>\n");
	printf("%s", "<td> [<a target=_blank href=bbssig>查看签名档</a>]");
}

void
printuploadattach()
{	//modify by mintbaggio 040809 for new www
	int i = u_info - &(shm_utmp->uinfo[0]);
	printf
	    (" [<a href=/cgi-bin/bbs/upload/%c%c%c%s target=uploadytht>添加/删除附件</a>]\n",
	     i / (26 * 26) + 'A', i / 26 % 26 + 'A', i % 26 + 'A',
	     u_info->sessionid);
}

void
printusemath(int checked)
{
	printf
	    ("<tr><td>使用Tex风格的数学公式<input type=checkbox name=usemath%s>\n",
	     checked ? " checked" : "");
	//printf
	//    (" <a href=home/boards/BBSHelp/html/itex/itexintro.html target=_blank>这是什么？</a></td></tr>\n");
	printf
	    ("设为不可回复<input type=checkbox name=nore%s></td></tr>\n",
	     checked ? " checked" : "");
}
