OBJS = src/eupustream.o src/protocol.o 

include ../common.mak

RELEASE_DIR = ../release

CFLAGS += -DOS_LINUX $(INC_CFLAGS)

TARGETS = libprotocol.a

all: $(TARGETS)

$(TARGETS): $(OBJS)
	test -d ../release || mkdir -p ../release
	ar -r $@ $^
	cp $@ $(RELEASE_DIR)

clean:
	rm -f $(OBJS) $(TARGETS)
	rm -f *.o src/*.d
	rm -f $(RELEASE_DIR)/$(TARGETS)
