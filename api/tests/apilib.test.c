#include <arpa/inet.h>
#include <check.h>
#include "ytht/fileop.h"
#include "bmy/convcode.h"
#include "../apilib.h"

#define DELIMITER "\xFF"

extern char *parse_article_js_internal(struct mmapfile *pmf, struct attach_link **attach_link_list, const char *bname, const char *fname);

START_TEST(parse_article_js_internal_plain_content) {
	char *s = "AUTHOR BOARD\n"
		"TITLE\n"
		"SITE\n"
		"\n"
		"foo\n"
		"--\n"
		"FROM";
	struct mmapfile mf = { .ptr = s, .size = strlen(s) };
	struct attach_link *root = NULL;

	char *result = parse_article_js_internal(&mf, &root, "", "");
	ck_assert_ptr_nonnull(result);
	ck_assert_ptr_null(root);
	ck_assert_str_eq(result, "foo\n");

	free(result);
}

START_TEST(parse_article_js_internal_plain_content_with_ansi) {
	char *s = "AUTHOR BOARD\n"
		"TITLE\n"
		"SITE\n"
		"\n"
		"\033[1;31mfoo\033[0m\n"
		"--\n"
		"FROM";
	struct mmapfile mf = { .ptr = s, .size = strlen(s) };
	struct attach_link *root = NULL;

	char *result = parse_article_js_internal(&mf, &root, "", "");
	ck_assert_ptr_nonnull(result);
	ck_assert_ptr_null(root);
	ck_assert_str_eq(result, "\033[1;31mfoo\033[0m\n");

	size_t result_utf8_size = 2 * strlen(result);
	char *result_utf8 = malloc(result_utf8_size);
	ck_assert_ptr_nonnull(result_utf8);
	g2u(result, strlen(result), result_utf8, result_utf8_size);
	ck_assert_str_eq(result_utf8, "\033[1;31mfoo\033[0m\n");

	free(result_utf8);
	free(result);
}

START_TEST(parse_article_js_internal_plain_content_with_binaryattach_txt) {
	char *s = "AUTHOR BOARD\n"
		"TITLE\n"
		"SITE\n"
		"\n"
		"foo\n"
		"beginbinaryattach 1.txt\n"
		"bar\n"
		"--\n"
		"FROM";
	struct mmapfile mf = { .ptr = s, .size = strlen(s) };
	struct attach_link *root = NULL;

	char *result = parse_article_js_internal(&mf, &root, "", "");
	ck_assert_ptr_nonnull(result);
	ck_assert_ptr_null(root);
	ck_assert_str_eq(result, "foo\nbeginbinaryattach 1.txt\nbar\n");

	free(result);
}

START_TEST(parse_article_js_internal_content_with_attach) {
	char *s = strdup("AUTHOR BOARD\n"
		"TITLE\n"
		"SITE\n"
		"\n"
		"foo\n"
		"beginbinaryattach 1.txt\n"
		DELIMITER
		"SIZE"
		"bar\n"
		"\n"
		"--\n"
		"FROM");
	ck_assert_ptr_nonnull(s);
	struct mmapfile mf = { .ptr = s, .size = strlen(s) };
	struct attach_link *root = NULL;

	// 更新附件
	char *delimiter = strchr(s, 0xFF);
	delimiter[0] = 0;
	unsigned int size_n = htonl(4 /* "bar\n" */);
	memcpy(delimiter + 1, &size_n, sizeof(unsigned int));
	ck_assert_int_eq(delimiter[1], 0);
	ck_assert_int_eq(delimiter[2], 0);
	ck_assert_int_eq(delimiter[3], 0);
	ck_assert_int_eq(delimiter[4], 4);

	char *result = parse_article_js_internal(&mf, &root, "b", "f");
	ck_assert_ptr_nonnull(result);
	ck_assert_ptr_nonnull(root);

	ck_assert_int_eq(root->size, 4);
	ck_assert_str_eq(root->name, "1.txt");
	char link[256];
	snprintf(link, sizeof(link), "/" SMAGIC "/bbscon/1.txt?B=b&F=f&attachpos=%zu&attachname=/1.txt", delimiter + 1 - s);
	ck_assert_str_eq(root->link, link);
	ck_assert_int_eq(root->signature[0], 'b');
	ck_assert_int_eq(root->signature[1], 'a');
	ck_assert_int_eq(root->signature[2], 'r');
	ck_assert_int_eq(root->signature[3], '\n');
	ck_assert_int_eq(root->signature[4], 0);
	ck_assert_ptr_null(root->next);

	ck_assert_str_eq(result, "foo\n#attach 1.txt\n\n");
	free(s);
	free(result);
	free_attach_link_list(root);
}

START_TEST(parse_article_js_internal_content_with_2_png_attaches) {
	unsigned int bin[] = {
		0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52
	};

	char *s = strdup("AUTHOR BOARD\n"
		"TITLE\n"
		"SITE\n"
		"\n"
		"foo\n"
		"beginbinaryattach 1.png\n"
		DELIMITER
		"SIZE"
		"0123456789ABCDEF\n"
		"beginbinaryattach 2.png\n"
		DELIMITER
		"SIZE"
		"0123456789ABCDEF\n"
		"\n"
		"--\n"
		"FROM");
	ck_assert_ptr_nonnull(s);

	struct mmapfile mf = { .ptr = s, .size = strlen(s) };
	struct attach_link *root = NULL;

	unsigned int pos1, pos2;

	char *delimiter = strrchr(s, 0xFF);
	delimiter[0] = 0;
	unsigned int size_n = htonl(16);
	memcpy(delimiter + 1, &size_n, sizeof(unsigned int));
	memcpy(delimiter + 5, bin, 16);
	pos2 = delimiter + 1 - s;

	delimiter = strchr(s, 0xFF);
	delimiter[0] = 0;
	memcpy(delimiter + 1, &size_n, sizeof(unsigned int));
	memcpy(delimiter + 5, bin, 16);
	pos1 = delimiter + 1 - s;

	char *result = parse_article_js_internal(&mf, &root, "b", "f");

	ck_assert_ptr_nonnull(result);
	ck_assert_ptr_nonnull(root);
	ck_assert_ptr_nonnull(root->next);
	ck_assert_ptr_null(root->next->next);

	struct attach_link *a_1st = root;
	struct attach_link *a_2nd = root->next;

	ck_assert_str_eq(a_1st->name, "1.png");
	ck_assert_str_eq(a_2nd->name, "2.png");

	char link[256];
	snprintf(link, sizeof(link), "/" SMAGIC "/bbscon/1.png?B=b&F=f&attachpos=%u&attachname=/1.png", pos1);
	ck_assert_str_eq(a_1st->link, link);
	snprintf(link, sizeof(link), "/" SMAGIC "/bbscon/2.png?B=b&F=f&attachpos=%u&attachname=/2.png", pos2);
	ck_assert_str_eq(a_2nd->link, link);

	ck_assert_int_eq(a_1st->size, 16);
	ck_assert_int_eq(a_2nd->size, 16);

	ck_assert_int_eq(memcmp(a_1st->signature, bin, BMY_SIGNATURE_LEN), 0);
	ck_assert_int_eq(memcmp(a_2nd->signature, bin, BMY_SIGNATURE_LEN), 0);

	ck_assert_str_eq(result, "foo\n#attach 1.png\n\n#attach 2.png\n\n\n");

	free(s);
	free(result);
	free_attach_link_list(root);
}

START_TEST(parse_article_js_internal_content_check_attach_linked_list) {
	unsigned int bin[] = {
		0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52
	};

	char *s = strdup("AUTHOR BOARD\n"
		"TITLE\n"
		"SITE\n"
		"\n"
		"foo\n"
		"beginbinaryattach 1.png\n"
		DELIMITER
		"SIZE"
		"0123456789ABCDEF\n"
		"beginbinaryattach 2.png\n"
		DELIMITER
		"SIZE"
		"0123456789ABCDEF\n"
		"beginbinaryattach 3.png\n"
		DELIMITER
		"SIZE"
		"0123456789ABCDEF\n"
		"\n"
		"--\n"
		"FROM");

	ck_assert_ptr_nonnull(s);

	struct mmapfile mf = { .ptr = s, .size = strlen(s) };
	struct attach_link *root = NULL;
	//unsigned int pos1, pos2, pos3;
	char *delimiter;
	unsigned int size_n = htonl(16);

	delimiter = strchr(s, 0xFF);
	delimiter[0] = 0;
	memcpy(delimiter + 1, &size_n, sizeof(unsigned int));
	memcpy(delimiter + 5, bin, 16);
	//pos1 = delimiter + 1 - s;

	delimiter = strchr(delimiter + 21, 0xFF);
	delimiter[0] = 0;
	memcpy(delimiter + 1, &size_n, sizeof(unsigned int));
	memcpy(delimiter + 5, bin, 16);
	//pos2 = delimiter + 1 - s;

	delimiter = strchr(delimiter + 21, 0xFF);
	delimiter[0] = 0;
	memcpy(delimiter + 1, &size_n, sizeof(unsigned int));
	memcpy(delimiter + 5, bin, 16);
	//pos3 = delimiter + 1 - s;

	char *result = parse_article_js_internal(&mf, &root, "b", "f");

	ck_assert_ptr_nonnull(result);
	ck_assert_ptr_nonnull(root);
	ck_assert_ptr_nonnull(root->next);
	ck_assert_ptr_nonnull(root->next->next);
	ck_assert_ptr_null(root->next->next->next);

	ck_assert_str_eq(root->name, "1.png");
	ck_assert_str_eq(root->next->name, "2.png");
	ck_assert_str_eq(root->next->next->name, "3.png");

	free(s);
	free(result);
	free_attach_link_list(root);
}

END_TEST

Suite *test_article_parser_js_internal(void) {
	Suite *s = suite_create("check article parse results");
	TCase *tc = tcase_create("");
	tcase_add_test(tc, parse_article_js_internal_plain_content);
	tcase_add_test(tc, parse_article_js_internal_plain_content_with_ansi);
	tcase_add_test(tc, parse_article_js_internal_plain_content_with_binaryattach_txt);
	tcase_add_test(tc, parse_article_js_internal_content_with_attach);
	tcase_add_test(tc, parse_article_js_internal_content_with_2_png_attaches);
	tcase_add_test(tc, parse_article_js_internal_content_check_attach_linked_list);
	suite_add_tcase(s, tc);

	return s;
}

