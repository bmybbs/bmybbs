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

	readstrvalue(filename, "attempts", value, sizeof(value));
	ck_assert_str_eq(value, "0");
}

START_TEST(test_gen_captcha_for_user_regen_immediately_should_remain_the_same) {
	struct BMYCaptcha c1, c2;
	int rc;

	savestrvalue(filename, "create_time", "0");

	rc = gen_captcha_for_user("foo", &c1);
	ck_assert_int_eq(rc, CAPTCHA_OK);

	rc = gen_captcha_for_user("foo", &c2);
	ck_assert_int_eq(rc, CAPTCHA_OK);

	ck_assert_int_eq(c1.timestamp, c2.timestamp);
	ck_assert_int_eq(c1.create_time, c2.create_time);
	ck_assert_str_eq(c1.value, c2.value);
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

	readstrvalue(filename, "attempts", value, sizeof(value));
	ck_assert_str_eq(value, "0");
}

START_TEST(test_verify_captcha_for_user) {
	struct BMYCaptcha c;
	int rc;
	char value[32];

	savestrvalue(filename, "create_time", "0");
	rc = gen_captcha_for_user("foo", &c);
	ck_assert_int_eq(rc, CAPTCHA_OK);

	rc = verify_captcha_for_user("foo", c.value);
	ck_assert_int_eq(rc, CAPTCHA_OK);

	readstrvalue(filename, "used", value, sizeof(value));
	ck_assert_str_eq(value, "true");
}

START_TEST(test_verify_captcha_for_user_case_insensitive) {
	struct BMYCaptcha c;
	int rc;
	char value[32];

	savestrvalue(filename, "create_time", "0");
	rc = gen_captcha_for_user("foo", &c);
	ck_assert_int_eq(rc, CAPTCHA_OK);

	c.value[0] -= 32;
	rc = verify_captcha_for_user("foo", c.value);
	ck_assert_int_eq(rc, CAPTCHA_OK);

	readstrvalue(filename, "used", value, sizeof(value));
	ck_assert_str_eq(value, "true");
}

START_TEST(test_verify_captcha_for_user_case_wrong_attempts) {
	struct BMYCaptcha c;
	int rc;
	char value[32];

	savestrvalue(filename, "create_time", "0");
	rc = gen_captcha_for_user("foo", &c);
	ck_assert_int_eq(rc, CAPTCHA_OK);

	c.value[0] -= 31;
	rc = verify_captcha_for_user("foo", c.value);
	ck_assert_int_eq(rc, CAPTCHA_WRONG);
	readstrvalue(filename, "used", value, sizeof(value));
	ck_assert_str_eq(value, "false");

	rc = verify_captcha_for_user("foo", c.value);
	ck_assert_int_eq(rc, CAPTCHA_WRONG);
	readstrvalue(filename, "used", value, sizeof(value));
	ck_assert_str_eq(value, "false");

	rc = verify_captcha_for_user("foo", c.value);
	ck_assert_int_eq(rc, CAPTCHA_WRONG);
	readstrvalue(filename, "used", value, sizeof(value));
	ck_assert_str_eq(value, "false");

	rc = verify_captcha_for_user("foo", c.value);
	ck_assert_int_eq(rc, CAPTCHA_WRONG);
	readstrvalue(filename, "used", value, sizeof(value));
	ck_assert_str_eq(value, "false");

	rc = verify_captcha_for_user("foo", c.value);
	ck_assert_int_eq(rc, CAPTCHA_WRONG);
	readstrvalue(filename, "used", value, sizeof(value));
	ck_assert_str_eq(value, "true");
}

START_TEST(test_verify_captcha_for_user_not_exists_or_wrong_length) {
	int rc;

	rc = verify_captcha_for_user("bar", "xxxxx");
	ck_assert_int_ne(rc, CAPTCHA_OK);

	rc = verify_captcha_for_user("foo", "xxxxxx");
	ck_assert_int_ne(rc, CAPTCHA_OK);

	rc = verify_captcha_for_user("foo", "xxxx");
	ck_assert_int_ne(rc, CAPTCHA_OK);

}

START_TEST(test_gen_captcha_url) {
	char buf[80];
	long long timestamp = 1532939106925709978L;

	gen_captcha_url(buf, sizeof(buf), timestamp);
	ck_assert_str_eq(buf, "http://bbs.xjtu.edu.cn/captcha/99/1532939106925709978.gif");
}
END_TEST

Suite * test_suite_captcha(void) {
	Suite *s = suite_create("check captcha");
	TCase *tc = tcase_create("check query captcha by id");
	tcase_add_test(tc, test_query_captcha_by_id);
	suite_add_tcase(s, tc);

	tc = tcase_create("check generate captcha to user foo");
	tcase_add_test(tc, test_gen_captcha_for_user);
	tcase_add_test(tc, test_gen_captcha_for_user_regen_immediately_should_remain_the_same);
	tcase_add_test(tc, test_gen_captcha_for_user_fake_create_time_should_allow);
	suite_add_tcase(s, tc);

	tc = tcase_create("check verify captcha to user foo");
	tcase_add_test(tc, test_verify_captcha_for_user);
	tcase_add_test(tc, test_verify_captcha_for_user_case_insensitive);
	tcase_add_test(tc, test_verify_captcha_for_user_case_wrong_attempts);
	tcase_add_test(tc, test_verify_captcha_for_user_not_exists_or_wrong_length);
	suite_add_tcase(s, tc);

	tc = tcase_create("check captcha url");
	tcase_add_test(tc, test_gen_captcha_url);
	suite_add_tcase(s, tc);

	return s;
}