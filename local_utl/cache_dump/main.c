#include <stdio.h>
#include <string.h>
#include "ythtbbs/cache.h"

extern void ythtbbs_cache_utmp_dump(FILE *fp);
extern void ythtbbs_cache_UserTable_dump(FILE *fp);
extern void ythtbbs_cache_UserIDHashTable_dump(FILE *fp);
extern void ythtbbs_cache_Board_dump(FILE *fp);

static void dump_utmp() {
	ythtbbs_cache_utmp_resolve();
	ythtbbs_cache_utmp_dump(stdout);
}

static void dump_user_hash() {
	ythtbbs_cache_UserTable_resolve();
	ythtbbs_cache_UserIDHashTable_dump(stdout);
}

static void dump_user() {
	ythtbbs_cache_UserTable_resolve();
	ythtbbs_cache_UserTable_dump(stdout);
}

static void dump_board() {
	ythtbbs_cache_Board_resolve();
	ythtbbs_cache_Board_dump(stdout);
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		dump_utmp();
		dump_user();
		dump_user_hash();
		dump_board();
	} else {
		if (!strcasecmp(argv[1], "utmp")) {
			dump_utmp();
		} else if (!strcasecmp(argv[1], "user")) {
			dump_user();
		} else if (!strcasecmp(argv[1], "userhash")) {
			dump_user_hash();
		} else if (!strcasecmp(argv[1], "board")) {
			dump_board();
		} else {
			fprintf(stderr, "Usage:\n\t%s [ utmp | user | userhash | board]\n", argv[0]);
		}
	}
	return 0;
}

