#include "bbslib.h"

static int search(char *id, char *pat, char *pat2, char *pat3, int dt);
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

static int search_callback(struct boardmem *board, int curr_idx, va_list ap) {
	const char *id = va_arg(ap, const char *);
	const char *pat = va_arg(ap, const char *);
	const char *pat2 = va_arg(ap, const char *);
	const char *pat3 = va_arg(ap, const char *);
	int dt = va_arg(ap, int);
	time_t starttime = va_arg(ap, time_t);
	int *sum = va_arg(ap, int *);

	char dir[256];
	int total, nr, j;
	int start;
	struct mmapfile mf = { .ptr = NULL };
	struct fileheader *x;
	if (!has_read_perm_x(&currentuser, board))
		return 0;

	snprintf(dir, sizeof(dir), "boards/%s/.DIR", board->header.filename);
	mmapfile(NULL, &mf);
	if (mmapfile(dir, &mf) < 0)
		return 0;

	x = (struct fileheader *) mf.ptr;

	nr = mf.size / sizeof(struct fileheader);
	if (nr == 0) {
		mmapfile(NULL, &mf);
		return 0;
	}

	start = Search_Bin(mf.ptr, starttime, 0, nr - 1);
	if (start < 0)
		start = - (start + 1);

	for (total = 0, j = start; j < nr; j++) {
		if (labs(now_t - x[j].filetime) > dt)
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
				board->header.filename, fh2fname(&x[j]), j + 1, feditmark(x[j]),
				nohtml(x[j].title));
		total++;
		*sum = *sum + 1;
		if (*sum > 999) {
			break;
		}
	}

	mmapfile(NULL, &mf);
	if (total) {
		printf("</table>\n");
		printf("<br>以上%d篇来自 <a href=%s%s>%s</a><br><br>\n", total, showByDefMode(), board->header.filename, board->header.filename);
	}

	if (*sum > 999) {
		return QUIT;
	}

	return 0;
}

static int search(char *id, char *pat, char *pat2, char *pat3, int dt) {
	char dir[256];
	int sum = 0;
	time_t starttime;
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
		ythtbbs_cache_Board_foreach_v(search_callback, id, pat, pat2, pat3, dt, starttime, &sum);
	}
	printf("一共找到%d篇文章符合查找条件<br>\n", sum);
	sprintf(dir, "%s bbsfind %d", currentuser.userid, sum);
	newtrace(dir);
	return sum;
}

