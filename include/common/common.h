// Copyright shenyizhong@gmail.com, 2014

#ifndef COMMON_COMMON_H_
#define COMMON_COMMON_H_

#ifdef OS_LINUX
#include <unistd.h>
#elif OS_WINDOWS
#include "myunistd.h"
#endif

#include <string>

void daemonize();

// const char *eupu_inet_ntop(int af, const void *src, char *dst, size_t size);
// int eupu_inet_pton(int af, const char *src, void *dst);

std::string fgNtoA(unsigned int ip);
unsigned int fgAtoN(const char *ip);

#ifdef OS_WINDOWS
unsigned int gettimeofday(struct timeval *tp, void *tzp);
#endif

#endif  // COMMON_COMMON_H_
