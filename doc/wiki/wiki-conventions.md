# Wiki Conventions

> Status: seeded

This page defines how `doc/wiki` is maintained as a shared working wiki between human contributors and agents.

## Canonical And Draft Pages

In the examples below, `foo` is a placeholder for a page topic name such as `project-overview`, `target-architecture`, or `article-storage`.

- `foo.md` is the canonical wiki page.
- `foo.human.md` is a human draft for discussion.
- `foo.codex.md` is a Codex draft for discussion.
- `foo.<agent>.md` may be used by other agents in the same way.

Only `foo.md` should be treated as canonical wiki content.

Draft pages are discussion artifacts:

- they are used to compare alternative wording or structure
- they should not be treated as final conclusions
- they should not be used directly as source material for implementation work
- they should not be linked from the wiki index as canonical pages

## Page Status Terms

Canonical pages should use a simple status model.

- `draft`
  - discussion draft, not canonical
  - usually used only for `*.human.md`, `*.codex.md`, or other sidecar draft pages
- `seeded`
  - canonical page exists
  - the page has an initial structure and core content
  - it can be treated as a formal first draft, but it is still shallow
- `grounded`
  - the canonical page has been checked against stronger evidence
  - evidence may include code reading, runtime behavior, historical context, or direct user confirmation
  - the page is more reliable than a seeded page, but may still contain open questions
- `actionable`
  - the canonical page is mature enough to guide refactor, migration, implementation planning, or other concrete work

For Chinese readers, these terms can be understood roughly as:

- `draft`: 草稿
- `seeded`: 初稿
- `grounded`: 已核实
- `actionable`: 可执行

## Suggested Workflow

A page normally evolves in this order:

1. A discussion starts in one or more draft pages such as `foo.human.md` and `foo.codex.md`.
2. Human and agent contributors compare drafts, identify disagreements, and refine the structure.
3. The agreed result is merged into canonical `foo.md`.
4. The canonical page status is updated to reflect its maturity.
5. Later revisions may repeat the same draft-and-merge process.

## Revising Published Pages

When a canonical page already exists and needs to be revised, new work should still happen in sidecar draft pages.

- keep the canonical page's current status while the revision is still under discussion
- use `*.human.md`, `*.codex.md`, or other sidecar drafts to explore the revision
- after the revision is merged into the canonical page, re-evaluate the canonical status
- canonical status may move upward or downward
- if a canonical page is downgraded, record the reason briefly near the status or in a short note

## Evidence And Conflicts

The wiki is shared working memory, not a perfect source of truth.

- source code is evidence, but not the sole truth
- user statements are high-value evidence, especially for historical intent and operational priorities
- legacy behavior is not automatically the desired future behavior
- conflicts between wiki pages, code, and user statements should be marked explicitly
- uncertainty should be recorded instead of hidden

## Special Files

The wiki relies on two operational files that the LLM must maintain alongside content pages:

- `index.md`
  - Content-oriented catalog of all wiki pages.
  - Each entry includes a link, a one-line summary, and optional metadata (date, source count, status).
  - Organized by category (entities, concepts, sources, skills, etc.).
  - Updated on every ingest, query, or lint pass. The LLM reads this first during queries to locate relevant pages.
- `logs.md`
  - Chronological, append-only record of wiki operations (ingests, queries, lint passes, status changes).
  - Entries should use a consistent prefix for parseability, e.g., `## [YYYY-MM-DD] ingest | Article Title`.
  - Provides a timeline of the wiki's evolution and helps the LLM understand recent activity.

## Scope

This conventions page describes how the wiki is maintained.

It does not replace:

- `AGENTS.md`, which defines repo-level operating rules for agents
- subsystem pages, which describe the project itself
- migration pages, which describe target architecture and planning
