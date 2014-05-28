#ifdef OS_LINUX

#include <sys/epoll.h>
#include "epollthread.h"
#include "sysqueue.h"
#include "common.h"
#include "globalmgr.h"
#include "eupulogger4system.h"
#include "sprotocol.h"

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
, m_index(0)
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
void CEpollThread::run()
{
	pause();

	m_bIsExit = false;

	doEpollEvent();
	m_bIsExit = true;
}

void CEpollThread::reset()
{
}

bool CEpollThread::startup()
{
	m_maxepollsize = CGlobalConfig::getInstance()->getMaxEpollSize();
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
		LOG(_ERROR_, "CEpollThread::startup() error, new m_recvbuffer failed");
		exit(-1);
	}

	m_events = new epoll_event[m_maxepollsize];
	if (!m_events)
	{
		LOG(_ERROR_, "CEpollThread::startup() error, new m_events failed");
		exit(-1);
	}

	m_epollfd = epoll_create(m_maxepollsize);
	if (m_epollfd < 0)
	{
		LOG(_ERROR_, "CEpollThread::startup() error, epoll_create() failed, request epoll size %d", m_maxepollsize);
		delete []m_recvbuffer;
		m_recvbuffer = NULL;
		delete []m_events;
		m_events = NULL;
		return false;
	}

	if (!doListen())
	{
		LOG(_ERROR_, "CEpollThread::startup() error, doListen() failed, m_epollfd=%d", m_epollfd);
		delete []m_recvbuffer;
		m_recvbuffer = NULL;
		delete []m_events;
		m_events = NULL;
		return false;
	}

	if (!start())
	{
		LOG(_ERROR_, "CEpollThread::startup() error, start() failed, m_epollfd=%d", m_epollfd);
		delete []m_recvbuffer;
		m_recvbuffer = NULL;
		delete []m_events;
		m_events = NULL;
		return false;
	}

	continues();

	return true;
}

time_t CEpollThread::getIndex()
{
	m_index++;
	if (m_index == 0xFFFFFFFF)
		m_index = 1;
	return m_index;
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
                LOG(_ERROR_, "CEpollThread::doEpollEvent() error, (SOCKET_KEY*)m_events[i].data.ptr == NULL, i=%d", i);
				continue;
			}

			if (m_events[i].events & EPOLLIN)
			{
				if (pkey->fd == m_listenfd)
				{
					if (!doAccept(pkey->fd))
					{
						LOG(_ERROR_, "CEpollThread::doEpollEvent() error, doAccept(pkey->fd) failed, fd=%d", pkey->fd);
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
                        LOG(_ERROR_, "CEpollThread::doEpollEvent() error, epoll_ctl(m_epollfd, EPOLL_CTL_MOD, pkey->fd, &wev) failed, fd=%d", pkey->fd);
						closeClient(pkey->fd, pkey->connect_time);
					}
				}
			}
			else
			{
                LOG(_ERROR_, "CEpollThread::doEpollEvent() error, epoll recv unknown error, fd=%d, conn_time=%u", pkey->fd, pkey->connect_time);
				closeClient(pkey->fd, pkey->connect_time);
			}
		}//end for

		//// end handle epoll events /////

		if (m_recvlist.size() > 0)
		{
			CSysQueue<NET_DATA>* precvlist = CGlobalMgr::getInstance()->getRecvQueue();
            precvlist->Lock();
            list<NET_DATA*>::iterator iter = m_recvlist.begin();
            for (; iter != m_recvlist.end(); ++iter)
            {
                if ( (*iter) == NULL)
                    continue;
                if (!precvlist->inQueueWithoutLock(*iter, false))
                {
                    LOG(_ERROR_, "CEpollThread::doEpollEvent() error, inQueueWithoutLock() failed");
                    delete (*iter);
                }
            }
            precvlist->UnLock();
            m_recvlist.clear();
		}

        doSystemEvent();

        struct epoll_event ev;
        CGlobalMgr::getInstance()->switchSendMap();
        map<int, list<NET_DATA*>*>* psendmap = CGlobalMgr::getInstance()->getBakSendMap();

        for (map<int, list<NET_DATA*>*>::iterator itersendmap = psendmap->begin(); itersendmap != psendmap->end(); ++itersendmap)
        {
            map<int, SOCKET_SET*>::iterator itersockmap = m_socketmap.find(itersendmap->first);
            if (itersockmap == m_socketmap.end() || itersockmap->second == NULL || itersockmap->second->key == NULL)
            {
                LOG(_ERROR_, "CEpollThread::doEpollEvent() error, m_socketmap.find(fd) failed");
                m_delsendfdlist.push_back(itersendmap->first);
                continue;
            }

            ev.events = EPOLLOUT | EPOLLET;
            ev.data.ptr = itersockmap->second->key;
            if (epoll_ctl(m_epollfd, EPOLL_CTL_MOD, itersendmap->first, &ev) < 0)
            {
                LOG(_ERROR_, "CEpollThread::doEpollEvent() error, epoll_ctl() failed, fd=%d, error=%s", itersendmap->first, strerror(errno));
                m_delsendfdlist.push_back(itersendmap->first);
                closeClient(itersockmap->second->key->fd, itersockmap->second->key->connect_time);
                continue;
            }
        }

        for (list<int>::iterator iterdelsendfdlist = m_delsendfdlist.begin(); iterdelsendfdlist != m_delsendfdlist.end(); ++iterdelsendfdlist)
        {
            deleteSendMsgFromSendMap(*iterdelsendfdlist);
        }
        m_delsendfdlist.clear();

        doKeepaliveTimeout();

        doSendKeepaliveToServer();
	}//end while

    return;
}

bool CEpollThread::stop()
{
	m_bOperate = false;
	while (!m_bIsExit)
	{
		usleep(10);
	}
	return true;
}

void CEpollThread::doKeepaliveTimeout()
{
	time_t curtime = time(NULL);
	if (curtime - m_checkkeepalivetime < m_keepaliveinterval)
	{
		return;
	}

	list<int> timelist;
	map<int, SOCKET_SET*>::iterator iter = m_socketmap.begin();
	for (; iter != m_socketmap.end(); ++iter)
	{
		if (iter->second != NULL)
		{
			if (iter->second->refresh_time + m_keepalivetimeout > curtime)
			{
				timelist.push_back(iter->first);
			}
		}
		else
		{
			timelist.push_back(iter->first);
		}
	}//end for

	bool bclosed = false;
	for (list<int>::iterator itertimeout = timelist.begin(); itertimeout != timelist.end(); ++itertimeout)
	{
		bclosed = false;
		map<int, SOCKET_SET*>::iterator itersocket = m_socketmap.find(*itertimeout);
		if (itersocket != m_socketmap.end())
		{
			if (itersocket->second != NULL)
			{
				if (itersocket->second->key != NULL)
				{
					bclosed = true;
					LOG(_ERROR_, "CEpollThread::doKeepaliveTimeout() error, close socket_set for timeout");
					closeClient(itersocket->first, itersocket->second->key->connect_time);
				}
			}

			if (!bclosed)
			{
				LOG(_ERROR_, "CEpollThread::doKeepaliveTimeout() error, close socket_set->key->fd close for timeout");
				close(itersocket->first);
				delete itersocket->second;
				m_socketmap.erase(itersocket);
			}
		}
	}//end for
	timelist.clear();
	m_checkkeepalivetime = time(NULL);
}

void CEpollThread::doSendKeepaliveToServer()
{
	time_t curtime = time(NULL);
	if (curtime - m_lastkeepalivetime < m_keepaliveinterval)
	{
		return;
	}

	CGlobalMgr::getInstance()->sendKeepaliveMsgToAllServer();

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
			LOG(_ERROR_, "CEpollThread::doListen() bind(m_listenfd...) error, m_listenfd=%d, ip=%s", m_listenfd, GETNULLSTR(m_serverip));
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

		SOCKET_SET* psockset = initSocketset(connfd, getIndex(), peerip, port, CLIENT_TYPE);

		if (!psockset)
		{
			LOG(_ERROR_, "CEpollThread::doAccept(fd) initSocketset(connfd...) error, connfd=%d, peerip=%s, port=%d, error=%s", connfd, peerip.c_str(), port, strerror(errno));
			close(connfd);
			continue;
		}

		if (!createConnectServerMsg(psockset))
		{
			LOG(_ERROR_, "CEpollThread::doAccept(fd) createConnectServerMsg(psockset) error, connfd=%d, conntime=%u, peerip=%s, port=%d, type=%d",
				psockset->key->fd, psockset->key->connect_time, GETNULLSTR(psockset->peer_ip), psockset->peer_port, psockset->type);
			delete psockset;
			close(connfd);
			continue;
		}

		if (!addClientToEpoll(psockset))
		{
			LOG(_ERROR_, "CEpollThread::doAccept(fd) addClientToEpoll(psockset) error, connfd=%d, conntime=%u, peerip=%s, port=%d, type=%d",
				psockset->key->fd, psockset->key->connect_time, GETNULLSTR(psockset->peer_ip), psockset->peer_port, psockset->type);
			delete psockset;
			close(connfd);
			continue;
		}
	}

	return true;
}

bool CEpollThread::addClientToEpoll(SOCKET_SET* psockset)
{
	if (psockset == NULL || psockset->key == NULL)
	{
		LOG(_ERROR_, "CEpollThread::addClientToEpoll() error"); 
		return false;
	}

	struct epoll_event ev = {0};
	ev.events = EPOLLIN | EPOLLET;
	ev.data.ptr = psockset->key;

	if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, psockset->key->fd, &ev) < 0)
	{
		LOG(_ERROR_, "CEpollThread::addClientToEpoll() epoll_ctl() error, fd=%d, error=%s", psockset->key->fd, strerror(errno));
		return false;
	}

	if (!addSocketToMap(psockset))
	{
		LOG(_ERROR_, "CEpollThread::addClientToEpoll() addSocketToMap() error, fd=%d, peerip=%s, port=%d", psockset->key->fd, GETNULLSTR(psockset->peer_ip), psockset->peer_port);
		return false;
	}

	return true;
}

void CEpollThread::doRecvMessage(SOCKET_KEY* pkey)
{
	if (pkey == NULL)
		return;

	int buflen = 0;
	int nret = 0;

	map<int, SOCKET_SET*>::iterator iter = m_socketmap.find(pkey->fd);

	if (iter == m_socketmap.end() || iter->second == NULL || iter->second->key == NULL)
	{
		LOG(_ERROR_, "CEpollThread::doRecvMessage() fd not found in m_socketmap, fd=%d, time=%u", pkey->fd, pkey->connect_time);
		closeClient(pkey->fd, pkey->connect_time);
		return;
	}

	if (iter->second->key != pkey)
	{
		LOG(_ERROR_, "CEpollThread::doRecvMessage() the found socket doesn't match, fd=%d, cur=%p, old=%p", pkey->fd, pkey, iter->second->key);
		closeClient(pkey->fd, pkey->connect_time);
		return;
	}

	SOCKET_SET* psockset = iter->second;

	while (1)
	{
		buflen = m_recvbuflen;
		memset(m_recvbuffer, 0, m_recvbuflen);

		buflen -= psockset->part_len;
		nret = recv_msg(pkey->fd, m_recvbuffer + psockset->part_len, buflen);

		if (nret < 0)
		{
			LOG(_ERROR_, "CEpollThread::doRecvMessage() recv_msg() error, fd=%d, time=%u, peerip=%s, port=%d", pkey->fd, pkey->connect_time, GETNULLSTR(iter->second->peer_ip), iter->second->peer_port);
			closeClient(key->fd, key->connect_time); 
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
				LOG(_ERROR_, "CEpollThread::doRecvMessage() parsePacketToRecvQueue() error, fd=%d, time=%u, peerip=%s, port=%d",
					pkey->fd, pkey->connect_time, GETNULLSTR(iter->second->peer_ip), iter->second->peer_port);
			}
		}

		if (nret == 0)
		{
            LOG(_INFO_,"the peer have closed the connection, fd:%d, time:%u, peerip:%s, port:%d",
                key->fd, key->connect_time, GETNULLSTR(iter->second->peer_ip), iter->second->peer_port);
		}

		if (nret == 0 || !bparse)
		{
			closeClient(pkey->fd, pkey->connect_time);
			return;
		}

		if (nret == 1)
		{
			break;  //EAGAIN
		}
	}//end while
}

int CEpollThread::doSendMessage(SOCKET_KEY* pkey)
{
	if (!pkey)
		return 0;

	map<int, SOCKET_SET*>::iterator itermap = m_socketmap.find(pkey->fd);

	if (itermap == m_socketmap.end() || itermap->second == NULL || itermap->second->key == NULL)
	{
		LOG(_ERROR_, "CEpollThread::doSendMessage() error, not found socket, fd=%d", pkey->fd);
		deleteSendMsgFromSendMap(pkey->fd);
		return 0;
	}

	int num = 0;
	int buflen = 0;
	map<int, list<NET_DATA*> *> * psendmap = CGlobalMgr::getInstance()->getBakSendMap();
	map<int, list<NET_DATA*> *>::iterator itersend = psendmap->find(pkey->fd);
	if (itersend == psendmap->end())
	{
		LOG(_WARN_, "CEpollThread::doSendMessage() not found the socket %d at send msg map", pkey->fd);
		return 0;
	}

	if (itersend->second == NULL)
	{
		LOG(_DEBUG_, "CEpollThread::doSendMessage(), the send message list is empty");
		psendmap->erase(itersend);
		return 0;
	}

	while (itersend->second->size() > 0)
	{
		NET_DATA* pdata = itersend->second->front();
		if (!pdata)
		{
			LOG(_WARN_, "CEpollThread::doSendMessage(), send message data is empty, fd=%d", pkey->fd);
			itersend->second->pop_front();
			continue;
		}

		if ( !((itermap->second->key->fd == pdata->fd) && (itermap->second->key->connect_time == pdata->connect_time)) )
		{
			LOG(_ERROR_, "CEpollThread::doSendMessage(), send message data connect time is not match, cur=%u, old=%u", pdata->connect_time, itermap->second->key->connect_time);
			LOGHEX(_ERROR_, "the send message:", pdata->pdata, pdata->data_len);
			delete pdata;
			itersend->second->pop_front();
			continue;
		}

		buflen = pdata->data_len;
		int nsend = send_msg(pdata->fd, pdata->pdata, buflen);
		if (nsend < 0)
		{
			LOG(_ERROR_, "CEpollThread::doSendMessage() send_msg() error, fd=%d, time=%u, peerip=%s, port=%d, err=%s",
				pdata->fd, pdata->connect_time, GETNULLSTR(itermap->second->peer_ip), itermap->second->peer_port, strerror(errno));
			LOGHEX(_DEBUG_, "send message failed:", pdata->pdata, pdata->data_len);
			deleteSendMsgFromSendMap(pkey->fd);
			return -1;
		}
		else if (nsend == 0)
		{
			num = pdata->data_len - buflen;
			if (num > 0)
			{
				LOG(_INFO_, "CEpollThread::doSendMessage() send part message success, fd=%d, time=%u, peerip=%s, port=%d",
					pdata->fd, pdata->connect_time, GETNULLSTR(itermap->second->peer_ip), itermap->second->peer_port);
				LOGHEX(_DEBUG_, "send part message:", pdata->pdata, buflen);
				pdata->data_len = num;
				memmove(pdata->pdata, pdata->pdata + buflen, num);
			}
			else
			{
				LOG(_INFO_, "CEpollThread::doSendMessage() send message success, fd=%d, time=%u, peerip=%s, port=%d",
					pdata->fd, pdata->connect_time, GETNULLSTR(itermap->second->peer_ip), itermap->second->peer_port);
				LOGHEX(_DEBUG_, "send message:", pdata->pdata, buflen);

				delete pdata;
				itersend->second->pop_front();
			}
			break;
		}
		else
		{
			LOG(_INFO_, "CEpollThread::doSendMessage() send message success, fd=%d, time=%u, peerip=%s, port=%d",
				pdata->fd, pdata->connect_time, GETNULLSTR(itermap->second->peer_ip), itermap->second->peer_port);
			LOGHEX(_DEBUG_, "send message:", pdata->pdata, buflen);
			delete pdata;
			itersend->second->pop_front();
			continue;
		}
	}//end while

	if (itersend->second->size() <= 0)
	{
		delete itersend->second;
		itersend->second = NULL;
		psendmap->erase(itersend);
	}
	return 1; 
}

bool CEpollThread::parsePacketToRecvQueue(SOCKET_SET* psockset, char* buf, int buflen)
{
	if (!psockset || !psockset->key)
	{
		LOG(_ERROR_, "CEpollThread::parsePacketToRecvQueue() error, param is null");
		return false;
	}

	if (buf == NULL || buflen <= 0)
	{
		LOG(_ERROR_, "CEpollThread::parsePackageToRecvQueue() error, buflen < 0");
		return false;
	}

	LOG(_DEBUG_, "recv message:", buf, buflen);

	int curpos = 0;

	while (curpos < buflen)
	{
		if (buflen - curpos < NET_HEAD_SIZE)
		{
			memcpy(psockset->part_buf, buf, buflen - curpos);
			psockset->part_len = buflen - curpos;
			return true;
		}

		unsigned short nmsgsize = *((unsigned short *)(buf + curpos));
		unsigned short nlen = ntohs(nmsgsize);

		if (nlen > MAX_SEND_SIZE || nlen < NET_HEAD_SIZE)
		{
			LOG(_ERROR_, "CEpollThread::parsePacketToRecvQueue() error, invalid message length, fd=%d, peerip=%s, port=%d, len=%d",
				psockset->key->fd, GETNULLSTR(psockset->peer_ip), psockset->peer_port, nlen);
			return false;
		}

		if (nlen > buflen - curpos)
		{
			memcpy(psockset->part_buf, buf, buflen - curpos);
			psockset->part_len = buflen - curpos;
			return true;
		}

		NET_DATA* pdata = new NET_DATA;
		if (!pdata)
		{
			LOG(_ERROR_, "CEpollThread::parsePacketToRecvQueue() error, create NET_DATA failed, fd=%d, peerip=%s, port=%d",
				psockset->key->fd, GETNULLSTR(psockset->peer_ip), psockset->peer_port);
			LOGHEX(_DEBUG_, "recv message:", buf, buflen);
			exit(-1);
		}

		if (!pdata->init(psockset->key->fd, psockset->key->connect_time, psockset->peer_ip, psockset->peer_port, psockset->type, nlen))
		{
			LOG(_ERROR_, "CEpollThread::parsePacketToRecvQueue() error, NET_DATA::init() failed, fd=%d, peerip=%s, port=%d",
				psockset->key->fd, GETNULLSTR(psockset->peer_ip), psockset->peer_port);
			LOGHEX(_DEBUG_, "recv message:", buf, buflen);
			delete pdata;
			pdata = NULL;
			return false;
		}

		memcpy(pdata->pdata, buf + curpos, nlen);
		pdata->data_len = nlen;
		m_recvlist.push_back(pdata);
		curpos += nlen;

	}//end while
	return true;
}

void CEpollThread::closeClient(int fd, time_t conntime)
{
	if (fd < 0)
	{
		LOG(_ERROR_, "CEpollThread::closeClient() error, fd < 0");
		return;
	}

	if (epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, NULL) < 0)
	{
		LOG(_ERROR_, "CEpollThread::closeClient() error, delete socket from epoll failed, fd=%d, error=%s", fd, strerror(errno));
	}

	map<int, SOCKET_SET*>::iterator iter = m_socketmap.find(fd);
	close(fd);
	if (iter == m_socketmap.end())
	{
		LOG(_ERROR_, "CEpollThread::closeClient() error, can't find socket in map, fd=%d", fd);
	}
	else
	{
		if (iter->second != NULL)
		{
			if (iter->second->type > CLIENT_TYPE)
			{
				CGlobalMgr::getInstance()->setServerSocket(-1, 0, "", 0, iter->second->type);
			}
			createClientCloseMsg(iter->second);
			delete iter->second;
			iter->second = NULL;
		}
		else
		{
			LOG(_ERROR_, "CEpollThread::closeClient(), the found socket is NULL");
		}
		m_socketmap.erase(iter);
	}
}

void CEpollThread::createClientCloseMsg(SOCKET_SET* psockset)
{
	if (psockset == NULL || psockset->key == NULL)
	{
		LOG(_ERROR_, "CEpollThread::createClientCloseMsg() error, param psockset == NULL");
		return;
	}

	char buf[MAX_SEND_SIZE];
	UINT buflen = sizeof(buf);
	memset(buf, 0, buflen);

	MP_Server_DisConnected msg;
	msg.m_nServer = psockset->type;
	if (!msg.Out((BYTE*)buf, buflen))
	{
		LOG(_ERROR_, "CEpollThread::createClientCloseMsg() error, msg.Out(buf, buflen) failed, fd=%d, time=%u, type=%d", psockset->key->fd, psockset->key->connect_time, psockset->type);
		return;
	}

	NET_DATA* pdata = new NET_DATA;
	if (!pdata)
	{
		LOG(_ERROR_, "CEpollThread::createClientCloseMsg() error, create NET_DATA failed, fd=%d, time=%u, type=%d", psockset->key->fd, psockset->key->connect_time, psockset->type);
		exit(-1);
	}

	if (!pdata->init(psockset->key->fd, psockset->key->connect_time, psockset->peer_ip, psockset->peer_port, psockset->type, buflen))
	{
		LOG(_ERROR_, "CEpollThread::createClientCloseMsg() error, pdata->init(fd...) failed, fd=%d, time=%u, type=%d", psockset->key->fd, psockset->key->connect_time, psockset->type);
		delete pdata;
		pdata = NULL;
		return;
	}

	memcpy(pdata->pdata, buf, buflen);
	pdata->data_len = buflen;
	m_recvlist.push_back(pdata);
}

bool CEpollThread::addSocketToMap(SOCKET_SET* psockset)
{
	if (psockset == NULL || psockset->key == NULL || psockset->key->fd < 0)
	{
		LOG(_ERROR_, "CEpollThread::addSocketToMap() error, param psockset == NULL");
		return false;
	}

	if (m_maxepollsize <= m_socketmap.size())
	{
		LOG(_ERROR_, "CEpollThread::addSocketToMap() error, the epoll poll is full, fd=%d, peerip=%s, port=%d", psockset->key->fd, GETNULLSTR(psockset->peer_ip), psockset->peer_port);
		return false;
	}

	map<int, SOCKET_SET*>::iterator iter = m_socketmap.find(psockset->key->fd);
	if (iter != m_socketmap.end())
	{
		if (iter->second && iter->second->key)
		{
			LOG(_WARN_, "CEpollThread::addSocketToMap() error, found timeout connect, fd=%d, peerip=%s, port=%d", psockset->key->fd, GETNULLSTR(psockset->peer_ip), psockset->peer_port);
		}
		else
		{
			LOG(_WARN_, "CEpollThread::addSocketToMap() error, found timeout connect, socket == NULL");
		}

		if (iter->second != NULL)
		{
			delete iter->second;
			iter->second = NULL;
		}

		m_socketmap.erase(iter);
	}

	m_socketmap.insert(map<int, SOCKET_SET*>::value_type(psockset->key->fd, psockset));
	return true;
}

void CEpollThread::deleteSendMsgFromSendMap(int fd)
{
	if (fd < 0)
	{
		LOG(_ERROR_, "CEpollThread::deleteSendMsgFromSendMap(int fd) error, invalid fd");
		return;
	}

	map<int, list<NET_DATA*>*>* psendmap = CGlobalMgr::getInstance()->getBakSendMap();
	map<int, list<NET_DATA*>*>::iterator itersend = psendmap->find(fd);

	if (itersend != psendmap->end())
	{
		if (itersend->second != NULL)
		{
			list<NET_DATA*>::iterator iterdata = itersend->second->begin();
			for (; iterdata != itersend->second->end(); ++iterdata)
			{
				delete *iterdata;
			}
			itersend->second->clear();
			delete itersend->second;
			itersend->second = NULL;
		}
		psendmap->erase(itersend);
	}

	return;

}

void CEpollThread::doSystemEvent()
{
	CSysQueue<NET_EVENT>* pevent = CGlobalMgr::getInstance()->getEventQueue();
	if (pevent == NULL || pevent->isEmptyWithoutLock())
	{
		return;
	}

	pevent->Lock();
	NET_EVENT* pdata = NULL;
	while (pevent->outQueueWithoutLock(pdata, true))
	{
		if (pdata == NULL)
			continue;

		switch (pdata->eventid)
		{
		case CLOSE_CLIENT:
			{
				if (pdata->data != NULL)
				{
					SOCKET_KEY* pkey = (SOCKET_KEY*)pdata->data;
					LOG(_INFO_, "CEpollThread::doSystemEvent(), closeClient(), fd=%d, conn_time=%u", pkey->fd, pkey->connect_time);
					closeClient(pkey->fd, pkey->connect_time);
					delete pkey;
				}
				break;
			}
		case ADD_CLIENT:
			{
				if (pdata->data == NULL)
				{
					LOG(_ERROR_, "CEpollThread::doSystemEvent(), ADD_CLIENT event data is null");
					break;
				}
				SOCKET_SET* psockset = (SOCKET_SET*)pdata->data;
				if (psockset->key == NULL)
				{
					LOG(_ERROR_, "CEpollThread::doSystemEvent(), ADD_CLIENT socket->key is null");
					delete psockset;
					break;
				}

				psockset->key->connect_time = getIndex();
				CGlobalMgr::getInstance()->setServerSocket(psockset->key->fd, psockset->key->connect_time, psockset->peer_ip, psockset->peer_port, psockset->type);

				if (!createConnectServerMsg(psockset))
				{
					close(psockset->key->fd);
					delete psockset;
					break;
				}

				if (!addClientToEpoll(psockset))
				{
					LOG(_ERROR_, "CEpollThread::doSystemEvent() error, addClientToEpoll() failed, fd=%d, time=%u, peer_ip=%s, port=%d",
						psockset->key->fd, psockset->key->connect_time, GETNULLSTR(psockset->peer_ip), psockset->peer_port);
					close(psockset->key->fd);
					delete psockset;
					break;
				}
				break;
			}
		default:
			{
				LOG(_ERROR_, "CEpollThread::doSystemEvent() error, invalid event");
				if (pdata->data != NULL)
					delete (char*)pdata->data;
				break;
			}
		}
	}//end while
	pevent->UnLock();
}

bool CEpollThread::createConnectServerMsg(SOCKET_SET* psockset)
{
	if (psockset == NULL || psockset->key == NULL)
	{
		LOG(_ERROR_, "CEpollThread::createConnectServerMsg() error, param SOCKET_SET* is NULL");
		return false;
	}


	char buf[MAX_SEND_SIZE];
	UINT buflen = sizeof(buf);
	memset(buf, 0, buflen);

	MP_Server_Connected msg;
	msg.m_nServer = psockset->type;

	if (!msg.Out((BYTE*)buf, buflen))
	{
		LOG(_ERROR_, "CEpollThread::createConnectServerMsg() error, msg.Out() failed, fd=%d, conn_time=%u, peer_ip=%s, port=%d",
			psockset->key->fd, psockset->key->connect_time, GETNULLSTR(psockset->peer_ip), psockset->peer_port);
		return false;
	}

	NET_DATA* pdata = new NET_DATA;
	if (pdata == NULL)
	{
		LOG(_ERROR_, "CEpollThread::createConnectServerMsg() error, net NET_DATA failed, fd=%d, conn_time=%u, peer_ip=%s, port=%d",
			psockset->key->fd, psockset->key->connect_time, GETNULLSTR(psockset->peer_ip), psockset->peer_port);
		exit(-1);
	}

	if (!pdata->init(psockset->key->fd, psockset->key->connect_time, psockset->peer_ip, psockset->peer_port, psockset->type, buflen))
	{
		LOG(_ERROR_, "CEpollThread::createConnectServerMsg() error, NET_DATA init() failed, fd=%d, conn_time=%u, peer_ip=%s, port=%d",
			psockset->key->fd, psockset->key->connect_time, psockset->peer_ip, psockset->peer_port);
		delete pdata;
		pdata = NULL;
		return false;
	}

	memcpy(pdata->pdata, buf, buflen);
	pdata->data_len = buflen;
	m_recvlist.push_back(pdata);
	return true;
}

#endif//OS_LINUX

