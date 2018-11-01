// Copyright shenyizhong@gmail.com

#ifndef NETWORK_EUPUEPOLLTHREAD_H_
#define NETWORK_EUPUEPOLLTHREAD_H_

#ifdef OS_LINUX

#include <unordered_map>
#include "common/globaldef.h"
#include "network/euputhread.h"
#include "network/netcommon.h"

class CEpollThread : public CEupuThread {
 public:
  CEpollThread();

  virtual ~CEpollThread();

  virtual bool stop();
  virtual void run();
  virtual void reset();

  bool startup();

 private:
  bool parsePacketToRecvQueue(SOCKET_SET *psockset, char *buf, int buflen);
  bool addSocketToMap(SOCKET_SET *psockset);
  bool addClientToEpoll(SOCKET_SET *psockset);
  bool doAccept(int fd);
  void doRecvMessage(SOCKET_KEY *pkey);
  int doSendMessage(SOCKET_KEY *pkey);
  bool doListen();

  void doEpollEvent();
  void doSystemEvent();
  void closeClient(int fd, time_t conntime);
  void createClientCloseMsg(SOCKET_SET *psockset);
  bool createConnectServerMsg(SOCKET_SET *psockset);
  void doKeepaliveTimeout();
  void doSendKeepaliveToServer();
  void deleteSendMsgFromSendMap(int fd);

  time_t getIndex();

 private:
  int m_epollfd;
  int m_listenfd;
  SOCKET_KEY *m_listenkey;
  int m_keepalivetimeout;   // connection timeout time
  int m_keepaliveinterval;  // send keepalive message to server time interval
  time_t m_checkkeepalivetime;
  time_t m_lastkeepalivetime;

  std::string m_serverip;
  uint32_t m_serverport;

  int m_readbufsize;
  int m_sendbufsize;
  char *m_recvbuffer;
  int m_recvbuflen;

  struct epoll_event *m_events;

  std::list<int> m_delsendfdlist;
  std::unordered_map<int, SOCKET_SET *> m_socketmap;

  uint32_t m_maxepollsize;

  std::list<NET_DATA *> m_recvtmplst;
  time_t m_index;
};

#endif  // OS_LINUX

#endif  // NETWORK_EUPUEPOLLTHREAD_H_
