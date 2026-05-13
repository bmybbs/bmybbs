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

static Suite *log_parser_suite(void) {
	Suite *s = suite_create("log importer parser");

	TCase *tc_core = tcase_create("core");
	tcase_add_test(tc_core, test_log_parser_null_string_should_fail);
	tcase_add_test(tc_core, test_log_parser_empty_string_should_fail);
	tcase_add_test(tc_core, test_log_parser_time);
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

	TCase *tc_session = tcase_create("session");
	tcase_add_test(tc_session, test_log_parser_user_enter_with_using);
	tcase_add_test(tc_session, test_log_parser_user_enter_without_using);
	tcase_add_test(tc_session, test_log_parser_user_session_cleanup);
	tcase_add_test(tc_session, test_log_parser_user_session_cleanup_legacy);
	tcase_add_test(tc_session, test_log_parser_user_multi_session_kick);
	tcase_add_test(tc_session, test_log_parser_user_kick);
	suite_add_tcase(s, tc_session);

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
