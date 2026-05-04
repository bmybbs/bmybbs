# Logging System Phase 1: Interface Refactoring

> Status: seeded

## Summary

Phase 1 is a storage-preserving refactor. It replaces direct `newtrace` calls with semantic `bmy_log_*` interfaces while keeping the existing runtime path unchanged.

## Scope

- Replace direct `newtrace` calls with clearer logging interfaces grouped by event family.
- Keep the current storage path unchanged:
  - callers still end up sending text to `newtrace`
  - `newtrace` still sends messages through the SysV queue
  - `bbslogd` still writes daily log files on disk
- Do not introduce PostgreSQL writes in this phase.
- Do not import existing historical log files in this phase.

## Goal

The goal is to make the write path explicit enough that later migration work can see what kinds of log categories exist in practice.

After this phase, the project should have a clearer picture of:

- which logs are business-facing
- which logs are moderation or audit-related
- which logs are statistics-oriented
- which logs are operational or debug-only

## Target Behavior

- Each caller should use a semantic interface instead of calling `newtrace` directly.
- The implementation behind those interfaces should still emit the same text messages into `newtrace`.
- The on-disk log format, file layout, and operational workflow should remain unchanged.
- Existing tools that inspect current log files should continue to work.

## Event Families

- account and session events
- board usage and statistics events
- post, mail, and content-lifecycle events
- moderation and administrative content-state events
- operational and integration diagnostics

The purpose is not to redesign storage yet, but to force direct `newtrace` usage into clearer categories.

## Current State

The semantic logging layer now lives in:

- [include/bmy/logging.h](../../../../include/bmy/logging.h)
- [libbmy/logging.c](../../../../libbmy/logging.c)

The current implementation still formats legacy-compatible text and calls `newtrace`.

## Exit Criteria

- Direct `newtrace` calls are replaced by the semantic logging interfaces.
- The emitted text remains unchanged for existing logging behavior.
- The project has a clearer, reviewable map of logging categories to guide database design in Phase 2.
