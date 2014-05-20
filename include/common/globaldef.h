#ifndef __GLOBAL_DEF__
#define __GLOBAL_DEF__

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
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

#define	MAX_SEND_SIZE	2048
#define NET_HEAD_SIZE   7


#endif//__GLOBAL_DEF__
