# Logging System Phase 2: Database Design And Historical Import

> Status: seeded

## Summary

Phase 2 will design the database structure after Phase 1 has clarified the logging categories. It will also introduce historical import for existing file-based `newtrace` logs.

This page is currently a phase placeholder created during the logging-system migration page split. Detailed schema design should be added in a separate follow-up draft after the split is reviewed.

## Preview

- Design the database structure after Phase 1 has clarified the logging categories.
- Introduce a log importer for existing file-based logs.
- The importer should accept one log-file path per invocation and handle only that file.
- Batch import should be done later by scripts calling the importer repeatedly, rather than by one monolithic import pass.
- The importer should remain useful during the transition period until direct database writes are deployed in production.

## Notes

- Keep this page focused on Phase 2.
- Keep category and schema design changes reviewable as separate amendments.
