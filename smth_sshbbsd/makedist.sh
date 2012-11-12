#!/bin/sh
echo "hehe,you must have automake 1.6.x and autoconf 2.5.x to use this,kick kcn@smth"
aclocal
autoheader
automake --add-missing
autoconf
