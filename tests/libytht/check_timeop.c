#include <check.h>
#include "ytht/timeop.h"

START_TEST(test_ytht_utc_time_s) {
	char buf[100];
	time_t t0;
	size_t rc;

	t0 = 0;
	rc = ytht_utc_time_s(buf, sizeof(buf), &t0);
	ck_assert_uint_gt(rc, 0);
	ck_assert_str_eq(buf, "Thu, 01 Jan 1970 00:00:00 GMT");
}

END_TEST

Suite * test_suite_timeop(void) {
	Suite *s = suite_create("check timeop");
	TCase *tc = tcase_create("check utc time str generated by ytht_utc_time_s");
	tcase_add_test(tc, test_ytht_utc_time_s);
	suite_add_tcase(s, tc);

	return s;
}

