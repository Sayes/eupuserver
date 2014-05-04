#
# Makefile of common class library for Solaris
#
OBJS = src/main.o
 
#  
# include standard include file
#
include ../common.mak


# RELEASE dir definition
RELEASE_DIR = ../release
LIB_DIR = ../lib

#CFLAGS += -DSUPPORT_NEW_DB -DOS_LINUX -D_SERVICE_PROTOCOL -D_CE_PROTOCOL -D_CM_PROTOCOL -D_CR_PROTOCOL -D_DE_PRTOCOL -D_DM_PROTOCOL -D_DR_PROTOCOL -D_DISTRIBUTE_PROTOCOL -D_MONITOR_PROTOCOL -D_SYSADMIN_PROTOCOL -D_LOG_PROTOCOL -DHAVE_MSGHDR_MSG_CONTROL -I$(LOG4CXX_INCLUDE_PATH) -I$(MYSQL_INCLUDE_PATH) -I$(MEMCACHED_INCLUDE_PATH) -I$(MEMCACHED_EXT_INCLUDE_PATH) $(LIB_CFLAGS) -I$(FGAME_DIR)/GmLogSvr/src

CFLAGS += -I$(LOG4CXX_INCLUDE_PATH) $(LIB_CFLAGS)

LDFLAGS += -L$(RELEASE_DIR) -L/usr/local/apr/lib

TARGETS = TestProj 

all: $(TARGETS) 

TestProj: $(OBJS)
	test -d ../release || mkdir -p ../release
	$(LINK) $(LDFLAGS) -o $@ $^ -Wreturn-type -lcommon -leupulogger -lprotocol -lnetwork -llog4cxx -lapr-1 -laprutil-1
	cp $@ ${RELEASE_DIR}

clean:
	rm -f $(OBJS) $(TARGETS)
	rm -f *.o src/*.d
	rm -f ${RELEASE_DIR}/TestProj
