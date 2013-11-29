#ifndef __GLOBALCONFIG_H__
#define __GLOBALCONFIG_H__

#include <string>

using namespace std;

struct __CONNECT_SERVER__ {
	string name;
	string host;
	unsigned short port;
	unsigned int send_buffer;
	unsigned int recv_buffer;
	__CONNECT_SERVER__()
	{
		port = 0;
		send_buffer = 8192;
		recv_buffer = 8192;
	}
};

typedef __CONNECT_SERVER__*	PCONNECT_SERVER;

#endif//__GLOBALCONFIG_H__
