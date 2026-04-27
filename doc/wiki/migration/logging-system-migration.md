# Logging System Migration

> Status: seeded

## Summary

This page describes the staged migration of the legacy logging system. The long-term goal is to move from the current file-based `newtrace` and `bbslogd` pipeline toward a database-backed logging system, but the first phase should not touch the database yet. Phase 1 is a refactoring phase whose purpose is to make the existing logging behavior easier to understand, classify, and migrate safely later.

## Why Logging Goes First

- The logging system is less tightly coupled to core user-facing behavior than boards, articles, or mail storage.
- It is a good place to validate migration workflow and refactoring discipline before touching more central data domains.
- The current logging path already contains several distinct event families, so it is a useful case study for separating business events from operational diagnostics.

## Phase 1: Interface Refactoring Without Storage Changes

### Scope

- Replace direct `newtrace` calls with clearer, meaningful logging interfaces grouped by event family.
- Keep the current storage path unchanged:
  - callers still end up sending text to `newtrace`
  - `newtrace` still sends messages through the SysV queue
  - `bbslogd` still writes daily log files on disk
- Do not introduce PostgreSQL writes in this phase.
- Do not import existing historical log files in this phase.

### Goal

The main goal of Phase 1 is to make the write path explicit enough that later migration work can see clearly what kinds of log categories exist in practice. After this phase, the project should have a cleaner picture of:

- which logs are business-facing
- which logs are moderation or audit-related
- which logs are statistics-oriented
- which logs are operational or debug-only

### Target Behavior

- Each caller should use a semantic interface instead of calling `newtrace` directly.
- In Phase 1, the implementation behind those interfaces should still emit the exact same text messages into `newtrace`.
- The on-disk log format, file layout, and operational workflow should remain unchanged.
- Existing tools that inspect current log files should continue to work.

### Event Families To Refactor First

- account and session events
- board usage and statistics events
- post, mail, and content-lifecycle events
- moderation and administrative content-state events
- operational and integration diagnostics

The purpose here is not yet to redesign storage, but to force every direct `newtrace` call into a clearer category.

### Validation Goals

- The refactored code should still produce the same log text as before.
- Direct `newtrace` calls should disappear from subsystem code and remain only behind the newly introduced interfaces.
- The new interface layout should make future database schema design easier, because event families are clearer than before.

### Exit Criteria

- Direct `newtrace` calls are replaced by the new semantic logging interfaces.
- The emitted text remains unchanged for existing logging behavior.
- The project has a clearer, reviewable map of logging categories to guide database design in the next phase.

## Phase 2 Preview: Database Design And Historical Import

- Design the database structure after Phase 1 has clarified the logging categories.
- Introduce a log importer for existing file-based logs.
- The importer should accept one log-file path per invocation and handle only that file.
- Batch import should be done later by scripts calling the importer repeatedly, rather than by one monolithic import pass.
- The importer should remain useful during the transition period until direct database writes are deployed in production.

## Phase 3 Preview: Direct Database Writes

- Move the production write path from file-only logging toward direct database writes.
- Keep the importer available for historical backfill, replay, or cutover support while the new path is being deployed.
- Use the Phase 1 interface boundaries and the Phase 2 schema/import experience as the basis for the final write-path transition.

## Import And Coexistence Strategy

- Phase 1: file-based logging remains the only active storage path.
- Phase 2: file-based historical logs are imported gradually through a single-file importer.
- Phase 3: direct database writes are introduced, while import tooling remains available until cutover is stable.

## Current Assessment

- Phase 1 is intentionally conservative.
- It does not prove PostgreSQL integration yet.
- Instead, it reduces ambiguity in the write path so that database work in the next phase is based on clearer event categories instead of raw `newtrace` call sites.

## Open Questions

- How many semantic logging interfaces should Phase 1 introduce, and how coarse or fine-grained should they be?
- Should some event families remain file-oriented even after database-backed logging exists?
- At what point during Phase 2 or Phase 3 should `bbslogd` shrink from an active business-log sink toward a smaller operational-only role?
