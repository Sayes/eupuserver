#include "globalmgr.h"


CSysQueue<NET_DATA>* CGlobalMgr::getRecvQueue()
{
    return &m_recvlist;
}
