

OBJS = src/netcommon.o src/threadlock.o src/euputhread.o src/epollthread.o src/globalmgr.o src/workbasethread.o


include ../common.mak

RELEASE_DIR = ../release
LIB_DIR = ../lib

CFLAGS += -DOS_LINUX $(LIB_CFLAGS)

#LDFLAGS += -L$(RELEASE_DIR)
TARGETS = libnetwork.so

all: $(TARGETS)

$(TARGETS): $(OBJS)
	test -d ../release || mkdir -p ../release
	$(LINK) $(LDFLAGS) -o $@ $^
	cp $@ $(RELEASE_DIR)

clean:
	rm -f $(OBJS) $(TARGETS)
	rm -f *.o src/*.d
	rm -f $(RELEASE_DIR)/libnetwork.so

