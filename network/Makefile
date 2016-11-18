OBJS = src/netcommon.o src/threadlock.o src/euputhread.o src/epollthread.o src/globalmgr.o src/workbasethread.o

include ../common.mak

RELEASE_DIR = ../release

CFLAGS += -DOS_LINUX $(INC_CFLAGS)

TARGETS = libnetwork.so

all: $(TARGETS)

$(TARGETS): $(OBJS)
	test -d ../release || mkdir -p ../release
	$(LINK) $(LFLAGS) -o $@ $^
	cp $@ $(RELEASE_DIR)

clean:
	rm -f $(OBJS) $(TARGETS)
	rm -f *.o src/*.d
	rm -f $(RELEASE_DIR)/libnetwork.so

