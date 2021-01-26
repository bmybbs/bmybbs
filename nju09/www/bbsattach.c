#include "bbslib.h"
#include "check_server.h"

extern struct cgi_applet *get_cgi_applet(char *needcgi);

int
bbsattach_main()
{
	char *ptr, *path_info;
	struct cgi_applet *a;
	path_info = g_is_nginx ? g_url : getsenv("SCRIPT_URL");
	path_info = strchr(path_info + 1, '/');
	if (NULL == path_info)
		http_fatal("错误的文件名");
	if (!strncmp(path_info, "/attach/", 8))
		path_info += 8;
	else
		http_fatal("错误的文件名");
	ptr = strchr(path_info, '/');
	if (NULL == ptr)
		http_fatal("错误的文件名");
	*ptr = 0;
	a = get_cgi_applet(path_info);
	if (NULL == a)
		http_fatal("错误的文件名");
	return (*(a->main)) ();
}
