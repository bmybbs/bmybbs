#include "bbslib.h"

int
bbsmail_main()
{	//modify by mintbaggio 20040829 for new www
	FILE *fp;
	int i, start, total;
	char buf[512], dir[80];
    /**
     * a string like "type=1"
     */
    char type_string[20];
	struct fileheader x;
    /**
     * type of mail box
     * 0 : in box [defualt]
     * 1 : out box
     */
    int box_type = 0; 
	if (!loginok || isguest)
		http_fatal("����δ��¼, ���ȵ�¼");
    strsncpy(buf, getparm("box_type"), 10);
    if(buf[0] != 0) {
        box_type = atoi(buf);
    }
    snprintf(type_string, sizeof(type_string), "box_type=%d", box_type);
    if(box_type == 1) {
        setsentmailfile(dir, currentuser.userid, ".DIR");
    } else {
        setmailfile(dir, currentuser.userid, ".DIR");
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
    const char *name_of_box;
    if(box_type == 1) {
        name_of_box = "�ѷ��ż�";
    } else {
        name_of_box = "�ż��б�";
    }

	printf("<td height=35>%s &gt;  <span id=topmenu_b>%s</span> [ʹ����: <span class=themetext>%s</span>] &#187; <b>��������:</b> <span class=smalltext>%dk</span> &#187; <b>���ÿռ�</b><spanclass=smalltext>%dk</span></td></tr>\n", BBSNAME, name_of_box, currentuser.userid,max_mail_size(),get_mail_size());
	printf("%s", "<tr>\n"
		"<td height=35 valign=top><a href='javascript:SelectAll()' class=\"btnsubmittheme\">ȫ��ѡ��</a>\n"
		"<a onclick='return confirm(\"�����Ҫɾ����Щ�ż���?\")' href='javascript:document.maillist.submit()' class=\"btnsubmittheme\">ɾ��ѡ�е��ʼ�</a>\n");
    if(box_type == 0) {
		printf("<a href='bbspstmail' class=\"btnsubmittheme\" title=\"�����ż� accesskey: m\" accesskey=\"m\">�����ż�</a>\n");
    }
	total = file_size(dir) / sizeof (struct fileheader);
	if (total < 0 || total > 30000)
		http_fatal("too many mails");
	if (!total) {
		printf("Ŀǰ��û���κ��ż�");
		http_quit();
		return 0;
	}
	start = getdocstart(total, w_info->t_lines);

	if (total > w_info->t_lines)
		printf("<a href='mail?S=1&%s' class=btnsubmittheme>��һҳ</a>\n"
			"<a href='mail?S=0&%s' class=btnsubmittheme>���һҳ</a>\n",
                type_string, type_string);
	//printf("start=%d, t_line=%d\n", start, w_info->t_lines);
	if(start > w_info->t_lines+1)
		printf("<a href='bbsmail?&S=%d&%s' class=btnsubmittheme>��һҳ</a>\n", start - w_info->t_lines ,type_string);
	if (start < total - w_info->t_lines + 1)
		printf("<a href='bbsmail?&S=%d&%s' class=btnsubmittheme>��һҳ</a>\n", start + w_info->t_lines, type_string);
	printf("<a href=# onclick='javascript:{location=location;return false;}' class=btnsubmittheme>ˢ��</a>\n");
	
	printf("</td>\n</tr>\n");
#if 0
	total = file_size(dir) / sizeof (struct fileheader);
	if (total < 0 || total > 30000)
		http_fatal("too many mails");
	if (!total) {
		printf("Ŀǰ��û���κ��ż�");
		http_quit();
		return 0;
	}
	start = getdocstart(total, w_info->t_lines);
#endif
	fp = fopen(dir, "r");
	if (fp == 0)
		http_fatal("�޷���Ŀ¼�ļ�, ��֪ͨϵͳά��");
	//printhr();
	printf("%s", "<tr><td width=40 class=\"level1\">&nbsp;</td>\n"
		"<td class=\"level1\"><br>\n"
		);

	printf("<table width=95%% cellpadding=2 cellspacing=0><form action=bbsdelmail?%s name=maillist>\n", type_string);
    printf("<input type=\"hidden\" name=\"box_type\" value=\"%d\"></input>\n", box_type);
	printf
	    ("%s","<tbody><tr>\n"
		"<td class=tdtitle>ѡ��</td>\n"
		"<td class=tdtitle>���</td>\n"
		"<td class=tdtitle>״̬</td>\n"
		"<td class=tdtitle>������</td>\n"
		"<td class=tdtitle>����</td>\n"
		"<td class=tdtitle>�ż�����</td></tr>\n");
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
		printf("<td class=tdborder><a href=bbsmailcon?file=%s&num=%d&%s>", fh2fname(&x),
		       i + start - 1, type_string);
		if (strncmp("Re: ", x.title, 4))
			printf("�� ");
		x.title[40] = 0;
		hprintf("%s", void1(x.title));
		printf("</a></td></tr>\n");
	}
	fclose(fp);
	printf("</tbody></form></table>\n");
	//printhr();
#if 0
	printf("[�ż�����: %d] ", total);
	printf("[<a href='javascript:SelectAll()'>ȫ��ѡ��</a>] ");
	printf
	    ("[<a onclick='return confirm(\"�����Ҫɾ����Щ�ż���?\")'"
	     " href='javascript:document.maillist.submit()'>ɾ��ѡ�е��ʼ�</a>] ");
	printf("[<a href=bbspstmail>�����ż�</a>] ");
	if (total > w_info->t_lines)
		printf("<a href=mail?S=1%s>��һҳ</a> <a href=mail?S=0%s>���һҳ</a> ", type_string, type_string);
	bbsdoc_helper("bbsmail?", start, total, w_info->t_lines);
	printf
	    ("<form><input type=submit value=��ת��> �� <input style='height:20px' type=text name=start size=3> ��</form></body>");
#endif
	printf("<form><input action=bbsmail?%s type=submit class=resetlong value=��ת��> �� <input type=text name=start size=3> ��\n", type_string);
    printf("<input type=\"hidden\" name=\"box_type\" value=\"%d\"></input>\n", box_type);
    printf("</form>\n");

	printf("</td></tr><table></td></tr></table></body>\n");
	http_quit();
	return 0;
}
