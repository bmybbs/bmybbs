# Logging System Phase 2: Importer Design

> Status: seeded

## Summary

This page records the design for the historical log importer.

The importer reads existing daily `newtrace` files, parses supported event families, and inserts categorized rows into PostgreSQL tables defined by [phase-2-database-design.md](./phase-2-database-design.md).

The parser design is based on the current semantic logging wrappers and current-state documentation. Older historical logs may use earlier formats and may become unrecognized or failed lines during import.

## Initial Importer Shape

- Accept one log date per invocation, for example `importer YYYY-MM-DD`.
- Support `--dry-run` to parse and summarize a log file without checking import state or writing to PostgreSQL.
- Locate the legacy log file using the canonical `$HOME/newtrace/YYYY-MM-DD.log` pattern.
- Store `YYYY-MM-DD.log`, not the full path, in `log_imported_lines.source_file`.
- Parse the event date from the invocation argument and the event time from each log line.
- Convert the reconstructed legacy local time to `TIMESTAMPTZ`.
- Use fixed UTC+8 for the first implementation.
- Read the file line by line.
- Treat `*` as ordinary content, not as a line-break marker.
- Insert category-table rows for all accepted event families defined by the database design.
- Insert the category-table row and the matching `log_imported_lines` row in the same transaction.
- Use `log_imported_lines(source_file, source_line)` as the idempotency boundary.
- Treat discarded APIs and unrecognized lines as skipped input, and report them in the importer summary.
- Use dry-run output to find old or unexpected log formats and turn them into parser test cases when needed.
- Support script-driven batch import later by keeping the single-file importer small and predictable.

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
- Dry-run parsing is implemented for historical-format discovery without database writes.
- The implementation is awaiting build and runtime validation in the test environment, including historical dry-run passes and database idempotency checks.

## Validation Goals

- Importing the same file twice should not create duplicate rows.
- Every inserted category-table row should have one matching `log_imported_lines` row.
- Article actions should map to the expected `action` value.
- Range delete rows should preserve scope and numeric range.
- Board usage rows should preserve the legacy stay duration as non-negative seconds.
- Session duration rows should preserve the legacy stay duration as non-negative seconds.
- Login failure rows should preserve the historical source host.
- Session rows should preserve available source host, target user, and login type information.
- Account rows should preserve user number, source host, and login type information when present.
- Mail rows should preserve sender and target.
- User interaction rows should preserve initiator, target, and action.
- User query rows should preserve query actor, target, and period.
- Announcement rows should preserve action, board, and either path or imported title information.
- Board deny rows should preserve operator, board, and target user.
- Imported text fields should be valid UTF-8.
- Discarded and unrecognized lines should be counted in importer output and remain available in the original files.
- Dry-run should parse and count accepted, discarded, unrecognized, and failed lines without inserting rows.

## Backlog

- Add configurable timezone handling if logs from another timezone need to be imported later.
