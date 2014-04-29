

os_type = $(shell uname -s)

ifeq ($(os_type), Linux)
MAKE = make
endif

SUBDIRS = common network logger protocol

INCLUDE_DIR = 

all:
	-for a in $(SUBDIRS); do cd $$a; $(MAKE) -f Makefile $@; cd ..; done; exit 0

clean:
	-for a in $(SUBDIRS); do cd $$a; $(MAKE) -f Makefile $@; cd ..; done; exit 0


