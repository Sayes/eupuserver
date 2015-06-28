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
/* 20150625
typedef __uint32_t UINT;
typedef __uint32_t ULONG;
typedef __int32_t LONG;
typedef __uint8_t BYTE;
typedef __uint32_t DWORD;
typedef __uint16_t USHORT;
typedef __uint16_t WORD;
typedef __uint64_t UINT64;
typedef __int64_t INT64;
typedef short SHORT;
typedef __int8_t INT8;
typedef __int32_t INT32;
*/

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
typedef uint8_t BYTE;

#elif OS_WINDOWS

#include <WinDef.h>
#define snprintf sprintf_s
#define vsnprintf vsnprintf_s
typedef signed __int64 int64;
typedef signed __int32 int32;
typedef signed __int16 int16;
typedef signed __int8 int8;
typedef unsigned __int64 uint64;
typedef unsigned __int32 uint32;
typedef unsigned __int16 uint16;
typedef unsigned __int8 uint8;
typedef unsighed __int8 BYTE;

typedef UINT64 uint64;
#endif

#define MAX_SEND_SIZE   2048
#define NET_HEAD_SIZE   8


#endif//__GLOBAL_DEF__
