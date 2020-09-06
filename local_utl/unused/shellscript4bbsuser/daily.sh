#!/bin/sh
cp -af /home/bbs/0Announce/bbslist/countusr /home/bbs/0Announce/bbslist/countusr.last
cp -af /home/bbs/0Announce/bbslist/newacct.today /home/bbs/0Announce/bbslist/newacct.last
cp -af /home/bbs/etc/posts/day /home/bbs/etc/posts/day.last
/home/bbs/bin/bbstop
/home/bbs/bin/autoundeny
