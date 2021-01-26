#include "bbslib.h"

// bbscon.c
extern int showbinaryattach(char *filename);

// bbs0an
extern int anc_readtitle(FILE * fp, char *title, int size);
extern int anc_readitem(FILE * fp, char *path, int sizepath, char *name, int sizename);
extern int anc_hidetitle(char *title);

int
bbsanc_main()
{	//modify by mintbaggio 20040829 for new www
	char *board, *ptr, path[PATHLEN], fn[PATHLEN], buf[PATHLEN],
	item[PATHLEN], names[PATHLEN], file[80], lastfile[80], title[256] = " ";
	int index = 0, found = 0;
	FILE *fp;
	changemode(DIGEST);
	ytht_strsncpy(path, getparm("path"), PATHLEN - 1);
	ytht_strsncpy(item, getparm("item"), PATHLEN - 1);
	board = getbfroma(path);
	if(!strcasecmp(board, "PersonalCorpus"))		//add by mintbaggio@BMY for the reason of www PersonalCorpus
		goto L;
	if (board[0] && !has_read_perm(&currentuser, board))
		http_fatal("目录不存在");
L:
	buf[0] = 0;
	if (board[0])
		sprintf(buf, "%s版", board);
	if (strstr(path, ".Search") || strstr(path, ".Names") || strstr(path, ".."))
		http_fatal("错误的文件名");
	snprintf(names, PATHLEN, "0Announce%s/.Names", path);
	fp = fopen(names, "r");
	if (fp == 0)
		http_fatal("目录不存在");
	if (anc_readtitle(fp, title, sizeof (title))) {
		fclose(fp);
		http_fatal("错误的目录");
	}
	if (anc_hidetitle(title)) {
		fclose(fp);
		http_fatal("目录不存在");
	}
	while (!anc_readitem(fp, file, sizeof (file), title, sizeof (title))) {
		if (anc_hidetitle(title))
			continue;
		snprintf(fn, sizeof (fn), "%s%s", path, file);
		if (!board[0]) {
			if(strcasecmp(board, "PersonalCorpus")){	//add by mintbaggio@BMY for the reason of www PersonalCorpus
				ptr = getbfroma(fn);
				if (*ptr && !has_read_perm(&currentuser, ptr))
					continue;
			}
		}
		if (!strcmp(file, item)) {
			found = 1;
			break;
		}
		index++;
		strcpy(lastfile, file);
	}
	if (!found) {
		fclose(fp);
		http_fatal("文件不存在");
	}
	snprintf(fn, PATHLEN, "0Announce%s%s", path, item);
	if (*getparm("attachname") == '/') {
		fclose(fp);
		showbinaryattach(fn);
		return 0;
	}
	html_header(1);
	check_msg();
	printf("<body><center><div class=rhead>%s -- <span class=h11>%s</span>精华区文章阅读\n", BBSNAME, board);
	if (strlen(title) > 0)
		printf("<br><font class=f3><span class=h11>%s</span></font>", void1(title));
	printf("</div><hr>");
	showcon(fn);
	if (index > 0) {
		snprintf(fn, PATHLEN, "0Announce%s%s", path, lastfile);
		if (file_exist(fn)) {
			if (file_isdir(fn)) {
				printf("[<a href=bbs0an?path=%s%s>上一项</a>] ", path, lastfile);
			} else {
				printf("[<a href=bbsanc?path=%s&item=%s>上一项</a>] ", path, lastfile);
			}
		}
	}
	printf("[<a href=bbs0an?path=%s>回到目录</a>] ", path);
	while (1) {
		if (anc_readitem(fp, file, sizeof (file), title, sizeof (title))) break;
		if (anc_hidetitle(title))
			break;
		snprintf(fn, PATHLEN, "0Announce%s%s", path, file);
		if (!board[0]) {
			if(strcasecmp(board, "PersonalCorpus")){	//add by mintbaggio@BMY for the reason of www PersonalCorpus
				ptr = getbfroma(buf);
				if (*ptr && !has_read_perm(&currentuser, ptr))
					break;
			}
		}
		if (file_exist(fn)) {
			if (file_isdir(fn)) {
				printf("[<a href=bbs0an?path%s%s>下一项</a>] ", path, file);
			} else {
				printf("[<a href=bbsanc?path=%s&item=%s>下一项</a>] ", path, file);
			}
		}
		break;
	}
	fclose(fp);
	if (board[0])
		printf("[<a href=%s%s>本讨论区</a>]", showByDefMode(), board);
	printf("</center></body>\n");
	http_quit();
	return 0;
}
