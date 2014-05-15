#include <fstream>
#include "json/json.h"
#include "globaldef.h"
#include "globalconfig.h"
#include "common.h"
#include "eupulogger4system.h"
#include "workthread.h"

using namespace std;

bool initSystem()
{
    return 0;
}

void exitSystem()
{
}

int main(int argc, char* argv[])
{

    daemonize();

    LOGSETLEVEL((LOGLEVEL)4);

    LOGSETDEBUG(true);

    LOG(_INFO_, "main() start");

    CGlobalConfig* pConfig = CGlobalConfig::getInstance();
    if (!pConfig->initSysConfig("sysconf.json"))
    {
        LOG(_DEBUG_, "init config failed");
    }
    unsigned int ping_timer = pConfig->getPingTimer();

    CWorkThread workthread;
    workthread.processMessage(NULL);

    do {
        if (!initSystem())
        {
            break;
        }

    } while (0);

    exitSystem();

    return 0;
}
