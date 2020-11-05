CC = gcc
CXX = g++
LINK = g++

LOG4CXX_PATH = $(LOG4CXX_HOME)
APR_PATH = $(APR_HOME)
APRUTIL_PATH = $(APR_UTIL_HOME)
PROTOBUF_PATH = $(PROTOBUF_HOME)
JSONCPP_PATH = $(JSONCPP_HOME)
XERCES_PATH = /usr/local/xerces
XML_SECURITY_PATH = /usr/local/xml-security-c

TS_DIR = /home/$(USER)/workshop/projects/eupuserver
TS_INCLUDE = $(TS_DIR)/include

os_type = $(shell uname -s)
os_arch = $(shell uname -m)

ifeq ($(os_arch), x86_64)
OSARCH = -m64
endif

ifeq ($(os_type), Linux)
CFLAGS = -O0 -g3 -Wall -fPIC -c -fmessage-length=0 -std=c++11 $(OSARCH)
LFLAGS = -shared $(OSARCH)

INC_CFLAGS = -I$(APR_PATH)/include -I$(APRUTIL_PATH) -I$(LOG4CXX_PATH)/include -I$(PROTOBUF_PATH)/include -I$(JSONCPP_PATH)/include -I$(XERCES_PATH)/include -I$(XML_SECURITY_PATH)/include -I$(TS_INCLUDE)
LIB_LFLAGS = -L$(APR_PATH)/lib -L$(APRUTIL_PATH)/lib -L$(LOG4CXX_PATH)/lib -L$(PROTOBUF_PATH)/lib -L$(JSONCPP_PATH)/lib -L$(XERCES_PATH)/lib -L$(XML_SECURITY_PATH)/lib

MAKE = make
endif

.SUFFIXES:
.SUFFIXES: .cpp .cc .c .o
.cpp.o:; $(CXX) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" $< -o $@
.cc.o:; $(CXX) $(CFLAGS) $< -o $@
.c.o:; $(CXX) $(CFLAGS) $< -o $@ 
