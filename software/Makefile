include Makefile.Base
fastcgi_v:=2.2.12
fcgi_v:=2.2.2
ghthashv=0.5.0-x86_64

fastcgi:=mod_fastcgi-$(fastcgi_v)
fastcgi_s:=$(fastcgi).tar.gz
fcgi:=fcgi-$(fcgi_v)
fcgi_s:=$(fcgi).tar.gz
CPU:=pentium

.PHONY:installfcgidev

installfcgidev:$(fcgi_s)
	rm -fr $(fcgi);tar xzvf $<;cd $(fcgi);./configure --prefix=/usr;make;make install
installghthash:libghthash-$(ghthashv).tar.gz
	tar xzvf $^;cd $(^:.tar.gz=);./configure;$(MAKE);$(MAKE) install

