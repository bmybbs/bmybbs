#include "bbslib.h"

int
bbsmnote_main()
{
	FILE *fp;
	char *ptr, path[256], buf[10000], board[256], notestr[40], buf2[128];
	struct boardmem *x;
	int mode;
	
	html_header(1);
	check_msg();
	printf("<center>\n");
	if (!loginok || isguest)
		http_fatal("匆匆过客，请先登录");
	changemode(EDIT);
	ytht_strsncpy(board, getparm("board"), 30);
	x = getboard(board);
	mode = atoi(getparm("mode"));
	if (!has_BM_perm(&currentuser, x))
		http_fatal("你无权进行本操作");
	switch (mode) {
	case 1:
		sprintf(path, "vote/%s/notes", board);
		sprintf(notestr, "一般备忘录");
		break;
	case 2:
		sprintf(path, "vote/%s/secnotes", board);
		sprintf(notestr, "秘密备忘录");
		break;
	case 3:
	default:
		setbfile(path, board, "introduction");
		sprintf(notestr, "版面简介");
		break;
	}
	if (!strcasecmp(getparm("type"), "update")){
		if (mode == 2){
			sprintf(buf2, "vote/%s/notespasswd", board);
			unlink(buf2);
		}
		save_note(path);
	}
	printf("%s -- 编辑%s [讨论区: %s]<hr>\n", BBSNAME, notestr, board);
	printf
	    ("<form name=form1 method=post action=bbsmnote?type=update&board=%s&mode=%d>\n",
	     board, mode);
	fp = fopen(path, "r");
	if (fp) {
		fread(buf, 9999, 1, fp);
		ptr = strcasestr(buf, "<textarea>");
		if (ptr)
			ptr[0] = 0;
		fclose(fp);
	}
	printf("<table border=1><tr><td>");
	printf
	    ("<textarea  onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.form1.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.form1.submit()' name=text rows=20 cols=80 wrap=virtual>\n");
	printf("%s", nohtml(void1(buf)));
	printf("</textarea></table>\n");
	printf("<input type=submit value=存盘> ");
	printf("<input type=reset value=复原>\n");
	printf("<hr>\n");
	http_quit();
	return 0;
}

void
save_note(char *path)
{
	FILE *fp;
	char buf[10000];
	fp = fopen(path, "w");
	ytht_strsncpy(buf, getparm("text"), 9999);
	fprintf(fp, "%s", buf);
	fclose(fp);
	printf("备忘录修改成功。<br>\n");
	printf("<a href='javascript:history.go(-2)'>返回</a>");
	http_quit();
}
