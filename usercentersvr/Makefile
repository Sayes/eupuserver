OBJS = src/main.o src/workthread.o
 
#  
# include standard include file
#
include ../common.mak

# RELEASE dir definition
RELEASE_DIR = ../release

CFLAGS += -DOS_LINUX $(LIB_CFLAGS) -I./src
LDFLAGS = $(LIB_LFLAGS) -L$(RELEASE_DIR)

TARGETS = usercentersvr 

all: $(TARGETS) 

$(TARGETS): $(OBJS)
	test -d ../release || mkdir -p ../release
	$(LINK) $(LDFLAGS) -o $@ $^ -lnetwork -lprotocol -leupulogger -llog4cxx -lcommon -leupu.base.pb -lprotobuf -lprotobuf-lite -lprotoc -ljsoncpp -lapr-1 -laprutil-1  
	cp $@ ${RELEASE_DIR}


clean:
	rm -f $(OBJS) $(TARGETS)
	rm -f src/*.o src/*.d
	rm -f ${RELEASE_DIR}/$(TARGETS)
