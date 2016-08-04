OBJS = src/main.o src/workthread.o
 
include ../common.mak

RELEASE_DIR = ../release

CFLAGS += -DOS_LINUX -I./src

LFLAGS += -L$(RELEASE_DIR)

TARGETS = usercentersvr 

all: $(TARGETS) 

$(TARGETS): $(OBJS)
	test -d ../release || mkdir -p ../release
	$(LINK) $(LFLAGS) -o $@ $^ -lnetwork -lprotocol -leupulogger -llog4cxx -lcommon -leupu.base.pb -lprotobuf -lprotobuf-lite -lprotoc -ljsoncpp -lapr-1 -laprutil-1  
	cp $@ ${RELEASE_DIR}


clean:
	rm -f $(OBJS) $(TARGETS)
	rm -f src/*.o src/*.d
	rm -f ${RELEASE_DIR}/$(TARGETS)
