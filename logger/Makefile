OBJS = src/eupulogger.o src/eupulogger4system.o	
 
include ../common.mak

RELEASE_DIR = ../release

CFLAGS += -DOS_LINUX

TARGETS = libeupulogger.so

all: $(TARGETS) 

$(TARGETS) : $(OBJS)
	test -d ../release || mkdir -p ../release
	$(LINK) $(LFLAGS) -o $@ $^
	cp $@ ${RELEASE_DIR}

clean:
	rm -f $(OBJS) $(TARGETS)
	rm -f *.o *.so src/*.d
	rm -f ${RELEASE_DIR}/$(TARGETS)
