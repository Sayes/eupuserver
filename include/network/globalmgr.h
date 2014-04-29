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
        static void Release();
        static CGlobalMgr* getInstance();
    public:
        bool init();
        void clean();

        void createServerConnect(int ntype);
        bool sendMsgToServer(int ntype, USHORT mainid, BYTE assistantid, BYTE code, BYTE reserve, CEupuStream* pstream, UINT nlen, bool blocked = true);
        void setServerSocket(int fd, time_t conntime, const string& peerip, USHORT peerport, int ntype);

        CSysQueue<NET_EVENT>* getEventQueue();
        CSysQueue<NET_DATA>* getRecvQueue();
    private:
        map<int, list<NET_DATA*> *> m_sendmap[2];
        map<int, list<NET_DATA*> *>* m_pcursendmap;
        map<int, list<NET_DATA*> *>* m_pbaksendmap;

        CSysQueue<NET_DATA> m_recvlist;

        static CGlobalMgr* m_pInstance;
        int m_maxSendList;
};


#endif//_GLOBALMGR_H_
