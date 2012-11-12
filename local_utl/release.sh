#!/bin/sh
ver=0.0.1
if [ $# -eq 0 ]; then
	time=`date +%Y%m%d`
else
	time=$1
fi
if [ -d ~/release4lepton ] ;then
	(cd ~/release4lepton;cvs update -dP -D $time;rm -f ChangeLog;cvs2cl.pl)
	cp ~/release4lepton ythtbbs-$ver-snap-$time -a
else
	(mkdir tmp.$$;cd tmp.$$;cvs co -D $time bbs;cd bbs;cvs2cl.pl -l "-d$time";cd ..;mv bbs ../ythtbbs-$ver-snap-$time;cd ..;rmdir tmp.$$)
fi
(cd ythtbbs-$ver-snap-$time;./makedist.sh)
rm -fr `find ythtbbs-$ver-snap-$time -name CVS -a -type d`
tar -cvpzf ythtbbs-$ver-snap-$time.tgz ythtbbs-$ver-snap-$time &> tarlog
rm -fr ythtbbs-$ver-snap-$time
