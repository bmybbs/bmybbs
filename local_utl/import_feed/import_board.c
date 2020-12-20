#include <stdio.h>
#include <string.h>
#include "bmy/convcode.h"
#include "bmy/board.h"
#include "ythtbbs/cache.h"
#include "ythtbbs/misc.h"
#include "common.h"
#include "db.h"

static int import_board_callback(struct boardmem *board, int curr_idx, va_list ap) {
	char boardname_zh[120];
	MYSQL_BIND params[4];
	int status;
	MYSQL_STMT *stmt = va_arg(ap, MYSQL_STMT *);

	if (bmy_board_is_system_board(board->header.filename))
		return 0;

	g2u(board->header.title, strlen(board->header.title), boardname_zh, sizeof(boardname_zh));

	memset(params, 0, sizeof(params));

	params[0].buffer_type = MYSQL_TYPE_LONG;
	params[0].buffer = (int []) { curr_idx + 1 }; // anonymous array as a pointer, boardnum = idx + 1;
	params[0].buffer_length = sizeof(int);

	params[1].buffer_type = MYSQL_TYPE_STRING;
	params[1].buffer = board->header.filename;
	params[1].buffer_length = strlen(board->header.filename);

	params[2].buffer_type = MYSQL_TYPE_STRING;
	params[2].buffer = boardname_zh;
	params[2].buffer_length = strlen(boardname_zh);

	params[3].buffer_type = MYSQL_TYPE_STRING;
	params[3].buffer = board->header.sec1;
	params[3].buffer_length = sizeof(char);

	status = mysql_stmt_bind_param(stmt, params);
	if (status != 0) {
		fprintf(stderr, "cannot import board %d - %s [bind stmt]\n", curr_idx, board->header.filename);
		mysql_error_stmt(stmt);
		return -1;
	}

	status = mysql_stmt_execute(stmt);
	if (status != 0) {
		fprintf(stderr, "cannot import board %d - %s [exec stmt]\n", curr_idx, board->header.filename);
		return -1;
	}

	fprintf(stdout, "imported %d - %s\n", curr_idx, board->header.filename);
	return 0;
}

int import_board(void) {
	MYSQL *s;
	MYSQL_STMT *stmt = NULL;
	int mysql_status = 0;
	MYBOOL bind_status;
	int result = 0;

	ythtbbs_cache_utmp_resolve();
	ythtbbs_cache_UserTable_resolve();
	ythtbbs_cache_Board_resolve();

	s = mysql_init(NULL);
	mysql_options(s, MYSQL_SET_CHARSET_NAME, "utf8");
	if (!my_connect_mysql(s)) {
		result = -1;
		goto END;
	}

	stmt = mysql_stmt_init(s);
	if (!stmt) {
		result = -1;
		goto END;
	}

	const char *sql = "CALL procedure_insert_board(?, ?, ?, ?)";
	mysql_status = mysql_stmt_prepare(stmt, sql, strlen(sql));
	if (mysql_status != 0) {
		result = -1;
		goto END;
	}

	ythtbbs_cache_Board_foreach_v(import_board_callback, stmt);
END:
	if (stmt) mysql_stmt_close(stmt);
	if (s)    mysql_close(s);
	return    result;
}

