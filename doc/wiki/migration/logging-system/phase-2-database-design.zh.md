# 日志系统第二阶段：数据库设计

> 状态：初稿

## 摘要

本页面记录了将遗留 `newtrace` 日志导入 PostgreSQL 的数据库设计。

可执行模式位于 [db/bmy.pg.sql](../../../../db/bmy.pg.sql)。本页面说明了表存在的原因、涵盖了哪些 API，以及哪些日志被排除在首次数据库导入之外。

## 设计原则

- 为稳定的事件家族使用类别表。
- 保持导入追踪与事件表分离。
- 不要在日志表中使用外键。
- 保留历史文本值，即使以后删除或重命名了用户、版面、邮件或文章。
- 保留已知的遗留局限性，而不是虚构缺失的标识符。
- 在真实的查询模式证明其必要性之前，保持索引最小化。

## 丢弃的 API

这些 API 被排除在首次数据库导入之外。其中一些是运行时诊断信息，在运维上可能仍有意义，但它们应由未来的服务日志处理，而不是业务事件表。

- `bmy_log_search_result_count`
- `bmy_log_cache_reload`
- `bmy_log_system_reload`
- `bmy_log_runtime_error`
- `bmy_log_thread_view`
- `bmy_log_search_trace`
- `bmy_log_selection_trace`

## 类别表

### `log_article_events`

版面上的类文章操作。

相关 API：

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

动作：

- 创建/预创建：`post`、`check1984`、`crosspost`、`sametitle`
- 涉及所有者的变更：`edit`、`del`、`undel`
- 标题变更：`changetitle`
- 标记类变更：`mark`、`unmark`、`digest`、`undigest`、`water`、`unwater`、`top`、`untop`

重要字段：

- `actor_userid`: 操作员
- `board`: 版面名称
- `owner_userid`: 文章所有者（如果可用）
- `title`: 当前标题，或 `changetitle` 的新标题
- `old_title`: `changetitle` 的旧标题
- `action`: 规范化的动作标识

遗留局限：

- 这些日志仅通过版面和标题识别文章活动。
- 它们不包含唯一准确定位特定文章所需的文章时间戳/ID。

### `log_range_delete_events`

文章列表和邮件列表的范围删除操作。

相关 API：

- `bmy_log_post_range_delete`
- `bmy_log_mail_range_delete`

重要字段：

- `scope`: `article`（文章）或 `mail`（邮件）
- `userid`: 操作员
- `board`: 文章范围删除的相关版面名称
- `from_id`, `to_id`: 遗留的数字范围

### `log_board_usage_events`

版面停留时间统计。

相关 API：

- `bmy_log_board_use`

重要字段：

- `userid`: 访问者
- `board`: 版面名称
- `stay_seconds`: 记录的版面停留时长

该表是独立的，因为 `stay_seconds` 对版面统计和计分很有用。

### `log_session_duration_events`

带有停留时长的会话结束事件。

相关 API：

- `bmy_log_logout`
- `bmy_log_disconnect`

重要字段：

- `userid`: 会话结束的用户
- `action`: `logout`（退出）或 `disconnect`（断开）
- `stay_seconds`: 记录的会话时长

### `log_login_failure_events`

按来源主机记录的登录失败尝试。

相关 API：

- `bmy_log_login_failure`

重要字段：

- `from_host`: IPv4 或 IPv6 来源字符串

该表很重要，因为站点封禁检查依赖于来自来源主机的重复登录失败。

### `log_session_events`

不带停留时长的会话相关事件。

相关 API：

- `bmy_log_login_success`
- `bmy_log_session_cleanup`
- `bmy_log_multi_login_kick`
- `bmy_log_user_kick`

重要字段：

- `action`: `login_success`、`session_cleanup`、`multi_login_kick` 或 `user_kick`
- `userid`: 事件的主要用户 ID
- `target_userid`: 目标用户（如果适用）
- `from_host`: 登录成功的来源主机
- `login_type`: 访问类型（如果已知）

### `log_account_events`

账号生命周期事件。

相关 API：

- `bmy_log_account_create`
- `bmy_log_account_expire_cleanup`

重要字段：

- `action`: `create`（创建）或 `expire_cleanup`（过期清理）
- `userid`: 账号用户 ID
- `usernum`: 遗留的用户编号（如果可用）
- `from_host`: 账号创建的来源主机
- `login_type`: 创建路径（如果已知）

### `log_mail_events`

邮件发送事件。

相关 API：

- `bmy_log_mail_send`
- `bmy_log_netmail_send`
- `bmy_log_utility_mail_send`

重要字段：

- `sender`: 历史发送者 ID 或工具发送者名称
- `target_userid`: 历史目标，比普通用户 ID 更宽泛，因为极少数 `netmail` 目标可能是电子邮件地址

在历史日志中 `netmail` 很少见，如果需要，以后可以通过类似电子邮件的目标来识别。

### `log_user_interaction_events`

轻量级的用户间互动事件。

相关 API：

- `bmy_log_talk_request`
- `bmy_log_send_goodwish`

重要字段：

- `action`: `talk`（交谈）或 `goodwish`（送祝福）
- `userid`: 发起者
- `target_userid`: 目标用户

### `log_user_query_events`

有用的用户查询/调试动作。

相关 API：

- `bmy_log_finddf`

重要字段：

- `action`: 目前仅为 `finddf`
- `userid`: 执行查询的用户
- `target`: 被查询的用户
- `day_count`: 请求的时间段

### `log_announcement_events`

公告区操作。

相关 API：

- `bmy_log_announce_action`
- `bmy_log_announce_import`

重要字段：

- `action`: `paste`（粘贴）、`moveitem`（移动项）、`additem`（添加项）或 `import`（导入）
- `userid`: 操作员
- `board`: 相关版面
- `path`: 普通操作的公告路径
- `owner_userid`, `title`: 导入事件的被导入文章元数据

### `log_board_deny_events`

版面级别的封禁 (deny) 日志。

相关 API：

- `bmy_log_board_deny`

重要字段：

- `operator_userid`: 添加封禁项的用户
- `board`: 应用封禁的版面
- `target_userid`: 被封禁的用户

## 导入追踪表

### `log_imported_lines`

该表记录了物理来源行是否已经产生了一个已导入的事件行。

重要字段：

- `source_file`: 规范的日志文件名，通常为 `YYYY-MM-DD.log`
- `source_line`: 来源文件中的物理行号
- `event_table`: 目标类别表名称
- `event_id`: 目标类别表中的行 ID

`(source_file, source_line)` 上的唯一约束是导入幂等性的边界。

## 索引设计

初始 SQL 模式使用最小化的二级索引。

- 为每个类别表添加一个 `occurred_at` 索引。
- 原因：历史日志检查预计会从时间范围开始。
- 为 `log_login_failure_events(from_host, occurred_at)` 添加索引，因为站点封禁检查通常在时间范围内按来源主机进行已知查询。
- 推迟其他用户、版面和主机的索引，直到真实的查询模式出现。
- `log_imported_lines(source_file, source_line)` 因其唯一约束而已经拥有唯一索引。

可能推迟的索引包括：用户 ID 加时间，以及版面加时间。

## SQL 资产

当前的可执行模式位于 [db/bmy.pg.sql](../../../../db/bmy.pg.sql)。

当该设计发生变更时，请同时更新本页面和对应的 SQL 文件。
