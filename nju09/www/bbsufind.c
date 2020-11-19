#include "bbslib.h"

void
printkick(char *buf, int size, struct user_info *x)
{
	if (isguest || x->pid != 1) {
		buf[0] = 0;
		return;
	}

	if((currentuser.userlevel & PERM_SYSOP) || !strcmp(currentuser.userid, x->userid))
		snprintf(buf, size, "<a onclick='return confirm(\"你真的要踢出该用户吗?\")' href=kick?k=%s&f=%s>X</a>", x->userid, x->from);
	else
		buf[0] = 0;
}

int
bbsufind_main()
{
	int i, total = 0, total2 = 0;
	int limit;
	int lockfd;
	struct user_info *user;
	const struct user_info *x;
	char search;
	char buf[128];
	html_header(1);
	check_msg();
	changemode(LUSERS);
	printf("<body><center>\n");
	printf("<div class=rhead>%s -- 在线用户查询 [在线总人数:<span class=h11> %d</span>人]</div><hr>\n", BBSNAME, count_online());
	search = toupper(getparm("search")[0]);
	limit = atoi(getparm("limit"));
	if (limit <=0)
		limit = 0;
	if (search != '*' && (search < 'A' || search > 'Z')) {
		http_fatal("错误的参数");
	}
	if (search == '*') {
		if (!(currentuser.userlevel & PERM_SYSOP)) {
			http_fatal("您无权使用该参数");
		}
		printf("所有在线使用者<br>\n");
	} else {
		printf("字母'%c'开头的在线使用者. \n", search);
		if (limit)
			printf("前 %d 位<br>\n", limit);
	}
	user = malloc(sizeof (struct user_info) * MAXACTIVE);
	if (!user)
		http_fatal("Insufficient memory");
	for (i = 0; i < MAXACTIVE; i++) {
		x = ythtbbs_cache_utmp_get_by_idx(i);
		if (x->active == 0)
			continue;
		if (x->invisible && !HAS_PERM(PERM_SEECLOAK, currentuser))
			continue;
		if (mytoupper(x->userid[0]) != search && search !='*')
			continue;
		memcpy(&user[total], x, sizeof (struct user_info));
		total++;
		if (total>limit && limit!=0)
			break;
	}

	printf("<table border=1>\n");
	printf("<tr><td>序号</td><td>友</td><td>使用者代号</td><td>使用者昵称</td><td>来自</td><td>动态</td><td>发呆</td></tr>\n");
	qsort(user, total, sizeof (struct user_info), (void *) cmpuser);
	for (i = 0; i < total; i++) {
		int dt = (now_t - user[i].lasttime) / 60;
		printf("<tr><td>%d</td>", i + 1);
		lockfd = ythtbbs_override_lock(currentuser.userid, YTHTBBS_OVERRIDE_FRIENDS);
		printf("<td>%s</td>", ythtbbs_override_included(currentuser.userid, YTHTBBS_OVERRIDE_FRIENDS, user[i].userid) ? "√" : "  ");
		ythtbbs_override_unlock(lockfd);
		printkick(buf, sizeof (buf), &user[i]);
		printf("<td><a href=bbsqry?userid=%s>%s</a> %s</td>", user[i].userid, user[i].userid, buf);
		printf("<td><a href=bbsqry?userid=%s>%24.24s</a></td>", user[i].userid, nohtml(void1(user[i].username)));
		printf("<td><font class=c%d>%20.20s</font></td>", user[i].pid == 1 ? 35 : 32, user[i].from);
		if(user[i].mode != USERDF4 )
			printf("<td>%s</td>", user[i].invisible ? "隐身中..." : ModeType(user[i].mode));
		else
			printf("<td>%s</td>", user[i].invisible ? "隐身中..." : user[i].user_state_temp);
		if (dt == 0) {
			printf("<td></td></tr>\n");
		} else {
			printf("<td>%d</td></tr>\n", dt);
		}
		total2++;
	}
	free(user);
	printf("</table>\n");
	printf("本项在线: %d人", total2);
	printf("<hr>");
//	if (search != '*')
//		printf("[<a href='bbsufind?search=*'>全部</a>] ");
	for (i = 'A'; i <= 'Z'; i++) {
//		if (i == search) {
//			printf("[%c]", i);
//		} else {
			printf("[<a href=bbsufind?search=%c>%c</a>]", i, i);
//		}
	}
	printf("<br>\n");
	printf("[<a href='javascript:history.go(-1)'>返回</a>]");
	printf("</center></body>\n");
	http_quit();
	return 0;
}
