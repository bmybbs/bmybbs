#include "bbslib.h"

int
bbsdelmsg_main()
{
	html_header(1);
	if (!loginok || isguest)
		http_fatal("匆匆过客不能处理讯息, 请先登录");
	changemode(LOOKMSGS);
	clear_msg(currentuser.userid);
	u_info->unreadmsg = 0;
	printf("已删除所有讯息备份");
	return 0;
}
