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
            m_maxSendList = 1000;

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
        bool createMsgToSendList(int fd, time_t conntime, const string& ip, USHORT port, int ntype,  USHORT mainid, USHORT assistantid, BYTE code, BYTE reserve, CEupuStream* stream, UINT nlen);
        bool addMsgToSendList(NET_DATA* pdata);
        bool sendMsgToServer(int ntype, USHORT mainid, USHORT assistantid, BYTE code, BYTE reserve, CEupuStream* pstream, UINT nlen, bool blocked = true);
        void setServerSocket(int fd, time_t conntime, const string& peerip, USHORT peerport, int ntype);
        void switchSendMap();

        CSysQueue<NET_EVENT>* getEventQueue();
        CSysQueue<NET_DATA>* getRecvQueue();

        map<int, list<NET_DATA*>* > * getBakSendMap()
        {
            return m_pbaksendmap;
        }

    private:
        map<int, list<NET_DATA*> *> m_sendmap[2];
        map<int, list<NET_DATA*> *>* m_pcursendmap;
        map<int, list<NET_DATA*> *>* m_pbaksendmap;

        CSysQueue<NET_DATA> m_recvlist;
        CSysQueue<NET_EVENT> m_eventlist;

        static CGlobalMgr* m_pInstance;
        int m_maxSendList;
};


#endif//_GLOBALMGR_H_
