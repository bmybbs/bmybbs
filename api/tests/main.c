#include <stdlib.h>
#include <check.h>

extern Suite *test_article_parser_js_internal(void);

int main(void) {
	int number_failed;

	Suite *s = suite_create("bmybbs api test suite");
	SRunner *sr = srunner_create(s);

	srunner_add_suite(sr, test_article_parser_js_internal());

	srunner_set_log (sr, "bmybbs.api.test.log");
	srunner_set_xml (sr, "bmybbs.api.test.xml");
	srunner_run_all(sr, CK_VERBOSE);

	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

