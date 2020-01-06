#include "bbslib.h"

int
anc_readtitle(FILE * fp, char *title, int size)
{
	char buf[512];
	while (fgets(buf, sizeof (buf), fp)) {
		if (!strncmp(buf, "# Title=", 8)) {
			strsncpy(title, buf + 8, size);
			return 0;
		}
	}
	return -1;
}

int
anc_readitem(FILE * fp, char *path, int sizepath, char *name, int sizename)
{
	char buf[512];
	int hasname = 0;
	while (fgets(buf, sizeof (buf), fp)) {
		if (!strncmp(buf, "Name=", 5)) {
			strsncpy(name, buf + 5, sizename);
			hasname = 1;
		}
		if (!hasname)
			continue;
		if (strncmp(buf, "Path=~", 6))
			continue;
		strsncpy(path, strtrim(buf + 6), sizepath);
		hasname = 2;
		break;
	}
	if (hasname != 2)
		return -1;
	return 0;
}

int
anc_hidetitle(char *title)
{
	if (!(currentuser.userlevel & PERM_SYSOP) &&
	    (strstr(title, "(BM: SYSOPS)") != NULL
	     || strstr(title, "(BM: SECRET)") != NULL
	     || strstr(title, "(BM: BMS)") != NULL))
		return 1;
	if (strstr(title, "<HIDE>") != NULL)
		return 1;
	return 0;
}

int
bbs0an_main()
{	//modify by mintbaggio 040825 for new www
	FILE *fp;
	int index = 0, visit[2];
	char *ptr, papath[PATHLEN], path[PATHLEN], names[PATHLEN],
	    file[80], buf[PATHLEN], title[256] = " ";
	char *board;
	html_header(1);
	changemode(DIGEST);
	check_msg();
	printf("<body leftmargin=0 topmargin=0>\n");
	printf("<table width=100%% border=0 cellpadding=0 cellspacing=0><tr><td height=30 colspan=2></td></tr>\n");
	printf("<tr><td height=70 colspan=2> <table width=100%% height=100%% border=0 cellpadding=0 cellspacing=0 class=level2><tr><td width=40 rowspan=2></td>\n");

	strsncpy(path, getparm("path"), PATHLEN - 1);
	if (strstr(path, ".."))
		http_fatal("此目录不存在");
	snprintf(names, PATHLEN, "0Announce%s/.Names", path);
	strcpy(papath, path);
	ptr = strrchr(papath, '/');
	if (ptr != NULL) {
		if (ptr != papath || !strcmp(papath, "/"))
			*ptr = '\0';
		else
			ptr[1] = 0;
	}
	board = getbfroma(path);
	if(!strcasecmp(board, "PersonalCorpus"))		//add by mintbaggio@BMY for the reason of www PersonalCorpus
		goto L;
	if (board[0] && !has_read_perm(&currentuser, board))
		http_fatal("1,目录不存在");
L:	fp = fopen(names, "r");
	if (fp == 0)
		http_fatal("2,目录不存在");
	if (anc_readtitle(fp, title, sizeof (title))) {
		fclose(fp);
		http_fatal("错误的目录");
	}
	if (anc_hidetitle(title)) {
		fclose(fp);
		http_fatal("3,目录不存在");
	}
	buf[0] = 0;
	if (board[0])
		sprintf(buf, "%s版", board);
	getvisit(visit, path);
	title[38] = 0;
	printf("<td height=35>%s &gt;<span id=topmenu_b>%s精华区</span> [本目录浏览统计: <span class=themetext> %4d</span>次<span class=themetext>%4d</span>%s]</td>\n",
	       BBSNAME, buf, visit[0],
	       (visit[1] > 100000) ? (visit[1] / 3600) : (visit[1] / 60),
	       (visit[1] > 100000) ? "时" : "分");
	printf("</tr>");
	if (papath[0])
		printf("<tr><td height=35 valign=top><a href=bbs0an?path=%s class=btnsubmittheme title=\"返回上层目录\" accesskey: b\" accesskey=\"b\">返回上层目录</a></td></tr>\n",
			papath);
	else
		printf("<tr><td height=35 valign=top><a href=\"#\" class=btnsubmittheme title=\"返回上层目录\" accesskey: b\" accesskey=\"b\">返回上层目录</a></td></tr>\n");
	printf("<tr><td width=40 class=level1>&nbsp;</td>\n");
	printf("<td class=level1><table width=95%% cellpadding=2 cellspacing=0>\n");
	printf("<tbody>\n");

	printf("<tr><td class=tdtitle colspan=5>%s</td></tr>", nohtml(void1(title)));
	printf
    		("<tr><td class=tdtitle>编号</td><td class=tdtitle>类别</td><td class=tdtitle>标题</td><td class=tdtitle>整理人</td><td class=tdtitle>日期</td></tr>");
	while (!anc_readitem(fp, file, sizeof (file), title, sizeof (title))) {
	char *id;
	if (anc_hidetitle(title))
		continue;
	snprintf(buf, sizeof (buf), "%s%s", path, file);
	if(strcasecmp(board, "PersonalCorpus")){	//add by mintbaggio@BMY for the reason of www PersonalCorpus
		ptr = getbfroma(buf);
		if (*ptr && !has_read_perm(&currentuser, ptr))
			continue;
	}
	index++;
	if (strlen(title) <= 39)
		id = "";
	else {
		title[38] = 0;
		id = title + 39;
		if (!strncmp(id, "BM: ", 4))
			id += 4;
		ptr = strchr(id, ')');
		if (ptr)
			ptr[0] = 0;
	}
	printf("<tr><td class=tdborder>%d</td>", index);
	snprintf(buf, sizeof (buf), "0Announce%s%s", path, file);
	/*if(access("/home/bbs/0Announce/", F_OK)<0)		//test by mintbaggio
		printf("errno=%d, ft!!\n", errno);
	else	printf("good!!\n");*/
	if (!file_exist(buf)) {
		printf("<td class=tdborder>[错误]</td><td class=tdborder>%s</td>", void1(titlestr(title)));
	} else/*bjgyt*/ if (file_isdir(buf)) {
		printf
		    ("<td class=tdborder>[目录]</td><td class=tdborder><a href=bbs0an?path=%s%s>%s</a></td>",
		     path, file, void1(titlestr(title)));
	} else {
		printf
		    ("<td class=tdborder>[文件]</td><td class=tdborder><a href=bbsanc?path=%s&item=%s>%s</a></td>",
		     path, file, void1(titlestr(title)));
	}
	if (id[0])
		printf("<td class=tdborder>%s</td>", userid_str(id));
	else
		printf("<td  class=tdborder> </td>");
	printf("<td>%6.6s %s</td></tr>", Ctime(file_time(buf)) + 4,
	       Ctime(file_time(buf)) + 20);

}
fclose(fp);
printf("</tr></tbody>");
printf("</table></td></tr></table></td></tr></table>");
	if (index <= 0) {
		printf("<br>&lt;&lt; 目前没有文章 &gt;&gt;\n");
	}
	if (papath[0])
		printf("<br>[<a href=bbs0an?path=%s>返回上一层目录</a>] ",
		       papath);
	else
		printf
		    ("<br>[<a href='javascript:history.go(-1)'>返回上一页</a>] ");
	if (board[0]) {
		printf("[<a href=%s%s>本讨论区</a>] ", showByDefMode(), board);
/*		printf
		    ("[<a href=\"ftp://" MY_BBS_DOMAIN
		     ":2121/pub/X/%s.tgz\">下载精华区</a>]", board);
*/
		printf
		    ("[<a href=bbsx/%s.tgz>下载精华区</a>]", board);
	}

printf("</body>");
http_quit();
return 0;
}

int
getvisit(int n[2], const char *path)
{
	char fn[PATH_MAX + 1];
	int fd;
	n[0] = 0;
	n[1] = 0;
	if (snprintf(fn, PATH_MAX + 1, "0Announce%s/.logvisit", path) >
	    PATH_MAX)
		return -1;
	fd = open(fn, O_RDONLY | O_CREAT, 0660);
	if (fd < 0)
		return -1;
	if (read(fd, n, sizeof (int) * 2) <= 0) {
		n[0] = 0;
		n[1] = 0;
	}
	close(fd);
	return 0;
}
