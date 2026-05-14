#include <check.h>
#include <stdlib.h>

#include "log_parser.h"

START_TEST(test_log_parser_null_string_should_fail)
{
	struct bmy_log_parse_result result;
	const char *log_msg = NULL;

	ck_assert(!bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_FAILED);
	ck_assert_str_eq(result.reason, "null line");

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_empty_string_should_fail)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "";

	ck_assert(!bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_FAILED);
	ck_assert_str_eq(result.reason, "wrong format");

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_time)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo enter 1.2.3.4";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.line_time.hour, 1);
	ck_assert_int_eq(result.line_time.minute, 2);
	ck_assert_int_eq(result.line_time.second, 3);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

#if 1 // article events
START_TEST(test_log_parser_article_mark_title_en)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo mark board owner title";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "mark");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "owner");
	ck_assert_str_eq(data->title, "title");
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_mark_title_en_multi_tokens)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo mark board owner title foo";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "mark");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "owner");
	ck_assert_str_eq(data->title, "title foo");
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_mark_title_zh)
{
	struct bmy_log_parse_result result;
	// "中文测试"
	const char *log_msg = "01:02:03 foo mark board owner \xD6\xD0\xCE\xC4\xB2\xE2\xCA\xD4";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "mark");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "owner");
	ck_assert_str_eq(data->title, "中文测试");
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_unmark_title_en)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo unmark board owner title";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "unmark");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "owner");
	ck_assert_str_eq(data->title, "title");
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_digest_title_en)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo digest board owner title";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "digest");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "owner");
	ck_assert_str_eq(data->title, "title");
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_undigest_title_en)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo undigest board owner title";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "undigest");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "owner");
	ck_assert_str_eq(data->title, "title");
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_water_title_en)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo water board owner title";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "water");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "owner");
	ck_assert_str_eq(data->title, "title");
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_unwater_title_en)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo unwater board owner title";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "unwater");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "owner");
	ck_assert_str_eq(data->title, "title");
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_top_title_en)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo \xD6\xC3\xB6\xA5 board owner title";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "top");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "owner");
	ck_assert_str_eq(data->title, "title");
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_untop_title_en)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo \xC8\xA5\xB5\xF4\xD6\xC3\xB6\xA5 board owner title";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "untop");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "owner");
	ck_assert_str_eq(data->title, "title");
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_edit)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo edit board owner title ";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "edit");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "owner");
	ck_assert_str_eq(data->title, "title ");
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_del)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo del board owner title ";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "del");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "owner");
	ck_assert_str_eq(data->title, "title ");
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_undel)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo undel board owner title ";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "undel");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "owner");
	ck_assert_str_eq(data->title, "title ");
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_post_en)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo post board en en";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "post");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->title, "en en");
	ck_assert_ptr_null(data->owner_userid);
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_post_zh)
{
	struct bmy_log_parse_result result;
	// "中文 en"
	const char *log_msg = "01:02:03 foo post board \xD6\xD0\xCE\xC4 en";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "post");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->title, "中文 en");
	ck_assert_ptr_null(data->owner_userid);
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_check1984)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo check1984 board en en";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "check1984");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->title, "en en");
	ck_assert_ptr_null(data->owner_userid);
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_crosspost)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo crosspost board en en";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "crosspost");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->title, "en en");
	ck_assert_ptr_null(data->owner_userid);
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_sametitle)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo sametitle board en en";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "sametitle");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->title, "en en");
	ck_assert_ptr_null(data->owner_userid);
	ck_assert_ptr_null(data->old_title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_changetitle_basic)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo changetitle board owner oldtitle:en newtitle:zh";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "changetitle");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "owner");
	ck_assert_str_eq(data->title, "zh");
	ck_assert_str_eq(data->old_title, "en");

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_changetitle_contain_newtitle)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo changetitle board owner oldtitle:newtitle:zh newtitle:zh newtitle:zh";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "changetitle");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "owner");
	ck_assert_str_eq(data->title, "zh newtitle:zh");
	ck_assert_str_eq(data->old_title, "newtitle:zh");

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_article_changetitle_contain_gbk)
{
	struct bmy_log_parse_result result;
	// "中文" "测试"
	const char *log_msg = "01:02:03 foo changetitle board owner oldtitle:\xD6\xD0\xCE\xC4 newtitle:\xB2\xE2\xCA\xD4";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);

	const struct bmy_log_article_event *data = &result.payload.article;

	ck_assert_str_eq(data->action, "changetitle");
	ck_assert_str_eq(data->actor_userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "owner");
	ck_assert_str_eq(data->title, "测试");
	ck_assert_str_eq(data->old_title, "中文");

	bmy_log_parse_result_cleanup(&result);
}
END_TEST
#endif

#if 1 // range delete
START_TEST(test_log_parser_range_delete)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo ranged bar 1 2";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_RANGE_DELETE);

	const struct bmy_log_range_delete_event *data = &result.payload.range_delete;
	ck_assert_str_eq(data->scope, "article");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_str_eq(data->board, "bar");
	ck_assert_int_eq(data->from_id, 1);
	ck_assert_int_eq(data->to_id, 2);

	bmy_log_parse_result_cleanup(&result);
}

START_TEST(test_log_parser_range_delete_mail)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo rangedmail 1 2";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_RANGE_DELETE);

	const struct bmy_log_range_delete_event *data = &result.payload.range_delete;
	ck_assert_str_eq(data->scope, "mail");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_int_eq(data->from_id, 1);
	ck_assert_int_eq(data->to_id, 2);
	ck_assert_ptr_null(data->board);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST
#endif

#if 1 // board usage
START_TEST(test_log_parser_board_usage)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo use bar 1234";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_BOARD_USAGE);

	const struct bmy_log_board_usage_event *data = &result.payload.board_usage;
	ck_assert_str_eq(data->board, "bar");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_int_eq(data->stay_seconds, 1234);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST
#endif

#if 1 // session duration
START_TEST(test_log_parser_session_exitbbs)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo exitbbs 111";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_SESSION_DURATION);

	const struct bmy_log_session_duration_event *data = &result.payload.session_duration;

	ck_assert_str_eq(data->userid, "foo");
	ck_assert_int_eq(data->stay_seconds, 111);
	ck_assert_str_eq(data->action, "logout");

	bmy_log_parse_result_cleanup(&result);
}

START_TEST(test_log_parser_session_drop)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo drop 111";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_SESSION_DURATION);

	const struct bmy_log_session_duration_event *data = &result.payload.session_duration;

	ck_assert_str_eq(data->userid, "foo");
	ck_assert_int_eq(data->stay_seconds, 111);
	ck_assert_str_eq(data->action, "disconnect");

	bmy_log_parse_result_cleanup(&result);
}
END_TEST
#endif

#if 1 // login failure
START_TEST(test_log_parser_login_failure)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 system passerr 1.2.3.4";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_LOGIN_FAILURE);

	ck_assert_str_eq(result.payload.login_failure.from_host, "1.2.3.4");
	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_login_failure_IPv6)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 system passerr 1:2:3::4";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_LOGIN_FAILURE);

	ck_assert_str_eq(result.payload.login_failure.from_host, "1:2:3::4");
	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_login_failure_wrong_username)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo passerr 1.2.3.4";

	ck_assert(!bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_UNRECOGNIZED);
	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_login_failure_wrong_IP)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 system passerr 1.2.3..4";

	ck_assert(!bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_UNRECOGNIZED);
	bmy_log_parse_result_cleanup(&result);
}
END_TEST
#endif

#if 1
START_TEST(test_log_parser_user_enter_with_using)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo enter 1.2.3.4 using TELNET";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_SESSION);

	const struct bmy_log_session_event *data = &result.payload.session;

	ck_assert_str_eq(data->action, "login_success");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_str_eq(data->from_host, "1.2.3.4");
	ck_assert_str_eq(data->login_type, "TELNET");
	ck_assert_ptr_null(data->target_userid);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_user_enter_without_using)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo enter 1.2.3.4";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_SESSION);

	const struct bmy_log_session_event *data = &result.payload.session;

	ck_assert_str_eq(data->action, "login_success");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_str_eq(data->from_host, "1.2.3.4");
	ck_assert_ptr_null(data->login_type);
	ck_assert_ptr_null(data->target_userid);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_user_session_cleanup)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo drop www/api";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_SESSION);

	const struct bmy_log_session_event *data = &result.payload.session;
	ck_assert_str_eq(data->action, "session_cleanup");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_ptr_null(data->target_userid);
	ck_assert_ptr_null(data->from_host);
	ck_assert_ptr_null(data->login_type);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_user_session_cleanup_legacy)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo drop www";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_SESSION);

	const struct bmy_log_session_event *data = &result.payload.session;
	ck_assert_str_eq(data->action, "session_cleanup");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_ptr_null(data->target_userid);
	ck_assert_ptr_null(data->from_host);
	ck_assert_ptr_null(data->login_type);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_user_multi_session_kick)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo kick foo multi-login";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_SESSION);

	const struct bmy_log_session_event *data = &result.payload.session;
	ck_assert_str_eq(data->action, "multi_login_kick");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_ptr_null(data->target_userid);
	ck_assert_ptr_null(data->from_host);
	ck_assert_ptr_null(data->login_type);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_user_kick)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo kick bar";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_SESSION);

	const struct bmy_log_session_event *data = &result.payload.session;
	ck_assert_str_eq(data->action, "user_kick");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_str_eq(data->target_userid, "bar");
	ck_assert_ptr_null(data->from_host);
	ck_assert_ptr_null(data->login_type);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST
#endif

#if 1 // account
START_TEST(test_log_parser_account_create)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo newaccount 1 1.2.3.4";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_ACCOUNT);

	const struct bmy_log_account_event *data = &result.payload.account;
	ck_assert_str_eq(data->action, "create");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_str_eq(data->from_host, "1.2.3.4");
	ck_assert_int_eq(data->usernum, 1);
	ck_assert_ptr_null(data->login_type);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_account_create_www)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo newaccount 1 1.2.3.4 www";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_ACCOUNT);

	const struct bmy_log_account_event *data = &result.payload.account;
	ck_assert_str_eq(data->action, "create");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_str_eq(data->from_host, "1.2.3.4");
	ck_assert_int_eq(data->usernum, 1);
	// TODO: not parsed yet
	ck_assert_ptr_null(data->login_type);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_account_expire_cleanup)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 system kill foo 1";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_ACCOUNT);

	const struct bmy_log_account_event *data = &result.payload.account;
	ck_assert_str_eq(data->action, "expire_cleanup");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_int_eq(data->usernum, 1);
	ck_assert_ptr_null(data->login_type);
	ck_assert_ptr_null(data->from_host);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST
#endif

#if 1 // mail
START_TEST(test_log_parser_mail)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo mail bar";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_MAIL);

	const struct bmy_log_mail_event *data = &result.payload.mail;
	ck_assert_str_eq(data->sender, "foo");
	ck_assert_str_eq(data->target_userid, "bar");

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_netmail)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo netmail bar@example.com";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_MAIL);

	const struct bmy_log_mail_event *data = &result.payload.mail;
	ck_assert_str_eq(data->sender, "foo");
	ck_assert_str_eq(data->target_userid, "bar@example.com");

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_mail_utility)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 XJTU-XANET mail bar";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_MAIL);

	const struct bmy_log_mail_event *data = &result.payload.mail;
	ck_assert_str_eq(data->sender, "XJTU-XANET");
	ck_assert_str_eq(data->target_userid, "bar");

	bmy_log_parse_result_cleanup(&result);
}
END_TEST
#endif

#if 1 // user interaction
START_TEST(test_log_parser_talk)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo talk bar";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_USER_INTERACTION);

	const struct bmy_log_user_interaction_event *data = &result.payload.user_interaction;
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_str_eq(data->target_userid, "bar");
	ck_assert_str_eq(data->action, "talk");

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_goodwish)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo sendgoodwish bar";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_USER_INTERACTION);

	const struct bmy_log_user_interaction_event *data = &result.payload.user_interaction;
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_str_eq(data->target_userid, "bar");
	ck_assert_str_eq(data->action, "goodwish");

	bmy_log_parse_result_cleanup(&result);
}
END_TEST
#endif

#if 1 // user query
START_TEST(test_log_parser_finddf)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo finddf bar 1";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_USER_QUERY);

	const struct bmy_log_user_query_event *data = &result.payload.user_query;
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_str_eq(data->target, "bar");
	ck_assert_str_eq(data->action, "finddf");
	ck_assert_int_eq(data->day_count, 1);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST
#endif

#if 1 // announcement
START_TEST(test_log_parser_announcement_import)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo import board bar title";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_ANNOUNCEMENT);

	const struct bmy_log_announcement_event *data = &result.payload.announcement;
	ck_assert_str_eq(data->action, "import");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "bar");
	ck_assert_str_eq(data->title, "title");
	ck_assert_ptr_null(data->path);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_announcement_import_title_zh)
{
	struct bmy_log_parse_result result;
	// "中文"
	const char *log_msg = "01:02:03 foo import board bar \xD6\xD0\xCE\xC4 1";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_ANNOUNCEMENT);

	const struct bmy_log_announcement_event *data = &result.payload.announcement;
	ck_assert_str_eq(data->action, "import");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->owner_userid, "bar");
	ck_assert_str_eq(data->title, "中文 1");
	ck_assert_ptr_null(data->path);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_announcement_paste)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo paste bar baz cat";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_ANNOUNCEMENT);


	const struct bmy_log_announcement_event *data = &result.payload.announcement;
	ck_assert_str_eq(data->action, "paste");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_str_eq(data->board, "bar");
	ck_assert_str_eq(data->path, "baz cat");
	ck_assert_ptr_null(data->owner_userid);
	ck_assert_ptr_null(data->title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_announcement_paste_path_zh)
{
	struct bmy_log_parse_result result;
	// "中文"
	const char *log_msg = "01:02:03 foo paste bar \xD6\xD0\xCE\xC4 cat";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_ANNOUNCEMENT);


	const struct bmy_log_announcement_event *data = &result.payload.announcement;
	ck_assert_str_eq(data->action, "paste");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_str_eq(data->board, "bar");
	ck_assert_str_eq(data->path, "中文 cat");
	ck_assert_ptr_null(data->owner_userid);
	ck_assert_ptr_null(data->title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_announcement_moveitem)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo moveitem bar baz";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_ANNOUNCEMENT);


	const struct bmy_log_announcement_event *data = &result.payload.announcement;
	ck_assert_str_eq(data->action, "moveitem");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_str_eq(data->board, "bar");
	ck_assert_str_eq(data->path, "baz");
	ck_assert_ptr_null(data->owner_userid);
	ck_assert_ptr_null(data->title);

	bmy_log_parse_result_cleanup(&result);
}

START_TEST(test_log_parser_announcement_additem)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo additem bar baz";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_ANNOUNCEMENT);


	const struct bmy_log_announcement_event *data = &result.payload.announcement;
	ck_assert_str_eq(data->action, "additem");
	ck_assert_str_eq(data->userid, "foo");
	ck_assert_str_eq(data->board, "bar");
	ck_assert_str_eq(data->path, "baz");
	ck_assert_ptr_null(data->owner_userid);
	ck_assert_ptr_null(data->title);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

#endif

#if 1 // board deny
START_TEST(test_log_parser_board_deny)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo deny board bar";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_ACCEPTED);
	ck_assert_int_eq(result.table, BMY_LOG_EVENT_BOARD_DENY);

	const struct bmy_log_board_deny_event *data = &result.payload.board_deny;
	ck_assert_str_eq(data->board, "board");
	ck_assert_str_eq(data->operator_userid, "foo");
	ck_assert_str_eq(data->target_userid, "bar");

	bmy_log_parse_result_cleanup(&result);
}
END_TEST
#endif

#if 1 // discarded
START_TEST(test_log_parser_discarded_exec)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo exec 1 2 3 4";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_mail_sender)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 [mail] foo send to bar smtp-status 1-2-3-4-5-6-7";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_insert_ut)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 failed to insert UT for foo";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_user_idx_changed)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 user_idx changed? foo: 1 --> 2";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_shm_err)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 SHM Error! key = 0x1111.";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_wechat_json_parse)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 [bmy/wechat] cannot parse response as JSON";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_wechat_session_error)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 [bmy/wechat] request session errcode[1] errmsg: foo";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_wechat_session_curl)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 [bmy/wechat] request session with curl result: 1";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_wechat_session_write_memory)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 [bmy/wechat] WriteMemoryCallback failed to realloc";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_search_stdout)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 [bmy/search] has problem of reading stdout";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_search_realloc)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 [bmy/search] cannot realloc";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_redis_set)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 [redis] SET foo and bar";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_2fa)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 generate 2fa failed for foo, status: 1";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_api_write_error)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 write error to fileheader foo, at No. 1 record, from file bar. Errno 1: baz.";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_api_lseek_error)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 lseek error on foo, at No. 1 record. Errno 1: bar.";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_search_result_count)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo bbsfind 10";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_reload_bmonline)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 system reload bmonline";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_reload_cache)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 system reload bcache 10";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_reload_movie)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 system reload movie 10";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_thread_view)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo thread bar";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_full_search)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo full_search board query abc";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST

START_TEST(test_log_parser_discarded_selection_trace)
{
	struct bmy_log_parse_result result;
	const char *log_msg = "01:02:03 foo select board 12 23";

	ck_assert(bmy_log_parse_line(log_msg, &result));
	ck_assert_int_eq(result.status, BMY_LOG_PARSE_DISCARDED);

	bmy_log_parse_result_cleanup(&result);
}
END_TEST
#endif

static Suite *log_parser_suite(void) {
	Suite *s = suite_create("log importer parser");

	TCase *tc_core = tcase_create("core");
	tcase_add_test(tc_core, test_log_parser_null_string_should_fail);
	tcase_add_test(tc_core, test_log_parser_empty_string_should_fail);
	tcase_add_test(tc_core, test_log_parser_time);

	tcase_add_test(tc_core, test_log_parser_board_usage);
	tcase_add_test(tc_core, test_log_parser_session_exitbbs);
	tcase_add_test(tc_core, test_log_parser_session_drop);

	tcase_add_test(tc_core, test_log_parser_account_create);
	tcase_add_test(tc_core, test_log_parser_account_create_www);
	tcase_add_test(tc_core, test_log_parser_account_expire_cleanup);

	tcase_add_test(tc_core, test_log_parser_mail);
	tcase_add_test(tc_core, test_log_parser_netmail);
	tcase_add_test(tc_core, test_log_parser_mail_utility);

	tcase_add_test(tc_core, test_log_parser_talk);
	tcase_add_test(tc_core, test_log_parser_goodwish);

	tcase_add_test(tc_core, test_log_parser_finddf);

	tcase_add_test(tc_core, test_log_parser_board_deny);
	suite_add_tcase(s, tc_core);

	TCase *tc_article = tcase_create("article");
	tcase_add_test(tc_article, test_log_parser_article_mark_title_en);
	tcase_add_test(tc_article, test_log_parser_article_mark_title_en_multi_tokens);
	tcase_add_test(tc_article, test_log_parser_article_mark_title_zh);
	tcase_add_test(tc_article, test_log_parser_article_unmark_title_en);
	tcase_add_test(tc_article, test_log_parser_article_digest_title_en);
	tcase_add_test(tc_article, test_log_parser_article_undigest_title_en);
	tcase_add_test(tc_article, test_log_parser_article_water_title_en);
	tcase_add_test(tc_article, test_log_parser_article_unwater_title_en);
	tcase_add_test(tc_article, test_log_parser_article_top_title_en);
	tcase_add_test(tc_article, test_log_parser_article_untop_title_en);
	tcase_add_test(tc_article, test_log_parser_article_edit);
	tcase_add_test(tc_article, test_log_parser_article_del);
	tcase_add_test(tc_article, test_log_parser_article_undel);
	tcase_add_test(tc_article, test_log_parser_article_post_en);
	tcase_add_test(tc_article, test_log_parser_article_post_zh);
	tcase_add_test(tc_article, test_log_parser_article_check1984);
	tcase_add_test(tc_article, test_log_parser_article_crosspost);
	tcase_add_test(tc_article, test_log_parser_article_sametitle);
	tcase_add_test(tc_article, test_log_parser_article_changetitle_basic);
	tcase_add_test(tc_article, test_log_parser_article_changetitle_contain_newtitle);
	tcase_add_test(tc_article, test_log_parser_article_changetitle_contain_gbk);
	suite_add_tcase(s, tc_article);

	TCase *tc_range_delete = tcase_create("range_delete");
	tcase_add_test(tc_range_delete, test_log_parser_range_delete);
	tcase_add_test(tc_range_delete, test_log_parser_range_delete_mail);
	suite_add_tcase(s, tc_range_delete);

	TCase *tc_login_failure = tcase_create("login failure");
	tcase_add_test(tc_login_failure, test_log_parser_login_failure);
	tcase_add_test(tc_login_failure, test_log_parser_login_failure_IPv6);
	tcase_add_test(tc_login_failure, test_log_parser_login_failure_wrong_username);
	tcase_add_test(tc_login_failure, test_log_parser_login_failure_wrong_IP);
	suite_add_tcase(s, tc_login_failure);

	TCase *tc_session = tcase_create("session");
	tcase_add_test(tc_session, test_log_parser_user_enter_with_using);
	tcase_add_test(tc_session, test_log_parser_user_enter_without_using);
	tcase_add_test(tc_session, test_log_parser_user_session_cleanup);
	tcase_add_test(tc_session, test_log_parser_user_session_cleanup_legacy);
	tcase_add_test(tc_session, test_log_parser_user_multi_session_kick);
	tcase_add_test(tc_session, test_log_parser_user_kick);
	suite_add_tcase(s, tc_session);

	TCase *tc_announcement = tcase_create("announcement");
	tcase_add_test(tc_announcement, test_log_parser_announcement_import);
	tcase_add_test(tc_announcement, test_log_parser_announcement_import_title_zh);
	tcase_add_test(tc_announcement, test_log_parser_announcement_paste);
	tcase_add_test(tc_announcement, test_log_parser_announcement_paste_path_zh);
	tcase_add_test(tc_announcement, test_log_parser_announcement_additem);
	tcase_add_test(tc_announcement, test_log_parser_announcement_moveitem);
	suite_add_tcase(s, tc_announcement);

	TCase *tc_discarded_runtime = tcase_create("discarded runtime");
	tcase_add_test(tc_discarded_runtime, test_log_parser_discarded_exec);
	tcase_add_test(tc_discarded_runtime, test_log_parser_discarded_mail_sender);
	tcase_add_test(tc_discarded_runtime, test_log_parser_discarded_insert_ut);
	tcase_add_test(tc_discarded_runtime, test_log_parser_discarded_user_idx_changed);
	tcase_add_test(tc_discarded_runtime, test_log_parser_discarded_shm_err);
	tcase_add_test(tc_discarded_runtime, test_log_parser_discarded_wechat_json_parse);
	tcase_add_test(tc_discarded_runtime, test_log_parser_discarded_wechat_session_error);
	tcase_add_test(tc_discarded_runtime, test_log_parser_discarded_wechat_session_curl);
	tcase_add_test(tc_discarded_runtime, test_log_parser_discarded_wechat_session_write_memory);
	tcase_add_test(tc_discarded_runtime, test_log_parser_discarded_search_stdout);
	tcase_add_test(tc_discarded_runtime, test_log_parser_discarded_search_realloc);
	tcase_add_test(tc_discarded_runtime, test_log_parser_discarded_redis_set);
	tcase_add_test(tc_discarded_runtime, test_log_parser_discarded_2fa);
	tcase_add_test(tc_discarded_runtime, test_log_parser_discarded_api_write_error);
	tcase_add_test(tc_discarded_runtime, test_log_parser_discarded_api_lseek_error);
	suite_add_tcase(s, tc_discarded_runtime);

	TCase *tc_discarded_rest = tcase_create("rest");
	tcase_add_test(tc_discarded_rest, test_log_parser_discarded_search_result_count);
	tcase_add_test(tc_discarded_rest, test_log_parser_discarded_reload_bmonline);
	tcase_add_test(tc_discarded_rest, test_log_parser_discarded_reload_cache);
	tcase_add_test(tc_discarded_rest, test_log_parser_discarded_reload_movie);
	tcase_add_test(tc_discarded_rest, test_log_parser_discarded_thread_view);
	tcase_add_test(tc_discarded_rest, test_log_parser_discarded_full_search);
	tcase_add_test(tc_discarded_rest, test_log_parser_discarded_selection_trace);
	suite_add_tcase(s, tc_discarded_rest);

	return s;
}

int main(void) {
	int number_failed;
	Suite *s = log_parser_suite();
	SRunner *sr = srunner_create(s);

	srunner_run_all(sr, CK_VERBOSE);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
