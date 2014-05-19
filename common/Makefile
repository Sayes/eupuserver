
OBJS = src/common.o src/exceptionhandle.o src/utility.o src/globalconfig.o

include ../common.mak

RELEASE_DIR = ../release

CFLAGS += -DOS_LINUX $(LIB_CFLAGS)

TARGETS = libcommon.a

all: $(TARGETS)

libcommon.a: $(OBJS)
	test -d ../release || mkdir -p ../release
	ar -r $@ $^
	cp $@ $(RELEASE_DIR)

clean:
	rm -f $(OBJS) $(TARGETS)
	rm -f *.o src/*.d
	rm -f $(RELEASE_DIR)/libcommon.a
