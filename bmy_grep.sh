#!/bin/sh
# 对bmy代码进行grep
# author: interma@bmy
# author: ironblood@bmy 合并多个 grep 语句变更为递归方式

grep -anr --color=auto --include "*.c" --include "*.h" $1 include ythtlib libythtbbs src nju09 local_utl atthttpd
