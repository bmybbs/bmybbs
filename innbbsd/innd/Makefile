# Makefile for bbsinnd

#  The source code directory for BBS (include bbs.h, ...)

BASEPATH = ../..
include $(BASEPATH)/Makefile.Base

OBJ  = bbspost.o
all:	bbspost

COBJS   :=$(CFILE:.c=.o)
.c.o:   ;  $(CC) -c $*.c -D_GNU_SOURCE $(CFLAGS) -Wall

bbspost: ${OBJ}
	${CC} ${CFLAGS} -o bbspost ${OBJ} -lcrypt ${BBSLIB}
install:
	cp mailpost /home/bbs/innd
	cp expirepost /home/bbs/innd
	cp bbspost /home/bbs/innd

clean:
	${RM} *.o bbspost

