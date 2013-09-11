BASEPATH = ..
include $(BASEPATH)/Makefile.Base
CFILE	:=boardrc.c misc.c record.c user.c article.c modetype.c docutil.c msg.c politics.c goodgbid.c binaryattach.c permissions.c bbseva.c attach.c regform.c announce.c board.c sectree.c sectreeop.c libiden.c notification.c
COBJS	:=$(CFILE:.c=.o)
.c.o:	;  $(CC) $(CFLAGS) -c $*.c
all: libythtbbs.a

ifeq (.depend,$(wildcard .depend))
include .depend
endif

depend dep .depend:
	gcc $(CFLAGS) -MM *.c >.depend

$(COBJS): .depend

libythtbbs.a:$(COBJS)
	ar r $@ $?
install:libythtbbs.a
	mkdir -p $(BBSHOME)/bin
	cp libythtbbs.a $(BBSHOME)/bin
clean:
	rm -fr $(COBJS) libythtbbs.a seclist.local gensectree gensectree.o sectree.c
seclist.local:
	if [ -f ../site/seclist.txt ];then ln -sf ../site/seclist.txt seclist.local;else ln -sf seclist.txt seclist.local;fi
sectree.c: gensectree seclist.local sectree.h
	./gensectree> sectree.c
gensectree: gensectree.o
	$(CC) $(CFLAGS) $^ -L$(BBSHOME)/bin -lytht -o $@
