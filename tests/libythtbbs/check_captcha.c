#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <check.h>
#include <unistd.h>
#include "captcha.h"
#include "fileop.h"

extern int query_captcha_by_id(unsigned int cap_id, struct BMYCaptcha *captcha);
static const char *filename = "/home/bbs/home/F/foo/.captcha";

START_TEST(test_query_captcha_by_id) {
	struct BMYCaptcha c;

	memset(&c, 0, sizeof(struct BMYCaptcha));
	query_captcha_by_id(1, &c);
	ck_assert_str_eq(c.value, "byaju");
	ck_assert_int_eq(c.timestamp, 1532939104569638468L);

	query_captcha_by_id(101010, &c);
	ck_assert_str_eq(c.value, "jhaoy");
	ck_assert_int_eq(c.timestamp, 1532942635514895868L);

	query_captcha_by_id(444444, &c);
	ck_assert_str_eq(c.value, "spauv");
	ck_assert_int_eq(c.timestamp, 1532958578602374436L);

	query_captcha_by_id(500000, &c);
	ck_assert_str_eq(c.value, "nlhtx");
	ck_assert_int_eq(c.timestamp, 1532961651915124117L);
}

START_TEST(test_gen_captcha_for_user) {
	struct BMYCaptcha c;
	int rc;
	char value[32];
	long ct;
	long long ts;

	unlink(filename);
	rc = gen_captcha_for_user("foo", &c);
	ck_assert_int_eq(rc, CAPTCHA_OK);

	readstrvalue(filename, "captcha", value, sizeof(value));
	ck_assert_str_eq(value, c.value);

	readstrvalue(filename, "timestamp", value, sizeof(value));
	ts = atoll(value);
	ck_assert_int_eq(ts, c.timestamp);

	readstrvalue(filename, "create_time", value, sizeof(value));
	ct = atol(value);
	ck_assert_int_eq(ct, c.create_time);

	readstrvalue(filename, "used", value, sizeof(value));
	ck_assert_str_eq(value, "false");
}

START_TEST(test_gen_captcha_for_user_regen_immediately_should_reject) {
	struct BMYCaptcha c;
	int rc;

	rc = gen_captcha_for_user("foo", &c);
	ck_assert_int_eq(rc, CAPTCHA_NOT_ALLOW_TO_REGEN);
}

START_TEST(test_gen_captcha_for_user_fake_create_time_should_allow) {
	struct BMYCaptcha c;
	int rc;
	char value[32];
	long ct;
	long long ts;

	savestrvalue(filename, "create_time", "0");

	rc = gen_captcha_for_user("foo", &c);
	ck_assert_int_eq(rc, CAPTCHA_OK);

	readstrvalue(filename, "captcha", value, sizeof(value));
	ck_assert_str_eq(value, c.value);

	readstrvalue(filename, "timestamp", value, sizeof(value));
	ts = atoll(value);
	ck_assert_int_eq(ts, c.timestamp);

	readstrvalue(filename, "create_time", value, sizeof(value));
	ct = atol(value);
	ck_assert_int_eq(ct, c.create_time);

	readstrvalue(filename, "used", value, sizeof(value));
	ck_assert_str_eq(value, "false");
}
END_TEST

Suite * test_suite_captcha(void) {
	Suite *s = suite_create("check captcha");
	TCase *tc = tcase_create("check query captcha by id");
	tcase_add_test(tc, test_query_captcha_by_id);

	tc = tcase_create("check generate captcha to user foo");
	tcase_add_test(tc, test_gen_captcha_for_user);
	tcase_add_test(tc, test_gen_captcha_for_user_regen_immediately_should_reject);
	tcase_add_test(tc, test_gen_captcha_for_user_fake_create_time_should_allow);

	suite_add_tcase(s, tc);

	return s;
}