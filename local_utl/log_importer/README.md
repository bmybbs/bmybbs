# Log Importer

`log_importer` 每次导入 `newtrace` 目录下的单个日志文件。

在验证历史日志格式的阶段可以使用 dry-run 模式：

```bash
/path/to/build/local_utl/log_importer/log_importer --dry-run YYYY-MM-DD
```

Dry-run 模式会读取 `$HOME/newtrace/YYYY-MM-DD.log`，但并不会连接到 PostgreSQL 数据库，也不会插入任何记录。最后总结会输出到 stdout。未识别的或者失败的日志行号会被输出到 stderr。

## Dry Run 一整年

导入程序被有意设计为单次调用仅处理某一天的日志。以下脚本可以批量 dry-run 一整年的日志，并将未识别的或者处理失败的日志收集存放。如果某一天的日志不存在会被自动略过。

```bash
#!/usr/bin/env bash
set -u

year=${1:?Usage: $0 YEAR}
importer=${IMPORTER:-/path/to/build/local_utl/log_importer/log_importer}
issues="./log-importer-${year}.issues.txt"

: > "$issues"

for file in "$HOME"/newtrace/"$year"-??-??.log; do
	[[ -e "$file" ]] || continue
	date=${file##*/}
	date=${date%.log}
	"$importer" --dry-run "$date" >/dev/null 2>>"$issues"
done

printf 'issues: %s\n' "$issues"
```

示例：

```bash
bash ./dry-run-year.sh 2016
```

## 导入某一个月的日志

实际的导入过程中涉及数据库写入，因此速度会慢很多。以下的脚本仅导入单个月的日志，并显示当前处理的文件。`--fast-import` 会在日志文件尚未导入时使用整文件事务；如果文件已经导入过，会回退到逐行检查模式。

```bash
#!/usr/bin/env bash
set -u

month=${1:?Usage: $0 YYYY-MM}
importer=${IMPORTER:-/path/to/build/local_utl/log_importer/log_importer}
issues="./log-importer-${month}.import-issues.txt"

: > "$issues"

for file in "$HOME"/newtrace/"$month"-??.log; do
	[[ -e "$file" ]] || continue
	date=${file##*/}
	date=${date%.log}
	printf '[%s] importing: %s\n' "$(date '+%F %T')" "$file"
	"$importer" --fast-import "$date" >/dev/null 2>>"$issues"
done

printf 'issues: %s\n' "$issues"
```

示例：

```bash
bash ./import-month.sh 2016-01
```

## 其他辅助脚本

配合 `watch(1)` 监视数据库的变化。

- [watch_sizes.sh](./watch_sizes.sh) 检查当前数据库规模
- [watch_counts.sh](./watch_counts.sh) 检查当前各个表的记录数
- [watch_latest.sh](./watch_latest.sh) 查看最新导入的行
