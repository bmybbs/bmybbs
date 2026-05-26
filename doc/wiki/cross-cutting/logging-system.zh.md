# 日志系统

> 状态：初稿

## 摘要

活跃的遗留日志路径以 `newtrace` 为核心：调用者将文本消息传递给 `newtrace`，`newtrace` 将其规范化为单行日志记录，通过 SysV 消息队列发送，然后 `bbslogd` 将其追加到 `newtrace/` 下的每日文件中。这条路径虽然简单且为人熟知，但它依赖于特定于操作系统的 IPC、一个独立的守护进程以及非结构化的文本文件。

经过核实的调用者显示，`newtrace` 同时被用于三类不同的用途：

- 业务与安全事件
- 仲裁与内容操作的审计追踪
- 运行时、缓存与集成诊断

这种混合使用是迁移设计中需要考虑的主要事实。未来的系统不应按模块逐个迁移当前的日志文件，而应首先分离事件家族 (event families)。

## 后端流程

### 组件

- `newtrace` 在 [include/ytht/msg.h](../../../include/ytht/msg.h) 中声明，并在 [libytht/newtrace.c](../../../libytht/newtrace.c) 中实现。
- `bbslogd` 在 [local_utl/bbslogd/bbslogd.c](../../../local_utl/bbslogd/bbslogd.c) 中消费同一个队列。

### 消息形态

- `newtrace` 构建的一行格式如下：
  - `HH:MM:SS`
  - 一个空格
  - 调用者提供的文本
  - 结尾换行符
- 调用者文本中嵌入的换行符会被替换为 `*`，因此每条 `newtrace` 记录都被规范化为单行。
- 通过一个小型本地实验也确认了这一行为：
  - `newtrace("single line")`
  - `newtrace("line 1\nline 2")`
  - 产生的结果：
    - `HH:MM:SS single line`
    - `HH:MM:SS line 1*line 2`
- 消息通过 `msgsnd(..., IPC_NOWAIT | MSG_NOERROR)` 发送。

### 队列与失败行为

- 发送端和接收端都使用 `BBSLOG_MSQKEY`。
- 双方都将队列字节限制设置为大约 `50 KiB`。
- `newtrace` 在进程内缓存队列 ID。
- 如果队列初始化失败一次，`newtrace` 会在该进程的剩余生命周期内禁用自身。

### 文件输出

- `bbslogd` 每天写入一个文件：
  - `MY_BBS_HOME/newtrace/YYYY-MM-DD.log`
- 它将记录规范化为以换行符结尾。
- 由此产生的存储是面向行的纯文本。
- 这使得现有的 `newtrace` 日志文件适合逐行解析。

### 重要行为说明

- `bbslogd` 不仅仅是一个被动的接收器。
- 至少 `system passerr` 还会驱动 `bbslogd` 内部的站点封禁/重复失败逻辑。

## 第一阶段重构状态

当前代码已为第一阶段引入了一个语义化日志层：

- 公共接口位于 [include/bmy/logging.h](../../../include/bmy/logging.h)
- 当前实现位于 [libbmy/logging.c](../../../libbmy/logging.c)
- 子系统代码应调用 `bmy_log_*` 接口，而非直接调用 `newtrace`

这仍然是一个保持存储方式不变的重构。新的 `bmy_log_*` 函数会格式化兼容遗留系统的文本，然后调用私有的 `newtrace` 后端。运行时路径保持不变：

- 子系统代码
- `bmy_log_*`
- `newtrace`
- SysV 消息队列
- `bbslogd`
- `newtrace/` 下的每日文件

在本次重构之后，直接对 `newtrace` 的引用应仅限于：

- [libbmy/logging.c](../../../libbmy/logging.c)，作为 `bmy_log_*` 背后的兼容性实现
- [libytht/newtrace.c](../../../libytht/newtrace.c)，作为底层后端
- 已失效或被注释的遗留代码
- 文档和本地实验

## 当前事件家族

### 账号与会话事件

这是目前发现的最清晰的结构化业务日志。

- [libythtbbs/user.c](../../../libythtbbs/user.c)
  - `system passerr %s`
    - 按来源地址记录的登录失败尝试
    - 也被 `bbslogd` 用于动态封禁站点逻辑
  - `%s enter %s using %s`
    - 登录成功
  - `%s exitbbs %ld`
    - 正常退出，带有在线时长
  - `system kill %s %d`
    - 过期账号清理
- [libythtbbs/cache/utmp.c](../../../libythtbbs/cache/utmp.c)
  - `%s drop www/api`
    - 陈旧的 `www` / `api` 会话清理
- [src/bbs/main.c](../../../src/bbs/main.c)
  - `%s enter %s`
    - 终端登录
  - `%s exitbbs %ld`
    - 终端退出
  - `%s drop %ld`
    - 异常断开或中止
  - `%s kick %s multi-login`
    - 强制重复登录清理
- [src/bbs/register.c](../../../src/bbs/register.c)
  - `%s newaccount %d %s`
    - 终端注册
- [src/bbs/delete.c](../../../src/bbs/delete.c)
  - `%s kick %s`
    - 行政踢出 (kick)
- [nju09/www/bbsdoreg.c](../../../nju09/www/bbsdoreg.c)
  - `%s newaccount %d %s www`
    - Web 注册

解读：

- 这些是第一阶段迁移的有力候选者
- 负载相对结构化
- 它们对编码的敏感度低于内容日志
- 同一个逻辑家族已经跨越了终端、Web 以及库/会话层

### 版面使用与统计事件

这些看起来很简单，但由于它们驱动了后期的统计和计分，因此非常重要。

- [src/bbs/boards.c](../../../src/bbs/boards.c)
  - `%s use %s %ld`
    - 版面停留时长
    - 被版面计分算法使用
- [nju09/www/BBSLIB.c](../../../nju09/www/BBSLIB.c)
  - `%s use %s %ld`
    - Web 端的版面停留时长记录
- [nju09/www/bbsfind.c](../../../nju09/www/bbsfind.c)
  - `%s bbsfind %d`
    - 搜索结果计数
    - 更像是搜索/统计追踪，而非核心业务状态

解读：

- `use` 与业务相关，而不仅仅是描述性的遥测数据
- `bbsfind` 优先级较低，可能属于分析端，而非第一个结构化迁移切片

### 发帖、邮件与内容生命周期事件

这些是重要的业务日志，但它们对编码非常敏感，且在不同的访问路径之间可能存在重复。

- [src/bbs/bbs.c](../../../src/bbs/bbs.c)
  - `%s post %s %s`
  - `%s edit %s %s %s`
  - `%s changetitle %s %s oldtitle:%s newtitle:%s`
  - `%s del %s %s %s`
  - `%s undel %s %s %s`
  - `%s crosspost %s %s`
  - `%s thread %s`
    - 还会触发遗留的 `bin/thread` 助手
- [src/bbs/1984.c](../../../src/bbs/1984.c)
  - `%s check1984 %s %s`
  - `%s post %s %s`
- [src/bbs/read.c](../../../src/bbs/read.c)
  - `%s sametitle %s %s`
- [src/bbs/mail.c](../../../src/bbs/mail.c)
  - `%s mail %s`
  - `%s netmail %s`
- [nju09/www/bbssnd.c](../../../nju09/www/bbssnd.c)
  - `%s post %s %s`
- [nju09/www/BBSLIB.c](../../../nju09/www/BBSLIB.c)
  - `%s mail %s`
- [api/api_article.c](../../../api/api_article.c)
  - `%s post %s %s`
- [local_utl/autoundeny/autoundeny.c](../../../local_utl/autoundeny/autoundeny.c)
  - `XJTU-XANET mail %s`
    - 由狭窄的实用程序触发的站内信副作用

解读：

- 该家族已跨越终端、Web、API 和实用程序路径
- 标题和衍生自内容的字段使其对 GBK 敏感
- 迁移将需要规范化和去重，而不是原始日志复制

### 仲裁与行政内容状态事件

这些与发帖生命周期有重叠，但值得单独列出，因为它们审计负担重，且即使在第一阶段不迁移，也可能有用。

- [src/bbs/bbs.c](../../../src/bbs/bbs.c)
  - `%s mark %s %s %s` / `%s unmark %s %s %s`
  - `%s water %s %s %s` / `%s unwater %s %s %s`
  - `%s digest %s %s %s` / `%s undigest %s %s %s`
  - `%s ranged %s %d %d`
  - `%s rangedmail %d %d`
  - 置顶 / 取消置顶操作
- [src/bbs/bm.c](../../../src/bbs/bm.c)
  - `%s deny %s %s`
- [src/bbs/announce.c](../../../src/bbs/announce.c)
  - `%s %s %s %s`
  - `%s import %s %s %s`
- [nju09/www/bbsdenyadd.c](../../../nju09/www/bbsdenyadd.c)
  - `%s deny %s %s`
- [nju09/www/bbsman.c](../../../nju09/www/bbsman.c)
  - 标记 / 取消标记
  - 精华 / 取消精华

解读：

- 这些是真实的业务与审计事件
- 许多携带标题或类似路径的负载
- 它们比第一阶段低风险切片更有可能属于后期的迁移阶段

### 社交、互动与小众用户事件

- [src/bbs/talk.c](../../../src/bbs/talk.c)
  - `%s five %s`
  - `%s talk %s`
- [src/bbs/xyz.c](../../../src/bbs/xyz.c)
  - `%s sendgoodwish %s`
  - `%s finddf %s %d`

解读：

- 这些是面向用户的，但优先级低于账号、版面使用和核心发帖/邮件事件

### 运维、缓存与集成诊断

这些是当前 `newtrace` 管道的一部分，但它们不是数据库支持的业务日志记录的良好首选候选者。

- [libythtbbs/cache/board.c](../../../libythtbbs/cache/board.c)
  - `system reload bcache %d`
- [libythtbbs/cache/user.c](../../../libythtbbs/cache/user.c)
  - `system reload ucache %d`
- [libythtbbs/cache/cache-internal.h](../../../libythtbbs/cache/cache-internal.h)
  - `SHM Error! ...`
- [libythtbbs/mailsender.c](../../../libythtbbs/mailsender.c)
  - SMTP 运行时状态追踪
- [src/bbs/comm_lists.c](../../../src/bbs/comm_lists.c)
  - `system reload sysconf.img2`
- [src/bbs/more.c](../../../src/bbs/more.c)
  - `system reload movie %d`
- [src/bbs/power_select.c](../../../src/bbs/power_select.c)
  - `%s full_search %s %s`
  - `%s select %s %d %d`
  - 较低优先级的追踪/调试类日志
- [api/api_article.c](../../../api/api_article.c)
  - `.DIR` 修复与文件头写入错误
- [api/api_oauth.c](../../../api/api_oauth.c)
  - 2FA 生成失败
- [api/api_user.c](../../../api/api_user.c)
  - Redis 活动追踪
- [libbmy/search.c](../../../libbmy/search.c)
  - 搜索助手/运行时失败
- [libbmy/wechat.c](../../../libbmy/wechat.c)
  - 微信集成失败

解读：

- 这些应大多保留在运维/调试端
- 它们仍然重要，但不应定义未来业务事件日志记录的模式

## 编码说明

### 较低风险日志

- 账号与会话事件
- 大多数 `use` 计时日志
- 一些行政清理事件

这些大多是 ASCII 结构化的，早期迁移较为安全。

### 较高风险日志

- 帖子标题
- 邮件以及带有标题的仲裁日志
- 公告操作与路径
- 衍生自内容的搜索词
- 旧/新标题对

这些非常可能携带源自 GBK 的负载或其他遗留文本。它们需要明确的迁移规则，而非幼稚的结构化提取。

### 解析警告

- `*` 出现在正常的历史日志内容中，特别是在标题中。
- 因此，在解析存档日志文件时，不应将 `*` 解读为前换行符的证据。
- 对于下一阶段，现有的 `newtrace` 文件仍应逐行解析，但必须将 `*` 视为普通的消息内容。

## 迁移相关性

### 强早期候选者

- 登录成功
- 退出
- 断开 / 失去连接
- 登录失败尝试
- 重复登录清理
- 账号创建
- 陈旧的 `www/api` 会话清理
- 版面使用计时

### 重要但建议推迟

- 发帖事件
- 邮件事件
- 仲裁状态变更
- 公告导入与操作日志
- 衍生自内容的搜索/统计追踪

### 可能保留在运维端

- 缓存重载日志
- 共享内存错误
- SMTP 运行时日志
- API 文件修复追踪
- OAuth / Redis 集成追踪
- `libbmy` 运行时及第三方集成失败

## 当前评估

- 日志后端简单且稳定，但与 SysV IPC 和每日文件系统日志紧密耦合。
- `newtrace` 目前负载过重：业务日志、仲裁审计日志和运行时诊断都共享同一个传输和接收器。
- 同样的逻辑事件家族已经出现在终端 BBS、遗留 Web CGI 以及 API 层中。
- 一些日志（特别是 `system passerr`）并非纯粹的审计记录，因为它们还会驱动运行时行为。
- 第一个迁移步骤应分离事件家族，而非在另一个存储引擎中复制当前的文本流。

## 开放性问题

- 哪些事件家族在第一阶段应作为结构化业务数据保留，哪些目前应保持基于文件的形式？
- 哪些调用者组在终端、Web 和 API 访问路径中重复记录了相同的现实世界事件？
- 当前 `bbslogd` 的行为（特别是 `system passerr` 副作用）应有多少转移到未来的日志管道中？
