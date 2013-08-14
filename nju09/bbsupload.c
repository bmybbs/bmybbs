#include "bbslib.h"

int
bbsupload_main()
{
	int i;
	html_header(1);
	if (!loginok || isguest)
		http_fatal("请登录!");
	changemode(POSTING);
	i = u_info - &(shm_utmp->uinfo[0]);
	printf("<body leftmargin=0 topmargin=0 marginwidth=0 marginheight=0>\n"
	       "<form method=POST ENCTYPE=\"multipart/form-data\" action=\"/cgi-bin/bbs/bbsupload.py/%c%c%c%s\"\n"
	       "<table width=100%%>\n"
	       "<td><INPUT NAME=\"userfile\" TYPE=\"file\" size=20><input type=submit value=\"粘贴\"></td>\n"
	       "<td><font color=red>不允许贴bmp格式的文件，不允许上传图片的附件，不允许在非贴图版面贴图，否则会被取消全站post权限。</font>在这里可以为文章附加点小图片小程序啥的, 不要贴太大的东西哦, 文件名里面也不要有括号问号什么的的, 否则会粘贴失败.</td></table>\n"
	       "</form>\n</body>", i / (26 * 26) + 'A', i / 26 % 26 + 'A',
	       i % 26 + 'A', u_info->sessionid);

	http_quit();
	return 0;
}
