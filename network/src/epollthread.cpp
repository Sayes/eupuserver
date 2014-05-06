#include <sys/epoll.h>
#include "epollthread.h"
#include "sysqueue.h"
#include "common.h"
#include "globalmgr.h"

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
    return bret;
}

bool CEpollThread::doAccept(int fd)
{
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
