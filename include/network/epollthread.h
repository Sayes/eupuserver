#ifndef __EUPUEPOLLTHREAD_H__
#define __EUPUEPOLLTHREAD_H__

#include <map>
#include <globaldef.h>
#include <euputhread.h>
#include <netcommon.h>

using std::map;

class CEpollThread : public CEupuThread {
public:
	CEpollThread();

	virtual ~CEpollThread();

	virtual bool Stop();
	virtual void Run();
	virtual void Reset();

	bool IsStop()
	{
		return false;
	}

	bool StartUp();

private:
	bool parsePacketToRecvQueue(SOCKET_SET* psocket, char* buf, int buflen);
	bool addSocketToMap(SOCKET_SET* psocket);
	bool addClientToEpoll(SOCKET_SET* psocket);
	bool doAccept(int fd);
	void doRecvMessage(SOCKET_KEY* pkey);
	void doSendMessage(SOCKET_KEY* pkey);
	bool doListen();

private:
	int m_epollfd;
	int m_listenfd;
	SOCKET_KEY* m_listenkey;
	int m_keepalivetimeout;		//connection timeout time
	int m_keepaliveinterval;	//send keepalive message to server time interval
	time_t m_checkkeepalivetime;
	time_t m_lastkeeplivetime;

	string m_serverip;
	unsigned int m_serverport;

	int m_readbufsize;
	int m_sendbufsize;
	char* m_recvbuffer;
	int m_recvbuflen;

	struct epoll_event* m_events;

	list<int> m_delsendfdlist;
	map<int, SOCKET_SET*> m_socketmap;

	int m_maxepollsize;

	list<NET_DATA*> m_recvlist;
	time_t m_Index;
	
};











#endif//__EUPUEPOLLTHREAD_H__
