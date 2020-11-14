#include "bbslib.h"

int search(char *id, char *pat, char *pat2, char *pat3, int dt);
char day[20], user[20], title[80];

int
bbsfind_main()
{
	char user[32], title3[80], title[80], title2[80];
	int day;
	html_header(1);
	check_msg();
	changemode(READING);
	ytht_strsncpy(user, getparm("user"), 13);
	ytht_strsncpy(title, getparm("title"), 50);
	ytht_strsncpy(title2, getparm("title2"), 50);
	ytht_strsncpy(title3, getparm("title3"), 50);
	day = atoi(getparm("day"));
	printf("<body>");
	if (day == 0) {
		printf("%s -- 站内文章查询<hr>\n", BBSNAME);
		printf("目前系统负载 %f。系统负载超过 5.0 或者上线人数超过 4000 时将不能进行查询。<br>"
				"系统负载统计图和上线人数统计图可以到<a href=%sbbslists>bbslists版</a>查看<br>",
				*system_load(),showByDefMode());
		if (!loginok || isguest)
			printf("<b>您还没有登录，请先登录再使用本功能</b><br>");
		printf("<form action=bbsfind>\n");
		printf("文章作者: <input maxlength=12 size=12 type=text name=user> (不填查找所有作者)<br>\n");
		printf("标题含有: <input maxlength=60 size=20 type=text name=title>");
		printf(" AND <input maxlength=60 size=20 type=text name=title2><br>\n");
		printf("标题不含: <input maxlength=60 size=20 type=text name=title3><br>\n");
		printf("查找最近: <input maxlength=5 size=5 type=text name=day value=7> 天以内的文章<br><br>\n");
		printf("<input type=submit value=提交查询></form>\n");
	} else {
		if (*system_load() >= 5.0 || count_online() > 4000)
			http_fatal("系统负载(%f)或上线人数(%d)过高, 请在上站人数较少的时间查询.", *system_load(), count_online());
		if (!loginok || isguest)
			http_fatal("请先登录再使用本功能。");
		search(user, title, title2, title3, day * 86400);
	}
	printf("</body>");
	http_quit();
	return 0;
}

int search(char *id, char *pat, char *pat2, char *pat3, int dt) {
	char board[256], dir[256];
	int total, i, sum = 0, nr, j;
	time_t starttime;
	int start;
	struct mmapfile mf = { .ptr = NULL };
	struct fileheader *x;
	printf("%s -- 站内文章查询结果 <br>\n", BBSNAME);
	printf("作者: %s ", id);
	printf("标题含有: '%s' ", nohtml(pat));
	if (pat2[0])
		printf("和 '%s' ", nohtml(pat2));
	if (pat3[0])
		printf("不含 '%s'", nohtml(pat3));
	printf("时间: %d 天<br><hr>\n", dt / 86400);
	starttime = now_t - dt;
	if (starttime < 0)
		starttime = 0;
	if (!search_filter(pat, pat2, pat3)) {
		for (i = 0; i < MAXBOARD && i < shm_bcache->number; i++) {
			strcpy(board, shm_bcache->bcache[i].header.filename);
			if (!has_read_perm_x(&currentuser, &(shm_bcache->bcache[i])))
				continue;
			sprintf(dir, "boards/%s/.DIR", board);
			mmapfile(NULL, &mf);
			if (mmapfile(dir, &mf) < 0) {
				continue;
			}
			x = (struct fileheader *) mf.ptr;
			nr = mf.size / sizeof (struct fileheader);
			if (nr == 0)
				continue;
			start = Search_Bin(mf.ptr, starttime, 0, nr - 1);
			if (start < 0)
				start = - (start + 1);
			for (total = 0, j = start; j < nr; j++) {
				if (abs(now_t - x[j].filetime) > dt)
					continue;
				if (id[0] != 0 && strcasecmp(x[j].owner, id))
					continue;
				if (pat[0] && !strcasestr(x[j].title, pat))
					continue;
				if (pat2[0] && !strcasestr(x[j].title, pat2))
					continue;
				if (pat3[0] && strcasestr(x[j].title, pat3))
					continue;
				if (total == 0)
					printf("<table border=1>\n");
				printf("<tr><td>%d<td><a href=bbsqry?userid=%s>%s</a>",
						j + 1, x[j].owner, x[j].owner);
				printf("<td>%6.6s", ytht_ctime(x[j].filetime) + 4);
				printf("<td><a href=con?B=%s&F=%s&N=%d&T=%lu>%s</a>\n",
						board, fh2fname(&x[j]), j + 1,feditmark(x[j]),
						nohtml(x[j].title));
				total++;
				sum++;
				if (sum > 999) {
					printf("</table> ....");
					break;
				}

			}
			if (sum > 999)
				break;
			if (!total)
				continue;
			printf("</table>\n");
			printf("<br>以上%d篇来自 <a href=%s%s>%s</a><br><br>\n",
					total, showByDefMode(), board, board);
		}
		mmapfile(NULL, &mf);
	}
	printf("一共找到%d篇文章符合查找条件<br>\n", sum);
	sprintf(dir, "%s bbsfind %d", currentuser.userid, sum);
	newtrace(dir);
	return sum;
}

