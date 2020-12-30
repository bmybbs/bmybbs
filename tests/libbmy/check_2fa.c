#include <check.h>
#include <stdlib.h>
#include "bmy/2fa.h"

START_TEST(test_2fa_check_code_should_get_str_and_del_record) {
	bmy_2fa_status status;
	const char *auth1 = "helloworld";
	char key[8], code[8], *auth2;

	status = bmy_2fa_create(key, sizeof(key));
	ck_assert_int_eq(status, BMY_2FA_SUCCESS);

	status = bmy_2fa_get_code(key, auth1, code, sizeof(code));
	ck_assert_int_eq(status, BMY_2FA_SUCCESS);
	ck_assert_int_eq(strlen(code), 6);

	auth2 = bmy_2fa_check_code(key, code);
	ck_assert_ptr_nonnull(auth2);
	ck_assert_str_eq(auth2, auth1);
	free(auth2);

	auth2 = bmy_2fa_check_code(key, code);
	ck_assert_ptr_null(auth2);
}

START_TEST(test_2fa_should_fail_after_5_attempts) {
	bmy_2fa_status status;
	char c;
	const char *auth1 = "helloworld";
	char key[8], code[8], *auth2;

	status = bmy_2fa_create(key, sizeof(key));
	ck_assert_int_eq(status, BMY_2FA_SUCCESS);

	status = bmy_2fa_get_code(key, auth1, code, sizeof(code));
	ck_assert_int_eq(status, BMY_2FA_SUCCESS);
	ck_assert_int_eq(strlen(code), 6);

	c = code[0];
	code[0] = '\0';

	// 1st
	auth2 = bmy_2fa_check_code(key, code);
	ck_assert_ptr_null(auth2);
	// 2nd
	auth2 = bmy_2fa_check_code(key, code);
	ck_assert_ptr_null(auth2);
	// 3rd
	auth2 = bmy_2fa_check_code(key, code);
	ck_assert_ptr_null(auth2);
	// 4th
	auth2 = bmy_2fa_check_code(key, code);
	ck_assert_ptr_null(auth2);
	// 5th
	auth2 = bmy_2fa_check_code(key, code);
	ck_assert_ptr_null(auth2);

	code[0] = c;
	auth2 = bmy_2fa_check_code(key, code);
	ck_assert_ptr_null(auth2);
}

END_TEST

Suite * test_suite_2fa(void) {
	Suite *s = suite_create("check 2FA");
	TCase *tc = tcase_create("check 2FA with normal and wrong conditions");
	tcase_add_test(tc, test_2fa_check_code_should_get_str_and_del_record);
	tcase_add_test(tc, test_2fa_should_fail_after_5_attempts);
	suite_add_tcase(s, tc);

	return s;
}

