#ifndef __THREADLOCK_H__
#define __THREADLOCK_H__

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

#endif //__THREADLOCK_H__
