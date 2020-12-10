#include <string.h>
#include "bmy/convcode.h"
#include "bmy/article.h"

void copy_to_utf_header(struct fileheader_utf *dest, struct fileheader *src) {
	memset(dest, 0, sizeof(struct fileheader_utf));
	dest->filetime = src->filetime;
	dest->edittime = src->edittime;
	dest->thread   = src->thread;
	dest->accessed = src->accessed;
	dest->sizebyte = src->sizebyte;
	dest->viewtime = src->viewtime;
	dest->hasvoted = src->hasvoted;
	dest->deltime  = src->deltime;
	dest->staravg50 = src->staravg50;
	memcpy(dest->owner, src->owner, 14);

	int len = strlen(src->title);
	if (len > 60)
		len = 60;
	g2u(src->title, len, dest->title, 120);

	dest->count = 1; // the first one!
}

