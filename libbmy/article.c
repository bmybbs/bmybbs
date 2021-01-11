#include <stdio.h>
#include <string.h>
#include "bmy/mysql_wrapper.h"
#include "bmy/convcode.h"
#include "bmy/article.h"
#include "bmy/algorithms.h"

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

	memset(params, 0, sizeof(params));

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

	execute_prep_stmt(sql, MYSQL_CHARSET_UTF8, params, NULL, NULL, NULL);
}

void bmy_article_update_thread_accessed(int boardnum, time_t tid, int accessed) {
	const char *sql = "CALL procedure_update_thread_accessed(?, ?, ?)";
	MYSQL_BIND params[3];

	memset(params, 0, sizeof(params));

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

static void bmy_article_list_subscription_callback(MYSQL_STMT *stmt, MYSQL_BIND *result_cols, void *result_set) {
	struct bmy_articles **ptr = result_set;
	struct fileheader_utf *fh;
	size_t idx, rows;

	*ptr = NULL;
	rows = mysql_stmt_num_rows(stmt);
	if (rows == 0)
		return;

	*ptr = malloc(sizeof(struct bmy_articles));
	(*ptr)->count = rows;
	(*ptr)->articles = calloc(rows, sizeof(struct fileheader_utf));

	for (idx = 0; idx < rows; idx++) {
		fh = &(*ptr)->articles[idx];
		mysql_stmt_fetch(stmt);

		strcpy(fh->boardname_en, result_cols[0].buffer);
		strcpy(fh->boardname_zh, result_cols[1].buffer);
		fh->thread = *(time_t *)result_cols[2].buffer;
		strncpy(fh->title, result_cols[3].buffer, sizeof(fh->title));
		strcpy(fh->owner, result_cols[4].buffer);
		fh->count = *(unsigned int *)result_cols[5].buffer;
		fh->accessed = *(unsigned int *)result_cols[6].buffer;
	}
}

static struct bmy_articles *bmy_article_list_internal(const char *sql) {
	struct bmy_articles *article_list;
	struct fileheader_utf result_buf;
	MYSQL_BIND results[7];

	memset(results, 0, sizeof(results));

	results[0].buffer_type = MYSQL_TYPE_STRING;
	results[0].buffer = result_buf.boardname_en;
	results[0].buffer_length = sizeof(result_buf.boardname_en);

	results[1].buffer_type = MYSQL_TYPE_STRING;
	results[1].buffer = result_buf.boardname_zh;
	results[1].buffer_length = sizeof(result_buf.boardname_zh);

	results[2].buffer_type = MYSQL_TYPE_LONGLONG;
	results[2].buffer = &result_buf.thread;
	results[2].buffer_length = sizeof(time_t);

	results[3].buffer_type = MYSQL_TYPE_STRING;
	results[3].buffer = result_buf.title;
	results[3].buffer_length = sizeof(result_buf.title);

	results[4].buffer_type = MYSQL_TYPE_STRING;
	results[4].buffer = result_buf.owner;
	results[4].buffer_length = sizeof(result_buf.owner);

	results[5].buffer_type = MYSQL_TYPE_LONG;
	results[5].buffer = &result_buf.count;
	results[5].buffer_length = sizeof(unsigned int);

	results[6].buffer_type = MYSQL_TYPE_LONG;
	results[6].buffer = &result_buf.accessed;
	results[6].buffer_length = sizeof(unsigned int);

	execute_prep_stmt(sql, MYSQL_CHARSET_UTF8, NULL, results, (void *) &article_list, bmy_article_list_subscription_callback);

	return article_list;
}

struct bmy_articles *bmy_article_list_subscription(const char *userid, size_t limit, size_t offset) {
	char sqlbuf[160];

	snprintf(sqlbuf, sizeof(sqlbuf), "SELECT `boardname_en`, `boardname_zh`, `timestamp`, `title`, `author`, `comments`, `accessed` from v_feed_%s LIMIT %zu OFFSET %zu", userid, limit, offset);

	return bmy_article_list_internal(sqlbuf);
}

struct bmy_articles *bmy_article_list_subscription_by_time(const char *userid, size_t limit, time_t t) {
	char sqlbuf[160];

	snprintf(sqlbuf, sizeof(sqlbuf), "SELECT `boardname_en`, `boardname_zh`, `timestamp`, `title`, `author`, `comments`, `accessed` from v_feed_%s WHERE `timestamp` <= %ld LIMIT %zu", userid, t, limit);

	return bmy_article_list_internal(sqlbuf);
}

struct bmy_articles *bmy_article_list_section(char secid, size_t limit, time_t t) {
	char sqlbuf[160];

	snprintf(sqlbuf, sizeof(sqlbuf), "SELECT `boardname_en`, `boardname_zh`, `timestamp`, `title`, `author`, `comments`, `accessed` from v_section_%c WHERE `timestamp` <= %ld LIMIT %zu", secid, t, limit);
	return bmy_article_list_internal(sqlbuf);
}

struct bmy_articles *bmy_article_list_selected_boards(const int boardnum_array[], size_t num, size_t limit, time_t t) {
	char *sqlbuf = NULL, *s = NULL;
	size_t size = 0;
	struct bmy_articles *p = NULL;

	s = bmy_algo_join_int_array_to_string(boardnum_array, num, ',');
	if (s == NULL)
		goto END;
	size = strlen(s) + 512;
	sqlbuf = calloc(size, sizeof(char));
	if (sqlbuf == NULL)
		goto END;

	snprintf(sqlbuf, size, "SELECT `boardname_en`, `boardname_zh`, `timestamp`, `title`, `author`, `comments`, `accessed` from `t_boards`, `t_threads` WHERE `t_boards`.`boardnum` IN (%s) AND `t_boards`.`boardnum` = `t_threads`.`boardnum` AND `timestamp` <= %ld ORDER BY `timestamp` desc LIMIT %zu", s, t, limit);

	p = bmy_article_list_internal(sqlbuf);

END:
	if (s) free(s);
	if (sqlbuf) free(sqlbuf);
	return p;
}

void bmy_article_list_free(struct bmy_articles *ptr) {
	if (ptr) {
		if (ptr->count > 0 && ptr->articles) {
			free(ptr->articles);
		}
		free(ptr);
	}
}

