#!/bin/sh
#这个脚本通过比较精华区备份的包的大小和精华区打包的包的大小 找出可疑的可能含有
#较多垃圾的精华区目录
anpath=/home/bbs/ftphome/root/pub/X
backuppath=/root/bbsbak/backup
dn=`date +%a`
for i in $anpath/*.tgz;do
	pksize=`ls -l $i|awk '{print $5}'`
	board=`basename $i|cut -d. -f1`
	bksize=`ls -l $backuppath/$dn/ytht.0An.$board.$dn.tgz|awk '{print $5}'`
	hide=`expr $bksize - $pksize`
	if [ $hide -gt 10000000 ] ; then
		echo $board $pksize $bksize
	fi
done
