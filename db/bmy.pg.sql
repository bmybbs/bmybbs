-- BMYBBS PostgreSQL schema.
--
-- Tables and indexes for logging.
-- NOTE: Intentionally no foreign keys.

CREATE TABLE IF NOT EXISTS log_article_events (
	id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

	occurred_at TIMESTAMPTZ NOT NULL,

	actor_userid VARCHAR(12) NOT NULL,
	board VARCHAR(32) NOT NULL,
	owner_userid VARCHAR(12),
	title TEXT NOT NULL,
	old_title TEXT,

	action VARCHAR(32) NOT NULL
		CHECK (action IN (
			'post',
			'check1984',
			'crosspost',
			'sametitle',
			'edit',
			'del',
			'undel',
			'changetitle',
			'mark',
			'unmark',
			'digest',
			'undigest',
			'water',
			'unwater',
			'top',
			'untop'
		))
);

CREATE TABLE IF NOT EXISTS log_range_delete_events (
	id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

	occurred_at TIMESTAMPTZ NOT NULL,

	scope VARCHAR(16) NOT NULL
		CHECK (scope IN ('article', 'mail')),
	userid VARCHAR(12) NOT NULL,
	board VARCHAR(32),
	from_id INTEGER NOT NULL,
	to_id INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS log_board_usage_events (
	id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

	occurred_at TIMESTAMPTZ NOT NULL,

	userid VARCHAR(12) NOT NULL,
	board VARCHAR(32) NOT NULL,
	stay_seconds BIGINT NOT NULL CHECK (stay_seconds >= 0)
);

CREATE TABLE IF NOT EXISTS log_session_duration_events (
	id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

	occurred_at TIMESTAMPTZ NOT NULL,

	userid VARCHAR(12) NOT NULL,
	action VARCHAR(16) NOT NULL
		CHECK (action IN ('logout', 'disconnect')),
	stay_seconds BIGINT NOT NULL CHECK (stay_seconds >= 0)
);

CREATE TABLE IF NOT EXISTS log_login_failure_events (
	id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

	occurred_at TIMESTAMPTZ NOT NULL,

	from_host VARCHAR(64) NOT NULL
);

CREATE TABLE IF NOT EXISTS log_session_events (
	id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

	occurred_at TIMESTAMPTZ NOT NULL,

	action VARCHAR(32) NOT NULL
		CHECK (action IN (
			'login_success',
			'session_cleanup',
			'multi_login_kick',
			'user_kick'
		)),

	userid VARCHAR(12) NOT NULL,
	target_userid VARCHAR(12),
	from_host VARCHAR(64),
	login_type VARCHAR(16)
);

CREATE TABLE IF NOT EXISTS log_account_events (
	id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

	occurred_at TIMESTAMPTZ NOT NULL,

	action VARCHAR(32) NOT NULL
		CHECK (action IN ('create', 'expire_cleanup')),

	userid VARCHAR(12) NOT NULL,
	usernum INTEGER,
	life_value INTEGER,
	from_host VARCHAR(64),
	login_type VARCHAR(16),

	CHECK (
		(action = 'create' AND usernum IS NOT NULL AND life_value IS NULL)
		OR
		(action = 'expire_cleanup' AND usernum IS NULL AND life_value < 0)
	)
);

CREATE TABLE IF NOT EXISTS log_mail_events (
	id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

	occurred_at TIMESTAMPTZ NOT NULL,

	sender VARCHAR(12) NOT NULL,
	target_userid VARCHAR(80) NOT NULL
);

CREATE TABLE IF NOT EXISTS log_user_interaction_events (
	id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

	occurred_at TIMESTAMPTZ NOT NULL,

	action VARCHAR(16) NOT NULL
		CHECK (action IN ('talk', 'goodwish')),
	userid VARCHAR(12) NOT NULL,
	target_userid VARCHAR(12) NOT NULL
);

CREATE TABLE IF NOT EXISTS log_user_query_events (
	id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

	occurred_at TIMESTAMPTZ NOT NULL,

	action VARCHAR(16) NOT NULL
		CHECK (action IN ('finddf')),
	userid VARCHAR(12) NOT NULL,
	target VARCHAR(12) NOT NULL,
	day_count INTEGER NOT NULL CHECK (day_count >= 0)
);

CREATE TABLE IF NOT EXISTS log_announcement_events (
	id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

	occurred_at TIMESTAMPTZ NOT NULL,

	action VARCHAR(16) NOT NULL
		CHECK (action IN ('paste', 'moveitem', 'additem', 'import')),
	userid VARCHAR(12) NOT NULL,
	board VARCHAR(32) NOT NULL,
	path TEXT,
	owner_userid VARCHAR(12),
	title TEXT
);

CREATE TABLE IF NOT EXISTS log_board_deny_events (
	id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

	occurred_at TIMESTAMPTZ NOT NULL,

	operator_userid VARCHAR(12) NOT NULL,
	board VARCHAR(32) NOT NULL,
	target_userid VARCHAR(12) NOT NULL
);

CREATE TABLE IF NOT EXISTS log_imported_lines (
	id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

	source_file TEXT NOT NULL,
	source_line INTEGER NOT NULL CHECK (source_line > 0),
	event_table VARCHAR(64) NOT NULL,
	event_id BIGINT NOT NULL,

	UNIQUE (source_file, source_line)
);

CREATE INDEX IF NOT EXISTS idx_log_article_events_occurred_at
	ON log_article_events (occurred_at);

CREATE INDEX IF NOT EXISTS idx_log_range_delete_events_occurred_at
	ON log_range_delete_events (occurred_at);

CREATE INDEX IF NOT EXISTS idx_log_board_usage_events_occurred_at
	ON log_board_usage_events (occurred_at);

CREATE INDEX IF NOT EXISTS idx_log_session_duration_events_occurred_at
	ON log_session_duration_events (occurred_at);

CREATE INDEX IF NOT EXISTS idx_log_login_failure_events_occurred_at
	ON log_login_failure_events (occurred_at);

CREATE INDEX IF NOT EXISTS idx_log_login_failure_events_from_host_occurred_at
	ON log_login_failure_events (from_host, occurred_at);

CREATE INDEX IF NOT EXISTS idx_log_session_events_occurred_at
	ON log_session_events (occurred_at);

CREATE INDEX IF NOT EXISTS idx_log_account_events_occurred_at
	ON log_account_events (occurred_at);

CREATE INDEX IF NOT EXISTS idx_log_mail_events_occurred_at
	ON log_mail_events (occurred_at);

CREATE INDEX IF NOT EXISTS idx_log_user_interaction_events_occurred_at
	ON log_user_interaction_events (occurred_at);

CREATE INDEX IF NOT EXISTS idx_log_user_query_events_occurred_at
	ON log_user_query_events (occurred_at);

CREATE INDEX IF NOT EXISTS idx_log_announcement_events_occurred_at
	ON log_announcement_events (occurred_at);

CREATE INDEX IF NOT EXISTS idx_log_board_deny_events_occurred_at
	ON log_board_deny_events (occurred_at);
