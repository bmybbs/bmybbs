# IP 00-0001: 日志导入器

> 状态：初稿
> 模块：日志系统
> 相关设计：../migration/logging-system/phase-2-importer.zh.md
> 相关用例：../use-cases/00-0001-import-one-daily-log.zh.md

## 目标

实现一个单日历史日志导入器，读取一个遗留的 `newtrace` 文件，并将已接受的事件写入 PostgreSQL 类别表中。

## 范围

- 提供一个命令行工具，用于导入单个每日日志文件。
- 支持不写入数据库的试运行 (dry-run) 解析。
- 从日期参数解析来源文件。
- 使用 `getline(3)` 逐行读取来源文件。
- 在解析每一行之前检查 `log_imported_lines(source_file, source_line)`。
- 使用来自 [00-0002-logging-importer-parser.zh.md](./00-0002-logging-importer-parser.zh.md) 的解析器组件。
- 在同一个事务中插入已接受的事件及其导入追踪行。
- 打印包含稳定计数器的摘要。

## 非目标

- 不在一个进程中批量导入多个日期的日志。
- 不将实时的生产日志直接写入 PostgreSQL。
- 不将已丢弃的运行时诊断 API 导入业务事件表。
- 不在 PostgreSQL 中存储跳过行的元数据。
- 不更改遗留的 `newtrace` 文件。
- 本计划不实现解析器内部逻辑。
- 初次实现不使用 `mmapfile` / `mmap(3)`。
- 初次实现不包含可配置的时区处理。

## 输入与输出

输入：

- 必需的日期参数：`YYYY-MM-DD`
- 针对遗留日志的固定 UTC+8 时区处理
- `$HOME/newtrace/YYYY-MM-DD.log`
- PostgreSQL 默认本地连接

输出：

- 由 [../migration/logging-system/phase-2-database-design.zh.md](../migration/logging-system/phase-2-database-design.zh.md) 定义的日志类别表中的行
- `log_imported_lines` 中的行
- 打印到 stdout 或 stderr 的易读摘要
- 进程退出码

试运行 (Dry-run) 输出：

- 仅包含易读摘要。
- 不需要 PostgreSQL 连接。
- 不执行 `log_imported_lines` 查找。
- 不插入类别表行或导入追踪行。

## 文件与模块

预计位置：

- `local_utl/log_importer/`

预计文件：

- `local_utl/log_importer/CMakeLists.txt`
- `local_utl/log_importer/main.c`
- `local_utl/log_importer/importer.c`
- `local_utl/log_importer/importer.h`
- `local_utl/log_importer/db.c`
- `local_utl/log_importer/db.h`
- `include/bmy/pg_wrapper.h`
- `libbmy/pg_wrapper.c`
- 由 [00-0002-logging-importer-parser.zh.md](./00-0002-logging-importer-parser.zh.md) 拥有的解析器文件

所有权边界：

- 本计划拥有 CLI、文件解析、文件读取、数据库写入、事务处理、摘要输出以及退出行为。
- `main.c` 拥有可执行文件入口点和面向用户的退出码。
- `importer.c` 拥有命令执行、文件读取、计数器以及解析器集成。
- `db.c` 拥有 PostgreSQL 连接、幂等性查找、事件插入以及导入追踪插入。
- 解析器计划拥有行分类以及从行文本到事件记录的转换。

实现语言：

- 使用 C。
- 使用 libpq 进行 PostgreSQL 访问。

## 命令行形态

初始命令：

```text
log_importer [--dry-run] YYYY-MM-DD
```

规则：

- `YYYY-MM-DD` 是必需的。
- `--dry-run` 是可选的。
- 遗留日志时间戳解释为 UTC+8。
- 解析后的日志文件为 `$HOME/newtrace/YYYY-MM-DD.log`。
- 在 `log_imported_lines.source_file` 中仅存储 `YYYY-MM-DD.log`。

数据库连接：

- 使用 libpq 默认连接行为，例如 `PQconnectdb("")`。
- 不在源代码中硬编码数据库名称。
- 预期的部署方式是使用本地 Unix 域套接字，以当前系统用户名作为 PostgreSQL 用户，并使用默认数据库名称。

## 数据流

1. 解析命令行参数。
2. 验证日期参数。
3. 解析来源文件名和完整来源路径。
4. 打开来源文件。
5. 初始化摘要计数器。
6. 使用 `getline(3)` 读取每一条物理行，并递增来源行号。
7. 检查 `log_imported_lines(source_file, source_line)`。
8. 如果已经导入过，递增 `already_imported` 并继续。
9. 通过解析器组件解析该行。
10. 如果解析结果为已丢弃、未识别或解析失败，递增匹配的计数器并继续。
11. 根据日期、行内时间以及固定的 UTC+8 时区重建 `occurred_at`。
12. 将事件行插入匹配的类别表中。
13. 插入匹配的 `log_imported_lines` 行。
14. 提交该导入事件的事务。
15. 打印最终摘要。

## 内部接口

实现应使用 C 和 libpq。

导入器配置：

```c
struct bmy_log_importer_config {
        const char *date_arg;        /* YYYY-MM-DD */
        const char *home_dir;
        const char *source_file;     /* YYYY-MM-DD.log */
        const char *source_path;     /* $HOME/newtrace/YYYY-MM-DD.log */
        bool dry_run;
};
```

摘要计数器：

```c
struct bmy_log_import_summary {
        unsigned long total_lines;
        unsigned long inserted;
        unsigned long already_imported;
        unsigned long discarded;
        unsigned long unrecognized;
        unsigned long failed;
};
```

主要导入器函数：

```c
int bmy_log_importer_parse_args(
        int argc,
        char **argv,
        struct bmy_log_importer_config *config);

void bmy_log_importer_config_cleanup(
        struct bmy_log_importer_config *config);

int bmy_log_importer_run(
        const struct bmy_log_importer_config *config,
        struct bmy_log_import_summary *summary);
```

数据库边界：

```c
PGconn *bmy_log_importer_db_connect(void);

int bmy_log_importer_is_line_imported(
        PGconn *conn,
        const char *source_file,
        unsigned long source_line,
        bool *imported);

bool bmy_log_importer_insert_event(
        PGconn *conn,
        const char *source_file,
        unsigned long source_line,
        const char *occurred_at,
        const struct bmy_log_parse_result *result);
```

接口规则：

- `bmy_log_importer_is_line_imported` 通过其返回值和输出参数区分查找失败与已导入/未导入的结果。
- `bmy_log_importer_insert_event` 拥有同时插入类别表行和 `log_imported_lines` 行的事务。
- 导入器外壳不应检查解析器内部细节，仅关注状态、行内时间、目标表以及类型化的负载字段。
- 解析器不应知道 PostgreSQL 或 `log_imported_lines`。
- 试运行 (Dry-run) 模式应调用解析器并更新计数器，但绝不能调用数据库助手。

## 事务策略

- 类别表插入和 `log_imported_lines` 插入必须是原子的。
- 如果任一插入失败，两行都不应保持已提交状态。
- 已导入、已丢弃、未识别以及解析失败的行不需要数据库写入。
- 试运行模式不开启数据库事务。

初步建议：

- 每条导入的事件行使用一个事务。
- 这为历史导入保持了简单的错误隔离。
- 如果导入速度成为实际问题，以后可以考虑分批优化。

## 摘要计数器

导入器应至少报告：

- `total_lines`（总行数）
- `inserted`（已插入）
- `already_imported`（已导入）
- `discarded`（已丢弃）
- `unrecognized`（未识别）
- `failed`（失败）
- `dry_run`（试运行）

可选计数器：

- 每个表的插入计数
- 第一个失败行的行号
- 第一个未识别行的行号

摘要格式：

- 易读文本即可。
- 初次实现不需要 JSON 输出。

## 实现状态

- 已实现：CLI 参数解析、文件解析、试运行行为、来源文件读取、计数器、解析器集成、幂等性查找、类别表插入、导入追踪插入以及每事件事务。
- PostgreSQL 访问使用了 `libbmy` 中的精简共享封装器；导入器特定的 SQL 和错误报告保留在 `local_utl/log_importer` 中。
- 构建和运行时验证在测试环境中仍待进行。

## 错误处理

- 来源文件缺失：报告并以非零状态退出，不插入任何行。
- 无效的日期参数：报告并以非零状态退出，不读取文件。
- 数据库连接失败：报告并以非零状态退出。
- 试运行模式：不连接 PostgreSQL 且不检查导入状态。
- 已导入的行：计数并继续。
- 已丢弃的行：计数并继续。
- 未识别的行：计数并继续。
- 解码或解析失败：计数并继续，除非失败导致无法安全处理文件。
- 单条已接受行的插入失败：回滚该行，报告失败的来源行，并立即以非零退出码停止导入器。

## 验证

- 解析器行为应首先通过 [00-0002-logging-importer-parser.zh.md](./00-0002-logging-importer-parser.zh.md) 拥有的测试进行验证。
- 导入包含至少一条已接受行的测试样本文件。
- 确认已接受的事件出现在类别表中。
- 确认 `log_imported_lines.source_file` 存储的是 `YYYY-MM-DD.log` 而非完整路径。
- 确认每一条插入的事件都有匹配的 `log_imported_lines` 行。
- 对同一日期运行两次导入器，确认没有重复行。
- 导入包含已丢弃和未识别行的样本文件，确认它们被计数但未被插入。
- 使用缺失日期的文件运行，确认非零退出且无插入。
- 使用 `--dry-run` 运行，确认没有 PostgreSQL 行被插入。
- 针对旧的历史日志运行试运行模式，收集未识别/失败的示例用于解析器测试。

## 工作分解

- 已实现：工具结构、CMake 集成、本地解析器测试目标、命令行解析、固定 UTC+8 来源解析、试运行行为、PostgreSQL 连接设置、`getline(3)` 文件处理、幂等性查找、解析器集成、插入分发、导入追踪、事务、摘要输出以及退出处理。
- 待办：构建验证、历史试运行检查、样本数据库导入以及在测试环境中的幂等性验证。

## 积压工作

- 如果以后需要导入来自另一个时区的日志，请添加可配置的时区处理。
- 根据旧历史日志中试运行发现的情况，添加解析器测试用例。
