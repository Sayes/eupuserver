/*************************************************************************
	> File Name: globalmgr.cpp
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: 2014年04月28日 星期一 23时13分44秒
 ************************************************************************/

#include "globalmgr.h"


CSysQueue<NET_DATA>* CGlobalMgr::getRecvQueue()
{
    return &m_recvlist;
}
