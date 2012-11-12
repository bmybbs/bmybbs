#!/bin/sh

rm -f -r /home/bbs
rm -f -r /home/apache
if [ -f /etc/apache2/sites-available/bbs ]
then
rm /etc/apache2/sites-available/bbs
fi
