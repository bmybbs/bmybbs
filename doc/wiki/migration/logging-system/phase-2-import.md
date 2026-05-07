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
- Importer design
  - Draft in progress, not canonical yet.

## Current State

- The database design is stable enough for initial implementation work.
- The initial PostgreSQL schema has been drafted in [db/bmy.pg.sql](../../../../db/bmy.pg.sql).
- Importer design has not been published yet.

## Boundaries

- Keep `bbslogd` and existing disk logs as the active production path during Phase 2.
- Preserve original log files as the raw backup source.
- Import only categorized business/event logs.
- Do not import discarded runtime diagnostics into business-event tables.
- Do not add foreign keys to logging tables.

## Open Work

- Draft and review the importer design.
- Implement the importer after the importer design is stable.
- Validate import idempotency and selected event-family mappings.
