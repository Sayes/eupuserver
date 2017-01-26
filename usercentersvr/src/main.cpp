#include <fstream>
#include "json/json.h"
#include "eupulogger4system.h"
#include "globaldef.h"
#include "globalconfig.h"
#include "globalmgr.h"
#include "common.h"
#include "euputhread.h"
#include "epollthread.h"
#include "workbasethread.h"
#include "workthread.h"
#include "eupu.base.pb.h"


CEpollThread* g_epollthread = NULL;
CWorkThread* g_workthread = NULL;

bool initSystem(bool isdaemon)
{
    CGlobalConfig* pConfig = CGlobalConfig::getInstance();
    if (!pConfig)
    {
        LOG(_ERROR_, "initSystem() error, CGlobalConfig::getInstance() failed");
        return false;
    }

    if (!pConfig->initSysConfig("sysconf.json"))
    {
        LOG(_ERROR_, "initSystem() error, CGlobalConfig::initSysConfig() failed");
        return false;
    }

    if (isdaemon)
    {
        LOGSETDEBUG(true);
    }

    CGlobalMgr* pglobalmgr = CGlobalMgr::getInstance();
    if (!pglobalmgr)
    {
        LOG(_ERROR_, "initSystem() error, CGlobalMgr::getInstance() failed");
        return false;
    }

    pglobalmgr->init();

    g_epollthread = new CEpollThread;

    if (!g_epollthread)
    {
        LOG(_ERROR_, "initSystem() error, new CEpollThread failed");
        return false;
    }

    if (!g_epollthread->startup())
    {
        LOG(_ERROR_, "initSystem() error, g_epollthread->startup() failed");
        return false;
    }

    g_workthread = new CWorkThread;

    if (!g_workthread)
    {
        LOG(_ERROR_, "initSystem() error, new CWorkThread failed");
        delete g_epollthread;
        g_epollthread = NULL;
        return false;
    }

    if (!g_workthread->start())
    {
        LOG(_ERROR_, "initSystem() error, g_workthread->start() failed");
        delete g_epollthread;
        g_epollthread = NULL;
        delete g_workthread;
        g_workthread = NULL;
        return false;
    }

    LOG(_INFO_, "initSystem() successed");

    return true;
}

void exitSystem()
{
    if (!g_workthread)
    {
        LOG(_ERROR_, "exitSystem() error, g_workthread == NULL");
    }
    else if (!g_workthread->stop())
    {
        LOG(_ERROR_, "exitSystem() error, g_workthread->stop() failed");
        delete g_workthread;
        g_workthread = NULL;
    }


    if (!g_epollthread)
    {
        LOG(_ERROR_, "exitSystem() error, g_epollthread == NULL");
    }
    else if (!g_epollthread->stop())
    {
        LOG(_ERROR_, "exitSystem() error, g_epollthread->stop() failed");
        delete g_epollthread;
        g_epollthread = NULL;
    }

    CGlobalConfig* pconfig = CGlobalConfig::getInstance();
    if (pconfig)
    {
        pconfig->release();
    }

    CGlobalMgr* pglobalmgr = CGlobalMgr::getInstance();
    if (pglobalmgr)
    {
        pglobalmgr->release();
    }
    sleep(1);

    LOG(_INFO_, "exitSystem() end");
    CEupuLogger4System::Release();
}

void showusage()
{
}

void showversion()
{
}

int main(int argc, char* argv[])
{

    int ch = 0;
    bool isdaemon = false;

    while ( (ch = getopt(argc, argv, "h:v:d")) != EOF)
    {
        switch (ch)
        {
            case 'h':
            {
                showusage();
                return 0;
            }
            case 'v':
            {
                showversion();
                return 0;
            }
            case 'd':
            {
                isdaemon = true;
                break;
            }
            case '?':
            {
                std::cout << "invalid opt" << std::endl;
                return 0;
            }
            default:
            {
                return 0;
            }
        }
    }

    if (isdaemon)
    {
        daemonize();
    }

    do {

        if (!initSystem(isdaemon))
        {
            break;
        }

        int interval = 0;
        while (1)
        {
            interval++;
            if (interval >= 12)
            {
                interval = 0;
                LOG(_INFO_, "main(), the recv queue has %d total messages", CGlobalMgr::getInstance()->getRecvQueue()->sizeWithoutLock());
            }
            sleep(5);
        }

    } while (0);

    exitSystem();

    return 0;
}
