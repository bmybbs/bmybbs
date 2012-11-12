BASEPATH =..
include $(BASEPATH)/Makefile.Base

PROGNAME = yftpd
VERSION = 1.2
FTPHOME = $(BBSHOME)/ftphome

CFILE = commands.c dirlist.c logging.c login.c main.c mystring.c pathop.c yftpdutmp.c
COBJS    :=$(CFILE:.c=.o)

DEFS =  -DVERSION=\"1.0\" -DHAVE_DIRENT_H=1 -DSTDC_HEADERS=1 -DHAVE_SYS_WAIT_H=1 \
	-DHAVE_FCNTL_H=1 -DHAVE_SYS_FILE_H=1 -DHAVE_SYS_TIME_H=1 -DHAVE_TIME_H=1 -DHAVE_UNISTD_H=1 \
	-DTIME_WITH_SYS_TIME=1 -DRETSIGTYPE=void -DHAVE_STRFTIME=1 -DHAVE_GETCWD=1 \
	-DHAVE_MKDIR=1 -DHAVE_RMDIR=1 -DHAVE_SELECT=1 -DHAVE_SOCKET=1 -DHAVE_STRDUP=1 \
	-DHAVE_STRERROR=1 -DHAVE_STRSTR=1 -DHAVE_STRTOUL=1 -D_FILE_OFFSET_BITS=64 -I.

.c.o:   ;   $(CC) $(CFLAGS) $(DEFS) -c $*.c

all: $(PROGNAME)

ifeq (.depend,$(wildcard .depend))
include .depend
endif


depend dep:
	gcc $(CFLAGS) $(DEFS) -M *.c >.depend

$(PROGNAME): $(COBJS) $(OTHERS)
	$(CC) -o $(PROGNAME) $(CFLAGS) $(COBJS) $(LIBS)  -L/usr/lib $(BBSLIB)

install: all
	mkdir -p $(FTPHOME)/root/pub
	mkdir -p $(FTPHOME)/root/boards
	cp ftpmotd $(FTPHOME)
	$(INSTALL) -s -m 550 -g $(BBSGRP) -o $(BBSUID) yftpd $(FTPHOME)/yftpd.new
	-mv $(FTPHOME)/yftpd $(FTPHOME)/yftpd.old
	mv $(FTPHOME)/yftpd.new $(FTPHOME)/yftpd

clean:
	rm -fr *.o core a.out

