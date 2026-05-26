# Logging System Phase 3: Direct Database Writes

> Status: seeded

## Summary

Phase 3 will move the production write path from file-only logging toward direct database writes. This phase is intentionally shallow for now because it depends on Phase 2 schema and importer experience.

## Expected Direction

- Use the Phase 1 semantic logging interfaces as the write boundary.
- Use the Phase 2 category schemas as the basis for database-backed writes.
- Keep import tooling available for historical backfill, replay, or cutover support while the new path is being deployed.
- Decide later whether `bbslogd` remains only for runtime diagnostics, shrinks further, or is retired.

## Open Questions

- Which categories should support direct database writes first?
- Should direct writes also keep file logging during a transition window?
- At what point should `bbslogd` stop being an active business-log sink?
