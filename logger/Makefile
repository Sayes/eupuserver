#
# Makefile of common class library for Solaris
#
OBJS = src/eupulogger.o src/eupulogger4system.o	
 
#  
# include standard include file
#
include ../common.mak

# RELEASE dir definition
RELEASE_DIR = ../release
CFLAGS += -DOS_LINUX $(LIB_CFLAGS)

TARGETS = libeupulogger.so

all: $(TARGETS) 

$(TARGETS) : $(OBJS)
	test -d ../release || mkdir -p ../release
	$(LINK) $(LDFLAGS) -o $@ $^
	cp $@ ${RELEASE_DIR}

clean:
	rm -f $(OBJS) $(TARGETS)
	rm -f *.o *.so src/*.d
	rm -f ${RELEASE_DIR}/$(TARGETS)
