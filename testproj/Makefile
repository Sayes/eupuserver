OBJS = src/main.o	
 
#  
# include standard include file
#
include ../common.mak

# RELEASE dir definition
RELEASE_DIR = ../release

CFLAGS += -DOS_LINUX -I$(LOG4CXX_INCLUDE_PATH) $(LIB_CFLAGS) -I$(TS_INCLUDE)/logger -I$(TS_INCLUDE)/common -I$(TS_INCLUDE)/protocol -I$(TS_INCLUDE)/network 
LDFLAGS = -L$(RELEASE_DIR) -L/usr/local/apr/lib -m32

TARGETS = testproj 

all: $(TARGETS) 

testproj: $(OBJS)
	test -d ../release || mkdir -p ../release
	$(LINK) $(LDFLAGS) -o $@ $^ -lpthread -lcommon -leupulogger -lprotocol -lnetwork -llog4cxx -lapr-1 -laprutil-1 
	cp $@ ${RELEASE_DIR}


clean:
	rm -f $(OBJS) $(TARGETS)
	rm -f src/*.o src/*.d
	rm -f ${RELEASE_DIR}/testproj
