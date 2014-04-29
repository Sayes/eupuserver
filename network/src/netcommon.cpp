/*
 * =====================================================================================
 *
 *       Filename:  netcommon.cpp
 *
 *    Description:  :
 *
 *        Version:  1.0
 *        Created:  2013
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <globaldef.h>
#include <common.h>
#include "netcommon.h"

SOCKET_SET* initSocketset(int fd, time_t conntime, const string& peerip, unsigned short peerport, int ntype)
{
	SOCKET_KEY* key = new SOCKET_KEY;
	if (!key)
	{
		exit(-1);
	}

	SOCKET_SET* socketkey = new SOCKET_SET;
	if (!socketkey)
	{
		delete key;
		key = NULL;
		exit(-1);
	}

	key->fd = fd;
	key->connect_time = conntime;
	if (!socketkey->init(key, peerip, peerport, ntype))
	{
		delete key;
		key = NULL;
		delete socketkey;
		socketkey = NULL;
		return NULL;
	}
	return socketkey;
}

bool setNonBlock(int sockfd)
{
	int opts = fcntl(sockfd, F_GETFL);
	if (-1 == opts)
	{
		return false;	
	}

	opts = opts | O_NONBLOCK;
	if (fcntl(sockfd, F_SETFL, opts) < 0)
	{
		return false;
	}
	return true;
}

int recv_msg(int fd, char* buf, int &nlen)
{
	int n = nlen;
	char* p = buf;
	int nRet = 2;
	int nread = 0;

	while (n > 0)
	{
		nread = recv(fd, p, n, MSG_NOSIGNAL);
		if (nread < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else if (errno == EAGAIN)
			{
				nRet = 1;
				break;
			}
			nRet = -1;
			break;
		}
		else if (nread == 0)
		{
			nRet = 0;
			break;
		}

		n -= nread;
		p += nread;
	}

	nlen = nlen - n;
	return nRet;
}

int send_msg(int fd, char* buf, int &nlen)
{
	int n = nlen;
	int nRet = 1;
	int nsend = 0;
	char* p = buf;

	while (n > 0)
	{
		nsend = send(fd, p, n, MSG_NOSIGNAL);
		if (nsend < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else if (errno == EAGAIN)
			{
				nRet = 0;
				break;
			}
			nRet = -1;
			break;
		}
		n -= nsend;
		p += nsend; 
	}

	nlen = nlen - n;
	return nRet;
}

int doNonBlockConnect(PCONNECT_SERVER pserver, int timeout, const string& localip)
{
	if (!pserver)
		return -1;

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		return -1;

	if (!localip.empty())
	{
		struct sockaddr_in localaddr = {0};
		localaddr.sin_family = AF_INET;
		localaddr.sin_addr.s_addr = fgAtoN(localip.c_str());

		if (!bind(fd, (struct sockaddr*)&localaddr, sizeof(localaddr)) < 0)
		{
			close(fd);
			return -1;
		}
	}

	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(pserver->port);
	addr.sin_addr.s_addr = fgAtoN(pserver->host.c_str());

	if (!setNonBlock(fd))
	{
		close(fd);
		return -1;
	}

	unsigned int nsend = pserver->send_buffer;
	unsigned int nrecv = pserver->recv_buffer;

	if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &nsend, sizeof(nsend)) < 0)
	{
		close(fd);
		return -1;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &nrecv, sizeof(nrecv)) < 0)
	{
		close(fd);
		return -1;
	}

	int opt = 1;
	if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0)
	{
		close(fd);
		return -1;
	}

	int nret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
	if (nret == 0)
		return fd;

	if (nret < 0)
	{
		if (!(errno == EINPROGRESS || errno == EWOULDBLOCK))
		{
			close(fd);
			return -1;
		}
	}

	fd_set wset;
	struct timeval tv = {0};

	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	FD_ZERO(&wset);
	FD_SET(fd, &wset);

	nret = select(fd + 1, NULL, &wset, NULL, &tv);
	if (nret == 0)
	{
		close(fd);
		return -1;
	}
	else if (nret < 0)
	{
		close(fd);
		return -1;
	}

	int sock_err = 0;
	int sock_err_len = sizeof(sock_err);
	nret = getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&sock_err, (socklen_t*)&sock_err_len);

	if (nret < 0 || sock_err != 0)
	{
		close(fd);
		return -1;
	}
	return fd;
}


