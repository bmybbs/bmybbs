#!/bin/sh
function updatedir
{
	dt=`date -d "$3 days ago" +%Y%m%d`
	rm $1
	ln $2/$dt $1 -sf
	cp ~/etc/club_check_u $1/club_users -f
}
#start here
cd ~/boards
updatedir tochecktoday .1984 0
updatedir tocheckyesterday .1984 1
