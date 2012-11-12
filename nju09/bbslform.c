#include "bbslib.h"
#define LOGTMP HTMPATH "/login.htm"

int
bbslform_main()
{
	static struct mmapfile mf = { ptr:NULL };
	html_header(1);
	if (loginok && !isguest)
		http_fatal("你已经登录了啊...");
	if (mmapfile(LOGTMP, &mf) < 0)
		http_fatal("无法打开模板文件");
	fwrite(mf.ptr, 1, mf.size, stdout);
	printf("<center><a href='/" SMAGIC "/bbsfindpass' target='_blank'> 找回用户名或密码</a></center>\n");
	return 0;
}
