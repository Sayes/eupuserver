
CC = gcc -g
CXX = g++ -g
LINK = g++ -g

LOG4CXX_PATH = /usr/local/log4cxx
APR_PATH = /usr/local/apr
APRUTIL_PATH = /usr/local/apr-util
PROTOBUF_PATH = /usr/local/protobuf
JSON_PATH = /usr/local/json

TS_DIR = /home/syz/eupuserver-code/trunk
#TS_DIR = /home/shenyizhong/workshop/projects/eupuserver-code/trunk

TS_INCLUDE = $(TS_DIR)/include

os_type = $(shell uname -s)

ifeq ($(os_type), Linux)
LDFLAGS = -shared -m32
CFLAGS = -O0 -g3 -Wall -c -fmessage-length=0 -m32
LIB_CFLAGS = -I$(APR_PATH)/include -I$(APRUTIL_PATH) -I$(LOG4CXX_PATH)/include -I$(PROTOBUF_PATH)/include -I$(JSON_PATH)/include -I$(TS_INCLUDE)/common -I$(TS_INCLUDE)/network -I$(TS_INCLUDE)/logger -I$(TS_INCLUDE)/protocol -I$(TS_INCLUDE)/appmsg
LIB_LFLAGS = -L$(APR_PATH)/lib -L$(APRUTIL_PATH)/lib -L$(LOG4CXX_PATH)/lib -L$(PROTOBUF_PATH)/lib -L$(JSON_PATH)/lib

MAKE = make
endif


.SUFFIXES:
.SUFFIXES: .cpp .cc .c .o
.cpp.o:; $(CXX) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" $< -o $@
.cc.o:; $(CXX) $(CFLAGS) $< -o $@
.c.o:; $(CXX) $(CFLAGS) $< -o $@

