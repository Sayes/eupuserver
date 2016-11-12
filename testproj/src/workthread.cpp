#include "eupulogger4system.h"
#include "workthread.h"
#include "netcommon.h"
#include "globalmgr.h"
#include "protocol.h"
#include "sprotocol.h"
#include "eupu.base.pb.h"

CWorkThread::CWorkThread()
: m_iUserCount(0)
{
}

CWorkThread::~CWorkThread()
{
i   m_iUserCount = 0;
}

int CWorkThread::processMessage(NET_DATA* pdata)
{
    LOG(_INFO_, "CWorkThread::processMessage() begin");
    //pdata will be NULL never
    if (!pdata)
    {
        LOG(_ERROR_,"the message is null");
        return 0;
    }

    uint32_t uTmp = pdata->data_len;
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
        case RS_CONNECTED:
        {
            LOG(_INFO_, "CWorkThread::processMessage() deal with RS_CONNECT, type=%d", pdata->type);
            if (pdata->type == CLIENT_TYPE)
            {
                LOG(_INFO_, "CWorkThread::processMessage(), new client connected");
                nret = 0;
            }
            if (pdata->type == USERCENTERSVR_TYPE)
            {
                nret = 0;
            }
            break;
        }
        case RS_DISCONNECTED:
        {
            LOG(_INFO_, "CWorkThread::processMessage() deal with RS_DISCONNECTED");
            if (pdata->type == CLIENT_TYPE)
            {
                nret = 0;
            }
            if (pdata->type == USERCENTERSVR_TYPE)
            {
                nret = 0;
            }
            break;
        }
        case KEEP_ALIVE_PING:
        {
            if (pdata->type == CLIENT_TYPE)
            {
                LOG(_INFO_, "CWorkThread::processMessage() deal with KEEP_ALIVE_PING");
                nret = 0;
            }
            else
            {
                LOG(_INFO_, "CWorkThread::processMessage() deal with KEEP_ALIVE_PING, pdata->type = %d", pdata->type);
            }
            if (pdata->type == USERCENTERSVR_TYPE)
            {
                nret = 0;
            }
            break;
        }
        default:
        {
            LOG(_ERROR_, "CWorkThread::processMessage() error, invalid message, header.uMainID=%d", header.uMainID);
            nret = -1;
            break;
        }
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

    bool bret = CGlobalMgr::getInstance()->createMsgToSendList(pdata->fddat,
                                                               pdata->connect_time,
                                                               pdata->peer_ip, pdata->peer_port, pdata->type, KEEP_ALIVE_PING, 0, 0, 0, NULL, 0);

    return (bret ? 1 : 0);

}
