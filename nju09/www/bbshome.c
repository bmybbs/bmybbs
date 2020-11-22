#include "bbslib.h"
#include "check_server.h"

char *
userid_str2(char *s)
{
	static char buf[512];
	char buf2[256], tmp[256], *ptr, *ptr2;
	ytht_strsncpy(tmp, s, 255);
	buf[0] = 0;
	ptr = strtok(tmp, " ,();\r\n\t");
	while (ptr && strlen(buf) < 400) {
		if ((ptr2 = strchr(ptr, '.'))) {
			ptr2[1] = 0;
			strcat(buf, ptr);
		} else {
			ptr = nohtml(ptr);
			sprintf(buf2,
				"<a href=qry?U=%s target=boardpage>%s</a>",
				ptr, ptr);
			strcat(buf, buf2);
		}
		ptr = strtok(0, " ,();\r\n\t");
		if (ptr != NULL)
			strcat(buf, " ");
	}
	return buf;
}

char *
get_mime_type(char *name)
{
	char *dot;

	dot = strrchr(name, '.');
	if (dot == (char *) 0)
		return "text/plain; charset=" CHARSET;
	if (strcasecmp(dot, ".html") == 0 || strcasecmp(dot, ".htm") == 0)
		return "text/html; charset=" CHARSET;
	if (strcasecmp(dot, ".jpg") == 0 || strcasecmp(dot, ".jpeg") == 0)
		return "image/jpeg";
	if (strcasecmp(dot, ".gif") == 0)
		return "image/gif";
	if (strcasecmp(dot, ".png") == 0)
		return "image/png";
	if (strcasecmp(dot, ".pcx") == 0)
		return "image/pcx";
	if (strcasecmp(dot, ".css") == 0)
		return "text/css";
	if (strcasecmp(dot, ".au") == 0)
		return "audio/basic";
	if (strcasecmp(dot, ".wav") == 0)
		return "audio/wav";
	if (strcasecmp(dot, ".avi") == 0)
		return "video/x-msvideo";
	if (strcasecmp(dot, ".mov") == 0 || strcasecmp(dot, ".qt") == 0)
		return "video/quicktime";
	if (strcasecmp(dot, ".mpeg") == 0 || strcasecmp(dot, ".mpe") == 0)
		return "video/mpeg";
	if (strcasecmp(dot, ".vrml") == 0 || strcasecmp(dot, ".wrl") == 0)
		return "model/vrml";
	if (strcasecmp(dot, ".midi") == 0 || strcasecmp(dot, ".mid") == 0)
		return "audio/midi";
	if (strcasecmp(dot, ".mp3") == 0)
		return "audio/mpeg";
	if (strcasecmp(dot, ".pac") == 0)
		return "application/x-ns-proxy-autoconfig";
	if (strcasecmp(dot, ".txt") == 0)
		return "text/plain; charset=" CHARSET;
	if (strcasecmp(dot, ".xht") == 0 || strcasecmp(dot, ".xhtml") == 0)
		return "application/xhtml+xml";
	if (strcasecmp(dot, ".xml") == 0)
		return "text/xml";
	return "application/octet-stream";
}

int
bbshome_main()
{	//modify by mintbaggio 040522 for new www
	char board[80], *path_info, *filename = NULL, *ptr, genbuf[STRLEN * 2];
	struct boardmem *x1;
	int i;
	char bmbuf[(IDLEN + 1) * 4];
	struct mmapfile mf = { ptr:NULL };
	changemode(READING);
	ytht_strsncpy(board, getparm("B"), 32);
	if (!board[0])
		ytht_strsncpy(board, getparm("board"), 32);
	if (strcmp(board, "")) {
		x1 = getboard(board);
		if (x1 == NULL) {
			html_header(1);
			check_msg();
			nosuchboard(board, "home");
		}
		sprintf(genbuf,
			MY_BBS_HOME "/ftphome/root/boards/%s/html/index.htm",
			board);
		if (*getparm("t") != 'b') {
			if (!file_exist(genbuf)) {
				return bbsdoc_main();
			}
			html_header(1);
			check_msg();
			printf
			    ("<frameset rows=\"%d, *\" frameSpacing=0 frameborder=0 border=0 >\n"
			     "<frame scrolling=no marginwidth=0 marginheight=0 name=bar "
			     "src=\"home?board=%s&t=b\">\n"
			     "<frame src=\"home/boards/%s/html/index.htm\" name=boardpage>\n"
			     "</frameset>", 15 + (wwwstylenum % 2) * 2, board,
			     board);
		} else {
			html_header(1);
			check_msg();
			printf("<body topmargin=0 leftmargin=0>\n");
			printf("%s", "<table width=\"100%\" border=0 cellpadding=0 cellspacing=0>\n"
				"<tr><td height=30 colspan=2>\n"
				"<table width=\"100%\"  border=0 cellspacing=0 cellpadding=0>\n"
				"<tr><td width=40><img src=\"/images/spacer.gif\" width=40 height=10 alt=\"\"></td>\n"
				"<td><table width=\"100%\" border=0 align=right cellpadding=0 cellspacing=0>\n");
			printf("<tr><td><a href=boa?secstr=%s target=f3>%s</a> / ",
			       x1->header.sec1,
			       nohtml(getsectree(x1->header.sec1)->title));
			printf("<a href=home?board=%s target=f3>%s版</a></td></tr></table></td>\n",
			       board, board);
			printf("<td><table border=0 align=right cellpadding=0 cellspacing=0>\n");
			printf("<tr><td>版主[%s]</td></tr>\n",
			       userid_str2(bm2str(bmbuf, &(x1->header))));
			printf("</table></td></tr></table></td></tr>\n");
/*			printf
			    ("<a href=bbsbrdadd?board=%s target=f3>预定本版</a> \n",
			     board);
			printf("<a href=doc?board=%s target=f3>讨论区</a> ",
			       board);
			printf("<a href=bbsgdoc?board=%s target=f3>文摘区</a> ",
			       board);
			printf("<a href=bbs0an?path=%s target=f3>精华区</a> ",
			       anno_path_of(board));
			sprintf(genbuf, "boards/.backnumbers/%s/.DIR", board);
			if (!politics(board) && file_exist(genbuf))
				printf
				    ("<a href=bbsbknsel?board=%s target=f3>过刊区</a> ",
				     board);*/
			printf("%s", "<tr><td height=70 colspan=2>\n"
				"<table width=\"100%\" height=\"100%\" border=0 cellpadding=0 cellspacing=0>\n"
				"<tr><td width=40>&nbsp;</td><td height=70>\n"
				"<table width=\"95%\" height=\"100%\"  border=0 cellpadding=0 cellspacing=0>\n"
				"<tr><td colspan=2 valign=bottom>\n"
				"<table width=\"100%\" border=0 cellpadding=0 cellspacing=0>\n");

		}
		http_quit();
	}
	path_info = g_is_nginx ? g_url : getsenv("SCRIPT_URL");
	path_info = strchr(path_info + 1, '/');
	if (NULL == path_info)
		http_fatal("错误的文件名");
	path_info = utf8_decode(path_info);
	if (!strncmp(path_info, "/bbshome/", 9))
		path_info += 9;
	else if (!strncmp(path_info, "/home/", 6))
		path_info += 6;
	else
		http_fatal("错误的文件名");
	if (!strncmp(path_info, "boards/", 7)) {
		if ((ptr = strchr(path_info + 7, '/')) != NULL) {
			*ptr = 0;
			ptr++;
		} else
			ptr = "";
		ytht_strsncpy(board, path_info + 7, 32);
		if (getboard(board) == NULL)
			http_fatal("错误的文件名");
		i = strlen(ptr);
		filename = malloc(i + 100);
		if (filename == NULL)
			http_fatal("错误的文件名");
		sprintf(filename, MY_BBS_HOME "/ftphome/root/boards/%s/%s",
			board, ptr);
	} else if (!strncmp(path_info, "pub/", 4)) {
		i = strlen(path_info);
		filename = malloc(i + 100);
		if (filename == NULL)
			http_fatal("错误的文件名");
		sprintf(filename, MY_BBS_HOME "/ftphome/root/%s", path_info);
	} else
		http_fatal("错误的文件名");

	if (strstr(filename, "..")) {
		free(filename);
		http_fatal("错误的文件名");
	}

	if (file_isdir(filename)) {
		if (filename[strlen(filename) - 1] == '/')
			strcat(filename, "index.htm");
		else {
			free(filename);
			http_fatal("错误的文件名");
			//strcat(filename,"/index.htm");
		}
	}
//      no_outcache();
	if (cache_header(file_time(filename), 600)) {
		free(filename);
		return 0;
	}
	MMAP_TRY {
		if (mmapfile(filename, &mf)) {
			MMAP_UNTRY;
			free(filename);
			http_fatal("错误的文件名");
		}
		printf("Content-type: %s\n\n", get_mime_type(filename));
//      printf("Content-Length: %d\n\n",mf.size);
		fwrite(mf.ptr, 1, mf.size, stdout);
	}
	MMAP_CATCH {
	}
	MMAP_END {
		free(filename);
		mmapfile(NULL, &mf);
	}
	return 0;
}
