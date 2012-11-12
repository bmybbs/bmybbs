#include "bbslib.h"

int
bbslogout_main()
{
	struct userec *tmp;
	int st;
	char buf[50];
	int uid;
	html_header(1);
	//modified by safari@20091222
	if (!loginok) {
		redirect(FIRST_PAGE);
		http_quit();
		//http_fatal("你没有登录");
	}
	if (isguest)
		http_fatal("guest不带注销的");
	tmp = getuser(currentuser.userid);
	currentuser.numposts = tmp->numposts;
	currentuser.userlevel = tmp->userlevel;
	currentuser.numlogins = tmp->numlogins;
	currentuser.stay = tmp->stay;
	if (now_t > w_info->login_start_time) {
		st = now_t - w_info->login_start_time;
		if (st > 86400)
			errlog("Strange long stay time,%d!, logout, %s", st, currentuser.userid);
		else {
			currentuser.stay += st;
			sprintf(buf, "%s exitbbs %d", currentuser.userid, st);
			newtrace(buf);
		}
	}
	save_user_data(&currentuser);
	uid = u_info->uid;
	remove_uindex(u_info->uid, utmpent);
	bzero(u_info, sizeof (struct user_info));
	if ((currentuser.userlevel & PERM_BOARDS) && count_uindex(uid)==0)
		setbmstatus(&currentuser, 0);
	redirect(FIRST_PAGE);
	return 0;
}
