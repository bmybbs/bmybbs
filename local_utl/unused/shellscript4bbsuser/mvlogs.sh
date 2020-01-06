#!/bin/sh
mv /home/bbs/trace /home1/bbslogs/trace.`date +%b%d`
mv /home/bbs/usies /home1/bbslogs/usies.`date +%b%d`
mv /home/bbs/logins.bad /home1/bbslogs/loginsbad.`date +%b%d`
gzip /home1/bbslogs/*.`date +%b%d`
cp /dev/null /home/bbs/delmsg.lst
cp /dev/null /home/bbs/mail-log
cp /home/bbs/.PASSWDS /home1/bbslogs/PASSWDS.`date +%b%d`
