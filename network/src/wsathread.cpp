#ifdef OS_WINDOWS

#include "wsathread.h"
#include "sysqueue.h"
#include "common.h"
#include "globalmgr.h"
#include "eupulogger4system.h"
#include "sprotocol.h"

CWSAThread::CWSAThread()
: m_listenfd(INVALID_SOCKET)
, m_listenkey(NULL)
, m_keepalivetimeout(0)
, m_keepaliveinterval(0)
, m_checkkeepalivetime(0)
, m_lastkeepalivetime(0)
, m_serverport(0)
, m_readbufsize(0)
, m_sendbufsize(0)
, m_recvbuffer(NULL)
, m_recvbuflen(0)
, m_index(0)
, m_maxwsaeventsize(0)
, m_nEventTotal(0)
{
	for (int i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
	{
		m_eventArray[i] = WSA_INVALID_EVENT;
		m_sockArray[i] = INVALID_SOCKET;
	}

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(1, 1);
	err = ::WSAStartup(wVersionRequested, &wsaData);
	if (err)
	{
		WSACleanup();
	}
}

CWSAThread::~CWSAThread()
{
	for (int i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
	{
		if (m_eventArray[i] != WSA_INVALID_EVENT)
			::WSACloseEvent(m_eventArray[i]);
		if (m_sockArray[i] != INVALID_SOCKET)
			closesocket(m_sockArray[i]);
	}
	WSACleanup();

	if (m_listenfd > 0)
	{
		closesocket(m_listenfd);
	}

	if (m_listenkey != NULL)
	{
		delete m_listenkey;
		m_listenkey = NULL;
	}

	map<int, SOCKET_SET*>::iterator itersockmap = m_socketmap.begin();
	for (; itersockmap != m_socketmap.end(); ++itersockmap)
	{
		if (itersockmap->second)
		{
			delete itersockmap->second;
			itersockmap->second = NULL;
		}
	}
	m_socketmap.clear();

	list<NET_DATA*>::iterator iterdatalist = m_recvlist.begin();
	for (; iterdatalist != m_recvlist.end(); ++iterdatalist)
	{
		if ( (*iterdatalist) != NULL)
		{
			delete (*iterdatalist);
			(*iterdatalist) = NULL;
		}
	}
	m_recvlist.clear();

	if (m_recvbuffer)
	{
		delete []m_recvbuffer;
		m_recvbuffer = NULL;
	}
}

void CWSAThread::run()
{
	pause();

	m_bIsExit = false;

	doWSAEvent();

	m_bIsExit = true;
}

void CWSAThread::reset()
{
}

bool CWSAThread::startup()
{
	m_maxwsaeventsize = WSA_MAXIMUM_WAIT_EVENTS;
	m_keepalivetimeout = CGlobalConfig::getInstance()->getKeepaliveTimer();
	m_keepaliveinterval = m_keepalivetimeout / 2;
	m_serverip = CGlobalConfig::getInstance()->getListenIp();
	m_serverport = CGlobalConfig::getInstance()->getListenPort();

	m_sendbufsize = CGlobalConfig::getInstance()->getSocketSendBuf();
	m_readbufsize = CGlobalConfig::getInstance()->getSocketRecvBuf();

	m_recvbuflen = m_sendbufsize;
	if (m_sendbufsize < m_readbufsize)
	{
		m_recvbuflen = m_readbufsize;
	}
	if (m_recvbuflen < MAX_SEND_SIZE)
	{
		m_recvbuflen = MAX_SEND_SIZE;
	}
	m_recvbuflen += MAX_SEND_SIZE;

	m_recvbuffer = new char[m_recvbuflen];
	if (!m_recvbuffer)
	{
		LOG(_ERROR_, "CWSAThread::startup() error, new char[m_recvbuflen] failed, m_recvbuflen=%d", m_recvbuflen);
		exit(-1);
	}

	if (!doListen())
	{
		LOG(_ERROR_, "CWSAThread::startup() error, doListen() failed");
		delete []m_recvbuffer;
		m_recvbuffer = NULL;
		return false;
	}

	if (!start())
	{
		LOG(_ERROR_, "CWSAThread::startup() error, start() failed");
		delete []m_recvbuffer;
		m_recvbuffer = NULL;
		return false;
	}

	continues();

	return true;
}

time_t CWSAThread::getIndex()
{
	m_index++;
	if (m_index == 0xFFFFFFFF)
		m_index = 1;
	return m_index;
}

void CWSAThread::doWSAEvent()
{
	SOCKET_KEY* pkey = NULL;

	while (m_bOperate)
	{
		//WSA_MAXIMUM_WAIT_EVENTS
		int nIndex = ::WSAWaitForMultipleEvents(m_nEventTotal, m_eventArray, FALSE, WSA_INFINITE, FALSE);
		nIndex = nIndex - WSA_WAIT_EVENT_0;
		for (int i = nIndex; i < m_nEventTotal; ++i)
		{
			nIndex = ::WSAWaitForMultipleEvents(1, &m_eventArray[i], TRUE, 10, FALSE);

			if (nIndex == WSA_WAIT_FAILED || nIndex == WSA_WAIT_TIMEOUT)
			{
				LOG(_ERROR_, "CWSAThread::doWSAEvent() error, (nIndex == WSA_WAIT_FAILED || nIndex == WSA_WAIT_TIMEOUT)");
				continue;
			}
			else
			{
				WSANETWORKEVENTS event;
				if (::WSAEnumNetworkEvents(m_sockArray[i], m_eventArray[i], &event) == SOCKET_ERROR)
				{
					LOG(_ERROR_, "CWSAThread::doWSAEvent() error, ::WSAEnumNetworkEvents() failed, error=%s", WSAGetLastError());
					continue;
				}

				SOCKET_KEY* pkey = m_keymap[i];
				if (!pkey)
				{
					LOG(_ERROR_, "CWSAThread::doWSAEvent() error, pkey == NULL, SOCKET_KEY* pkey = m_keymap[%d]", i);
					continue;
				}

				if (event.lNetworkEvents & FD_ACCEPT)
				{
					LOG(_INFO_, "CWSAThread::doWSAEvent(), m_listenfd=%d, m_sockArray[i]=%d", m_listenfd, m_sockArray[i]);
					if (event.iErrorCode[FD_ACCEPT_BIT] == 0)
					{
						if (m_nEventTotal > WSA_MAXIMUM_WAIT_EVENTS)
						{
							LOG(_INFO_, "CWSAThread::doWSAEvent() error, m_nEventTotal > WSA_MAXIMUM_WAIT_EVENTS");
							continue;
						}

						if (m_listenfd == m_sockArray[i] && m_listenfd == pkey->fd)
						{
							if (!doAccept(m_listenfd))
							{
								LOG(_ERROR_, "CWSAThread::doWSAEvent() error, m_listenfd=%d", m_listenfd);
							}
							continue;
						}

						doRecvMessage(pkey);
					}
				}
				else if (event.lNetworkEvents & FD_READ)
				{
					LOG(_INFO_, "CWSAThread::doWSAEvent(), m_listenfd=%d, m_sockArray[i]=%d", m_listenfd, m_sockArray[i]);
					if (event.iErrorCode[FD_ACCEPT_BIT] == 0)
					{
						if (m_nEventTotal > WSA_MAXIMUM_WAIT_EVENTS)
						{
							LOG(_INFO_, "CWSAThread::doWSAEvent() error, m_nEventTotal > WSA_MAXIMUM_WAIT_EVENTS");
							continue;
						}

						doRecvMessage(pkey);
					}
				}
				else if (event.lNetworkEvents & FD_WRITE)
				{
					if (doSendMessage(pkey) < 0)
					{
						closeClient(pkey->fd, pkey->connect_time);
					}
					else
					{
						//TODO
						//WSAEVENT newevent = ::WSACreateEvent();
						//if (newevent == WSA_INVALID_EVENT)
						//{
						//	LOG(_ERROR_, "CWSAThread::doWSAEvent() error, WSAEventSelect() failed, fd=%d", psockset->key->fd);
						//	closesocket(psockset->key->fd);
						//	return false;
						//}
						//if (::WSAEventSelect(psockset->key->fd, newevent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
						//{
						//	LOG(_ERROR_, "CWSAThread::WSAEventSelect() error, WSAEventSelect() failed, fd=%d", psockset->key->fd);
						//	closesocket(psockset->key->fd);
						//	::WSACloseEvent(newevent);
						//	return false;
						//}
						//m_eventArray[m_nEventTotal] = newevent;
						//m_sockArray[m_nEventTotal] = psockset->key->fd;
						//m_keymap[m_nEventTotal] = psockset->key;
						//m_nEventTotal++;
					}
				}
				else
				{
					closeClient(pkey->fd, pkey->connect_time);
				}
			}
		}
	}
}

bool CWSAThread::stop()
{
	m_bOperate = false;
	while (!m_bIsExit)
	{
		Sleep(100);
	}
	return true;
}

void CWSAThread::doKeepaliveTimeout()
{
}

void CWSAThread::doSendKeepaliveToServer()
{
}

bool CWSAThread::doListen()
{
	bool bret = false;
	sockaddr_in serv_addr;
	m_listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	do {
		if (m_listenfd == INVALID_SOCKET)
		{
			LOG(_ERROR_, "CWSAThread::doListen() error, create listen socket failed");
			break;
		}
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(m_serverport);
		if (m_serverip.empty())
		{
			serv_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		}
		else
		{
			serv_addr.sin_addr.S_un.S_addr = fgAtoN(m_serverip.c_str());
		}

		int tmp = 1;
		if (setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, (char*)&tmp, sizeof(tmp)) == SOCKET_ERROR)
		{
			LOG(_ERROR_, "CWSAThread::doListen() error, setsockopt() failed, listen fd=%d, error=%ld", m_listenfd, WSAGetLastError());
			break;
		}

		if (!setNonBlock(m_listenfd))
		{
			LOG(_ERROR_, "CWSAThread::doListen() error, setNonBlock() failed, listen fd=%d", m_listenfd);
			break;
		}

		if (bind(m_listenfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
		{
			LOG(_ERROR_, "CWSAThread::doListen() error, bind() failed, listen fd=%d, error=%ld", m_listenfd, WSAGetLastError());
			break;
		}

		if (listen(m_listenfd, 4096) == SOCKET_ERROR)
		{
			LOG(_ERROR_, "CWSAThread::doListen() error, listen() failed, listen fd=%d, error=%ld", m_listenfd, WSAGetLastError());
			break;
		}

		m_listenkey = new SOCKET_KEY;
		if (!m_listenkey)
		{
			LOG(_ERROR_, "CWSAThread::doListen() error, new SOCKET_KEY failed, listen fd=%d", m_listenfd);
			closesocket(m_listenfd);
			m_listenfd = INVALID_SOCKET;
			exit(-1);
		}

		m_listenkey->fd = m_listenfd;
		m_listenkey->connect_time = getIndex();

		WSAEVENT event = ::WSACreateEvent();
		if (event == WSA_INVALID_EVENT)
		{
			LOG(_ERROR_, "CWSAThread::doListen() error, new SOCKET_KEY failed, listen fd=%d", m_listenfd);
			break;
		}

		if (::WSAEventSelect(m_listenfd, event, FD_ACCEPT | FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE) == SOCKET_ERROR)
		{
			LOG(_ERROR_, "CWSAThread::doListen() error, WSAEventSelect() failed, listen fd=%d, error=%ld", m_listenfd, WSAGetLastError());
			break;
		}

		m_eventArray[m_nEventTotal] = event;
		m_sockArray[m_nEventTotal] = m_listenfd;
		m_keymap[m_nEventTotal] = m_listenkey;
		m_nEventTotal++;

		LOG(_INFO_, "CWSAThread::doListen() successed, listen fd=%d", m_listenfd);

		bret = true;

	} while(false);

	return bret;
}

bool CWSAThread::doAccept(int fd)
{
	struct sockaddr_in addr = {0};
	socklen_t addrlen = sizeof(addr);
	int connfd = INVALID_SOCKET;

	while (true)
	{
		addrlen = sizeof(addr);
		connfd = accept(fd, (sockaddr*)&addr, &addrlen);
		if (connfd == INVALID_SOCKET)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				LOG(_ERROR_, "CWSAThread::doAccept() error, fd=%d, error=%d", fd, WSAGetLastError());
			}
			return true;
		}

		string peerip = fgNtoA(ntohl(addr.sin_addr.S_un.S_addr));
		unsigned short port = ntohs(addr.sin_port);

		if (!setNonBlock(connfd))
		{
			LOG(_ERROR_, "CWSAThread::doAccept() error, fd=%d, connfd=%d", fd, connfd);
			closesocket(connfd);
			continue;
		}

		if (setsockopt(connfd, SOL_SOCKET, SO_SNDBUF, (char*)&m_sendbufsize, sizeof(m_sendbufsize)) == SOCKET_ERROR)
		{
			LOG(_ERROR_, "CWSAThread::doAccept() error, setsockopt(SO_SNDBUF=%d) failed, fd=%d, connfd=%d, error=%d", m_sendbufsize, fd, connfd, WSAGetLastError());
			closesocket(connfd);
			continue;
		}

		if (setsockopt(connfd, SOL_SOCKET, SO_RCVBUF, (char*)&m_readbufsize, sizeof(m_readbufsize)) == SOCKET_ERROR)
		{
			LOG(_ERROR_, "CWSAThread::doAccept() error, setsockopt(SO_RCVBUF=%d) failed, fd=%d, connfd=%d, error=%d", m_readbufsize, fd, connfd, WSAGetLastError());
			closesocket(connfd);
			continue;
		}

		int opt = 1;
		if (setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, sizeof(opt)) < 0)
		{
			LOG(_ERROR_, "CWSAThread::doAccept() error, setsockopt(TCP_NODELAY) failed, fd=%d, connfd=%d, error=%d", fd, connfd, WSAGetLastError());
			closesocket(connfd);
			continue;
		}

		SOCKET_SET* psockset = initSocketset(connfd, getIndex(), peerip, port, CLIENT_TYPE);

		if (!psockset)
		{
			LOG(_ERROR_, "CWSAThread::doAccept() error, initSocketset() failed, fd=%d, connfd=%d, peerip=%s, port=%d", fd, connfd, GETNULLSTR(peerip), port);
			closesocket(connfd);
			continue;
		}

		if (!createConnectServerMsg(psockset))
		{
			LOG(_ERROR_, "CWSAThread::doAccept() error, initSocketset() failed, fd=%d, connfd=%d, peerip=%s, port=%d", fd, connfd, GETNULLSTR(peerip), port);
			delete psockset;
			psockset = NULL;
			closesocket(connfd);
			continue;
		}

		if (!addClientToWSA(psockset))
		{
			LOG(_ERROR_, "CWSAThread::doAccept() error, addClientToWSA() failed, fd=%d, connfd=%d, peerip=%s, port=%d", fd, connfd, GETNULLSTR(peerip), port);
			delete psockset;
			psockset = NULL;
			closesocket(connfd);
			continue;
		}
	}

	return true;
}

bool CWSAThread::addClientToWSA(SOCKET_SET* psockset)
{
	if (psockset == NULL || psockset->key == NULL)
	{
		return false;
	}

	WSAEVENT newevent = ::WSACreateEvent();
	if (newevent == WSA_INVALID_EVENT)
	{
		LOG(_ERROR_, "CWSAThread::doWSAEvent() error, WSAEventSelect() failed, fd=%d", psockset->key->fd);
		closesocket(psockset->key->fd);
		return false;
	}
	if (::WSAEventSelect(psockset->key->fd, newevent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
	{
		LOG(_ERROR_, "CWSAThread::WSAEventSelect() error, WSAEventSelect() failed, fd=%d", psockset->key->fd);
		closesocket(psockset->key->fd);
		::WSACloseEvent(newevent);
		return false;
	}
	m_eventArray[m_nEventTotal] = newevent;
	m_sockArray[m_nEventTotal] = psockset->key->fd;
	m_keymap[m_nEventTotal] = psockset->key;
	m_nEventTotal++;

	return true;
}

void CWSAThread::doRecvMessage(SOCKET_KEY* pkey)
{
	if (pkey == NULL)
	{
		return;
	}

	int buflen = 0;
	int nret = 0;

	map<int, SOCKET_SET*>::iterator itersockmap = m_socketmap.find(pkey->fd);
	if (itersockmap == m_socketmap.end() || itersockmap->second == NULL || itersockmap->second->key == NULL)
	{
		LOG(_ERROR_, "CWSAThread::doRecvMessage() error, can't find socket in map, fd=%d", pkey->fd);
		return;
	}

	if (itersockmap->second->key != pkey)
	{
		LOG(_ERROR_, "CWSAThread::doRecvMessage() error, the found socket dones't match pkey, fd=%d", pkey->fd);
		return;
	}

	SOCKET_SET* psockset = itersockmap->second;

	while (1)
	{
		buflen = m_recvbuflen;
		memset(m_recvbuffer, 0, buflen);

		buflen -= psockset->part_len;
		nret = recv_msg(pkey->fd, m_recvbuffer + psockset->part_len, buflen);
		if (nret < 0)
		{
			LOG(_ERROR_, "CWSAThread::doRecvMessage() error, recv_msg() failed, fd=%d, time=%u, peerip=%s, port=%d", psockset->key->fd, psockset->key->connect_time, GETNULLSTR(psockset->peer_ip), psockset->peer_port);
			return;
		}

		psockset->refresh_time = time(NULL);
		bool bparse = true;
		if (buflen > 0)
		{
			memcpy(m_recvbuffer, psockset->part_buf, psockset->part_len);
			buflen += psockset->part_len;
			psockset->part_len = 0;
			bparse = parsePacketToRecvQueue(psockset, m_recvbuffer, buflen);
			if (!bparse)
			{
				LOG(_ERROR_, "CWSAThread::doRecvMessage() error, parsePacketToRecvQueue() failed, fd=%d, time=%u, peerip=%s, port=%d", psockset->key->fd, psockset->key->connect_time, GETNULLSTR(psockset->peer_ip), psockset->peer_port);
			}
		}

		if (nret == 0)
		{
			LOG(_ERROR_, "CWSAThread::doRecvMessage() error, recv_msg() return 0, fd=%d, time=%u, peerip=%s, port=%d", psockset->key->fd, psockset->key->connect_time, GETNULLSTR(psockset->peer_ip), psockset->peer_port);
		}

		if (!bparse || nret == 0)
		{
			closeClient(pkey->fd, pkey->connect_time);
			return;
		}

		if (nret == 1)
		{
			break;	//EAGAIN
		}
	}
}

int CWSAThread::doSendMessage(SOCKET_KEY* pkey)
{
	if (pkey == NULL)
		return 0;

	map<int, SOCKET_SET*>::iterator itersockmap = m_socketmap.find(pkey->fd);
	if (itersockmap == m_socketmap.end() || itersockmap->second || itersockmap->second->key)
	{
		LOG(_ERROR_, "CWSAThread::doSendMessage() error, do not find socket in m_socketmap, fd=%d", pkey->fd);
		deleteSendMsgFromSendMap(pkey->fd);
		return 0;
	}

	int num = 0;
	int buflen = 0;

	map<int, list<NET_DATA*>*>* psendmap = CGlobalMgr::getInstance()->getBakSendMap();
	map<int, list<NET_DATA*>*>::iterator itersendmap = psendmap->find(pkey->fd);
	if (itersendmap == psendmap->end())
	{
		LOG(_ERROR_, "CWSAThread::doSendMessage() error, do not find data in m_sendmap, fd=%d", pkey->fd);
		return 0;
	}

	if (itersendmap->second == NULL)
	{
		LOG(_ERROR_, "CWSAThread::doSendMessage() error, data in m_sendmap is NULL, fd=%d", pkey->fd);
		return 0;
	}

	while (itersendmap->second->size() > 0)
	{
		NET_DATA* pdata = itersendmap->second->front();
		if (pdata == NULL)
		{
			itersendmap->second->pop_front();
			continue;
		}

		if (!((itersockmap->second->key->fd == pdata->fd) && (itersockmap->second->key->connect_time == pdata->connect_time)))
		{
			LOG(_ERROR_, "CWSAThread::doSendMessage() error, data and socket don't match in fd and connect_time, data fd=%d", pdata->fd);
			delete pdata;
			itersendmap->second->pop_front();
			continue;
		}

		buflen = pdata->data_len;
		int nsend = send_msg(pdata->fd, pdata->pdata, buflen);
		if (nsend < 0)
		{
			LOG(_ERROR_, "CWSAThread::doSendMessage() error, send_msg() failed, fd=%d", pdata->fd);
			deleteSendMsgFromSendMap(pkey->fd);
			return -1;
		}
		else if (nsend == 0)
		{
			//TODO, check here
			num = pdata->data_len - buflen;
			if (num > 0)
			{
				LOG(_INFO_, "CWSAThread::doSendMessage() error, send_msg() send part of data, fd=%d, len=%d", pdata->fd, buflen);
				pdata->data_len = num;
				memmove(pdata->pdata, pdata->pdata + buflen, num);
			}
			else
			{
				LOG(_INFO_, "CWSAThread::doSendMessage() successed, fd=%d, time=%u, peerip=%s, port=%d", pdata->fd, pdata->connect_time, GETNULLSTR(pdata->peer_ip), pdata->peer_port);
				delete pdata;
				itersendmap->second->pop_front();
			}
			break;
		}
		else
		{
			LOG(_INFO_, "CWSAThread::doSendMessage() successed, fd=%d, time=%u, peerip=%s, port=%d", pdata->fd, pdata->connect_time, GETNULLSTR(pdata->peer_ip), pdata->peer_port);
			delete pdata;
			itersendmap->second->pop_front();
			continue;
		}
	}//end while

	if (itersendmap->second->size() <= 0)
	{
		delete itersendmap->second;
		itersendmap->second = NULL;
		psendmap->erase(itersendmap);
	}

	return 1;
}

bool CWSAThread::parsePacketToRecvQueue(SOCKET_SET *psockset, char *buf, int buflen)
{
	return false;
}

void CWSAThread::closeClient(int fd, time_t conn_time)
{
}

void CWSAThread::createClientCloseMsg(SOCKET_SET *psockset)
{
}

bool CWSAThread::addSocketToMap(SOCKET_SET *psockset)
{
	return true;
}

void CWSAThread::deleteSendMsgFromSendMap(int fd)
{
}

void CWSAThread::doSystemEvent()
{
}

bool CWSAThread::createConnectServerMsg(SOCKET_SET *psockset)
{
	return false;
}

#endif//OS_WINDOWS