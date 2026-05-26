---
name: review-use-case
description: Review a wiki use case for clarity, testability, convention compliance, and usefulness for implementation planning.
---

# Review Use Case

Use this skill when the user asks to review, check, validate, or improve a use case.

## Required References

Read these files before reviewing:

- `doc/wiki/use-cases/README.md`
- `doc/wiki/module-ids.md`
- `doc/wiki/wiki-conventions.md`

Also read linked related design pages when available.

## Review Checklist

Check whether:

- filename follows `MM-NNNN-short-name.md` or an allowed sidecar draft form
- module id is assigned in `doc/wiki/module-ids.md`
- status follows `doc/wiki/wiki-conventions.md`
- goal is concrete and scenario-based
- preconditions are sufficient
- basic flow describes observable behavior, not implementation tasks
- alternative flows cover meaningful branches, retries, skipped input, and expected errors
- postconditions are clear
- validation steps are practical enough for human or agent follow-up
- assumptions and uncertainty are explicit
- related design links are present and useful

## Review Style

- Prioritize findings that block implementation planning.
- Flag ambiguous wording and hidden assumptions.
- Suggest compact rewrites when they make the scenario more testable.
- Do not turn the use case into an implementation plan.
- If the use case is solid, say so and mention any residual risks.

## Output

Return findings first, ordered by severity. Include file references when reviewing an existing file.
