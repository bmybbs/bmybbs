#include <string.h>
#include "config.h"
#include "ytht/fileop.h"
#include "ythtbbs/user.h"

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("usage: %s userid\n", argv[0]);
		return -1;
	}
	const char *userid = argv[1];
	struct mmapfile mf = { .ptr = NULL };
	struct userec *ec, *x;
	size_t count, i;

	if (mmapfile(MY_BBS_HOME "/" PASSFILE, &mf) >= 0) {
		ec = (struct userec *) mf.ptr;
		count = mf.size / sizeof(struct userec);

		for (i = 0; i < count; i++) {
			x = &ec[i];
			if (strcasecmp(x->userid, userid) == 0) {
				printf("%ld: %s\n", i, x->userid);
			}
		}
		mmapfile(NULL, &mf);
	}
	return 0;
}
