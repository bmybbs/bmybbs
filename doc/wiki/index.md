# Wiki Index

> Status: seeded

This wiki is the shared working knowledge base for understanding the legacy BMYBBS system and planning its migration.

Use this index as the entry point to canonical wiki pages. Sidecar draft pages such as `*.human.md`, `*.codex.md`, or `*.<agent>.md` are discussion artifacts and are not listed here as canonical content.

## How To Use This Wiki

- Read [wiki-conventions.md](./wiki-conventions.md) first for page status terms, draft workflow, and evidence rules.
- Use canonical `*.md` pages as the current shared understanding.
- Treat source code as evidence, not as the sole truth.
- Record conflicts, uncertainty, and open questions explicitly.

## Canonical Pages

- [wiki-conventions.md](./wiki-conventions.md)
  - Status: `seeded`
  - Defines wiki workflow, page status terms, draft handling, and evidence rules.
- [module-ids.md](./module-ids.md)
  - Status: `seeded`
  - Shared module/domain id registry for structured wiki documents such as use cases and implementation plans.
- [use-cases/README.md](./use-cases/README.md)
  - Status: `seeded`
  - Defines use-case document naming, template, and writing rules.
- [use-cases/index.md](./use-cases/index.md)
  - Status: `seeded`
  - Index of canonical use cases.
- [implementation-plans/README.md](./implementation-plans/README.md)
  - Status: `seeded`
  - Defines implementation-plan document naming, template, and writing rules.
- [implementation-plans/index.md](./implementation-plans/index.md)
  - Status: `seeded`
  - Index of canonical implementation plans.
- [project-overview.md](./project-overview.md)
  - Status: `seeded`
  - High-level description of the current system, its maintenance problems, and the target migration direction.
- [target-architecture.md](./target-architecture.md)
  - Status: `seeded`
  - Seed architecture page for the future system shape, layer responsibilities, retirements, and migration direction.
- [legacy-subsystems.md](./legacy-subsystems.md)
  - Status: `seeded`
  - High-level map of major legacy access layers, service daemons, shared libraries, and their likely migration fate.
- [logs.md](./logs.md)
  - Status: `seeded`
  - Chronological record of important wiki operations and major status changes.
- [open-questions.md](./open-questions.md)
  - Status: `seeded`
  - Curated backlog of genuinely open strategic questions that may affect architecture, migration, or planning.
- [migration-stages.md](./migration/migration-stages.md)
  - Status: `seeded`
  - Staged migration strategy from low-risk PostgreSQL validation through data migration, API migration, and legacy retirement.
- [cross-cutting/logging-system.md](./cross-cutting/logging-system.md)
  - Status: `seeded`
  - Current-state map of the legacy `newtrace` and `bbslogd` logging pipeline, its event families, encoding risks, and migration relevance.
- [migration/logging-system-migration.md](./migration/logging-system-migration.md)
  - Status: `seeded`
  - Phase-oriented plan for refactoring the logging write path first, then designing database import and direct-write phases later.

## Planned Pages
