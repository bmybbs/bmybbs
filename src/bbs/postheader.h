#ifndef BMYBBS_POSTHEADER_H
#define BMYBBS_POSTHEADER_H
#include "config.h"

struct postheader {
	char title[STRLEN];
	char ds[40];
	int reply_mode;
	char include_mode;
	int chk_anony;
	int postboard;
	int canreply;
	int mailreply;
};

int post_header(struct postheader *header);
#endif //BMYBBS_POSTHEADER_H
