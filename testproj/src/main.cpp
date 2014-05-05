#include <string>
#include <fstream>
#include "common.h"
#include "eupulogger4system.h"
#include "json/json.h"
using namespace std;

int main(int argc, char* argv[])
{
    CEupuLogger4System::Logger()->Debug4Sys("main");
    LOG(_DEBUG_, "This is a new test");
    std::ifstream f;
    f.open("sysconf.json");
    if (!f.is_open())
        CEupuLogger4System::Logger()->Debug4Sys("open json file failed");

    Json::Value v;
    Json::Reader r;

    if (!r.parse(f, v, false))
    {
        CEupuLogger4System::Logger()->Debug4Sys("parse json failed");
    }

    unsigned int listen_port = v["listen_port"].asInt();
    std::string listen_ip = v["listen_ip"].asString();

    CEupuLogger4System::Logger()->Debug4Sys(listen_ip.c_str());

    f.close();
    for (int i = 0; i < 10; ++i)
        i += 1;
    return 0;
}
