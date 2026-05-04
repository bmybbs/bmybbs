# Logging System Migration

> Status: seeded

## Summary

This page is the overview for the staged migration of the legacy logging system. The long-term goal is to move from the current file-based `newtrace` and `bbslogd` pipeline toward database-backed business logs, while keeping the migration small enough to review phase by phase.

Logging is the first migration experiment because it is less tightly coupled to core user-facing behavior than boards, articles, or mail storage. It is also a useful case study for separating business events from operational diagnostics.

## Phase Overview

### Phase 1: Interface Refactoring

Detailed page: [logging-system/phase-1-refactor.md](./logging-system/phase-1-refactor.md)

Phase 1 replaces direct `newtrace` calls with semantic `bmy_log_*` interfaces while keeping the existing file-based storage path unchanged.

Current state:

- semantic interfaces live in [include/bmy/logging.h](../../../include/bmy/logging.h)
- compatibility implementation lives in [libbmy/logging.c](../../../libbmy/logging.c)
- storage still flows through `newtrace`, the SysV queue, `bbslogd`, and daily disk files

### Phase 2: Database Design And Historical Import

Detailed page: [logging-system/phase-2-import.md](./logging-system/phase-2-import.md)

Phase 2 designs the database structure after Phase 1 has clarified the logging categories. It also introduces historical import for existing file-based `newtrace` logs. It should still avoid direct production database writes from live code.

Current preview:

- design schemas category by category
- give stable event groups dedicated tables
- keep unmodeled events out of import scope until their category is designed
- keep import tracking separate from category tables
- introduce a log importer for existing file-based logs
- accept one log-file path per invocation and handle only that file
- support script-driven batch import later instead of one monolithic import pass
- keep the importer useful during the transition period until direct database writes are deployed in production

### Phase 3: Direct Database Writes

Detailed page: [logging-system/phase-3-direct-write.md](./logging-system/phase-3-direct-write.md)

Phase 3 will move the production write path from file-only logging toward direct database writes. It is intentionally shallow until Phase 2 proves the schema and importer approach.

## Import And Coexistence Strategy

- Phase 1: file-based logging remains the only active storage path.
- Phase 2: file-based historical logs are imported gradually through a single-file importer.
- Phase 3: direct database writes are introduced, while import tooling remains available until cutover is stable.

## Current Assessment

- Phase 1 has produced the semantic logging boundary needed for later schema design.
- Phase 2 remains a database design and historical import phase, with schemas designed incrementally by event category.
- Runtime diagnostics may eventually use a separate service logging approach instead of the business-log schema.

## Open Questions

- Which event categories should be modeled first in Phase 2?
- Should some event families remain file-oriented even after database-backed logging exists?
- At what point during Phase 2 or Phase 3 should `bbslogd` shrink from an active business-log sink toward a smaller operational-only role?
