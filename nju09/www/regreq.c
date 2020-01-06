#include "bbslib.h"

int
regreq_main()
{
	html_header(1);
	printf("<nobr><center>%s -- 注册提示<hr>\n", BBSNAME);
	printf("<table width=100%%>\n");
	printf("亲爱的" MY_BBS_NAME "用户，您已经在本站浏览了%ld分钟了<br>",
	       (now_t - (w_info->login_start_time)) / 60);
	printf("请点<a href=bbsreg>这里</a>进行注册<br><br>");
	printf("· 一定要注册吗？<br>");
	printf("为了给各位网友提供更加优质的服务，更好地与大家交流沟通，");
	printf(MY_BBS_NAME "建议建议大家完成用户注册。");
	printf
	    ("注册成为我们的普通注册用户是十分方便快捷的。您将不会后悔花这一点点的时间，");
	printf("因为" MY_BBS_NAME
	       "将为注册成功的普通注册用户提供更多的特色服务。");
	printf("当然，即使没有进行注册也可轻松浏览" MY_BBS_NAME "的信息，");
	printf("但对于一些特色服务就只能望网兴叹了。");
	printf
	    ("为了您以后更好地享受" MY_BBS_NAME
	     "带给您的周到服务，赶快去注册吧！<br><br>");

	printf("· 先登录再浏览有什么好处？<br>");
	printf("我们建议习惯于使用www方式访问" MY_BBS_NAME
	       "时先进行用户登录，");
	printf("因为" MY_BBS_NAME
	       "许多版面的讨论和许多服务功能都必须进行用户登录才能使用。");
	printf
	    ("作为普通用户，养成一上网即登录的好习惯，能明显地节省您整个上网，");
	printf("讨论和查询的时间，并享受无以伦比的个性化服务。");
	printf("</table><br><hr>\n");
	printf("</center>");
	http_quit();
	return 0;
}
