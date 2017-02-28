#ifndef _GLOBALMGR_H_
#define _GLOBALMGR_H_

#include "protocol/eupustream.h"
#include "network/netcommon.h"
#include "network/sysqueue.h"
#include "network/threadlock.h"
#include <list>
#include <map>
#include <set>

// using std::set;
// using std::map;
// using std::list;

class CGlobalMgr {
private:
  CGlobalMgr() {
    m_nMaxSendList = 1000;

    m_pcursendmap = &m_sendmap[0];
    m_pbaksendmap = &m_sendmap[1];
  }

  ~CGlobalMgr() { clean(); }

public:
  static void release();
  static CGlobalMgr *getInstance();

public:
  bool init();
  void clean();

  void createServerConnect(int ntype);
  bool createCloseConnectEvent(int fd, time_t conntime);
  bool createMsgToSendList(int fd, time_t conntime, const std::string &ip,
                           uint16_t port, int ntype, uint16_t mainid,
                           uint16_t assistantid, BYTE code, BYTE reserve,
                           CEupuStream *stream, uint32_t nlen);
  bool addMsgToSendList(NET_DATA *pdata);
  bool sendMsgToServer(int ntype, uint16_t mainid, uint16_t assistantid,
                       BYTE code, BYTE reserve, CEupuStream *pstream,
                       uint32_t nlen, bool blocked = true);
  void setServerSocket(int fd, time_t conntime, const std::string &peerip,
                       uint16_t peerport, int ntype);
  void sendKeepaliveMsgToAllServer();

  void switchSendMap();

  void setMainSocket(int fd, time_t conntime, const std::string &peerip,
                     uint16_t peerport, int ntype);
  void setDistributeSocket(int fd, time_t conntime, const std::string &peerip,
                           uint16_t peerport, int ntype);
  void setUserCenterSocket(int fd, time_t conntime, const std::string &peerip,
                           uint16_t peerport, int ntype);
  void setLogSocket(int fd, time_t conntime, const std::string &peerip,
                    uint16_t peerport, int ntype);

  SysQueue<NET_EVENT> *getEventQueue();
  SysQueue<NET_DATA> *getRecvQueue();

  std::map<int, std::list<NET_DATA *> *> *getBakSendMap() {
    return m_pbaksendmap;
  }

private:
  NET_DATA m_logkey;
  NET_DATA m_usercenterkey;
  NET_DATA m_mainkey;
  NET_DATA m_distributekey;
  CThreadLock m_serverlock;

  std::map<int, std::list<NET_DATA *> *> m_sendmap[2];
  std::map<int, std::list<NET_DATA *> *> *m_pcursendmap;
  std::map<int, std::list<NET_DATA *> *> *m_pbaksendmap;
  CThreadLock m_sendmaplock;

  SysQueue<NET_DATA> m_recvlist;
  SysQueue<NET_EVENT> m_eventlist;

  static CGlobalMgr *m_pInstance;
  int m_nMaxSendList;
};

#endif //_GLOBALMGR_H_
