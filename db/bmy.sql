--
-- tables
--

CREATE TABLE `t_users` (
	`usernum` int NOT NULL COMMENT '对应于在 PASSWDS 文件中的索引，从 1 开始计数',
	`userid` varchar(14) NOT NULL,
	PRIMARY KEY （`usernum`)
);

CREATE TABLE `t_boards` (
	`boardnum` int NOT NULL COMMENT '对应于 BOARDS 文件中的索引，从 1 开始计数',
	`boardname_en` varchar(40) NOT NULL COMMENT '对应于 ythtbbs::boardheader.filename char(24)',
	`boardname_zh` varchar(40) NOT NULL COMMENT '对应于 ythtbbs::boardheader.title char(24)使用 GBK 编码，转换为 UTF8 预计最长 36 字符'，
	PRIMARY KEY （`boardnum`)
);

CREATE TABLE `t_threads` (
	`id` int NOT NULL AUTO_INCREMENT COMMENT '用于数据库内部管理，和 BMY 数据不产生映射',
	`boardnum` int NOT NULL COMMENT '外键，用于视图中显示版面名称（版面中英文名可能发生变化）',
	`timestamp` bigint NOT NULL COMMENT '使用长整形保存 time_t 主题创建时间',
	`title` varchar(120) NULL COMMENT '对应 ythtbbs::fileheader.title char(60)，留足 UTF8 编码空间',
	`author` varchar(16) NULL COMMENT '作者，这里就不使用外键关联了，原作者 ID 可能不存在',
	`comments` int NULL DEFAULT 1 COMMENT '包含原文在内的讨论计数',
	PRIMARY KEY (`id`)，
	KEY `fk_thread_board_idx` (`boardnum`),
	KEY `idx_timestamp` (`timestamp`),
	CONSTRAINT `fk_thread_board` FOREIGN KEY (`boardnum`) REFERENCES `t_boards` (`boardnum`)
);

CREATE TABLE `t_feed_meta` (
	`id` int NOT NULL AUTO_INCREMENT,
	`usernum` int NOT NULL,
	`lastread` bigint NOT NULL DEFAULT 0,
	`latest_tid` bigint NOT NULL DEFAULT 0,
	PRIMARY KEY (`id`),
	KEY `fk_feedmeta_user_idx` (`usernum`),
	CONSTRAINT `fk_feedmeta_user` FOREIGN KEY (`usernum`) REFERENCES `t_users` (`usernum`) ON DELETE CASCADE
);

CREATE TABLE `t_user_subscriptions` (
	`id` int NOT NULL AUTO_INCREMENT,
	`usernum` int NOT NULL,
	`boardnum` int NOT NULL,
	`lastread` bigint DEFAULT '0',
	`latest_tid` bigint DEFAULT '0',
	PRIMARY KEY (`id`),
	KEY `fk_sub_user_idx` (`usernum`),
	KEY `fk_sub_board_idx` (`boardnum`),
	CONSTRAINT `fk_sub_board` FOREIGN KEY (`boardnum`) REFERENCES `t_boards` (`boardnum`),
	CONSTRAINT `fk_sub_user` FOREIGN KEY (`usernum`) REFERENCES `t_users` (`usernum`) ON DELETE CASCADE
);

