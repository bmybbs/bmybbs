#include <stdio.h>
#include <string.h>
#include "ythtbbs/cache.h"
#include "ythtbbs/misc.h"
#include "ythtbbs/user.h"
#include "bmy/board.h"
#include "common.h"
#include "db.h"

static int import_user_callback(const struct ythtbbs_cache_User *user, int curr_idx, va_list ap) {
	MYSQL_STMT **stmt = va_arg(ap, MYSQL_STMT **);
	MYSQL_STMT *stmt_insert_user = stmt[0];
	MYSQL_STMT *stmt_insert_subs = stmt[1];
	MYSQL_BIND params_user[2], params_subs[2];
	int status;
	int usernum, boardnum;
	size_t l;
	char boardname[80];
	char filename[200];
	FILE *fp;
	struct boardmem *b;

	if (!is_valid_username(user->userid))
		return 0;

	memset(params_user, 0, sizeof(params_user));
	memset(params_subs, 0, sizeof(params_subs));

	usernum = curr_idx + 1;
	params_user[0].buffer_type = MYSQL_TYPE_LONG;
	params_user[0].buffer = &usernum;
	params_user[0].buffer_length = sizeof(int);
	params_user[1].buffer_type = MYSQL_TYPE_STRING;
	params_user[1].buffer = (void *) user->userid; // discard const
	params_user[1].buffer_length = strlen(user->userid);

	params_subs[0].buffer_type = MYSQL_TYPE_LONG;
	params_subs[0].buffer = &usernum;
	params_subs[0].buffer_length = sizeof(int);
	params_subs[1].buffer_type = MYSQL_TYPE_LONG;
	params_subs[1].buffer = &boardnum;
	params_subs[1].buffer_length = sizeof(int);

	status = mysql_stmt_bind_param(stmt_insert_user, params_user);
	if (status != 0) {
		fprintf(stderr, "cannot insert user %d - %s [bind stmt]\n", curr_idx, user->userid);
		mysql_error_stmt(stmt_insert_user);
		return -1;
	}

	status = mysql_stmt_bind_param(stmt_insert_subs, params_subs);
	if (status != 0) {
		fprintf(stderr, "cannot insert subs %d - %s [bind stmt]\n", curr_idx, user->userid);
		mysql_error_stmt(stmt_insert_subs);
		return -1;
	}

	status = mysql_stmt_execute(stmt_insert_user);
	if (status != 0) {
		fprintf(stderr, "cannot insert user %d - %s [exec stmt]\n", curr_idx, user->userid);
		mysql_error_stmt(stmt_insert_user);
		return -1;
	}

	// goodboard
	sethomefile_s(filename, sizeof(filename), user->userid, ".goodbrd");
	fp = fopen(filename, "r");
	if (fp) {
		while (fgets(boardname, sizeof(boardname), fp) != NULL) {
			l = strlen(boardname);
			if (boardname[l - 1] == '\n')
				boardname[l - 1] = 0;

			if (bmy_board_is_system_board(boardname))
				continue;

			b = ythtbbs_cache_Board_get_board_by_name(boardname);
			if (b == NULL)
				continue;

			boardnum = ythtbbs_cache_Board_get_idx_by_ptr(b) + 1;
			status = mysql_stmt_execute(stmt_insert_subs);
			if (status != 0) {
				fprintf(stderr, "cannot insert subs %d - %s [exec stmt]\n", curr_idx, user->userid);
				mysql_error_stmt(stmt_insert_subs);
				return -1;
			}
		}
		fclose(fp);
	}

	fprintf(stdout, "imported user: %d - %s\n", curr_idx, user->userid);
	return 0;
}

int import_user(void) {
	MYSQL *s;
	MYSQL_STMT *stmt[2] = { NULL, NULL };
	int mysql_status = 0;
	int result = 0;
	int i;

	ythtbbs_cache_utmp_resolve();
	ythtbbs_cache_UserTable_resolve();
	ythtbbs_cache_Board_resolve();

	s = mysql_init(NULL);
	if (!my_connect_mysql(s)) {
		result = -1;
		goto END;
	}

	const char *sql[2] = {
		"CALL procedure_insert_user(?, ?)",
		"INSERT INTO `t_user_subscriptions`(`usernum`, `boardnum`) VALUE(?, ?)"
	};

	for (i = 0; i < 2; i++) {
		stmt[i] = mysql_stmt_init(s);
		if (!stmt[i]) {
			result = -1;
			goto END;
		}

		mysql_status = mysql_stmt_prepare(stmt[i], sql[i], strlen(sql[i]));
		if (mysql_status != 0) {
			result = -1;
			goto END;
		}
	}

	ythtbbs_cache_UserTable_foreach_v(import_user_callback, stmt);

END:
	if (stmt[0]) mysql_stmt_close(stmt[0]);
	if (stmt[1]) mysql_stmt_close(stmt[1]);
	if (s)    mysql_close(s);
	return result;
}

