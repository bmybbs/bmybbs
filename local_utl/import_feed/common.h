#ifndef LOCAL_UTL_IMPORT_FEED_COMMON_H
#define LOCAL_UTL_IMPORT_FEED_COMMON_H
#include <time.h>
#include <stdbool.h>
#include "ythtbbs/article.h"

struct fileheader_utf {
	time_t filetime;
	time_t edittime;
	time_t thread;
	unsigned int accessed;
	char title[120];
	char owner[14];
	unsigned short viewtime;
	unsigned char sizebyte;
	unsigned char staravg50;
	unsigned char hasvoted;
	char deltime;
	unsigned int count;
	int boardnum;

	struct fileheader_utf *next;
};

bool is_system_board(const char *boardname);
void copy_to_utf_header(struct fileheader_utf *dest, struct fileheader *src);

#endif

