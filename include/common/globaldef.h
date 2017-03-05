// Copyright shenyizhong@gmail.com, 2014

#ifndef COMMON_GLOBAL_DEF_
#define COMMON_GLOBAL_DEF_

#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#ifdef OS_LINUX
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#elif OS_WINDOWS
#include <ws2tcpip.h>
#endif
#include <deque>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <queue>
#include <string>
#include <vector>

#ifdef OS_LINUX
typedef __uint8_t BYTE;
#elif OS_WINDOWS
#include <WinDef.h>
#include <stdint.h>
#define snprintf sprintf_s
#define vsnprintf vsnprintf_s
#endif

#define MAX_SEND_SIZE 2048
#define NET_HEAD_SIZE 8

#endif  // COMMON_GLOBAL_DEF_
