BASEPATH = ..
include $(BASEPATH)/Makefile.Base
CC	= gcc -Wall -g
CFILE	:=uudecode.c strhash.c common.c numbyte.c strop.c fileop.c strlib.c shmop.c uuencode.c named_socket.c crypt.c limitcpu.c timeop.c smth_filter.c mgrep.c
COBJS	:=$(CFILE:.c=.o)
.c.o:	;  $(CC) -c $*.c $(CFLAGS) -D_GNU_SOURCE
all: libytht.a

ifeq (.depend,$(wildcard .depend))
include .depend
endif

depend dep .depend:
	gcc $(CFLAGS) -MM *.c >.depend

$(COBJS): .depend

libytht.a:$(COBJS)
	ar r $@ $?
install:libytht.a
	mkdir -p $(BBSHOME)/bin
	cp libytht.a $(BBSHOME)/bin
clean:
	rm -fr $(COBJS) libytht.a .depend
