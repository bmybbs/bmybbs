#ecnegrevid 2001.7.20
BASEPATH = ../..
include $(BASEPATH)/Makefile.Base

PROGNAME = cgiman
.SUFFIXES: .o .c

.c.o:   ;   gcc -g -c $*.c $(CFLAGS)

all: $(PROGNAME)

ifeq (.depend,$(wildcard .depend))
include .depend
endif

depend dep:
	gcc $(CFLAGS) -M *.c >.depend
cgiman: cgiman.o
	gcc -g -o cgiman cgiman.o $(CFLAGS)
clean:
	rm *.o
	rm $(PROGNAME)
