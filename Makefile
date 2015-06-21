

os_type = $(shell uname -s)
os_arch = $(shell uname -m)

ifeq ($(os_type), Linux)
MAKE = make
endif

ifeq ($(os_arch), x86_64)
ARCH = 64
endif

SUBDIRS = common logger protocol network appmsg usercentersvr testproj

INCLUDE_DIR = 

all:
	-for a in $(SUBDIRS); do cd $$a; $(MAKE) -f Makefile $@; cd ..; done; exit 0

clean:
	-for a in $(SUBDIRS); do cd $$a; $(MAKE) -f Makefile $@; cd ..; done; exit 0


