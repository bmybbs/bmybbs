# IP 00-0001: Logging Importer

> Status: seeded
> Module: logging system
> Related design: ../migration/logging-system/phase-2-importer.md
> Related use cases: ../use-cases/00-0001-import-one-daily-log.md

## Goal

Implement a single-day historical log importer that reads one legacy `newtrace` file and writes accepted events into PostgreSQL category tables.

## Scope

- Provide one command-line utility for importing one daily log file.
- Resolve the source file from a date argument.
- Read the source file line by line with `getline(3)`.
- Check `log_imported_lines(source_file, source_line)` before parsing each line.
- Use the parser component from [00-0002-logging-importer-parser.md](./00-0002-logging-importer-parser.md).
- Insert accepted events and their import-tracking rows in one transaction.
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

Initial command:

```text
log_importer YYYY-MM-DD
```

Rules:

- `YYYY-MM-DD` is required.
- Legacy log timestamps are interpreted as UTC+8.
- The resolved log file is `$HOME/newtrace/YYYY-MM-DD.log`.
- Store only `YYYY-MM-DD.log` in `log_imported_lines.source_file`.

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
6. Read each physical line with `getline(3)` and increment the source line number.
7. Check `log_imported_lines(source_file, source_line)`.
8. If already imported, increment `already_imported` and continue.
9. Parse the line through the parser component.
10. If parser result is discarded, unrecognized, or failed, increment the matching counter and continue.
11. Reconstruct `occurred_at` from date, line time, and fixed UTC+8 timezone.
12. Insert the event row into the matching category table.
13. Insert the matching `log_imported_lines` row.
14. Commit the transaction for that imported event.
15. Print the final summary.

## Internal Interfaces

The implementation should use C and libpq.

Importer configuration:

```c
struct bmy_log_importer_config {
	const char *date_arg;        /* YYYY-MM-DD */
	const char *home_dir;
	const char *source_file;     /* YYYY-MM-DD.log */
	const char *source_path;     /* $HOME/newtrace/YYYY-MM-DD.log */
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
};
```

Main importer functions:

```c
int bmy_log_importer_parse_args(
	int argc,
	char **argv,
	struct bmy_log_importer_config *config);

int bmy_log_importer_run(
	const struct bmy_log_importer_config *config,
	struct bmy_log_import_summary *summary);
```

Database boundary:

```c
bool bmy_log_importer_is_line_imported(
	PGconn *conn,
	const char *source_file,
	unsigned long source_line);

bool bmy_log_importer_insert_event(
	PGconn *conn,
	const char *source_file,
	unsigned long source_line,
	const struct bmy_log_parse_result *result);
```

Interface rule:

- `bmy_log_importer_insert_event` owns the transaction that inserts both the category-table row and the `log_imported_lines` row.
- The importer shell should not inspect parser internals beyond status, line time, target table, and typed payload fields.
- The parser should not know about PostgreSQL or `log_imported_lines`.

## Transaction Strategy

- The category-table insert and `log_imported_lines` insert must be atomic.
- If either insert fails, neither row should remain committed.
- Already-imported, discarded, unrecognized, and parse-failed lines do not need database writes.

Initial recommendation:

- Use one transaction per imported event line.
- This keeps failure isolation simple for historical imports.
- Batch optimization can be considered later if import speed becomes a real problem.

## Summary Counters

The importer should report at least:

- `total_lines`
- `inserted`
- `already_imported`
- `discarded`
- `unrecognized`
- `failed`

Optional counters:

- per-table inserted counts
- first failed line number
- first unrecognized line number

Summary format:

- Human-readable text is enough.
- JSON output is not needed for the first implementation.

## Error Handling

- Missing source file: report and exit non-zero without inserting rows.
- Invalid date argument: report and exit non-zero without reading a file.
- Database connection failure: report and exit non-zero.
- Already imported line: count and continue.
- Discarded line: count and continue.
- Unrecognized line: count and continue.
- Decode or parse failure: count and continue unless the failure prevents safe file processing.
- Insert failure for one accepted line: roll back that line, report the failing source line, and stop the importer immediately with a non-zero exit code.

## Validation

- Parser behavior should be validated first through tests owned by [00-0002-logging-importer-parser.md](./00-0002-logging-importer-parser.md).
- Import a small sample file containing at least one accepted line.
- Confirm accepted events appear in category tables.
- Confirm `log_imported_lines.source_file` stores `YYYY-MM-DD.log`, not a full path.
- Confirm every inserted event has a matching `log_imported_lines` row.
- Run the importer twice for the same date and confirm no duplicate rows.
- Import a sample file containing discarded and unrecognized lines and confirm they are counted but not inserted.
- Run with a missing date file and confirm non-zero exit without inserts.

## Work Breakdown

- Create utility directory and build integration.
- Add local `libcheck` based parser test target under `local_utl/log_importer`.
- Implement command-line parsing.
- Implement date, fixed UTC+8, and source-path resolution.
- Implement PostgreSQL connection setup.
- Implement `getline(3)` based source file reading and physical line numbering.
- Implement idempotency lookup by `(source_file, source_line)`.
- Integrate parser result handling.
- Implement category-table insertion dispatch.
- Implement `log_imported_lines` insertion.
- Implement transaction handling.
- Implement summary output and exit codes.
- Validate with sample logs.

## Backlog

- Add configurable timezone handling if logs from another timezone need to be imported later.
