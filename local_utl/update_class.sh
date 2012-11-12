#!/bin/sh
all=`cat ~/etc/class`
hasup=0
for i in $all; do
	if [ ~/0Announce/groups/GROUP_0/${i}_ytml/class -nt ~/etc/classes ];then
		hasup=1
		break
	fi
done
if [ $hasup -eq 1 ];then
	> ~/etc/classes.new
	for i in $all; do
	cat ~/0Announce/groups/GROUP_0/${i}_ytml/class >> ~/etc/classes.new
	done
	mv ~/etc/classes.new ~/etc/classes
fi
