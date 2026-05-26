# Use Case Conventions

> Status: seeded

This folder stores concrete behavior scenarios that bridge design pages and implementation work.

Use cases describe what must work. They should not describe how the code will be built.

## File Naming

Use:

```text
MM-NNNN-short-name.md
```

- `MM`: module/domain id from [../module-ids.md](../module-ids.md)
- `NNNN`: four-digit decimal use-case number
- `short-name`: lowercase words joined by hyphens

Example: `00-0001-import-one-daily-log.md`

Use the canonical/draft workflow and status terms from [../wiki-conventions.md](../wiki-conventions.md).

## Template

```md
# UC MM-NNNN: Short Title

> Status: draft
> Module: module name
> Related design: ../relative/path/to/design.md

## Goal

Describe the scenario in one or two concrete sentences.

## Preconditions

- Required data, configuration, schema, service, or environment state.

## Basic Flow

1. First normal step.
2. Second normal step.
3. Final normal step.

## Alternative Flows

- Important branch, error path, retry path, or skipped path.

## Postconditions

- Expected state after the flow completes.

## Validation

- Concrete checks that can prove the use case works.

## Notes

- Optional context, uncertainty, or implementation hints.
```

## Writing Rules

- Keep the use case concrete and scenario-based.
- Prefer observable behavior over internal implementation details.
- Link to related design pages instead of repeating them.
- Record assumptions explicitly.
- Include alternative flows when they affect implementation or validation.
- Keep validation practical enough for a human or agent to turn into checks.
