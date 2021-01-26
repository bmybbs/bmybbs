#include "bbslib.h"

extern int showbinaryattach(char *filename);

int
bbsbkncon_main()
{
	char board[80], bkn[80], dir[256], file[256], filename[256], *ptr;
	struct fileheader *x, *dirinfo;
	struct mmapfile mf = { .ptr = NULL };
	int num, total;
	changemode(BACKNUMBER);
	ytht_strsncpy(board, getparm("board"), 32);
	ytht_strsncpy(bkn, getparm("bkn"), 32);
	ytht_strsncpy(file, getparm("file"), 32);
	num = atoi(getparm("num"));
	ptr = bkn;
	while (*ptr) {
		if (*ptr != 'B' && *ptr != '.' && !isdigit(*ptr))
			http_fatal("错误的参数");
		ptr++;
	}
	if (strlen(bkn) < 3)
		http_fatal("错误的参数");
	if (getboard(board) == NULL)
		http_fatal("错误的讨论区");
	if (strncmp(file, "M.", 2) && strncmp(file, "G.", 2))
		http_fatal("错误的参数1");
	if (strstr(file, "..") || strstr(file, "/"))
		http_fatal("错误的参数2");
	snprintf(dir, 256, "boards/.backnumbers/%s/%s/.DIR", board, bkn);
	total = file_size(dir) / sizeof (*x);
	if (total <= 0)
		http_fatal("此卷过刊不存在或者为空");
	sprintf(filename, "boards/.backnumbers/%s/%s/%s", board, bkn, file);
	if (*getparm("attachname") == '/') {
		showbinaryattach(filename);
		return 0;
	}
	html_header(1);
	check_msg();
	printf("<body><center>\n");
	printf("%s -- 文章阅读 [讨论区: %s--过刊]<hr>", BBSNAME, board);
	MMAP_TRY {
		if (mmapfile(dir, &mf) == -1) {
			MMAP_UNTRY;
			http_fatal("此讨论区不存在或者为空");
		}
		if (num < 0 || num >= total) {
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			http_fatal("本文不存在或者已被删除");
		}
		dirinfo = (struct fileheader *) (mf.ptr + num * sizeof (struct fileheader));
		if (dirinfo->owner[0] == '-') {
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			http_fatal("本文已被删除");
		}
		if (dirinfo->accessed & FH_HIDE) {
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			http_fatal("本文已被删除");
		}
		if (dirinfo->filetime != atoi(file + 2)) {
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			http_fatal("编号不太对啊...");
		}
		showcon(filename);
		printf("[<a href=bbsfwd?board=%s&file=%s>转寄</a>]", board, file);
		printf("[<a href=bbsccc?board=%s&file=%s>转贴</a>]", board, file);
		if (num > 0) {
			x = (struct fileheader *) (mf.ptr + (num - 1) * sizeof (struct fileheader));
			printf("[<a href=bbsbkncon?board=%s&bkn=%s&file=%s&num=%d>上一篇</a>]",
					board, bkn, fh2fname(x), num - 1);
		}
		printf("[<a href=bbsbkndoc?board=%s&bkn=%s>本卷过刊</a>]", board, bkn);
		if (num < total - 1) {
			x = (struct fileheader *) (mf.ptr + (num + 1) * sizeof (struct fileheader));
			printf("[<a href=bbsbkncon?board=%s&bkn=%s&file=%s&num=%d>下一篇</a>]",
					board, bkn, fh2fname(x), num + 1);
		}
	}
	MMAP_CATCH {
		mmapfile(NULL, &mf);
		MMAP_RETURN(-1);
	}
	MMAP_END mmapfile(NULL, &mf);
	printf("</center></body>\n");
	http_quit();
	return 0;
}
