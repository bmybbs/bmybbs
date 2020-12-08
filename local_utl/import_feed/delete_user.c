#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>
#include "ythtbbs/cache.h"
#include "db.h"
#include "common.h"

static int delete_user_callback(const struct ythtbbs_cache_User *user, int curr_idx, va_list ap) {
	MYSQL_STMT *stmt = va_arg(ap, MYSQL_STMT *);
	MYSQL_BIND params[2];

	int status, usernum;

	if (!is_valid_username(user->userid))
		return 0;

	memset(params, 0, sizeof(params));
	params[0].buffer_type = MYSQL_TYPE_LONG;
	params[0].buffer = (int []) { curr_idx + 1 };
	params[0].buffer_length = sizeof(int);
	params[1].buffer_type = MYSQL_TYPE_STRING;
	params[1].buffer = (void *) user->userid;
	params[1].buffer_length = strlen(user->userid);

	status = mysql_stmt_bind_param(stmt, params);
	if (status != 0) {
		fprintf(stderr, "cannot delete user %d - %s [bind stmt]\n", curr_idx, user->userid);
		mysql_error_stmt(stmt);
		return -1;
	}

	status = mysql_stmt_execute(stmt);
	if (status != 0) {
		fprintf(stderr, "cannot delete user %d - %s [exec stmt]\n", curr_idx, user->userid);
		mysql_error_stmt(stmt);
		return -1;
	}
	return 0;
}

int delete_user(void) {
	MYSQL *s;
	MYSQL_STMT *stmt = NULL;
	int mysql_status = 0;
	int result = 0;

	MYSQL_BIND params[2];
	fprintf(stdout, "sizeof MYSQL_BIND: %lu\n", sizeof(MYSQL_BIND));
	fprintf(stdout, "sizeof params: %lu\n", sizeof(params));

	ythtbbs_cache_utmp_resolve();
	ythtbbs_cache_UserTable_resolve();

	s = mysql_init(NULL);
	if (!my_connect_mysql(s)) {
		result = -1;
		goto END;
	}

	stmt = mysql_stmt_init(s);
	if (stmt) {
		result = -1;
		goto END;
	}

	const char *sql = "CALL procedure_delete_user(?, ?)";
	mysql_status = mysql_stmt_prepare(stmt, sql, strlen(sql));
	if (mysql_status != 0) {
		result = -1;
		goto END;
	}

	ythtbbs_cache_UserTable_foreach_v(delete_user_callback, stmt);

END:
	if (stmt) mysql_stmt_close(stmt);
	if (s) mysql_close(s);
	return result;
}

