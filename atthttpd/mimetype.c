#include <string.h>
#define CHARSET "gb2312"
#define HTTP_HEADER     "HTTP/1.0 200 OK\nContent-type: "
#define EOL	"\r\n"
char *
get_mime_type(char *name)
{
	char *dot;
	dot = strrchr(name, '.');
	if (dot == (char *) 0)
		return HTTP_HEADER "text/plain; charset=" CHARSET EOL;
	if (strcasecmp(dot, ".html") == 0 || strcasecmp(dot, ".htm") == 0)
		return HTTP_HEADER "text/html; charset=" CHARSET EOL;
	if (strcasecmp(dot, ".jpg") == 0 || strcasecmp(dot, ".jpeg") == 0)
		return HTTP_HEADER "image/jpeg" EOL;
	if (strcasecmp(dot, ".gif") == 0)
		return HTTP_HEADER "image/gif" EOL;
	if (strcasecmp(dot, ".png") == 0)
		return HTTP_HEADER "image/png" EOL;
	if (strcasecmp(dot, ".pcx") == 0)
		return HTTP_HEADER "image/pcx" EOL;
	if (strcasecmp(dot, ".css") == 0)
		return HTTP_HEADER "text/css" EOL;
	if (strcasecmp(dot, ".au") == 0)
		return HTTP_HEADER "audio/basic" EOL;
	if (strcasecmp(dot, ".wav") == 0)
		return HTTP_HEADER "audio/wav" EOL;
	if (strcasecmp(dot, ".avi") == 0)
		return HTTP_HEADER "video/x-msvideo" EOL;
	if (strcasecmp(dot, ".mov") == 0 || strcasecmp(dot, ".qt") == 0)
		return HTTP_HEADER "video/quicktime" EOL;
	if (strcasecmp(dot, ".mpeg") == 0 || strcasecmp(dot, ".mpe") == 0)
		return HTTP_HEADER "video/mpeg" EOL;
	if (strcasecmp(dot, ".vrml") == 0 || strcasecmp(dot, ".wrl") == 0)
		return HTTP_HEADER "model/vrml" EOL;
	if (strcasecmp(dot, ".midi") == 0 || strcasecmp(dot, ".mid") == 0)
		return HTTP_HEADER "audio/midi" EOL;
	if (strcasecmp(dot, ".mp3") == 0)
		return HTTP_HEADER "audio/mpeg" EOL;
	if (strcasecmp(dot, ".pac") == 0)
		return HTTP_HEADER "application/x-ns-proxy-autoconfig" EOL;
	if (strcasecmp(dot, ".txt") == 0)
		return HTTP_HEADER "text/plain; charset=" CHARSET EOL;
	if (strcasecmp(dot, ".xht") == 0 || strcasecmp(dot, ".xhtml") == 0)
		return HTTP_HEADER "application/xhtml+xml" EOL;
	if (strcasecmp(dot, ".xml") == 0)
		return "text/xml";
	return HTTP_HEADER "application/octet-stream" EOL;
}

char *
get_error_type(int errornum)
{
	(void) errornum;
	return "HTTP/1.1 404 Not Found" EOL;
}
