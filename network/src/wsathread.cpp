#ifdef OS_WINDOWS

#include "wsathread.h"
#include "common.h"
#include "eupulogger4system.h"
#include "globalmgr.h"
#include "sprotocol.h"
#include "sysqueue.h"
#include <functional>
#include <typeinfo>

CWSAThread::CWSAThread()
    : CEupuThread(), m_listenfd(INVALID_SOCKET), m_listenkey(NULL),
      m_keepalivetimeout(120), m_keepaliveinterval(60), m_serverport(0),
      m_readbufsize(0), m_sendbufsize(0), m_recvbuffer(NULL), m_recvbuflen(0),
      m_index(0)
//, m_maxwsaeventsize(0)
//, m_nEventTotal(0)
{
  // for (int i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
  //{
  //    m_eventArray[i] = WSA_INVALID_EVENT;
  //    m_sockArray[i] = INVALID_SOCKET;
  //}
  // memset(m_keymap, 0, sizeof(SOCKET_KEY*) * WSA_MAXIMUM_WAIT_EVENTS);

  WORD wVersionRequested;
  WSADATA wsaData;
  int err;
  wVersionRequested = MAKEWORD(1, 1);
  err = ::WSAStartup(wVersionRequested, &wsaData);
  if (err) {
    WSACleanup();
  }
  m_checkkeepalivetime = time(NULL);
  m_lastkeepalivetime = time(NULL);
}

CWSAThread::~CWSAThread() {
  // for (int i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
  //{
  //    if (m_eventArray[i] != WSA_INVALID_EVENT)
  //        ::WSACloseEvent(m_eventArray[i]);
  //    if (m_sockArray[i] != INVALID_SOCKET)
  //        ::closesocket(m_sockArray[i]);
  //}

  if (m_listenfd > 0) {
    ::closesocket(m_listenfd);
  }

  if (m_listenkey != NULL) {
    delete m_listenkey;
    m_listenkey = NULL;
  }

  std::unordered_map<int, SOCKET_SET *>::iterator itersockmap =
      m_socketmap.begin();
  for (; itersockmap != m_socketmap.end(); ++itersockmap) {
    if (itersockmap->second) {
      delete itersockmap->second;
      itersockmap->second = NULL;
    }
  }
  m_socketmap.clear();

  list<NET_DATA *>::iterator iterdatalist = m_recvtmplst.begin();
  for (; iterdatalist != m_recvtmplst.end(); ++iterdatalist) {
    if ((*iterdatalist) != NULL) {
      delete (*iterdatalist);
      (*iterdatalist) = NULL;
    }
  }
  m_recvtmplst.clear();

  if (m_recvbuffer) {
    delete[] m_recvbuffer;
    m_recvbuffer = NULL;
  }
  WSACleanup();
}

void CWSAThread::run() {
  pause();

  m_bIsExit = false;

  doWSAEvent();

  m_bIsExit = true;
}

int add1(int i, int j, int k) { return i + j + k; }

void CWSAThread::reset() {
  auto add2 = std::bind(add1, std::placeholders::_1, std::placeholders::_2, 10);
}

bool CWSAThread::startup() {
  // m_maxwsaeventsize = WSA_MAXIMUM_WAIT_EVENTS;
  m_keepalivetimeout = CGlobalConfig::getInstance()->getKeepaliveTimer();
  m_keepaliveinterval = m_keepalivetimeout / 2;
  m_serverip = CGlobalConfig::getInstance()->getListenIp();
  m_serverport = CGlobalConfig::getInstance()->getListenPort();

  m_sendbufsize = CGlobalConfig::getInstance()->getSocketSendBuf();
  m_readbufsize = CGlobalConfig::getInstance()->getSocketRecvBuf();

  m_recvbuflen = m_sendbufsize;
  if (m_sendbufsize < m_readbufsize) {
    m_recvbuflen = m_readbufsize;
  }
  if (m_recvbuflen < MAX_SEND_SIZE) {
    m_recvbuflen = MAX_SEND_SIZE;
  }
  m_recvbuflen += MAX_SEND_SIZE;

  m_recvbuffer = new char[m_recvbuflen];
  if (!m_recvbuffer) {
    LOG(_ERROR_, "CWSAThread::startup() error, _new char[m_recvbuflen] failed, "
                 "m_recvbuflen=%d",
        m_recvbuflen);
    ::exit(-1);
  }

  if (!doListen()) {
    LOG(_ERROR_, "CWSAThread::startup() error, doListen() failed");
    return false;
  }

  if (!start()) {
    LOG(_ERROR_, "CWSAThread::startup() error, start() failed");
    return false;
  }

  continues();

  return true;
}

time_t CWSAThread::getIndex() {
  m_index++;
  if (m_index == 0xFFFFFFFF)
    m_index = 1;
  return m_index;
}

void CWSAThread::doWSAEvent() {
  SOCKET_KEY *pkey = NULL;

  while (m_bOperate) {

    for (std::unordered_map<int, SOCKET_SET *>::iterator itersockmap =
             m_socketmap.begin();
         itersockmap != m_socketmap.end(); ++itersockmap) {
      if (itersockmap->second == NULL || itersockmap->second->key == NULL) {
        // LOG HERE
        continue;
      }
      if (m_listenfd == itersockmap->first) {
        if (!doAccept(m_listenfd)) {
          LOG(_ERROR_, "CWSAThread::doWSAEvent() error, m_listenfd=%d",
              m_listenfd);
        }
        continue;
      } else {
        doRecvMessage(itersockmap->second->key);
        if (doSendMessage(pkey) < 0) {
          closeClient(pkey->fd, pkey->connect_time);
        }
      }
    }

    /*
                    //TODO check here, WSA_MAXIMUM_WAIT_EVENTS
                    int nIndex = ::WSAWaitForMultipleEvents(m_nEventTotal,
    m_eventArray, FALSE, 10, FALSE);
                    nIndex = nIndex - WSA_WAIT_EVENT_0;
                    for (int i = nIndex; i < m_nEventTotal; ++i)
                    {
                            if (m_eventArray[i] == WSA_INVALID_EVENT)
                            {
                                    continue;
                            }

                            int nret = ::WSAWaitForMultipleEvents(1,
    &m_eventArray[i], TRUE, 10, FALSE);

                            if (nret == WSA_WAIT_FAILED || nret ==
    WSA_WAIT_TIMEOUT)
                            {
                                    if (nret == WSA_WAIT_FAILED)
                                    LOG(_ERROR_, "CWSAThread::doWSAEvent()
    error, (nIndex == WSA_WAIT_FAILED)");
                                    continue;
                            }
                            else
                            {
                                    WSANETWORKEVENTS event;
                                    if (::WSAEnumNetworkEvents(m_sockArray[i],
    m_eventArray[i], &event) == SOCKET_ERROR)
                                    {
                                            LOG(_ERROR_,
    "CWSAThread::doWSAEvent() error, ::WSAEnumNetworkEvents() failed, error=%s",
    WSAGetLastError());
                                            continue;
                                    }

                                    SOCKET_KEY* pkey = m_keymap[i];
                                    if (pkey == NULL)
                                    {
                                            LOG(_ERROR_,
    "CWSAThread::doWSAEvent() error, pkey == NULL, SOCKET_KEY* pkey =
    m_keymap[%d]", i);
                                            continue;
                                    }

                                    if (event.lNetworkEvents & FD_ACCEPT)
                                    {
                                            LOG(_INFO_,
    "CWSAThread::doWSAEvent(), FD_ACCEPT, m_listenfd=%d, m_sockArray[i]=%d",
    m_listenfd, m_sockArray[i]);
                                            if (event.iErrorCode[FD_ACCEPT_BIT]
    == 0)
                                            {
                                                    if (m_nEventTotal >
    WSA_MAXIMUM_WAIT_EVENTS)
                                                    {
                                                            LOG(_INFO_,
    "CWSAThread::doWSAEvent() error, m_nEventTotal > WSA_MAXIMUM_WAIT_EVENTS");
                                                            continue;
                                                    }

                                                    if (m_listenfd ==
    m_sockArray[i] && m_listenfd == pkey->fd)
                                                    {
                                                            if
    (!doAccept(m_listenfd))
                                                            {
                                                                    LOG(_ERROR_,
    "CWSAThread::doWSAEvent() error, m_listenfd=%d", m_listenfd);
                                                            }
                                                    }
                                            }
                                    }
                                    else if (event.lNetworkEvents & FD_READ)
                                    {
                                            LOG(_INFO_,
    "CWSAThread::doWSAEvent(), FD_READ, m_listenfd=%d, m_sockArray[i]=%d",
    m_listenfd, m_sockArray[i]);
                                            if (event.iErrorCode[FD_ACCEPT_BIT]
    == 0)
                                            {
                                                    if (m_nEventTotal >
    WSA_MAXIMUM_WAIT_EVENTS)
                                                    {
                                                            LOG(_INFO_,
    "CWSAThread::doWSAEvent() error, m_nEventTotal > WSA_MAXIMUM_WAIT_EVENTS");
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
            else if (event.lNetworkEvents & FD_WRITE)
            {
            LOG(_INFO_, "CWSAThread::doWSAEvent(), FD_WRITE, m_listenfd=%d,
    m_sockArray[i]=%d", m_listenfd, m_sockArray[i]);
            if (doSendMessage(pkey) < 0)
            {
            closeClient(pkey->fd, pkey->connect_time);
            }
            else
            {
            //TODO reset socket event here ?
            }
            }
            else
            {
            LOG(_INFO_, "CWSAThread::doWSAEvent(), FD_XXX, m_listenfd=%d,
    m_sockArray[i]=%d", m_listenfd, m_sockArray[i]);
            closeClient(pkey->fd, pkey->connect_time);
            }
            }
            }//end for
            //// end handle WSAWaitForMultipleEvents /////
            */

    /////////////////begin copy all recv message to recv list/////////////////
    if (m_recvtmplst.size() > 0) {
      SysQueue<NET_DATA> *precvlist = CGlobalMgr::getInstance()->getRecvQueue();
      precvlist->Lock();
      list<NET_DATA *>::iterator iter = m_recvtmplst.begin();
      for (; iter != m_recvtmplst.end(); ++iter) {
        if ((*iter) == NULL)
          continue;
        if (!precvlist->inQueueWithoutLock(*iter, false)) {
          LOG(_ERROR_,
              "CWSAThread::doWSAEvent() error, inQueueWithoutLock() failed");
          delete (*iter);
        }
      }
      precvlist->UnLock();
      m_recvtmplst.clear();
    }
    /////////////////end copy all recv message to recv list/////////////////

    //////////////////begin handle system events////////////////////////////
    doSystemEvent();
    //////////////////end handle system events//////////////////////////////////

    //////////////////begin set write wsa event by sendset////////////////
    CGlobalMgr::getInstance()->switchSendMap();
    map<int, list<NET_DATA *> *> *psendmap =
        CGlobalMgr::getInstance()->getBakSendMap();

    for (map<int, list<NET_DATA *> *>::iterator itersendmap = psendmap->begin();
         itersendmap != psendmap->end(); ++itersendmap) {
      std::unordered_map<int, SOCKET_SET *>::iterator itersockmap =
          m_socketmap.find(itersendmap->first);
      if (itersockmap == m_socketmap.end() || itersockmap->second == NULL ||
          itersockmap->second->key == NULL) {
        LOG(_ERROR_, "CWSAThread::doWSAEvent() error, m_socketmap.find(fd=%d) "
                     "failed, itersockmap == m_socketmap.end() %s, "
                     "itersockmap->second == NULL %s, itersockmap->second->key "
                     "== NULL %s",
            itersendmap->first,
            itersockmap == m_socketmap.end() ? "true" : "false",
            itersockmap->second == NULL ? "true" : "false",
            itersockmap->second->key == NULL ? "true" : "false");
        m_delsendfdlist.push_back(itersendmap->first);
        continue;
      }

      // int i = 0;
      // for (; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
      //{
      //    if (itersendmap->first == m_sockArray[i])
      //        break;
      //}
      // if (i > WSA_MAXIMUM_WAIT_EVENTS)
      //{
      //    LOG(_ERROR_, "CEpollThread::doEpollEvent() error, m_sockArray[i]
      //    can't find(fd) failed");
      //    m_delsendfdlist.push_back(itersendmap->first);
      //    continue;
      //}
      // if (::WSAEventSelect(m_sockArray[i], m_eventArray[i], FD_WRITE) ==
      // SOCKET_ERROR)
      //{
      //    LOG(_ERROR_, "CWSAThread::doEpollEvent() error, WSAEventSelect()
      //    failed, listen fd=%d, error=%ld", m_listenfd, WSAGetLastError());
      //    break;
      //}
    }

    for (list<int>::iterator iterdelsendfdlist = m_delsendfdlist.begin();
         iterdelsendfdlist != m_delsendfdlist.end(); ++iterdelsendfdlist) {
      deleteSendMsgFromSendMap(*iterdelsendfdlist);
    }
    m_delsendfdlist.clear();

    // doKeepaliveTimeout();

    doSendKeepaliveToServer();
  } // end while
}

bool CWSAThread::stop() {
  m_bOperate = false;
  while (!m_bIsExit) {
    Sleep(10);
  }
  return true;
}

void CWSAThread::doKeepaliveTimeout() {
  time_t curtime = time(NULL);
  if (curtime - m_checkkeepalivetime < m_keepaliveinterval) {
    return;
  }

  list<int> timelist;
  std::unordered_map<int, SOCKET_SET *>::iterator itersockmap =
      m_socketmap.begin();
  for (; itersockmap != m_socketmap.end(); ++itersockmap) {
    if (itersockmap->second != NULL) {
      // TODO check here, > or < ?
      if (itersockmap->second->refresh_time + m_keepalivetimeout < curtime) {
        timelist.push_back(itersockmap->first);
      }
    } else {
      timelist.push_back(itersockmap->first);
    }
  } // end for

  bool bclosed = false;
  for (list<int>::iterator itertimeout = timelist.begin();
       itertimeout != timelist.end(); ++itertimeout) {
    bclosed = false;
    itersockmap = m_socketmap.find(*itertimeout);
    if (itersockmap != m_socketmap.end()) {
      if (itersockmap->second != NULL) {
        if (itersockmap->second->key != NULL) {
          bclosed = true;
          // LOG(_ERROR_, "CWSAThread::doKeepaliveTimeout() error, close
          // socket_set for timeout, fd=%d, time=%u, peerip=%s, port=%d",
          //    itersocket->first, itersocket->second->key->connect_time,
          //    GETNULLSTR(itersocket->second->peer_ip),
          //    itersocket->second->peer_port);
          closeClient(itersockmap->first,
                      itersockmap->second->key->connect_time);
        }
      }

      if (!bclosed) {
        LOG(_ERROR_, "CWSAThread::doKeepaliveTimeout() error, close "
                     "socket_set->key->fd close for timeout");
        ::closesocket(itersockmap->first);
        delete itersockmap->second;
        m_socketmap.erase(itersockmap);
      }
    }
  } // end for
  timelist.clear();
  m_checkkeepalivetime = time(NULL);
}

void CWSAThread::doSendKeepaliveToServer() {
  time_t curtime = time(NULL);
  if (curtime - m_lastkeepalivetime < m_keepaliveinterval) {
    return;
  }

  CGlobalMgr::getInstance()->sendKeepaliveMsgToAllServer();

  m_lastkeepalivetime = time(NULL);
}

bool CWSAThread::doListen() {
  bool bret = false;
  sockaddr_in serv_addr;
  m_listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  do {
    if (m_listenfd == INVALID_SOCKET) {
      LOG(_ERROR_, "CWSAThread::doListen() error, create listen socket failed");
      break;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(m_serverport);
    if (m_serverip.empty()) {
      serv_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    } else {
      // serv_addr.sin_addr.S_un.S_addr = fgAtoN(m_serverip.c_str());
      serv_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    }

    int tmp = 1;
    if (setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&tmp,
                   sizeof(tmp)) == SOCKET_ERROR) {
      LOG(_ERROR_, "CWSAThread::doListen() error, setsockopt() failed, listen "
                   "fd=%d, error=%ld",
          m_listenfd, WSAGetLastError());
      break;
    }

    if (!setNonBlock(m_listenfd)) {
      LOG(_ERROR_,
          "CWSAThread::doListen() error, setNonBlock() failed, listen fd=%d",
          m_listenfd);
      break;
    }

    if (::bind(m_listenfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) ==
        SOCKET_ERROR) {
      LOG(_ERROR_, "CWSAThread::doListen() error, bind() failed, listen fd=%d, "
                   "error=%ld",
          m_listenfd, WSAGetLastError());
      break;
    }

    if (::listen(m_listenfd, 4096) == SOCKET_ERROR) {
      LOG(_ERROR_, "CWSAThread::doListen() error, listen() failed, listen "
                   "fd=%d, error=%ld",
          m_listenfd, WSAGetLastError());
      break;
    }

    m_listenkey = new SOCKET_KEY;
    if (!m_listenkey) {
      LOG(_ERROR_,
          "CWSAThread::doListen() error, _new SOCKET_KEY failed, listen fd=%d",
          m_listenfd);
      ::closesocket(m_listenfd);
      m_listenfd = INVALID_SOCKET;
      ::exit(-1);
    }

    m_listenkey->fd = m_listenfd;
    m_listenkey->connect_time = getIndex();

    // WSAEVENT event = ::WSACreateEvent();
    // if (event == WSA_INVALID_EVENT)
    //{
    //    LOG(_ERROR_, "CWSAThread::doListen() error, _new SOCKET_KEY failed,
    //    listen fd=%d", m_listenfd);
    //    break;
    //}

    // TODO check here, if we need set FD_CONNECT, FD_CLOSE event
    // if (::WSAEventSelect(m_listenfd, event, FD_ACCEPT | FD_READ | FD_CONNECT
    // | FD_CLOSE) == SOCKET_ERROR)
    //{
    //    LOG(_ERROR_, "CWSAThread::doListen() error, WSAEventSelect() failed,
    //    listen fd=%d, error=%ld", m_listenfd, WSAGetLastError());
    //    break;
    //}

    // m_eventArray[m_nEventTotal] = event;
    // m_sockArray[m_nEventTotal] = m_listenfd;
    // m_keymap[m_nEventTotal] = m_listenkey;
    // m_nEventTotal++;

    // LOG(_INFO_, "CWSAThread::doListen() successed, listen fd=%d,
    // m_nEventTotal=%d", m_listenfd, m_nEventTotal);

    bret = true;

  } while (false);

  return bret;
}

bool CWSAThread::doAccept(int fd) {
  struct sockaddr_in addr = {0};
  socklen_t addrlen = sizeof(addr);
  int connfd = INVALID_SOCKET;

  while (true) {
    addrlen = sizeof(addr);
    connfd = accept(fd, (sockaddr *)&addr, &addrlen);
    if (connfd == INVALID_SOCKET) {
      if (WSAGetLastError() != WSAEWOULDBLOCK) {
        LOG(_ERROR_, "CWSAThread::doAccept() error, fd=%d, error=%d", fd,
            WSAGetLastError());
      }
      return true;
    }

    string peerip = fgNtoA(ntohl(addr.sin_addr.S_un.S_addr));
    unsigned short port = ntohs(addr.sin_port);
    LOG(_INFO_, "CWSAThread::doAccept(), peerip=%s, port=%d",
        GETNULLSTR(peerip), port);

    send(connfd, NULL, 0, 0);

    if (!setNonBlock(connfd)) {
      LOG(_ERROR_, "CWSAThread::doAccept() error, fd=%d, connfd=%d", fd,
          connfd);
      ::closesocket(connfd);
      continue;
    }

    if (setsockopt(connfd, SOL_SOCKET, SO_SNDBUF, (char *)&m_sendbufsize,
                   sizeof(m_sendbufsize)) == SOCKET_ERROR) {
      LOG(_ERROR_, "CWSAThread::doAccept() error, setsockopt(SO_SNDBUF=%d) "
                   "failed, fd=%d, connfd=%d, error=%d",
          m_sendbufsize, fd, connfd, WSAGetLastError());
      ::closesocket(connfd);
      continue;
    }

    if (setsockopt(connfd, SOL_SOCKET, SO_RCVBUF, (char *)&m_readbufsize,
                   sizeof(m_readbufsize)) == SOCKET_ERROR) {
      LOG(_ERROR_, "CWSAThread::doAccept() error, setsockopt(SO_RCVBUF=%d) "
                   "failed, fd=%d, connfd=%d, error=%d",
          m_readbufsize, fd, connfd, WSAGetLastError());
      ::closesocket(connfd);
      continue;
    }

    int opt = 1;
    if (setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, (char *)&opt,
                   sizeof(opt)) < 0) {
      LOG(_ERROR_, "CWSAThread::doAccept() error, setsockopt(TCP_NODELAY) "
                   "failed, fd=%d, connfd=%d, error=%d",
          fd, connfd, WSAGetLastError());
      ::closesocket(connfd);
      continue;
    }

    SOCKET_SET *psockset =
        initSocketset(connfd, getIndex(), peerip, port, CLIENT_TYPE);

    if (!psockset) {
      LOG(_ERROR_, "CWSAThread::doAccept() error, initSocketset() failed, "
                   "fd=%d, connfd=%d, peerip=%s, port=%d",
          fd, connfd, GETNULLSTR(peerip), port);
      ::closesocket(connfd);
      continue;
    }

    if (!createConnectServerMsg(psockset)) {
      LOG(_ERROR_, "CWSAThread::doAccept() error, initSocketset() failed, "
                   "fd=%d, connfd=%d, peerip=%s, port=%d",
          fd, connfd, GETNULLSTR(peerip), port);
      ::closesocket(connfd);
      delete psockset;
      continue;
    }

    if (!addClientToWSA(psockset)) {
      LOG(_ERROR_, "CWSAThread::doAccept() error, addClientToWSA() failed, "
                   "fd=%d, connfd=%d, peerip=%s, port=%d",
          fd, connfd, GETNULLSTR(peerip), port);
      delete psockset;
      ::closesocket(connfd);
      continue;
    }
  }

  return true;
}

bool CWSAThread::addClientToWSA(SOCKET_SET *psockset) {
  if (psockset == NULL || psockset->key == NULL) {
    return false;
  }

  // WSAEVENT newevent = ::WSACreateEvent();
  // if (newevent == WSA_INVALID_EVENT)
  //{
  //    LOG(_ERROR_, "CWSAThread::addClientToWSA() error, WSACreateEvent()
  //    failed, fd=%d", psockset->key->fd);
  //    ::closesocket(psockset->key->fd);
  //    return false;
  //}
  // if (::WSAEventSelect(psockset->key->fd, newevent, FD_READ | FD_WRITE |
  // FD_CLOSE) == SOCKET_ERROR)
  //{
  //    LOG(_ERROR_, "CWSAThread::addClientToWSA() error, WSAEventSelect()
  //    failed, fd=%d", psockset->key->fd);
  //    return false;
  //}

  // int i = 0;
  // for (; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
  //{
  //    if (m_sockArray[i] == INVALID_SOCKET)
  //        break;
  //}
  // m_eventArray[i] = newevent;
  // m_sockArray[i] = psockset->key->fd;
  // m_keymap[i] = psockset->key;
  // if ((i + 1) <= WSA_MAXIMUM_WAIT_EVENTS && m_eventArray[i + 1] ==
  // WSA_INVALID_EVENT)
  //{
  //    m_nEventTotal++;
  //}

  if (!addSocketToMap(psockset)) {
    LOG(_ERROR_, "CWSAThread::addClientToWSA() error, addSocketToMap() failed, "
                 "fd=%d, peerip=%s, port=%d",
        psockset->key->fd, GETNULLSTR(psockset->peer_ip), psockset->peer_port);
    return false;
  }

  return true;
}

void CWSAThread::doRecvMessage(SOCKET_KEY *pkey) {
  if (pkey == NULL) {
    return;
  }

  int buflen = 0;
  int nret = 0;

  std::unordered_map<int, SOCKET_SET *>::iterator itersockmap =
      m_socketmap.find(pkey->fd);
  if (itersockmap == m_socketmap.end() || itersockmap->second == NULL ||
      itersockmap->second->key == NULL) {
    LOG(_ERROR_,
        "CWSAThread::doRecvMessage() error, can't find socket in map, fd=%d",
        pkey->fd);
    closeClient(pkey->fd, pkey->connect_time);
    // TODO check here, will return or not
    // return;
  }

  if (itersockmap->second->key != pkey) {
    LOG(_ERROR_, "CWSAThread::doRecvMessage() error, the found socket dones't "
                 "match pkey, fd=%d",
        pkey->fd);
    closeClient(pkey->fd, pkey->connect_time);
    // TODO check here, will return or not
    // return;
  }

  SOCKET_SET *psockset = itersockmap->second;

  while (1) {
    buflen = m_recvbuflen;
    memset(m_recvbuffer, 0, buflen);

    buflen -= psockset->part_len;
    nret = recv_msg(pkey->fd, m_recvbuffer + psockset->part_len, buflen);
    if (nret < 0) {
      LOG(_ERROR_, "CWSAThread::doRecvMessage() error, recv_msg() failed, "
                   "fd=%d, time=%u, peerip=%s, port=%d",
          psockset->key->fd, psockset->key->connect_time,
          GETNULLSTR(psockset->peer_ip), psockset->peer_port);
      return;
    }

    psockset->refresh_time = time(NULL);
    bool bparse = true;
    if (buflen > 0) {
      memcpy(m_recvbuffer, psockset->part_buf, psockset->part_len);
      buflen += psockset->part_len;
      psockset->part_len = 0;
      bparse = parsePacketToRecvQueue(psockset, m_recvbuffer, buflen);
      if (!bparse) {
        LOG(_ERROR_, "CWSAThread::doRecvMessage() error, "
                     "parsePacketToRecvQueue() failed, fd=%d, time=%u, "
                     "peerip=%s, port=%d",
            psockset->key->fd, psockset->key->connect_time,
            GETNULLSTR(psockset->peer_ip), psockset->peer_port);
      }
    }

    if (nret == 0) {
      LOG(_ERROR_, "CWSAThread::doRecvMessage() error, recv_msg() return 0, "
                   "fd=%d, time=%u, peerip=%s, port=%d",
          psockset->key->fd, psockset->key->connect_time,
          GETNULLSTR(psockset->peer_ip), psockset->peer_port);
    }

    if (!bparse || nret == 0) {
      closeClient(pkey->fd, pkey->connect_time);
      return;
    }

    if (nret == 1) {
      break; // EAGAIN
    }
  }
}

int CWSAThread::doSendMessage(SOCKET_KEY *pkey) {
  if (pkey == NULL)
    return 0;

  std::unordered_map<int, SOCKET_SET *>::iterator itersockmap =
      m_socketmap.find(pkey->fd);
  if (itersockmap == m_socketmap.end() || itersockmap->second == NULL ||
      itersockmap->second->key == NULL) {
    LOG(_ERROR_, "CWSAThread::doSendMessage() error, do not find socket in "
                 "m_socketmap, fd=%d",
        pkey->fd);
    deleteSendMsgFromSendMap(pkey->fd);
    return 0;
  }

  int num = 0;
  int buflen = 0;

  map<int, list<NET_DATA *> *> *psendmap =
      CGlobalMgr::getInstance()->getBakSendMap();
  map<int, list<NET_DATA *> *>::iterator itersendmap = psendmap->find(pkey->fd);
  if (itersendmap == psendmap->end()) {
    LOG(_WARN_, "CWSAThread::doSendMessage(), do not find data in m_sendmap, "
                "fd=%d, mapsize=%d",
        pkey->fd, psendmap->size());
    return 0;
  }

  if (itersendmap->second == NULL) {
    LOG(_DEBUG_,
        "CWSAThread::doSendMessage() error, data in m_sendmap is NULL, fd=%d",
        pkey->fd);
    psendmap->erase(itersendmap);
    return 0;
  }

  while (itersendmap->second->size() > 0) {
    NET_DATA *pdata = itersendmap->second->front();
    if (pdata == NULL) {
      itersendmap->second->pop_front();
      continue;
    }

    if (!((itersockmap->second->key->fd == pdata->fd) &&
          (itersockmap->second->key->connect_time == pdata->connect_time))) {
      LOG(_ERROR_, "CWSAThread::doSendMessage() error, data and socket don't "
                   "match in fd and connect_time, data fd=%d",
          pdata->fd);
      LOGHEX(_ERROR_, "the send message:", pdata->pdata, pdata->data_len);
      delete pdata;
      itersendmap->second->pop_front();
      continue;
    }

    buflen = pdata->data_len;
    int nsend = send_msg(pdata->fd, pdata->pdata, buflen);
    if (nsend < 0) {
      LOG(_ERROR_,
          "CWSAThread::doSendMessage() error, send_msg() failed, fd=%d",
          pdata->fd);
      LOGHEX(_DEBUG_, "send message failed:", pdata->pdata, pdata->data_len);
      deleteSendMsgFromSendMap(pkey->fd);
      return -1;
    } else if (nsend == 0) {
      // TODO, check here
      num = pdata->data_len - buflen;
      if (num > 0) {
        LOG(_INFO_, "CWSAThread::doSendMessage() error, send_msg() send part "
                    "of data, fd=%d, len=%d",
            pdata->fd, buflen);
        LOGHEX(_DEBUG_, "send part message:", pdata->pdata, buflen);
        pdata->data_len = num;
        memmove(pdata->pdata, pdata->pdata + buflen, num);
      } else {
        LOG(_INFO_, "CWSAThread::doSendMessage() successed, fd=%d, time=%u, "
                    "peerip=%s, port=%d",
            pdata->fd, pdata->connect_time, GETNULLSTR(pdata->peer_ip),
            pdata->peer_port);
        LOGHEX(_DEBUG_, "send message:", pdata->pdata, buflen);
        delete pdata;
        itersendmap->second->pop_front();
      }
      break;
    } else {
      LOG(_INFO_, "CWSAThread::doSendMessage() successed, fd=%d, time=%u, "
                  "peerip=%s, port=%d",
          pdata->fd, pdata->connect_time, GETNULLSTR(pdata->peer_ip),
          pdata->peer_port);
      LOGHEX(_DEBUG_, "send message:", pdata->pdata, buflen);
      delete pdata;
      itersendmap->second->pop_front();
      continue;
    }
  } // end while

  if (itersendmap->second->size() <= 0) {
    delete itersendmap->second;
    itersendmap->second = NULL;
    psendmap->erase(itersendmap);
  }

  return 1;
}

bool CWSAThread::parsePacketToRecvQueue(SOCKET_SET *psockset, char *buf,
                                        int buflen) {
  if (psockset == NULL || psockset->key == NULL) {
    LOG(_ERROR_, "CWSAThread::parsePacketToRecvQueue() error, psockset == NULL "
                 "|| psockset->key == NULL");
    return false;
  }

  if (buf == NULL || buflen <= 0) {
    LOG(_ERROR_, "CWSAThread::parsePacketToRecvQueue() error, buf == NULL || "
                 "buflen <= 0");
    // TODO check here, return true or false ?
    return true;
  }

  LOGHEX(_DEBUG_, "recv message:", buf, buflen);

  int curpos = 0;

  while (curpos < buflen) {
    if (buflen - curpos < NET_HEAD_SIZE) {
      memcpy(psockset->part_buf, buf, buflen - curpos);
      psockset->part_len = buflen - curpos;
      return true;
    }

    unsigned short nmsgsize = *((unsigned short *)(buf + curpos));
    unsigned short nlen = ntohs(nmsgsize);

    if (nlen > MAX_SEND_SIZE || nlen < NET_HEAD_SIZE) {
      LOG(_ERROR_, "CWSAThread::parsePacketToRecvQueue() error, invalid "
                   "message size, fd=%d, time=%u, peerip=%s, port=%d",
          psockset->key->fd, psockset->key->connect_time,
          GETNULLSTR(psockset->peer_ip), psockset->peer_port);
      return false;
    }

    if (nlen > buflen - curpos) {
      // TODO check here, buf or buf + curpos ?
      memcpy(psockset->part_buf, buf + curpos, buflen - curpos);
      psockset->part_len = buflen - curpos;
      return true;
    }

    NET_DATA *pdata = new NET_DATA;
    if (pdata == NULL) {
      LOG(_ERROR_, "CWSAThread::parsePacketToRecvQueue() error, _new NET_DATA "
                   "failed, fd=%d, time=%u, peerip=%s, port=%d",
          psockset->key->fd, psockset->key->connect_time,
          GETNULLSTR(psockset->peer_ip), psockset->peer_port);
      // TODO check here, buf or buf + curpos
      LOGHEX(_DEBUG_, "recv message:", buf + curpos, buflen);
      ::exit(-1);
    }

    if (!pdata->init(psockset->key->fd, psockset->key->connect_time,
                     psockset->peer_ip, psockset->peer_port, psockset->type,
                     nlen)) {
      LOG(_ERROR_, "CWSAThread::parsePacketToRecvQueue() error, pdata->init() "
                   "failed, fd=%d, time%u, peerip=%s, port=%d",
          psockset->key->fd, psockset->key->connect_time,
          GETNULLSTR(psockset->peer_ip), psockset->peer_port);
      LOGHEX(_DEBUG_, "recv message:", buf, buflen);
      delete pdata;
      return false;
    }

    memcpy(pdata->pdata, buf + curpos, nlen);
    pdata->data_len = nlen;
    m_recvtmplst.push_back(pdata);
    curpos += nlen;
  } // end while

  return true;
}

void CWSAThread::closeClient(int fd, time_t conn_time) {
  if (fd < 0) {
    LOG(_ERROR_, "CWSAThread::closeClient() error, fd < 0");
    return;
  }

  // int i = WSA_WAIT_EVENT_0;
  // for (; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
  //{
  //    if (fd == m_sockArray[i])
  //        break;
  //}

  // TODO, check here, we needn't do m_nEventTotal--
  // if (WSAEventSelect(fd, m_eventArray[i], 0) == SOCKET_ERROR)
  //{
  //    LOG(_ERROR_, "CWSAThread::closeClient() error, WSAEventSelect(0) failed,
  //    fd=%d", fd);
  //}
  // if (m_keymap[i])
  //{
  //    delete m_keymap[i];
  //    m_keymap[i] = NULL;
  //}
  // m_eventArray[i] = WSA_INVALID_EVENT;
  // m_sockArray[i] = INVALID_SOCKET;

  std::unordered_map<int, SOCKET_SET *>::iterator itersockmap =
      m_socketmap.find(fd);
  ::closesocket(fd);

  if (itersockmap == m_socketmap.end()) {
    LOG(_ERROR_, "CWSAThread::closeClient() error, can not find socket in "
                 "m_socketmap, fd=%d",
        fd);
  } else {
    if (itersockmap->second != NULL) {
      if (itersockmap->second->type > CLIENT_TYPE) {
        CGlobalMgr::getInstance()->setServerSocket(-1, 0, "", 0,
                                                   itersockmap->second->type);
      }
      createClientCloseMsg(itersockmap->second);
      delete itersockmap->second;
      itersockmap->second = NULL;
    } else {
      LOG(_ERROR_,
          "CWSAThread::closeClient() error, the found socket is NULL, fd=%d",
          fd);
    }
    m_socketmap.erase(itersockmap);
  }
}

void CWSAThread::createClientCloseMsg(SOCKET_SET *psockset) {
  if (psockset == NULL || psockset->key == NULL) {
    LOG(_ERROR_, "CWSAThread::createClientCloseMsg() error, psockset == NULL "
                 "|| psockset->key == NULL");
    return;
  }

  char buf[MAX_SEND_SIZE];
  UINT buflen = sizeof(buf);
  memset(buf, 0, buflen);

  MP_Server_DisConnected msg;
  msg.m_nServer = psockset->type;
  if (!msg.Out((BYTE *)buf, buflen)) {
    LOG(_ERROR_, "CWSAThread::createClientCloseMsg() error, msg.Out() failed, "
                 "fd=%d, time=%u, peerip=%s, port=%d",
        psockset->key->fd, psockset->key->connect_time,
        GETNULLSTR(psockset->peer_ip), psockset->peer_port);
    msg.Debug();
    return;
  }

  NET_DATA *pdata = new NET_DATA;
  if (pdata == NULL) {
    LOG(_ERROR_, "CWSAThread::createClientCloseMsg() error, _new NET_DATA "
                 "failed, fd=%d, time=%u, peerip=%s, port=%d",
        psockset->key->fd, psockset->key->connect_time,
        GETNULLSTR(psockset->peer_ip), psockset->peer_port);
    ::exit(-1);
  }

  if (!pdata->init(psockset->key->fd, psockset->key->connect_time,
                   psockset->peer_ip, psockset->peer_port, psockset->type,
                   buflen)) {
    LOG(_ERROR_, "CWSAThread::createClientCloseMsg() error, pdata->init() "
                 "failed, fd=%d, time=%u, peerip=%s, port=%d",
        psockset->key->fd, psockset->key->connect_time,
        GETNULLSTR(psockset->peer_ip), psockset->peer_port);
    delete pdata;
    return;
  }

  memcpy(pdata->pdata, buf, buflen);
  pdata->data_len = buflen;
  m_recvtmplst.push_back(pdata);
}

bool CWSAThread::addSocketToMap(SOCKET_SET *psockset) {
  if (psockset == NULL || psockset->key == NULL || psockset->key->fd < 0) {
    LOG(_ERROR_, "CWSAThread::addSocketToMap() error, psockset == NULL || "
                 "psockset->key == NULL || psockset->key->fd < 0");
    return false;
  }

  // if (m_maxwsaeventsize <= m_socketmap.size())
  //{
  //    LOG(_ERROR_, "CWSAThread::addSocketToMap() error, the wsa event handler
  //    is full, size=%d", m_maxwsaeventsize);
  //    return false;
  //}

  std::unordered_map<int, SOCKET_SET *>::iterator itersockmap =
      m_socketmap.find(psockset->key->fd);
  if (itersockmap != m_socketmap.end()) {
    if (itersockmap->second && itersockmap->second->key) {
      LOG(_WARN_, "CWSAThread::addSocketToMap() error, found timeout socket in "
                  "m_socketmap, fd=%d",
          psockset->key->fd);
    } else {
      LOG(_WARN_, "CWSAThread::addSocketToMap() error, found timeout socket in "
                  "m_socketmap, but sockset == NULL || key == NULL, fd=%d",
          psockset->key->fd);
    }

    if (itersockmap->second != NULL) {
      delete itersockmap->second;
      itersockmap->second = NULL;
    }
    m_socketmap.erase(itersockmap);
  }

  m_socketmap.insert(std::unordered_map<int, SOCKET_SET *>::value_type(
      psockset->key->fd, psockset));

  LOG(_INFO_, "CWSAThread::addSocketToMap() successed, fd=%d, time=%u, ip=%s, "
              "port=%d, type=%d",
      psockset->key->fd, psockset->key->connect_time, psockset->peer_ip.c_str(),
      psockset->peer_port, psockset->type);
  LOG(_INFO_, "CWSAThread::addSocketToMap() successed, m_socketmap size=%d",
      m_socketmap.size());

  return true;
}

void CWSAThread::deleteSendMsgFromSendMap(int fd) {
  if (fd < 0) {
    LOG(_ERROR_, "CWSAThread::deleteSendMsgFromSendMap() error, fd < 0");
    return;
  }

  map<int, list<NET_DATA *> *> *psendmap =
      CGlobalMgr::getInstance()->getBakSendMap();
  map<int, list<NET_DATA *> *>::iterator itersendmap = psendmap->find(fd);

  if (itersendmap != psendmap->end()) {
    if (itersendmap->second != NULL) {
      list<NET_DATA *>::iterator iterdata = itersendmap->second->begin();
      for (; iterdata != itersendmap->second->end(); ++iterdata) {
        delete *iterdata;
      }
      itersendmap->second->clear();
      delete itersendmap->second;
      itersendmap->second = NULL;
    }
    psendmap->erase(itersendmap);
  }
  return;
}

void CWSAThread::doSystemEvent() {
  SysQueue<NET_EVENT> *pevent = CGlobalMgr::getInstance()->getEventQueue();
  if (pevent == NULL) {
    return;
  }

  if (pevent->isEmptyWithoutLock()) {
    return;
  }

  pevent->Lock();
  NET_EVENT *pdata = NULL;
  while (pevent->outQueueWithoutLock(pdata, true)) {
    if (pdata == NULL) {
      continue;
    }

    switch (pdata->eventid) {
    case CLOSE_CLIENT: {
      if (pdata->data != NULL) {
        SOCKET_KEY *pkey = (SOCKET_KEY *)pdata->data;
        LOG(_INFO_, "CWSATread::doSystemEvent() handle CLOSE_CLIENT");
        closeClient(pkey->fd, pkey->connect_time);
        delete pkey;
      }
      break;
    }
    case ADD_CLIENT: {
      if (pdata->data == NULL) {
        LOG(_ERROR_, "CWSAThread::doSystemEvent() error, event data is NULL");
        break;
      }
      SOCKET_SET *psockset = (SOCKET_SET *)pdata->data;
      if (psockset->key == NULL) {
        LOG(_ERROR_,
            "CWSAThread::doSystemEvent() error, event data->key is NULL");
        delete psockset;
        break;
      }

      psockset->key->connect_time = getIndex();
      CGlobalMgr::getInstance()->setServerSocket(
          psockset->key->fd, psockset->key->connect_time, psockset->peer_ip,
          psockset->peer_port, psockset->type);
      LOG(_INFO_, "CWSAThread::doSystemEvent(), handle ADD_CLIENT event, "
                  "fd=%d, time=%u, peerip=%s, port=%d",
          psockset->key->fd, psockset->key->connect_time,
          GETNULLSTR(psockset->peer_ip), psockset->peer_port);

      if (!createConnectServerMsg(psockset)) {
        ::closesocket(psockset->key->fd);
        delete psockset;
        break;
      }

      if (!addClientToWSA(psockset)) {
        LOG(_ERROR_, "CWSATread::doSystemEvent() error, addClientToWSA() "
                     "failed, fd=%d, time=%u",
            psockset->key->fd, psockset->key->connect_time);
        ::closesocket(psockset->key->fd);
        delete psockset;
        break;
      }
      break;
    }
    default: {
      LOG(_ERROR_, "CWSAThread::doSystemEvent() error, invalid event, event=%d",
          pdata->eventid);
      if (pdata->data != NULL) {
        delete pdata->data;
      }
      break;
    }
    }
    // TODO check here, delete pdata or not ?
    delete pdata;
    pdata = NULL;
  } // end while
  pevent->UnLock();
}

bool CWSAThread::createConnectServerMsg(SOCKET_SET *psockset) {
  if (psockset == NULL || psockset->key == NULL) {
    LOG(_ERROR_,
        "CWSAThread::createConnectServerMsg() error, psockset == NULL");
    return false;
  }

  MP_Server_Connected msg;
  msg.m_nServer = psockset->type;

  char buf[MAX_SEND_SIZE];
  UINT buflen = sizeof(buf);
  memset(buf, 0, buflen);

  if (!msg.Out((BYTE *)buf, buflen)) {
    LOG(_ERROR_, "CWSAThread::createConnectServerMsg() error, msg.Out() "
                 "failed, fd=%d, time=%u, peerip=%s, port=%d",
        psockset->key->fd, psockset->key->connect_time,
        GETNULLSTR(psockset->peer_ip), psockset->peer_port);
    msg.Debug();
    return false;
  }

  NET_DATA *pdata = new NET_DATA;
  if (pdata == NULL) {
    LOG(_ERROR_, "CWSAThread::createConnectServerMsg() error, _new NET_DATA "
                 "failed, fd=%d, time=%u, peerip=%s, port=%d",
        psockset->key->fd, psockset->key->connect_time,
        GETNULLSTR(psockset->peer_ip), psockset->peer_port);
    ::exit(-1);
  }

  if (!pdata->init(psockset->key->fd, psockset->key->connect_time,
                   psockset->peer_ip, psockset->peer_port, psockset->type,
                   buflen)) {
    LOG(_ERROR_, "CEpollThread::createConnectServerMsg() error, NET_DATA "
                 "init() failed, fd=%d, conn_time=%u, peer_ip=%s, port=%d",
        psockset->key->fd, psockset->key->connect_time, psockset->peer_ip,
        psockset->peer_port);
    return false;
  }

  memcpy(pdata->pdata, buf, buflen);
  pdata->data_len = buflen;
  m_recvtmplst.push_back(pdata);

  return true;
}

#endif // OS_WINDOWS
