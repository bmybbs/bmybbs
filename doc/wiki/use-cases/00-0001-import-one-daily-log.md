# UC 00-0001: Import One Daily Log

> Status: seeded
> Module: logging system
> Related design: ../migration/logging-system/phase-2-importer.md

## Goal

Import one legacy daily `newtrace` log file into PostgreSQL, preserving accepted logging events as categorized rows.

## Preconditions

- The PostgreSQL logging schema from `db/bmy.pg.sql` has been applied.
- The source log file exists as `$HOME/newtrace/YYYY-MM-DD.log`.
- The log date is known by the operator.
- The importer can connect to the target PostgreSQL database.

## Basic Flow

1. The operator runs the importer with a log date such as `2026-05-07`.
2. The importer resolves the source file as `$HOME/newtrace/2026-05-07.log`.
3. The importer reads the source file line by line.
4. For each source line, the importer checks whether the source filename and line number already exist in `log_source_files` and `log_imported_lines`.
5. If the source line has already been imported, the importer skips it and counts it as already imported.
6. For each accepted event line, the importer reconstructs `occurred_at` from the date argument, line time, and fixed UTC+8 timezone.
7. The importer converts legacy free-text fields such as title/path from GBK to UTF-8.
8. The importer inserts the event into the matching category table.
9. The importer records the source filename in `log_source_files` when needed, then records the source-file id and line number in `log_imported_lines`.
10. The importer prints a summary of inserted, already-imported, discarded, unrecognized, and failed lines.

## Alternative Flows

- If the source log file does not exist, the importer reports the missing file and exits without inserting rows.
- If a line has already been imported, the importer skips it and counts it as already imported.
- If a line belongs to a discarded API, the importer skips it and counts it as discarded.
- If a line is unrecognized, the importer skips it and counts it as unrecognized.
- If a legacy free-text field cannot be converted with `g2u`, the importer skips that line and counts it as failed.
- If an accepted event line cannot be inserted, the importer reports the failure and does not record that line as imported.
- If the operator runs with `--dry-run`, the importer parses and summarizes the file without checking `log_imported_lines` or inserting rows.
- If older historical log formats are unrecognized or fail parsing, the importer reports counts so those lines can be reviewed and added to parser tests later.

## Postconditions

- Accepted event lines are stored in their category tables.
- Each inserted event row has one matching `log_imported_lines` row.
- Already-imported, discarded, unrecognized, or failed lines do not create new category-table rows.
- The original log file remains unchanged.

## Validation

- Query at least one expected category table and confirm inserted rows exist for the imported date.
- Query `log_source_files` and confirm inserted rows use `YYYY-MM-DD.log` as `source_file`.
- Confirm every inserted category row from this import has a matching `log_imported_lines` row.
- Run the importer twice for the same date and confirm the second run does not duplicate rows.
- Import a sample log containing accepted, discarded, and unrecognized lines.
- Confirm discarded and unrecognized lines do not create category-table rows.
- Confirm the importer summary reports inserted, already-imported, discarded, unrecognized, and failed line counts.
- Run with `--dry-run` and confirm no category-table or `log_imported_lines` rows are inserted.

## Notes

- This use case does not define parser structure, implementation language, or transaction granularity.
- If a developer deletes a `log_imported_lines` row during development, that source line becomes eligible for re-import.
- Historical format drift is expected because old logs may predate the current semantic logging wrappers.
