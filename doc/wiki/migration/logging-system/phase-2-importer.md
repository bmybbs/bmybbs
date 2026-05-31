# Logging System Phase 2: Importer Design

> Status: seeded

## Summary

This page records the design for the historical log importer.

The importer reads existing daily `newtrace` files, parses supported event families, and inserts categorized rows into PostgreSQL tables defined by [phase-2-database-design.md](./phase-2-database-design.md).

The parser design is based on the current semantic logging wrappers and current-state documentation. Older historical logs may use earlier formats and may become unrecognized or failed lines during import.

## Initial Importer Shape

- Accept one log date per invocation, for example `importer YYYY-MM-DD`.
- Support `--dry-run` to parse and summarize a log file without checking import state or writing to PostgreSQL.
- Support `--fast-import` as an optimized non-dry-run mode only when the source file does not already exist in `log_source_files`.
- Locate the legacy log file using the canonical `$HOME/newtrace/YYYY-MM-DD.log` pattern.
- Store `YYYY-MM-DD.log`, not the full path, once in `log_source_files`.
- Parse the event date from the invocation argument and the event time from each log line.
- Convert the reconstructed legacy local time to `TIMESTAMPTZ`.
- Use fixed UTC+8 for the first implementation.
- Read the file line by line.
- Treat `*` as ordinary content, not as a line-break marker.
- Insert category-table rows for all accepted event families defined by the database design.
- In non-dry-run mode, create or load the `log_source_files` row once per source file.
- Insert the category-table row and matching `log_imported_lines` row in the same transaction.
- Use `log_imported_lines(source_file_id, source_line)` as the idempotency boundary.
- Treat discarded APIs and unrecognized lines as skipped input, and report them in the importer summary.
- Report each unrecognized or failed line to standard error using its source filename, physical line number, and status only; do not print raw legacy log text by default.
- Use dry-run output to find old or unexpected log formats and turn them into parser test cases when needed.
- Support script-driven batch import later by keeping the single-file importer small and predictable.

## Import Modes

### Default Idempotent Mode

Default non-dry-run import uses the conservative path.

- Create or load the `log_source_files` row before processing lines.
- Check `log_imported_lines(source_file_id, source_line)` before parsing each physical line.
- Insert each accepted event row and matching `log_imported_lines` row in one per-event transaction.
- Allow safe reruns of partially imported files.

This mode is safer but slow for large historical imports because it performs database work per line.

### Fast File Mode

`--fast-import` is an optimized path for already dry-run-validated historical files.

- Before importing, check whether `source_file` exists in `log_source_files`.
- If the source file already exists, fall back to default idempotent mode.
- If the source file does not exist, open one transaction for the whole file.
- Create the `log_source_files` row inside that transaction.
- Parse all lines and insert accepted event rows plus matching `log_imported_lines` rows inside the same transaction.
- Commit once after the whole file succeeds.
- Roll back the whole file if any database insert fails.

This mode trades per-line resumability for import speed. It is suitable only after dry-run validation has made parser failures unlikely.

## Encoding Rules

- Convert legacy title/path text from GBK to UTF-8 before inserting category-table rows.
- Treat GBK-origin text as expected legacy data, not as a parse failure by itself.
- Use `g2u` from `bmy/convcode.h`; ASCII-only input remains unchanged.
- If text cannot be converted, skip that line for now instead of inserting lossy data.
- Preserve original text files as the backup source for raw bytes.

## Supported Event Families

The importer should support every accepted event family from the database design:

- article events
- range delete events
- board usage events
- session duration events
- login failure events
- security events
- session events
- account events
- mail events
- user interaction events
- user query events
- announcement events
- board deny events

Discarded APIs should not be imported into business-event tables.

## Implementation State

- The importer is implemented in `local_utl/log_importer`.
- The parser and tokenizer cover the designed accepted event families and recognized discarded log families.
- Non-dry-run import performs categorized inserts and `log_imported_lines` tracking through per-event transactions.
- `--fast-import` supports one transaction per never-imported source file and falls back to default idempotent mode when the source file already exists.
- Dry-run parsing is implemented for historical-format discovery without database writes.
- Unrecognized and failed parser results are reported by filename, physical line number, and status for later inspection in an editor.
- A broad historical dry-run pass has been completed on a test site. Only a small number of lines remained unrecognized or failed, so the parser is considered stable enough for database-import validation.
- The implementation is still awaiting non-dry-run database validation in the test environment, including idempotency checks.

## Validation Goals

- Importing the same file twice should not create duplicate rows.
- Every inserted category-table row should have one matching `log_imported_lines` row.
- Article actions should map to the expected `action` value.
- Range delete rows should preserve scope and numeric range.
- Board usage rows should preserve the legacy stay duration as non-negative seconds.
- Session duration rows should preserve the legacy stay duration as non-negative seconds.
- Login failure rows should preserve the historical source host.
- Security rows should preserve `bot_login`, `bot_register`, `bot_query`, or `bot_reset`, any available submitted trap input, and the source host from deployed `nju09` trap records.
- Session rows should preserve available source host, target user, and login type information.
- Account creation rows should preserve the raw legacy numeric index value and source host, and normalize legacy `www` creation-path markers to login type `NJU09`.
- Account expiration-cleanup rows should preserve the negative legacy `countlife()` value as `life_value`, not as a user number.
- Mail rows should preserve sender and target.
- User interaction rows should preserve initiator, target, and action.
- User query rows should preserve query actor, target, and period.
- Announcement rows should preserve action, board, and either path or imported title information.
- Board deny rows should preserve operator, board, and target user.
- Imported text fields should be valid UTF-8.
- Discarded and unrecognized lines should be counted in importer output and remain available in the original files.
- Unrecognized and failed lines should be locatable from standard-error diagnostics without writing raw legacy text to the terminal.
- Dry-run should parse and count accepted, discarded, unrecognized, and failed lines without inserting rows.
- Fast import should fall back to default idempotent mode when the source file already exists in `log_source_files`.
- Fast import should roll back the whole source file if any database insert fails.

## Backlog

- Add configurable timezone handling if logs from another timezone need to be imported later.
