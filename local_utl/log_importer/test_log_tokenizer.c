#include <check.h>
#include <stdlib.h>

#include "log_tokenizer.h"

START_TEST(test_log_tokenizer_parse_less_than_max_tokens)
{
	struct bmy_log_tokens tokens;
	const char *s = "a bb ccc";

	ck_assert(bmy_log_tokenize(s, &tokens));
	ck_assert(bmy_log_token_eq(&tokens.items[0], "a"));
	ck_assert(bmy_log_token_eq(&tokens.items[1], "bb"));
	ck_assert(bmy_log_token_eq(&tokens.items[2], "ccc"));
	ck_assert_int_eq(tokens.count, 3);
	ck_assert(!tokens.truncated);
}
END_TEST

START_TEST(test_log_tokenizer_parse_max_tokens)
{
	struct bmy_log_tokens tokens;
	const char *s = "0 1 2 3 4 5 6 7 8 9 a b c d e f";

	ck_assert(bmy_log_tokenize(s, &tokens));
	ck_assert(bmy_log_token_eq(&tokens.items[0], "0"));
	ck_assert(bmy_log_token_eq(&tokens.items[1], "1"));
	ck_assert(bmy_log_token_eq(&tokens.items[2], "2"));
	ck_assert(bmy_log_token_eq(&tokens.items[3], "3"));
	ck_assert(bmy_log_token_eq(&tokens.items[4], "4"));
	ck_assert(bmy_log_token_eq(&tokens.items[5], "5"));
	ck_assert(bmy_log_token_eq(&tokens.items[6], "6"));
	ck_assert(bmy_log_token_eq(&tokens.items[7], "7"));
	ck_assert(bmy_log_token_eq(&tokens.items[8], "8"));
	ck_assert(bmy_log_token_eq(&tokens.items[9], "9"));
	ck_assert(bmy_log_token_eq(&tokens.items[10], "a"));
	ck_assert(bmy_log_token_eq(&tokens.items[11], "b"));
	ck_assert(bmy_log_token_eq(&tokens.items[12], "c"));
	ck_assert(bmy_log_token_eq(&tokens.items[13], "d"));
	ck_assert(bmy_log_token_eq(&tokens.items[14], "e"));
	ck_assert(bmy_log_token_eq(&tokens.items[15], "f"));
	ck_assert_int_eq(tokens.count, 16);
	ck_assert(!tokens.truncated);
}
END_TEST

START_TEST(test_log_tokenizer_parse_more_than_max_tokens)
{
	struct bmy_log_tokens tokens;
	const char *s = "0 1 2 3 4 5 6 7 8 9 a b c d e f 0";

	ck_assert(bmy_log_tokenize(s, &tokens));
	ck_assert(bmy_log_token_eq(&tokens.items[0], "0"));
	ck_assert(bmy_log_token_eq(&tokens.items[1], "1"));
	ck_assert(bmy_log_token_eq(&tokens.items[2], "2"));
	ck_assert(bmy_log_token_eq(&tokens.items[3], "3"));
	ck_assert(bmy_log_token_eq(&tokens.items[4], "4"));
	ck_assert(bmy_log_token_eq(&tokens.items[5], "5"));
	ck_assert(bmy_log_token_eq(&tokens.items[6], "6"));
	ck_assert(bmy_log_token_eq(&tokens.items[7], "7"));
	ck_assert(bmy_log_token_eq(&tokens.items[8], "8"));
	ck_assert(bmy_log_token_eq(&tokens.items[9], "9"));
	ck_assert(bmy_log_token_eq(&tokens.items[10], "a"));
	ck_assert(bmy_log_token_eq(&tokens.items[11], "b"));
	ck_assert(bmy_log_token_eq(&tokens.items[12], "c"));
	ck_assert(bmy_log_token_eq(&tokens.items[13], "d"));
	ck_assert(bmy_log_token_eq(&tokens.items[14], "e"));
	ck_assert(bmy_log_token_eq(&tokens.items[15], "f"));
	ck_assert_int_eq(tokens.count, 16);
	ck_assert(tokens.truncated);
}
END_TEST

START_TEST(test_log_tokenizer_dup)
{
	struct bmy_log_tokens tokens;
	const char *s = "0 1 2";
	char *t;

	ck_assert(bmy_log_tokenize(s, &tokens));
	ck_assert_int_eq(tokens.count, 3);
	t = bmy_log_token_dup(&tokens.items[0]);
	ck_assert_ptr_nonnull(t);
	ck_assert_str_eq(t, "0");
	free(t);
}
END_TEST

START_TEST(test_log_tokenizer_rest_after)
{
	struct bmy_log_tokens tokens;
	struct bmy_log_token rest;
	const char *s = "0 1 2";

	ck_assert(bmy_log_tokenize(s, &tokens));
	ck_assert_int_eq(tokens.count, 3);

	rest = bmy_log_token_rest_after(&tokens.items[0]);
	ck_assert(bmy_log_token_eq(&rest, "1 2"));
	rest = bmy_log_token_rest_after(&tokens.items[1]);
	ck_assert(bmy_log_token_eq(&rest, "2"));
	rest = bmy_log_token_rest_after(&tokens.items[2]);
	ck_assert(bmy_log_token_empty(&rest));
}
END_TEST

START_TEST(test_log_tokenizer_rest_after_with_trailing_spaces)
{
	struct bmy_log_tokens tokens;
	struct bmy_log_token rest;
	const char *s = "0 1 2 ";

	ck_assert(bmy_log_tokenize(s, &tokens));
	ck_assert_int_eq(tokens.count, 3);

	ck_assert(bmy_log_token_eq(&tokens.items[0], "0"));
	ck_assert(bmy_log_token_eq(&tokens.items[1], "1"));
	ck_assert(bmy_log_token_eq(&tokens.items[2], "2"));

	rest = bmy_log_token_rest_after(&tokens.items[0]);
	ck_assert(bmy_log_token_eq(&rest, "1 2 "));
	rest = bmy_log_token_rest_after(&tokens.items[1]);
	ck_assert(bmy_log_token_eq(&rest, "2 "));
	rest = bmy_log_token_rest_after(&tokens.items[2]);
	ck_assert(bmy_log_token_empty(&rest));
}
END_TEST

START_TEST(test_log_tokenizer_rest_after_with_manual_token)
{
	struct bmy_log_token token;
	const char *s = "0123";

	token.ptr = s;
	token.len = 1;

	token = bmy_log_token_rest_after(&token);
	ck_assert(bmy_log_token_eq(&token, "123"));
	ck_assert_int_eq(token.len, 3);
}
END_TEST

START_TEST(test_log_token_starts_with)
{
	struct bmy_log_token token;
	const char *s = "0123";

	token.ptr = s;
	token.len = 1;

	ck_assert(bmy_log_token_starts_with(&token, "0"));
	ck_assert(!bmy_log_token_starts_with(&token, "01"));
	ck_assert(!bmy_log_token_starts_with(&token, "00"));

	token.len = 2;

	ck_assert(bmy_log_token_starts_with(&token, "0"));
	ck_assert(bmy_log_token_starts_with(&token, "01"));
	ck_assert(!bmy_log_token_starts_with(&token, "00"));
}
END_TEST

static Suite *log_tokenizer_suite(void) {
	Suite *s = suite_create("log importer tokenizer");

	TCase *tc_core = tcase_create("core");
	tcase_add_test(tc_core, test_log_tokenizer_parse_less_than_max_tokens);
	tcase_add_test(tc_core, test_log_tokenizer_parse_max_tokens);
	tcase_add_test(tc_core, test_log_tokenizer_parse_more_than_max_tokens);
	tcase_add_test(tc_core, test_log_tokenizer_dup);
	tcase_add_test(tc_core, test_log_tokenizer_rest_after);
	tcase_add_test(tc_core, test_log_tokenizer_rest_after_with_trailing_spaces);
	tcase_add_test(tc_core, test_log_tokenizer_rest_after_with_manual_token);
	tcase_add_test(tc_core, test_log_token_starts_with);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(void) {
	int number_failed;
	Suite *s = log_tokenizer_suite();
	SRunner *sr = srunner_create(s);

	srunner_run_all(sr, CK_VERBOSE);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
