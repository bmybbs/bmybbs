BINDIR=/usr/local/bin
LIBDIR=/usr/local/lib/sinocode
MANDIR=/usr/local/man

CC=gcc
CFLAGS=-ansi -g

COD=bg bj gb gj jb jg

DISTFILES=COPYING README Makefile sinocode.c sinocode.man ChangeLog *.cod

DISTFILENAME=sinocode-0.11.tar

all: sinocode

sinocode: sinocode.c
	$(CC) $(CFLAGS) -o sinocode sinocode.c \
		-DTRANSLATIONSFILESDIR=\"${LIBDIR}/\"

install: sinocode
	install -m 755 sinocode ${BINDIR}
	if test ! -d ${LIBDIR}; then mkdir ${LIBDIR}; fi
	for F in ${COD}; do \
	  tr -d '\015\032' < sinoco$$F.cod > ${LIBDIR}/sinoco$$F.cod; \
	done
	cp sinocode.man ${MANDIR}/



dist:	$(DISTFILES)
	tar cvf $(DISTFILENAME) $(DISTFILES);\
	compress $(DISTFILENAME)

uu-dist:
	uuencode $(DISTFILENAME).Z $(DISTFILENAME).Z > $(DISTFILENAME).Z.uu
