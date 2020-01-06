#ylsdd 2002/11/19
BASEPATH = ../..
include $(BASEPATH)/Makefile.Base
PROGNAME = cgifreeip freeip
.SUFFIXES: .o .c

.c.o:   ;   gcc -g -c $*.c $(CFLAGS)

all: $(PROGNAME)

ifeq (.depend,$(wildcard .depend))
include .depend
endif

depend dep:
	gcc $(CFLAGS) -M *.c >.depend
cgifreeip: cgifreeip.o displayip.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)
freeip: freeip.o displayip.o
	$(CC) $(CFLAGS) -o $@ $^
clean:
	rm *.o
	rm $(PROGNAME)
