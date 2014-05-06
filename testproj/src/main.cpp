#include <string>
#include <fstream>
#include "common.h"
#include "globaldef.h"
#include "globalconfig.h"
#include "eupulogger4system.h"
#include "json/json.h"
using namespace std;

int main(int argc, char* argv[])
{
    //CEupuLogger4System::CreateInstance(".");

    LOGSETLEVEL((LOGLEVEL)1);

    LOGSETDEBUG(true);

    LOG(_INFO_, "main() start");

    //CGlobalConfig* pConfig = CGlobalConfig()->getInstance();
    //if (!pConfig->InitSysConfig("sysconf.json"))
    {
        LOG(_DEBUG_, "init config failed");
    }

    LOG(_ERROR_, "sigh!");
    //unsigned int ping_timer = pConfig->GetPingTimer();

    //char szErrMsg[256];
    //sprintf(szErrMsg, "%d", ping_timer);

    //LOG(_DEBUG_, szErrMsg);

    return 0;
}
