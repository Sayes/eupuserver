#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include "globaldef.h"
#include "eupustream.h"

struct NetMessageHead : public CEupuStream
{
	USHORT uMessageSize;
	USHORT uMainID;
	BYTE bAssistantID;
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

	inline bool Out(BYTE* pDest, UINT& nLen)
	{
		if (pDest == NULL)
			return false;

		INT32 nret = 0;
		INT32 buflen = nLen;
		INT32 ntmp = 0;

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

	inline bool In(BYTE* pSrc, UINT& nLen)
	{
		if (pSrc == NULL)
			return false;

		INT32 nret = 0;
		INT32 buflen = nLen;
		INT32 ntmp = 0;

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
