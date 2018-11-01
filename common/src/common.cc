// Copyright shenyizhong@gmail.com, 2014

#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#ifdef OS_LINUX
#include <arpa/inet.h>
#include <sys/resource.h>
#include <sys/socket.h>
#elif OS_WINDOWS
#include <ws2tcpip.h>
#endif
#include <iostream>
#include "common/common.h"
#include "common/eupu_inet.h"

std::string fgNtoA(unsigned int ip) {
  char buf[100];
  struct in_addr addr = {0};
  addr.s_addr = htonl(ip);
#ifdef OS_LINUX
  const char *p = inet_ntop(AF_INET, (void *)&addr, buf, (socklen_t)sizeof(buf));
#elif OS_WINDOWS
  const char *p = eupu_inet_ntop(AF_INET, (void *)&addr, buf, (socklen_t)sizeof(buf));
#endif
  return std::string(p);
}

unsigned int fgAtoN(const char *ip) {
  struct in_addr addr = {0};
#ifdef OS_LINUX
  if (inet_pton(AF_INET, ip, &addr) < 0)
#elif OS_WINDOWS
  if (eupu_inet_pton(AF_INET, ip, &addr) < 0)
#endif
  {
    return 0;
  }
  return addr.s_addr;
}

void daemonize() {
#ifdef OS_LINUX
  pid_t pid;
  pid_t sid;
  int fd;

  pid = fork();
  if (pid == -1) {
    ::exit(-1);
  }
  if (pid) {
    ::exit(0);
  }

  for (int fd = 0; fd < 255; ++fd) {
    close(fd);
  }

  sid = setsid();
  if (sid == -1) {
    abort();
  }

  fd = open("/dev/null", O_RDWR);
  if (fd >= 0) {
    if (fd != 0) {
      dup2(fd, 0);
    }
    if (fd != 1) {
      dup2(fd, 1);
    }
    if (fd != 2) {
      dup2(fd, 2);
    }
    if (fd > 2) {
      close(fd);
    }
  }

  struct rlimit rl;
  if (getrlimit(RLIMIT_NOFILE, &rl) != 0) {
    abort();
  }

  rl.rlim_cur = rl.rlim_max;
  if (setrlimit(RLIMIT_NOFILE, &rl) != 0) {
    abort();
  }

  signal(SIGPIPE, SIG_IGN);
#endif
}

#ifdef OS_WINDOWS
unsigned int gettimeofday(struct timeval *tp, void *tzp) {
  time_t clock;
  struct tm tm;
  SYSTEMTIME wtm;
  GetLocalTime(&wtm);
  tm.tm_year = wtm.wYear - 1900;
  tm.tm_mon = wtm.wMonth - 1;
  tm.tm_mday = wtm.wDay;
  tm.tm_hour = wtm.wHour;
  tm.tm_min = wtm.wMinute;
  tm.tm_sec = wtm.wSecond;
  tm.tm_isdst = -1;
  clock = mktime(&tm);
  tp->tv_sec = clock;
  tp->tv_usec = wtm.wMilliseconds * 1000;
  return (0);
}
#endif
