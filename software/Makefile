include Makefile.Base
ghthashv=0.5.0-x86_64

.PHONY:installghthash

installghthash:libghthash-$(ghthashv).tar.gz
	tar xzvf $^;cd $(^:.tar.gz=);./configure;$(MAKE);sudo $(MAKE) install

