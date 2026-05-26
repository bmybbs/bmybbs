# Logging System Phase 2: Database Design

> Status: seeded

## Summary

This page records the database design for importing legacy `newtrace` logs into PostgreSQL.

The executable schema lives in [db/bmy.pg.sql](../../../../db/bmy.pg.sql). This page explains why the tables exist, which APIs they cover, and which logs are excluded from the first database import.

## Design Principles

- Use category tables for stable event families.
- Keep import tracking separate from event tables.
- Do not use foreign keys in log tables.
- Preserve historical text values even if users, boards, mails, or articles are later deleted or renamed.
- Preserve known legacy limitations instead of inventing missing identifiers.
- Keep indexes minimal until real query patterns justify more.

## Discarded APIs

These APIs are excluded from the first database import. Some are runtime diagnostics that may still matter operationally, but they should be handled by future service logging instead of business-event tables.

- `bmy_log_search_result_count`
- `bmy_log_cache_reload`
- `bmy_log_system_reload`
- `bmy_log_runtime_error`
- `bmy_log_thread_view`
- `bmy_log_search_trace`
- `bmy_log_selection_trace`

## Category Tables

### `log_article_events`

Article-like actions on a board.

Related APIs:

- `bmy_log_post_create`
- `bmy_log_post_check_1984`
- `bmy_log_post_crosspost`
- `bmy_log_post_same_title`
- `bmy_log_post_edit`
- `bmy_log_post_delete`
- `bmy_log_post_restore`
- `bmy_log_post_title_change`
- `bmy_log_post_mark`
- `bmy_log_post_unmark`
- `bmy_log_post_digest`
- `bmy_log_post_undigest`
- `bmy_log_post_water`
- `bmy_log_post_unwater`
- `bmy_log_post_top`
- `bmy_log_post_untop`

Actions:

- create/pre-create: `post`, `check1984`, `crosspost`, `sametitle`
- owner-aware changes: `edit`, `del`, `undel`
- title change: `changetitle`
- mark-style changes: `mark`, `unmark`, `digest`, `undigest`, `water`, `unwater`, `top`, `untop`

Important fields:

- `actor_userid`: operator
- `board`: board name
- `owner_userid`: article owner when available
- `title`: current title, or new title for `changetitle`
- `old_title`: old title for `changetitle`
- `action`: normalized action token

Legacy limitation:

- These logs identify article activity by board and title only.
- They do not include the article timestamp/id needed to locate a specific article uniquely.

### `log_range_delete_events`

Range-delete operations for article lists and mail lists.

Related APIs:

- `bmy_log_post_range_delete`
- `bmy_log_mail_range_delete`

Important fields:

- `scope`: `article` or `mail`
- `userid`: operator
- `board`: board name for article range delete
- `from_id`, `to_id`: legacy numeric range

### `log_board_usage_events`

Board stay-time statistics.

Related API:

- `bmy_log_board_use`

Important fields:

- `userid`: visitor
- `board`: board name
- `stay_seconds`: recorded board stay duration

This table is separate because `stay_seconds` is useful for board statistics and scoring.

### `log_session_duration_events`

Session end events that carry a stay duration.

Related APIs:

- `bmy_log_logout`
- `bmy_log_disconnect`

Important fields:

- `userid`: user whose session ended
- `action`: `logout` or `disconnect`
- `stay_seconds`: recorded session duration

### `log_login_failure_events`

Failed login attempts by source host.

Related API:

- `bmy_log_login_failure`

Important fields:

- `from_host`: IPv4 or IPv6 source string

This table is important because site-ban checks rely on repeated login failures from a source host.

### `log_session_events`

Session-related events that do not carry a stay duration.

Related APIs:

- `bmy_log_login_success`
- `bmy_log_session_cleanup`
- `bmy_log_multi_login_kick`
- `bmy_log_user_kick`

Important fields:

- `action`: `login_success`, `session_cleanup`, `multi_login_kick`, or `user_kick`
- `userid`: main user id for the event
- `target_userid`: target user when applicable
- `from_host`: source host for login success
- `login_type`: access type when known

### `log_account_events`

Account lifecycle events.

Related APIs:

- `bmy_log_account_create`
- `bmy_log_account_expire_cleanup`

Important fields:

- `action`: `create` or `expire_cleanup`
- `userid`: account user id
- `usernum`: legacy user number when available
- `from_host`: source host for account creation
- `login_type`: creation path when known

### `log_mail_events`

Mail-send events.

Related APIs:

- `bmy_log_mail_send`
- `bmy_log_netmail_send`
- `bmy_log_utility_mail_send`

Important fields:

- `sender`: historical sender id or utility sender name
- `target_userid`: historical target, wider than a normal user id because rare `netmail` targets may be email addresses

`netmail` is rare in historical logs and can be identified later by email-like targets if needed.

### `log_user_interaction_events`

Lightweight user-to-user interaction events.

Related APIs:

- `bmy_log_talk_request`
- `bmy_log_send_goodwish`

Important fields:

- `action`: `talk` or `goodwish`
- `userid`: initiator
- `target_userid`: target user

### `log_user_query_events`

Useful user-query/debug actions.

Related API:

- `bmy_log_finddf`

Important fields:

- `action`: currently only `finddf`
- `userid`: user who ran the query
- `target`: user being queried
- `day_count`: requested period

### `log_announcement_events`

Announcement-area operations.

Related APIs:

- `bmy_log_announce_action`
- `bmy_log_announce_import`

Important fields:

- `action`: `paste`, `moveitem`, `additem`, or `import`
- `userid`: operator
- `board`: related board
- `path`: announcement path for normal actions
- `owner_userid`, `title`: imported article metadata for import events

### `log_board_deny_events`

Board-level deny logs.

Related API:

- `bmy_log_board_deny`

Important fields:

- `operator_userid`: user who added the deny entry
- `board`: board where the deny was applied
- `target_userid`: denied user

## Import Tracking Table

### `log_imported_lines`

This table records which physical source line has already produced an imported event row.

Important fields:

- `source_file`: canonical log filename, normally `YYYY-MM-DD.log`
- `source_line`: physical line number in the source file
- `event_table`: target category table name
- `event_id`: row id in the target category table

The unique constraint on `(source_file, source_line)` is the import idempotency boundary.

## Index Design

The initial SQL schema uses minimal secondary indexes.

- Add one `occurred_at` index for each category table.
- Reason: historical log inspection is expected to start from a time range.
- Add `log_login_failure_events(from_host, occurred_at)` because site-ban checks have a known lookup by source host, often within a time range.
- Defer other user, board, and host indexes until real query patterns appear.
- `log_imported_lines(source_file, source_line)` already has a unique index from its unique constraint.

Likely deferred indexes include user id plus time and board plus time.

## SQL Asset

The current executable schema is [db/bmy.pg.sql](../../../../db/bmy.pg.sql).

When this design changes, update both this page and the SQL file together.
