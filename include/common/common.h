#ifndef __COMMON_H__
#define __COMMON_H__

#include <string>
using std::string;

string fgNtoA(unsigned int ip);
unsigned int fgAtoN(const char* ip);
void daemonize();

#endif//__COMMON_H__
