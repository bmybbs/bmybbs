#ifndef BMY_LOG_IMPORTER_DB_H
#define BMY_LOG_IMPORTER_DB_H

#include <stdbool.h>

#include <libpq-fe.h>

#include "log_parser.h"

PGconn *bmy_log_importer_db_connect(void);

/**
 * @brief 确保来源文件存在于 log_source_files，并返回其 id
 *
 * @warning 成功时需要使用 free 释放 source_file_id
 */
bool bmy_log_importer_ensure_source_file(
	PGconn *conn,
	const char *source_file,
	char **source_file_id);

/**
 * @brief 判断某一行是否已经导入数据库，并将结果写入 imported
 * @returns 执行失败返回 -1，成功返回 0
 */
int bmy_log_importer_is_line_imported(
	PGconn *conn,
	const char *source_file_id,
	unsigned long source_line,
	bool *imported);

/**
 * @brief 将某一行日志解析的结果写入数据库
 *
 * 采用事物的方式同时写入导入记录
 */
bool bmy_log_importer_insert_event(
	PGconn *conn,
	const char *source_file_id,
	unsigned long source_line,
	const char *occurred_at,
	const struct bmy_log_parse_result *result);

#endif
