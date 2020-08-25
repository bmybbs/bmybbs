#include <check.h>
#include <stdio.h>

#define IDLEN 12
#define STRLEN 80
#include "identify.h"

struct active_data ad;

static void prep_ad() {
	snprintf(ad.userid, IDLEN+1, "foo5"),
	snprintf(ad.name, STRLEN, "foo5_name");
	snprintf(ad.dept, STRLEN, "foo5_dept");
	snprintf(ad.ip, 20, "localhost");
	snprintf(ad.operator, IDLEN+1, "foo5_op");
	snprintf(ad.email, VALUELEN, "foo5@xjtu.edu.cn");
	snprintf(ad.phone, VALUELEN, "foo5_phone");
	snprintf(ad.idnum, VALUELEN, "foo5_idnum");
	snprintf(ad.stdnum, VALUELEN, "foo5_studnum");
	ad.status = 1;
}

extern int execute_prep_stmt(const char* sqlbuf, MYSQL_BIND *params, MYSQL_BIND *result_cols, void *result_set, void (*callback)(MYSQL_STMT *, MYSQL_BIND *, void*));

static void clearup() {
	const char *sqlbuf = "delete from userreglog where id<>0 and userid=\"foo5\"";
	execute_prep_stmt(sqlbuf, NULL, NULL, NULL, NULL);
}

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

START_TEST(test_query_record_num) {
	ck_assert_int_eq(query_record_num("foo@xjtu.edu.cn", MAIL_ACTIVE), 4);
	ck_assert_int_eq(query_record_num("bar@xjtu.edu.cn", MAIL_ACTIVE), 2);
	ck_assert_int_eq(query_record_num("baz@xjtu.edu.cn", MAIL_ACTIVE), 0);
	ck_assert_int_eq(query_record_num("null@xjtu.edu.cn", MAIL_ACTIVE), 0);
}

START_TEST(test_read_active) {
	struct active_data ad1;
	int count;
	memset(&ad1, 0, sizeof(struct active_data));
	count = read_active("foo5", &ad1);
	ck_assert_int_eq(count, 1);
	ck_assert_str_eq(ad1.userid, ad.userid);
	ck_assert_str_eq(ad1.name, ad.name);
	ck_assert_str_eq(ad1.dept, ad.dept);
	ck_assert_str_eq(ad1.ip, ad.ip);
	ck_assert_str_eq(ad1.operator, ad.userid);
	ck_assert_str_eq(ad1.email, ad.email);
	ck_assert_str_eq(ad1.phone, ad.phone);
	ck_assert_str_eq(ad1.idnum, ad.idnum);
	ck_assert_str_eq(ad1.stdnum, ad.stdnum);
	ck_assert_int_eq(ad1.status, ad.status);
}

START_TEST(test_read_active_after_delete) {
	struct active_data ad1;
	int count;
	memset(&ad1, 0, sizeof(struct active_data));
	clearup();
	count = read_active("foo5", &ad1);
	ck_assert_int_eq(count, 0);
}

START_TEST(test_write_active) {
	struct active_data ad1;
	memcpy(&ad1, &ad, sizeof(struct active_data));

	ck_assert_int_eq(write_active(&ad1), WRITE_SUCCESS);

	snprintf(ad1.operator, IDLEN+1, "foo5");
	ck_assert_int_eq(write_active(&ad1), UPDATE_SUCCESS);
}

END_TEST

Suite * test_suite_identify(void) {
	prep_ad();

	Suite *s = suite_create("libidentify");

	TCase *tc = tcase_create("check associated ids");
	tcase_add_test(tc, test_associate_ids_with_baz);
	tcase_add_test(tc, test_associate_ids_with_foo);
	tcase_add_test(tc, test_associated_ids_with_email_not_exist);
	suite_add_tcase(s, tc);

	tc = tcase_create("check query_record_num");
	tcase_add_test(tc, test_query_record_num);
	suite_add_tcase(s, tc);

	tc = tcase_create("check write_active");
	tcase_add_test(tc, test_write_active);
	suite_add_tcase(s, tc);

	tc = tcase_create("check read_active");
	tcase_add_test(tc, test_read_active);
	tcase_add_test(tc, test_read_active_after_delete);
	suite_add_tcase(s, tc);
	return s;
}