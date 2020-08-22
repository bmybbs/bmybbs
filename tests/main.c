#include <stdlib.h>
#include <check.h>

extern Suite * test_suite_mailsender(void);
extern Suite * test_suite_identify(void);

Suite * main_suit(void) {
	Suite *s = suite_create("bmybbs test suite");
	return s;
}

int main (void)
{
	int number_failed;

	Suite *s = main_suit();
	SRunner *sr = srunner_create(s);
	srunner_add_suite(sr, test_suite_mailsender());
	srunner_add_suite(sr, test_suite_identify());

	srunner_set_log (sr, "bmybbs.test.log");
	srunner_set_xml (sr, "bmybbs.test.xml");
	srunner_run_all(sr, CK_VERBOSE);

	number_failed = srunner_ntests_failed(sr);

	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
