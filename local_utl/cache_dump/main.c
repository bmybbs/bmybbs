#include <stdio.h>
#include "ythtbbs/cache.h"

extern void ythtbbs_cache_utmp_dump(FILE *fp);
extern void ythtbbs_cache_UserTable_dump(FILE *fp);
extern void ythtbbs_cache_UserIDHashTable_dump(FILE *fp);

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	ythtbbs_cache_utmp_resolve();
	ythtbbs_cache_UserTable_resolve();

	ythtbbs_cache_UserIDHashTable_dump(stdout);
	ythtbbs_cache_UserTable_dump(stdout);
	ythtbbs_cache_utmp_dump(stdout);
	return 0;
}

