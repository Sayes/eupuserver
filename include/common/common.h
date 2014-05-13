#ifndef __COMMON_H__
#define __COMMON_H__

#include <string>
using std::string;

void daemonize();
string fgNtoA(unsigned int ip);
unsigned int fgAtoN(const char* ip);

#endif//__COMMON_H__
