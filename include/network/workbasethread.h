#ifndef _WORKBASETHREAD_H_
#define _WORKBASETHREAD_H_

#include "euputhread.h"
#include "netcommon.h"

class CWorkBaseThread : public CEupuThread {
public:
    void reset();
    bool start();
    virtual bool stop();
    bool isStop()
    {
        return m_bIsExist;
    }

    CWorkBaseThread();
    virtual ~CWorkBaseThread();

protected:
    virtual int processMessage(NET_DATA* pdata) = 0;
    virtual void loop();
    void run();
};


#endif//_WORKBASETHREAD_H_
