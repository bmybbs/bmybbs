#include <string.h>
#include "bmy/mysql_wrapper.h"
#include "config.h"

static void bmy_user_interal_call(const char *sql, int usernum, char *userid) {
	MYSQL_BIND params[2];
	memset(params, 0, sizeof(params));

	params[0].buffer_type = MYSQL_TYPE_LONG;
	params[0].buffer = &usernum;
	params[0].buffer_length = sizeof(int);

	params[1].buffer_type = MYSQL_TYPE_STRING;
	params[1].buffer = &userid;
	params[1].buffer_length = strlen(userid);

	if (params[1].buffer_length > IDLEN)
		params[1].buffer_length = IDLEN;

	execute_prep_stmt(sql, MYSQL_CHARSET_UTF8, params, NULL, NULL, NULL);
}

void bmy_user_create(int usernum, char *userid) {
	const char *sql = "CALL procedure_insert_user(?, ?)";

	bmy_user_interal_call(sql, usernum, userid);
}

void bmy_user_delete(int usernum, char *userid) {
	const char *sql = "CALL procedure_delete_user(?, ?)";

	bmy_user_interal_call(sql, usernum, userid);
}

void bmy_user_associate_openid(int usernum, char *openid) {
	const char *sql = "UPDATE `t_users` SET `openid`=? WHERE `usernum`=? AND `openid` IS NULL";
	MYSQL_BIND params[2];

	memset(params, 0, sizeof(params));
	params[0].buffer_type = MYSQL_TYPE_STRING;
	params[0].buffer = openid;
	params[0].buffer_length = strlen(openid);
	params[1].buffer_type = MYSQL_TYPE_LONG;
	params[1].buffer = &usernum;
	params[1].buffer_length = sizeof(int);

	execute_prep_stmt(sql, MYSQL_CHARSET_UTF8, params, NULL, NULL, NULL);
}

void bmy_user_dissociate_openid(int usernum) {
	const char *sql = "UPDATE `t_users` SET `openid`=NULL WHERE `usernum`=?";

	MYSQL_BIND params[1];
	memset(params, 0, sizeof(params));
	params[0].buffer_type = MYSQL_TYPE_LONG;
	params[0].buffer = &usernum;
	params[0].buffer_length = sizeof(int);

	execute_prep_stmt(sql, MYSQL_CHARSET_UTF8, params, NULL, NULL, NULL);
}

static void bmy_user_mysql_simple_callback(MYSQL_STMT *stmt, MYSQL_BIND *result_col, void *result_set) {
	if (mysql_stmt_num_rows(stmt) > 0) {
		mysql_stmt_fetch(stmt);
	}
}

bool bmy_user_has_openid(int usernum) {
	const char *sql = "SELECT openid FROM t_users where usernum=?";
	char openid[128];
	MYSQL_BIND params[1], results[1];

	memset(openid, 0, sizeof(openid));
	memset(params, 0, sizeof(params));
	memset(results, 0, sizeof(results));

	params[0].buffer_type = MYSQL_TYPE_LONG;
	params[0].buffer = &usernum;
	params[0].buffer_length = sizeof(int);
	results[0].buffer_type = MYSQL_TYPE_STRING;
	results[0].buffer = openid;
	results[0].buffer_length = sizeof(openid);

	execute_prep_stmt(sql, MYSQL_CHARSET_UTF8, params, results, NULL, bmy_user_mysql_simple_callback);

	return openid[0] != '\0';
}

int bmy_user_getusernum_by_openid(char *openid) {
	int usernum = 0;
	const char *sql = "SELECT `usernum` FROM `t_users` WHERE `openid` = ?";

	MYSQL_BIND params[1], results[1];
	memset(params, 0, sizeof(params));
	memset(results, 0, sizeof(results));

	params[0].buffer_type = MYSQL_TYPE_STRING;
	params[0].buffer = openid;
	params[0].buffer_length = strlen(openid);

	results[0].buffer_type = MYSQL_TYPE_LONG;
	results[0].buffer = &usernum;
	results[0].buffer_length = sizeof(int);

	execute_prep_stmt(sql, MYSQL_CHARSET_UTF8, params, results, NULL, bmy_user_mysql_simple_callback);

	return usernum;
}

