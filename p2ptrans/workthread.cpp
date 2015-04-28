#include "stdafx.h"
#include "eupulogger4system.h"
#include "netcommon.h"
#include "globalmgr.h"
#include "protocol.h"
#include "sprotocol.h"
#include "workthread.h"

CWorkThread::CWorkThread()
{
}

CWorkThread::~CWorkThread()
{
}

int CWorkThread::processMessage(NET_DATA* pdata)
{
    if (!pdata)
    {
        LOG(_ERROR_, "CWorkThread::processMessage() error, the message is null");
        return 0;
    }

    UINT uTmp = pdata->data_len;
    NetMessageHead header;

    if (!header.In((BYTE*)pdata->pdata, uTmp))
    {
        LOG(_ERROR_, "CWorkThread::processMessage() error, parse message head failed");
        return -1;
    }

    if (header.uMainID != KEEP_ALIVE_PING)
        header.Debug();

    int nret = -1;
    switch (header.uMainID)
    {
        case KEEP_ALIVE_PING:
            {
                LOG(_INFO_, "CWorkThread::processMessage(), deal with KEEP_ALIVE_PING");
                break;
            }
        case RS_SERVER_CONNECTED:
            {
                LOG(_INFO_, "CWorkThread::processMessage(), deal with RS_SERVER_CONNECTED");
                break;
            }
        case RS_SERVER_DISCONNECTED:
            {
                LOG(_INFO_, "CWorkThread::processMessage(), deal with RS_SERVER_DISCONNECTED");
                break;
            }
        default:
            {
                LOG(_INFO_, "CWorkThread::processMessage(), deal with unknown message, mainid=%d", header.uMainID);
                break;
            }
    }

    return 1;
}

int CWorkThread::ProcessServerConnected(NET_DATA* pdata)
{
    return 1;
}

