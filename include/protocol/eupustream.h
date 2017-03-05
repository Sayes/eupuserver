// Copyright shenyizhong@gmail.com, 2014

#ifndef PROTOCOL_EUPUSTREAM_H_
#define PROTOCOL_EUPUSTREAM_H_

#include <sys/types.h>
#include <string>
#ifdef OS_LINUX
#include <arpa/inet.h>
#include <sys/socket.h>
#elif OS_WINDOWS
#include <ws2tcpip.h>
#endif
#include "common/common.h"
#include "common/eupu_inet.h"
#include "common/globaldef.h"
#include "logger/eupulogger4system.h"

class CEupuStream {
   public:
    CEupuStream();
    virtual ~CEupuStream();

    bool is_big_endian() {
        int32_t n = 0x01020304;
        if (*(char *)&n == 0x01) {
            return true;
        }
        return false;
    }

    uint64_t hl64ton(uint64_t hostvalue) {
        uint64_t ret = 0;
        uint32_t high, low;

        low = hostvalue & 0xFFFFFFFF;
        high = (hostvalue >> 32) & 0xFFFFFFFF;
        low = htonl(low);
        high = htonl(high);
        ret = low;
        ret <<= 32;
        ret |= high;
        return ret;
    }

    uint64_t ntohl64(uint64_t netvalue) {
        uint64_t ret = 0;
        uint32_t high, low;
        low = netvalue & 0xFFFFFFFF;
        high = (netvalue >> 32) & 0xFFFFFFFF;
        low = ntohl(low);
        high = ntohl(high);
        ret = low;
        ret <<= 32;
        ret |= high;
        return ret;
    }

    std::string cmnNtoA(uint32_t ip) {
        char buf[100];
        struct in_addr addr;
        memset(&addr, 0, sizeof(addr));
        addr.s_addr = htonl(ip);
#ifdef OS_LINUX
        const char *p =
            inet_ntop(AF_INET, (void *)&addr, buf, (socklen_t)sizeof(buf));
#elif OS_WINDOWS
        const char *p =
            eupu_inet_ntop(AF_INET, (void *)&addr, buf, (socklen_t)sizeof(buf));
#endif
        return std::string(p);
    }

    virtual bool In(BYTE *pSrc, uint32_t &nLen) = 0;
    virtual bool Out(BYTE *pDst, uint32_t &nLen) = 0;
    virtual void Debug() = 0;

    template <class T>
    int OutputValue(BYTE *buf, int32_t buflen, T value) {
        int32_t nLen = sizeof(value);
        if (nLen > buflen) return -1;

        if (nLen == sizeof(BYTE)) {
            memcpy(buf, &value, nLen);
            return nLen;
        } else if (nLen == sizeof(uint16_t)) {
            uint16_t tmp = htons(value);
            memcpy(buf, &tmp, nLen);
            return nLen;
        } else if (nLen == sizeof(uint32_t)) {
            uint32_t tmp = htonl(value);
            memcpy(buf, &tmp, nLen);
            return nLen;
        } else if (nLen == sizeof(uint64_t)) {
            if (is_big_endian()) {
                memcpy(buf, &value, nLen);
            } else {
                uint64_t tmp = hl64ton(value);
                memcpy(buf, &tmp, nLen);
            }
            return nLen;
        }
        return -1;
    }

    int OutputString(BYTE *buf, int32_t buflen, char *szSrc) {
        int32_t nLen = 0;
        int32_t nStrLen = strlen(szSrc);
        if (static_cast<int32_t>(nStrLen + sizeof(int32_t)) > buflen) return -1;

        uint32_t tmp = htonl(nStrLen);
        memcpy(buf, &tmp, sizeof(int32_t));
        nLen += sizeof(int32_t);

        memcpy(buf + sizeof(int32_t), szSrc, nStrLen);
        nLen += nStrLen;
        return nLen;
    }

    template <class T>
    int32_t InputValue(BYTE *buf, int32_t buflen, T &value) {
        int32_t nLen = sizeof(value);
        if (nLen > buflen) return -1;

        if (nLen == sizeof(BYTE)) {
            memcpy(&value, buf, nLen);
            return nLen;
        } else if (nLen == sizeof(uint16_t)) {
            uint16_t tmp = 0;
            memcpy(&tmp, buf, nLen);
            value = (T)ntohs(tmp);
            return nLen;
        } else if (nLen == sizeof(int32_t)) {
            uint32_t tmp = 0;
            memcpy(&tmp, buf, nLen);
            value = (T)ntohl(tmp);
            return nLen;
        } else if (nLen == sizeof(int64_t)) {
            if (is_big_endian()) {
                memcpy(&value, buf, nLen);
            } else {
                uint64_t tmp = 0;
                memcpy(&tmp, buf, nLen);
                value = (T)ntohl64(tmp);
            }
            return nLen;
        }
        return -1;
    }

    int InputString(BYTE *buf, int32_t buflen, char *szDst,
                    int32_t &dstbuflen) {
        int32_t nLen = 0;
        int32_t nStrLen = 0;

        if (static_cast<int32_t>(sizeof(int32_t)) > buflen) return -1;

        memcpy((BYTE *)&nStrLen, buf, sizeof(int32_t));
        uint32_t tmp = ntohl(nStrLen);

        if (int32_t(sizeof(int32_t) + tmp) > buflen) return -1;

        if (int32_t(tmp + 1) > dstbuflen) return -2;

        nLen += sizeof(int32_t);

        memcpy(szDst, buf + sizeof(int32_t), tmp);
        nLen += tmp;

        szDst[tmp] = '\0';
        dstbuflen = tmp + 1;

        return nLen;
    }
};

#endif  // PROTOCOL_EUPUSTREAM_H_
