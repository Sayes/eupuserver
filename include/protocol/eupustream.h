#ifndef _EUPUSTREAM_H_
#define _EUPUSTREAM_H_

#include <string>
#include <sys/types.h>
#ifdef OS_LINUX
#include <sys/socket.h>
#include <arpa/inet.h>
#elif OS_WINDOWS
#include <ws2tcpip.h>
#endif
#include "globaldef.h"

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

    __uint64_t hl64ton(__uint64_t hostvalue)
    {
        __uint64_t ret = 0;
        unsigned int high, low;

        low = hostvalue & 0xFFFFFFFF;
        high = (hostvalue >> 32) & 0xFFFFFFFF;
        low = htonl(low);
        high = htonl(high);
        ret = low;
        ret <<= 32;
        ret |= high;
        return ret;
    }

    __uint64_t ntohl64(__uint64_t netvalue)
    {
        __uint64_t ret = 0;
        unsigned int high, low;
        low = netvalue & 0xFFFFFFFF;
        high = (netvalue >> 32) & 0xFFFFFFFF;
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
        struct in_addr addr;
        memset(&addr, 0, sizeof(addr));
        addr.s_addr = htonl(ip);
        const char* p = inet_ntop(AF_INET, (void*)&addr, buf, (socklen_t)sizeof(buf));
        return string(p);
    }

    virtual bool In(BYTE* pSrc, UINT& nLen) = 0;
    virtual bool Out(BYTE* pDst, UINT& nLen) = 0;
    virtual void Debug() = 0;

    template<class T> int OutputValue(BYTE* buf, INT32 buflen, T value)
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
                __uint64_t tmp = hl64ton(value);
                memcpy(buf, &tmp, nLen);
            }
            return nLen;
        }
        return -1;
    }

    int OutputString(BYTE* buf, INT32 buflen, char* szSrc)
    {
        INT32 nLen = 0;
        UINT nStrLen = strlen(szSrc);
        if (nStrLen + (UINT)sizeof(INT32) > buflen)
            return -1;

        UINT tmp = htonl(nStrLen);
        memcpy(buf, &tmp, sizeof(INT32));
        nLen += sizeof(INT32);

        memcpy(buf + sizeof(INT32), szSrc, nStrLen);
        nLen += nStrLen;
        return nLen; 
    }

    template<class T> int InputValue(BYTE* buf, UINT buflen, T& value)
    {
        INT32 nLen = sizeof(value);
        if (nLen > buflen)
            return -1;

        if (nLen == sizeof(BYTE))
        {
            memcpy(&value, buf, nLen);
            return nLen;
        }
        else if(nLen == sizeof(USHORT))
        {
            USHORT tmp = 0;
            memcpy(&tmp, buf, nLen);
            value = (T)ntohs(tmp);
            return nLen;
        }
        else if(nLen == sizeof(INT32))
        {
            UINT tmp = 0;
            memcpy(&tmp, buf, nLen);
            value = (T)ntohl(tmp);
            return nLen;
        }
        else if(nLen == sizeof(INT64))
        {
            if (is_big_endian())
            {
                memcpy(&value, buf, nLen);
            }
            else
            {
                __uint64_t tmp = 0;
                memcpy(&tmp, buf, nLen);
                value = (T)ntohl64(tmp);
            }
            return nLen;
        }
        return -1;
    }

    int InputString(BYTE* buf, INT32 buflen, char* szDst, INT32& dstbuflen)
    {
        INT32 nLen = 0;
        unsigned int nStrLen = 0;

        if (buflen < sizeof(INT32))
            return -1;

        memcpy((BYTE*)&nStrLen, buf, sizeof(INT32));
        unsigned int tmp = ntohl(nStrLen);

        if (buflen < sizeof(INT32) + tmp)
            return -1;

        if (dstbuflen < tmp + 1)
            return -2;

        nLen += sizeof(INT32);

        memcpy(szDst, buf + sizeof(INT32), tmp);
        nLen += tmp;

        szDst[tmp] = '\0';
        dstbuflen = tmp + 1;

        return nLen;
    }
};

#endif//_EUPUSTREAM_H_
