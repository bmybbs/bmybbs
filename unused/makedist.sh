#!/bin/sh
rm configure
for i in config.sub config.guess install-sh;do
	cp /usr/share/automake/$i .
done
autoconf
