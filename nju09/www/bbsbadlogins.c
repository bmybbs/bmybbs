#include "bbslib.h"

int
bbsbadlogins_main()
{
	char file[STRLEN];
	html_header(1);
	printf("<body>");
	if (!loginok || isguest) {
		printf("哦，你并没有登陆，不用检查密码记录了</body></html>");
		return 0;
	}

	sethomefile_s(file, sizeof(file), currentuser.userid, BADLOGINFILE);
	if (!file_exist(file)) {
		printf("没有任何密码输入错误记录</body></html>");
		return 0;
	}
	if (*getparm("del") == '1') {
		unlink(file);
		printf("密码输入错误记录已被删除<br>");
		printf("<a href='#' onClick='javascript:window.close()'>关闭窗口</a>");
	} else {
		printf("发现以下密码输入错误记录<br><pre>");
		showfile(file);
		printf("</pre>");
		printf("<a href=bbsbadlogins?del=1>删除密码输入错误记录</a>");
	}
	printf("</body></html>");
	return 0;
}
