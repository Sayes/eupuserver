#include "globalmgr.h"

CGlobalMgr* CGlobalMgr::m_pInstance = NULL;

CGlobalMgr* CGlobalMgr::getInstance()
{
    if (m_pInstance == NULL)
        m_pInstance = new CGlobalMgr;
    return m_pInstance;
}

void CGlobalMgr::release()
{
    if (m_pInstance != NULL)
        delete m_pInstance;
    m_pInstance = NULL;
}

void CGlobalMgr::createServerConnect(int ntype)
{
}

bool CGlobalMgr::createMsgToSendList(int fd, time_t conntime, const string& ip, USHORT port, int ntype, USHORT mainid, USHORT assistantid, BYTE code, BYTE reserve, CEupuStream* stream, UINT nlen)
{
    return false;
}

bool CGlobalMgr::addMsgToSendList(NET_DATA* pdata)
{
    return false;
}

bool CGlobalMgr::init()
{
    return false;
}

void CGlobalMgr::clean()
{
}

CSysQueue<NET_DATA>* CGlobalMgr::getRecvQueue()
{
    return &m_recvlist;
}

CSysQueue<NET_EVENT>* CGlobalMgr::getEventQueue()
{
    return &m_eventlist;
}

bool CGlobalMgr::sendMsgToServer(int ntype, USHORT mainid, USHORT assistantid, BYTE code, BYTE reserve, CEupuStream* stream, UINT nlen, bool blocked)
{
    return false;
}

void CGlobalMgr::setServerSocket(int fd, time_t conntime, const string& ip, USHORT port, int ntype)
{
}

void CGlobalMgr::switchSendMap()
{
}
