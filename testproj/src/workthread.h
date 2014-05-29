#ifndef _WORKTHREAD_H_
#define _WORKTHREAD_H_

#include "workbasethread.h"

class CWorkThread : public CWorkBaseThread {
public:
    CWorkThread();
    virtual ~CWorkThread();

    int processMessage(NET_DATA* pdata);
	bool ProcessServerConnected(NET_DATA* pdata);
	bool ProcessDistributeConnect(NET_DATA* pdata);
	bool ProcessMainConnected(NET_DATA* pdata);
private:
	int m_iUserCount;
};

#endif//_WORKTHREAD_H_
