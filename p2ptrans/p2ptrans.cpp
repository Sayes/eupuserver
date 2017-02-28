// p2ptrans.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "json/json.h"
#include "zlib.h"
#include "logger/eupulogger4system.h"
#include "common/globaldef.h"
#include "common/globalconfig.h"
#include "network/globalmgr.h"
#include "common/common.h"
#include "network/euputhread.h"
#include "network/wsathread.h"
#include "network/workbasethread.h"
#include "workthread.h"
#include "appmsg/eupu.base.pb.h"
#include "common/ringbuffer.h"

CWSAThread* g_wsathread = NULL;
CWorkThread* g_workthread = NULL;

bool initSystem()
{
    CGlobalConfig* pconfig = CGlobalConfig::getInstance();
    if (pconfig == NULL)
    {
        LOG(_ERROR_, "initSystem() error, CGlobalConfig::getInstance() failed");
        return false;
    }

    if (!pconfig->initSysConfig("sysconf.json"))
    {
        LOG(_ERROR_, "initSystem() error, CGlobalConfig::initSysConfig() failed");
        return false;
    }

    CGlobalMgr* pmgr = CGlobalMgr::getInstance();
    if (pmgr == NULL)
    {
        LOG(_ERROR_, "initSystem() error, CGlobalMgr::getInstance() failed");
        return false;
    }

    pmgr->init();

    g_wsathread = new CWSAThread;
    if (g_wsathread == NULL)
    {
        LOG(_ERROR_, "initSystem() error, _new CWSAThread failed");
        return false;
    }

    if (!g_wsathread->startup())
    {
        LOG(_ERROR_, "initSystem() error, g_wsathread->startup() failed");
        delete g_wsathread;
        g_wsathread = NULL;
        return false;
    }

    g_workthread = new CWorkThread;
    if (g_workthread == NULL)
    {
        LOG(_ERROR_, "initSystem() error, _new CWorkThread failed");
        delete g_wsathread;
        g_wsathread = NULL;
        return false;
    }

    if (!g_workthread->start())
    {
        LOG(_ERROR_, "initSystem() error, g_workthread->start() failed");
        delete g_wsathread;
        g_wsathread = NULL;
        delete g_workthread;
        g_workthread = NULL;
        return false;
    }

    LOG(_INFO_, "initSystem() successed");

    return true;
}

void exitSystem()
{
    if (g_workthread && g_workthread->stop())
    {
        LOG(_ERROR_, "exitSystem() error, g_workthread->stop() failed");
        Sleep(1000);
        delete g_workthread;
        g_workthread = NULL;
    }

    if (g_wsathread)
    {
        LOG(_ERROR_, "exitSystem() error, g_epollthread == NULL");
        g_wsathread->stop();
        Sleep(1000);
        delete g_wsathread;
        g_wsathread = NULL;
    }

    CGlobalConfig* pconfig = CGlobalConfig::getInstance();
    if (pconfig)
    {
        pconfig->release();
    }

    CGlobalMgr* pmgr = CGlobalMgr::getInstance();
    if (pmgr)
    {
        pmgr->release();
    }

    Sleep(1000);

    LOG(_INFO_, "exitSystem() end");

    CEupuLogger4System::Release();
}

int _tmain(int argc, _TCHAR* argv[])
{

    do
    {

        if (!initSystem())
        {
            break;
        }

        int interval = 0;

        while (true)
        {
            CGlobalMgr::getInstance()->createServerConnect(USERCENTERSVR_TYPE);
            interval++;
            if (interval >= 12)
            {
                interval = 0;
                LOG(_INFO_, "main(), the recv queue has %d total messages", CGlobalMgr::getInstance()->getRecvQueue()->sizeWithoutLock());
            }
            Sleep(5000);
        }

    }
    while (false);

    exitSystem();

    return 0;
}
