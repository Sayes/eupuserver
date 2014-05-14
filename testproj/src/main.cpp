#include <fstream>
#include "json/json.h"
#include "globaldef.h"
#include "globalconfig.h"
#include "common.h"
#include "eupulogger4system.h"

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

    LOGSETLEVEL((LOGLEVEL)1);

    LOGSETDEBUG(true);

    LOG(_INFO_, "main() start");

    CGlobalConfig* pConfig = CGlobalConfig::getInstance();
    if (!pConfig->initSysConfig("sysconf.json"))
    {
        LOG(_DEBUG_, "init config failed");
    }

    daemonize();
    fgNtoA(2000);

    unsigned int ping_timer = pConfig->getPingTimer();

    do {
        if (!initSystem())
        {
            break;
        }

    } while (0);

    exitSystem();

    return 0;
}
