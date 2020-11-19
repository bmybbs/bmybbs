#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include "ythtbbs/notification.h"

START_TEST(send_post_notification_and_delete_it)
{
	// first time add and count
	add_post_notification("IronBlood", "SYSOP", "sysop", 1234, "回来吃饭");
	ck_assert(count_notification_num("IronBlood") == 1);
	ck_assert(is_post_in_notification("IronBlood", "sysop", 1234) == 1);
	ck_assert(is_post_in_notification("IronBlood", "sysop", 12) == 0);

	// delete fake post
	del_post_notification("IronBlood", "sysop", 123);
	ck_assert(count_notification_num("IronBlood") == 1);
	ck_assert(is_post_in_notification("IronBlood", "sysop", 1234) == 1);

	// delete real post
	del_post_notification("IronBlood", "sysop", 1234);
	ck_assert(count_notification_num("IronBlood") == 0);
	ck_assert(is_post_in_notification("IronBlood", "sysop", 1234) == 0);

	// re add
	add_post_notification("IronBlood", "SYSOP", "sysop", 1234, "回来吃饭");
	ck_assert(count_notification_num("IronBlood") == 1);
	ck_assert(is_post_in_notification("IronBlood", "sysop", 1234) == 1);
	ck_assert(is_post_in_notification("IronBlood", "sysop", 12) == 0);

	// re delete
	del_post_notification("IronBlood", "sysop", 1234);
	ck_assert(count_notification_num("IronBlood") == 0);
	ck_assert(is_post_in_notification("IronBlood", "sysop", 1234) == 0);
}
END_TEST

Suite * test_suite_notification(void) {
	Suite *s = suite_create("notification");

	TCase *tc_core = tcase_create("core");
	tcase_add_test(tc_core, send_post_notification_and_delete_it);
	suite_add_tcase(s, tc_core);

	return s;
}
