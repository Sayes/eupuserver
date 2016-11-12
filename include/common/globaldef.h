#ifndef __GLOBAL_DEF__
#define __GLOBAL_DEF__

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#ifdef OS_LINUX
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#elif OS_WINDOWS
#include <ws2tcpip.h>
#endif
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <deque>
#include <map>
#include <list>
#include <limits>

using namespace std;

#ifdef OS_LINUX
typedef __uint8_t BYTE;
#elif OS_WINDOWS
#include <WinDef.h>
#include <stdint.h>
#define snprintf sprintf_s
#define vsnprintf vsnprintf_s
#endif

#define MAX_SEND_SIZE   2048
#define NET_HEAD_SIZE   8


#endif//__GLOBAL_DEF__
