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

    __uint64_t ntohl64(__uint64_t host)
    {
        __uint64_t ret = 0;
        unsigned int high, low;
        low = host & 0xFFFFFFFF;
        high = (host >> 32) & 0xFFFFFFFF;
        low = ntohl(low);
        high = ntohl(high);
        ret = low;
        ret <<= 32;
        ret |= high;
        return ret;
    }

    string cmnNtoA(unsigned int ip)
    {
        char buf[100];
        struct addr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.s_addr = htonl(ip);
        const char* p = inet_ntop(AF_INET, (void*)&addr, buf, (socklen_t)sizeof(buf));
        return string(p);
    }

    virtual bool In(BYTE* pSrc, UINT& len) = 0;
    virtual bool Out(BYTE* pDst, UINT& len) = 0;
    
    virtual void Debug() = 0;

    template<class T>OutputValue(BYTE* buf, INT32 buflen, T value)
    {
        INT32 nLen = sizeof(value);
        if (nLen > buflen)
            return -1;

        if (nLen == sizeof(BYTE))
        {
            memcpy(buf, &value, nLen);
            return nLen;
        }
        else if (nLen == sizeof(USHORT))
        {
            USHORT tmp = htons(value);
            memcpy(buf, &tmp, nLen);
            return nLen;
        }
        else if (nLen == sizeof(UINT))
        {
            UINT tmp = htonl(value);
            memcpy(buf, &tmp, nLen);
            return nLen;
        }
        else if (nLen == sizeof(UINT64))
        {
            if (is_big_endian())
            {
                memcpy(buf, &value, nLen);
            }
            else
            {
                unsigned __uint64_t tmp = hl64ton(value);
                memcpy(buf, &tmp, nLen);
            }
            return nLen;
        }
        return -1;
    }

    int OutputString(BYTE* buf, INT32 buflen, char* szDst)
    {
        INT32 nLen = 0;
        UINT nStrLen = strlen(szDst);
        if (nStrLen + (UINT)sizeof(INT32) > buflen)
            return -1;

        UINT ntmp = htonl(nStrLen);
        memcpy(buf, &ntmp, sizeof(INT32));
        nLen += sizeof(INT32);

        memcpy(buf + sizeof(INT32), szDst, nStrLen);
        nLen += nStrlen;
        return nLen; 
    }

    template<class T> int InputValue(BYTE* buf, UINT buflen, T value)
    {
        INT32 nLen = sizeof(value);
        if (nLen > buflen)
            return -1;

        if (nLen == sizeof(BYTE))
        {
        }
        else (nLen == sizeof(USHORT))
        {
        }
        else (nLen == sizeof(INT32))
        {
        }
        else (nLen == sizeof(INT64))
        {
        }
    }
};

#endif//_EUPUSTREAM_H_
