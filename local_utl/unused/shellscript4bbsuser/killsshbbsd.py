#!/usr/bin/env python
# auto kill restart sshbbsd
# author: InterMa@BMY 2007-6-13
import popen2

cmd2 = "/home/bbs/bin/sshbbsd"
cmd1 = "ps aux|grep -e '"+cmd2+"'|grep -v 'grep'|gawk '{ print $2,$5 }'"

r, w= popen2.popen2(cmd1)

for line in r.readlines():
    pid, mem = line.split()
    if int(mem) < 10000:
        print line
        popen2.popen2('kill '+pid)
        popen2.popen2(cmd2)
        print 'done!'
