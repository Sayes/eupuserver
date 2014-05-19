
OBJS = src/eupu.base.pb.o

include ../common.mak

RELEASE_DIR = ../release

CFLAGS += -DOS_LINUX $(LIB_CFLAGS)

TARGETS = libeupu.base.pb.a

all: $(TARGETS)

libeupu.base.pb.a: $(OBJS)
	test -d ../release || mkdir -p ../release
	ar -r $@ $^
	cp $@ $(RELEASE_DIR)

clean:
	rm -f $(OBJS) $(TARGETS)
	rm -f *.o src/*.d
	rm -f $(RELEASE_DIR)/libeupu.base.pb.a
