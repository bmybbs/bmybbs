BASEPATH = ..
include $(BASEPATH)/Makefile.Base

#UTILITY  = repassold repass266 mergeb account averun bfinger newacct poststat usage showuser refriend2 autoundeny autoclear bbstop auto_rm_junk userip tianqi annIndex
UTILITY  =  finddf mergeb averun showuser refriend2 autoundeny autoclear bbstop auto_rm_junk userip tianqi annIndex  nbstat 2nd_expire bbsstatlog bbsstatproclog clear_junk clear_attach searchLastMark sortdir printSecLastMark searchDIR auto_rm_fakedir bbslogd MakeIndex3 bm bm2 filter find_lost_mail find_rm_lost fixdir id_boards id_distr logfreq near ptyexec reminder save_brc searchauther selpersonal showwhoperm sumcgitime tuijian upersonal cpersonal transuu2bin bbspop3d combine_arc repsync upnew newtop10 newboards printSecLastUpdate check_ulevel setdefaultkey \
watchman upgrade postfile 

##  bbsevad
ifeq (.depend,$(wildcard .depend))
include .depend
endif

.c.o:   ;   $(CC) $(CFLAGS) $(CPPFLAGS) $(DEFINES) -c -g $*.c 

all: $(UTILITY)

depend dep:
	gcc $(CFLAGS) -M *.c $(BASEPATH)/src/record.c >.depend

install: $(UTILITY)
	chmod 0700 $(UTILITY)
	cp -f $(UTILITY) $(BBSHOME)/bin

clean:
	rm -fr *.o *.BAK *.log $(UTILITY)
	rm -fr core .depend

averun: averun.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

bfinger: bfinger.o
	$(CC) $(CFLAGS) -o bfinger bfinger.o

usage: usage.o
	$(CC) $(CFLAGS) -o usage usage.o $(BBSLIB)

auto_rm_junk: auto_rm_junk.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

searchDIR: searchDIR.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

clear_junk: clear_junk.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

printSecLastMark: printSecLastMark.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

printSecLastUpdate: printSecLastUpdate.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

clear_attach: clear_attach.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

nbstat: nbstat.o bmstat.o boardstat2.o boardscore.o bbslists.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

sortdir:sortdir.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

mergeb: mergeb.o record.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

2nd_expire: 2nd_expire.o record.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

chboard: chboard.o record.o
	$(CC) $(CFLAGS) -o $@ $^

refriend2: refriend2.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

repassold: repassold.o
	$(CC) $(CFLAGS) -o $@ $^

repass266: repass266.o
	$(CC) $(CFLAGS) -o repass266 repass266.o

autoundeny: autoundeny.o sysrecord.o 
	$(CC) $(CFLAGS) -o autoundeny autoundeny.o sysrecord.o $(BBSLIB)

autoclear: autoclear.o sysrecord.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

bbspop3d:  bbspop3d.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

postfile:  postfile.o sysrecord.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

bbstop: bbstop.o
	$(CC) $(CFLAGS) -o $@ $^

noOUT: noOUT_FLAG.o
	$(CC) $(CFLAGS)  -o $@ $^
ptyexec: ptyexec.o
	$(CC) $(CFLAGS) -lutil -o $@ $^

searchLastMark: searchLastMark.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

trans_brc: trans_brc.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)
save_brc: save_brc.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

find_lost_mail: find_lost_mail.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

find_rm_lost: find_rm_lost.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

find_save_lost: find_save_lost.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

fixdir: fixdir.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

trans_fhbknh: trans_fhbknh.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

cpersonal: cpersonal.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

bbsstatlog: bbsstatlog.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

transuu2bin: transuu2bin.c
	$(CC) -D_GNU_SOURCE $(CFLAGS) -o $@ $^ $(BBSLIB)

combine_arc: combine_arc.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

repsync: repsync.o
	$(CC) $(CFLAGS) -o $@ $^

bbsevad: bbsevad.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)  $(LIBS) $(LIBSQL)

selpersonal: selpersonal.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB) -lm

MakeIndex3: MakeIndex3.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

id_boards: id_boards.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

bm: bm.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

gen_title: gen_title.o
	 $(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

sortdir_all: sortdir_all.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

newsec: newsec.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)
finddf: finddf.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)
newtop10: newtop10.o sysrecord.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB) -lghthash

newboards: newboards.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

fixzapbuf: fixzapbuf.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

fixtitle: fixtitle.o
	$(CC) $(CFLAGSTEST) -o $@ $^ $(BBSLIB)

bbslogd: bbslogd.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB) -lghthash

watchman: watchman.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

mailallmsg: mailallmsg.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

changeuser: changeuser.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

upgrade: upgrade.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

check_ulevel: check_ulevel.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)

setdefaultkey: setdefaultkey.o
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)
