####################################################
# this is a bbs <--> news gateway
####################################################
# Don't change them
VERSION=0.50beta-5F
LASTVERSION=0.50beta-4
####################################################
# this will show up in the greeting message of innbbsd
ADMINUSER= SYSOP.bbs@ytht.net
#
####################################################
#BBSHOME = /tmp/innbbsd-$(VERSION)
#BBS_SRC = /u/staff/bbsroot/csie_util/bntpd/bbs_src
BASEPATH = ..
include $(BASEPATH)/Makefile.Base
#LANG=BIG5
LANG=GB
#LANG=ENGLISH

######################################################
# if you want to do gb <--> big5 converstion on the fly
# You should install HC or WORDGB before adding this line
#FILTER=-DFILTER -DBIG2GB -DGB2BIG -DHZ2GB
#FILTER=-DFILTER -DBIG2GB -DGB2BIG -DHZ2GB -DBIG2JIS -DJIS2BIG
#FILTER=WORDG2B
FILTER=-DFILTER
FILTEROBJ= filter.o \
wildmat.o \
#          b2g.o \
#          b2g_tables.o \
#	  hz2gb.o \
#          log.o \

#######################
# if you want to make a "bbsnnrp" only, 
# change TARGET=$(BBSNNRP)
TARGET  = $(INNBBSD)
#TARGET  = $(BBSNNRP)
#TARGET  = $(BBSLINK)
#TARGET  = $(BBSNNRP) $(BBSLINK)
####################################################
INSTALL=cp
#
#OPT= -g -pg 
OPT= -g

BBS_DEP = FirebirdBBS
BBS_POST = innd/bbspost

DEBUGOBJ = /usr/lib/debug/mallocmap.o
CFLAGS := -c $(CFLAGS) -I. -D$(BBS_DEP) $(EXTRAFLAGS) -DDBZDEBUG \
	-D_PATH_BBSHOME=\"$(BBSHOME)\" -DVERSION=\"$(VERSION)\" \
	-DADMINUSER=\"$(ADMINUSER)\" $(FILTER) \
	-DLANG=\"$(LANG)\"
#
####################################################

OBJS = nocem.o inndchannel.o innbbsd.o connectsock.o rfc931.o \
       daemon.o file.o pmain.o parsdate.o his.o dbz.o \
       closeonexec.o dbztool.o inntobbs.o echobbslib.o receive_article.o \
       qp_conv.o lang.o\
       port.o $(FILTEROBJ)
SRCS = $(OBJS:.o=.c)

MOBJS    = makedbz.o bbslib.o file.o dbz.o closeonexec.o 
HOBJS    = mkhistory.o bbslib.o file.o his.o dbz.o port.o closeonexec.o
DBZOBJS  = dbzserver.o dbzinnbbsd.o pmain.o connectsock.o bbslib.o his.o \
	   daemon.o file.o dbz.o dbztool.o rfc931.o port.o closeonexec.o
NNRPOBJS = bbsnnrp.o pmain.o bbslib.o connectsock.o file.o 
LINKOBJS = bbslink.o pmain.o inntobbs.o echobbslib.o connectsock.o file.o \
	   port.o  parsdate.o lang.o $(FILTEROBJ)
CTLOBJS  = ctlinnbbsd.o pmain.o bbslib.o connectsock.o file.o 
INNBBSD  = innbbsd makedbz mkhistory dbzserver bbsnnrp \
	   ctlinnbbsd $(BBS_POST) dbz_query_sample $(BBSLINK) 
BBSNNRP  = bbsnnrp
BBSLINK  = bbslink
#BBSLINK  = 
EXES     = $(TARGET)

.c.o:
	$(CC) $(OPT) $(CFLAGS) $*.c


all: 
	@echo "Your Configurations are listed as follows" 
	@echo "  CC = $(CC)"
	@echo "  BBSHOME = $(BBSHOME)"
	@echo "  BBS_SRC = $(BBS_SRC)"
	@echo "  BBS_DEP = $(BBS_DEP)"
	@echo " BBS_POST = $(BBS_POST)"
	@echo "ADMINUSER = $(ADMINUSER)"
	@echo "Change them in Makefile if not correct."
	@echo ""
	@echo "Please enter 'make sys-type', "
	@echo " make sun     : for Sun-OS 4.x and maybe some BSD systems, cc or gcc" 
	@echo " make linux   : for Linux 1.x"
	@echo " make solaris : for Sun-OS 5.x gcc" 
	@echo " make aix     : for IBM AIX" 
	@echo " make hpux    : for HP UX"
	@echo " make irix    : for IRIS"
	@echo " make alpha   : for DEC Alpha running OSF/1" 
	@echo " make freebsd : for BSD 4.4 systems" 
	@echo " make bsd     : for BSD systems, cc or gcc, if not in the above listts" 
	@echo " make sysv    : for SYSV systems, cc or gcc, if not in the above lists" 

sun: 
	@$(MAKE) EXTRAFLAGS="-DMMAP -DGETRUSAGE -DMALLOCMAP" LDFLAGS="$(DEBUGOBJ)" target


aix: 
	@$(MAKE) CC=cc EXTRAFLAGS="-DMMAP -DGETRUSAGE -DAIX -DSYSV" target

linux: 
	@$(MAKE) EXTRAFLAGS="-DLINUX -DUSE_NCM_PATCH -DGETRUSAGE" target

hpux: 
	@$(MAKE) OPT= EXTRAFLAGS="-DMMAP -DHPUX -DSYSV" target

irix: 
	@$(MAKE) EXTRAFLAGS="-DMMAP -DIRIX -DSYSV" target

solaris:
	@$(MAKE) EXTRAFLAGS="-DMMAP -DSOLARIS -DSYSV" LDFLAGS="-lsocket -lnsl" YACC="bison -y" target

freebsd:
	@$(MAKE) CC=cc EXTRAFLAGS="-DBSD44 -DMMAP -DGETRUSAGE" LIBCRYPT=-lcrypt  target

bsd:
	@$(MAKE) CC=cc EXTRAFLAGS="-DMMAP -DGETRUSAGE" target

alpha:
	@$(MAKE) CC=cc EXTRAFLAGS="-DMMAP -DDEC_ALPHA -DGETRUSAGE" target

sysv: 
	@$(MAKE) OPT= EXTRAFLAGS="-DMMAP -DSYSV" target

target: $(EXES)

r_port.o: r_port.c
	$(CC) $(OPT) $(CFLAGS) -DWITH_RECORD_O r_port.c 

echobbslib.o: echobbslib.c
	$(CC) $(OPT) $(CFLAGS) -DWITH_ECHOMAIL echobbslib.c

$(BBS_POST): $(BBS_POST).c
	$(MAKE) -C innd

tar: 
	test -d innbbsd-$(VERSION) || mkdir innbbsd-$(VERSION)
	cp *.c *.h Makefile innbbsd-$(VERSION)
	tar zcf innbbsd-$(VERSION).tar.gz innbbsd-$(VERSION)
	cp innbbsd-$(VERSION).tar.gz /net/ftphome/CSIE/innbbsd

innbbsd-patch:
	rm -rf /tmp/innbbsd-$(LASTVERSION)
	zcat /net/ftphome/ftp-data/bbs/misc/innbbsd/innbbsd-$(LASTVERSION).tar.gz | (cd /tmp; tar xf -)	
	(cd /tmp/innbbsd-$(VERSION); make clean; rm -f r_port.c dbzserver.c dbzinnbbsd.c  echobbslib.c )
	(cd /tmp/innbbsd-$(LASTVERSION); make clean; rm -f r_port.c dbzserver.c dbzinnbbsd.c  echobbslib.c )
	(cd /tmp; diff -rcs innbbsd-$(LASTVERSION) innbbsd-$(VERSION) > /tmp/innbbsd-patch.$(LASTVERSION)-$(VERSION); echo "diff -rcs")
	(cd /tmp/innbbsd-$(VERSION); ln -s port.c r_port.c)
	(cd /tmp/innbbsd-$(VERSION); ln -s inndchannel.c dbzserver.c)
	(cd /tmp/innbbsd-$(VERSION); ln -s innbbsd.c dbzinnbbsd.c)
	(cd /tmp/innbbsd-$(VERSION); ln -s bbslib.c echobbslib.c)


distribution:
	rm -rf  /tmp/innbbsd-$(VERSION)
	test -d /tmp/innbbsd-$(VERSION) || mkdir /tmp/innbbsd-$(VERSION)
	test -d /tmp/innbbsd-$(VERSION)/innd || mkdir /tmp/innbbsd-$(VERSION)/innd
	rm -f r_port.c
	rm -f dbzserver.c
	rm -f dbzinnbbsd.c
	rm -f echobbslib.c
	cp -r doc hisconvert FEATURES CHANGES README* TODO *.c *.h *.y Makefile boards /tmp/innbbsd-$(VERSION)
	(cd /tmp/innbbsd-$(VERSION); ln -s port.c r_port.c)
	(cd /tmp/innbbsd-$(VERSION); ln -s inndchannel.c dbzserver.c)
	(cd /tmp/innbbsd-$(VERSION); ln -s innbbsd.c dbzinnbbsd.c)
	(cd /tmp/innbbsd-$(VERSION); ln -s bbslib.c echobbslib.c)
	(cd innd ; cp -r Makefile README innbbs.conf filter.ctl *.pl bntplink bntplink.palmbbs *.active* *.c *.y *.bbs src /tmp/innbbsd-$(VERSION)/innd)
	cp -r convert /tmp/innbbsd-$(VERSION)
	(cd /tmp/innbbsd-$(VERSION); make clean)
	(cd /tmp; tar zcf innbbsd-$(VERSION).tar.gz innbbsd-$(VERSION) ) 
	ln -s port.c r_port.c
	ln -s inndchannel.c dbzserver.c
	ln -s innbbsd.c dbzinnbbsd.c
	ln -s bbslib.c echobbslib.c

remotetest: distribution
	/usr/local/krb4/rcp /tmp/innbbsd-$(VERSION).tar.gz bbs_src.tar.gz skhuang:/tmp
	/usr/local/krb4/rcp /tmp/innbbsd-$(VERSION).tar.gz bbs_src.tar.gz linux:/tmp
	mv /u/staff/skhuang/.tcshrc /u/staff/skhuang/skhuang.tcshrc
	rcp /tmp/innbbsd-$(VERSION).tar.gz bbs_src.tar.gz ccsun36:/tmp
	rcp /tmp/innbbsd-$(VERSION).tar.gz bbs_src.tar.gz ccibm1:/tmp
	mv /u/staff/skhuang/skhuang.tcshrc /u/staff/skhuang/.tcshrc
	rcp /tmp/innbbsd-$(VERSION).tar.gz bbs_src.tar.gz cciris3:/tmp

ftp: distribution innbbsd-patch
	rsh ccsun42 mv /home8/ftp/pub/bbs/misc/innbbsd/innbbsd-$(VERSION).tar.gz /home8/ftp/pub/bbs/misc/innbbsd/.innbbsd-$(VERSION).tar.gz
	rcp /tmp/innbbsd-$(VERSION).tar.gz ccsun42:/home8/ftp/pub/bbs/misc/innbbsd
	rcp README* ccsun42:/home8/ftp/pub/bbs/misc/innbbsd
	rcp /tmp/innbbsd-patch.$(LASTVERSION)-$(VERSION) ccsun42:/home8/ftp/pub/bbs/misc/innbbsd

innbbspost.o: innbbspost.c
	$(CC) $(CFLAGS) -c innbbspost.c 

makedbz: $(MOBJS)
	$(CC) $(OPT) -o makedbz $(MOBJS) $(LDFLAGS)

dbzserver.o: dbzserver.c
	$(CC) $(CFLAGS) -DDBZSERVER -c dbzserver.c   

dbzinnbbsd.o: dbzinnbbsd.c
	$(CC) $(CFLAGS) -DDBZSERVER -c dbzinnbbsd.c   

dbzserver: $(DBZOBJS)
	$(CC) $(OPT) -o dbzserver $(DBZOBJS) $(LDFLAGS)

bbsnnrp: $(NNRPOBJS)
	$(CC) -o bbsnnrp $(NNRPOBJS) $(OPT) $(LDFLAGS)

bbslink: $(LINKOBJS)
	$(CC) -o bbslink $(LINKOBJS) $(OPT) $(LDFLAGS)

ctlinnbbsd: $(CTLOBJS)
	$(CC) $(OPT) -o ctlinnbbsd $(CTLOBJS) $(LDFLAGS)

mkhistory: $(HOBJS)
	$(CC) $(OPT) -o mkhistory $(HOBJS) $(LDFLAGS)

dbz_query_sample: dbz_query_sample.o
	$(CC) $(OPT) -o dbz_query_sample dbz_query_sample.o $(LDFLAGS)


innbbsd: $(OBJS)
#	mv tnrpd.exe tnrpd.exe.old
	$(CC) -o innbbsd $(OBJS) $(OPT) $(LDFLAGS) $(BBSLIB)

install:
	@echo "Read README and do the following manually"
	@echo "copy innbbsd bbsnnrp mkhistory to $(BBSHOME)/innd"	
	@echo "copy innd/bbspost innd/bntplink to $(BBSHOME)/innd"	
	test -d $(BBSHOME)/innd || mkdir $(BBSHOME)/innd
	@echo "$(BBSHOME)/innd must be owner by 'bbs'"
	-mv -f $(BBSHOME)/innd/innbbsd $(BBSHOME)/innd/innbbsd.OLD
	$(INSTALL) innbbsd $(BBSHOME)/innd/innbbsd
	-mv -f $(BBSHOME)/innd/bbsnnrp $(BBSHOME)/innd/bbsnnrp.OLD
	$(INSTALL) bbsnnrp $(BBSHOME)/innd/bbsnnrp
	-mv -f $(BBSHOME)/innd/ctlinnbbsd $(BBSHOME)/innd/ctlinnbbsd.OLD
	$(INSTALL) ctlinnbbsd $(BBSHOME)/innd/ctlinnbbsd
	-mv -f $(BBSHOME)/innd/bbspost $(BBSHOME)/innd/bbspost.OLD
	$(INSTALL) $(BBS_POST) $(BBSHOME)/innd/bbspost
	-mv -f $(BBSHOME)/innd/bntplink $(BBSHOME)/innd/bntplink.OLD
	$(INSTALL) innd/bntplink $(BBSHOME)/innd/bntplink
	-mv -f $(BBSHOME)/innd/mkhistory $(BBSHOME)/innd/mkhistory.OLD
	$(INSTALL) mkhistory $(BBSHOME)/innd/mkhistory
#	-mv -f $(BBSHOME)/innd/parsedate $(BBSHOME)/innd/parsedate.OLD 
#	$(INSTALL) innd/parsedate $(BBSHOME)/innd/parsedate
	-mv -f $(BBSHOME)/innd/tcp.pl $(BBSHOME)/innd/tcp.pl.OLD 
	$(INSTALL) innd/tcp.pl $(BBSHOME)/innd/tcp.pl
	-mv -f $(BBSHOME)/innd/innbbslib.pl $(BBSHOME)/innd/innbbslib.pl.OLD 
	$(INSTALL) innd/innbbslib.pl $(BBSHOME)/innd/innbbslib.pl
	-mv -f $(BBSHOME)/innd/bbslink $(BBSHOME)/innd/bbslink.OLD 
	$(INSTALL) bbslink $(BBSHOME)/innd/bbslink

clean:
	rm -f *.o $(EXES) core innd/src/*.o
	cd convert/hztty; make clean

depend:
	 makedepend -- $(CFLAGS) -- $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

