--
-- 这里是用于测试数据库脚本的数据
-- 不要在正式环境调用
--

CALL procedure_insert_board(1, "SYSOP", "站长工作室", 0);
CALL procedure_insert_board(2, "XJTUnews", "说说咱交大", 1);
CALL procedure_insert_board(3, "doctor", "博士", 1);

CALL procedure_insert_user(1, "sysop");
CALL procedure_insert_user(2, "foo");
CALL procedure_insert_user(3, "bar");
CALL procedure_insert_user(4, "baz");

INSERT INTO `t_user_subscriptions`
	(`usernum`, `boardnum`)
VALUES
	(1, 1),
	(1, 2),
	(2, 3),
	(3, 1),
	(3, 3),
	(4, 2),
	(4, 3);

INSERT INTO `t_threads`
	(`boardnum`, `timestamp`, `title`, `author`, `comments`)
VALUES
	(1, 1, "post 1", "a", 12),
	(1, 2, "post 2", "a", 12),
	(2, 3, "post 3", "a", 12),
	(3, 3, "post 4", "a", 12),
	(1, 5, "post 5", "a", 12),
	(2, 7, "post 6", "a", 12);

