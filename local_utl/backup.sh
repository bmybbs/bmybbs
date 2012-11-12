#!/bin/bash

BBSNAME=ytht
KEYID=YTHTBBS
BBSHOME=/home/bbs
BACKUPDIR=/root/bbsbak/backup
TMPFILE=/tmp/$$.list
SPECIAL=/root/specialdir
LASTFULL=/root/lastfull
NEEDFULL=/root/needfull
level="-1"

if [ -f $LASTFULL ];then
	lastfd="-N `cat $LASTFULL`"
fi

if [ -f $NEEDFULL ];then
	lastfd=""
fi
	

[ -L $TMPFILE ] && exit
>$TMPFILE
for i in 4 5 6; do
	dn=`/bin/date --date="$i days ago" +%a`
	/bin/rm -fr $BACKUPDIR/$dn/*
	/bin/sync
done
dn=`/bin/date +%a`
if [ ! -d $BACKUPDIR/$dn ];then
	/bin/rm -fr $BACKUPDIR/$dn
	/bin/mkdir -p $BACKUPDIR/$dn
fi

if [ ! -d $BACKUPDIR/backnumbers ];then
	/bin/rm -fr $BACKUPDIR/backnumbers
	/bin/mkdir -p $BACKUPDIR/backnumbers
fi

cd $BBSHOME
for i in .* *
do
	if /bin/grep $i $SPECIAL &> /dev/null; then
		continue
	else
		/bin/echo $i >> $TMPFILE
	fi		
	/bin/sync
done
/bin/tar -cp $lastfd `cat $TMPFILE`|/bin/gzip $level| /usr/bin/gpg -r $KEYID -e > $BACKUPDIR/$dn/$BBSNAME.root.$dn.tgz
/bin/rm -f $TMPFILE
for i in 0Announce/groups/*/*
do
	if [ -L $i ]; then
		continue
	fi
	name=`/bin/echo $i|/usr/bin/cut -d'/' -f4`
	/bin/tar -cp $lastfd $i|/bin/gzip $level|/usr/bin/gpg -r $KEYID -e > $BACKUPDIR/$dn/$BBSNAME.0An.$name.$dn.tgz
	/bin/sync
done
for i in boards home mail
do
	for j in $i/*
	do
	if [ -L $j ]; then
		continue
	fi
		name=`/bin/echo $j|/usr/bin/cut -d'/' -f2`
		/bin/tar -cp $lastfd $j|/bin/gzip $level|/usr/bin/gpg -r $KEYID -e > $BACKUPDIR/$dn/$BBSNAME.$i.$name.$dn.tgz
		/bin/sync
	done
done

for i in backnumbers/*/*
do
	name=`/bin/echo $i|/usr/bin/tr / .`
	if [ -L $i ]; then
		continue
	fi
	if [ ! -e $BACKUPDIR/backnumbers/$name.tgz -o $i -nt $BACKUPDIR/backnumbers/$name.tgz ] ; then
		/bin/mv $BACKUPDIR/backnumbers/$name.tgz $BACKUPDIR/backnumbers/$name.tgz.old
		/bin/tar -cp $lastfd $i|/bin/gzip $level|/usr/bin/gpg -r $KEYID -e >  $BACKUPDIR/backnumbers/$name.tgz
	fi
	/bin/sync
done
/bin/tar -cp $lastfd backnumbers/*/.DIR|/bin/gzip $level|/usr/bin/gpg -r $KEYID -e > $BACKUPDIR/backnumbers/dir.tgz

if [ "$lastfd" != "" ];then
	for i in home etc
	do
	/bin/tar -cpl $lastfd /$i|/bin/gzip $level|/usr/bin/gpg -r $KEYID -e > $BACKUPDIR/$dn/$BBSNAME.disk.$i.$dn.tgz
	/bin/sync
	done
	touch $BACKUPDIR/$dn/base.`echo $lastfd|cut -d' ' -f2`
fi
	
touch $BACKUPDIR/$dn/finish.mark
