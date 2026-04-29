#include <stdio.h>
#include "bmy/logging.h"
#include "bmy/user.h"

// 在 libytht 中实现，转为非公开的接口
extern void newtrace(const char *s);

void bmy_log_login_success(const char *userid, const char *fromhost, enum ythtbbs_user_login_type login_type) {
	char buf[80];

	snprintf(buf, sizeof buf, "%s enter %s using %s", userid, fromhost, ythtbbs_user_get_login_type_str(login_type));
	newtrace(buf);
}

void bmy_log_login_failure(const char *fromhost) {
	char buf[80];

	snprintf(buf, sizeof buf, "system passerr %s", fromhost);
	newtrace(buf);
}

void bmy_log_logout(const char *userid, long stay) {
	char buf[80];

	snprintf(buf, sizeof buf, "%s exitbbs %ld", userid, stay);
	newtrace(buf);
}

void bmy_log_disconnect(const char *userid, long stay) {
	char buf[80];

	snprintf(buf, sizeof buf, "%s drop %ld", userid, stay);
	newtrace(buf);
}

void bmy_log_session_cleanup(const char *userid) {
	char buf[80];

	snprintf(buf, sizeof buf, "%s drop www/api", userid);
	newtrace(buf);
}

void bmy_log_multi_login_kick(const char *userid) {
	char buf[80];

	snprintf(buf, sizeof buf, "%s kick %s multi-login", userid, userid);
	newtrace(buf);
}

void bmy_log_user_kick(const char *operator_userid, const char *target_userid) {
	char buf[80];

	snprintf(buf, sizeof buf, "%s kick %s", operator_userid, target_userid);
	newtrace(buf);
}

void bmy_log_account_create(const char *userid, int usernum, const char *fromhost, enum ythtbbs_user_login_type session_type) {
	char buf[80];

	if (session_type == YTHTBBS_LOGIN_NJU09) {
		snprintf(buf, sizeof buf, "%s newaccount %d %s www", userid, usernum, fromhost);
	} else {
		snprintf(buf, sizeof buf, "%s newaccount %d %s", userid, usernum, fromhost);
	}

	newtrace(buf);
}

void bmy_log_account_expire_cleanup(const char *userid, int usernum) {
	char buf[80];

	snprintf(buf, sizeof buf, "system kill %s %d", userid, usernum);
	newtrace(buf);
}
