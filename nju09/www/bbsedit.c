#include "bbslib.h"
#include "bmy/article.h"
#include "bmy/board.h"

int update_form(char *board, char *file, char *title);

int
bbsedit_main()
{
	FILE *fp;
	int type = 0, num;
	char buf[512], path[512], file[30], board[32], title[80];
	int base64, isa = 0;
	size_t len;
	char *fn = NULL;
	struct boardmem *brd;
	struct fileheader *x = NULL;
	char bmbuf[IDLEN * 4 + 4];
	struct mmapfile mf = { .ptr = NULL };
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("匆匆过客不能修改文章，请先登录");
	changemode(EDIT);
	ytht_strsncpy(title, getparm("title"), 60);
	ytht_strsncpy(board, getparm("B"), sizeof(board));
	if (!board[0])
		ytht_strsncpy(board, getparm("board"), sizeof(board));
	type = atoi(getparm("type"));
	brd = getboard(board);
	if (brd == 0)
		http_fatal("错误的讨论区");
	ytht_strsncpy(file, getparm("F"), sizeof(file));
	if (!file[0])
		ytht_strsncpy(file, getparm("file"), sizeof(file));
	if (!has_post_perm(&currentuser, brd))
		http_fatal("错误的讨论区或者您无权在此讨论区发表文章");
	if (noadm4political(board))
		http_fatal("对不起,因为没有版面管理人员在线,本版暂时封闭.");
	sprintf(path, "boards/%s/.DIR", board);
	MMAP_TRY {
		if (mmapfile(path, &mf) == -1) {
			MMAP_UNTRY;
			http_fatal("错误的讨论区");
		}
		num = -1;
		x = findbarticle(&mf, file, &num, 1);
	}
	MMAP_CATCH {
		x = NULL;
	}
	MMAP_END mmapfile(NULL, &mf);
	if (strstr(file, "..") || strstr(file, "/"))
		http_fatal("错误的参数");
	if (x == 0)
		http_fatal("错误的参数");
	if (strcmp(x->owner, currentuser.userid) && (!has_BM_perm(&currentuser, brd)))
		http_fatal("你无权修改此文章");
	if (!strcmp(board, "syssecurity"))
		http_fatal("你无权修改系统记录");
	if (brd->header.flag & IS1984_FLAG)
		http_fatal("本讨论区文章禁止修改");

	if (type != 0)
		return update_form(board, file, title);

	printf("<body leftmargin=0 topmargin=0>\n");
	printf("<table width=\"100%%\" border=0 cellpadding=0 cellspacing=0>\n");
	printf("%s", "<tr>\n<td height=30 colspan=2>\n"
		"<table width=\"100%\"  border=0 cellspacing=0 cellpadding=0>\n"
		"<tr><td width=40><img src=\"/images/spacer.gif\" width=40 height=10 alt=\"\"></td>\n"
		"<td><table width=\"100%\" border=0 align=right cellpadding=0 cellspacing=0>\n"
		"<tr><td>\n");
	printf("<a href=\"boa?secstr=%s\">%s</a> / <a href=\"%s%s\">%s版</a> / 修改文章 </td>\n"
		"</tr></table></td>\n<td><table border=0 align=right cellpadding=0 cellspacing=0>\n"
		"<tr><td> 版主 %s \n"
		"</td></tr></table></td></tr></table></td></tr>\n",
		brd->header.sec1,nohtml(getsectree(brd->header.sec1)->title), showByDefMode(), board, board, userid_str(bm2str(bmbuf, &(brd->header))));
//	if (x->header.flag & IS1984_FLAG)
//		printf("<tr><td height=30 colspan=2><font color=red>请注意，本文发表后需通过审查</font></td></tr>");
	printf("%s", "<tr><td height=70 colspan=2>\n"
		"<table width=\"100%\" height=\"100%\" border=0 cellpadding=0 cellspacing=0 bgcolor=\"#efefef\">\n"
		"<tr><td width=40>&nbsp; </td>\n"
		"<td height=70><table width=\"95%\" height=\"100%\"  border=0 cellpadding=0 cellspacing=0>\n"
		"<tr>\n");
	printf("<td valign=bottom>\n"
		"<table width=\"100%%\" border=0 cellpadding=0 cellspacing=0>\n"
		"<tr><td class=F0002><div class=\"menu\">\n"
		"<DIV class=btncurrent>&lt;%s&gt;</DIV>\n"
		"<DIV><A class=btnfunc href=\"%s%s\" title=\"返回讨论区 accesskey: b\" accesskey=\"b\">/ 返回讨论区</A></DIV>\n"
		"<DIV style=\"width:10px\" class=N1001></DIV>\n"
		"</div></td></tr></table></td></tr>\n", void1(titlestr(brd->header.title)), showByDefMode(), board);
	printf("<tr><td width=\"100%%\"><table  border=0 cellspacing=0 cellpadding=0>\n"
		"<tr><td> 发文注意事项: <br>\n"
		"发文时应慎重考虑文章内容是否适合公开场合发表。谢谢您的合作。 </td>\n"
		"</tr></table></td></table></td></tr>\n");

	printf("%s", "<tr><td width=40 class=\"level1\"></td>\n"
		"<td class=\"level1\"><br>\n"
		"<TABLE width=\"95%\" cellpadding=5 cellspacing=0>\n"
		"<TBODY><TR><TD class=tdtitletheme>&nbsp;</TD>\n"
		"</TR>\n");
	printf("<TR><TD class=bordertheme>\n"
		"<form name=form1 method=post action=bbsedit>\n");
	printf("<table width=\"100%%\"  border=0 cellspacing=0 cellpadding=0>\n"
		"<tr>\n<td><table border=0 cellpadding=0 cellspacing=0>\n"
		"<tr><td> 使用标题：</td>\n"

			//ArthurF修改部分开始
			//预计实现功能 www下的标题长度限制 标题将通过js限制在45个英文和22个汉字之内
			//失去焦点的时候进行统计 超过则弹出提示框 要求修改
					"<script>\n"
					"function realLen(v){\n"
					"  l=0;\n"
					"  for (var i=0;i<v.length;i++){\n"
					"    if (Math.abs(v.charCodeAt(i))>255)\n"
					"      l+=2;\n"
					"    else\n"
					"      l++;\n"
					"  }\n"
					"  return l;}\n"
					"</script>\n"
					"\n"
					"<td><input name=title type=text class=inputtitle maxlength=45 size=50 value='%s' onblur=\"if (realLen(value)>45){alert('文章标题长度不能超过22个汉字或45个英文长度,否则会丢失信息,请修改文章标题.')}\"></td>\n"
					, (void1(nohtml(x->title))));
			//修改部分结束

	printf("<td height=20>\n"
		" 讨论区：[%s]</td>\n"
		"</tr></table></td></tr>\n", board);
	printf("%s", "<tr><td><table border=0 cellpadding=0 cellspacing=0><tr>\n");
	printf("<td> 作者：%s &nbsp</td>\n<td>", fh2owner(x));

	printuploadattach();
	printf("</td></tr></table></td></tr>\n");
	printf("<tr><td><a href=home/boards/BBSHelp/html/itex/itexintro.html target=_blank>使用Tex风格的数学公式</a><input type=checkbox name=usemath%s>\n", x->accessed & FH_MATH ? " checked" : "");
	printf("设为不可回复<input type=checkbox name=nore%s></td></tr>\n",
			x->accessed & FH_NOREPLY ? " checked" : "");
	sprintf(path, "boards/%s/%s", board, file);
	fp = fopen(path, "r");
	if (fp == 0)
		http_fatal("文件丢失");
	snprintf(path, sizeof (path), PATHUSERATTACH "/%s", currentuser.userid);
	clearpath(path);
	keepoldheader(fp, SKIPHEADER);
	printf("<tr><td><textarea  onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.form1.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.form1.submit()' name=text rows=20 cols=76 wrap=virtual class=f2>\n");
	while (1) {
		if (fgets(buf, 500, fp) == 0)
			break;
		if (isa && (!strcmp(buf, "\r\n") || !strcmp(buf, "\n")))	//附件之后吞一个空行
			continue;
		base64 = isa = 0;
		if (!strncmp(buf, "begin 644", 10)) {
			isa = 1;
			base64 = 1;
			len = 0;
			fn = buf + 10;
		} else if (checkbinaryattach(buf, fp, &len)) {
			isa = 1;
			base64 = 0;
			fn = buf + 18;
		}
		if (isa) {
			if (!getattach(fp, fn, path, base64, len, 0)) {
				printf("#attach %s\n", fn);
			}
		} else if (!strncmp(buf, "--\n", 3) || !strncmp(buf, "--\r\n", 4)) { // 签名以及变更日志单独存放 IronBlood
			char filelog[256];
			sprintf(filelog, "bbstmpfs/tmp/filelog-%s-%s", board, file);
			FILE *fp_log = fopen(filelog, "w");
			fprintf(fp_log, "\n%s", buf);
			while(1) {
				if(fgets(buf, 500, fp) == 0)
					break;

				fprintf(fp_log, "%s", buf);
			}
			fclose(fp_log);
		} else
			printf("%s", nohtml(void1(buf)));
	}
	fclose(fp);
	printf("</textarea></td></tr>\n");

	printf("<input type=hidden name=type value=1>\n");
	printf("<input type=hidden name=board value=%s>\n", board);
	printf("<input type=hidden name=file value=%s>\n", file);
	printf("%s", "<tr><td><input name=Submit2 type=submit class=resetlong value=\"存盘\" "
			"onclick=\"this.value='文章提交中，请稍候...';this.disabled=true;form1.submit();\">\n"
			"<input name=Submit3 type=reset class=sumbitlong value=\"重置\" onclick='return confirm(\"确定要全部清除吗?\")'></td>\n"
			"</tr>\n");
	printf("%s", "</table></TD></TR></TBODY></TABLE></form></td></tr>\n"
		"<tr>\n<td height=40 bgcolor=\"#FFFFFF\">　</td>\n"
		"<td height=40 bgcolor=\"#FFFFFF\">　</td>\n"
		"</tr></table></td></tr></table>\n");
	http_quit();
	return 0;
}

int
Origin2(text)
char text[256];
{
	char tmp[STRLEN];

	sprintf(tmp, ":．%s %s．[FROM:", BBSNAME, BBSHOST);
	if (strstr(text, tmp))
		return 1;
	sprintf(tmp, ":．%s %s [FROM:", BBSNAME, "http://" MY_BBS_DOMAIN);
	if (strstr(text, tmp))
		return 1;
	else
		return 0;
}

int
update_form(char *board, char *file, char *title)
{
	FILE *fp, *fp_old;
	char *buf = getparm("text"), path[80], path_new[80 + 8];
	int num = 0, filetime;
	int usemath, useattach, nore;
	char dir[STRLEN];
	char filename[STRLEN];
	struct fileheader x;
	struct mmapfile mf = { .ptr = NULL };
	enum ytht_smth_filter_result dangerous = YTHT_SMTH_FILTER_RESULT_SAFE;
	size_t i;
	long l;
	unsigned old_accessed;
	filetime = atoi(file + 2);
	usemath = strlen(getparm("usemath"));
	nore = strlen(getparm("nore"));
	if (usemath)
		usemath = bmy_article_include_math(buf);

	for (i = 0; i < strlen(title); i++)
		if (title[i] <= 27 && title[i] >= -1)
			title[i] = ' ';
	l = strlen(title) - 1;
	while (l >= 0 && isspace(title[l])) {
		title[l] = 0;
		l--;
	}
	if (title[0] == 0)
		http_fatal("标题不能为空");
	if (!ythtbbs_board_is_hidden(board)) {
		dangerous = dofilter_edit(title, buf, ythtbbs_board_is_political(board) ? YTHT_SMTH_FILTER_OPTION_NORMAL : YTHT_SMTH_FILTER_OPTION_SIMPLE);
		if (dangerous == YTHT_SMTH_FILTER_RESULT_1984) {
			post_mail_buf(currentuser.userid, title, buf, currentuser.userid, currentuser.username, fromhost, -1, 0);
			http_fatal(BAD_WORD_NOTICE);
		} else if (dangerous == YTHT_SMTH_FILTER_RESULT_WARN) {
			char mtitle[256];
			sprintf(mtitle, "[修改报警] %s %.60s", board, title);
			post_mail_buf("delete", mtitle, buf, currentuser.userid, currentuser.username, fromhost, -1, 0);
			ythtbbs_cache_Board_updatelastpost("deleterequest");
		}
	}
	sprintf(filename, "bbstmpfs/tmp/%d.tmp", thispid);
	useattach = (insertattachments(filename, buf, currentuser.userid));
	sprintf(path, "boards/%s/%s", board, file);
	sprintf(path_new, "%s.new", path); // 编辑后的文件放置在新的文件中
	fp_old = fopen(path, "r+");
	fp = fopen(path_new, "w+");
	if (fp == 0) {
		http_fatal("无法存盘");
	}
	if (fp_old == 0) {
		fclose(fp);
		http_fatal("无法存盘");
	}
	copyheadertofile(fp_old, fp);
	fclose(fp_old);
	mmapfile(filename, &mf);
	fwrite(mf.ptr, mf.size, 1, fp);
	mmapfile(NULL, &mf);
	unlink(filename);

	sprintf(filename, "bbstmpfs/tmp/filelog-%s-%s", board, file);
	if(access(filename, F_OK) == 0) { // 如果文件操作日志存在的话，则追加写入
		mmapfile(filename, &mf);
		fwrite(mf.ptr, mf.size, 1, fp);
		mmapfile(NULL, &mf);
		unlink(filename);
	}

	fclose(fp);
	add_edit_mark(path_new, currentuser.userid, now_t, fromhost);
	rename(path_new, path); // 替换原来的文件
	sprintf(dir, "boards/%s/.DIR", board);
	fp = fopen(dir, "r");
	if (fp == 0)
		http_fatal("错误的参数");
	while (1) {
		if (fread(&x, sizeof (struct fileheader), 1, fp) <= 0)
			break;
		if (x.filetime == filetime) {
			old_accessed = x.accessed;
			x.edittime = now_t;
			x.sizebyte = ytht_num2byte(eff_size(path));
			x.title[sizeof(x.title) - 1] = 0;
			if (!bmy_board_is_system_board(board) && x.filetime == x.thread && strcmp(x.title, title) != 0) {
				// 对于主题贴，且标题发生变化
				bmy_article_update_thread_title(ythtbbs_cache_Board_get_idx_by_name(board) + 1, x.thread, title);
			}
			ytht_strsncpy(x.title, title, sizeof(x.title));
			if (nore)
				x.accessed |= FH_NOREPLY;
			else
				x.accessed &= ~FH_NOREPLY;
			if (usemath)
				x.accessed |= FH_MATH;
			else
				x.accessed &= ~FH_MATH;
			if (useattach)
				x.accessed |= FH_ATTACHED;
			else
				x.accessed &= ~FH_ATTACHED;
			if (dangerous != YTHT_SMTH_FILTER_RESULT_SAFE)
				x.accessed |= FH_DANGEROUS;
			if (!bmy_board_is_system_board(board) && x.filetime == x.thread && x.accessed != old_accessed) {
				// 对于主题帖且标记发生变化
				bmy_article_update_thread_accessed(ythtbbs_cache_Board_get_idx_by_name(board) + 1, x.thread, x.accessed);
			}
			put_record(&x, sizeof (struct fileheader), num, dir);
			ythtbbs_cache_Board_updatelastpost(board);
			break;
		}
		num++;
	}
	fclose(fp);
	outgo_post(&x, board, currentuser.userid, currentuser.username);
	printf("修改文章成功.<br><a href=%s%s>返回本讨论区</a>", showByDefMode(), board);
	return 0;
}

int
getpathsize(char *path)
{
	DIR *pdir;
	struct dirent *pdent;
	char fname[1024];
	int totalsize = 0, size;
	pdir = opendir(path);
	if (!pdir)
		return -1;
	while ((pdent = readdir(pdir))) {
		if (!strcmp(pdent->d_name, "..") || !strcmp(pdent->d_name, "."))
			continue;
		if (strlen(pdent->d_name) + strlen(path) >= sizeof (fname)) {
			totalsize = -1;
			break;
		}
		sprintf(fname, "%s/%s", path, pdent->d_name);
		size = file_size(fname);
		if (size < 0) {
			totalsize = -1;
			break;
		}
		totalsize += size;
	}
	closedir(pdir);
	return totalsize;
}
