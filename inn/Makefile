BASEPATH = ..
include $(BASEPATH)/Makefile.Base
.PHONY:all install clean
bin:=bbsfindout bbsinnd bbsoutd
inst=$(patsubst %,$(BBSHOME)/bin/%,$(bin))
all:bbsfindout bbsinnd bbsoutd
bbsinnd: bbsinnd.c
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)
bbsoutd: bbsoutd.c
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)
bbsfindout: bbsfindout.c
	$(CC) $(CFLAGS) -o $@ $^ $(BBSLIB)
install:$(inst)
clean:
	rm -fr $(bin)
