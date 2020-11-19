#include "bbslib.h"

static void printkickitem(const struct user_info *x) {
	//modify by mintbaggio@BMY
	int dt = (now_t - x->lasttime) / 60;
	if (x->active == 0) {
		return;
	}
	printf("<tr>");
	printf("<td><a href=bbsqry?userid=%s>%s</a></td>", x->userid, x->userid);
	printf("<td><a href=bbsqry?userid=%s>%s</a></td>", x->userid, x->userid);
	printf("<td><font class=c%d>%20.20s</font></td>", x->pid == 1 ? 35 : 32, x->from);
	if(x -> mode != USERDF4 )
	printf("<td>%s</td>", x->invisible ? "隐身中..." : ModeType(x->mode));
	else
	printf("<td>%s</td>", x->invisible ? "隐身中..." : x->user_state_temp);
	if (dt == 0) {
		printf("<td></td>\n");
	} else {
		printf("<td>%d</td>\n", dt);
	}
	printf("<td><a onclick='return confirm(\"你真的要踢出该用户吗?\")' href=kick?k=%s&f=%s>Kick</a></td></tr>", x->userid, x->from);
}

static void
printkicklist(const char *usertokick)
{
	const struct user_info *x;
	int i, multilognum = 0;
	printf("<hr><table border=1>\n");
	printf("<tr><td>序号</td><td>使用者代号</td><td>使用者昵称</td><td>动态</td><td>发呆</td><td>踢人</td></tr>\n");
	for (i = 0; i < MAXACTIVE; i++) {
		x = ythtbbs_cache_utmp_get_by_idx(i);
		if (x->active == 0 || x->pid != 1)
			continue;
		if (!(strcasecmp(usertokick, x->userid))) {
			printkickitem(x);
			multilognum++;
		}
	}
	if (!multilognum) {
		printf("</table>");
		printf("没发现这个人在线吖,你敲错了?<br>");
		printf("<a href=kick> 再试一次? </a>");
		return;
	}
	printf("</table><hr>");
}

int
bbskick_main()
{
	char *tokick;
	char *f;
	int i;
	const struct user_info *x;
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("匆匆过客不能进行此操作, 请先登录");
	changemode(LUSERS);
	tokick = getparm("k");
	if (strlen(tokick) == 0) {
		//没有用户名参数k就要求输入一个,但不检查f
		printf("<hr><br><center><form action=kick name=userkicker method=post>");
		printf("请输入希望查询其在线情况的用户id  <input type=text name=k ><input type=submit value=查看></form><hr>");
	} else {
		if (!(currentuser.userlevel & PERM_SYSOP) && strcmp(currentuser.userid, tokick))
			http_fatal("错误的参数");
		f = getparm("f");
		// f长度为0时一般认为是从菜单访问的
		//生成按输入的id查询出来同时带k和f参数的名单
		if ((strlen(f) == 0) && (currentuser.userlevel & PERM_SYSOP)) {
			printkicklist(tokick);
			return 0;
		} else {
			for (i = 0; i < MAXACTIVE; i++) {
				x = ythtbbs_cache_utmp_get_by_idx(i);
				if (x->active == 0 || x->pid != 1 || strcmp(x->userid, tokick) || strcmp(x->from, f))
					continue;
				ythtbbs_cache_utmp_set_www_kicked(i);
				printf("已经踢出用户");
				return 0;
			}
		}
		printf("无法找到要踢出的用户");
	}
	return 0;
}

