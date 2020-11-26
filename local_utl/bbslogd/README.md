bbslogd
=======

bbslogd 是基于 [SysV 消息队列 - svipc(7)](https://linux.die.net/man/7/svipc) 实现的处理 BMYBBS 日志的守护进程。发送日志使用 `ythtbbs/misc::newtrace(char *)` 接口，对应的记录存放在 `$MY_BBS_HOME/newtrace/yyyy-mm-dd-log` 文件中，并且在消息前附带上时间。`newtrace` 不需要换行符。

日志处理流程中有一块对用户登录的额外分析和处理，对于超过密码错误次数限制的 IP 记录在 `$MY_BBS_HOME/bbstmpfs/dynamic/bansite` 文件中。该文件在登录逻辑中被首先检验。

TODO
----

- [ ] 对 IPv6 的支持
- [ ] 采用 UTF8 输出日志

