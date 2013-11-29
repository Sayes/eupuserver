#ifndef __COMMON_H__
#define __COMMON_H__

#include <string>
using std::string;

void daemonize();
unsigned int fgAtoN(const char* ip);
string fgNtoA(unsigned int ip);

#endif//__COMMON_H__
