#include "eupulogger4system.h"
#include "workthread.h"
#include "netcommon.h"
#include "globalmgr.h"
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
    {
		header.Debug();
    }
    else
    {
        LOG(_INFO_, "CWorkThread::processMessage() deal with KEEP_ALIVE_PING");
    }

    int nret = -1;
	switch (header.uMainID)
	{
    case RS_SERVER_CONNECTED:
        {
            LOG(_INFO_, "CWorkThread::processMessage() deal with RS_SERVER_CONNECT");
            if (pdata->type == CLIENT_TYPE)
            {
                LOG(_INFO_, "CWorkThread::processMessage(), net client connected");
                nret = ProcessKeepalive(pdata);
            }
            break;
        }

	default:
		{
            LOG(_ERROR_, "CWorkThread::processMessage() error, invalid message");
            nret = -1;
		}
		break;
	}
	
    return nret;
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

int CWorkThread::ProcessKeepalive(NET_DATA* pdata)
{
    if (pdata == NULL)
        return 0;

    bool bret = CGlobalMgr::getInstance()->createMsgToSendList(pdata->fd, pdata->connect_time, pdata->peer_ip, pdata->peer_port, pdata->type, KEEP_ALIVE_PING, 0, 0, 0, NULL, 0);
    return (bret ? 1 : 0);
}
