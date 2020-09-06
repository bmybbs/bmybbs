#!/bin/sh
#this script will do some heavy work such as backup on night
export DELAYT=400
su bbs -c "sh -x /home/bbs/bin/bbsheavywork.sh"
/root/bin/backup.sh &> /dev/null
date
sleep $DELAYT
/root/bin/backsource.sh &> /dev/null
