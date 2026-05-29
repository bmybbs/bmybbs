#!/usr/bin/env bash
set -euo pipefail

psql -P pager=off -c "
SELECT 'log_imported_lines' AS table_name, count(*) AS rows FROM log_imported_lines
UNION ALL SELECT 'log_source_files', count(*) FROM log_source_files
UNION ALL SELECT 'log_article_events', count(*) FROM log_article_events
UNION ALL SELECT 'log_range_delete_events', count(*) FROM log_range_delete_events
UNION ALL SELECT 'log_board_usage_events', count(*) FROM log_board_usage_events
UNION ALL SELECT 'log_session_duration_events', count(*) FROM log_session_duration_events
UNION ALL SELECT 'log_login_failure_events', count(*) FROM log_login_failure_events
UNION ALL SELECT 'log_security_events', count(*) FROM log_security_events
UNION ALL SELECT 'log_session_events', count(*) FROM log_session_events
UNION ALL SELECT 'log_account_events', count(*) FROM log_account_events
UNION ALL SELECT 'log_mail_events', count(*) FROM log_mail_events
UNION ALL SELECT 'log_user_interaction_events', count(*) FROM log_user_interaction_events
UNION ALL SELECT 'log_user_query_events', count(*) FROM log_user_query_events
UNION ALL SELECT 'log_announcement_events', count(*) FROM log_announcement_events
UNION ALL SELECT 'log_board_deny_events', count(*) FROM log_board_deny_events
ORDER BY table_name;
"
