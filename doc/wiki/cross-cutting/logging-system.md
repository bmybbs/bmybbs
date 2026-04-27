# Logging System

> Status: seeded

## Summary

The active legacy logging path is centered on `newtrace`: callers pass a text message to `newtrace`, `newtrace` normalizes it into a single-line log record, sends it through a SysV message queue, and `bbslogd` appends it to a daily file under `newtrace/`. This path is simple and familiar, but it depends on OS-specific IPC, a separate daemon, and unstructured text files.

The verified callers show that `newtrace` is used for three different things at once:

- business and security events
- moderation and content-action audit trails
- runtime, cache, and integration diagnostics

That mixed usage is the main design fact for migration. The future system should not migrate the current log files module by module. It should separate event families first.

## Backend Flow

### Components

- `newtrace` is declared in [include/ytht/msg.h](../../../include/ytht/msg.h) and implemented in [libytht/newtrace.c](../../../libytht/newtrace.c).
- `bbslogd` consumes the same queue in [local_utl/bbslogd/bbslogd.c](../../../local_utl/bbslogd/bbslogd.c).

### Message Shape

- `newtrace` builds one line as:
  - `HH:MM:SS`
  - one space
  - caller-provided text
  - trailing newline
- Embedded newlines inside caller text are replaced with `*`, so each `newtrace` record is normalized to a single line.
- This behavior was also confirmed by a small local experiment using:
  - `newtrace("single line")`
  - `newtrace("line 1\nline 2")`
  - which produced:
    - `HH:MM:SS single line`
    - `HH:MM:SS line 1*line 2`
- Messages are sent with `msgsnd(..., IPC_NOWAIT | MSG_NOERROR)`.

### Queue And Failure Behavior

- Both sender and receiver use `BBSLOG_MSQKEY`.
- Both sides set the queue byte limit to roughly `50 KiB`.
- `newtrace` caches the queue id in-process.
- If queue initialization fails once, `newtrace` disables itself for the rest of that process lifetime.

### File Output

- `bbslogd` writes one file per day:
  - `MY_BBS_HOME/newtrace/YYYY-MM-DD.log`
- It normalizes records to end with a newline.
- The resulting storage is line-oriented plain text.
- This makes existing `newtrace` log files suitable for line-by-line parsing.

### Important Behavioral Note

- `bbslogd` is not only a passive sink.
- At least `system passerr` also feeds site-ban / repeated-failure logic inside `bbslogd`.

## Current Event Families

### Account And Session Events

These are the cleanest structured business logs found so far.

- [libythtbbs/user.c](../../../libythtbbs/user.c)
  - `system passerr %s`
    - failed login attempts by source address
    - also consumed by `bbslogd` for dynamic ban-site logic
  - `%s enter %s using %s`
    - successful login
  - `%s exitbbs %ld`
    - normal logout with stay duration
  - `system kill %s %d`
    - expired-account cleanup
- [libythtbbs/cache/utmp.c](../../../libythtbbs/cache/utmp.c)
  - `%s drop www/api`
    - stale `www` / `api` session cleanup
- [src/bbs/main.c](../../../src/bbs/main.c)
  - `%s enter %s`
    - terminal login
  - `%s exitbbs %ld`
    - terminal logout
  - `%s drop %ld`
    - abnormal disconnect or abort
  - `%s kick %s multi-login`
    - forced duplicate-login cleanup
- [src/bbs/register.c](../../../src/bbs/register.c)
  - `%s newaccount %d %s`
    - terminal registration
- [src/bbs/delete.c](../../../src/bbs/delete.c)
  - `%s kick %s`
    - administrative kick
- [nju09/www/bbsdoreg.c](../../../nju09/www/bbsdoreg.c)
  - `%s newaccount %d %s www`
    - web registration

Interpretation:

- these are strong Stage 1 candidates
- payloads are relatively structured
- they are less encoding-sensitive than content logs
- the same logical family already spans terminal, web, and library/session layers

### Board Usage And Statistics Events

These look simple, but they are important because they feed later statistics and scoring.

- [src/bbs/boards.c](../../../src/bbs/boards.c)
  - `%s use %s %ld`
    - board usage time
    - used by the board scoring algorithm
- [nju09/www/BBSLIB.c](../../../nju09/www/BBSLIB.c)
  - `%s use %s %ld`
    - web-side board usage timing
- [nju09/www/bbsfind.c](../../../nju09/www/bbsfind.c)
  - `%s bbsfind %d`
    - search result count
    - more like search/statistics tracing than core business state

Interpretation:

- `use` is business-relevant, not just descriptive telemetry
- `bbsfind` is lower-priority and may belong on the analytics side instead of the first structured migration slice

### Post, Mail, And Content-Lifecycle Events

These are important business logs, but they are much more encoding-sensitive and may duplicate each other across access paths.

- [src/bbs/bbs.c](../../../src/bbs/bbs.c)
  - `%s post %s %s`
  - `%s edit %s %s %s`
  - `%s changetitle %s %s oldtitle:%s newtitle:%s`
  - `%s del %s %s %s`
  - `%s undel %s %s %s`
  - `%s crosspost %s %s`
  - `%s thread %s`
    - also triggers the legacy `bin/thread` helper
- [src/bbs/1984.c](../../../src/bbs/1984.c)
  - `%s check1984 %s %s`
  - `%s post %s %s`
- [src/bbs/read.c](../../../src/bbs/read.c)
  - `%s sametitle %s %s`
- [src/bbs/mail.c](../../../src/bbs/mail.c)
  - `%s mail %s`
  - `%s netmail %s`
- [nju09/www/bbssnd.c](../../../nju09/www/bbssnd.c)
  - `%s post %s %s`
- [nju09/www/BBSLIB.c](../../../nju09/www/BBSLIB.c)
  - `%s mail %s`
- [api/api_article.c](../../../api/api_article.c)
  - `%s post %s %s`
- [local_utl/autoundeny/autoundeny.c](../../../local_utl/autoundeny/autoundeny.c)
  - `XJTU-XANET mail %s`
    - narrow utility-triggered station-mail side effect

Interpretation:

- this family already spans terminal, web, API, and utility paths
- titles and content-derived fields make it GBK-sensitive
- migration will need normalization and deduplication, not raw log copying

### Moderation And Administrative Content-State Events

These overlap with post lifecycle but are worth calling out because they are audit-heavy and may be useful even if not migrated in Stage 1.

- [src/bbs/bbs.c](../../../src/bbs/bbs.c)
  - `%s mark %s %s %s` / `%s unmark %s %s %s`
  - `%s water %s %s %s` / `%s unwater %s %s %s`
  - `%s digest %s %s %s` / `%s undigest %s %s %s`
  - `%s ranged %s %d %d`
  - `%s rangedmail %d %d`
  - top / un-top actions
- [src/bbs/bm.c](../../../src/bbs/bm.c)
  - `%s deny %s %s`
- [src/bbs/announce.c](../../../src/bbs/announce.c)
  - `%s %s %s %s`
  - `%s import %s %s %s`
- [nju09/www/bbsdenyadd.c](../../../nju09/www/bbsdenyadd.c)
  - `%s deny %s %s`
- [nju09/www/bbsman.c](../../../nju09/www/bbsman.c)
  - mark / unmark
  - digest / undigest

Interpretation:

- these are real business and audit events
- many carry titles or path-like payloads
- they are more likely to belong in later migration stages than the first low-risk slice

### Social, Interaction, And Niche User Events

- [src/bbs/talk.c](../../../src/bbs/talk.c)
  - `%s five %s`
  - `%s talk %s`
- [src/bbs/xyz.c](../../../src/bbs/xyz.c)
  - `%s sendgoodwish %s`
  - `%s finddf %s %d`

Interpretation:

- these are user-facing, but lower-priority than account, board-usage, and core post/mail events

### Operational, Cache, And Integration Diagnostics

These are part of the current `newtrace` pipeline, but they are not good first candidates for database-backed business logging.

- [libythtbbs/cache/board.c](../../../libythtbbs/cache/board.c)
  - `system reload bcache %d`
- [libythtbbs/cache/user.c](../../../libythtbbs/cache/user.c)
  - `system reload ucache %d`
- [libythtbbs/cache/cache-internal.h](../../../libythtbbs/cache/cache-internal.h)
  - `SHM Error! ...`
- [libythtbbs/mailsender.c](../../../libythtbbs/mailsender.c)
  - SMTP runtime status traces
- [src/bbs/comm_lists.c](../../../src/bbs/comm_lists.c)
  - `system reload sysconf.img2`
- [src/bbs/more.c](../../../src/bbs/more.c)
  - `system reload movie %d`
- [src/bbs/power_select.c](../../../src/bbs/power_select.c)
  - `%s full_search %s %s`
  - `%s select %s %d %d`
  - lower-priority tracing/debug-like logs
- [api/api_article.c](../../../api/api_article.c)
  - `.DIR` repair and fileheader write errors
- [api/api_oauth.c](../../../api/api_oauth.c)
  - 2FA generation failure
- [api/api_user.c](../../../api/api_user.c)
  - Redis activity trace
- [libbmy/search.c](../../../libbmy/search.c)
  - search helper/runtime failures
- [libbmy/wechat.c](../../../libbmy/wechat.c)
  - WeChat integration failures

Interpretation:

- these should mostly remain on the operational/debug side
- they still matter, but they should not define the schema of future business-event logging

## Encoding Notes

### Lower-Risk Logs

- account and session events
- most `use` timing logs
- some administrative cleanup events

These are mostly ASCII-structured and safer to migrate early.

### Higher-Risk Logs

- post titles
- mail and title-bearing moderation logs
- announcement actions and paths
- content-derived search terms
- old/new title pairs

These are much more likely to carry GBK-origin payloads or other legacy text. They need explicit migration rules instead of naive structured extraction.

### Parsing Caveat

- `*` appears in normal historical log content, especially in titles.
- Therefore `*` should not be interpreted as evidence of a former line break when parsing archived log files.
- For the next stage, existing `newtrace` files should still be parsed line by line, but `*` must be treated as ordinary message content.

## Migration Relevance

### Strong Early Candidates

- login success
- logout
- drop / disconnect
- failed login attempts
- duplicate-login cleanup
- account creation
- stale `www/api` session cleanup
- board usage timing

### Important, But Better Deferred

- post events
- mail events
- moderation state changes
- announcement imports and action logs
- content-derived search/statistics traces

### Likely To Remain Operational

- cache reload logs
- shared-memory errors
- SMTP runtime logs
- API file-repair traces
- OAuth / Redis integration traces
- `libbmy` runtime and third-party integration failures

## Current Assessment

- The logging backend is simple and stable, but tightly coupled to SysV IPC and daily filesystem logs.
- `newtrace` is currently overloaded: business logs, moderation audit logs, and runtime diagnostics all share the same transport and sink.
- The same logical event families already appear across terminal BBS, legacy web CGI, and the API layer.
- Some logs, especially `system passerr`, are not pure audit records because they also drive runtime behavior.
- The first migration step should separate event families, not replicate the current text stream in another storage engine.

## Open Questions

- Which event families should be preserved as structured business data in Stage 1, and which should remain file-based for now?
- Which caller groups duplicate the same real-world event across terminal, web, and API access paths?
- How much of the current `bbslogd` behavior should move into the future logging pipeline, especially the `system passerr` side effects?
