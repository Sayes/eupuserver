
OBJS = src/eupu.base.pb.o

include ../common.mak

RELEASE_DIR = ../release

CFLAGS += -DOS_LINUX

TARGETS = libeupu.base.pb.a

all: $(TARGETS)

$(TARGETS): $(OBJS)
	test -d ../release || mkdir -p ../release
	ar -r $@ $^
	cp $@ $(RELEASE_DIR)

clean:
	rm -f $(OBJS) $(TARGETS)
	rm -f *.o src/*.d
	rm -f $(RELEASE_DIR)/$(TARGETS)
