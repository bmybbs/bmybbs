#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "config.h"
#include "ythtbbs/record.h"
#include "ythtbbs/article.h"

int main(int argc, char *argv[]) {
	unsigned int post_no;
	time_t timestamp;
	struct fileheader fh;
	char buf[256];

	if (argc != 4) {
		fprintf(stderr, "[usage] program <boardname> <post_no.> <timestamp>\n");
		return -1;
	}

	post_no = atoi(argv[2]);
	timestamp = atol(argv[3]);

	snprintf(buf, sizeof(buf), MY_BBS_HOME "/boards/%s/.DIR", argv[1]);
	if (get_record(buf, &fh, sizeof(struct fileheader), post_no) != 0) {
		fprintf(stderr, "cannot get record at %s:%d\n", buf, post_no);
		return -2;
	}

	fh.thread = timestamp;
	if (substitute_record(buf, &fh, sizeof(struct fileheader), post_no) != 0) {
		fprintf(stderr, "cannot substitute record at %s:%d\n", buf, post_no);
		return -3;
	}

	fprintf(stdout, "Board[%s] No.[%d] updated\n", argv[1], post_no);
	return 0;
}

