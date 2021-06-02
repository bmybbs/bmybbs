#include "ythtbbs/commend.h"
#include "bbslib.h"
#include "tmpl.h"
char *size_str(int size);

int top_file(const char *call_type);

void
printdocform(char *cginame, char *board)
{
	printf("<script>docform('%s','%s');</script>", cginame, board);
}

//吧lepton的这个代码写成一个函数
static int nosuchboard_callback(struct boardmem *board, int curr_idx, va_list ap) {
	(void) curr_idx;
	const char *bname = va_arg(ap, const char *);
	const char *cginame = va_arg(ap, const char *);
	int *j = va_arg(ap, int *);
	char *buf = va_arg(ap, char *);
	size_t buflen = va_arg(ap, size_t);

	if (!strcasestr(board->header.filename, bname) && !strcasestr(board->header.title, bname))
		return 0;

	if (!has_read_perm_x(&currentuser, board))
		return 0;

	printf("<tr><td>");
	printf("<a href=%s?board=%s>%s (%s)</a>",
			cginame, board->header.filename,
			void1(titlestr(board->header.title)),
			board->header.filename);
	printf("</td></tr>");

	*j = *j + 1;
	if (*j == 1) {
		snprintf(buf, buflen, "%s?board=%s", cginame, board->header.filename);
	}
	return 0;
}

void nosuchboard(char *board, char *cginame) {
	int j = 0;
	char buf[128];
	printf("没有这个讨论区啊，可能的选择:<p>");
	printf("<table width=300>");
	ythtbbs_cache_Board_foreach_v(nosuchboard_callback, board, cginame, &j, buf, sizeof(buf));
	printf("</table>");
	if (!j)
		printf("喔？我真的帮你找了，你那个讨论区一定输入错了, 没有叫做 \"%s\" 的讨论区啊", nohtml(board));
	if (j == 1)
		redirect(buf);
	printf("<p><a href=javascript:history.go(-1)>快速返回</a>");
	http_quit();
}

void
printboardhot(struct boardmem *x)
{
	char path[STRLEN];
	struct mmapfile mf = { .ptr = NULL, .size = 0 };
	sprintf(path, "boards/%s/TOPN", x->header.filename);
	MMAP_TRY {
		if (mmapfile(path, &mf) == -1) {
			MMAP_RETURN_VOID;
		}
		if (mf.size) {
			fwrite(mf.ptr, mf.size, 1, stdout);
			printhr();
		}
	}
	MMAP_CATCH {
	}
	MMAP_END mmapfile(NULL, &mf);
}

void
printboardtop(struct boardmem *x)
{	//modify by mintbaggio 040522 for new www
	char *board = x->header.filename, bmbuf[IDLEN * 4 + 4];
	char sbmbuf[IDLEN * 12 + 12];
	printf("%s","<tr><td height=30 colspan=2>\n"
		"<table width=\"100%\"  border=0 cellspacing=0 cellpadding=0>\n"
		"<tr><td width=40><img src=\"/images/spacer.gif\" width=40 height=10 alt=\"\"></td>\n"
		"<td><table width=\"100%\" border=0 align=right cellpadding=0 cellspacing=0>\n");
	printf("<tr><td><a href=boa?secstr=%s target=f3>%s</a> / ",
			x->header.sec1, nohtml(getsectree(x->header.sec1)->title));
	printf("<a href=%s%s target=f3>%s版</a>&nbsp;<a href=\"http://" MY_BBS_DOMAIN "/" SMAGIC "/rss?board=%s\" target=\"blank\"><img  src=\"/images/rss.gif\" border=\"0\" /></a></td></tr></table></td>\n", showByDefMode(), board, board, board);

	printf("<td><table border=0 align=right cellpadding=0 cellspacing=0>\n");
	printf("<tr><td>版主[%s]</td></tr>\n", userid_str(bm2str(bmbuf, &(x->header))));
	if(strlen(sbm2str(sbmbuf, &(x->header)))){
		printf("<tr><td align=right>小版主[%s]</td></tr>", userid_str(sbmbuf));
	}
	printf("</table></td></tr></table></td></tr>\n");

	printf("%s", "<tr><td height=70 colspan=2>\n"
		"<table width=\"100%\" border=0 cellpadding=0 cellspacing=0 class=\"level2\">\n"
		"<tr><td width=40>&nbsp;</td><td height=70>\n"
		"<table width=\"95%\"  border=0 cellpadding=0 cellspacing=0>\n"
		"<tr><td colspan=2 valign=bottom>\n"
		"<table width=\"100%\" border=0 cellpadding=0 cellspacing=0>\n");
	printf("<tr><td><div class=\"menu\">\n");
	printf("<div class=btncurrent>&lt;%s&gt;</div>\n", void1(titlestr(x->header.title)));
	//add by macintosh 070530 for board digest
	printf("<a class=btnlinkgrey href=\"not?B=%s&mode=3\" title=\"版面简介 accesskey: d\" accesskey=\"d\">&lt;版面简介&gt;</a>\n", board);
	printf("<a class=btnlinkgrey href=\"not?B=%s&mode=1\" title=\"备忘录 accesskey: w\" accesskey=\"w\">&lt;备忘录&gt;</a>\n", board);
	printf("<a class=btnlinkgrey href=\"gdoc?B=%s\" title=\"文摘区 accesskey: g\" accesskey=\"g\">&lt;文摘区&gt;</a>\n", board);
	//add by macintosh 050519 for marked file list
	printf("<a class=btnlinkgrey href=\"mmdoc?B=%s\" title=\"被M文章 accesskey: m\" accesskey=\"m\">&lt;被M文章&gt;</a>\n",board);
	printf("<a class=btnlinkgrey href=\"bknsel?B=%s\">&lt;过刊区&gt;</a>\n", board);
	printf("<a class=btnlinkgrey href=\"0an?path=%s\" title=\"精华区 accesskey: x\" accesskey=\"x\">&lt;精华区&gt;</a>\n", anno_path_of(board));
	readmybrd(currentuser.userid);
	if (ythtbbs_mybrd_exists(&g_GoodBrd, board)) {
		printf("<a class=btnfunc href=\"brddel?B=%s\" title=\"移除预定 accesskey: a\" accesskey=\"a\"> 移除预定</a>\n", board);
	} else {
		printf("<a class=btnfunc href=\"brdadd?B=%s\" title=\"预定本版 accesskey: a\" accesskey=\"a\"> 预定本版</a>\n", board);
	}
	printf("<a class=btnfunc href=\"bfind?B=%s\" title=\"版内查询 accesskey: s\" accesskey=\"s\"> 版内查询</a>\n", board);
	// 注释掉进版页面的功能 by IronBlood@bmy 20120510
	//sprintf(genbuf, MY_BBS_HOME "/ftphome/root/boards/%s/html/index.htm", board);
	//if (!access(genbuf, R_OK))
		//printf("<a href=%s%s class=btnfunc title=\"进版页面 accesskey: f\" accesskey=\"f0\">进版页面</a> ",showByDefMode(), board);

	if (x->header.flag & VOTE_FLAG)
		printf("<a class=btnfunc href=vote?B=%s title=\"投票 accesskey: v\" accesskey=\"v\"> 投票</a>", board);
	printf("</div></td></tr></table></td></tr>\n");
}

int
getdocstart(int total, int lines)
{
	char *ptr;
	int start;
	ptr = getparm("S");
	if (!ptr[0])
		ptr = getparm("start");
//      if (sscanf(ptr, "%d", &start) != 1)
//              start = 0;
	start = atoi(ptr);
	if (start == 0 || start > total - lines + 1)
		start = total - lines + 1;
	if (start <= 0)
		start = 1;
	return start;
}

void
bbsdoc_helper(char *cgistr, int start, int total, int lines)
{
	if (start > 1)
		printf("<a href=%s&S=%d>上一页</a> ", cgistr, start - lines);
	if (start < total - lines + 1)
		printf("<a href=%s&S=%d>下一页</a> ", cgistr, start + lines);
	printf("<a href=# onclick='javascript:{location=location;return false;}' class=blu>刷新</a> ");
}


int
printkeywords(char* keywordstr)
{
	char delims[] = " \t,.;:";
	char *result = NULL;

	result = strtok(keywordstr, delims);
	while( result != NULL ) {
		printf("<a href=bbssbs?keyword=%s target=f3>", result);
		hprintf("%s", result);
		printf("</a> ");
		hprintf(" ");
		result = strtok(NULL, delims);
	}
	return 1;
}

// 显示相关版面
void printrelationboards(char *buf)
{
	char delims[] = ",";
	char *result = NULL;

	result = strtok(buf, delims);
	while( result != NULL ) {
		printf("<a href=%s%s target=f3>",showByDefMode(), result);
		hprintf("%s", result);
		printf("</a> ");
		hprintf(" ");
		result = strtok(NULL, delims);
	}

}


int
bbsdoc_main()
{	//modify by mintbaggio 040522 for new www
	FILE *fp;
	char board[32 /* max: 24 */], dir[80], genbuf[STRLEN], buf[STRLEN],only_for_b[80];
	struct boardmem *x1;
	struct fileheader x, x2;
	int i, start, total;
	changemode(READING);
	ytht_strsncpy(board, getparm("B"), sizeof(board));
	if (!board[0])
		ytht_strsncpy(board, getparm("board"), sizeof(board));
	x1 = getboard(board);
	if (x1 == 0) {
		html_header(1);
		nosuchboard(board, "doc");
	}
	updateinboard(x1);
	strcpy(board, x1->header.filename);
	sprintf(dir, "boards/%s/.DIR", board);
//	if(cache_header(file_time(dir),10))
//		return 0;
	int hastmpl;
	sprintf(genbuf, "boards/%s/%s", board, ".tmpl");
	if ((fp=fopen(genbuf, "r")) == 0)
		hastmpl = 0;
	else{
		hastmpl = file_size(genbuf) / sizeof (struct a_template);
		fclose(fp);
	}
	//add by macintosh 060319 for template post
	html_header(1);
	check_msg();
	printf("<script src=/function.js></script>\n");

	fp = fopen(dir, "r");
	total = x1->total;
	start = getdocstart(total, w_info->t_lines);
	brc_initial(currentuser.userid, board);
	printf("<body topmargin=0 leftmargin=0>\n");
	printf("<table width=\"100%%\" border=0 cellpadding=0 cellspacing=0>\n"
		"<td><form name=form1 action=bbsdoc>\n");
	printboardtop(x1);
//	printf("一般模式 文章数[%d] ", total);
	printf("<tr><td><a href=\"pst?B=%s\" class=\"btnsubmittheme\" title=\"我要发表文章 accesskey: p\" accesskey=\"p\">我要发表文章</a>\n", board);
	if (hastmpl > 0)
		printf("<a href=bbstmpl?action=show&board=%s class=\"btnsubmittheme\" title=\"模板发文 accesskey: t\" accesskey=\"t\">模板发文</a>\n", board);
	//add by macintosh 060319 for template post

	printf("文章数[%d] 在线[%d]</td>", total, x1->inboard);
	printf("<td align=right><a href=\"tdoc?B=%s\">主题模式</a>\n", board);
	if (has_BM_perm(&currentuser, x1))
		printf("<a href=mdoc?B=%s>管理模式</a> ", board);
	printf("<a href=\"clear?B=%s&S=%d\">清除未读</a> <a href=# onclick='javascript:{location=location;return false;}'>刷新</a>\n", board, start);
	printf("<a href=\"doc?B=%s&S=%d\" title=\"第一页 accesskey: 1\" accesskey=\"1\">第一页</a>\n", board, 1);
	if(start > w_info->t_lines+1) printf("<a href=\"doc?B=%s&S=%d\" title=\"上一页 accesskey: f\" accesskey=\"f\">上一页</a>\n", board, (start-w_info->t_lines));
	if(start < total-w_info->t_lines+1) printf("<a href=\"doc?B=%s&S=%d\" title=\"下一页 accesskey: n\" accesskey=\"n\">下一页</a>\n", board, (start+w_info->t_lines));
	printf("<a href=\"doc?B=%s&S=%d\" title=\"最后一页 accesskey: l\" accesskey=\"l\">最后一页</a>\n", board, (total-w_info->t_lines+1));
	//add by macintosh 050519 for func "Go"
	printf("<input type=hidden name=B value=%s>", board);
	printf("<input name=Submit1 type=Submit class=sumbitgrey value=Go>\n"
		"<input name=S type=text style=\"font-size:11px;font-family:verdana;\" size=4>\n");
	//add by liuche 20120206 for pagenumber
	if((start-1)%w_info->t_lines==0)
		printf(" Page: %d/%d\n",(start-1)/w_info->t_lines+1,(total-1)/w_info->t_lines+1);
	else
		printf(" Page: %d/%d\n",(start-1)/w_info->t_lines+2,(total-1)/w_info->t_lines+1);
	printf("</td></tr></table></td></tr></form>\n");
	printf("</table></td>");
//	printf("<a href=tdoc?B=%s>主题模式</a> ", board);
/*	if (has_BM_perm(&currentuser, x1))
		printf("<a href=mdoc?B=%s>管理模式</a> ", board);
	printf("<a href=pst?B=%s class=red>我要发表文章！</a> \n", board);
	printf("<a href=bfind?B=%s>版内查询</a> ", board);
	printf("<a href=clear?B=%s&S=%d>清除未读</a> ", board, start);
	sprintf(genbuf, "doc?B=%s", board);
	bbsdoc_helper(genbuf, start, total, w_info->t_lines);
*/
//	show_header(board, x1, start, total, w_info->t_lines);
	if (NULL == fp)
		http_fatal("本讨论区目前没有文章");
	if (total <= 0) {
		fclose(fp);
		http_fatal("本讨论区目前没有文章");
	}
//	printhr();
	printboardhot(x1);
/*	printf("<table>\n");
	printf("<tr><td>序号</td><td>状态</td><td>作者</td><td>" "日期</td><td>标题</td><td>星级</td><td>评价</td></tr>\n");	//<td>点击</td></tr>\n");
*/
	printf("%s", "<tr><td class=\"level1\"><TABLE width=\"95%\" cellpadding=2 cellspacing=0 align=\"center\" >\n"
		"<TBODY>\n");
	printf("%s", "<TR>\n"
		"<TD class=tdtitle>序号</TD>\n"
		"<TD class=tdtitle>状态</TD>\n"
		"<TD class=tduser>作者</TD>\n"
		"<TD align=center class=tdtitle>日期</TD>\n"
		"<TD align=center class=tdtitle>标题</TD>\n"
		"<TD class=tdtitle>星级</TD>\n"
		"<TD class=tdtitle>评价</TD>\n"
		"</TR>\n");

	fseek(fp, (start - 1) * sizeof (struct fileheader), SEEK_SET);
	//top_file(); //add by wjbta
	for (i = 0; i < w_info->t_lines; i++) {
		char filename[80];
		char *ptr;//, *cls = "";
		if (fread(&x, sizeof (x), 1, fp) <= 0)
			break;
		while (x.sizebyte == 0) {
			int fd;
			sprintf(filename, "boards/%s/%s", board, fh2fname(&x));
			x.sizebyte = ytht_num2byte(eff_size(filename));
			fd = open(dir, O_RDWR);
			if (fd < 0)
				break;
			flock(fd, LOCK_EX);
			lseek(fd, (start - 1 + i) * sizeof (struct fileheader), SEEK_SET);
			if (read(fd, &x2, sizeof (x2)) == sizeof (x2) && x.filetime == x2.filetime) {
				x2.sizebyte = x.sizebyte;
				lseek(fd, -1 * sizeof (x2), SEEK_CUR);
				write(fd, &x2, sizeof (x2));
			}
			flock(fd, LOCK_UN);
			close(fd);
			break;
		}

		//判断是否有b标记
		if(x.accessed & FH_ALLREPLY)
			strcpy(only_for_b,"style='color:red;' ");
		else
			strcpy(only_for_b,"");
		if(!strncmp(x.title, "Re: ",4))
			strcpy(only_for_b,"");



		ptr = flag_str2(x.accessed, !brc_un_read(&x));
		/*if ((ptr[0] == 'N') || (ptr[0]=='n'))
			cls = " class=B0500";
		else if(!strncasecmp(ptr, "g", 1) || !strncasecmp(ptr, "b", 1) || !strncasecmp(ptr, "m", 1))
			cls = " class=B050B";*/
		sprintf(filename, "boards/%s/%s", board, fh2fname(&x));
		if(!(i%2))
			printf("<tr class=d0>"); // 奇数行样式,从0开始索引
		else
			printf("<tr>");

		if (has_BM_perm(&currentuser, x1)) {
			printf("<td class='tdborder'>%d</td><td class='tdborder'>", start+i);
			printf("%s%s%s", (x.accessed & FH_ISWATER) ? "<span style='text-decoration:underline;'>" : "",
					ptr[0] == 0 ? "&nbsp;&nbsp;" : ptr,
					(x.accessed & FH_ISWATER) ? "</span>" : "");
			printf("</td><td class='tduser'>%s</td>", userid_str(fh2owner(&x)));
		} else {
			printf("<td class=tdborder>%d</td><td class=tdborder>%s</td><td class=tduser>%s</td>",
					start + i, ptr[0] == 0 ? "&nbsp;" : ptr, userid_str(fh2owner(&x)));
		}
		if (!i)
			printf("<td align=center class=tdborder><NOBR>%12.12s</NOBR></td>", ytht_ctime(x.filetime) + 4);
		else
			printf("<td align=center class=tdborder>%12.12s</td>", ytht_ctime(x.filetime) + 4);
		x.title[48] = 0;
		printf("<td class=tdborder ><a href=\"con?B=%s&F=%s&N=%d&T=%ld\" %s>%s%s</a>%s</td>",
				board, fh2fname(&x), start + i, feditmark(x), only_for_b,strncmp(x.title, "Re: ", 4) ? "○ " : "",
				void1(titlestr(x.title)), (x.owner[0] == '-') ? "" : size_str(ytht_byte2num(x.sizebyte)));
		if (x.staravg50) {
			printf("<td class=tdborder>%d</td>", x.staravg50 / 50);
			printf("<td class=tdborder>%d人</td>\n", x.hasvoted);
		} else {
			printf("<td class=tdborder>0</td><td class=tdborder>0人</td>\n");
		}
		//printf("<td>%d次</td></tr>",x.viewtime);
	}
	top_file("bbsdoc");
	printf("</TR> </TBODY></TABLE></td></tr>\n");
/*	printhr();
	printf("一般模式 文章数[%d] ", total);
	printf("<a href=tdoc?B=%s>主题模式</a> ", board);
	if (has_BM_perm(&currentuser, x1))
		printf("<a href=mdoc?B=%s>管理模式</a> ", board);
	printf("<a href=pst?B=%s class=red>我要发表文章！</a> \n", board);
	printf("<a href=bfind?B=%s>版内查询</a> ", board);
	printf("<a href=clear?B=%s&S=%d>清除未读</a> ", board, start);
	sprintf(genbuf, "doc?B=%s", board);
	bbsdoc_helper(genbuf, start, total, w_info->t_lines);
*/
//	show_header(board, x1, start, total, w_info->t_lines);
	printf("<tr><td height=40 class=\"level1\"><TD class=level1 width=40>&nbsp;</TD>\n"
		"<table width=\"95%%\"  border=0 cellpadding=0 cellspacing=0 class=\"level1\" align=\"center\">\n"
		"<td><form name=form2 action=bbsdoc>\n");
	printf("<tr><td><a href=\"pst?B=%s\" class=btnsubmittheme>我要发表文章</a>\n", board);
	if (hastmpl > 0)
		printf("<a href=bbstmpl?action=show&board=%s class=btnsubmittheme>模板发文</a>\n", board);
	//add by macintosh 060319 for template post
	printf("文章数[%d] 在线[%d]</td>\n", total, x1->inboard);
	printf("<td align=\"right\"><a href=\"tdoc?B=%s\">主题模式</a>\n", board);
	if (has_BM_perm(&currentuser, x1))
		printf("<a href=mdoc?B=%s>管理模式</a> ", board);
	printf("<a href=\"clear?B=%s&S=%d\">清除未读</a> <a href=# onclick='javascript:{location=location;return false;}'>刷新</a>\n", board, start);
	printf("<a href=\"doc?B=%s&S=%d\">第一页</a>\n", board, 1);
	if(start > w_info->t_lines+1) printf("<a href=\"doc?B=%s&S=%d\">上一页</a>\n", board, (start-w_info->t_lines));
	if(start < total-w_info->t_lines+1) printf("<a href=\"doc?B=%s&S=%d\">下一页</a>\n", board, (start+w_info->t_lines));
	printf("<a href=\"doc?B=%s&S=%d\">最后一页</a>\n", board, (total-w_info->t_lines+1));
	//add by macintosh 050519 for func "Go"
	printf("<input type=hidden name=B value=%s>", board);
	printf("<input name=Submit2 type=Submit class=sumbitgrey value=Go>\n"
		"<input name=S type=text style=\"font-size:11px;font-family:verdana;\" size=4></td>\n"
		"</tr></form>");
	printf("</table></td></tr>\n");
	printf("</td>");
	fclose(fp);
//	printdocform("doc", board);
	printf("</table></td></tr>");
	sprintf(buf, "%s", ytht_strtrim(x1->header.keyword));
	if (strlen(buf)){
		printf("<table width=\"100%%\" cellpadding=2 cellspacing=0><tr><td class=tdtitle align=center>\n");
		printf("本版关键字: ");
		printkeywords(buf);
		printf("</td></tr></table>\n");
	}

	sprintf(genbuf, "boards/%s/boardrelation", board);
	fp = fopen(genbuf, "r");
	if (fp != NULL) {
		char linebuf[128];
		fgets(linebuf, 128, fp);
		printf("<table width=\"100%%\" cellpadding=2 cellspacing=0><tr><td class=tdtitle align=center>\n");
		printf("来这个版的朋友也常去这些版面: ");
		printrelationboards(linebuf);
		printf("</td></tr></table>\n");
		fclose(fp);
	}


	printf("</table></body>\n");
	http_quit();
	return 0;
}

char *
size_str(int size)
{
	static char buf[256];
	if (size < 1000) {
		sprintf(buf, "(<font class=tea>%d字</font>)", size);
	} else {
		sprintf(buf, "(<font class=red>%d.%d千字</font>)", size / 1000, (size / 100) % 10);
	}
	return buf;
}
//add by wjbta
int top_file(const char *call_type)
{	//modify by mintbaggio 040522 for new www
	FILE *fp;
	char board[80], buf[128], title[80], *ptr;
	struct boardmem *x1;
	struct fileheader x;
	int i, start, total;
	int flag; // 0 为 bbsdoc, 1 为 bbstdoc，两者标记判断调用方法不一致
	if(!strcmp(call_type,"bbsdoc")) flag = 0;
	if(!strcmp(call_type,"bbstdoc")) flag = 1;

	ytht_strsncpy(board, getparm2("B", "board"), 32);
	x1 = getboard(board);
	if (x1 == NULL)
		return -1;
	updateinboard(x1);
	strcpy(board, x1->header.filename);
	sprintf(buf, "boards/%s/.TOPFILE", board);
	fp = fopen(buf, "r");
	if (fp == 0)
		return -1;
	total = file_size(buf) / sizeof (struct fileheader);
	start = getdocstart(total, w_info->t_lines);
	fseek(fp, (start - 1) * sizeof (struct fileheader), SEEK_SET);
	for (i = 0; i < w_info->t_lines; i++) {
		if (fread(&x, sizeof (x), 1, fp) <= 0)
			break;

		x.title[sizeof(x.title) - 1] = 0;
		x.owner[sizeof(x.owner) - 1] = 0;
		strcpy(title, fh2fname(&x));
		if(title[0]=='T')
			title[0]='M';

		if(!flag)
			ptr=flag_str2(x.accessed, !brc_un_read(&x));
		else
			ptr=flag_str(x.accessed);

		printf("<tr class='doctop'><td class='tdborder doctopword'>提示</td>\n"
				"<td class='tdborder'>%s</td><td class='tduser'>%s</td>",(ptr[0] == ' ' || ptr[0] == 0)? "&nbsp;" : ptr,userid_str(x.owner));
		printf("<td align=center class='tdborder'>%12.12s</td>", ytht_ctime(x.filetime) + 4);
		printf("<td class='tdborder'><a href=con?B=%s&F=%s class=1103>%s%s</a></td>\n",board, title, strncmp(x.title,"Re: ", 4) ? "● ":" ",void1(titlestr(x.title)));;
		if(!flag)
			printf("<td class='tdborder'>&nbsp;</td><td class='tdborder'>&nbsp;</td></tr>");
		else
			printf("<td class='tdborder'>&nbsp;</td></tr>");
	}
	fclose(fp);
	return 0;
}

//add by wjbta
// warning by IronBlood: unused function
void show_rec() {
	FILE *fp;
	struct commend x;
	int no=0;
	int i, total;
	fp=fopen(".commend", "r");
	if(!fp) return;
	printf("%s", "<table width='100%' style='BORDER: 2px solid; BORDER-COLOR:e8e8e8;'>\n");
	total=file_size(".commend")/sizeof(struct commend);
	for(i=total-1; i>=0; i--) {
		fseek(fp, sizeof(struct commend)*i, SEEK_SET);
		if(fread(&x, sizeof(struct commend), 1, fp)<=0) break;
		//if(!x.flag) continue;

		x.title[sizeof(x.title) - 1] = 0;
		x.board[sizeof(x.board) - 1] = 0;
		x.filename[sizeof(x.filename) - 1] = 0;
		no++;
		if(no>=17) break;
		if(no%2==1) printf("<tr style=\'line-height:12px\'>");
		printf("<td>○<a href=con?B=%s&F=%s&top=1>%s </a> [<a href=board?B=%s&top=2>%s</a>]\n",x.board , x.filename, void1(titlestr(x.title)), x.board,x.board);
	}
	printf("</table>\n");
	printf("<table width='100%%'><tr>");
	printf("<td align=right><a style='color:#208020'href=%sLilyDigest>→兵马俑精华←</a> <a style='color:#208020'href=bbsrec2?top=3>→更多推荐文章(共%d篇)←</a></table>\n", showByDefMode(), total);
	fclose(fp);
}

