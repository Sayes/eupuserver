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

		LOG(_INFO_, "CWSAThread::doListen() successed, listen fd=%d", m_listenfd);

		bret = true;

	} while(false);

	return bret;
}

bool CWSAThread::doAccept(int fd)
{
	return false;
}

bool CWSAThread::addClientToWSA(SOCKET_SET* psockset)
{
	return false;
}

void CWSAThread::doRecvMessage(SOCKET_KEY* pkey)
{
}

int CWSAThread::doSendMessage(SOCKET_KEY* pkey)
{
	return 0;
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
	return false;
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