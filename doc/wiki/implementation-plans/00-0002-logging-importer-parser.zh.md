# IP 00-0002: 日志导入器解析器

> 状态：初稿
> 模块：日志系统
> 相关设计：../migration/logging-system/phase-2-importer.zh.md
> 相关用例：../use-cases/00-0001-import-one-daily-log.zh.md

## 目标

实现解析器组件，用于对单条遗留的 `newtrace` 日志行进行分类，并将已接受的行转换为类型化的事件记录，供日志导入器使用。

解析器的第一个版本基于当前的日志封装器和现状文档。较旧的历史日志可能会使用更早的格式，应通过试运行 (dry-run) 导入扫描来发现这些格式。

## 范围

- 一次解析一条物理日志行。
- 提取行内时间。
- 将行分类为：已接受 (accepted)、已丢弃 (discarded)、未识别 (unrecognized) 或失败 (failed)。
- 将已接受的事件家族映射到目标类别表和列值。
- 使用 `g2u` 将遗留的自由文本字段从 GBK 转换为 UTF-8。
- 保留 `*` 作为普通内容。

## 非目标

- 不打开文件。
- 不连接 PostgreSQL。
- 不插入数据库行。
- 不检查 `log_imported_lines`。
- 不决定事务边界。
- 不存储跳过行的元数据。

## 输入与输出

输入：

- 来自每日 `newtrace` 文件的一条物理日志行。

输出：

- 解析后的行内时间。
- 解析器状态：已接受、已丢弃、未识别或失败。
- 对于已接受的行，包含目标表名和类型化的字段值。
- 对于已丢弃、未识别或失败的行，包含足够的理由文本用于摘要/调试输出。

## 文件与模块

预计的解析器文件：

- `local_utl/log_importer/log_parser.c`
- `local_utl/log_importer/log_parser.h`
- `local_utl/log_importer/log_tokenizer.c`
- `local_utl/log_importer/log_tokenizer.h`
- `local_utl/log_importer/test_log_parser.c`
- `local_utl/log_importer/test_log_tokenizer.c`

[00-0001-logging-importer.zh.md](./00-0001-logging-importer.zh.md) 中的导入器外壳负责调用解析器并将解析结果写入 PostgreSQL。

测试位置：

- 解析器单元测试应位于同一个 `local_utl/log_importer` 目录下。
- 使用 `libcheck` 作为第一个解析器测试可执行文件。
- 保持解析器测试靠近该工具，而不是过早地将解析器代码移动到共享库中。

## 行格式

预期的物理行形状：

```text
HH:MM:SS 消息内容
```

规则：

- `HH:MM:SS` 被解析为行内本地时间。
- 导入器提供日期和时区。
- 解析器接收或返回足够的数据，以便导入器重建 `occurred_at`。
- `*` 是普通内容，绝不能被视为换行标记。

## 解析器结果形状

解析器应返回以下之一：

- 已接受的事件
- 已丢弃的行
- 未识别的行
- 失败的行

已接受的事件应包括：

- 目标表标识符
- 动作 (action) 或范围 (scope) 值（如果适用）
- 目标表所需的类型化字符串/整数基础字段
- 已解码的 UTF-8 文本字段
- 已经转换为整数的类型化数字字段

失败的行应包括：

- 失败原因
- 该行是否应计为“失败”而非“未识别”

## 内部接口

解析器接口应使行分类明确化，并将数据库工作保持在解析器之外。

状态枚举：

```c
enum bmy_log_parse_status {
        BMY_LOG_PARSE_UNSET,
        BMY_LOG_PARSE_ACCEPTED,
        BMY_LOG_PARSE_DISCARDED,
        BMY_LOG_PARSE_UNRECOGNIZED,
        BMY_LOG_PARSE_FAILED,
};
```

目标表枚举：

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

行内时间：

```c
struct bmy_log_line_time {
        int hour;
        int minute;
        int second;
};
```

通用解析结果：

```c
struct bmy_log_parse_result {
        enum bmy_log_parse_status status;
        struct bmy_log_line_time line_time;
        enum bmy_log_event_table table;
        union bmy_log_event_payload payload;
        const char *reason;
};
```

表示规则：

- 使用类型化的联合体 (union) 负载方法。
- 每个目标表都有一个对应的负载结构体。
- 导入器外壳根据 `table` 进行分发并消费匹配的负载结构体。
- 数字字段应由解析器解析，以便数据库插入代码不需要再次解析字符串。

事件负载联合体：

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

代表性的负载结构体：

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

解析器入口点：

```c
bool bmy_log_parse_line(
        const char *line,
        struct bmy_log_parse_result *result);

void bmy_log_parse_result_cleanup(
        struct bmy_log_parse_result *result);
```

所有权规则：

- 解析器结果应在下一次解析调用或显式清理调用之前保持有效。
- 如果实现分配了已解码的 UTF-8 字符串，则必须提供一个清理函数，如 `bmy_log_parse_result_cleanup`。
- 导入器外壳绝不能直接释放负载字段，除非解析器接口明确转移了所有权。

## 已接受的事件家族

解析器应支持来自 [../migration/logging-system/phase-2-database-design.zh.md](../migration/logging-system/phase-2-database-design.zh.md) 的所有已接受事件家族：

- 文章事件
- 范围删除事件
- 版面使用事件
- 会话时长事件
- 登录失败事件
- 会话事件
- 账号事件
- 邮件事件
- 用户互动事件
- 用户查询事件
- 公告事件
- 版面封禁事件

## 丢弃的 API

解析器应识别已丢弃的 API 行，以便它们可以与未识别的行分开计数。

丢弃的家族：

- `bmy_log_search_result_count`
- `bmy_log_cache_reload`
- `bmy_log_system_reload`
- `bmy_log_runtime_error`
- `bmy_log_thread_view`
- `bmy_log_search_trace`
- `bmy_log_selection_trace`

丢弃的行不是错误，不应产生数据库事件。

## 匹配策略

- 首先解析 `HH:MM:SS` 前缀。
- 将通用的消息形状视为：主语 (subject)、谓语 (verb) 以及剩余参数。
- 在首次实现中使用手动分词 (tokenization)，而不是 `sscanf` 或正则表达式。
- 尽可能使用谓语 (verb) 作为主要的分类器。
- 在匹配广泛的用户消息模式之前，先匹配具有辨识度的系统消息。
- 在匹配可能同时满足的较短模式之前，先匹配更长或更具体的模式。
- 在回退到“未识别”之前，先对已知的丢弃模式进行分类。
- 将看起来像已接受但格式错误的行视为“失败”，而非“未识别”。
- 将旧的不支持格式视为“未识别”，除非它们清晰地匹配一个已知已接受家族但字段格式错误。

需要仔细处理的歧义示例：

- `kick multi-login` 与普通的 `kick`
- `drop www/api` 与带时长的 `drop`
- 文章 `ranged` 与邮件 `rangedmail`
- 普通邮件与工具邮件

## 文本解码

- 在返回已接受的事件字段之前，将遗留的标题/路径文本从 GBK 转换为 UTF-8。
- 将源自 GBK 的文本视为预期情况。
- 使用 `libbmy/convcode.h` 中的 `g2u` 进行转换。
- 对所有遗留自由文本字段运行 `g2u`；纯 ASCII 输入将保持不变。
- 已知的自由文本字段包括文章标题以及公告路径/标题。
- 初次实现不添加单独的编码检测。
- 如果 `g2u` 失败，返回“行解析失败”状态。
- 不在解析器输出中保留原始字节；原始文件仍作为原始备份。

## 实现状态

- 解析器及其分词助手已在 `local_utl/log_importer` 下实现。
- 已接受的事件负载涵盖了所有已设计的类别表。
- 已知的丢弃日志家族在不进行数据库插入的情况下完成了分类。
- 解析器和分词器的测试源码已存在；测试环境验证和历史试运行发现仍待进行。

## 数据流

1. 接收一条物理行。
2. 解析并验证时间前缀。
3. 将消息体与时间前缀分离。
4. 将消息体与已接受和已丢弃的模式进行匹配。
5. 当匹配的事件家族需要时，使用 `g2u` 转换自由文本字段。
6. 向导入器外壳返回解析器结果。

## 错误处理

- 缺失或格式错误的时间前缀：行解析失败。
- 已知的丢弃模式：行已丢弃。
- 无已知模式：行未识别。
- 看起来像已接受的模式，但字段计数无效或数字无效：行解析失败。
- `g2u` 转换失败：行解析失败。
- 失败输出不应包含原始消息体；导入器可以报告来源文件名和行号。

## 验证

- 在将解析器接入导入器外壳之前编写解析器测试。
- 为每个已接受的事件家族提供样本行。
- 在可行的情况下，为每个已丢弃的家族提供样本行。
- 提供看起来像已接受但格式错误的行。
- 提供未识别的行。
- 将解析器测试作为具体样本覆盖率的来源。
- 利用从旧历史日志中试运行发现的情况添加解析器回归测试。
- 确认解析器输出映射到预期的目标表和字段值。
- 确认类型化的数字负载字段已经是整数。
- 确认 `*` 在解析后的文本字段中仍作为普通内容保留。

## 工作分解

- 定义解析器结果结构体/枚举。
- 创建包含第一批样本行测试的 `test_log_parser.c`。
- 实现时间前缀解析。
- 实现手动的“主语/谓语/剩余”分词。
- 实现已接受的事件家族解析器。
- 实现已丢弃模式的识别。
- 实现基于 `g2u` 的文本转换助手。
- 实现解析器样本验证。
- 针对真实的遗留示例评审匹配顺序。

## 积压工作

- 当发现新的解析器缺口时，添加更多真实的业务样本行。
- 如果后期的事件家族在文章标题和公告路径/标题之外包含源自 GBK 的文本，请重新评审自由文本字段的覆盖范围。
