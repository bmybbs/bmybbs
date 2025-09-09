#include <stdbool.h>
#include <string.h>
#include "bmy/mysql_wrapper.h"
#include "bmy/convcode.h"

bool bmy_board_is_system_board(const char *boardname) {
	return (!strcasecmp(boardname, "newcomers")
			|| !strcasecmp(boardname, "millionairesrec")
			|| !strcasecmp(boardname, "sysopmail")
			|| !strcasecmp(boardname, "bbslists")
			|| !strcasecmp(boardname, "ProgramLog")
			|| !strcasecmp(boardname, "TopTen")
			|| !strcasecmp(boardname, "syssecurity"));
}

static void bmy_board_internal_call(const char *sql, int boardnum, char *name_en, char *name_zh_gbk, char *secstr) {
	char name_zh_utf8[48];
	MYSQL_BIND params[4];

	g2u(name_zh_gbk, strlen(name_zh_gbk), name_zh_utf8, sizeof(name_zh_utf8));

	memset(params, 0, sizeof(params));
	params[0].buffer_type = MYSQL_TYPE_LONG;
	params[0].buffer = &boardnum;
	params[0].buffer_length = sizeof(int);

	params[1].buffer_type = MYSQL_TYPE_STRING;
	params[1].buffer = name_en;
	params[1].buffer_length = strlen(name_en);

	params[2].buffer_type = MYSQL_TYPE_STRING;
	params[2].buffer = name_zh_utf8;
	params[2].buffer_length = strlen(name_zh_utf8);

	params[3].buffer_type = MYSQL_TYPE_STRING;
	params[3].buffer = secstr;
	params[3].buffer_length = sizeof(char);

	execute_prep_stmt(sql, MYSQL_CHARSET_UTF8, params, NULL, NULL, NULL);
}

void bmy_board_create(int boardnum, char *name_en, char *name_zh_gbk, char *secstr) {
	const char *sql = "CALL procedure_insert_board(?, ?, ?, ?)";

	bmy_board_internal_call(sql, boardnum, name_en, name_zh_gbk, secstr);
}

void bmy_board_rename(int boardnum, char *name_en, char *name_zh_gbk, char *secstr) {
	const char *sql = "CALL procedure_update_board(?, ?, ?, ?)";

	bmy_board_internal_call(sql, boardnum, name_en, name_zh_gbk, secstr);
}

void bmy_board_delete(int boardnum, char *name_en) {
	const char *sql = "CALL procedure_delete_board(?, ?)";
	MYSQL_BIND params[2];

	memset(params, 0, sizeof(params));
	params[0].buffer_type = MYSQL_TYPE_LONG;
	params[0].buffer = &boardnum;
	params[0].buffer_length = sizeof(int);
	params[1].buffer_type = MYSQL_TYPE_STRING;
	params[1].buffer = name_en;
	params[1].buffer_length = strlen(name_en);

	execute_prep_stmt(sql, MYSQL_CHARSET_UTF8, params, NULL, NULL, NULL);
}

