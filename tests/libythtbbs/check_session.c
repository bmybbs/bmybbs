#include <check.h>
#include "ythtbbs/session.h"

START_TEST(test_session_set_and_get) {
	ck_assert_int_eq(ythtbbs_session_set("foo", "bar", 42), 0);
	ck_assert_int_eq(ythtbbs_session_get_utmp_idx("foo", "bar"), 42);
	ck_assert_int_eq(ythtbbs_session_get_utmp_idx("foo", "baz"), -1);
	ck_assert_int_eq(ythtbbs_session_get_utmp_idx("bar", "bar"), -1);
	ck_assert_int_eq(ythtbbs_session_del("foo"), 0);
	ck_assert_int_eq(ythtbbs_session_del("bar"), 0);
}

END_TEST

Suite * test_suite_session(void) {
	Suite *s = suite_create("check session");
	TCase *tc = tcase_create("check session connection");
	tcase_add_test(tc, test_session_set_and_get);
	suite_add_tcase(s, tc);

	return s;
}

