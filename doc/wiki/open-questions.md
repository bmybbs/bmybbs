# Open Questions

> Status: seeded

## Summary

This page tracks important questions that are still genuinely open from the current planning perspective. It is not intended to duplicate every local uncertainty already mentioned on other pages. Instead, it focuses on questions that may affect architecture, migration cost, implementation strategy, or project planning in a substantial way.

## Architecture Questions

### Storage Cost After Migration

- Question:
  - After switching to PostgreSQL and object storage, how much total storage space will the new system require?
- Why it matters:
  - Storage cost affects infrastructure planning, backup policy, and how comfortable the project can be with keeping legacy-compatible GBK data alongside UTF-8 data.
  - The new design may reduce space in some areas through compression or attachment reuse, but dual-format storage may also increase total size.
- Related pages:
  - `project-overview.md`
  - `target-architecture.md`
- Current status:
  - Open.

## Migration Questions

No high-priority migration questions have been promoted to this page yet. Many migration-related unknowns are expected to be resolved later through reverse-engineering and redesign work rather than treated as open strategic questions now.

## Data And Encoding Questions

No separate high-priority questions are listed here yet. Some data and encoding issues are already directionally decided for the transition period, even if implementation details remain unresolved.

## Subsystem Questions

### TUI Implementation Cost

- Question:
  - How much time and effort would it take to design and implement a useful TUI client?
- Why it matters:
  - The future TUI is part of the target direction, but the current team has limited experience with `ratatui` and related Rust UI tooling.
  - This uncertainty may affect scheduling, subsystem boundaries, and how aggressively the project can depend on a TUI in earlier migration phases.
- Related pages:
  - `project-overview.md`
  - `target-architecture.md`
- Current status:
  - Open.

## Operational Questions

No operational questions have been promoted to this page yet.

## Deferred Questions

- Many behavior-level questions about legacy workflows are intentionally deferred until reverse-engineering begins.
- Questions that are unknown to the agent but not truly open from the human contributor's perspective do not need to be listed here yet.
