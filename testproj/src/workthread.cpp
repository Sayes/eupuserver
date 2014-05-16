#include "eupulogger4system.h"
#include "workthread.h"
#include "netcommon.h"

CWorkThread::CWorkThread()
{
}

CWorkThread::~CWorkThread()
{
}

int CWorkThread::processMessage(NET_DATA* pdata)
{
    LOG(_INFO_, "CWorkThread::processMessage() begin");
    return 0;
}
