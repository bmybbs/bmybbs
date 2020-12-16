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

void bmy_article_add_thread(int boardnum, time_t tid, char *title_gbk, char *author, int accessed) {
	const char *sql = "INSERT INTO `t_threads`(`boardnum`, `timestamp`, `title`, `author`, `comments`, `accessed`) VALUES(?, ?, ?, ?, ?, ?)";
	char title_utf[120];
	size_t len;
	MYSQL_BIND params[6];
	char *anonymous = "Anonymous";

	len = strlen(title_gbk);
	if (len > 60) /* magic number: see struct fileheader::title */
		len = 60;
	g2u(title_gbk, len, title_utf, sizeof(title_utf));

	memset(params, 0, sizeof(params));

	params[0].buffer_type = MYSQL_TYPE_LONG;
	params[0].buffer = &boardnum;
	params[0].buffer_length = sizeof(int);

	params[1].buffer_type = MYSQL_TYPE_LONGLONG;
	params[1].buffer = &tid;
	params[1].buffer_length = sizeof(time_t);

	params[2].buffer_type = MYSQL_TYPE_STRING;
	params[2].buffer = title_utf;
	params[2].buffer_length = strlen(title_utf);

	params[3].buffer_type = MYSQL_TYPE_STRING;
	params[3].buffer = (author && author[0] ? author : anonymous);
	params[3].buffer_length = strlen(params[3].buffer);

	params[4].buffer_type = MYSQL_TYPE_LONG;
	params[4].buffer = (int []){ 1 }; // anonymous array
	params[4].buffer_length = sizeof(int);

	params[5].buffer_type = MYSQL_TYPE_LONG;
	params[5].buffer = &accessed;
	params[5].buffer_length = sizeof(int);

	execute_prep_stmt(sql, MYSQL_CHARSET_UTF8, params, NULL, NULL, NULL);
}

void bmy_article_del_thread(int boardnum, time_t tid) {
	const char *sql = "CALL procedure_delete_thread(?, ?)";
	MYSQL_BIND params[2];

	params[0].buffer_type = MYSQL_TYPE_LONG;
	params[0].buffer = &boardnum;
	params[0].buffer_length = sizeof(int);
	params[1].buffer_type = MYSQL_TYPE_LONGLONG;
	params[1].buffer = &tid;
	params[1].buffer_length = sizeof(time_t);

	execute_prep_stmt(sql, MYSQL_CHARSET_UTF8, params, NULL, NULL, NULL);
}

void bmy_article_update_thread_title(int boardnum, time_t tid, char *title_gbk) {
	const char *sql = "CALL procedure_update_thread_title(?, ?, ?)";
	char title_utf[120];
	MYSQL_BIND params[3];
	size_t len;

	len = strlen(title_gbk);
	if (len > 60)
		len = 60;
	g2u(title_gbk, len, title_utf, sizeof(title_utf));

	params[0].buffer_type = MYSQL_TYPE_LONG;
	params[0].buffer = &boardnum;
	params[0].buffer_length = sizeof(int);

	params[1].buffer_type = MYSQL_TYPE_LONGLONG;
	params[1].buffer = &tid;
	params[1].buffer_length = sizeof(time_t);

	params[2].buffer_type = MYSQL_TYPE_STRING;
	params[2].buffer = title_utf;
	params[2].buffer_length = strlen(title_utf);

	execute_prep_stmt(sql, MYSQL_CHARSET_UTF8, params, NULL, NULL, NULL);
}

void bmy_article_update_thread_accessed(int boardnum, time_t tid, int accessed) {
	const char *sql = "CALL procedure_update_thread_accessed(?, ?, ?)";
	MYSQL_BIND params[3];

	params[0].buffer_type = MYSQL_TYPE_LONG;
	params[0].buffer = &boardnum;
	params[0].buffer_length = sizeof(int);

	params[1].buffer_type = MYSQL_TYPE_LONGLONG;
	params[1].buffer = &tid;
	params[1].buffer_length = sizeof(time_t);

	params[2].buffer_type = MYSQL_TYPE_LONG;
	params[2].buffer = &accessed;
	params[2].buffer_length = sizeof(int);

	execute_prep_stmt(sql, MYSQL_CHARSET_UTF8, params, NULL, NULL, NULL);
}

