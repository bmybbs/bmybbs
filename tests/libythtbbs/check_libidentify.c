#include <check.h>
#define IDLEN 12
#define STRLEN 80
#include "identify.h"

// test sample data:
// foo@xjtu.edu.cn: foo1 foo2 foo3 foo4
// bar@xjtu.edu.cn: bar1 bar2 bar3
// baz@xjtu.edu.cn: baz1
// null@xjtu.edu.cn: N/A

/*
insert into userreglog
	(userid, name, dept, ip, email, phone, idnum, studnum, operator)
values
	("foo1", "", "", "", "foo@xjtu.edu.cn", "", "", "", ""),
	("foo2", "", "", "", "foo@xjtu.edu.cn", "", "", "", ""),
	("foo3", "", "", "", "foo@xjtu.edu.cn", "", "", "", ""),
	("foo4", "", "", "", "foo@xjtu.edu.cn", "", "", "", ""),
	("bar1", "", "", "", "bar@xjtu.edu.cn", "", "", "", ""),
	("bar2", "", "", "", "bar@xjtu.edu.cn", "", "", "", ""),
	("bar3", "", "", "", "bar@xjtu.edu.cn", "", "", "", ""),
	("baz1", "", "", "", "baz@xjtu.edu.cn", "", "", "", "");
 */

START_TEST(test_associated_ids_with_email_not_exist) {
	ck_assert_ptr_null(get_associated_userid("null@xjtu.edu.cn"));
}

START_TEST(test_associate_ids_with_1_id) {
	struct associated_userid *au = get_associated_userid("baz@xjtu.edu.cn");
	ck_assert_ptr_nonnull(au);
	ck_assert_int_eq(au->count, 1);
	ck_assert_ptr_nonnull(au->id_array);
	ck_assert_str_eq(au->id_array[0], "baz1");
	free_associated_userid(au);
}

START_TEST(test_associate_ids_with_4_ids) {
	struct associated_userid *au = get_associated_userid("foo@xjtu.edu.cn");
	ck_assert_ptr_nonnull(au);
	ck_assert_int_eq(au->count, 4);
	ck_assert_ptr_nonnull(au->id_array);
	ck_assert_ptr_nonnull(au->id_array[0]);
	ck_assert_str_eq(au->id_array[0], "foo1");
	ck_assert_ptr_nonnull(au->id_array[1]);
	ck_assert_str_eq(au->id_array[1], "foo2");
	ck_assert_ptr_nonnull(au->id_array[2]);
	ck_assert_str_eq(au->id_array[2], "foo3");
	ck_assert_ptr_nonnull(au->id_array[3]);
	ck_assert_str_eq(au->id_array[3], "foo4");
	free_associated_userid(au);
}

END_TEST

Suite * test_suite_identify(void) {
	Suite *s = suite_create("libidentify");

	TCase *tc = tcase_create("check associated ids");
	tcase_add_test(tc, test_associate_ids_with_1_id);
	tcase_add_test(tc, test_associate_ids_with_4_ids);
	tcase_add_test(tc, test_associated_ids_with_email_not_exist);
	suite_add_tcase(s, tc);

	return s;
}