#include <stdlib.h>
#include <check.h>

extern Suite * test_suite_timeop(void);

extern Suite * test_suite_mailsender(void);
extern Suite * test_suite_identify(void);
extern Suite * test_suite_captcha(void);
extern Suite * test_suite_session(void);

extern Suite * test_suite_iphash(void);
extern Suite * test_suite_redis(void);
extern Suite * test_suite_cookie(void);
extern Suite * test_suite_2fa(void);

Suite * main_suit(void) {
	Suite *s = suite_create("bmybbs test suite");
	return s;
}

int main (void)
{
	int number_failed;

	Suite *s = main_suit();
	SRunner *sr = srunner_create(s);

	srunner_add_suite(sr, test_suite_timeop());

	srunner_add_suite(sr, test_suite_mailsender());
	//srunner_add_suite(sr, test_suite_identify());
	//srunner_add_suite(sr, test_suite_captcha());

	srunner_add_suite(sr, test_suite_iphash());
	srunner_add_suite(sr, test_suite_redis());
	srunner_add_suite(sr, test_suite_cookie());
	srunner_add_suite(sr, test_suite_2fa());

	srunner_add_suite(sr, test_suite_session());

	srunner_set_log (sr, "bmybbs.test.log");
	srunner_set_xml (sr, "bmybbs.test.xml");
	srunner_run_all(sr, CK_VERBOSE);

	number_failed = srunner_ntests_failed(sr);

	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

