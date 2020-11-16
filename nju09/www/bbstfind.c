#include "bbslib.h"

int
bbstfind_main()
{
	char buf[1024], board[80], dir[80];
	struct boardmem *x1;
	struct fileheader *x;
	int i, total = 0, start = 0, numrecords;
	int thread;
	struct mmapfile mf = { .ptr = NULL };
	html_header(1);
	check_msg();
	changemode(READING);
	ytht_strsncpy(board, getparm("B"), 32);
	if (!board[0])
		ytht_strsncpy(board, getparm("board"), 32);
	thread = atoi(getparm("th"));
	x1 = getboard(board);
	if (x1 == 0)
		http_fatal("错误的讨论区");
	sprintf(buf, "bbsman?board=%s&mode=1", board);
	sprintf(dir, "boards/%s/.DIR", board);
	MMAP_TRY {
		if (mmapfile(dir, &mf) == -1) {
			MMAP_UNTRY;
			http_fatal("错误的讨论区");
		}
		if (mf.size == 0) {
			MMAP_UNTRY;
			mmapfile(NULL, &mf);
			http_fatal("本讨论区目前没有文章");
		}
		numrecords = mf.size / sizeof (struct fileheader);
		brc_initial(currentuser.userid, board);
		if (thread != 0) {
			i = Search_Bin(mf.ptr, thread, 0, numrecords - 1);
			if (i < 0)
				i = -(i + 1);
		} else
			i = 0;
		for (; i < numrecords; i++) {
			x = (struct fileheader *) (mf.ptr + i * sizeof (struct fileheader));
				if (thread != x->thread)
					continue;
			if (!(x->accessed & (FH_MARKED | FH_DIGEST))) {
				char buf2[25];
				if (strlen(buf) < 500)
					strcat(buf, buf2);
			}
			total++;
			if (total == 1)
				start = i + 1;
		}
	}
	MMAP_CATCH {
		total = 0;
	}
	MMAP_END mmapfile(NULL, &mf);
//1

	ytht_strsncpy(board, getparm("B"), 32);
	if (!board[0])
		ytht_strsncpy(board, getparm("board"), 32);
	thread = atoi(getparm("th"));
	x1 = getboard(board);
	if (x1 == 0)
		http_fatal("错误的讨论区");
	sprintf(buf, "bbsman?board=%s&mode=1", board);
	sprintf(dir, "boards/%s/.DIR", board);
	MMAP_TRY {
		if (mmapfile(dir, &mf) == -1) {
			MMAP_UNTRY;
			http_fatal("错误的讨论区");
		}
		if (mf.size == 0) {
			MMAP_UNTRY;
			mmapfile(NULL, &mf);
			http_fatal("本讨论区目前没有文章");
		}
		numrecords = mf.size / sizeof (struct fileheader);
		brc_initial(currentuser.userid, board);
		printf("<body><center>%s -- 同主题查找 [讨论区: %s] ", BBSNAME, board);
//2

	if (total > 0) {
		printf("<a href=bbstcon?board=%s&start=%d&th=%d>本主题全部展开</a> ",board, start - 1, thread);
		if (has_BM_perm(&currentuser, x1))
			printf("<a onclick='return confirm(\"确定同主题全部删除?\")' href=%s>同主题删除</a>", buf);}

//2
		printf("<table border=1><tr><td>编号</td><td>状态</td><td>作者</td>"
				"<td>日期</td><td>标题　　　　　&nbsp;</td><td>星级</td><td>评价</td></tr>\n");
		if (thread != 0) {
			i = Search_Bin(mf.ptr, thread, 0, numrecords - 1);
			if (i < 0)
				i = -(i + 1);
		} else
			i = 0;
		for (; i < numrecords; i++) {
			x = (struct fileheader *) (mf.ptr + i * sizeof (struct fileheader));
				if (thread != x->thread)
					continue;
			printf("<tr><td>%d</td><td>%s</td>", i + 1, flag_str2(x->accessed, !brc_un_read(x)));
			printf("<td>%s</td>", userid_str(fh2owner(x)));
			if (!(x->accessed & (FH_MARKED | FH_DIGEST))) {
				char buf2[25];
				sprintf(buf2, "&box%s=on", fh2fname(x));
				if (strlen(buf) < 500)
					strcat(buf, buf2);
			}
			printf("<td>%6.6s</td>", ytht_ctime(x->filetime) + 4);
			printf("<td><a href=con?B=%s&F=%s&N=%d&st=1&T=%ld>%s </a>\n",
					board, fh2fname(x), i + 1, feditmark(*x), nohtml(x->title));
			if (x->sizebyte)
				printf(" %s", size_str(ytht_byte2num(x->sizebyte)));
			printf("</td><td class=%s>%d</td>\n",
					x->staravg50 ? "red" : "blk", x->staravg50 / 50);
			printf("<td class=%s>%d人</td></tr>\n",
					x->hasvoted ? "red" : "blk", x->hasvoted);
			total++;
			if (total == 1)
				start = i + 1;
		}
	}
	MMAP_CATCH {
		total = 0;
	}
	MMAP_END mmapfile(NULL, &mf);
	printf("</table><hr>\n");
	printf("<a name=\"footer\">共找到 %d 篇 </a>", total/2);
	printf("<a href=%s%s&start=%d>本讨论区</a> ", showByDefMode(), board, (start > 5) ? (start - 5) : 1);
	if (total > 0) {
		printf("<a href=bbstcon?board=%s&start=%d&th=%d>本主题全部展开</a> ",board, start - 1, thread);
		if (has_BM_perm(&currentuser, x1))
			printf("<a onclick='return confirm(\"确定同主题全部删除?\")' href=%s>同主题删除</a>", buf);}
	http_quit();

//1
	return 0;
}

