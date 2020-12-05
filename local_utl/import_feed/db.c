/**
 * 局部文件，便于批量导入数据
 * 基本是 libbmy/mysql_wrapper.c 的拆解
 */
#include <sys/file.h>
#include "config.h"
#include "ytht/fileop.h"
#include "db.h"

MYSQL * my_connect_mysql(MYSQL *s) {
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

