SUBDIRS = common logger protocol network appmsg usercentersvr testproj

all:
	-for a in $(SUBDIRS); do cd $$a; $(MAKE) -f Makefile $@; cd ..; done; exit 0

clean:
	-for a in $(SUBDIRS); do cd $$a; $(MAKE) -f Makefile $@; cd ..; done; exit 0


