// Copyright shenyizhong@gmail.com, 2014

#ifndef NETWORK_WSATHREAD_H_
#define NETWORK_WSATHREAD_H_

#ifdef OS_WINDOWS

#include <unordered_map>
#include "common/globaldef.h"
#include "network/euputhread.h"
#include "network/netcommon.h"

class CWSAThread : public CEupuThread {
   public:
    CWSAThread();
    virtual ~CWSAThread();

    virtual bool stop();
    virtual void run();
    virtual void reset();

    bool startup();

   private:
    bool parsePacketToRecvQueue(SOCKET_SET *psockset, char *buf, int buflen);
    bool addSocketToMap(SOCKET_SET *psockset);
    bool addClientToWSA(SOCKET_SET *psockset);
    bool doAccept(int fd);
    void doRecvMessage(SOCKET_KEY *pkey);
    int doSendMessage(SOCKET_KEY *pkey);
    bool doListen();

    void doWSAEvent();
    void doSystemEvent();
    void closeClient(int fd, time_t conn_time);
    void createClientCloseMsg(SOCKET_SET *psockset);
    bool createConnectServerMsg(SOCKET_SET *psockset);
    void doKeepaliveTimeout();
    void doSendKeepaliveToServer();
    void deleteSendMsgFromSendMap(int fd);

    time_t getIndex();

   private:
    int m_listenfd;
    SOCKET_KEY *m_listenkey;
    int m_keepalivetimeout;
    int m_keepaliveinterval;
    time_t m_checkkeepalivetime;
    time_t m_lastkeepalivetime;

    std::string m_serverip;
    unsigned int m_serverport;

    int m_readbufsize;
    int m_sendbufsize;
    char *m_recvbuffer;
    int m_recvbuflen;

    std::list<int> m_delsendfdlist;
    std::unordered_map<int, SOCKET_SET *> m_socketmap;

    // unsigned int m_maxwsaeventsize;

    std::list<NET_DATA *> m_recvtmplst;
    time_t m_index;

    // data for windows
    // WSAEVENT m_eventArray[WSA_MAXIMUM_WAIT_EVENTS];
    // SOCKET m_sockArray[WSA_MAXIMUM_WAIT_EVENTS];
    // SOCKET_KEY* m_keymap[WSA_MAXIMUM_WAIT_EVENTS];
    // int m_nEventTotal;
};

#endif  // OS_WINDOWS

#endif  // NETWORK_WSATHREAD_H_
