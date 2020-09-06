#ecnegrevid 2001.7.20
BASEPATH = ../..
include $(BASEPATH)/Makefile.Base

PROGNAME = recite
CFILE = recite.c items.c
COBJS = recite.o items.o
COBJSSTEST = recite.t items.t
.SUFFIXES: .o .c .ln .t

.c.o:   ;   gcc -c $(CFLAGS) $*.c

.c.t:   ;   gcc -c $(CFLAGS) $*.c -o $*.t

all: $(PROGNAME)

ifeq (.depend,$(wildcard .depend))
include .depend
endif

depend dep:
	gcc $(CFLAGS) -M *.c >.depend

$(PROGNAME): $(COBJS)
	gcc -g -o $(PROGNAME) $(COBJS) -lnewt
