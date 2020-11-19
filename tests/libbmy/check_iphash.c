#include <check.h>
#include "bmy/iphash.h"

START_TEST(test_iphash) {
	const char * ip;
	const unsigned int MOD1 = 11, MOD2 = 13;

	ip = "2001:470:0:1:3439:431a:92dc:1062";
	ck_assert_int_eq(bmy_iphash(ip, MOD1), 5);
	ck_assert_int_eq(bmy_iphash(ip, MOD2), 10);

	ip = "2001:470:0:1:3439:431a:33dc:efef";
	ck_assert_int_eq(bmy_iphash(ip, MOD1), 10);
	ck_assert_int_eq(bmy_iphash(ip, MOD2), 9);

	ip = "d066:35d5:dcf4:dad5:5178:6e9e:9400:bf9f";

	ck_assert_int_eq(bmy_iphash(ip, MOD1), 10);
	ck_assert_int_eq(bmy_iphash(ip, MOD2), 2);

	ip = "192.168.1.1";
	ck_assert_int_eq(bmy_iphash(ip, MOD1), 6);
	ck_assert_int_eq(bmy_iphash(ip, MOD2), 5);

	ip = "192.168.2.1";
	ck_assert_int_eq(bmy_iphash(ip, MOD1), 4);
	ck_assert_int_eq(bmy_iphash(ip, MOD2), 8);
}

END_TEST

Suite * test_suite_iphash(void) {
	Suite *s = suite_create("check iphash");
	TCase *tc = tcase_create("check iphash with calculated results");
	tcase_add_test(tc, test_iphash);
	suite_add_tcase(s, tc);

	return s;
}