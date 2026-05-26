# Module IDs

> Status: seeded

This page reserves module/domain ids used by structured wiki documents such as use cases and implementation plans.

## Assigned IDs

| ID | Module / Domain | Notes |
| --- | --- | --- |
| `00` | logging system | Legacy `newtrace`, semantic logging APIs, PostgreSQL logging import, and direct log writes. |

## Rules

- IDs are two-character hexadecimal values from `00` to `FF`.
- Use the same ID across use cases and implementation plans for the same module/domain.
- Add a new row before creating structured documents for a new module/domain.
- Do not reuse an ID for a different module/domain.
- If a module/domain needs more than 10,000 documents of one type, assign another ID and explain the split in the notes.
