# Implementation Plan Conventions

> Status: seeded

This folder stores concrete engineering plans for building changes described by design pages and use cases.

Implementation plans describe how work should be done in code.

## File Naming

Use:

```text
MM-NNNN-short-name.md
```

- `MM`: module/domain id from [../module-ids.md](../module-ids.md)
- `NNNN`: four-digit decimal implementation-plan number
- `short-name`: lowercase words joined by hyphens

Example: `00-0001-logging-importer.md`

Use the canonical/draft workflow and status terms from [../wiki-conventions.md](../wiki-conventions.md).

## Template

```md
# IP MM-NNNN: Short Title

> Status: draft
> Module: module name
> Related design: ../relative/path/to/design.md
> Related use cases: ../relative/path/to/use-case.md

## Goal

Describe the implementation outcome in one or two concrete sentences.

## Scope

- What this plan will implement.

## Non-Goals

- What this plan intentionally will not implement.

## Inputs And Outputs

- Required inputs, configuration, files, database tables, or APIs.
- Expected outputs, side effects, or persisted data.

## Files And Modules

- Files or modules expected to be created or changed.
- Ownership boundaries if work may be delegated.

## Data Flow

1. Main processing step.
2. Main processing step.
3. Main processing step.

## Error Handling

- Expected error cases and how the implementation should report or recover.

## Validation

- Commands, checks, sample data, or manual review steps that prove the plan works.

## Work Breakdown

- Small implementation tasks that can be assigned to humans or agents.

## Open Questions

- Questions that must be answered before or during implementation.
```

## Writing Rules

- Keep the plan concrete enough to guide coding.
- Link to related design pages and use cases instead of repeating them.
- Identify files and modules when they are known.
- Record non-goals to prevent scope creep.
- Prefer explicit data flow and error handling over vague implementation notes.
- Split large plans when independent contributors can work on separate parts.
- Mark delegation boundaries clearly if multiple agents or humans may work in parallel.
