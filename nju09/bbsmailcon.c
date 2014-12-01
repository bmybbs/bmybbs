#include "bbslib.h"

int
bbsmailcon_main()
{	// modify by mintbaggio for new www 041227
	FILE *fp = NULL;
	char dir[80], file[80], path[80], *id;
	struct fileheader x;
	int num, total = 0;
	if ((!loginok || isguest) && (!tempuser))
		http_fatal("���ȵ�¼%d", tempuser);

    /**
     * type of mail box
     * 0 : in box [defualt]
     * 1 : out box
     */
    int box_type = 0;
    char buf[10];
    strsncpy(buf, getparm("box_type"), 10);
    if(buf[0] != 0) {
        box_type = atoi(buf);
    }
    char type_string[20];
    snprintf(type_string, sizeof(type_string), "box_type=%d", box_type);

	//if (tempuser) http_fatal("user %d", tempuser);
	changemode(RMAIL);
	strsncpy(file, getparm("file"), 32);
	num = atoi(getparm("num"));
	id = currentuser.userid;
	if (strncmp(file, "M.", 2))
		http_fatal("����Ĳ���1");
	if (strstr(file, "..") || strstr(file, "/"))
		http_fatal("����Ĳ���2");
    if(box_type == 1) {
        setsentmailfile(path, id, file);
    }else {
        setmailfile(path, id, file);
    }
	if (*getparm("attachname") == '/') {
		showbinaryattach(path);
		return 0;
	}
	if (!tempuser) {
        if(box_type == 1) {
            setsentmailfile(dir, id, ".DIR");
        }else {
            setmailfile(dir, id, ".DIR");
        }
		total = file_size(dir) / sizeof (x);
		if (total <= 0)
			http_fatal("����Ĳ���3 %s", dir);
		fp = fopen(dir, "r+");
		if (fp == 0)
			http_fatal("dir error2");
		fseek(fp, sizeof (x) * num, SEEK_SET);
		if (1 != fread(&x, sizeof (x), 1, fp)) {
			fclose(fp);
			http_fatal("dir error3");
		}
	}
	html_header(1);
	check_msg();
	//printf("<body><center>\n");
	printf("<body leftmargin=0 topmargin=0>");
	printf("<table width=100%% border=0 cellpadding=0 cellspacing=0>\n"
		"<tr>\n<td height=30 colspan=2></td>\n"
		"</tr>\n<tr>\n"
		"<td height=70 colspan=2>\n<br>\n");
	printf("<table width=100%% height=100%% border=0 cellpadding=0 cellspacing=0 class=\"level2\">\n"
		"<tr>\n<td width=40 rowspan=2>&nbsp; </td>\n");
	printf("<td height=35>%s &gt; <span id=\"topmenu_b\">�Ķ��ż�</span> ʹ����:[<span class=\"themetext\"> %s </span>]</td></tr>\n", BBSNAME, id);
#if 0
	if (!tempuser) {
		printf("</center>����: %s<br>", void1(titlestr(x.title)));
		printf("������: %s<br>", titlestr(x.owner));
	}
#endif
	if (loginok && !isguest && !(currentuser.userlevel & PERM_LOGINOK) &&
	    !tempuser && !strncmp(x.title, "<ע��ʧ��>", 10)
	    && !has_fill_form())
		printf
		    ("--&gt;<a href=bbsform><font color=RED size=+1>������дע�ᵥ</font></a>&lt;--\n<br>");
#if 0
	printf("<center>");
	sprintf(path, "mail/%c/%s/%s", mytoupper(id[0]), id, file);
	showcon(path);
#endif
	if (!tempuser) {
		printf("<tr><td height=35 valign=top>\n");
		printf("<a onclick='return confirm(\"�����Ҫɾ���������?\")' href=bbsdelmail?file=%s&%s class=btnsubmittheme title=\"ɾ�� accesskey: d\" accesskey=\"d\">ɾ��</a>", file, type_string);
		if (num > 0) {
			fseek(fp, sizeof (x) * (num - 1), SEEK_SET);
			fread(&x, sizeof (x), 1, fp);
			printf("<a href=bbsmailcon?file=%s&num=%d&%s class=\"btnsubmittheme\" title=\"��һƪ accesskey: f\" accesskey=\"f\">��һƪ</a>", fh2fname(&x), num - 1, type_string);
		}
		printf("<a href=bbseditmail?file=%s&%s class=btnsubmittheme title=\"�༭ accesskey: e\" accesskey=\"e\">�༭</a>", file, type_string);
		printf("<a href=bbsmail?%s class=\"btnsubmittheme\" title=\"�����ż��б� accesskey: b\" accesskey=\"b\">�����ż��б�</a>", type_string);
		if (num < total - 1) {
			fseek(fp, sizeof (x) * (num + 1), SEEK_SET);
			fread(&x, sizeof (x), 1, fp);
			printf("<a href=bbsmailcon?file=%s&num=%d&%s class=\"btnsubmittheme\" title=\"��һƪ accesskey: n\" accesskey=\"n\">��һƪ</a>",
			       fh2fname(&x), num + 1, type_string);
		}
		if (num >= 0 && num < total) {
			fseek(fp, sizeof (x) * num, SEEK_SET);
			if (fread(&x, sizeof (x), 1, fp) > 0
			    && !(x.accessed & FH_READ)) {
				x.accessed |= FH_READ;
				fseek(fp, sizeof (x) * num, SEEK_SET);
				fwrite(&x, sizeof (x), 1, fp);
				printf
				    ("<script>top.f4.location.reload();</script>");
			}
		}
        //send box only support 'foward'
        if(box_type == 0) {
		    printf("<a href='bbspstmail?file=%s&num=%d' class=\"btnsubmittheme\" title=\"���� accesskey: m\" accesskey=\"m\">����</a>",
		       fh2fname(&x), num);
		    printf("<a href='bbscccmail?file=%s' class=\"btnsubmittheme\" title=\"ת�� accesskey: c\" accesskey=\"c\">ת��</a>",
		       fh2fname(&x));
        }
		printf("<a href='bbsfwdmail?file=%s&%s' class=\"btnsubmittheme\" title=\"ת�� accesskey: u\" accesskey=\"u\">ת��</a>",
		       fh2fname(&x), type_string);
		fclose(fp);
	}
    if(box_type == 1) {
        setsentmailfile(path, id, file);
    } else {
        setmailfile(path, id, file);
    }
	showcon(path);

	printf("</table></td></tr></table></body>\n");
	http_quit();
	return 0;
}
