#include "eupulogger4system.h"
#include "globalmgr.h"
#include "globalconfig.h"
#include "sprotocol.h"

CGlobalMgr* CGlobalMgr::m_pInstance = NULL;

CGlobalMgr* CGlobalMgr::getInstance()
{
	if (m_pInstance == NULL)
		m_pInstance = new CGlobalMgr;
	return m_pInstance;
}

void CGlobalMgr::release()
{
	if (m_pInstance != NULL)
		delete m_pInstance;
	m_pInstance = NULL;
}

bool CGlobalMgr::createMsgToSendList(int fd, time_t conntime, const string& ip, USHORT port, int ntype, USHORT mainid, USHORT assistantid, BYTE code, BYTE reserve, CEupuStream* pstream, UINT nlen)
{
	if (fd < 0)
	{
		LOG(_ERROR_, "CGlobalMgr::createMsgToSendList() error, param fd < 0");
		return false;
	}

	if (pstream == NULL)
	{
		nlen = 0;
	}

	NET_DATA* pdata = new NET_DATA;
	if (pdata == NULL)
	{
		LOG(_ERROR_, "CGlobalMgr::createMsgToSendList() error, new NET_DATA failed");
		exit(-1);
	}

	if (!pdata->init(fd, conntime, ip, port, ntype, nlen + NET_HEAD_SIZE))
	{
		LOG(_ERROR_, "CGlobalMgr::createMsgToSendList() error, NET_DATA init() failed");
		delete pdata;
		pdata = NULL;
		return false;
	}

	UINT buflen = pdata->data_len - NET_HEAD_SIZE;

	if (pstream)
	{
		if (!pstream->Out((BYTE*)(pdata->pdata + NET_HEAD_SIZE), buflen))
		{
			LOG(_ERROR_, "CGlobalMgr::createMsgToSendList() error, param stream->Out() failed");
			delete pdata;
			pdata = NULL;
			return false;
		}
		pdata->data_len = buflen + NET_HEAD_SIZE;
	}

	if (pdata->data_len > MAX_SEND_SIZE)
	{
		LOG(_ERROR_, "CGlobalMgr::createMsgToSendList() error, data size > MAX_SEND_SIZE, fd=%d, peer_ip=%s, port=%d, msg_id=%d, msg_size=%d, MAX_SEND_SIZE=%d",
			fd, GETNULLSTR(ip), port, mainid, pdata->data_len, MAX_SEND_SIZE);
		delete pdata;
		pdata = NULL;
		return false;
	}

	NetMessageHead msgHead; 
	msgHead.uMessageSize = pdata->data_len;
	msgHead.uMainID = mainid;
	msgHead.bAssistantID = assistantid;
	msgHead.bHandleCode = code;
	msgHead.bReserve = reserve;

	buflen = NET_HEAD_SIZE;

	if (!msgHead.Out((BYTE*)(pdata->pdata), buflen))
	{
		LOG(_ERROR_, "CGlobalMgr::createMsgToSendList() error, NetMessageHead.Out() failed, fd=%d, peer_ip=%s, port=%d, msg_id=%d",
			fd, GETNULLSTR(ip), port, mainid);
		delete pdata;
		pdata = NULL;
		return false;
	}

	if (!addMsgToSendList(pdata))
	{
		LOG(_ERROR_, "CGlobalMgr::createMsgToSendList() error, addMsgToSendList(pdata) failed, fd=%d, peer_ip=%s, port=%d, msg_id=%d",
			fd, GETNULLSTR(ip), port, mainid);
		delete pdata;
		pdata = NULL;
		return false;
	}
	return true;
}

bool CGlobalMgr::addMsgToSendList(NET_DATA* pmsg)
{
	if (pmsg == NULL || m_pcursendmap == NULL)
	{
		LOG(_ERROR_, "CGlobalMgr::addMsgToSendList() error, param NET_DATA == NULL || m_pcursendmap == NULL");
		return false;
	}

	m_sendmaplock.Lock();
	int fd = pmsg->fd;

	map<int, list<NET_DATA*>*>::iterator iter = m_pcursendmap->find(fd);
	if (iter == m_pcursendmap->end())
	{
		list<NET_DATA*>* plst = new list<NET_DATA*>;
		if (!plst)
		{
			LOG(_ERROR_, "CGlobalMgr::addMsgToSendList() error, net list<NET_DATA*> failed");
			exit(-1);
		}
		plst->push_back(pmsg);
		m_pcursendmap->insert(map<int, list<NET_DATA*>*>::value_type(fd, plst));
	}
	else
	{
		list<NET_DATA*>* plst = iter->second;
		if (!plst)
		{
			LOG(_ERROR_, "CGlobalMgr::addMsgToSendList() error, found NET_DATA list is NULL, fd=%d", fd);
			return false;
		}
		plst->push_back(pmsg);
		m_pcursendmap->insert(map<int, list<NET_DATA*>*>::value_type(fd, plst));
	}
	m_sendmaplock.UnLock();

	return true;
}

bool CGlobalMgr::init()
{
	LOG(_INFO_, "CGlobalMgr::init() start");
	m_nMaxSendList = CGlobalConfig::getInstance()->getSendQueueSize();
	m_recvlist.setQueueSize(CGlobalConfig::getInstance()->getRecvQueueSize());

	return true;
}

void CGlobalMgr::clean()
{
	m_sendmaplock.Lock();
	map<int, list<NET_DATA * > * >::iterator iter = m_pcursendmap->begin();
	for (; iter != m_pcursendmap->end(); ++iter)
	{
		list<NET_DATA*>* plst = iter->second;
		if (plst)
		{
			for (list<NET_DATA*>::iterator iterdata = plst->begin(); iterdata != plst->end(); ++iterdata)
			{
				if (*iterdata)
				{
					delete *iterdata;
					*iterdata = NULL;
				}
			}
			plst->clear();
			delete plst;
			plst = NULL;
		}
	}
	m_pcursendmap->clear();

	iter = m_pbaksendmap->begin();
	for (; iter != m_pbaksendmap->end(); ++iter)
	{
		list<NET_DATA*>* plst = iter->second;
		if (plst)
		{
			for (list<NET_DATA*>::iterator iterdata = plst->begin(); iterdata != plst->end(); ++iterdata)
			{
				if (*iterdata)
				{
					delete *iterdata;
					*iterdata = NULL;
				}
			}
			plst->clear();
			delete plst;
			plst = NULL;
		}
	}
	m_pbaksendmap->clear();

	m_sendmaplock.UnLock();

	m_recvlist.clearQueue();
	m_eventlist.clearQueue();
}

CSysQueue<NET_DATA>* CGlobalMgr::getRecvQueue()
{
	return &m_recvlist;
}

CSysQueue<NET_EVENT>* CGlobalMgr::getEventQueue()
{
	return &m_eventlist;
}

bool CGlobalMgr::sendMsgToServer(int ntype, USHORT mainid, USHORT assistantid, BYTE code, BYTE reserve, CEupuStream* stream, UINT nlen, bool blocked)
{
	if (blocked)
		m_sendmaplock.Lock();

	NET_DATA* pdata = NULL;

	switch (ntype)
	{
	case MAINSVR_TYPE:
		{
			pdata = &m_mainkey;
			break;
		}
	case DISSVR_TYPE:
		{
			pdata = &m_distributekey;
			break;
		}
	case USERCENTERSVR_TYPE:
		{
			pdata = &m_usercenterkey;
			break;
		}
	case LOGSVR_TYPE:
		{
			pdata = &m_logkey;
			break;
		}
	default:
		{
			if (blocked)
				m_sendmaplock.UnLock();
			return false;
		}
	}

	bool bret = false;

	if (pdata->fd > 0)
	{
		bret = createMsgToSendList(pdata->fd, pdata->connect_time, pdata->peer_ip, pdata->peer_port, ntype, mainid, assistantid, code, reserve, stream, nlen);
	}

	if (blocked)
		m_sendmaplock.UnLock();

	return bret;
}

void CGlobalMgr::setServerSocket(int fd, time_t conntime, const string& ip, USHORT port, int ntype)
{
	m_serverlock.Lock();

	switch (ntype)
	{
	case MAINSVR_TYPE:
		{
			setMainSocket(fd, conntime, ip, port, ntype);
			break;
		}
	case DISSVR_TYPE:
		{
			setDistributeSocket(fd, conntime, ip, port, ntype);
			break;
		}
	case USERCENTERSVR_TYPE:
		{
			setUserCenterSocket(fd, conntime, ip, port, ntype);
			break;
		}
	case LOGSVR_TYPE:
		{
			setLogSocket(fd, conntime, ip, port, ntype);
			break;
		}
	default:
		{
			LOG(_DEBUG_, "CGlobalMgr::setServerSocket(), invalid server type, fd=%d, time=%u, ip=%s, port=%d, type=%d",
				fd, conntime, GETNULLSTR(ip), port, ntype);
			break;
		}
	}

	m_serverlock.UnLock();
}

void CGlobalMgr::setMainSocket(int fd, time_t conntime, const string& peerip, USHORT peerport, int ntype)
{
}

void CGlobalMgr::setDistributeSocket(int fd, time_t conntime, const string& peerip, USHORT peerport, int ntype)
{
}

void CGlobalMgr::setUserCenterSocket(int fd, time_t conntime, const string& peerip, USHORT peerport, int ntype)
{
}

void CGlobalMgr::setLogSocket(int fd, time_t conntime, const string& peerip, USHORT peerport, int ntype)
{
}

void CGlobalMgr::switchSendMap()
{
	if (m_pcursendmap == NULL || m_pbaksendmap == NULL)
	{
		LOG(_ERROR_, "CGlobalMgr::switchSendMap() error, m_pcursendmap == NULL || m_pbaksendmap == NULL");
		return;
	}

	m_sendmaplock.Lock();

	map<int, list<NET_DATA*>*>::iterator iterbak = m_pbaksendmap->begin();
	for (; iterbak != m_pbaksendmap->end(); ++iterbak)
	{
		if (iterbak->second == NULL)
			continue;

		map<int, list<NET_DATA*>*>::iterator itercur = m_pcursendmap->find(iterbak->first);
		if (itercur != m_pcursendmap->end())
		{
			if (itercur->second == NULL)
			{
				itercur->second = iterbak->second;
				continue;
			}

			if (itercur->second->size() + iterbak->second->size() < m_nMaxSendList)
			{
				int needmove = m_nMaxSendList - itercur->second->size();
				int idx = 0;

				for (list<NET_DATA*>::iterator itersend = iterbak->second->begin(); itersend != iterbak->second->end(); ++itersend)
				{
					if (idx >= needmove)
					{
						if (*itersend)
						{
							delete *itersend;
							*itersend = NULL;
						}
					}
					else
					{
						itercur->second->push_back(*itersend);
					}
				}

				iterbak->second->clear();
				delete iterbak->second;
				iterbak->second = NULL;
				LOG(_ERROR_, "CGlobalMgr::switchSendMap(), send list full, delete %d message, current message size: %d", idx, itercur->second->size());
			}
			else
			{
				itercur->second->insert(itercur->second->begin(), iterbak->second->begin(), iterbak->second->end());
				iterbak->second->clear();
				delete iterbak->second;
				iterbak->second = NULL;
			}
		}
		else
		{
			m_pcursendmap->insert(map<int, list<NET_DATA*>*>::value_type(iterbak->first, iterbak->second));
		}
	}//end for

	m_pbaksendmap->clear();

	map<int, list<NET_DATA*>*>* p = m_pcursendmap;
	m_pcursendmap = m_pbaksendmap;
	m_pbaksendmap = p;


	m_sendmaplock.UnLock();

	return;
}

void CGlobalMgr::createServerConnect(int ntype)
{
	m_serverlock.Lock();

	PCONNECT_SERVER pserver = NULL;
	NET_DATA* pdata = NULL;

	switch (ntype)
	{
	case MAINSVR_TYPE:
		{
			pserver = CGlobalConfig::getInstance()->getMainServer();
			pdata = &m_mainkey;
			break;
		}
	case DISSVR_TYPE:
		{
			pserver = CGlobalConfig::getInstance()->getDistributeServer();
			pdata = &m_distributekey;
			break;
		}
	case USERCENTERSVR_TYPE:
		{
			pserver = CGlobalConfig::getInstance()->getUserCenterServer();
			pdata = &m_usercenterkey;
			break;
		}
	case LOGSVR_TYPE:
		{
			pserver = CGlobalConfig::getInstance()->getLogServer();
			pdata = &m_logkey;
			break;
		}
	default:
		{
			LOG(_ERROR_, "CGlobalMgr::createServerConnect() error, invalid server type, type=%d", ntype);
			break;
		}
	}//end switch

	if (pserver == NULL)
	{
		LOG(_ERROR_, "CGlobalMgr::createServerConnect() error, not found config option for server, type=%d", ntype);
		m_serverlock.UnLock();
		return;
	}

	if (pdata->fd >= 0)
	{
		m_serverlock.UnLock();
		return;
	}

	m_serverlock.UnLock();

	int fd = doNonblockConnect(pserver, 3, CGlobalConfig::getInstance()->getListenIp());

	if (fd < 0)
	{
		LOG(_ERROR_, "CGlobalMgr::createServerConnect() error, doNonblockConnect() failed");
		return;
	}

	time_t conntime = time(NULL);
	SOCKET_SET* psockset = initSocketset(fd, conntime, pserver->host, pserver->port, ntype);

	if (psockset == NULL)
	{
		LOG(_ERROR_, "CGlobalMgr::createServerConnect() error, initSocketset() failed");
#ifdef OS_LINUX
		close(fd);
#elif OS_WINDOWS
		closesocket(fd);
#endif
		exit(-1);
	}

	NET_EVENT* pevent = new NET_EVENT;
	if (pevent == NULL)
	{
		LOG(_ERROR_, "CGlobalMgr::createServerConnect() error, new NET_EVENT failed");
#ifdef OS_LINUX
		close(fd);
#elif OS_WINDOWS
		closesocket(fd);
#endif
		delete psockset;
		psockset = NULL;
		exit(-1);
	}

	setServerSocket(fd, conntime, pserver->host, pserver->port, ntype);

	pevent->eventid = ADD_CLIENT;
	pevent->data = (char*)psockset;

	if (!m_eventlist.inQueue(pevent, false))
	{
		LOG(_ERROR_, "CGlobalMgr::createServerConnect() error, EventQueue->inQueue() failed");
#ifdef OS_LINUX
		close(fd);
#elif OS_WINDOWS
		closesocket(fd);
#endif
		delete psockset;
		psockset = NULL;
		delete pevent;
		pevent = NULL;
	}
}

bool CGlobalMgr::createCloseConnectEvent(int fd, time_t conntime)
{
	SOCKET_KEY* pkey = new SOCKET_KEY;
	NET_EVENT* pevent = new NET_EVENT;

	if (pkey == NULL || pevent == NULL)
	{
		LOG(_ERROR_, "CGlobalMgr::createCloseConnectEvent() error, new SOCKET_KEY || new NET_EVENT failed");
		if (pkey)
			delete pkey;
		pkey = NULL;
		if (pevent)
			delete pevent;
		pevent = NULL;
		return false;
	}

	pkey->fd = fd;
	pkey->connect_time = conntime;

	pevent->eventid = CLOSE_CLIENT;
	pevent->data = (char*)pkey;

	if (!m_eventlist.inQueue(pevent, false))
	{
		LOG(_ERROR_, "CGlobalMgr::createCloseConnectEvent() error, m_eventlist.inQueue() failed");
		delete pkey;
		pkey = NULL;
		delete pevent;
		pevent = NULL;
		return false;
	}
	return true;
}

void CGlobalMgr::sendKeepaliveMsgToAllServer()
{
	m_serverlock.Lock();

	sendMsgToServer(MAINSVR_TYPE, KEEP_ALIVE_PING, 0, 0, 0, NULL, 0, false);
	sendMsgToServer(DISSVR_TYPE, KEEP_ALIVE_PING, 0, 0, 0, NULL, 0, false);
	sendMsgToServer(USERCENTERSVR_TYPE, KEEP_ALIVE_PING, 0, 0, 0, NULL, 0, false);
	sendMsgToServer(LOGSVR_TYPE, KEEP_ALIVE_PING, 0, 0, 0, NULL, 0, false);

	m_serverlock.UnLock();
}
