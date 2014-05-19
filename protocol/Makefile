



OBJS = src/eupustream.o src/protocol.o 


include ../common.mak


RELEASE_DIR = ../release
LIB_DIR = ../lib

CFLAGS += -DOS_LINUX $(LIB_CFLAGS)

TARGETS = libprotocol.a


all: $(TARGETS)

libprotocol.a: $(OBJS)
	test -d ../release || mkdir -p ../release
	test -d ../lib || mkdir -p ../lib
	ar -r $@ $^
	cp $@ $(RELEASE_DIR)
#cp $@ $(LIB_DIR)

clean:
	rm -f $(OBJS) $(TARGETS)
	rm -f *.o src/*.d
	rm -f $(RELEASE_DIR)/libprotocol.a
#rm -f $(LIB_DIR)/libprotocol.a
