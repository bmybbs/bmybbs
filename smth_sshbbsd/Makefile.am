BASEPATH = ..
include $(BASEPATH)/Makefile.Base
HOST_KEY_FILE 	= $(BBSHOME)/etc/ssh_host_key
SERVER_CONFIG_FILE = $(BBSHOME)/etc/sshd_config
ETCDIR = $(BBSHOME)/etc
PIDDIR = $(BBSHOME)

ADDLIBS         =  -lgmp -lz

noinst_HEADERS = cipher.h getput.h compress.h idea.h \
    randoms.h crc32.h includes.h \
    userfile.h blowfish.h des.h md5.h rsa.h version.h bufaux.h \
    mpaux.h servconf.h xmalloc.h buffer.h emulate.h packet.h ssh.h \
    deattack.h snprintf.h

COMMON_OBJS = rsa.o randoms.o buffer.o emulate.o packet.o compress.o \
	xmalloc.o bufaux.o authfile.o \
	crc32.o rsaglue.o cipher.o des.o match.o mpaux.o \
	userfile.o signals.o blowfish.o deattack.o md5.o

BBSCFILES = bbs.c record.c delete.c main.c stuff.c mail.c register.c \
           xyz.c talk.c read.c comm_lists.c namecomplete.c chat.c \
           maintain.c edit.c more.c help.c bcache.c boards.c \
           smth_screen.c io.c term.c userinfo.c vote.c announce.c sendmsg.c\
           bm.c list.c goodbye.c bbsgopher.c fileshm.c postheader.c \
           convcode.c five.c tmachine.c addressbook.c\
           backnumber.c common.c sec_hand.c regular.c 1984.c moneycenter.c\
	   editboard.c tmpl.c power_select.c identify.c

BBSOBJS = $(BBSCFILES:.c=.o)

SSHD_OBJS = sshd.o auth-passwd.o \
	log-server.o canohost.o servconf.o tildexpand.o \
	serverloop.o $(COMMON_OBJS) $(BBSOBJS)

EXTRA_SRCS = memmove.c snprintf.c strerror.c remove.c random.c \
	putenv.c crypt.c socketpair.c

EXTRA_DIST= COPYING README README.SECURID README.TIS README.SECURERPC \
	README.CIPHERS README.DEATTACK INSTALL TODO libdes-README \
	libdes-COPYING libdes-ARTISTIC gnu-COPYING-GPL OVERVIEW \
	configure configure.ac config.guess config.sub Makefile.in \
	config.sample acconfig.h config.h.in install-sh ChangeLog \
	idea.c

CLEANFILES=@EXTRA_CLEAN@
bin_PROGRAMS = sshbbsd
sshbbsd_LDADD = @CONFOBJS@  $(LIBS) $(BBSLIB) $(ADDLIBS)
sshbbsd_DEPENDENCIES = @CONFOBJS@ ~/bin/libythtbbs.a ~/bin/libytht.a
sshbbsd_SOURCES = $(SSHD_OBJS:.o=.c)

AM_CFLAGS=$(OS_DEF) $(CSIE_DEF) $(CPPFLAGS) -DBACK_DELETE_RANGE -DSSHBBS -I. -I $(BASEPATH)/src -DHOST_KEY_FILE=\"$(HOST_KEY_FILE)\" -DSERVER_CONFIG_FILE=\"$(SERVER_CONFIG_FILE)\" -DSSH_PROGRAM=\"$(SSH_PROGRAM)\" -DETCDIR=\"$(ETCDIR)\" -DPIDDIR=\"$(PIDDIR)\" -include $(BASEPATH)/src/proto.h -include $(BASEPATH)/ythtlib/ythtlib.h -include $(BASEPATH)/libythtbbs/ythtbbs.h
AM_LDFLAGS=-rdynamic

# Creates /etc/ssh_host_key
generate-host-key:
	-@if test -f $(HOST_KEY_FILE); \
          then echo "You already have a host key."; \
	  else \
	    umask 022; echo "Generating 1024 bit host key."; \
	    ssh-keygen -t rsa1 -f $(HOST_KEY_FILE) -N ''; \
	  fi
.PHONY:update
update: $(bin_PROGRAMS) $(BBSHOME)/bin/$(bin_PROGRAMS)
$(BBSHOME)/bin/$(bin_PROGRAMS):$(bin_PROGRAMS)
	-mv $(BBSHOME)/bin/sshbbsd $(BBSHOME)/bin/sshbbsd.old
	cp sshbbsd $(BBSHOME)/bin
	@if [ -f $(BBSHOME)/sshd.pid ];then \
		kill -HUP `cat $(BBSHOME)/sshd.pid`; \
	else \
		echo "no sshd.pid found! please kill -hup it yourself"; \
	fi
$(BBSCFILES):
	ln $(BASEPATH)/src/$@ . -s
