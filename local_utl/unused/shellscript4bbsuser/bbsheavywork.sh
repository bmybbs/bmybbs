#!/bin/sh
#this script will do some heavy work such as backup on night
if [ $DELAYT = "" ];then
	export DELAYT=400
fi
LDELAYT=`expr $DELAYT / 10`
nowday=`/bin/date +%j`
nowweek=`/bin/date +%w`

cd /home/bbs

/home/bbs/bin/nbstat s 7
date
sleep $LDELAYT
/home/bbs/bin/bbsinfo > /home/bbs/0Announce/groups/GROUP_0/BM_Club/7days
date
sleep $LDELAYT
/home/bbs/bin/bm2|sort -k2|awk '{print $1,$3,$4,$5,$6,$7}' > /home/bbs/0Announce/bm2
date
sleep $LDELAYT
/home/bbs/bin/delvote.sh
date
sleep $LDELAYT
/home/bbs/bin/auto_rm_junk >/home/bbs/main/junklog
date
sleep $DELAYT
/home/bbs/bin/clear_junk
date
sleep $DELAYT
/home/bbs/bin/reminder
date
sleep $LDELAYT
/home/bbs/bin/MakeIndex3.sh
date
sleep $DELAYT

if [ $nowweek -eq 1 ];then
	/home/bbs/bin/nbstat mu 7
	/home/bbs/bin/cvslog.sh &> /dev/null
	/home/bbs/bin/delvote.sh
	date
	sleep $LDELAYT
	killall -TTOU bbs
	date
	sleep $LDELAYT
fi

if [ `expr $nowday % 3` -eq 0 ];then
	/home/bbs/bin/bm |sort -f > /home/bbs/0Announce/groups/GROUP_0/BM_Club/boardlist
	/home/bbs/bin/bm2|/home/bbs/bin/id_boards s|sort -f > /home/bbs/0Announce/groups/GROUP_0/BM_Club/bmlist
	/home/bbs/bin/anall &> /root/bbsbak/bbslog/xlog
	date
	sleep $DELAYT
fi

if [ `expr $nowday % 3` -eq 1 ];then
	/home/bbs/bin/find_rm_lost >/home/bbs/main/lostlog
	date
	sleep $DELAYT
fi

if [ `expr $nowday % 9` -eq 2 ];then
	/home/bbs/bin/find_lost_mail >/home/bbs/main/lostmaillog
	date
	sleep $DELAYT
fi
