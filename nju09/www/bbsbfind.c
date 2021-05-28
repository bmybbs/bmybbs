#include "bbslib.h"
#include "bmy/search.h"
#include "bmy/convcode.h"

static int show_form(char *board);

int
bbsbfind_main()
{
	FILE *fp;
	int num = 0, total = 0, type, dt, mg = 0, og = 0, at = 0;
	char dir[80], title[80], title2[80], title3[80], board[32], userid[80];
	struct boardmem *brd;
	struct fileheader x;
	html_header(1);
	changemode(READING);
	check_msg();
	printf("<center>%s -- 版内文章搜索<hr>\n", BBSNAME);
	type = atoi(getparm("type"));
	ytht_strsncpy(board, getparm("B"), sizeof(board));
	if (!board[0])
		ytht_strsncpy(board, getparm("board"), sizeof(board));
	if (type == 0) {
		if ((brd = getboard(board)) != NULL) {
			return show_form(brd->header.filename);
		} else {
			http_fatal("错误的讨论区");
		}
	}
	// 一般搜索
	else if (type == 1)
	{

		ytht_strsncpy(title, getparm("title"), 60);
		ytht_strsncpy(title2, getparm("title2"), 60);
		ytht_strsncpy(title3, getparm("title3"), 60);
		ytht_strsncpy(userid, getparm("userid"), 60);
	if (!strcasecmp(userid, "Anonymous"))
		userid[0] = 0;
	dt = atoi(getparm("dt"));
	if (!strcasecmp(getparm("mg"), "on"))
		mg = 1;
	if (!strcasecmp(getparm("at"), "on"))
		at = 1;
	if (!strcasecmp(getparm("og"), "on"))
		og = 1;
	if (dt < 0)
		dt = 0;
	if (dt > 9999)
		dt = 9999;
	brd = getboard(board);
	if (brd == 0)
		http_fatal("错误的讨论区");
	sprintf(dir, "boards/%s/.DIR", board);
	fp = fopen(dir, "r");
	if (fp == 0)
		http_fatal("讨论区错误或没有目前文章");
	printf("查找讨论区'%s'内, 标题含: '%s' ", board, nohtml(title));

	if (title2[0])
		printf("和 '%s' ", nohtml(title2));

	if (title3[0])
		printf("不含 '%s' ", nohtml(title3));

	printf("作者为: '%s', '%d'天以内的%s%s文章.<br>\n",
			userid[0] ? userid_str(userid) : "所有作者", dt,
			mg ? "精华" : "所有", at ? "有附件" : "");
	printf("<table>\n");
	printf("<tr><td>编号<td>标记<td>作者<td>日期<td>标题\n");
	if (search_filter(title, title2, title3))
		goto E;

	while (1) {
		if (fread(&x, sizeof (x), 1, fp) == 0)
			break;
		num++;

		x.owner[sizeof(x.owner) - 1] = 0;
		x.title[sizeof(x.title) - 1] = 0;

		if (title[0] && !strcasestr(x.title, title))
			continue;
		if (title2[0] && !strcasestr(x.title, title2))
			continue;
		if (userid[0] && strcasecmp(x.owner, userid))
			continue;
		if (title3[0] && strcasestr(x.title, title3))
			continue;
		if (labs(now_t - x.filetime) > dt * 86400)
			continue;
		if (mg && !(x.accessed & FH_MARKED) && !(x.accessed & FH_DIGEST))
			continue;
		if (at && !(x.accessed & FH_ATTACHED))
			continue;
		if (og && !strncmp(x.title, "Re: ", 4))
			continue;
		total++;
		printf("<tr><td>%d", num);
		printf("<td>%s", flag_str(x.accessed));
		printf("<td>%s", userid_str(x.owner));
		printf("<td>%12.12s", 4 + ytht_ctime(x.filetime));
		printf("<td><a href=con?B=%s&F=%s&N=%d&T=%ld>%40.40s </a>\n", board,
				fh2fname(&x), num, feditmark(x), x.title);
		if (total >= 999)
			break;
	}

	}
	//全文搜索
	else if (type == 2)

	{
		char content[200];
		size_t search_size, i;
		ytht_strsncpy(content, getparm("content"), 200);

		brd = getboard(board);
		if (brd == 0)
			http_fatal("错误的讨论区");

		struct fileheader_utf *articles = bmy_search_board_gbk(board, content, &search_size);
		if (articles == NULL)
			http_fatal("检索程序出错，或者无法搜索到相关内容");

		printf("查找讨论区'%s'内, 正文含: '%s' 的所有文章", board, nohtml(content));
		printf("<table>\n");
		printf("<tr><td>作者<td>日期<td>标题\n");
		for (i = 0; i < search_size; i++) {
			// 这里偷懒使用 content，200 字节应该足够容纳 120 字节转换后的编码内容
			u2g(articles[i].title, strlen(articles[i].title), content, sizeof(content));
			printf("<tr><td><a href=qry?U=\"%s\">%s</a></td><td>%s</td><td><a href=\"con?B=%s&F=M.%ld.A\">%40.40s</a></td></tr>", articles[i].owner, articles[i].owner, ytht_ctime(articles[i].filetime), board, articles[i].filetime, content);
			total++;
			if (total >= 999)
				break;
		}
		free(articles);
		goto E;
	}
	//精华区检索
	else if(type==3)
	{
		if (*system_load() >= 5.0 || count_online() > 4000)
			http_fatal("系统负载(%f)或上线人数(%d)过高, 请在上站人数较少的时间查询.", *system_load(), count_online());

		brd = getboard(board);
		if (brd == 0)
			http_fatal("错误的讨论区");

		char essential_path[80]="\0";
		strncpy(title,getparm("title"),60);
		fp=fopen("0Announce/.Search","r");
		char linebuf[512];
		char *tempbuf = NULL;
		int flag=0;
		int brdlen=strlen(board);
		while(fgets(linebuf,512,fp)!=NULL) {
			if(strncmp(linebuf,board,brdlen)==0)
			{
				linebuf[sizeof(linebuf) - 1] = 0;
				tempbuf=strstr(linebuf,": ")+2;
				flag=1;
				break;
			}
		}
		fclose(fp);
		if(flag==0)
			http_fatal("错误的讨论区");

		ytht_strsncpy(essential_path, tempbuf, sizeof(essential_path));
		printf("查找讨论区'%s'的精华区内, 标题含: '%s' 的所有文章", board, nohtml(title));
		printf("<table>\n");
		printf("<tr><td width=80px>编号<td width=350px>标题<td width=200px>存放路径\n");
		char searchcmd[256];
		sprintf(searchcmd,MY_BBS_HOME "/bin/esearch %s \"%s\"",essential_path,title);
		fp = popen(searchcmd,"r");
		if (fp == NULL)
			http_fatal("无法搜索");

		while(fgets(linebuf,512,fp)!=NULL)
		{
			char postindex[64];
			char posttitle[64];
			char postpath[256];
			char postnum[32];
			char *ptemp;
			ptemp=strtok(linebuf,",");
			ytht_strsncpy(postindex, ptemp, sizeof(postindex));
			ptemp=strtok(NULL,",");
			ytht_strsncpy(postpath, ptemp, sizeof(postpath));
			ptemp=strtok(NULL,",");
			ytht_strsncpy(postnum, ptemp, sizeof(postnum));
			ptemp=strtok(NULL,"\n");
			ytht_strsncpy(posttitle, ptemp, sizeof(posttitle));
			printf("<tr><td>%d",total+1);
			printf("<td><a href=bbsanc?path=%s&item=%s>%40.40s </a>",postpath,postnum,posttitle);
			printf("<td>%s\n",postindex);
			total++;
			if(total>999)
				break;
		}
	}
E:
	if (type == 1 && fp != NULL)
		fclose(fp);
	else if (type == 3 && fp != NULL)
		pclose(fp);
	printf("</table>\n");
	printf("<br>共找到 %d 篇文章符合条件", total);
	if (total > 999)
		printf("(匹配结果过多, 省略第1000以后的查询结果)");
	printf("<br>\n");
	printf("[<a href=bbsdoc?board=%s>返回本讨论区</a>] [<a href='javascript:history.go(-1)'>返回上一页</a>]", board);
	http_quit();
	return 0;
}

static int
show_form(char *board)
{
	printf("<table><form action=bbsbfind?type=1 method=post>\n");
	printf("<tr><td><h4>一般查询</h4>\n");
	printf("<tr><td>版面名称: <input type=text maxlength=24 size=24 name=board value='%s'><br>\n", board);
	printf("<tr><td>标题含有: <input type=text maxlength=50 size=20 name=title> AND ");
	printf("<input type=text maxlength=50 size=20 name=title2>\n");
	printf("<tr><td>标题不含: <input type=text maxlength=50 size=20 name=title3>\n");
	printf("<tr><td>作者帐号: <input type=text maxlength=12 size=12 name=userid><br>\n");
	printf("<tr><td>时间范围: <input type=text maxlength=4  size=4  name=dt value=7> 天以内<br>\n");
	printf("<tr><td>被M文章: <input type=checkbox name=mg> 含有附件: <input type=checkbox name=at> 不含跟贴: <input type=checkbox name=og><br><br>\n");

	/* modified by freely@BMY@20070601 */
	printf("<tr><td><input type=submit value=递交查询结果>\n");
	printf("</form></table>");

	printf("<hr />\n");

	printf("<table><form action=bbsbfind?type=2 method=post>\n");
	printf("<tr><td><h4>全文查询</h4>\n");
	printf("<tr><td>版面名称: <input type=text maxlength=24 size=24 name=board value='%s'><br>\n", board);
	printf("<tr><td>正文含有: <input type=text maxlength=100 size=50 name=content>\n");
	printf("<tr><td>(支持谓词: AND, OR, NOT)<br><br>\n");
	printf("<tr><td><input type=submit value=递交查询结果>\n");
	printf("</form></table>");

	printf("<hr />\n");
	printf("<table><form action=bbsbfind?type=3 method=post>\n");
	printf("<tr><td><h4>精华区查询</h4>\n");
	printf("<tr><td>版面名称: <input type=text maxlength=24 size=24 name=board value='%s'><br>\n",		board);
	printf("<tr><td>标题含有: <input type=text maxlength=50 size=50 name=title>\n");
	printf("<tr><td><input type=submit value=递交查询结果>\n");
	printf("</form></table>");
	printf("<hr />\n");

	printf("[<a href='bbsdoc?board=%s'>返回上一页</a>] [<a href=bbsfind>全站文章查询</a>]", board);
	http_quit();
	return 0;
}
