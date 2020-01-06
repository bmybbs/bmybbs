#!/bin/sh
#这个脚本用于从别的机器定期下载备份BBS站上面的东西,计划保留
#最近3天,3周,3月(如果有空间,还可以保留3季)九个备份
#以下是可配置参数
export LC_ALL=en #星期一还是用Mon吧，好过“一”..
hostip=162.105.31.222
daysdelay=0
minsize=1 #完整的备份至少有... 不知道
BACKROOT=/backup
#以下是程序主体

function hasbak
{
	thisd=`echo $2|cut -d. -f1`
	case $3 in
		week)
			check=`date -d $thisd +%w`
			check=`expr $check + 1`
			;;
		month)
			check=`date -d $thisd +%e`
			;;
	esac
	i=1
	while [ $i -lt $check ];do
		eff=`date -d "$thisd $i days ago" +%Y%m%d.%a`
		if [ -d $1/$eff ];then
			true
			return
		fi
		i=`expr $i + 1`
	done
	false
	return
}

function delold
{
	while true; do
		nhas=`ls $1|wc -l`
		if [ $nhas -gt $3 ];then
			oldest=`ls $1|sort|head -n1`
			if [ $2 = "null" ] || hasbak $2 $oldest $4 ;then
				rm -fr $1/$oldest
			else
				mv $1/$oldest $2
			fi
		else
			return
		fi
	done
}
# main function start here
[ -d $BACKROOT/day ] || mkdir -p $BACKROOT/day
[ -d $BACKROOT/week ] || mkdir -p $BACKROOT/week
[ -d $BACKROOT/month ] || mkdir -p $BACKROOT/month

cd $BACKROOT
nowdate=`date -d "$daysdelay day ago" +%Y%m%d.%a`
todown=`echo $nowdate|cut -d. -f2`
lftp -c "open $hostip -p 4000;mirror $todown $nowdate"
if [ ! -f $nowdate/finish.mark -o `du -s $nowdate|cut -f1` -lt $minsize ];then
	rm -fr $nowdate
	exit
fi
mv $nowdate day
delold day week 3 week
delold week month 3 month
delold month null 3
lftp -c "open $hostip -p 4000;mirror backnumbers -n;get ythtsrc.$todown.tar.bz2"
