#ifndef _WORKTHREAD_H_
#define _WORKTHREAD_H_

#include "network/workbasethread.h"

class CWorkThread : public CWorkBaseThread {
   public:
    CWorkThread();
    virtual ~CWorkThread();

    int processMessage(NET_DATA* pdata);
    int ProcessServerConnected(NET_DATA* pdata);
};

#endif  //_WORKTHREAD_H_