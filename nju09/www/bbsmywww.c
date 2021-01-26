#include "bbslib.h"

static int save_set(int t_lines, int link_mode, int def_mode, int att_mode, int doc_mode);

int
bbsmywww_main()
{	//modify by mintbaggio 20040829 for new www
	char *ptr, buf[256];
	int t_lines = 20, link_mode = 0, def_mode = 0, att_mode = 0, doc_mode = 0, type;
	html_header(1);
	check_msg();
	printf("<body>");
	if (!loginok || isguest)
		http_fatal("匆匆过客不能定制界面");
	changemode(GMENU);
	if (readuservalue(currentuser.userid, "t_lines", buf, sizeof (buf)) >= 0)
		t_lines = atoi(buf);
	if (readuservalue(currentuser.userid, "link_mode", buf, sizeof (buf)) >= 0)
		link_mode = atoi(buf);
	if (readuservalue(currentuser.userid, "def_mode", buf, sizeof (buf)) >= 0)
		def_mode = atoi(buf);
//      if (readuservalue(currentuser.userid, "att_mode", buf, sizeof(buf)) >= 0) att_mode=atoi(buf);
	att_mode = w_info->att_mode;
	doc_mode = w_info->doc_mode;
	type = atoi(getparm("type"));
	ptr = getparm("t_lines");
	if (ptr[0])
		t_lines = atoi(ptr);
	ptr = getparm("link_mode");
	if (ptr[0])
		link_mode = atoi(ptr);
	ptr = getparm("def_mode");
	if (ptr[0])
		def_mode = atoi(ptr);
	ptr = getparm("att_mode");
	if (ptr[0])
		att_mode = atoi(ptr);
	ptr = getparm("doc_mode");
	if (ptr[0])
		doc_mode = atoi(ptr);
	printf("<center><div class=rhead>%s -- WWW个人定制 [使用者: <span class=h11>%s</span>]</div><hr>", BBSNAME, currentuser.userid);
//      if (type > 0)
//              return save_set(t_lines, link_mode, def_mode, att_mode);
	if (t_lines < 10 || t_lines > 40)
		t_lines = 20;
	if (link_mode < 0 || link_mode > 1)
		link_mode = 0;
	if (att_mode < 0 || att_mode > 1)
		att_mode = 0;
	if (doc_mode < 0 || doc_mode > 1)
		doc_mode = 0;

	if (type > 0)
		return save_set(t_lines, link_mode, def_mode, att_mode,
				doc_mode);
	printf("<table><form action=bbsmywww>\n");
	printf("<tr><td>\n");
	printf("<input type=hidden name=type value=1>");
	printf("一屏显示的文章行数(10-40): <input name=t_lines size=8 value=%d><br>\n", t_lines);
	printf("链接识别 (0识别, 1不识别): <input name=link_mode size=8 value=%d><br>\n", link_mode);
	printf("缺省模式 (0一般, 1主题): &nbsp;&nbsp;<input name=def_mode size=8 value=%d><br>\n", def_mode);
	printf("<font color=red>need refresh after changed.</font><br>\n");
	printf("附件模式 (0普通，1特殊): &nbsp;&nbsp;<input name=att_mode size=8 value=%d><br><br>\n", att_mode);
	printf("<font color=red>附件模式设置为 0 可能速度比较快。图片显示有问题者可以将附件模式设置为 1。</font><br>\n");
	printf("文章模式 (0普通，1特殊): &nbsp;&nbsp;<input name=doc_mode size=8 value=%d><br><br>\n", doc_mode);
	printf("<font color=red>文章模式设置为 0 可能速度比较快。文章显示有问题者可以将文章模式设置为 1。</font><br>\n");
	printf("</td></tr><tr><td align=center><input type=submit value=确定> <input type=reset value=复原>\n");
	printf("</td></tr></form></table></body></html>\n");
	return 0;
}

static int save_set(int t_lines, int link_mode, int def_mode, int att_mode, int doc_mode) {
	char buf[20];
	if (t_lines < 10 || t_lines > 40)
		http_fatal("错误的行数");
	if (link_mode < 0 || link_mode > 1)
		http_fatal("错误的链接识别参数");
	if (def_mode < 0 || def_mode > 1)
		http_fatal("错误的缺省模式");
	if (att_mode < 0 || att_mode > 1)
		http_fatal("错误的附件模式");
	if (doc_mode < 0 || doc_mode > 1)
		http_fatal("错误的文章模式");

	sprintf(buf, "%d", t_lines);
	saveuservalue(currentuser.userid, "t_lines", buf);
	sprintf(buf, "%d", link_mode);
	saveuservalue(currentuser.userid, "link_mode", buf);
	sprintf(buf, "%d", def_mode);
	saveuservalue(currentuser.userid, "def_mode", buf);
	sprintf(buf, "%d", att_mode);
	saveuservalue(currentuser.userid, "att_mode", buf);
	sprintf(buf, "%d", doc_mode);
	saveuservalue(currentuser.userid, "doc_mode", buf);

	w_info->t_lines = t_lines;
	w_info->def_mode = def_mode;
	w_info->link_mode = link_mode;
	w_info->att_mode = att_mode;
	w_info->doc_mode = doc_mode;
	printf("WWW定制参数设定成功.<br>\n");
	printf("[<a href='javascript:history.go(-2)'>返回</a>]");
	return 0;
}
