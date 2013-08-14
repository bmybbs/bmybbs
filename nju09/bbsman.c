#include "bbslib.h"

int
bbsman_main()
{
	int i, total = 0, mode;
	char board[80], tbuf[256], *cbuf;
	char dir[80];
	struct boardmem *brd;
	char *data= NULL;
	int size, last;
	int fd;
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("请先登录");
	changemode(READING);
	strsncpy(board, getparm("board"), 60);
	mode = atoi(getparm("mode"));
	brd = getboard(board);
	if (brd == 0)
		http_fatal("错误的讨论区");
	if (!has_BM_perm(&currentuser, brd))
		http_fatal("你无权访问本页");
	if (mode <= 0 || mode > 5)
		http_fatal("错误的参数");
	printf("<table>");
	cbuf = "none_op";

	sprintf(dir, "boards/%s/.DIR", board);
	size = file_size(dir);
	if (!size)
		http_fatal("空讨论区");
	fd = open(dir, O_RDWR);
	if (fd < 0)
		http_fatal("空讨论区");
	MMAP_TRY {
		data =
		    mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		close(fd);
		if (data == (void *) -1) {
			MMAP_UNTRY;
			http_fatal("无法读取文章列表");
		}
		last = 0;
		for (i = 0; i < parm_num && i < 40; i++) {
			if (!strncmp(parm_name[i], "box", 3)) {
				total++;
				if (mode == 1) {
#ifndef WWW_BM_DO_DEL
					do_set(data, size, parm_name[i] + 3,
					       FH_DEL, board);
#else
					do_del(board,parm_name[i]+3);
#endif
					cbuf = "delete";
				} else if (mode == 2) {
					do_set(data, size, parm_name[i] + 3,
					       FH_MARKED, board);
					cbuf = "mark";
				} else if (mode == 3) {
					do_set(data, size, parm_name[i] + 3,
					       FH_DIGEST, board);
					cbuf = "digest";
				} else if (mode == 5) {
					do_set(data, size, parm_name[i] + 3, 0, board);
					cbuf = "clear_flag";
				}
			}
		}
		printf("</table>");
	}
	MMAP_CATCH {
		close(fd);
	}
	MMAP_END {
		munmap(data, size);
	}
	if (total <= 0)
		printf("请先选定文章<br>\n");
	snprintf(tbuf, sizeof (tbuf), "WWW batch %s on board %s,total %d",
		 cbuf, board, total);
	securityreport(tbuf, tbuf);
	printf("<br><a href=bbsmdoc?board=%s>返回管理模式</a>", board);
	http_quit();
	return 0;
}

int
do_del(char *board, char *file)
{
	FILE *fp;
	int num = 0, filetime;
	char path[256], buf[256], dir[256], *id = currentuser.userid;
	struct fileheader f;
	filetime = atoi(file + 2);
	sprintf(dir, "boards/%s/.DIR", board);
	sprintf(path, "boards/%s/%s", board, file);
	fp = fopen(dir, "r");
	if (fp == 0)
		http_fatal("错误的参数");
	while (1) {
		if (fread(&f, sizeof (struct fileheader), 1, fp) <= 0)
			break;
		if (f.filetime == filetime) {
			del_record(dir, sizeof (struct fileheader), num);
			cancelpost(board, id, &f, !strcmp(id, f.owner));
			updatelastpost(board);
			fclose(fp);
			printf("<tr><td>%s  <td>标题:%s <td>删除成功.\n",
			       fh2owner(&f), nohtml(f.title));
			snprintf(buf, 256, "%s del %s %s %s",
				 currentuser.userid, board, fh2owner(&f),
				 f.title);
			newtrace(buf);
			return 0;
		}
		num++;
	}
	fclose(fp);
	printf("<tr><td><td>%s<td>文件不存在.\n", file);
	return -1;
}

int
do_set(char *dirptr, int size, char *file, int flag, char *board)
{
	char path[256], buf[256];
	struct fileheader *f;
	int om, og, nm, ng, filetime;
	int start;
	int total = size / sizeof(struct fileheader);
	filetime = atoi(file + 2);
	sprintf(path, "boards/%s/%s", board, file);

	start = Search_Bin(dirptr, filetime,0 , total - 1);
	if (start >= 0) {
		f = (struct fileheader*)(dirptr + start * sizeof(struct fileheader));
		om = f->accessed & FH_MARKED;
		og = f->accessed & FH_DIGEST;
		f->accessed |= flag;
		if (flag == 0) 
			f->accessed = 0;
		nm = f->accessed & FH_MARKED;
		ng = f->accessed & FH_DIGEST;
		printf("<tr><td>%s<td>标题:%s<td>标记成功.\n",
		       fh2owner(f), nohtml(f->title));
		buf[0] = 0;
		if ((!om) && (nm))
			snprintf(buf, 256, "%s mark %s %s %s",
				 currentuser.userid, board,
				 fh2owner(f), f->title);
		if ((om) && (!nm))
			snprintf(buf, 256, "%s unmark %s %s %s",
				 currentuser.userid, board,
				 fh2owner(f), f->title);
		if ((!og) && (ng))
			snprintf(buf, 256, "%s digest %s %s %s",
				 currentuser.userid, board,
				 fh2owner(f), f->title);
		if ((og) && (!ng))
			snprintf(buf, 256, "%s undigest %s %s %s",
				 currentuser.userid, board,
				 fh2owner(f), f->title);
		if (buf[0] != 0)
			newtrace(buf);
		return 0;
	}
	printf("<td><td><td>%s<td>文件不存在.\n", file);
	return -1;
}
