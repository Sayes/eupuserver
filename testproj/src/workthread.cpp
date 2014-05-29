#include "eupulogger4system.h"
#include "workthread.h"
#include "netcommon.h"

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

	if(header.bMainID != KEEP_ALIVE_PING)
		header.Debug();

	switch (header.bMainID)
	{
	case RS_SERVER_CONNECTED:
		{
			LOG(_INFO_, "process RS_SERVER_CONNECTED message,peerip:%s,peerport:%d, type:%d",GETNULLSTR(pdata->peer_ip),pdata->peer_port,pdata->type);
			if(pdata->type == USERCENTERSVR_TYPE)
			{
				return ProcessServerConnected(pdata);
			}
			else if (pdata->type == DISSVR_TYPE)
			{
				return ProcessDistributeConnect(pdata);
			}
			else if (pdata->type == MAINSVR_TYPE)
			{
				return ProcessMainConnected(pdata);
			}
			else if(pdata->type == CLIENT_TYPE)
			{
				LOG(_INFO_, "process CLIENT_TYPE message,peerip:%s, peerport:%d, type:%d",GETNULLSTR(pdata->peer_ip),pdata->peer_port,pdata->type);
				m_iUserCount++;
			}
			return 1;
		}
	case RS_SERVER_DISCONNECTED:
		{
			LOG(_INFO_, "process RS_SERVER_DISCONNECTED message,peerip:%s,peerport:%d, type:%d",GETNULLSTR(pdata->peer_ip),pdata->peer_port,pdata->type);
			return ProcessServDisConnect(pdata);
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