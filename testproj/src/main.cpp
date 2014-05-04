#include <fstream>
#include "common.h"
#include "eupulogger4system.h"
#include "../../include/json/json.h"
using namespace Json;

int main(int argc, char* argv[])
{
    CEupuLogger4System::Logger()->Debug4Sys("main");
    std::ifstream f;
    f.open("a.json");
    if (!f.is_open())
        CEupuLogger4System::Logger()->Debug4Sys("open json file failed");

    Value v;
    Reader r;

    if (!r.parse(f, v, false))
    {
        CEupuLogger4System::Logger()->Debug4Sys("parse json failed");
    }

    std::string name = v["firstname"].asString();

    CEupuLogger4System::Logger()->Debug4Sys(name.c_str());

    f.close();
    for (int i = 0; i < 10; ++i)
        i += 1;
    return 0;
}
