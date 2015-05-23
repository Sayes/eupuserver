#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef OS_LINUX
#include<unistd.h>
#elif OS_WINDOWS
#include "myunistd.h"
#endif

#include <string>
using std::string;

void daemonize();

const char* eupu_inet_ntop(int af, const void* src, char* dst, size_t size);
int eupu_inet_pton(int af, const char* src, void* dst);

string fgNtoA(unsigned int ip);
unsigned int fgAtoN(const char* ip);

#ifdef OS_WINDOWS
int gettimeofday(struct timeval* tp, void* tzp);
#endif

#endif//__COMMON_H__
