# Makefile of QKMJ

CC=gcc

qkmj: qkmj.o input.o screen.o socket.o command.o message.o check.o checkscr.o card.o chkmake.o misc.o 
	$(CC) -g qkmj.o input.o screen.o socket.o command.o message.o check.o checkscr.o card.o chkmake.o misc.o -o qkmj -lncurses -ltermcap -lm

qkmj.o: qkmj.c input.h socket.h mjdef.h
	$(CC) -c qkmj.c -g

input.o: input.c qkmj.h mjdef.h misc.h
	$(CC) -c input.c -g

screen.o: screen.c qkmj.h mjdef.h
	$(CC) -c screen.c -g

socket.o: socket.c qkmj.h mjdef.h
	$(CC) -c socket.c -g

command.o: command.c qkmj.h mjdef.h
	$(CC) -c command.c -g

message.o: message.c qkmj.h mjdef.h misc.h
	$(CC) -c message.c -g

check.o: check.c qkmj.h mjdef.h
	$(CC) -c check.c -g

checkscr.o: checkscr.c qkmj.h mjdef.h
	$(CC) -c checkscr.c -g

card.o: card.c qkmj.h mjdef.h
	$(CC) -c card.c -g

chkmake.o: chkmake.c qkmj.h mjdef.h
	$(CC) -c chkmake.c -g

misc.o: misc.c qkmj.h mjdef.h
	$(CC) -c misc.c -g
