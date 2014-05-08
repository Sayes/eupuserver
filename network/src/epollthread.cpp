#include <sys/epoll.h>
#include "epollthread.h"
#include "sysqueue.h"
#include "common.h"
#include "globalmgr.h"
#include "eupulogger4system.h"

CEpollThread::CEpollThread()
: CEupuThread()
, m_epollfd(-1)
, m_listenfd(-1)
, m_listenkey(NULL)
, m_maxepollsize(5000)
, m_keepalivetimeout(120)
, m_keepaliveinterval(60)
, m_serverport(0)
, m_events(NULL)
, m_recvbuffer(NULL)
, m_recvbuflen(0)
, m_Index(0)
{
	m_checkkeepalivetime = time(NULL);
	m_lastkeepalivetime = time(NULL);
}

CEpollThread::~CEpollThread()
{
	if (m_epollfd > 0)
	{
		close(m_epollfd);
	}

	if (m_listenfd > 0)
	{
		close(m_listenfd);
	}

	if (m_listenkey != NULL)
	{
		delete m_listenkey;
		m_listenkey = NULL;
	}

	map<int, SOCKET_SET*>::iterator it = m_socketmap.begin();
	for (; it != m_socketmap.end(); ++it)
	{
		delete it->second;
		it->second = NULL;
	}
	m_socketmap.clear();

	list<NET_DATA*>::iterator recviter = m_recvlist.begin();
	for (; recviter != m_recvlist.end(); ++recviter)
	{
		if ((*recviter) != NULL)
		{
			delete (*recviter);
			(*recviter) = NULL;
		}
	}
	m_recvlist.clear();

	if (m_events)
	{
		delete []m_events;
		m_events = NULL;
	}

	if (m_recvbuffer)
	{
		delete []m_recvbuffer;
		m_recvbuffer = NULL;
	}
}

// the thread work function, it will wait for the continue signal
// otherwise it will be pause
void CEpollThread::Run()
{
	pause();

	m_bIsExist = false;

	doEpollEvent();
	m_bIsExist = true;
}

void CEpollThread::Reset()
{
}

bool CEpollThread::StartUp()
{
	return false;
}

time_t CEpollThread::getIndex()
{
    m_Index++;
    if (m_Index == 0xFFFFFFFF)
        m_Index = 1;
    return m_Index;
}

void CEpollThread::doEpollEvent()
{
	int nevent = 0;
	
	SOCKET_KEY* pkey = NULL;

	while (m_bOperate)
	{
		///////begin handle epoll events///////
		nevent = epoll_wait(m_epollfd, m_events, m_maxepollsize, 1);
		for (int i = 0; i < nevent; ++i)
		{
			pkey = (SOCKET_KEY*)m_events[i].data.ptr;
			if (!pkey)
			{
				continue;
			}

			if (m_events[i].events & EPOLLIN)
			{
				if (pkey->fd == m_listenfd)
				{
					if (!doAccept(pkey->fd))
					{
						//LOG
					}
					continue;
				}

                doRecvMessage(pkey);
            }
            else if (m_events[i].events && EPOLLOUT)
            {
                if (doSendMessage(pkey) < 0)
                {
                    closeClient(pkey->fd, pkey->connect_time);
                }
                else
                {
                    struct epoll_event wev;
                    wev.events = EPOLLIN | EPOLLET;
                    wev.data.ptr = pkey;
                    if (epoll_ctl(m_epollfd, EPOLL_CTL_MOD, pkey->fd, &wev) < 0)
                    {
                        closeClient(pkey->fd, pkey->connect_time);
                    }
                }
            }
            else
            {
                closeClient(pkey->fd, pkey->connect_time);
            }
		}//end for

        //// end handle epoll events /////

        if (m_recvlist.size() > 0)
        {
            CSysQueue<NET_DATA>* precvlist = CGlobalMgr::getInstance()->getRecvQueue();
        }
	}
}


bool CEpollThread::Stop()
{
    return false;
}

void CEpollThread::doKeepaliveTimeout()
{

}

void CEpollThread::doSendkeepaliveToServer()
{
    time_t curtime = time(NULL);
    if (curtime - m_lastkeepalivetime < m_keepaliveinterval)
    {
        return;
    }

    m_lastkeepalivetime = time(NULL);

}

bool CEpollThread::doListen()
{
    bool bret = false;

    sockaddr_in serv_addr;
    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);

    do {
        if (m_listenfd < 0)
        {
            LOG(_ERROR_, "CEpollThread::doListen() create listen socket error");
            break;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(m_serverport);
        if (m_serverip.empty())
        {
            serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else
        {
            serv_addr.sin_addr.s_addr = fgAtoN(m_serverip.c_str());
        }

        int tmp = 1;
        if (setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp)) < 0)
        {
            LOG(_ERROR_, "CEpollThread::doListen() setsockopt(m_listenfd...) error");
            break;
        }

        if (!setNonBlock(m_listenfd))
        {
            LOG(_ERROR_, "CEpollThread::doLIsten() setNonBlock(m_listenfd) error");
            break;
        }

        if (bind(m_listenfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        {
            LOG(_ERROR_, "CEpollThread::doListen() bind(m_listenfd...) error");
            break;
        }

        if (listen(m_listenfd, 4096) < 0) //max pending queue
        {
            LOG(_ERROR_, "CEpollThread::doListen() listen(m_listenfd...) error");
            break;
        }

        m_listenkey = new SOCKET_KEY;
        if (!m_listenkey)
        {
            LOG(_ERROR_, "CEpollThread::doListen() new SOCKET_KEY error");
            exit(-1);
        }

        m_listenkey->fd = m_listenfd;
        m_listenkey->connect_time = getIndex();

        struct epoll_event ev;
        ev.data.ptr = m_listenkey;
        ev.events = EPOLLIN | EPOLLET;

        if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_listenfd, &ev) < 0)
        {
            LOG(_ERROR_, "CEpollThread::doListen() epoll_ctl(m_epollfd...) error");
            break;
        }

        LOG(_INFO_, "CEpollThread::doListen() successed, listen and add to epoll poll, m_listenfd=%d, m_epollfd=%d ", m_listenfd, m_epollfd);
        bret = true;

    } while (false);

    return bret;
}

bool CEpollThread::doAccept(int fd)
{
    struct sockaddr_in addr = {0};
    socklen_t addrlen = sizeof(addr);
    int connfd = -1;

    while (true)
    {
        addrlen = sizeof(addr);
        memset(&addr, 0, addrlen);
        connfd = accept(fd, (sockaddr*)&addr, &addrlen);

        if (connfd < 0)
        {
            if (errno != EAGAIN)
            {
                LOG(_ERROR_, "CEpollThread::doAccept(fd) accept(fd) error, fd=%d, errno=%s", fd, strerror(errno));
            }
            return true;
        }

        string peerip = fgNtoA(ntohl(addr.sin_addr.s_addr));
        unsigned short port = ntohs(addr.sin_port);

        if (!setNonBlock(connfd))
        {
            LOG(_ERROR_, "CEpollThread::doAccept(fd) setNonBlock(fd) error, fd=%d, peerip=%s, port=%d", fd, peerip.c_str(), port);
            close(connfd);
            continue;
        }

        if (setsockopt(connfd, SOL_SOCKET, SO_SNDBUF, &m_sendbufsize, sizeof(m_sendbufsize)) < 0)
        {
            LOG(_ERROR_, "CEpollThread::doAccept(fd) setsockopt(connfd...) error, connfd=%d, peerip=%s, port=%d, error=%s", connfd, peerip.c_str(), port, strerror(errno));
            close(connfd);
            continue;
        }

        if (setsockopt(connfd, SOL_SOCKET, SO_RCVBUF, &m_readbufsize, sizeof(m_readbufsize)) < 0)
        {
            LOG(_ERROR_, "CEpollThread::doAccept(fd) setsockopt(connfd...) error, connfd=%d, peerip=%s, port=%d, error=%s", connfd, peerip.c_str(), port, strerror(errno));
            close(connfd);
            continue;
        }

        int opt = 1;
        if (setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0)
        {
            LOG(_ERROR_, "CEpollThread::doAccept(fd) setsockopt(connfd...) error, connfd=%d, peerip=%s, port=%d, error=%s", connfd, peerip.c_str(), port, strerror(errno));
            close(connfd);
            continue;
        }

        SOCKET_SET* psocket = initSocketset(connfd, getIndex(), peerip, port, CLIENT_TYPE);

        if (!psocket)
        {
            LOG(_ERROR_, "CEpollThread::doAccept(fd) initSocketset(connfd...) error, connfd=%d, peerip=%s, port=%d, error=%s", connfd, peerip.c_str(), port, strerror(errno));
            close(connfd);
            continue;
        }

        if (!createConnectServerMsg(psocket))
        {
            LOG(_ERROR_, "CEpollThread::doAccept(fd) createConnectServerMsg(psocket) error, connfd=%d, conntime=%u, peerip=%s, port=%d, type=%d",
                psocket->key->fd, psocket->key->connect_time, GETNULLSTR(psocket->peer_ip), psocket->peer_port, psocket->type);
            delete psocket;
            close(connfd);
            continue;
        }

        if (!addClientToEpoll(psocket))
        {
            LOG(_ERROR_, "CEpollThread::doAccept(fd) addClientToEpoll(psocket) error, connfd=%d, conntime=%u, peerip=%s, port=%d, type=%d",
                psocket->key->fd, psocket->key->connect_time, GETNULLSTR(psocket->peer_ip), psocket->peer_port, psocket->type);
            delete psocket;
            close(connfd);
            continue;
        }
    }

    return true;
}

bool CEpollThread::addClientToEpoll(SOCKET_SET* psocket)
{
    if (psocket == NULL || psocket->key == NULL)
    {
        LOG(_ERROR_, "CEpollThread::addClientToEpoll() error"); 
        return false;
    }

    struct epoll_event ev = {0};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = psocket->key;

    if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, psocket->key->fd, &ev) < 0)
    {
        LOG(_ERROR_, "CEpollThread::addClientToEpoll() epoll_ctl() error, fd=%d, error=%s", psocket->key->fd, strerror(errno));
        return false;
    }

    if (!addSocketToMap(psocket))
    {
        LOG(_ERROR_, "CEpollThread::addClientToEpoll() addSocketToMap() error, fd=%d, peerip=%s, port=%d", psocket->key->fd, GETNULLSTR(psocket->peer_ip), psocket->peer_port);
        return false;
    }

    return true;
}

void CEpollThread::doRecvMessage(SOCKET_KEY* key)
{

}

int CEpollThread::doSendMessage(SOCKET_KEY* key)
{
    return 0; 
}

bool CEpollThread::parsePacketToRecvQueue(SOCKET_SET* psocket, char* buf, int buflen)
{
    return true;
}

void CEpollThread::closeClient(int fd, time_t conntime)
{

}
