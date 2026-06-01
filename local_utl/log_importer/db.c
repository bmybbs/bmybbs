#include "db.h"
#include "bmy/pg_wrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void dump_sql_error(PGconn *conn);

static bool bmy_log_importer_insert_imported_line(
	PGconn *conn,
	const char *source_file_id,
	unsigned long source_line,
	const char *event_table,
	const char *event_id);
static bool bmy_log_importer_insert_article(
	PGconn *conn,
	const char *occurred_at,
	const struct bmy_log_article_event *event,
	char **event_id);
static bool bmy_log_importer_insert_range_delete(
	PGconn *conn,
	const char *occurred_at,
	const struct bmy_log_range_delete_event *event,
	char **event_id);
static bool bmy_log_importer_insert_board_usage(
	PGconn *conn,
	const char *occurred_at,
	const struct bmy_log_board_usage_event *event,
	char **event_id);
static bool bmy_log_importer_insert_session_duration(
	PGconn *conn,
	const char *occurred_at,
	const struct bmy_log_session_duration_event *event,
	char **event_id);
static bool bmy_log_importer_insert_login_failure(
	PGconn *conn,
	const char *occurred_at,
	const struct bmy_log_login_failure_event *event,
	char **event_id);
static bool bmy_log_importer_insert_security(
	PGconn *conn,
	const char *occurred_at,
	const struct bmy_log_security_event *event,
	char **event_id);
static bool bmy_log_importer_insert_login_success(
	PGconn *conn,
	const char *occurred_at,
	const struct bmy_log_login_success_event *event,
	char **event_id);
static bool bmy_log_importer_insert_session(
	PGconn *conn,
	const char *occurred_at,
	const struct bmy_log_session_event *event,
	char **event_id);
static bool bmy_log_importer_insert_account(
	PGconn *conn,
	const char *occurred_at,
	const struct bmy_log_account_event *event,
	char **event_id);
static bool bmy_log_importer_insert_mail(
	PGconn *conn,
	const char *occurred_at,
	const struct bmy_log_mail_event *event,
	char **event_id);
static bool bmy_log_importer_insert_user_interaction(
	PGconn *conn,
	const char *occurred_at,
	const struct bmy_log_user_interaction_event *event,
	char **event_id);
static bool bmy_log_importer_insert_user_query(
	PGconn *conn,
	const char *occurred_at,
	const struct bmy_log_user_query_event *event,
	char **event_id);
static bool bmy_log_importer_insert_announcement(
	PGconn *conn,
	const char *occurred_at,
	const struct bmy_log_announcement_event *event,
	char **event_id);
static bool bmy_log_importer_insert_board_deny(
	PGconn *conn,
	const char *occurred_at,
	const struct bmy_log_board_deny_event *event,
	char **event_id);
/**
 * @brief 对插入记录的封装
 *
 * @warning 在执行成功，也就是返回值为 true 的情况下，需要使用 `free` 释放 event_id
 *
 * @param conn 数据库连接
 * @param sql SQL Statement
 * @param n_params 参数个数
 * @param params 参数列表
 * @param event_id 记录 ID
 * @returns true 成功 false 失败
 */
static bool bmy_log_importer_exec_params_returning_id(
	PGconn *conn,
	const char *sql,
	int n_params,
	const char *const *params,
	char **event_id);
static const char *bmy_log_importer_table_name(enum bmy_log_event_table table);

PGconn * bmy_log_importer_db_connect(void) {
	PGconn *conn = bmy_pg_connect_default();

	if (conn == NULL) {
		fprintf(stderr, "PostgreSQL connection allocation failed\n");
		return NULL;
	}

	if (PQstatus(conn) != CONNECTION_OK) {
		fprintf(stderr, "PostgreSQL connection failed: %s", PQerrorMessage(conn));
		PQfinish(conn);
		return NULL;
	}

	return conn;
}

int bmy_log_importer_source_file_exists(PGconn *conn, const char *source_file, bool *exists) {
	const char *params[] = { source_file };
	PGresult *res;

	*exists = false;
	res = bmy_pg_exec_params(conn,
		"SELECT 1 FROM log_source_files WHERE source_file = $1 LIMIT 1",
		1, params);
	if (res == NULL) {
		fprintf(stderr, "PostgreSQL query allocation failed\n");
		return -1;
	}
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "source file lookup failed: %s", PQerrorMessage(conn));
		PQclear(res);
		return -1;
	}

	*exists = PQntuples(res) > 0;
	PQclear(res);
	return 0;
}

bool bmy_log_importer_ensure_source_file(PGconn *conn, const char *source_file, char **source_file_id) {
	const char *params[] = { source_file };
	PGresult *res;
	bool ok;

	res = bmy_pg_exec_params(conn,
		"WITH inserted AS ("
		"INSERT INTO log_source_files (source_file) VALUES ($1) "
		"ON CONFLICT (source_file) DO NOTHING "
		"RETURNING id"
		") "
		"SELECT id FROM inserted "
		"UNION ALL "
		"SELECT id FROM log_source_files WHERE source_file = $1 "
		"LIMIT 1",
		1, params);
	if (res == NULL) {
		fprintf(stderr, "PostgreSQL query allocation failed\n");
		return false;
	}

	ok = PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) == 1;
	if (!ok) {
		fprintf(stderr, "source file lookup failed: %s", PQerrorMessage(conn));
		PQclear(res);
		return false;
	}

	*source_file_id = strdup(PQgetvalue(res, 0, 0));
	PQclear(res);
	return *source_file_id != NULL;
}

int bmy_log_importer_is_line_imported(PGconn *conn, const char *source_file_id, unsigned long source_line, bool *imported) {
	char source_line_buf[32];
	const char *params[2];
	PGresult *res;

	snprintf(source_line_buf, sizeof(source_line_buf), "%lu", source_line);
	params[0] = source_file_id;
	params[1] = source_line_buf;
	*imported = false;

	res = bmy_pg_exec_params(conn,
		"SELECT 1 FROM log_imported_lines WHERE source_file_id = $1 AND source_line = $2 LIMIT 1",
		2, params);
	if (res == NULL) {
		fprintf(stderr, "PostgreSQL query allocation failed\n");
		return -1;
	}
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "import lookup failed: %s", PQerrorMessage(conn));
		PQclear(res);
		return -1;
	}

	*imported = PQntuples(res) > 0;
	PQclear(res);
	return 0;
}

bool bmy_log_importer_insert_event(PGconn *conn, const char *source_file_id, unsigned long source_line, const char *occurred_at, const struct bmy_log_parse_result *result) {
	bool ok;

	if (!bmy_pg_begin(conn)) {
		dump_sql_error(conn);
		return false;
	}

	ok = bmy_log_importer_insert_event_in_transaction(conn, source_file_id, source_line, occurred_at, result);

	if (!ok) {
		if (!bmy_pg_rollback(conn)) {
			dump_sql_error(conn);
		}
		return false;
	}

	if (!(ok = bmy_pg_commit(conn))) {
		dump_sql_error(conn);
		return false;
	}
	return ok;
}

bool bmy_log_importer_insert_event_in_transaction(PGconn *conn, const char *source_file_id, unsigned long source_line, const char *occurred_at, const struct bmy_log_parse_result *result) {
	const char *event_table;
	char *event_id = NULL;
	bool ok = false;

	switch (result->table) {
		case BMY_LOG_EVENT_ARTICLE:
			ok = bmy_log_importer_insert_article(conn, occurred_at, &result->payload.article, &event_id);
			break;
		case BMY_LOG_EVENT_RANGE_DELETE:
			ok = bmy_log_importer_insert_range_delete(conn, occurred_at, &result->payload.range_delete, &event_id);
			break;
		case BMY_LOG_EVENT_BOARD_USAGE:
			ok = bmy_log_importer_insert_board_usage(conn, occurred_at, &result->payload.board_usage, &event_id);
			break;
		case BMY_LOG_EVENT_SESSION_DURATION:
			ok = bmy_log_importer_insert_session_duration(conn, occurred_at, &result->payload.session_duration, &event_id);
			break;
		case BMY_LOG_EVENT_LOGIN_FAILURE:
			ok = bmy_log_importer_insert_login_failure(conn, occurred_at, &result->payload.login_failure, &event_id);
			break;
		case BMY_LOG_EVENT_SECURITY:
			ok = bmy_log_importer_insert_security(conn, occurred_at, &result->payload.security, &event_id);
			break;
		case BMY_LOG_EVENT_LOGIN_SUCCESS:
			ok = bmy_log_importer_insert_login_success(conn, occurred_at, &result->payload.login_success, &event_id);
			break;
		case BMY_LOG_EVENT_SESSION:
			ok = bmy_log_importer_insert_session(conn, occurred_at, &result->payload.session, &event_id);
			break;
		case BMY_LOG_EVENT_ACCOUNT:
			ok = bmy_log_importer_insert_account(conn, occurred_at, &result->payload.account, &event_id);
			break;
		case BMY_LOG_EVENT_MAIL:
			ok = bmy_log_importer_insert_mail(conn, occurred_at, &result->payload.mail, &event_id);
			break;
		case BMY_LOG_EVENT_USER_INTERACTION:
			ok = bmy_log_importer_insert_user_interaction(conn, occurred_at, &result->payload.user_interaction, &event_id);
			break;
		case BMY_LOG_EVENT_USER_QUERY:
			ok = bmy_log_importer_insert_user_query(conn, occurred_at, &result->payload.user_query, &event_id);
			break;
		case BMY_LOG_EVENT_ANNOUNCEMENT:
			ok = bmy_log_importer_insert_announcement(conn, occurred_at, &result->payload.announcement, &event_id);
			break;
		case BMY_LOG_EVENT_BOARD_DENY:
			ok = bmy_log_importer_insert_board_deny(conn, occurred_at, &result->payload.board_deny, &event_id);
			break;
	}

	event_table = bmy_log_importer_table_name(result->table);
	if (ok) {
		ok = bmy_log_importer_insert_imported_line(conn, source_file_id, source_line, event_table, event_id);
	}

	free(event_id);
	return ok;
}

static bool bmy_log_importer_insert_imported_line( PGconn *conn, const char *source_file_id, unsigned long source_line, const char *event_table, const char *event_id) {
	char source_line_buf[32];
	const char *params[4];
	PGresult *res;
	bool ok;

	snprintf(source_line_buf, sizeof(source_line_buf), "%lu", source_line);
	params[0] = source_file_id;
	params[1] = source_line_buf;
	params[2] = event_table;
	params[3] = event_id;

	res = bmy_pg_exec_params(conn,
		"INSERT INTO log_imported_lines (source_file_id, source_line, event_table, event_id) VALUES ($1, $2, $3, $4)",
		4, params);
	if (res == NULL) {
		fprintf(stderr, "PostgreSQL query allocation failed\n");
		return false;
	}
	ok = PQresultStatus(res) == PGRES_COMMAND_OK;
	if (!ok) {
		fprintf(stderr, "insert log_imported_lines failed: %s", PQerrorMessage(conn));
	}
	PQclear(res);
	return ok;
}

static bool bmy_log_importer_insert_article(PGconn *conn, const char *occurred_at, const struct bmy_log_article_event *event, char **event_id) {
	const char *params[] = {
		occurred_at,
		event->actor_userid,
		event->board,
		event->owner_userid,
		event->title,
		event->old_title,
		event->action,
	};

	return bmy_log_importer_exec_params_returning_id(conn,
		"INSERT INTO log_article_events (occurred_at, actor_userid, board, owner_userid, title, old_title, action) VALUES ($1, $2, $3, $4, $5, $6, $7) RETURNING id",
		7, params, event_id);
}

static bool bmy_log_importer_insert_range_delete(PGconn *conn, const char *occurred_at, const struct bmy_log_range_delete_event *event, char **event_id) {
	char from_id[32];
	char to_id[32];
	const char *params[] = {
		occurred_at,
		event->scope,
		event->userid,
		event->board,
		from_id,
		to_id,
	};

	snprintf(from_id, sizeof(from_id), "%d", event->from_id);
	snprintf(to_id, sizeof(to_id), "%d", event->to_id);
	return bmy_log_importer_exec_params_returning_id(conn,
		"INSERT INTO log_range_delete_events (occurred_at, scope, userid, board, from_id, to_id) VALUES ($1, $2, $3, $4, $5, $6) RETURNING id",
		6, params, event_id);
}

static bool bmy_log_importer_insert_board_usage(PGconn *conn, const char *occurred_at, const struct bmy_log_board_usage_event *event, char **event_id) {
	char stay_seconds[32];
	const char *params[] = {
		occurred_at,
		event->userid,
		event->board,
		stay_seconds,
	};

	snprintf(stay_seconds, sizeof(stay_seconds), "%ld", event->stay_seconds);
	return bmy_log_importer_exec_params_returning_id(conn,
		"INSERT INTO log_board_usage_events (occurred_at, userid, board, stay_seconds) VALUES ($1, $2, $3, $4) RETURNING id",
		4, params, event_id);
}

static bool bmy_log_importer_insert_session_duration(PGconn *conn, const char *occurred_at, const struct bmy_log_session_duration_event *event, char **event_id) {
	char stay_seconds[32];
	const char *params[] = {
		occurred_at,
		event->userid,
		event->action,
		stay_seconds,
	};

	snprintf(stay_seconds, sizeof(stay_seconds), "%ld", event->stay_seconds);
	return bmy_log_importer_exec_params_returning_id(conn,
		"INSERT INTO log_session_duration_events (occurred_at, userid, action, stay_seconds) VALUES ($1, $2, $3, $4) RETURNING id",
		4, params, event_id);
}

static bool bmy_log_importer_insert_login_failure(PGconn *conn, const char *occurred_at, const struct bmy_log_login_failure_event *event, char **event_id) {
	const char *params[] = {
		occurred_at,
		event->from_host,
	};

	return bmy_log_importer_exec_params_returning_id(conn,
		"INSERT INTO log_login_failure_events (occurred_at, from_host) VALUES ($1, $2) RETURNING id",
		2, params, event_id);
}

static bool bmy_log_importer_insert_security(PGconn *conn, const char *occurred_at, const struct bmy_log_security_event *event, char **event_id) {
	const char *params[] = {
		occurred_at,
		event->action,
		event->input_value,
		event->from_host,
	};

	return bmy_log_importer_exec_params_returning_id(conn,
		"INSERT INTO log_security_events (occurred_at, action, input_value, from_host) VALUES ($1, $2, $3, $4) RETURNING id",
		4, params, event_id);
}

static bool bmy_log_importer_insert_login_success(PGconn *conn, const char *occurred_at, const struct bmy_log_login_success_event *event, char **event_id) {
	const char *params[] = {
		occurred_at,
		event->userid,
		event->from_host,
		event->login_type,
	};

	return bmy_log_importer_exec_params_returning_id(conn,
		"INSERT INTO log_login_success_events (occurred_at, userid, from_host, login_type) VALUES ($1, $2, $3, $4) RETURNING id",
		4, params, event_id);
}

static bool bmy_log_importer_insert_session(PGconn *conn, const char *occurred_at, const struct bmy_log_session_event *event, char **event_id) {
	const char *params[] = {
		occurred_at,
		event->action,
		event->userid,
		event->target_userid,
	};

	return bmy_log_importer_exec_params_returning_id(conn,
		"INSERT INTO log_session_events (occurred_at, action, userid, target_userid) VALUES ($1, $2, $3, $4) RETURNING id",
		4, params, event_id);
}

static bool bmy_log_importer_insert_account(PGconn *conn, const char *occurred_at, const struct bmy_log_account_event *event, char **event_id) {
	char user_index_value[32];
	char life_value[32];
	const bool is_create = strcmp(event->action, "create") == 0;
	const char *params[] = {
		occurred_at,
		event->action,
		event->userid,
		is_create ? user_index_value : NULL,
		is_create ? NULL : life_value,
		event->from_host,
		event->login_type,
	};

	snprintf(user_index_value, sizeof(user_index_value), "%d", event->user_index_value);
	snprintf(life_value, sizeof(life_value), "%d", event->life_value);
	return bmy_log_importer_exec_params_returning_id(conn,
		"INSERT INTO log_account_events (occurred_at, action, userid, user_index_value, life_value, from_host, login_type) VALUES ($1, $2, $3, $4, $5, $6, $7) RETURNING id",
		7, params, event_id);
}

static bool bmy_log_importer_insert_mail(PGconn *conn, const char *occurred_at, const struct bmy_log_mail_event *event, char **event_id) {
	const char *params[] = {
		occurred_at,
		event->sender,
		event->target_userid,
	};

	return bmy_log_importer_exec_params_returning_id(conn,
		"INSERT INTO log_mail_events (occurred_at, sender, target_userid) VALUES ($1, $2, $3) RETURNING id",
		3, params, event_id);
}

static bool bmy_log_importer_insert_user_interaction(PGconn *conn, const char *occurred_at, const struct bmy_log_user_interaction_event *event, char **event_id) {
	const char *params[] = {
		occurred_at,
		event->action,
		event->userid,
		event->target_userid,
	};

	return bmy_log_importer_exec_params_returning_id(conn,
		"INSERT INTO log_user_interaction_events (occurred_at, action, userid, target_userid) VALUES ($1, $2, $3, $4) RETURNING id",
		4, params, event_id);
}

static bool bmy_log_importer_insert_user_query(PGconn *conn, const char *occurred_at, const struct bmy_log_user_query_event *event, char **event_id) {
	char day_count[32];
	const char *params[] = {
		occurred_at,
		event->action,
		event->userid,
		event->target,
		day_count,
	};

	snprintf(day_count, sizeof(day_count), "%d", event->day_count);
	return bmy_log_importer_exec_params_returning_id(conn,
		"INSERT INTO log_user_query_events (occurred_at, action, userid, target, day_count) VALUES ($1, $2, $3, $4, $5) RETURNING id",
		5, params, event_id);
}

static bool bmy_log_importer_insert_announcement(PGconn *conn, const char *occurred_at, const struct bmy_log_announcement_event *event, char **event_id) {
	const char *params[] = {
		occurred_at,
		event->action,
		event->userid,
		event->board,
		event->path,
		event->owner_userid,
		event->title,
	};

	return bmy_log_importer_exec_params_returning_id(conn,
		"INSERT INTO log_announcement_events (occurred_at, action, userid, board, path, owner_userid, title) VALUES ($1, $2, $3, $4, $5, $6, $7) RETURNING id",
		7, params, event_id);
}

static bool bmy_log_importer_insert_board_deny(PGconn *conn, const char *occurred_at, const struct bmy_log_board_deny_event *event, char **event_id) {
	const char *params[] = {
		occurred_at,
		event->operator_userid,
		event->board,
		event->target_userid,
	};

	return bmy_log_importer_exec_params_returning_id(conn,
		"INSERT INTO log_board_deny_events (occurred_at, operator_userid, board, target_userid) VALUES ($1, $2, $3, $4) RETURNING id",
		4, params, event_id);
}

static bool bmy_log_importer_exec_params_returning_id(PGconn *conn, const char *sql, int n_params, const char *const *params, char **event_id) {
	PGresult *res = bmy_pg_exec_params(conn, sql, n_params, params);
	bool ok;

	if (res == NULL) {
		fprintf(stderr, "PostgreSQL query allocation failed\n");
		return false;
	}

	ok = PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) == 1;

	if (!ok) {
		fprintf(stderr, "insert event failed: %s", PQerrorMessage(conn));
		PQclear(res);
		return false;
	}

	*event_id = strdup(PQgetvalue(res, 0, 0));
	PQclear(res);
	return *event_id != NULL;
}

static const char * bmy_log_importer_table_name(enum bmy_log_event_table table) {
	switch (table) {
		case BMY_LOG_EVENT_ARTICLE:
			return "log_article_events";
		case BMY_LOG_EVENT_RANGE_DELETE:
			return "log_range_delete_events";
		case BMY_LOG_EVENT_BOARD_USAGE:
			return "log_board_usage_events";
		case BMY_LOG_EVENT_SESSION_DURATION:
			return "log_session_duration_events";
		case BMY_LOG_EVENT_LOGIN_FAILURE:
			return "log_login_failure_events";
		case BMY_LOG_EVENT_SECURITY:
			return "log_security_events";
		case BMY_LOG_EVENT_LOGIN_SUCCESS:
			return "log_login_success_events";
		case BMY_LOG_EVENT_SESSION:
			return "log_session_events";
		case BMY_LOG_EVENT_ACCOUNT:
			return "log_account_events";
		case BMY_LOG_EVENT_MAIL:
			return "log_mail_events";
		case BMY_LOG_EVENT_USER_INTERACTION:
			return "log_user_interaction_events";
		case BMY_LOG_EVENT_USER_QUERY:
			return "log_user_query_events";
		case BMY_LOG_EVENT_ANNOUNCEMENT:
			return "log_announcement_events";
		case BMY_LOG_EVENT_BOARD_DENY:
			return "log_board_deny_events";
	}

	return "";
}

static void dump_sql_error(PGconn *conn) {
	fprintf(stderr, "SQL failed: %s", PQerrorMessage(conn));
}
