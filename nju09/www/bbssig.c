#include "bbslib.h"

static void save_sig(char *path);

int
bbssig_main()
{	//modify by mintbaggio 20040829 for new www
	FILE *fp;
	char path[256], buf[512];
	html_header(1);
	if (!loginok || isguest)
		http_fatal("匆匆过客不能设置签名档，请先登录");
	changemode(EDITUFILE);
	printf("<body><center><div class=rhead>%s -- 设置签名档 [使用者: <span class=h11>%s</span>]</div><hr>\n", BBSNAME, currentuser.userid);
	sprintf(path, "home/%c/%s/signatures", mytoupper(currentuser.userid[0]), currentuser.userid);
	if (!strcasecmp(getparm("type"), "1"))
		save_sig(path);
	printf("<form name=form1 method=post action=bbssig?type=1>\n");
	printf("签名档每6行为一个单位, 可设置多个签名档.<br>"
			"(<a href=bbscon?B=Announce&F=M.1047666649.A>"
			"[临时公告]关于图片签名档的大小限制</a>)<br>");
	printf("<textarea  onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.form1.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.form1.submit()' name=text rows=20 cols=80>\n");
	fp = fopen(path, "r");
	if (fp) {
		while (fgets(buf, sizeof (buf), fp))
			printf("%s", nohtml(void1(buf)));
		fclose(fp);
	}
	printf("</textarea><br>\n");
	printf("<input type=submit value=存盘> ");
	printf("<input type=reset value=复原>\n");
	printf("</form><hr></body>\n");
	http_quit();
	return 0;
}

static void save_sig(char *path) {
	FILE *fp;
	char *buf;
	fp = fopen(path, "w");
	buf = getparm("text");
	fprintf(fp, "%s", buf);
	fclose(fp);
	printf("签名档修改成功。");
	http_quit();
}
