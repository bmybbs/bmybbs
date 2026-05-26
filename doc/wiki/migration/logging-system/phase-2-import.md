# Logging System Phase 2: Database Design And Historical Import

> Status: seeded

## Summary

Phase 2 proves that legacy file-based `newtrace` logs can become queryable PostgreSQL data.

This phase has two work tracks:

- database design: classify semantic log APIs and define stable PostgreSQL tables
- historical import: build an importer that reads existing daily log files and writes categorized rows

Phase 2 should still avoid direct production database writes from live code. Direct live writes belong to Phase 3.

## Detailed Pages

- Database design: [phase-2-database-design.md](./phase-2-database-design.md)
  - Canonical database design for category tables, discarded APIs, import tracking, and indexes.
  - The executable SQL asset is [db/bmy.pg.sql](../../../../db/bmy.pg.sql).
- Importer design: [phase-2-importer.md](./phase-2-importer.md)
  - Canonical importer design for command shape, parsing boundary, transactions, encoding, and validation.

## Current State

- The initial PostgreSQL schema has been implemented in [db/bmy.pg.sql](../../../../db/bmy.pg.sql).
- A C importer under `local_utl/log_importer` implements single-day import, dry-run parsing, categorized insertion, import tracking, and summary counters.
- The importer uses a thin PostgreSQL wrapper in `libbmy` and is integrated into the CMake build definition.
- Runtime validation against the test environment and historical log files is still pending.

## Boundaries

- Keep `bbslogd` and existing disk logs as the active production path during Phase 2.
- Preserve original log files as the raw backup source.
- Import only categorized business/event logs.
- Do not import discarded runtime diagnostics into business-event tables.
- Do not add foreign keys to logging tables.

## Open Work

- Validate build and PostgreSQL execution in the test environment.
- Run dry-run passes against historical logs to find older or unexpected formats.
- Validate import idempotency and selected event-family mappings with test data.
