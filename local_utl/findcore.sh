#!/bin/bash
while true;do
	if [ -f /home/bbs/core ] ;then
		now=`/bin/date +%Y.%m.%d.%H:%M:%S`
		mv /home/bbs/core /home/bbs/core.tmp/core.$now
		execfile=`gdb -c /home/bbs/core.tmp/core.$now -batch|head -n1|cut -d' ' -f5|cut -d"'" -f1|cut -d '\`' -f2`
		if [ $execfile = "/home/httpd/cgi-bin/www" ] ;then
			cp $execfile /home/bbs/core.tmp/www.$now
		else
			cp /home/bbs/bin/bbs.r /home/bbs/core.tmp/bbs.$now
		fi
		sleep 900
	fi
	sleep 5		
done
