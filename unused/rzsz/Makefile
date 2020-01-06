BASEPATH = ..
include $(BASEPATH)/Makefile.Base
CC	= gcc -Wall -g
CFILE	:=canit.c crctab.c lsz.c rbsb.c zm.c zreadline.c
COBJS	:=$(CFILE:.c=.o)
.c.o:	;  $(CC) -c $*.c $(CFLAGS)
all: librzsz.a

ifeq (.depend,$(wildcard .depend))
include .depend
endif

depend dep .depend:
	gcc $(CFLAGS) -M *.c >.depend

$(COBJS): .depend

librzsz.a:$(COBJS)
	ar r $@ $?
install:librzsz.a
	mkdir -p /home/bbs/bin
	cp librzsz.a /home/bbs/bin
clean:
	rm -fr $(COBJS) librzsz.a
