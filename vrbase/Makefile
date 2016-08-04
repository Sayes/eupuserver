OBJS = src/vrbase.o src/IOStreamOutputter.o

include ../common.mak

RELEASE_DIR = ../release

TARGETS = libvrbase.a

all: $(TARGETS)

libvrbase.a: $(OBJS)
	test -d ../release || mkdir -p ../release
	ar -r $@ $^
	cp $@ $(RELEASE_DIR)

clean:
	rm -f $(OBJS) $(TARGETS)
	rm -f *.o src/*.d
	rm -f $(RELEASE_DIR)/libvrbase.a

