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

