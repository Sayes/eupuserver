#include "common.h"
#include "eupulogger4system.h"

int main(int argc, char* argv[])
{
    CEupuLogger4System::Logger()->Debug4Sys("main");
    for (int i = 0; i < 10; ++i)
        i += 1;
    return 0;
}
