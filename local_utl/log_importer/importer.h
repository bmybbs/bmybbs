#ifndef BMY_LOG_IMPORTER_IMPORTER_H
#define BMY_LOG_IMPORTER_IMPORTER_H

#include <stdbool.h>

struct bmy_log_importer_config {
	const char *date_arg;
	const char *home_dir;
	const char *source_file;
	const char *source_path;
	bool dry_run;
};

struct bmy_log_import_summary {
	unsigned long total_lines;
	unsigned long inserted;
	unsigned long already_imported;
	unsigned long discarded;
	unsigned long unrecognized;
	unsigned long failed;
};

/**
 * 解析 argv 并将结果存放于 config
 * @returns 解析失败返回 -1，成功返回 0
 */
int bmy_log_importer_parse_args(
	int argc,
	char **argv,
	struct bmy_log_importer_config *config);

/**
 * @brief 释放 source_file 和 source_path 并将结构体置 0
 */
void bmy_log_importer_config_cleanup(struct bmy_log_importer_config *config);

int bmy_log_importer_run(
	const struct bmy_log_importer_config *config,
	struct bmy_log_import_summary *summary);

#endif
