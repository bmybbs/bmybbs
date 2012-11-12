BASEPATH = ..
include $(BASEPATH)/Makefile.Base
CFILE	:=boardrc.c misc.c record.c user.c article.c modetype.c docutil.c msg.c politics.c goodgbid.c binaryattach.c permissions.c bbseva.c attach.c regform.c announce.c board.c sectree.c sectreeop.c libiden.c
COBJS	:=$(CFILE:.c=.o)
.c.o:	;  $(CC) -c $*.c -D_GNU_SOURCE $(CFLAGS) $(OS_DEF) -g -L/usr/lib/mysql -lmysqlclient
all: libythtbbs.a

ifeq (.depend,$(wildcard .depend))
include .depend
endif

depend dep .depend:
	gcc $(CFLAGS) -M *.c >.depend

$(COBJS): .depend

libythtbbs.a:$(COBJS)
	ar r $@ $?
install:libythtbbs.a
	mkdir -p $(BBSHOME)/bin
	cp libythtbbs.a $(BBSHOME)/bin
clean:
	rm -fr $(COBJS) libythtbbs.a seclist.local
seclist.local:
	if [ -f ../site/seclist.txt ];then ln -sf ../site/seclist.txt seclist.local;else ln -sf seclist.txt seclist.local;fi
sectree.c: gensectree seclist.local sectree.h
	./gensectree> sectree.c
gensectree: gensectree.o
	$(CC) $(CFLAGS) -o $@ $^ -L$(BBSHOME)/bin -lytht 
