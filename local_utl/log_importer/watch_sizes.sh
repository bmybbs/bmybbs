#!/usr/bin/env bash
set -euo pipefail

psql -P pager=off -c "
SELECT
	relname AS table_name,
	pg_size_pretty(pg_total_relation_size(relid)) AS total_size,
	pg_size_pretty(pg_relation_size(relid)) AS table_size,
	pg_size_pretty(pg_indexes_size(relid)) AS index_size
FROM pg_catalog.pg_statio_user_tables
WHERE relname LIKE 'log_%'
ORDER BY pg_total_relation_size(relid) DESC;
"
