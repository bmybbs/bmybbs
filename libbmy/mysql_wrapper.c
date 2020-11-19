#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include "config.h"
#include "ytht/fileop.h"
#include "bmy/mysql_wrapper.h"

#if MYSQL_VERSION_ID < 80000
typedef my_bool MYBOOL;
#else
typedef bool MYBOOL;
#endif

/**
 * 从配置文件读取 sql 的连接信息，该函数属于 mysql_real_connect 的封装。
 * 当配置文件存在且连接正常时，返回 MYSQL* 指针。
 * @param s
 * @return
 */
static MYSQL * my_connect_mysql(MYSQL *s)
{
	const char *MYSQL_CONFIG_FILE = MY_BBS_HOME "/etc/mysqlconfig";

	FILE *cfg_fp;
	int   cfg_fd;
	char  sql_user[16];
	char  sql_pass[16];
	char  sql_db[24];
	char  sql_port[8];
	char  sql_host[32];

	cfg_fp = fopen(MYSQL_CONFIG_FILE, "r");
	if (!cfg_fp)
		return NULL;

	cfg_fd = fileno(cfg_fp);
	flock(cfg_fd, LOCK_SH);

	readstrvalue_fp(cfg_fp, "SQL_USER", sql_user, sizeof(sql_user));
	readstrvalue_fp(cfg_fp, "SQL_PASS", sql_pass, sizeof(sql_pass));
	readstrvalue_fp(cfg_fp, "SQL_DB", sql_db, sizeof(sql_db));
	readstrvalue_fp(cfg_fp, "SQL_PORT", sql_port, sizeof(sql_port));
	readstrvalue_fp(cfg_fp, "SQL_HOST", sql_host, sizeof(sql_host));

	flock(cfg_fd, LOCK_UN);
	fclose(cfg_fp);
	return mysql_real_connect(s, sql_host, sql_user, sql_pass, sql_db, atoi(sql_port), NULL, CLIENT_IGNORE_SIGPIPE);
}

int execute_prep_stmt(const char* sqlbuf, MYSQL_BIND *params, MYSQL_BIND *result_cols, void *result_set, BMY_MYSQL_WRAPPER_CALLBACK callback) {
	MYSQL *s;
	MYSQL_STMT *stmt;
	int mysql_status;
	MYBOOL bind_status;
	int result;

	s = NULL;
	stmt = NULL;
	result = MYSQL_OK;

	s = mysql_init(NULL);
	mysql_options(s, MYSQL_SET_CHARSET_NAME, "gbk");
	if (!my_connect_mysql(s)) {
		result = MYSQL_CANNOT_CONNECT_TO_MYSQL;
		goto END;
	}

	stmt = mysql_stmt_init(s);
	if (!stmt) {
		goto END;
	}

	mysql_status = mysql_stmt_prepare(stmt, sqlbuf, strlen(sqlbuf));
	if (mysql_status != 0) {
		result = MYSQL_CANNOT_INIT_STMT;
		goto END;
	}

	if (params != NULL) {
		bind_status = mysql_stmt_bind_param(stmt, params);
		if (bind_status != 0) {
			result = MYSQL_CANNOT_BIND_PARAMS;
			goto END;
		}
	}

	if (result_cols != NULL) {
		bind_status = mysql_stmt_bind_result(stmt, result_cols);
		if (bind_status != 0) {
			result = MYSQL_CANNOT_BIND_RESULT_COLS;
			goto END;
		}
	}

	mysql_status = mysql_stmt_execute(stmt);
	if (mysql_status != 0) {
		result = MYSQL_CANNOT_EXEC_STMT;
		goto END;
	}

	mysql_status = mysql_stmt_store_result(stmt);
	if (mysql_status != 0) {
		result = MYSQL_CANNOT_STORE_RESULT;
		goto END;
	}

	if (callback != NULL) {
		callback(stmt, result_cols, result_set);
	}

END:
	if(stmt) mysql_stmt_close(stmt);
	if(s)    mysql_close(s);
	return   result;
}