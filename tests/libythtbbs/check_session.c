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

START_TEST(test_session_gen_session_id) {
	char buf[8];
	int i;
	ythtbbs_session_generate_id(buf, 8);

	ck_assert_int_eq(strlen(buf), 7);

	for (i = 0; i < 7; i++) {
		ck_assert((buf[i] >= 'A' && buf[i] <= 'Z')
			|| (buf[i] >= 'a' && buf[i] <= 'z')
			|| (buf[i] >= '0' && buf[i] <= '9'));
	}
}

END_TEST

Suite * test_suite_session(void) {
	Suite *s = suite_create("check session");
	TCase *tc = tcase_create("check session connection");
	tcase_add_test(tc, test_session_set_and_get);
	tcase_add_test(tc, test_session_gen_session_id);
	suite_add_tcase(s, tc);

	return s;
}

