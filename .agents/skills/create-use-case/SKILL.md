---
name: create-use-case
description: Draft a structured wiki use case under doc/wiki/use-cases from a scenario or design discussion.
---

# Create Use Case

Use this skill when the user asks to create, draft, frame, or write a use case.

## Required References

Read these files before drafting:

- `doc/wiki/use-cases/README.md`
- `doc/wiki/module-ids.md`
- `doc/wiki/wiki-conventions.md`

Also read any related design page named by the user.

## Workflow

1. Identify the module/domain and choose the module id from `doc/wiki/module-ids.md`.
2. Pick the next reasonable `NNNN` id by inspecting existing files in `doc/wiki/use-cases`.
3. Use the filename format from `doc/wiki/use-cases/README.md`.
4. Draft a sidecar file first unless the user explicitly asks for canonical output.
5. Keep the use case focused on observable behavior, not implementation details.
6. Link to the related design page when known.

## Drafting Rules

- Use the template from `doc/wiki/use-cases/README.md`.
- Use `> Status: draft` for sidecar drafts.
- Prefer `*.codex.md` for Codex-created drafts.
- Write `Basic Flow` as scenario steps.
- Put branches, retries, skipped input, and error paths in `Alternative Flows`.
- Put code layout, function names, parser structure, and task breakdown in implementation plans, not use cases.
- Record assumptions and uncertainty explicitly.

## Output

After creating or updating a use-case draft, report:

- file path
- selected use-case id
- related design link
- any assumptions or open questions
