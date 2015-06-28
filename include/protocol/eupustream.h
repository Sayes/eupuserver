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
#include "common.h"
#include "globaldef.h"
#include "eupulogger4system.h"

using namespace std;

class CEupuStream {
public:
    CEupuStream();
    virtual ~CEupuStream();

    bool is_big_endian()
    {
        int32 n = 0x01020304;
        if (*(char*)&n == 0x01)
        {
            return true;
        }
        return false;
    }

    uint64 hl64ton(uint64 hostvalue)
    {
        uint64 ret = 0;
        uint32 high, low;

        low = hostvalue & 0xFFFFFFFF;
        high = (hostvalue >> 32) & 0xFFFFFFFF;
        low = htonl(low);
        high = htonl(high);
        ret = low;
        ret <<= 32;
        ret |= high;
        return ret;
    }

    uint64 ntohl64(uint64 netvalue)
    {
        uint64 ret = 0;
        uint32 high, low;
        low = netvalue & 0xFFFFFFFF;
        high = (netvalue >> 32) & 0xFFFFFFFF;
        low = ntohl(low);
        high = ntohl(high);
        ret = low;
        ret <<= 32;
        ret |= high;
        return ret;
    }

    string cmnNtoA(uint32 ip)
    {
        char buf[100];
        struct in_addr addr;
        memset(&addr, 0, sizeof(addr));
        addr.s_addr = htonl(ip);
#ifdef OS_LINUX
        const char* p = inet_ntop(AF_INET, (void*)&addr, buf, (socklen_t)sizeof(buf));
#elif OS_WINDOWS
        const char* p = eupu_inet_ntop(AF_INET, (void*)&addr, buf, (socklen_t)sizeof(buf));
#endif
        return string(p);
    }

    virtual bool In(BYTE* pSrc, uint32& nLen) = 0;
    virtual bool Out(BYTE* pDst, uint32& nLen) = 0;
    virtual void Debug() = 0;

    template<class T> int OutputValue(BYTE* buf, int32 buflen, T value)
    {
        int32 nLen = sizeof(value);
        if (nLen > buflen)
            return -1;

        if (nLen == sizeof(BYTE))
        {
            memcpy(buf, &value, nLen);
            return nLen;
        }
        else if (nLen == sizeof(uint16))
        {
            uint16 tmp = htons(value);
            memcpy(buf, &tmp, nLen);
            return nLen;
        }
        else if (nLen == sizeof(uint32))
        {
            uint32 tmp = htonl(value);
            memcpy(buf, &tmp, nLen);
            return nLen;
        }
        else if (nLen == sizeof(uint64))
        {
            if (is_big_endian())
            {
                memcpy(buf, &value, nLen);
            }
            else
            {
                uint64 tmp = hl64ton(value);
                memcpy(buf, &tmp, nLen);
            }
            return nLen;
        }
        return -1;
    }

    int OutputString(BYTE* buf, int32 buflen, char* szSrc)
    {
        int32 nLen = 0;
        int32 nStrLen = strlen(szSrc);
        if (nStrLen + sizeof(int32) > buflen)
            return -1;

        uint32 tmp = htonl(nStrLen);
        memcpy(buf, &tmp, sizeof(int32));
        nLen += sizeof(int32);

        memcpy(buf + sizeof(int32), szSrc, nStrLen);
        nLen += nStrLen;
        return nLen;
    }

    template<class T> int32 InputValue(BYTE* buf, int32 buflen, T& value)
    {
        int32 nLen = sizeof(value);
        if (nLen > buflen)
            return -1;

        if (nLen == sizeof(BYTE))
        {
            memcpy(&value, buf, nLen);
            return nLen;
        }
        else if (nLen == sizeof(uint16))
        {
            uint16 tmp = 0;
            memcpy(&tmp, buf, nLen);
            value = (T)ntohs(tmp);
            return nLen;
        }
        else if (nLen == sizeof(int32))
        {
            uint32 tmp = 0;
            memcpy(&tmp, buf, nLen);
            value = (T)ntohl(tmp);
            return nLen;
        }
        else if (nLen == sizeof(int64))
        {
            if (is_big_endian())
            {
                memcpy(&value, buf, nLen);
            }
            else
            {
                uint64 tmp = 0;
                memcpy(&tmp, buf, nLen);
                value = (T)ntohl64(tmp);
            }
            return nLen;
        }
        return -1;
    }

    int InputString(BYTE* buf, int32 buflen, char* szDst, int32& dstbuflen)
    {
        int32 nLen = 0;
        int32 nStrLen = 0;

        if (sizeof(int32) > buflen)
            return -1;

        memcpy((BYTE*)&nStrLen, buf, sizeof(int32));
        uint32 tmp = ntohl(nStrLen);

        if (int32(sizeof(int32) + tmp) > buflen)
            return -1;

        if (int32(tmp + 1) > dstbuflen)
            return -2;

        nLen += sizeof(int32);

        memcpy(szDst, buf + sizeof(int32), tmp);
        nLen += tmp;

        szDst[tmp] = '\0';
        dstbuflen = tmp + 1;

        return nLen;
    }
};

#endif//_EUPUSTREAM_H_
