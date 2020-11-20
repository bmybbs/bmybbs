#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

bool g_is_nginx = false;
char *g_url;
char g_url_buf[1024];

void check_server(void) {
	char *s = getenv("SERVER_SOFTWARE");

	if (s == NULL)
		return;

	if (strncasecmp(s, "nginx/", 6) == 0)
		g_is_nginx = true;
}

