Ma.PHONY:all install clean
all:
	$(MAKE) -C ythtlib
	$(MAKE) -C libythtbbs
	$(MAKE) -C src proto
	$(MAKE) -C src depend
	$(MAKE) -C src
	$(MAKE) -C software installghthash
	$(MAKE) -C local_utl
	$(MAKE) -C nju09 proto
	$(MAKE) -C nju09 depend
	$(MAKE) -C nju09
install:
	$(MAKE) -C ythtlib install
	$(MAKE) -C libythtbbs install
	$(MAKE) -C src install
	$(MAKE) -C local_utl install
	$(MAKE) -C nju09 install
	$(MAKE) -C randpics install
clean:
	$(MAKE) -C ythtlib clean
	$(MAKE) -C libythtbbs clean
	$(MAKE) -C src clean
	$(MAKE) -C local_utl clean
	$(MAKE) -C nju09 clean

