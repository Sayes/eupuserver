#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include "globaldef.h"
#include "eupustream.h"

struct NetMessageHead : public CEupuStream
{
    uint16 uMessageSize;
    uint16 uMainID;
    uint16 bAssistantID;
    BYTE bHandleCode;
    BYTE bReserve;
public:
    NetMessageHead()
    {
        uMessageSize = 0;
        uMainID = 0;
        bAssistantID = 0;
        bHandleCode = 0;
        bReserve = 0;
    }

    void Debug()
    {
        LOG(_DEBUG_, "NetMessageHead::Debug() object members:");
        LOG(_DEBUG_, "\tuMessageSize: %u", uMessageSize);
        LOG(_DEBUG_, "\tuMainID: %u", uMainID);
        LOG(_DEBUG_, "\tbAssistantID: %u", bAssistantID);
        LOG(_DEBUG_, "\tbHandleCode: %u", bHandleCode);
        LOG(_DEBUG_, "\tbReserve: %u", bReserve);
    }

    inline bool Out(BYTE* pDest, uint32& nLen)
    {
        if (pDest == NULL)
            return false;

        int32 nret = 0;
        uint32 ntmp = 0;
        uint32 buflen = nLen;

        nret = OutputValue(pDest + ntmp, buflen - ntmp, uMessageSize);
        if (nret < 0)
            return false;

        ntmp += nret;
        nret = OutputValue(pDest + ntmp, buflen - ntmp, uMainID);
        if (nret < 0)
            return false;

        ntmp += nret;
        nret = OutputValue(pDest + ntmp, buflen - ntmp, bAssistantID);
        if (nret < 0)
            return false;

        ntmp += nret;
        nret = OutputValue(pDest + ntmp, buflen - ntmp, bHandleCode);
        if (nret < 0)
            return false;

        ntmp += nret;
        nret = OutputValue(pDest + ntmp, buflen - ntmp, bReserve);

        if (nret < 0)
            return false;

        ntmp += nret;
        nLen = ntmp;
        return true;
    }

    inline bool In(BYTE* pSrc, uint32& nLen)
    {
        if (pSrc == NULL)
            return false;

        int32 nret = 0;
        uint32 ntmp = 0;
        int32 buflen = nLen;

        nret = InputValue(pSrc + ntmp, buflen - ntmp, uMessageSize);
        if (nret < 0)
            return false;
        ntmp += nret;

        nret = InputValue(pSrc + ntmp, buflen - ntmp, uMainID);
        if (nret < 0)
            return false;
        ntmp += nret;

        nret = InputValue(pSrc + ntmp, buflen - ntmp, bAssistantID);
        if (nret < 0)
            return false;
        ntmp += nret;

        nret = InputValue(pSrc + ntmp, buflen - ntmp, bHandleCode);
        if (nret < 0)
            return false;
        ntmp += nret;

        nret = InputValue(pSrc + ntmp, buflen - ntmp, bReserve);
        if (nret < 0)
            return false;
        ntmp += nret;

        nLen = ntmp;
        return true;
    }

};

#endif//_PROTOCOL_H_
