#include <stdio.h>
#include <string.h>
#include "bmy/logging.h"
#include "bmy/user.h"

// 在 libytht 中实现，转为非公开的接口
extern void newtrace(const char *s);

void bmy_log_login_success(const char *userid, const char *fromhost, enum ythtbbs_user_login_type login_type) {
	char buf[80];

	if (login_type == YTHTBBS_LOGIN_TELNET || login_type == YTHTBBS_LOGIN_SSH) {
		snprintf(buf, sizeof buf, "%s enter %s", userid, fromhost);
	} else {
		snprintf(buf, sizeof buf, "%s enter %s using %s", userid, fromhost, ythtbbs_user_get_login_type_str(login_type));
	}
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

void bmy_log_board_use(const char *userid, const char *board, long stay) {
	char buf[80];

	snprintf(buf, sizeof buf, "%s use %s %ld", userid, board, stay);
	newtrace(buf);
}

void bmy_log_search_result_count(const char *userid, int result_count) {
	char buf[80];

	snprintf(buf, sizeof buf, "%s bbsfind %d", userid, result_count);
	newtrace(buf);
}

void bmy_log_post_create(const char *userid, const char *board, const char *title) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s post %s %s", userid, board, title);
	newtrace(buf);
}

void bmy_log_post_check_1984(const char *userid, const char *board, const char *title) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s check1984 %s %s", userid, board, title);
	newtrace(buf);
}

void bmy_log_post_edit(const char *userid, const char *board, const char *owner, const char *title) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s edit %s %s %s", userid, board, owner, title);
	newtrace(buf);
}

void bmy_log_post_delete(const char *userid, const char *board, const char *owner, const char *title) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s del %s %s %s", userid, board, owner, title);
	newtrace(buf);
}

void bmy_log_post_restore(const char *userid, const char *board, const char *owner, const char *title) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s undel %s %s %s", userid, board, owner, title);
	newtrace(buf);
}

void bmy_log_post_crosspost(const char *userid, const char *board, const char *title) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s crosspost %s %s", userid, board, title);
	newtrace(buf);
}

void bmy_log_post_same_title(const char *userid, const char *board, const char *title) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s sametitle %s %s", userid, board, title);
	newtrace(buf);
}

void bmy_log_thread_view(const char *userid, const char *board) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s thread %s", userid, board);
	newtrace(buf);
}

void bmy_log_mail_send(const char *userid, const char *target_userid) {
	char buf[80];

	snprintf(buf, sizeof buf, "%s mail %s", userid, target_userid);
	newtrace(buf);
}

void bmy_log_netmail_send(const char *userid, const char *target_userid) {
	char buf[80];

	snprintf(buf, sizeof buf, "%s netmail %s", userid, target_userid);
	newtrace(buf);
}

void bmy_log_utility_mail_send(const char *sender, const char *target_userid) {
	bmy_log_mail_send(sender, target_userid);
}

void bmy_log_board_deny(const char *operator_userid, const char *board, const char *target_userid) {
	char buf[80];

	snprintf(buf, sizeof buf, "%s deny %s %s", operator_userid, board, target_userid);
	newtrace(buf);
}

void bmy_log_post_mark(const char *userid, const char *board, const char *owner, const char *title) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s mark %s %s %s", userid, board, owner, title);
	newtrace(buf);
}

void bmy_log_post_unmark(const char *userid, const char *board, const char *owner, const char *title) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s unmark %s %s %s", userid, board, owner, title);
	newtrace(buf);
}

void bmy_log_post_digest(const char *userid, const char *board, const char *owner, const char *title) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s digest %s %s %s", userid, board, owner, title);
	newtrace(buf);
}

void bmy_log_post_undigest(const char *userid, const char *board, const char *owner, const char *title) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s undigest %s %s %s", userid, board, owner, title);
	newtrace(buf);
}

void bmy_log_post_water(const char *userid, const char *board, const char *owner, const char *title) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s water %s %s %s", userid, board, owner, title);
	newtrace(buf);
}

void bmy_log_post_unwater(const char *userid, const char *board, const char *owner, const char *title) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s unwater %s %s %s", userid, board, owner, title);
	newtrace(buf);
}

void bmy_log_post_title_change(const char *userid, const char *board, const char *owner, const char *old_title, const char *new_title) {
	char buf[300];

	snprintf(buf, sizeof buf,
		"%s changetitle %s %s oldtitle:%s newtitle:%s",
		userid, board,
		owner, old_title, new_title);
	newtrace(buf);
}

void bmy_log_post_range_delete(const char *userid, const char *board, int from_id, int to_id) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s ranged %s %d %d", userid, board, from_id, to_id);
	newtrace(buf);
}

void bmy_log_mail_range_delete(const char *userid, int from_id, int to_id) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s rangedmail %d %d", userid, from_id, to_id);
	newtrace(buf);
}

void bmy_log_post_top(const char *userid, const char *board, const char *owner, const char *title) {
	char buf[256];

	// 置顶
	snprintf(buf, sizeof buf, "%s \xD6\xC3\xB6\xA5 %s %s %s", userid, board, owner, title);
	newtrace(buf);
}

void bmy_log_post_untop(const char *userid, const char *board, const char *owner, const char *title) {
	char buf[256];

	// 去掉置顶
	snprintf(buf, sizeof buf, "%s \xC8\xA5\xB5\xF4\xD6\xC3\xB6\xA5 %s %s %s", userid, board, owner, title);
	newtrace(buf);
}

void bmy_log_announce_action(const char *userid, const char *action, const char *board, const char *path) {
	char buf[256];

	snprintf(buf, 256, "%s %s %s %s", userid, action, board, path);
	newtrace(buf);
}

void bmy_log_announce_import(const char *userid, const char *board, const char *owner, const char *title) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s import %s %s %s", userid, board, owner, title);
	newtrace(buf);
}

void bmy_log_talk_request(const char *userid, const char *target_userid, const char *kind) {
	// NOTE: 事实上 kind 只会为 "talk"
	char buf[80];

	snprintf(buf, sizeof buf, "%s %s %s", userid, kind, target_userid);
	newtrace(buf);
}

void bmy_log_send_goodwish(const char *userid, const char *target_userid) {
	char buf[80];

	snprintf(buf, sizeof buf, "%s sendgoodwish %s", userid, target_userid);
	newtrace(buf);
}

void bmy_log_finddf(const char *userid, const char *target, int count) {
	char buf[256];

	snprintf(buf, sizeof buf, "%s finddf %s %d", userid, target, count);
	newtrace(buf);
}

void bmy_log_cache_reload(const char *cache_name, int count) {
	char buf[80];

	if (strcmp(cache_name, "bmonline") == 0) {
		newtrace("system reload bmonline");
	} else {
		snprintf(buf, sizeof buf, "system reload %s %d", cache_name, count);
		newtrace(buf);
	}
}

void bmy_log_system_reload(const char *resource_name, int count) {
	char buf[80];

	if (strcmp(resource_name, "movie") == 0) {
		snprintf(buf, sizeof buf, "system reload movie %d", count);
	} else {
		snprintf(buf, sizeof buf, "system reload %s", resource_name);
	}
	newtrace(buf);
}

void bmy_log_runtime_error(const char *message) {
	newtrace(message);
}

void bmy_log_search_trace(const char *userid, const char *board, const char *query) {
	char buf[80];

	snprintf(buf, sizeof buf, "%s full_search %s %s", userid, board, query);
	newtrace(buf);
}

void bmy_log_selection_trace(const char *userid, const char *board, int id1, int id2) {
	char buf[80];

	snprintf(buf, sizeof buf, "%s select %s %d %d", userid, board, id1, id2);
	newtrace(buf);
}
