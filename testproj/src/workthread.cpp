#include "eupulogger4system.h"
#include "workthread.h"
#include "netcommon.h"
#include "protocol.h"
#include "sprotocol.h"

CWorkThread::CWorkThread()
: m_iUserCount(0)
{
}

CWorkThread::~CWorkThread()
{
}

int CWorkThread::processMessage(NET_DATA* pdata)
{
    LOG(_INFO_, "CWorkThread::processMessage() begin");
	if (!pdata)
	{
		LOG(_ERROR_,"the message is null");
		return 0;
	}

	UINT uTmp = pdata->data_len;
	NetMessageHead header;

	if(!header.In((BYTE *)pdata->pdata, uTmp))
	{
		LOG(_ERROR_,"parse message head failed");
		return -1;
	}

	if(header.uMainID != KEEP_ALIVE_PING)
		header.Debug();

	switch (header.uMainID)
	{
    case RS_SERVER_CONNECTED:
        {
            LOG(_INFO_, "CWorkThread::processMessage() deal with RS_SERVER_CONNECT");
            break;
        }

	default:
		{
		}
		break;
	}
	
    return 1;
}

bool CWorkThread::ProcessServerConnected(NET_DATA* pdata)
{
	return false;
}

bool CWorkThread::ProcessDistributeConnect(NET_DATA* pdata)
{
	return false;
}

bool CWorkThread::ProcessMainConnected(NET_DATA* pdata)
{
	return true;
}
