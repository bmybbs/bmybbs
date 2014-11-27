#include "bbslib.h"

int
bbspstmail_main()
{	//modify by mintbaggio 040821 for new www
	FILE *fp;
	int i, num, fullquote = 0;
	char mymaildir[80], userid[80], buf[512], path[512], file[512],
	    board[40], title[80] = "", buff[512];
	struct fileheader *dirinfo;
	struct mmapfile mf = { ptr:NULL };
	html_header(1);
	check_msg();
	if (!loginok)
		http_fatal("�Ҵҹ��Ͳ���д�ţ����ȵ�¼");
	if (!((currentuser.userlevel )& (PERM_CHAT|PERM_PAGE|PERM_POST)))
		http_fatal("��û��Ȩ�޷���");
	if (HAS_PERM(PERM_DENYMAIL))
		http_fatal("�����������Ȩ");
	sprintf(mymaildir, "mail/%c/%s/.DIR", mytoupper(currentuser.userid[0]),
		currentuser.userid);
	if (check_maxmail(mymaildir)){
		sprintf(buff,"����ԭ��: ����˽���ż��ܴ�С�ߴ� %d k,���� %d k ʱ ,�����޷�ʹ�ñ�վ�ķ��Ź���.<br>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp �������ż�,���ż��ܴ�С������ %d k ��,�Ա�֤���Ź�������ʹ��",get_mail_size(),max_mail_size()+20,max_mail_size());
		http_fatal(buff);
		}
	changemode(SMAIL);
	strsncpy(board, getparm("B"), 32);
	if (!board[0])
		strsncpy(board, getparm("board"), 32);
	if (board[0] && !getboard(board))
		http_fatal("�����������");
	strsncpy(file, getparm("F"), 20);
	if (!file[0])
		strsncpy(file, getparm("file"), 20);
	if (file[0] != 'M' && file[0])
		http_fatal("������ļ���");
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
					dirinfo =
					    findbarticle(&mf, file, &num, 1);
				}
			}
			MMAP_CATCH {
				dirinfo = NULL;
			}
			MMAP_END mmapfile(NULL, &mf);
		}
		if (dirinfo) {
			strsncpy(userid, dirinfo->owner, sizeof (userid));
			if (strchr(userid, '.')) {
				if (board[0])
					sprintf(buf, "boards/%s/%s", board,
						fh2fname(dirinfo));
				else
					setmailfile(buf, currentuser.userid,
						    fh2fname(dirinfo));
				getdocauthor(buf, userid, sizeof (userid));
			}
			if (strncmp(dirinfo->title, "Re: ", 4))
				snprintf(title, 55, "Re: %s", dirinfo->title);
			else
				strsncpy(title, dirinfo->title, 55);
		} else
			http_fatal("������ļ���");
	} else
		strsncpy(userid, getparm("userid"), 20);
	if (isguest && strcmp(userid, "SYSOP"))
		http_fatal("�Ҵҹ��Ͳ���д�ţ����ȵ�¼");

	printf("<script language=\"javascript\">\n");
	printf("	function chguserid(){\n");
	printf("	if(document.form1.allfriend.checked==true){\n");
	printf("		document.form1.userid.value='���к���';\n");
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
/*	printf("%s -- �����Ÿ� [ʹ����: %s]<hr>\n", BBSNAME,
	       currentuser.userid);
	printf("<table border=1><tr><td>\n");
	printf("<form name=form1 method=post action=bbssndmail?userid=%s>\n",
	       userid);
	printf
	    ("�ż�����: <input type=text name=title size=40 maxlength=100 value='%s'><br>",
	     (noquote_html(title)));
	printf("������: %s<br>\n", currentuser.userid);
	printf
	    ("������: <input type=text name=userid value='%s'> ���͸����к���<input type=checkbox name=allfriend onclick=chguserid();>[<a href=bbsfall target=_blank>�鿴��������</a>]<br>",
	     nohtml(userid));
	printselsignature();
	printuploadattach();
	printf(" ����<input type=checkbox name=backup>\n");
*/
	printf("<body>\n");
	
	/* ʹ����ID */
	printf("<div style=\"width: 100%; height: 24px; text-align: center;\" class=\"level2\">\n");
	printf("<p style=\"line-height: 24px;\">%s -- �����Ÿ� [ʹ����: %s]</p>\n", BBSNAME, currentuser.userid);
	printf("</div>\n");
	
	printf("<form style=\"margin-left: 20px; margin-top: 10px;\" name=\"form1\" method=\"post\" action=\"bbssndmail?userid=%s\">\n", userid);
	printf("<table>\n");
	
	/* һ��pp����ɫ�� */
	printf("	<tr class=\"tdtitletheme\" style=\"height: 20px;\">\n");
	printf("		<td colspan=\"3\">&nbsp;</td>\n");
	printf("	</tr>\n");
	
	/* �����һ�� */
	printf("	<tr>\n");
	printf("		<td>�ż�����:</td>\n");
	printf("		<td colspan=\"2\"><input id=titleedit name=\"title\" type=\"text\" size=\"40\" value=\"%s\"></td>\n", (noquote_html(title)));
	printf("	</tr>\n");
	
	/* �����˵�һ�� */
	printf("	<tr height=\"20px\">\n");
	printf("		<td>������:</td>\n");
	printf("		<td colspan=\"1\">%s</td>\n", currentuser.userid);
    //add 2014-11-23 save to sent-mail-box
    printf("        <td><span>���浽������</span><input type=\"checkbox\" name=\"backup\" checked></td>");
	printf("	</tr>\n");
	
	/* �����˵�һ�� */
	printf("	<tr>\n");
	printf("		<td>������:</td>\n");
	printf("		<td colspan=\"2\"><input name=\"userid\" type=\"text\" size=\"20\" value=\"%s\">\n", nohtml(userid));
	printf("		<span style=\"margin-left: 60px;\">���͸����к���<input type=\"checkbox\" name=\"allfriend\" onclick=\"chguserid();\">[ <a href=\"bbsfall\" target=\"_blank\">�鿴��������</a> ]</span></td>\n");
	printf("	</tr>\n");
	
	/* ѡ��ǩ����������������ģʽ��һ�� */
	printf("	<tr>\n");
	printselsignature();
	printuploadattach();
	
	/* ����ģʽ */
if (file[0]) {
	printf("	<span style=\"margin-left: 20px;\">����ģʽ: %s ", fullquote ? "��ȫ" : "����");
	printf("		[<a target=\"_self\" href=\"bbspstmail?inframe=1&board=%s&file=%s&num=%d", board, file, num);
	printf("&fullquote=%d\">�л�Ϊ%sģʽ</a> (����������������)]", !fullquote, (!fullquote) ? "��ȫ" : "����");
	printf("	</span>\n");
} /* end of if (file[0])  */

	printf("</td>\n"); /* �ֲ�printselsignature()��������յ�</td> */
	printf("	</tr>\n");
	
	/* �ż����������� */
	printf("	<tr>\n");
	printf("		<td colspan=\"3\">\n");
	printf("		<textarea  id=textedit onkeydown=\"if(event.keyCode==87 && event.ctrlKey) {document.form1.submit(); return false;}\"  onkeypress=\"iif(event.keyCode==10) return document.form1.submit()\" name=\"text\" rows=\"14\" cols=\"76\" wrap=\"virtual\" class=\"f2\">\n");
	if (file[0]) {
		int lines = 0;
		if (board[0]) {
			printf("�� �� %s �Ĵ������ᵽ: ��\n", userid);
			sprintf(path, "boards/%s/%s", board, file);
			i = 3;
		} else {
			printf("�� �� %s ���������ᵽ: ��\n", userid);
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
				if (!strncmp(buf, ": ��", 4))
					continue;
				if (!strncmp(buf, ": : ", 4))
					continue;
				if (!strncmp(buf, "--\n", 3))
					break;
				if (!strncmp(buf, "begin 644 ", 10)
				    || !strncmp(buf, "beginbinaryattach ", 18))
					break;
				if (buf[0] == '\n')
					continue;;
				printf(": %s", nohtml(void1(buf)));
				if (!fullquote && lines++ > 10) {
					printf(": (��������ʡ��...)\n");
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
	
	/* ��ť��һ�� */
	printf("	<tr>\n");
	printf("	<td colspan=\"3\">\n");
	printf("		<input name=\"Submit2\" type=\"submit\" class=\"resetlong\" style=\"border: 1px solid #fff;\" value=\"����\" onclick=\"this.value='�ż������У����Ժ�...'; this.disabled=true;form1.submit();\">\n");
	printf("		<input name=\"Submit3\" type=\"reset\" class=\"sumbitlong\" style=\"border: 1px solid #fff;\" value=\"���\" onclick=\"return confirm('ȷ��Ҫȫ�������?');\">\n");
	printf("	</td>\n");
	printf("	</tr>\n");	
	printf("</table>\n");
	printf("</form>\n"); /* ��Ҫ�ֶ���գ� */
	printf("</body>\n"); /* ��Ҫ�ֶ���գ� */
	
	http_quit();
	return 0;
}
