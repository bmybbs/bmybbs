#include "bbslib.h"
#include "ytht/random.h"
#include "bmy/cookie.h"

int bbslogin_main() {
	int rc;
	char buf[128], id[20], pw[20];
	struct user_info info;
	struct bmy_cookie cookie;

	ytht_strsncpy(id, getparm("id"), 13);
	ytht_strsncpy(pw, getparm("pw"), 13);

	if (loginok && strcasecmp(id, currentuser.userid) && !isguest) {
		http_fatal("系统检测到目前你的计算机上已经登录有一个帐号 %s，请先退出.(选择正常logout)", currentuser.userid);
	}

	if (!strcmp(id, "")) {
		strcpy(id, "guest");
	}

	rc = ythtbbs_user_login(id, pw, fromhost, YTHTBBS_LOGIN_NJU09, &info, &currentuser, NULL);
	if (rc != YTHTBBS_USER_LOGIN_OK) {
		switch(rc) {
		case YTHTBBS_USER_NOT_EXIST:
			http_fatal("错误的使用者帐号");
			break;
		case YTHTBBS_USER_WRONG_PASSWORD:
			http_fatal("密码错误");
			break;
		case YTHTBBS_USER_SUSPENDED:
			http_fatal("此帐号已被停机, 若有疑问, 请用其他帐号在sysop版询问.");
			break;
		case YTHTBBS_USER_SITE_BAN:
			http_fatal("对不起, 本站不欢迎来自 [%s] 的登录. <br>若有疑问, 请与SYSOP联系.", fromhost);
			break;
		case YTHTBBS_USER_USER_BAN:
			http_fatal("本ID已设置禁止从%s登录", fromhost);
			break;
		case YTHTBBS_USER_TOO_FREQUENT:
			http_fatal("两次登录间隔过密!");
			break;
		case YTHTBBS_USER_IN_PRISON:
			http_fatal("安心改造，不要胡闹");
			break;
		case YTHTBBS_USER_ONLINE_FULL:
			http_fatal("抱歉，目前在线用户数已达上限，无法登录。请稍后再来。");
			break;
		default:
			http_fatal("unknown");
			break;
		}
	}

	if (strcasecmp(id, "guest")) {
		if (!readuservalue(info.userid, "wwwstyle", buf, sizeof (buf)))
			wwwstylenum = atoi(buf);
		if (wwwstylenum < 0 || wwwstylenum >= NWWWSTYLE)
			wwwstylenum = 1;
	} else {
		wwwstylenum = 1;
	}

	cookie.userid = info.userid;
	cookie.sessid = info.sessionid;
	cookie.token  = info.token;
	cookie.extraparam = getextrparam_str(wwwstylenum);
	bmy_cookie_gen(buf, sizeof(buf), &cookie);

	printf("Set-Cookie: " SMAGIC "=%s; SameSite=Strict; HttpOnly;", buf);
	html_header(3);
	redirect("/" SMAGIC "/"); // URL 不再附带 session 信息
	http_quit();
	return 0;
}

