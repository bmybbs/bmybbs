# IP 00-0002: Logging Importer Parser

> Status: seeded
> Module: logging system
> Related design: ../migration/logging-system/phase-2-importer.md
> Related use cases: ../use-cases/00-0001-import-one-daily-log.md

## Goal

Implement the parser component that classifies one legacy `newtrace` line and converts accepted lines into typed event records for the logging importer.

The first parser version is based on current logging wrappers and current-state documentation. Older historical logs may use earlier formats and should be discovered through dry-run import passes.

## Scope

- Parse one physical log line at a time.
- Extract the line time.
- Classify lines as accepted, discarded, unrecognized, or failed.
- Map accepted event families to target category tables and column values.
- Convert legacy free-text fields from GBK to UTF-8 with `g2u`.
- Preserve `*` as ordinary content.

## Non-Goals

- Do not open files.
- Do not connect to PostgreSQL.
- Do not insert database rows.
- Do not check `log_imported_lines`.
- Do not decide transaction boundaries.
- Do not store skipped-line metadata.

## Inputs And Outputs

Input:

- One physical log line from a daily `newtrace` file.

Output:

- Parsed line time.
- Parser status: accepted, discarded, unrecognized, or failed.
- For accepted lines, target table name and typed field values.
- For discarded, unrecognized, or failed lines, enough reason text for summary/debug output.

## Files And Modules

Expected parser files:

- `local_utl/log_importer/log_parser.c`
- `local_utl/log_importer/log_parser.h`
- `local_utl/log_importer/log_tokenizer.c`
- `local_utl/log_importer/log_tokenizer.h`
- `local_utl/log_importer/test_log_parser.c`
- `local_utl/log_importer/test_log_tokenizer.c`

The importer shell in [00-0001-logging-importer.md](./00-0001-logging-importer.md) owns calling the parser and writing parser results to PostgreSQL.

Test location:

- Parser unit tests should live under the same `local_utl/log_importer` directory.
- Use `libcheck` for the first parser test executable.
- Keep parser tests close to the utility instead of moving parser code into a shared library prematurely.

## Line Format

Expected physical line shape:

```text
HH:MM:SS message
```

Rules:

- `HH:MM:SS` is parsed as the line-local time.
- The importer supplies the date and timezone.
- The parser receives or returns enough data for the importer to reconstruct `occurred_at`.
- `*` is ordinary content and must not be treated as a line-break marker.

## Parser Result Shape

The parser should return one of:

- accepted event
- discarded line
- unrecognized line
- failed line

Accepted event should include:

- target table identifier
- action or scope value when applicable
- typed string/integer fields needed by the target table
- decoded UTF-8 text fields
- typed numeric fields already converted to integers

Failed line should include:

- failure reason
- whether the line should be counted as failed rather than unrecognized

## Internal Interfaces

The parser interface should make line classification explicit and keep database work outside the parser.

Status enum:

```c
enum bmy_log_parse_status {
	BMY_LOG_PARSE_UNSET,
	BMY_LOG_PARSE_ACCEPTED,
	BMY_LOG_PARSE_DISCARDED,
	BMY_LOG_PARSE_UNRECOGNIZED,
	BMY_LOG_PARSE_FAILED,
};
```

Target table enum:

```c
enum bmy_log_event_table {
	BMY_LOG_EVENT_ARTICLE,
	BMY_LOG_EVENT_RANGE_DELETE,
	BMY_LOG_EVENT_BOARD_USAGE,
	BMY_LOG_EVENT_SESSION_DURATION,
	BMY_LOG_EVENT_LOGIN_FAILURE,
	BMY_LOG_EVENT_SESSION,
	BMY_LOG_EVENT_ACCOUNT,
	BMY_LOG_EVENT_MAIL,
	BMY_LOG_EVENT_USER_INTERACTION,
	BMY_LOG_EVENT_USER_QUERY,
	BMY_LOG_EVENT_ANNOUNCEMENT,
	BMY_LOG_EVENT_BOARD_DENY,
};
```

Line time:

```c
struct bmy_log_line_time {
	int hour;
	int minute;
	int second;
};
```

Common parse result:

```c
struct bmy_log_parse_result {
	enum bmy_log_parse_status status;
	struct bmy_log_line_time line_time;
	enum bmy_log_event_table table;
	union bmy_log_event_payload payload;
	const char *reason;
};
```

Representation rule:

- Use the typed union payload approach.
- Each target table has one corresponding payload struct.
- The importer shell dispatches by `table` and consumes the matching payload struct.
- Numeric fields should be parsed by the parser so database insertion code does not parse strings again.

Event payload union:

```c
union bmy_log_event_payload {
	struct bmy_log_article_event article;
	struct bmy_log_range_delete_event range_delete;
	struct bmy_log_board_usage_event board_usage;
	struct bmy_log_session_duration_event session_duration;
	struct bmy_log_login_failure_event login_failure;
	struct bmy_log_session_event session;
	struct bmy_log_account_event account;
	struct bmy_log_mail_event mail;
	struct bmy_log_user_interaction_event user_interaction;
	struct bmy_log_user_query_event user_query;
	struct bmy_log_announcement_event announcement;
	struct bmy_log_board_deny_event board_deny;
};
```

Representative payload structs:

```c
struct bmy_log_article_event {
	const char *actor_userid;
	const char *board;
	const char *owner_userid;
	const char *title;
	const char *old_title;
	const char *action;
};

struct bmy_log_range_delete_event {
	const char *scope;
	const char *userid;
	const char *board;
	int from_id;
	int to_id;
};

struct bmy_log_login_failure_event {
	const char *from_host;
};
```

Parser entry point:

```c
bool bmy_log_parse_line(
	const char *line,
	struct bmy_log_parse_result *result);

void bmy_log_parse_result_cleanup(
	struct bmy_log_parse_result *result);
```

Ownership rule:

- The parser result should remain valid until the next parse call or explicit cleanup call.
- If the implementation allocates decoded UTF-8 strings, it must provide a cleanup function such as `bmy_log_parse_result_cleanup`.
- The importer shell must not free payload fields directly unless the parser interface explicitly transfers ownership.

## Accepted Event Families

The parser should support all accepted event families from [../migration/logging-system/phase-2-database-design.md](../migration/logging-system/phase-2-database-design.md):

- article events
- range delete events
- board usage events
- session duration events
- login failure events
- session events
- account events
- mail events
- user interaction events
- user query events
- announcement events
- board deny events

## Discarded APIs

The parser should recognize discarded API lines so they can be counted separately from unrecognized lines.

Discarded families:

- `bmy_log_search_result_count`
- `bmy_log_cache_reload`
- `bmy_log_system_reload`
- `bmy_log_runtime_error`
- `bmy_log_thread_view`
- `bmy_log_search_trace`
- `bmy_log_selection_trace`

Discarded lines are not errors and should not produce database events.

## Matching Strategy

- Parse the `HH:MM:SS` prefix first.
- Treat the common message shape as subject, verb, and remaining arguments.
- Use manual tokenization instead of `sscanf` or regular expressions in the first implementation.
- Use the verb as the primary classifier when possible.
- Match distinctive system messages before broad user-message patterns.
- Match longer or more specific patterns before shorter patterns that could also match.
- Classify known discarded patterns before falling back to unrecognized.
- Treat malformed accepted-looking lines as failed, not unrecognized.
- Treat old unsupported formats as unrecognized unless they clearly match a known accepted family with malformed fields.

Examples of ambiguity to handle carefully:

- `kick multi-login` versus normal `kick`
- `drop www/api` versus duration-bearing `drop`
- article `ranged` versus mail `rangedmail`
- normal mail versus utility mail

## Text Decoding

- Convert legacy title/path text from GBK to UTF-8 before returning accepted event fields.
- Treat GBK-origin text as expected.
- Use `g2u` from `bmy/convcode.h` for conversion.
- Run `g2u` for all legacy free-text fields; ASCII-only input remains unchanged.
- Known free-text fields include article titles and announcement paths/titles.
- Do not add separate encoding detection in the first implementation.
- If `g2u` fails, return failed line status.
- Do not preserve raw bytes in parser output; original files remain the raw backup.

## Implementation State

- The parser and its tokenizer helper are implemented under `local_utl/log_importer`.
- Accepted event payloads cover every designed category table.
- Known discarded logging families are classified without database insertion.
- Parser and tokenizer test sources exist; test-environment validation and historical dry-run discovery remain pending.

## Data Flow

1. Receive one physical line.
2. Parse and validate the time prefix.
3. Split the message body from the time prefix.
4. Match the message body against accepted and discarded patterns.
5. Convert free-text fields with `g2u` when required by the matched event family.
6. Return a parser result to the importer shell.

## Error Handling

- Missing or malformed time prefix: failed line.
- Known discarded pattern: discarded line.
- No known pattern: unrecognized line.
- Accepted-looking pattern with invalid field count or invalid number: failed line.
- Failed `g2u` conversion: failed line.
- Failure output should not include the raw message body; the importer can report source filename and line number.

## Validation

- Write parser tests before wiring the parser into the importer shell.
- Provide sample lines for each accepted event family.
- Provide sample lines for each discarded family where practical.
- Provide malformed accepted-looking lines.
- Provide unrecognized lines.
- Treat parser tests as the source of concrete sample coverage.
- Use dry-run discoveries from older historical logs to add parser regression tests.
- Confirm parser output maps to the expected target table and field values.
- Confirm typed numeric payload fields are already integers.
- Confirm `*` stays ordinary content in parsed text fields.

## Work Breakdown

- Define parser result structs/enums.
- Create `test_log_parser.c` with the first sample-line tests.
- Implement time-prefix parsing.
- Implement manual subject/verb/rest tokenization.
- Implement accepted event-family parsers.
- Implement discarded-pattern recognition.
- Implement `g2u` based text conversion helpers.
- Implement parser sample validation.
- Review matching order against real legacy examples.

## Backlog

- Add more real historical sample lines when new parser gaps are found.
- Revisit free-text field coverage if later event families contain GBK-origin text outside article titles and announcement paths/titles.
