#include <stdio.h>

#include <unistd.h>

#include "sv_core.h"

int
main(void)
{
	nice(1);
#if 0
	sv_core_httpd();
#endif
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
	if (fork() == 0) {
		setsid();
		sv_core_httpd();
	}
	return 0;
}
