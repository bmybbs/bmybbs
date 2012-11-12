BASEPATH = ../..
include $(BASEPATH)/Makefile.Base

BIN:= an anall
.PHONY:all install clean
all:$(BIN)
an: an.c
	$(CC) -o $@ $(CFLAGS) $< $(BBSLIB)
anall:anall.c
	$(CC) -o $@ $< $(CFLAGS)

all: $(BIN)
install: all
	cp $(BIN) /home/bbs/bin
clean:
	rm -f $(BIN)
