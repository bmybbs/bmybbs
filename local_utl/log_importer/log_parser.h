#ifndef BMY_LOG_IMPORTER_LOG_PARSER_H
#define BMY_LOG_IMPORTER_LOG_PARSER_H

#include <stdbool.h>

enum bmy_log_parse_status {
	// NOTE: DO NOT use this value
	BMY_LOG_PARSE_UNSET = 0,
	BMY_LOG_PARSE_ACCEPTED,
	BMY_LOG_PARSE_DISCARDED,
	BMY_LOG_PARSE_UNRECOGNIZED,
	BMY_LOG_PARSE_FAILED,
};

enum bmy_log_event_table {
	BMY_LOG_EVENT_ARTICLE,
	BMY_LOG_EVENT_RANGE_DELETE,
	BMY_LOG_EVENT_BOARD_USAGE,
	BMY_LOG_EVENT_SESSION_DURATION,
	BMY_LOG_EVENT_LOGIN_FAILURE,
	BMY_LOG_EVENT_SECURITY,
	BMY_LOG_EVENT_LOGIN_SUCCESS,
	BMY_LOG_EVENT_SESSION,
	BMY_LOG_EVENT_ACCOUNT,
	BMY_LOG_EVENT_MAIL,
	BMY_LOG_EVENT_USER_INTERACTION,
	BMY_LOG_EVENT_USER_QUERY,
	BMY_LOG_EVENT_ANNOUNCEMENT,
	BMY_LOG_EVENT_BOARD_DENY,
};

struct bmy_log_line_time {
	int hour;
	int minute;
	int second;
};

struct bmy_log_article_event {
	const char *actor_userid;
	const char *board;
	const char *owner_userid;
	const char *title;
	const char *old_title;
	const char *action;
};

struct bmy_log_range_delete_event {
	const char *scope;
	const char *userid;
	const char *board;
	int from_id;
	int to_id;
};

struct bmy_log_board_usage_event {
	const char *userid;
	const char *board;
	long stay_seconds;
};

struct bmy_log_session_duration_event {
	const char *userid;
	const char *action;
	long stay_seconds;
};

struct bmy_log_login_failure_event {
	const char *from_host;
};

struct bmy_log_security_event {
	const char *action;
	const char *input_value;
	const char *from_host;
};

struct bmy_log_login_success_event {
	const char *userid;
	const char *from_host;
	const char *login_type;
};

struct bmy_log_session_event {
	const char *action;
	const char *userid;
	const char *target_userid;
};

struct bmy_log_account_event {
	const char *action;
	const char *userid;
	int user_index_value;
	int life_value;
	const char *from_host;
	const char *login_type;
};

struct bmy_log_mail_event {
	const char *sender;
	const char *target_userid;
};

struct bmy_log_user_interaction_event {
	const char *action;
	const char *userid;
	const char *target_userid;
};

struct bmy_log_user_query_event {
	const char *action;
	const char *userid;
	const char *target;
	int day_count;
};

struct bmy_log_announcement_event {
	const char *action;
	const char *userid;
	const char *board;
	const char *path;
	const char *owner_userid;
	const char *title;
};

struct bmy_log_board_deny_event {
	const char *operator_userid;
	const char *board;
	const char *target_userid;
};

union bmy_log_event_payload {
	struct bmy_log_article_event article;
	struct bmy_log_range_delete_event range_delete;
	struct bmy_log_board_usage_event board_usage;
	struct bmy_log_session_duration_event session_duration;
	struct bmy_log_login_failure_event login_failure;
	struct bmy_log_security_event security;
	struct bmy_log_login_success_event login_success;
	struct bmy_log_session_event session;
	struct bmy_log_account_event account;
	struct bmy_log_mail_event mail;
	struct bmy_log_user_interaction_event user_interaction;
	struct bmy_log_user_query_event user_query;
	struct bmy_log_announcement_event announcement;
	struct bmy_log_board_deny_event board_deny;
};

struct bmy_log_parse_result {
	enum bmy_log_parse_status status;
	struct bmy_log_line_time line_time;
	enum bmy_log_event_table table;
	union bmy_log_event_payload payload;
	const char *reason;
};

bool bmy_log_parse_line(const char *line, struct bmy_log_parse_result *result);
void bmy_log_parse_result_cleanup(struct bmy_log_parse_result *result);

#endif
