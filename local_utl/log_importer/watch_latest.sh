#!/usr/bin/env bash
set -euo pipefail

psql -P pager=off -c "
SELECT
	imported.id,
	source.source_file,
	imported.source_line,
	imported.event_table,
	imported.event_id
FROM log_imported_lines AS imported
JOIN log_source_files AS source ON source.id = imported.source_file_id
ORDER BY imported.id DESC
LIMIT 1;
"
