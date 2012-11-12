BASEPATH = ..
include $(BASEPATH)/Makefile.Base

PROGNAME  = atthttpd
CFILE = main.c sv_core.c conn.c mimetype.c
COBJS   :=$(CFILE:.c=.o)
.c.o:   ;  $(CC) -c $*.c $(OS_DEF) $(CFLAGS) $(CPPFLAGS)

all: $(PROGNAME)

ifeq (.depend,$(wildcard .depend))
include .depend
endif

depend dep .depend:
	gcc $(CFLAGS) -M $(CFILE) >.depend

$(COBJS):  .depend

$(PROGNAME): $(COBJS)
	$(CC) -o $(PROGNAME) $(COBJS) $(LIBS) $(BBSLIB) -lghthash

clean:
	rm -f $(COBJS) $(PROGNAME)

