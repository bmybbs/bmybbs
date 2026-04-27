# Wiki 索引

> 状态：初稿

本 Wiki 是用于了解遗留 BMYBBS 系统并规划其迁移工作的共享知识库。

请将此索引作为进入规范 Wiki 页面的入口。草稿页面（如 `*.human.md`、`*.codex.md` 或 `*.<agent>.md`）是讨论产物，未作为规范内容列在此处。

## 如何使用本 Wiki

- 请先阅读 [wiki-conventions.md](./wiki-conventions.zh.md) 以了解页面状态术语、草稿工作流和证据规则。
- 使用规范的 `*.md` 页面作为当前的共同理解。
- 将源代码视为证据，而非唯一可信源。
- 明确记录冲突、不确定性和开放性问题。

## 规范页面

- [wiki 约定](./wiki-conventions.zh.md)
  - 状态：`初稿`
  - 定义了 Wiki 工作流、页面状态术语、草稿处理和证据规则。
- [项目总览](./project-overview.zh.md)
  - 状态：`初稿`
  - 对当前系统及其维护问题以及目标迁移方向的概要描述。
- [目标架构](./target-architecture.zh.md)
  - 状态：`初稿`
  - 未来系统形态、各层职责、退役计划和迁移方向的初步架构页面。
- [遗留子系统](./legacy-subsystems.zh.md)
  - 状态：`初稿`
  - 主要遗留访问层、服务守护进程、共享库及其可能迁移命运的高层图谱。
- [日志](./logs.md)
  - 状态：`初稿`
  - 重要 Wiki 操作和主要状态变更的时间顺序记录。
- [开放性问题](./open-questions.zh.md)
  - 状态：`初稿`
  - 真正开放的、可能影响架构、迁移或规划的战略性问题清单。
- [迁移阶段](./migration/migration-stages.zh.md)
  - 状态：`初稿`
  - 从低风险 PostgreSQL 验证到数据迁移、API 迁移和遗留退役的分阶段迁移策略。
- [日志系统](./cross-cutting/logging-system.zh.md)
  - 状态：`初稿`
  - 遗留 `newtrace` 和 `bbslogd` 日志管道的现状图谱、事件家族、编码风险及迁移相关性。
- [日志系统迁移](./migration/logging-system-migration.zh.md)
  - 状态：`初稿`
  - 分阶段的迁移计划，重点是首先重构日志写入路径，然后设计数据库导入和直接写入阶段。

## 计划页面
