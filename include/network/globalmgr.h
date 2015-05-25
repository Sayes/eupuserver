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

    void createServerConnect(int ntype);
    bool createCloseConnectEvent(int fd, time_t conntime);
    bool createMsgToSendList(int fd, time_t conntime, const string& ip, USHORT port, int ntype,  USHORT mainid, USHORT assistantid, BYTE code, BYTE reserve, CEupuStream* stream, UINT nlen);
    bool addMsgToSendList(NET_DATA* pdata);
    bool sendMsgToServer(int ntype, USHORT mainid, USHORT assistantid, BYTE code, BYTE reserve, CEupuStream* pstream, UINT nlen, bool blocked = true);
    void setServerSocket(int fd, time_t conntime, const string& peerip, USHORT peerport, int ntype);
    void sendKeepaliveMsgToAllServer();

    void switchSendMap();

    void setMainSocket(int fd, time_t conntime, const string& peerip, USHORT peerport, int ntype);
    void setDistributeSocket(int fd, time_t conntime, const string& peerip, USHORT peerport, int ntype);
    void setUserCenterSocket(int fd, time_t conntime, const string& peerip, USHORT peerport, int ntype);
    void setLogSocket(int fd, time_t conntime, const string& peerip, USHORT peerport, int ntype);

    SysQueue<NET_EVENT>* getEventQueue();
    SysQueue<NET_DATA>* getRecvQueue();

    map<int, list<NET_DATA*>* >* getBakSendMap()
    {
        return m_pbaksendmap;
    }

private:
    NET_DATA m_logkey;
    NET_DATA m_usercenterkey;
    NET_DATA m_mainkey;
    NET_DATA m_distributekey;
    CThreadLock m_serverlock;

    map<int, list<NET_DATA*> *> m_sendmap[2];
    map<int, list<NET_DATA*> *>* m_pcursendmap;
    map<int, list<NET_DATA*> *>* m_pbaksendmap;
    CThreadLock m_sendmaplock;

    SysQueue<NET_DATA> m_recvlist;
    SysQueue<NET_EVENT> m_eventlist;

    static CGlobalMgr* m_pInstance;
    int m_nMaxSendList;
};


#endif//_GLOBALMGR_H_
