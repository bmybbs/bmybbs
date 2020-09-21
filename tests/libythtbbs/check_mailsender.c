#include <check.h>
#include "ythtbbs/mailsender.h"

extern enum mail_sender_code
check_mail_to_address(const char *mail_to);

START_TEST (test_email_parsing_xjtu_domain)
{
	ck_assert_int_eq(check_mail_to_address("aaa@stu.xjtu.edu.cn"), MAIL_SENDER_SUCCESS);
	ck_assert_int_eq(check_mail_to_address("bbb@xjtu.edu.cn"), MAIL_SENDER_SUCCESS);
	ck_assert_int_eq(check_mail_to_address("aaa@mail.xjtu.edu.cn"), MAIL_SENDER_SUCCESS);
}

START_TEST (test_email_no_name)
{
	ck_assert_int_eq(check_mail_to_address("@xjtu.edu.cn"), MAIL_SENDER_WRONG_EMAIL);
	ck_assert_int_eq(check_mail_to_address("@stu.xjtu.edu.cn"), MAIL_SENDER_WRONG_EMAIL);
	ck_assert_int_eq(check_mail_to_address("@mail.xjtu.edu.cn"), MAIL_SENDER_WRONG_EMAIL);
}

START_TEST (test_email_wrong_domain)
{
	ck_assert_int_eq(check_mail_to_address("aaa@xjtu1.edu.cn"), MAIL_SENDER_WRONG_EMAIL);
	ck_assert_int_eq(check_mail_to_address("bbb@stu1.xjtu.edu.cn"), MAIL_SENDER_WRONG_EMAIL);
	ck_assert_int_eq(check_mail_to_address("ccc@mail2.xjtu.edu.cn"), MAIL_SENDER_WRONG_EMAIL);
}

START_TEST (test_email_extream_condition)
{
	ck_assert_int_eq(check_mail_to_address("a@xjtu@xjtu.edu.cn"), MAIL_SENDER_WRONG_EMAIL);
}
END_TEST

Suite * test_suite_mailsender(void) {
	Suite *s = suite_create("mailsender");

	TCase *tc = tcase_create("mail address check");

	tcase_add_test(tc, test_email_parsing_xjtu_domain);
	tcase_add_test(tc, test_email_no_name);
	tcase_add_test(tc, test_email_wrong_domain);
	tcase_add_test(tc, test_email_extream_condition);

	suite_add_tcase(s, tc);

	return s;
}