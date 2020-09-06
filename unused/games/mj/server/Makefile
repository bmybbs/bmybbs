SOURCE = mjrec.c
OBJECT = test_gf.o rscoder.o rs.o
TARGET = mjgps mjrec

CC = gcc 

CFLAGS = -O2 -m486 -fomit-frame-pointer -pipe
  
LDFLAGS = $(CFLAGS) -s

.c.o:
	$(CC) -c $(CFLAGS) $<
 
all: $(TARGET)

mjgps: mjgps.h mjgps.c
	$(CC) $(CFLAGS) -lcrypt  -s -o $@ mjgps.c
	strip $@

mjrec: mjgps.h mjrec.c
	$(CC) $(CFLAGS) -s -N -o $@ mjrec.c
	strip $@

install: all
	install -c -s -g bin -m 700 $(TARGET) /usr/local/games/bin

format:
	indent -bad -bap -bli0 -cdb -psl -sc -ss -i4 *.c
	rm -f *.{c,h}~

clean:
	rm -f $(TARGET) $(OBJECT) *.a *~ *.o core
