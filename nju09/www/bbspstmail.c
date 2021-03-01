#include "bbslib.h"

int
bbspstmail_main()
{	//modify by mintbaggio 040821 for new www
	FILE *fp;
	int i, num, fullquote = 0;
	char mymaildir[80], userid[80], buf[512], path[512], file[20], board[40], title[80] = "", buff[512];
	struct fileheader *dirinfo;
	struct mmapfile mf = { .ptr = NULL };
	html_header(1);
	check_msg();
	if (!loginok)
		http_fatal("匆匆过客不能写信，请先登录");
	if (!((currentuser.userlevel )& (PERM_CHAT|PERM_PAGE|PERM_POST)))
		http_fatal("您没有权限发信");
	if (HAS_PERM(PERM_DENYMAIL, currentuser))
		http_fatal("您被封禁发信权");
	sprintf(mymaildir, "mail/%c/%s/.DIR", mytoupper(currentuser.userid[0]),
		currentuser.userid);
	if (check_maxmail()){
		sprintf(buff,"出错原因: 您的私人信件总大小高达 %d k,超过 %d k 时 ,您将无法使用本站的发信功能.<br>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp 请整理信件,将信件总大小控制在 %d k 内,以保证发信功能正常使用",get_mail_size(),max_mail_size()+20,max_mail_size());
		http_fatal(buff);
		}
	changemode(SMAIL);
	ytht_strsncpy(board, getparm("B"), 32);
	if (!board[0])
		ytht_strsncpy(board, getparm("board"), 32);
	if (board[0] && !getboard(board))
		http_fatal("错误的讨论区");
	ytht_strsncpy(file, getparm("F"), sizeof(file));
	if (!file[0])
		ytht_strsncpy(file, getparm("file"), sizeof(file));
	if (file[0] != 'M' && file[0])
		http_fatal("错误的文件名");
	if (file[0]) {
		fullquote = atoi(getparm("fullquote"));
		num = atoi(getparm("num"));
		buf[0] = '\0';
		if (board[0])
			sprintf(buf, "boards/%s/.DIR", board);
		else if (loginok && !isguest)
			sprintf(buf, "mail/%c/%s/.DIR",
				mytoupper(currentuser.userid[0]),
				currentuser.userid);
		if ('\0' == buf[0])
			dirinfo = NULL;
		else {
			MMAP_TRY {
				if (mmapfile(buf, &mf) == -1)
					dirinfo = NULL;
				else {
					dirinfo = findbarticle(&mf, file, &num, 1);
				}
			}
			MMAP_CATCH {
				dirinfo = NULL;
			}
			MMAP_END mmapfile(NULL, &mf);
		}
		if (dirinfo) {
			ytht_strsncpy(userid, dirinfo->owner, sizeof(userid));
			if (strchr(userid, '.')) {
				if (board[0])
					sprintf(buf, "boards/%s/%s", board,
						fh2fname(dirinfo));
				else
					setmailfile_s(buf, sizeof(buf), currentuser.userid, fh2fname(dirinfo));
				getdocauthor(buf, userid, sizeof (userid));
			}
			if (strncmp(dirinfo->title, "Re: ", 4))
				snprintf(title, sizeof(title), "Re: %s", dirinfo->title);
			else
				ytht_strsncpy(title, dirinfo->title, sizeof(title));
		} else
			http_fatal("错误的文件名");
	} else
		ytht_strsncpy(userid, getparm("userid"), 20);
	if (isguest && strcmp(userid, "SYSOP"))
		http_fatal("匆匆过客不能写信，请先登录");

	printf("<script language=\"javascript\">\n");
	printf("	function chguserid(){\n");
	printf("	if(document.form1.allfriend.checked==true){\n");
	printf("		document.form1.userid.value='所有好友';\n");
	printf("		document.form1.userid.disabled=true;\n");
	printf("	}else {\n");
	printf("		document.form1.userid.value='';\n");
	printf("		document.form1.userid.disabled=false;\n");
	printf("		document.form1.userid.focus();\n");
	printf("	}\n");
	printf("}\n");
	printf("</script>\n");
	printf("</head>");
//	printf("<body><center>\n");
/*	printf("%s -- 寄语信鸽 [使用者: %s]<hr>\n", BBSNAME, currentuser.userid);
	printf("<table border=1><tr><td>\n");
	printf("<form name=form1 method=post action=bbssndmail?userid=%s>\n", userid);
	printf("信件标题: <input type=text name=title size=40 maxlength=100 value='%s'><br>", (noquote_html(title)));
	printf("发信人: %s<br>\n", currentuser.userid);
	printf("收信人: <input type=text name=userid value='%s'> 发送给所有好友<input type=checkbox name=allfriend onclick=chguserid();>[<a href=bbsfall target=_blank>查看好友名单</a>]<br>", nohtml(userid));
	printselsignature();
	printuploadattach();
	printf(" 备份<input type=checkbox name=backup>\n");
*/
	printf("<body>\n");

	/* 使用者ID */
	printf("<div style=\"width: 100%%; height: 24px; text-align: center;\" class=\"level2\">\n");
	printf("<p style=\"line-height: 24px;\">%s -- 寄语信鸽 [使用者: %s]</p>\n", BBSNAME, currentuser.userid);
	printf("</div>\n");

	printf("<form style=\"margin-left: 20px; margin-top: 10px;\" name=\"form1\" method=\"post\" action=\"bbssndmail?userid=%s\">\n", userid);
	printf("<table>\n");

	/* 一个pp的颜色条 */
	printf("	<tr class=\"tdtitletheme\" style=\"height: 20px;\">\n");
	printf("		<td colspan=\"3\">&nbsp;</td>\n");
	printf("	</tr>\n");

	/* 标题的一行 */
	printf("	<tr>\n");
	printf("		<td>信件标题:</td>\n");
	printf("		<td colspan=\"2\"><input id=titleedit name=\"title\" type=\"text\" size=\"40\" value=\"%s\"></td>\n", (noquote_html(title)));
	printf("	</tr>\n");

	/* 发信人的一行 */
	printf("	<tr height=\"20px\">\n");
	printf("		<td>发信人:</td>\n");
	printf("		<td colspan=\"1\">%s</td>\n", currentuser.userid);
	//add 2014-11-23 save to sent-mail-box
	printf("        <td><span>保存到发件箱</span><input type=\"checkbox\" name=\"backup\" checked></td>");
	printf("	</tr>\n");

	/* 收信人的一行 */
	printf("	<tr>\n");
	printf("		<td>收信人:</td>\n");
	printf("		<td colspan=\"2\"><input name=\"userid\" type=\"text\" size=\"20\" value=\"%s\">\n", nohtml(userid));
	printf("		<span style=\"margin-left: 60px;\">发送给所有好友<input type=\"checkbox\" name=\"allfriend\" onclick=\"chguserid();\">[ <a href=\"bbsfall\" target=\"_blank\">查看好友名单</a> ]</span></td>\n");
	printf("	</tr>\n");

	/* 选择签名档，附件和引文模式的一行 */
	printf("	<tr>\n");
	printselsignature();
	printuploadattach();

	/* 引文模式 */
if (file[0]) {
	printf("	<span style=\"margin-left: 20px;\">引文模式: %s ", fullquote ? "完全" : "精简");
	printf("		[<a target=\"_self\" href=\"bbspstmail?inframe=1&board=%s&file=%s&num=%d", board, file, num);
	printf("&fullquote=%d\">切换为%s模式</a> (将丢弃所更改内容)]", !fullquote, (!fullquote) ? "完全" : "精简");
	printf("	</span>\n");
} /* end of if (file[0])  */

	printf("</td>\n"); /* 弥补printselsignature()函数不封闭的</td> */
	printf("	</tr>\n");

	/* 信件的正文内容 */
	printf("	<tr>\n");
	printf("		<td colspan=\"3\">\n");
	printf("		<textarea  id=textedit onkeydown=\"if(event.keyCode==87 && event.ctrlKey) {document.form1.submit(); return false;}\"  onkeypress=\"iif(event.keyCode==10) return document.form1.submit()\" name=\"text\" rows=\"14\" cols=\"76\" wrap=\"virtual\" class=\"f2\">\n");
	if (file[0]) {
		int lines = 0;
		if (board[0]) {
			printf("【 在 %s 的大作中提到: 】\n", userid);
			sprintf(path, "boards/%s/%s", board, file);
			i = 3;
		} else {
			printf("【 在 %s 的来信中提到: 】\n", userid);
			sprintf(path, "mail/%c/%s/%s",
				mytoupper(currentuser.userid[0]),
				currentuser.userid, file);
			i = 4;
		}
		fp = fopen(path, "r");
		if (fp) {
			for (; i > 0; i--)
				if (fgets(buf, 500, fp) == 0)
					break;
			while (1) {
				if (fgets(buf, 500, fp) == 0)
					break;
				if (!strncmp(buf, ": 【", 4))
					continue;
				if (!strncmp(buf, ": : ", 4))
					continue;
				if (!strncmp(buf, "--\n", 3))
					break;
				if (!strncmp(buf, "begin 644 ", 10) || !strncmp(buf, "beginbinaryattach ", 18))
					break;
				if (buf[0] == '\n')
					continue;;
				printf(": %s", nohtml(void1(buf)));
				if (!fullquote && lines++ > 10) {
					printf(": (以下引言省略...)\n");
					break;
				}
			}
			fclose(fp);
		}
	}
	printf("</textarea>\n");
	printf("		</td>\n");
	printf("	</tr>\n");
	if(file[0])
		printf("<script language=\"JavaScript\">\n"
				" document.getElementById(\"textedit\").focus(); \n"
				"  </script>");
	else
		printf("<script language=\"JavaScript\">\n"
				" document.getElementById(\"titleedit\").focus(); \n"
				"  </script>");

	/* 按钮的一行 */
	printf("	<tr>\n");
	printf("	<td colspan=\"3\">\n");
	printf("		<input name=\"Submit2\" type=\"submit\" class=\"resetlong\" style=\"border: 1px solid #fff;\" value=\"发表\" onclick=\"this.value='信件发送中，请稍候...'; this.disabled=true;form1.submit();\">\n");
	printf("		<input name=\"Submit3\" type=\"reset\" class=\"sumbitlong\" style=\"border: 1px solid #fff;\" value=\"清除\" onclick=\"return confirm('确定要全部清除吗?');\">\n");
	printf("	</td>\n");
	printf("	</tr>\n");
	printf("</table>\n");
	printf("</form>\n"); /* 需要手动封闭？ */
	printf("</body>\n"); /* 需要手动封闭？ */

	http_quit();
	return 0;
}
