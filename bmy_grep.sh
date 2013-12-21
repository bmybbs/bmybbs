#!/bin/sh
# 对bmy代码进行grep
# author: interma@bmy

grep --color=auto -n $1 include/*.h
grep --color=auto -n $1 ythtlib/*.c
grep --color=auto -n $1 ythtlib/*.h
grep --color=auto -n $1 libythtbbs/*.c
grep --color=auto -n $1 libythtbbs/*.h
grep --color=auto -n $1 src/*.c
grep --color=auto -n $1 src/*.h
grep --color=auto -n $1 nju09/*.c
grep --color=auto -n $1 nju09/*.h
grep --color=auto -n $1 local_utl/*.c
grep --color=auto -n $1 local_utl/*.h

