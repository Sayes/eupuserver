#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#ifdef OS_LINUX
#include <sys/resource.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#elif OS_WINDOWS
#include <ws2tcpip.h>
#endif
#include <iostream>
#include "eupu_inet.h"
#include "common.h"

using namespace std;

string fgNtoA(unsigned int ip)
{
	char buf[100];
	struct in_addr addr = {0};
	addr.s_addr = htonl(ip);
#ifdef OS_LINUX
	const char* p = inet_ntop(AF_INET, (void*)&addr, buf, (socklen_t)sizeof(buf));
#elif OS_WINDOWS
	const char* p = eupu_inet_ntop(AF_INET, (void*)&addr, buf, (socklen_t)sizeof(buf));
#endif
	return string(p);
}

unsigned int fgAtoN(const char* ip)
{
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


void daemonize()
{
#ifdef OS_LINUX
	pid_t pid;
	pid_t sid;
	int fd;

	pid = fork();
	if (pid == -1)
	{
		exit(-1);		
	}
	if (pid)
	{
		exit(0);
	}

	for (int fd = 0; fd < 255; ++fd)
	{
		close(fd);
	}

	sid = setsid();
	if (sid == -1)
	{
		abort();
	}

	fd = open("/dev/null", O_RDWR);
	if (fd >= 0)
	{
		if (fd != 0)
		{
			dup2(fd, 0);
		}
		if (fd != 1)
		{
			dup2(fd, 1);
		}
		if (fd != 2)
		{
			dup2(fd, 2);
		}
		if (fd > 2)
		{
			close(fd);
		}
	}

	struct rlimit rl;
	if (getrlimit(RLIMIT_NOFILE, &rl) != 0)
	{
		abort();
	}

	rl.rlim_cur = rl.rlim_max;
	if (setrlimit(RLIMIT_NOFILE, &rl) != 0)
	{
		abort();	
	}

	signal(SIGPIPE, SIG_IGN);
#endif
}
