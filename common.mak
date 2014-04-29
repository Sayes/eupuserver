

CC = gcc -g
CXX = g++ -g

LINK = g++ -g

LOG4CXX_INCLUDE_PATH = /usr/local/log4cxx
TS_DIR = /home/syz/tinyserver-code/trunk
TS_INCLUDE = $(TS_DIR)/include

LFLAGS = -L$(LIB_DIR)

os_type = $(shell uname -s)
ifeq ($(os_type), Linux)
LDFLAGS = -shared -m32
CFLAGS = -O0 -g3 -Wall -c -fmessage-length=0 -m32 -I.
LIB_CFLAGS = -I$(TS_INCLUDE)/common -I$(TS_INCLUDE)/network

MAKE = make
endif


.SUFFIXES:
.SUFFIXES: .cpp .c .o
.cpp.o:; $(CXX) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" $< -o $@
.c.o:; $(CXX) $(CFLAGS) $< -o $@


