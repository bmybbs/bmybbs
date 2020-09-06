BASEPATH = ..
include $(BASEPATH)/Makefile.Base

MAIL2BBS=mail2bbs
PERLHELP=bbsmail bbsmail_wrap
INSTALLDIR=$(BBSHOME)/bin

all:
	@echo use 'make mail2bbs' to make mail2bbs
	@echo use 'make install_mail2bbs' to install the mail2bbs prog
	@echo use 'make install' to install the prog and the perl scripts

$(MAIL2BBS): mail2bbs.c qp_conv.c
	$(CC) -g $(CFLAGS) -o $(MAIL2BBS) $^ $(BBSLIB)
 

install_mail2bbs: $(MAIL2BBS)
	test -d $(INSTALLDIR) || mkdir -p $(INSTALLDIR)
	-install -g $(BBSGRP) -m 4750 -o $(BBSUID) $(MAIL2BBS) $(INSTALLDIR)

install: install_mail2bbs
	-install -g $(BBSGRP) -m  750 -o $(BBSUID) $(PERLHELP) $(INSTALLDIR)
