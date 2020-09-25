#include "bbslib.h"

int
bbsnot_main()
{
	FILE *fp;
	char buf[512], board[80], filename[128], notestr[STRLEN];
	struct boardmem *x;
	int mode;
	size_t i;
	
	html_header(1);
	check_msg();
	changemode(READING);
	ytht_strsncpy(board, getparm("B"), 32);
	if (!board[0])
		ytht_strsncpy(board, getparm("board"), 32);
	if (!(x = getboard(board)))
		http_fatal("错误的版面");
	mode = atoi(getparm("mode"));
	switch (mode) {
	case 1:
		sprintf(filename, "vote/%s/notes", board);
		sprintf(notestr, "一般备忘录");
		break;
/*	case '2':
		sprintf(filename, "vote/%s/secnotes", board);
		sprintf(notestr, "秘密备忘录");
		break;
*/
	case 3:
	default:
		setbfile(filename, board, "introduction");
		sprintf(notestr, "版面简介");
		break;
	}
	
	printf("<center>\n");
	printf("%s -- %s [讨论区: %s]<hr>\n", notestr, BBSNAME, board);
	fp = fopen(filename, "r");
	if (fp == 0) {
		printf("<br>本讨论区尚无「%s」。\n", notestr);
		http_quit();
	}
	printf("<table border=1><tr><td class=f2>\n");
	while (1) {
		char *s;
		bzero(buf, 512);
		if (fgets(buf, 512, fp) == 0)
			break;
		while (1) {
			s = strstr(buf, "$userid");
			if (s == 0)
				break;
			for (i = 0; i < 7; i++)
				s[i] = 32;
			for (i = 0; i < strlen(currentuser.userid); i++)
				s[i] = currentuser.userid[i];
		}
		fhhprintf(stdout, "%s", buf);
	}
	fclose(fp);
	printf("</table><hr>\n");
	printf("[<a href=%s%s>本讨论区</a>] ", showByDefMode(), board);
	if (has_BM_perm(&currentuser, x))
		printf("[<a href=bbsmnote?board=%s&mode=%d>编辑%s</a>]", board, mode, notestr);
	printf("</center>\n");
	http_quit();
	return 0;
}
