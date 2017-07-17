// Copyright shenyizhong@gmail.com, 2014

#ifndef NETWORK_THREADLOCK_H_
#define NETWORK_THREADLOCK_H_

#include <pthread.h>

class CThreadLock {
   public:
    CThreadLock();
    virtual ~CThreadLock();
    void Lock();
    void UnLock();

   private:
    pthread_mutex_t m_Mutex;
};

#endif  // NETWORK_THREADLOCK_H_
