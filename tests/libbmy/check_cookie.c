#include <stdio.h>
#include <check.h>
#include "bmy/cookie.h"

START_TEST(test_bmy_cookie_gen_with_NULL_buf) {
	char buf[20];
	ck_assert_int_eq(bmy_cookie_gen(NULL, 0, NULL), -1);
	ck_assert_int_eq(bmy_cookie_gen(buf, 0, NULL), -1);
	ck_assert_int_eq(bmy_cookie_gen(NULL, sizeof(buf), NULL), -1);
}

START_TEST(test_bmy_cookie_gen_with_NULL_cookie) {
	char buf[60];
	ck_assert_int_eq(bmy_cookie_gen(buf, sizeof(buf), NULL), 0);
}

START_TEST(test_bmy_cookie_gen) {
	char buf[60];
	char *A = "A";
	char *B = "B";
	char *C = "C";
	char *E = "E";
	struct bmy_cookie cookie;

	cookie.userid = A;
	cookie.sessid = B;
	cookie.token  = C;
	cookie.extraparam = E;
	ck_assert_int_eq(bmy_cookie_gen(buf, sizeof(buf), &cookie), 7);
	ck_assert_str_eq(buf, "A-B-C-E");

	cookie.userid = NULL;
	ck_assert_int_eq(bmy_cookie_gen(buf, sizeof(buf), &cookie), 6);
	ck_assert_str_eq(buf, "-B-C-E");

	cookie.sessid = NULL;
	ck_assert_int_eq(bmy_cookie_gen(buf, sizeof(buf), &cookie), 5);
	ck_assert_str_eq(buf, "--C-E");

	cookie.token = NULL;
	ck_assert_int_eq(bmy_cookie_gen(buf, sizeof(buf), &cookie), 4);
	ck_assert_str_eq(buf, "---E");

	cookie.extraparam = NULL;
	ck_assert_int_eq(bmy_cookie_gen(buf, sizeof(buf), &cookie), 3);
	ck_assert_str_eq(buf, "---");
}

START_TEST(test_bmy_cookie_parse) {
	struct bmy_cookie cookie;
	char buf[60];

	sprintf(buf, "A-B-C-E");
	memset(&cookie, 0, sizeof(struct bmy_cookie));
	bmy_cookie_parse(buf, &cookie);
	ck_assert_str_eq(cookie.userid, "A");
	ck_assert_str_eq(cookie.sessid, "B");
	ck_assert_str_eq(cookie.token, "C");
	ck_assert_str_eq(cookie.extraparam, "E");

	sprintf(buf, "A--C-E");
	memset(&cookie, 0, sizeof(struct bmy_cookie));
	bmy_cookie_parse(buf, &cookie);
	ck_assert_str_eq(cookie.userid, "A");
	ck_assert_str_eq(cookie.sessid, "");
	ck_assert_str_eq(cookie.token, "C");
	ck_assert_str_eq(cookie.extraparam, "E");

	sprintf(buf, "--C-E");
	memset(&cookie, 0, sizeof(struct bmy_cookie));
	bmy_cookie_parse(buf, &cookie);
	ck_assert_str_eq(cookie.userid, "");
	ck_assert_str_eq(cookie.sessid, "");
	ck_assert_str_eq(cookie.token, "C");
	ck_assert_str_eq(cookie.extraparam, "E");

	sprintf(buf, "---");
	memset(&cookie, 0, sizeof(struct bmy_cookie));
	bmy_cookie_parse(buf, &cookie);
	ck_assert_str_eq(cookie.userid, "");
	ck_assert_str_eq(cookie.sessid, "");
	ck_assert_str_eq(cookie.token, "");
	ck_assert_str_eq(cookie.extraparam, "");

	sprintf(buf, "--");
	memset(&cookie, 0, sizeof(struct bmy_cookie));
	bmy_cookie_parse(buf, &cookie);
	ck_assert_str_eq(cookie.userid, "");
	ck_assert_str_eq(cookie.sessid, "");
	ck_assert_str_eq(cookie.token, "");
	ck_assert_ptr_null(cookie.extraparam);

	sprintf(buf, "--");
	memset(&cookie, 0, sizeof(struct bmy_cookie));
	bmy_cookie_parse(buf, &cookie);
	ck_assert_str_eq(cookie.userid, "");
	ck_assert_str_eq(cookie.sessid, "");
	ck_assert_ptr_null(cookie.token);
	ck_assert_ptr_null(cookie.extraparam);

	sprintf(buf, "-");
	memset(&cookie, 0, sizeof(struct bmy_cookie));
	bmy_cookie_parse(buf, &cookie);
	ck_assert_str_eq(cookie.userid, "");
	ck_assert_ptr_null(cookie.sessid);
	ck_assert_ptr_null(cookie.token);
	ck_assert_ptr_null(cookie.extraparam);

	sprintf(buf, "");
	memset(&cookie, 0, sizeof(struct bmy_cookie));
	bmy_cookie_parse(buf, &cookie);
	ck_assert_str_eq(cookie.userid, "");
	ck_assert_ptr_null(cookie.sessid);
	ck_assert_ptr_null(cookie.token);
	ck_assert_ptr_null(cookie.extraparam);
}

END_TEST

Suite * test_suite_cookie(void) {
	Suite *s = suite_create("check cookie");

	TCase *tc = tcase_create("check cookie gen");
	tcase_add_test(tc, test_bmy_cookie_gen_with_NULL_buf);
	tcase_add_test(tc, test_bmy_cookie_gen_with_NULL_cookie);
	tcase_add_test(tc, test_bmy_cookie_gen);
	suite_add_tcase(s, tc);

	tc = tcase_create("check cookie parse");
	tcase_add_test(tc, test_bmy_cookie_parse);
	suite_add_tcase(s, tc);

	return s;
}

