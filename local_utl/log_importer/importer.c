#include "importer.h"

#include "bmy/pg_wrapper.h"
#include "db.h"
#include "log_parser.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 解析 date 并简单判断月、日范围
 */
static bool bmy_log_importer_validate_date(const char *date);

/**
 * @brief 检查一个 char 是否是数字
 */
static bool bmy_log_importer_is_digit(char ch);

/**
 * 依据 date 生成日志名称，使用完成需要调用 free()
 */
static char *bmy_log_importer_make_source_file(const char *date);

/**
 * 依据 home_dir 和 source_file 生成 source path
 */
static char *bmy_log_importer_make_source_path(const char *home_dir, const char *source_file);

/**
 * 将日期和时间以字符串的形式写入缓冲区 buf （默认 UTC+8）
 */
static void bmy_log_importer_make_occurred_at(
	const char *date,
	const struct bmy_log_line_time *line_time,
	char *buf,
	size_t buf_size);

/**
 * Report a parser result without writing legacy raw text to the terminal.
 */
static void bmy_log_importer_report_parse_issue(
	const struct bmy_log_importer_config *config,
	unsigned long source_line,
	const char *status);

int bmy_log_importer_parse_args(
	int argc,
	char **argv,
	struct bmy_log_importer_config *config) {
	const char *date_arg = NULL;
	const char *home_dir;
	int i;

	if (config == NULL)
		return -1;

	memset(config, 0, sizeof(*config));

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--dry-run") == 0) {
			config->dry_run = true;
			continue;
		}
		if (strcmp(argv[i], "--fast-import") == 0) {
			config->fast_import = true;
			continue;
		}

		// NOTE: 只接受一个 date_arg
		if (date_arg != NULL)
			return -1;
		date_arg = argv[i];
	}

	if (!bmy_log_importer_validate_date(date_arg))
		return -1;
	if (config->dry_run && config->fast_import)
		return -1;

	home_dir = getenv("HOME");
	if (home_dir == NULL || home_dir[0] == '\0')
		return -1;

	config->date_arg = date_arg;
	config->home_dir = home_dir;

	// NOTE: to be freed
	if ((config->source_file = bmy_log_importer_make_source_file(date_arg)) == NULL) {
		return -1;
	}

	// NOTE: to be freed
	if ((config->source_path = bmy_log_importer_make_source_path(home_dir, config->source_file)) == NULL) {
		bmy_log_importer_config_cleanup(config);
		return -1;
	}

	return 0;
}

void bmy_log_importer_config_cleanup(struct bmy_log_importer_config *config) {
	if (config == NULL)
		return;

	free((void *)config->source_file);
	free((void *)config->source_path);
	memset(config, 0, sizeof(*config));
}

int bmy_log_importer_run(
	const struct bmy_log_importer_config *config,
	struct bmy_log_import_summary *summary) {
	FILE *fp;
	PGconn *conn = NULL;
	char *source_file_id = NULL;
	char *line = NULL;
	size_t line_cap = 0;
	ssize_t line_len;
	bool fast_file_mode = false;
	bool transaction_open = false;
	int rc = 0;

	if (config == NULL || summary == NULL)
		return -1;

	memset(summary, 0, sizeof(*summary));

	fp = fopen(config->source_path, "r");
	if (fp == NULL) {
		fprintf(stderr, "failed to open %s: %s\n", config->source_path, strerror(errno));
		return -1;
	}

	// NOTE: 当 dry_run 为 false 时建立数据库连接
	if (!config->dry_run) {
		bool source_file_exists = false;

		conn = bmy_log_importer_db_connect();
		if (conn == NULL) {
			fclose(fp);
			return -1;
		}

		if (config->fast_import) {
			int exists_rc = bmy_log_importer_source_file_exists(conn, config->source_file, &source_file_exists);
			if (exists_rc != 0) {
				PQfinish(conn);
				fclose(fp);
				return -1;
			}
		}

		fast_file_mode = config->fast_import && !source_file_exists;
		summary->fast_import_used = fast_file_mode;
		if (fast_file_mode) {
			if (!bmy_pg_begin(conn)) {
				fprintf(stderr, "failed to begin fast import transaction\n");
				PQfinish(conn);
				fclose(fp);
				return -1;
			}
			transaction_open = true;
		}

		if (!bmy_log_importer_ensure_source_file(conn, config->source_file, &source_file_id)) {
			if (transaction_open && !bmy_pg_rollback(conn)) {
				fprintf(stderr, "failed to rollback fast import transaction\n");
			}
			PQfinish(conn);
			fclose(fp);
			return -1;
		}
	}

	while ((line_len = getline(&line, &line_cap, fp)) != -1) {
		struct bmy_log_parse_result result = {0};
		char occurred_at[32];
		bool imported = false;

		(void)line_len;
		summary->total_lines++;

		if (!config->dry_run && !fast_file_mode) {
			int lookup_rc = bmy_log_importer_is_line_imported(
				conn, source_file_id, summary->total_lines, &imported);
			if (lookup_rc != 0) {
				summary->failed++;
				rc = -1;
				break;
			}
			if (imported) {
				summary->already_imported++;
				continue;
			}
		}

		if (!bmy_log_parse_line(line, &result)) {
			if (result.status == BMY_LOG_PARSE_UNRECOGNIZED) {
				summary->unrecognized++;
				bmy_log_importer_report_parse_issue(
					config, summary->total_lines, "unrecognized");
			} else if (result.status == BMY_LOG_PARSE_DISCARDED) {
				summary->discarded++;
			} else {
				summary->failed++;
				bmy_log_importer_report_parse_issue(
					config, summary->total_lines, "failed");
			}
			bmy_log_parse_result_cleanup(&result);
			continue;
		}

		switch (result.status) {
			case BMY_LOG_PARSE_ACCEPTED:
				bmy_log_importer_make_occurred_at(
					config->date_arg, &result.line_time,
					occurred_at, sizeof(occurred_at));
				if (config->dry_run) {
					summary->inserted++;
				} else {
					bool insert_ok;

					if (fast_file_mode) {
						insert_ok = bmy_log_importer_insert_event_in_transaction(
							conn, source_file_id, summary->total_lines,
							occurred_at, &result);
					} else {
						insert_ok = bmy_log_importer_insert_event(
							conn, source_file_id, summary->total_lines,
							occurred_at, &result);
					}

					if (insert_ok) {
						summary->inserted++;
					} else {
						fprintf(stderr, "failed to insert %s:%lu\n",
							config->source_file, summary->total_lines);
						summary->failed++;
						rc = -1;
					}
				}
				break;
			case BMY_LOG_PARSE_DISCARDED:
				summary->discarded++;
				break;
			case BMY_LOG_PARSE_UNRECOGNIZED:
				summary->unrecognized++;
				bmy_log_importer_report_parse_issue(
					config, summary->total_lines, "unrecognized");
				break;
			case BMY_LOG_PARSE_FAILED:
			case BMY_LOG_PARSE_UNSET:
				summary->failed++;
				bmy_log_importer_report_parse_issue(
					config, summary->total_lines, "failed");
				break;
		}

		bmy_log_parse_result_cleanup(&result);
		if (rc != 0)
			break;
	}

	free(line);

	if (ferror(fp)) {
		fprintf(stderr, "failed to read %s\n", config->source_path);
		rc = -1;
	}

	if (transaction_open) {
		if (rc == 0) {
			if (!bmy_pg_commit(conn)) {
				fprintf(stderr, "failed to commit fast import transaction\n");
				rc = -1;
			}
		} else if (!bmy_pg_rollback(conn)) {
			fprintf(stderr, "failed to rollback fast import transaction\n");
		}
	}

	free(source_file_id);
	if (conn != NULL)
		PQfinish(conn);
	fclose(fp);

	return rc;
}

static bool bmy_log_importer_validate_date(const char *date) {
	int month;
	int day;

	if (date == NULL || strlen(date) != 10)
		return false;
	if (!bmy_log_importer_is_digit(date[0]) ||
		!bmy_log_importer_is_digit(date[1]) ||
		!bmy_log_importer_is_digit(date[2]) ||
		!bmy_log_importer_is_digit(date[3]) ||
		date[4] != '-' ||
		!bmy_log_importer_is_digit(date[5]) ||
		!bmy_log_importer_is_digit(date[6]) ||
		date[7] != '-' ||
		!bmy_log_importer_is_digit(date[8]) ||
		!bmy_log_importer_is_digit(date[9]))
		return false;

	month = (date[5] - '0') * 10 + (date[6] - '0');
	day = (date[8] - '0') * 10 + (date[9] - '0');

	return month >= 1 && month <= 12 && day >= 1 && day <= 31;
}

static bool bmy_log_importer_is_digit(char ch) {
	return ch >= '0' && ch <= '9';
}

static char * bmy_log_importer_make_source_file(const char *date) {
	size_t len = strlen(date) + strlen(".log") + 1;
	char *source_file = malloc(len);

	if (source_file == NULL)
		return NULL;

	snprintf(source_file, len, "%s.log", date);
	return source_file;
}

static char * bmy_log_importer_make_source_path(const char *home_dir, const char *source_file) {
	size_t len = strlen(home_dir) + strlen("/newtrace/") + strlen(source_file) + 1;
	char *source_path = malloc(len);

	if (source_path == NULL)
		return NULL;

	snprintf(source_path, len, "%s/newtrace/%s", home_dir, source_file);
	return source_path;
}

static void bmy_log_importer_make_occurred_at(
	const char *date,
	const struct bmy_log_line_time *line_time,
	char *buf,
	size_t buf_size) {
	snprintf(buf, buf_size, "%s %02d:%02d:%02d+08",
		date, line_time->hour, line_time->minute, line_time->second);
}

static void bmy_log_importer_report_parse_issue(const struct bmy_log_importer_config *config, unsigned long source_line, const char *status) {
	fprintf(stderr, "%s:%lu: %s\n", config->source_file, source_line, status);
}
