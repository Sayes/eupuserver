// Copyright shenyizhong@gmail.com, 2014

#ifndef NETWORK_WORKBASETHREAD_H_
#define NETWORK_WORKBASETHREAD_H_

#include "euputhread.h"
#include "netcommon.h"

class CWorkBaseThread : public CEupuThread {
   public:
    void reset();
    bool start();
    virtual bool stop();

    CWorkBaseThread();
    virtual ~CWorkBaseThread();

   protected:
    virtual int processMessage(NET_DATA *pdata) = 0;
    virtual void loop();
    void run();
};

#endif  // NETWORK_WORKBASETHREAD_H_
