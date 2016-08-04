#ifndef _GLOBALMGR_H_
#define _GLOBALMGR_H_

#include <set>
#include <map>
#include <list>
#include "netcommon.h"
#include "threadlock.h"
#include "sysqueue.h"
#include "eupustream.h"

using std::set;
using std::map;
using std::list;

class CGlobalMgr {
private:
    CGlobalMgr()
    {
        m_nMaxSendList = 1000;

        m_pcursendmap = &m_sendmap[0];
        m_pbaksendmap = &m_sendmap[1];
    }

    ~CGlobalMgr()
    {
        clean();
    }
public:
    static void release();
    static CGlobalMgr* getInstance();
public:
    bool init();
    void clean();

    void createServerConnect(int32_t ntype);
    bool createCloseConnectEvent(int32_t fd, time_t conntime);
    bool createMsgToSendList(int32_t fd, time_t conntime, const string& ip, uint16_t port, int32_t ntype,  uint16_t mainid, uint16_t assistantid, BYTE code, BYTE reserve, CEupuStream* stream, uint32_t nlen);
    bool addMsgToSendList(NET_DATA* pdata);
    bool sendMsgToServer(int32_t ntype, uint16_t mainid, uint16_t assistantid, BYTE code, BYTE reserve, CEupuStream* pstream, uint32_t nlen, bool blocked = true);
    void setServerSocket(int32_t fd, time_t conntime, const string& peerip, uint16_t peerport, int32_t ntype);
    void sendKeepaliveMsgToAllServer();

    void switchSendMap();

    void setMainSocket(int32_t fd, time_t conntime, const string& peerip, uint16_t peerport, int32_t ntype);
    void setDistributeSocket(int32_t fd, time_t conntime, const string& peerip, uint16_t peerport, int32_t ntype);
    void setUserCenterSocket(int32_t fd, time_t conntime, const string& peerip, uint16_t peerport, int32_t ntype);
    void setLogSocket(int32_t fd, time_t conntime, const string& peerip, uint16_t peerport, int32_t ntype);

    SysQueue<NET_EVENT>* getEventQueue();
    SysQueue<NET_DATA>* getRecvQueue();

    map<int32_t, list<NET_DATA*>* >* getBakSendMap()
    {
        return m_pbaksendmap;
    }

private:
    NET_DATA m_logkey;
    NET_DATA m_usercenterkey;
    NET_DATA m_mainkey;
    NET_DATA m_distributekey;
    CThreadLock m_serverlock;

    map<int32_t, list<NET_DATA*> *> m_sendmap[2];
    map<int32_t, list<NET_DATA*> *>* m_pcursendmap;
    map<int32_t, list<NET_DATA*> *>* m_pbaksendmap;
    CThreadLock m_sendmaplock;

    SysQueue<NET_DATA> m_recvlist;
    SysQueue<NET_EVENT> m_eventlist;

    static CGlobalMgr* m_pInstance;
    int32_t m_nMaxSendList;
};


#endif//_GLOBALMGR_H_
