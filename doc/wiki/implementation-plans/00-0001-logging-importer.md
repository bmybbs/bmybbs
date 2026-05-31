# IP 00-0001: Logging Importer

> Status: seeded
> Module: logging system
> Related design: ../migration/logging-system/phase-2-importer.md
> Related use cases: ../use-cases/00-0001-import-one-daily-log.md

## Goal

Implement a single-day historical log importer that reads one legacy `newtrace` file and writes accepted events into PostgreSQL category tables.

## Scope

- Provide one command-line utility for importing one daily log file.
- Support dry-run parsing without database writes.
- Resolve the source file from a date argument.
- Read the source file line by line with `getline(3)`.
- Check source filename and line number import state before parsing each line in default idempotent mode.
- Use the parser component from [00-0002-logging-importer-parser.md](./00-0002-logging-importer-parser.md).
- Insert accepted events and their import-tracking rows atomically.
- Support constrained `--fast-import` for never-imported source files.
- Print a summary with stable counters.

## Non-Goals

- Do not batch-import multiple dates in one process.
- Do not write live production logs directly to PostgreSQL.
- Do not import discarded runtime-diagnostic APIs into business-event tables.
- Do not store skipped-line metadata in PostgreSQL.
- Do not change the legacy `newtrace` files.
- Do not implement parser internals in this plan.
- Do not use `mmapfile`/`mmap(3)` in the first implementation.
- Do not implement configurable timezone handling in the first implementation.

## Inputs And Outputs

Inputs:

- Required date argument: `YYYY-MM-DD`
- Fixed UTC+8 timezone handling for legacy logs
- `$HOME/newtrace/YYYY-MM-DD.log`
- PostgreSQL default local connection

Outputs:

- Rows in logging category tables defined by [../migration/logging-system/phase-2-database-design.md](../migration/logging-system/phase-2-database-design.md)
- Rows in `log_imported_lines`
- Human-readable summary printed to stdout or stderr
- Process exit code

Dry-run output:

- Human-readable summary and per-line location diagnostics for unrecognized or failed lines.
- No PostgreSQL connection is required.
- No `log_imported_lines` lookup is performed.
- No category-table rows or import-tracking rows are inserted.

## Files And Modules

Expected location:

- `local_utl/log_importer/`

Expected files:

- `local_utl/log_importer/CMakeLists.txt`
- `local_utl/log_importer/main.c`
- `local_utl/log_importer/importer.c`
- `local_utl/log_importer/importer.h`
- `local_utl/log_importer/db.c`
- `local_utl/log_importer/db.h`
- `include/bmy/pg_wrapper.h`
- `libbmy/pg_wrapper.c`
- parser files owned by [00-0002-logging-importer-parser.md](./00-0002-logging-importer-parser.md)

Ownership boundary:

- This plan owns CLI, file resolution, file reading, database writes, transaction handling, summary output, and exit behavior.
- `main.c` owns the executable entry point and user-facing exit code.
- `importer.c` owns command execution, file reading, counters, and parser integration.
- `db.c` owns PostgreSQL connection, idempotency lookup, event insertion, and import tracking insertion.
- The parser plan owns line classification and conversion from line text to event records.

Implementation language:

- Use C.
- Use libpq for PostgreSQL access.

## Command Shape

Supported commands:

```text
log_importer YYYY-MM-DD
log_importer --dry-run YYYY-MM-DD
log_importer --fast-import YYYY-MM-DD
```

Rules:

- `YYYY-MM-DD` is required.
- `--dry-run` is optional.
- `--fast-import` is optional and only affects non-dry-run import.
- `--dry-run` and `--fast-import` should not be combined.
- Legacy log timestamps are interpreted as UTC+8.
- The resolved log file is `$HOME/newtrace/YYYY-MM-DD.log`.
- Store only `YYYY-MM-DD.log` in `log_source_files.source_file`.

Database connection:

- Use libpq default connection behavior, such as `PQconnectdb("")`.
- Do not hardcode a database name in source code.
- The expected deployment uses a local Unix-domain socket, the current system username as the PostgreSQL user, and the default database name.

## Data Flow

1. Parse command-line arguments.
2. Validate the date argument.
3. Resolve the source filename and full source path.
4. Open the source file.
5. Initialize summary counters.
6. In non-dry-run mode, create or load the `log_source_files` row once and keep its id.
7. Read each physical line with `getline(3)` and increment the source line number.
8. Check `log_imported_lines` for the source-file id and line number.
9. If already imported, increment `already_imported` and continue.
10. Parse the line through the parser component.
11. If parser result is discarded, unrecognized, or failed, increment the matching counter; for unrecognized or failed results, report filename, line number, and status without raw text; then continue.
12. Reconstruct `occurred_at` from date, line time, and fixed UTC+8 timezone.
13. Insert the event row into the matching category table.
14. Insert the matching `log_imported_lines` row.
15. Commit the transaction for that imported event.
16. Print the final summary.

## Internal Interfaces

The implementation should use C and libpq.

Importer configuration:

```c
struct bmy_log_importer_config {
	const char *date_arg;        /* YYYY-MM-DD */
	const char *home_dir;
	const char *source_file;     /* YYYY-MM-DD.log */
	const char *source_path;     /* $HOME/newtrace/YYYY-MM-DD.log */
	bool dry_run;
	bool fast_import;
};
```

Summary counters:

```c
struct bmy_log_import_summary {
	unsigned long total_lines;
	unsigned long inserted;
	unsigned long already_imported;
	unsigned long discarded;
	unsigned long unrecognized;
	unsigned long failed;
	bool fast_import_used;
};
```

Main importer functions:

```c
int bmy_log_importer_parse_args(
	int argc,
	char **argv,
	struct bmy_log_importer_config *config);

void bmy_log_importer_config_cleanup(
	struct bmy_log_importer_config *config);

int bmy_log_importer_run(
	const struct bmy_log_importer_config *config,
	struct bmy_log_import_summary *summary);
```

Database boundary:

```c
PGconn *bmy_log_importer_db_connect(void);

int bmy_log_importer_source_file_exists(
	PGconn *conn,
	const char *source_file,
	bool *exists);

bool bmy_log_importer_ensure_source_file(
	PGconn *conn,
	const char *source_file,
	char **source_file_id);

int bmy_log_importer_is_line_imported(
	PGconn *conn,
	const char *source_file_id,
	unsigned long source_line,
	bool *imported);

bool bmy_log_importer_insert_event(
	PGconn *conn,
	const char *source_file_id,
	unsigned long source_line,
	const char *occurred_at,
	const struct bmy_log_parse_result *result);
```

Interface rule:

- `bmy_log_importer_is_line_imported` separates lookup failure from an imported/not-imported result through its return value and output parameter.
- `bmy_log_importer_source_file_exists` is used by `--fast-import` to decide whether the optimized path is allowed.
- `bmy_log_importer_ensure_source_file` should run once per non-dry-run import and return the reusable `log_source_files.id`.
- `bmy_log_importer_insert_event` owns the transaction that inserts both the category-table row and the `log_imported_lines` row.
- The importer shell should not inspect parser internals beyond status, line time, target table, and typed payload fields.
- The parser should not know about PostgreSQL or `log_imported_lines`.
- Dry-run mode should call the parser and update counters, but must not call database helpers.

## Transaction Strategy

- The category-table insert and `log_imported_lines` insert must be atomic.
- If either insert fails, neither row should remain committed.
- Already-imported, discarded, unrecognized, and parse-failed lines do not need database writes.
- Dry-run mode does not open a database transaction.

Default idempotent mode:

- Use one transaction per imported event line.
- This keeps failure isolation simple for historical imports.
- Use this mode when `--fast-import` is not set, or when the source file already exists in `log_source_files`.

Fast file mode:

- Use only when `--fast-import` is set and the source file does not exist in `log_source_files`.
- Start one transaction before creating the `log_source_files` row.
- Insert all accepted event rows and matching `log_imported_lines` rows inside that transaction.
- Commit once after the whole file succeeds.
- Roll back the whole file on the first database insertion failure.
- Do not use per-line already-imported checks in this mode, because the file is known to be new.

## Summary Counters

The importer should report at least:

- `total_lines`
- `inserted`
- `already_imported`
- `discarded`
- `unrecognized`
- `failed`
- `dry_run`
- `fast_import`
- `fast_import_used`

Optional counters:

- per-table inserted counts
- first failed line number
- first unrecognized line number

Summary format:

- Human-readable text is enough.
- Print final counters to standard output.
- Print each unrecognized or failed parser result to standard error as `source_file:source_line: status`.
- Do not print the raw legacy line by default because it may contain GBK text; use the reported location for editor inspection.
- JSON output is not needed for the first implementation.

## Implementation State

- CLI argument parsing, file resolution, dry-run behavior, source-file reading, counters, parser issue location reporting, idempotency lookup, category-table insertion, import tracking insertion, and per-event transactions are implemented.
- PostgreSQL access uses thin shared wrappers in `libbmy`; importer-specific SQL and error reporting remain in `local_utl/log_importer`.
- Build and runtime validation remain pending in the test environment.

## Error Handling

- Missing source file: report and exit non-zero without inserting rows.
- Invalid date argument: report and exit non-zero without reading a file.
- Database connection failure: report and exit non-zero.
- Dry-run mode: do not connect to PostgreSQL and do not check import state.
- Already imported line: count and continue.
- Discarded line: count and continue.
- Unrecognized line: count and continue.
- Decode or parse failure: count and continue unless the failure prevents safe file processing.
- Insert failure for one accepted line: roll back that line, report the failing source line, and stop the importer immediately with a non-zero exit code.

## Validation

- Parser behavior should be validated first through tests owned by [00-0002-logging-importer-parser.md](./00-0002-logging-importer-parser.md).
- Import a small sample file containing at least one accepted line.
- Confirm accepted events appear in category tables.
- Confirm `log_source_files.source_file` stores `YYYY-MM-DD.log`, not a full path.
- Confirm every inserted event has a matching `log_imported_lines` row.
- Run the importer twice for the same date and confirm no duplicate rows.
- Import a sample file containing discarded and unrecognized lines and confirm they are counted but not inserted.
- Run with a missing date file and confirm non-zero exit without inserts.
- Run with `--dry-run` and confirm no PostgreSQL rows are inserted.
- Run dry-run against old historical logs and collect unrecognized/failed examples for parser tests.

## Work Breakdown

- Implemented: utility structure, CMake integration, local parser test targets, command-line parsing, fixed UTC+8 source resolution, dry-run behavior, PostgreSQL connection setup, `getline(3)` file processing, idempotency lookup, parser integration, insertion dispatch, import tracking, transactions, summary output, and exit handling.
- Pending: build validation, historical dry-run examination, sample database import, and idempotency validation in the test environment.

## Backlog

- Add configurable timezone handling if logs from another timezone need to be imported later.
- Add parser test cases from dry-run discoveries in older historical logs.
