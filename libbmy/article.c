#include <string.h>
#include "bmy/mysql_wrapper.h"
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

static void bmy_article_comment_internal(int boardnum, time_t tid, int delta) {
	const char *sql = "CALL procedure_update_thread_comments(?, ?, ?)";
	MYSQL_BIND params[3];

	memset(params, 0, sizeof(params));
	params[0].buffer_type = MYSQL_TYPE_LONG;
	params[0].buffer = &boardnum;
	params[0].buffer_length = sizeof(int);
	params[1].buffer_type = MYSQL_TYPE_LONGLONG;
	params[1].buffer = &tid;
	params[1].buffer_length = sizeof(time_t);
	params[2].buffer_type = MYSQL_TYPE_LONG;
	params[2].buffer = &delta;
	params[2].buffer_length = sizeof(int);

	execute_prep_stmt(sql, MYSQL_CHARSET_UTF8, params, NULL, NULL, NULL);
}

void bmy_article_add_comment(int boardnum, time_t tid) {
	bmy_article_comment_internal(boardnum, tid, 1);
}

void bmy_article_del_comment(int boardnum, time_t tid) {
	bmy_article_comment_internal(boardnum, tid, -1);
}

