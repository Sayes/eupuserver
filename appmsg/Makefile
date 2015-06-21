
OBJS = src/eupu.base.pb.o

ifeq ($(ARCH), 64)
include ../common64.mak
else
include ../common.mak
endif

RELEASE_DIR = ../release

CFLAGS += -DOS_LINUX $(LIB_CFLAGS)

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
