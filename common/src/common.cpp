#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "common.h"

using namespace std;

unsigned int fgAtoN(const char* ip)
{
	struct in_addr addr = {0};
	if (inet_pton(AF_INET, ip, &addr) < 0)
	{	 
		return 0;
	}
	return addr.s_addr;
}

string fgNtoA(unsigned int ip)
{
	char buf[100];
	struct in_addr addr = {0};
	addr.s_addr = htonl(ip);
	const char* p = inet_ntop(AF_INET, (void*)&addr, buf, (socklen_t)sizeof(buf));
	return string(p);
}

void daemonize()
{
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
}
