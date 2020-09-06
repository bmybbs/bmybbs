#include "commands.h"
#include <sys/types.h>

extern FILE *yftpdutmp;

struct yftpdutmp {
	char bu_type;
	pid_t bu_pid;
	char bu_name[USERLEN + 1];
	char bu_host[256];
	time_t bu_time;
};

void yftpdutmp_init();
void yftpdutmp_end();
int yftpdutmp_log(char type);
char yftpdutmp_pidexists(pid_t pid);
