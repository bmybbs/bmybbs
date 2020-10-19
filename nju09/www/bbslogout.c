#include "bbslib.h"

int
bbslogout_main()
{
	char buf[50];
	time_t t0;

	//modified by safari@20091222
	if (!loginok) {
		redirect(FIRST_PAGE);
		http_quit();
		//http_fatal("你没有登录");
	}
	if (isguest)
		http_fatal("guest\xB2\xBB\xB4\xF8\xD7\xA2\xCF\xFA"); // guest不带注销的

	ythtbbs_user_logout(currentuser.userid, utmpent - 1);

	t0 = 0;
	ytht_utc_time_s(buf, sizeof(buf), &t0);
	printf("Set-Cookie: " SMAGIC "=; Expires=%s", buf);
	html_header(1);
	redirect(FIRST_PAGE);
	return 0;
}

