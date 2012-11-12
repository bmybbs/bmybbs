#ecnegrevid 2001.7.20
BASEPATH = ../..
include $(BASEPATH)/Makefile.Base

PROGNAME = ncce cgincce
.SUFFIXES: .o .c

.c.o:   ;   gcc -g -c $*.c $(CFLAGS)

all: $(PROGNAME)

ifeq (.depend,$(wildcard .depend))
include .depend
endif

depend dep:
	gcc $(CFLAGS) -M *.c >.depend
ncce: libncce.o ncce.o
	gcc -g -o ncce libncce.o ncce.o -lnewt $(CFLAGS)
cgincce: libncce.o cgincce.o
	gcc -g -o cgincce libncce.o cgincce.o $(CFLAGS)
clean:
	rm *.o
	rm ncce
