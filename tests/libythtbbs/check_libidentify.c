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
	(userid, name, dept, ip, email, phone, idnum, studnum, operator, status)
values
	("foo1", "", "", "", "foo@xjtu.edu.cn", "", "", "", "", 1),
	("foo2", "", "", "", "foo@xjtu.edu.cn", "", "", "", "", 2),
	("foo3", "", "", "", "foo@xjtu.edu.cn", "", "", "", "", 3),
	("foo4", "", "", "", "foo@xjtu.edu.cn", "", "", "", "", 4),
	("bar1", "", "", "", "bar@xjtu.edu.cn", "", "", "", "", 1),
	("bar2", "", "", "", "bar@xjtu.edu.cn", "", "", "", "", 2),
	("bar3", "", "", "", "bar@xjtu.edu.cn", "", "", "", "", 0),
	("baz1", "", "", "", "baz@xjtu.edu.cn", "", "", "", "", 0);
 */

START_TEST(test_associated_ids_with_email_not_exist) {
	ck_assert_ptr_null(get_associated_userid("null@xjtu.edu.cn"));
}

START_TEST(test_associate_ids_with_baz) {
	struct associated_userid *au = get_associated_userid("baz@xjtu.edu.cn");
	ck_assert_ptr_nonnull(au);
	ck_assert_int_eq(au->count, 1);
	ck_assert_ptr_nonnull(au->id_array);
	ck_assert_str_eq(au->id_array[0], "baz1");
	ck_assert_ptr_nonnull(au->status_array);
	ck_assert_int_eq(au->status_array[0], 0);
	free_associated_userid(au);
}

START_TEST(test_associate_ids_with_foo) {
	struct associated_userid *au = get_associated_userid("foo@xjtu.edu.cn");
	ck_assert_ptr_nonnull(au);
	ck_assert_int_eq(au->count, 4);
	ck_assert_ptr_nonnull(au->id_array);
	ck_assert_ptr_nonnull(au->status_array);

	ck_assert_ptr_nonnull(au->id_array[0]);
	ck_assert_str_eq(au->id_array[0], "foo1");
	ck_assert_int_eq(au->status_array[0], 1);

	ck_assert_ptr_nonnull(au->id_array[1]);
	ck_assert_str_eq(au->id_array[1], "foo2");
	ck_assert_int_eq(au->status_array[1], 2);

	ck_assert_ptr_nonnull(au->id_array[2]);
	ck_assert_str_eq(au->id_array[2], "foo3");
	ck_assert_int_eq(au->status_array[2], 3);

	ck_assert_ptr_nonnull(au->id_array[3]);
	ck_assert_str_eq(au->id_array[3], "foo4");
	ck_assert_int_eq(au->status_array[3], 4);

	free_associated_userid(au);
}

END_TEST

Suite * test_suite_identify(void) {
	Suite *s = suite_create("libidentify");

	TCase *tc = tcase_create("check associated ids");
	tcase_add_test(tc, test_associate_ids_with_baz);
	tcase_add_test(tc, test_associate_ids_with_foo);
	tcase_add_test(tc, test_associated_ids_with_email_not_exist);
	suite_add_tcase(s, tc);

	return s;
}