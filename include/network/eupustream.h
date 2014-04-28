/*************************************************************************
	> File Name: eupustream.h
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: 2014年04月28日 星期一 23时18分53秒
 ************************************************************************/

#ifndef _EUPUSTREAM_H_
#define _EUPUSTREAM_H_

#ifdef OS_LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "globaldef.h"
#endif

#include <string>
#define INT32 int
#define __int64 INT64

using namespace std;

class CEupuStream
{
    public:
    CEupuStream();
    virtual ~CEupuStream();

    bool is_big_endian()
    {
        int n = 0x01020304;
        if (*(char*)&n == 0x01)
        {
            return true;
        }
        return false;
    }

    __uint64_t hl64ton(__uint64_t host)
    {
        __uint64_t ret = 0;
        unsigned int high, low;

        low = host & 0xFFFFFFFF;
        high = (host >> 32) & 0xFFFFFFFF;
        low = htonl(low);
        high = htonl(high);
        ret = low;
        ret <<= 32;
        ret |= high;
        return ret;
    }


};

#endif//_EUPUSTREAM_H_
