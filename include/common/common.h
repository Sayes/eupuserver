#ifndef __COMMON_H__
#define __COMMON_H__

#include <string>
using std::string;

void daemonize();

const char *eupu_inet_ntop(int af, const void *src, char *dst, size_t size);
int eupu_inet_pton(int af, const char *src, void *dst);

string fgNtoA(unsigned int ip);
unsigned int fgAtoN(const char* ip);

#endif//__COMMON_H__
