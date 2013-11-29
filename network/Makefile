

OBJS = src/netcommon.o


include ../common.mak

RELEASE_DIR = ../release
LIB_DIR = ../lib

CFLAGS += -DOS_LINUX $(LIB_CFLAGS)

LDFLAGS += -L$(RELEASE_DIR)
TARGETS = libnetwork.so

all: $(TARGETS)

libnetwork.so : $(OBJS)
	test -d ../release || mkdir -p ../release
	$(LINK) $(LDFLAGS) -o $@ $^ -lcommon
	cp $@ $(RELEASE_DIR)
	cp $@ $(LIB_DIR)

clean:
	rm -f $(OBJS) $(TARGETS)
	rm -f *.o src/*.d
	rm -f $(RELEASE_DIR)/libnetwork.so
	rm -f $(LIB_DIR)/libnetwork.so

