#include "network/threadlock.h"

CThreadLock::CThreadLock() { pthread_mutex_init(&m_Mutex, NULL); }

CThreadLock::~CThreadLock() { pthread_mutex_destroy(&m_Mutex); }

void CThreadLock::Lock() { pthread_mutex_lock(&m_Mutex); }

void CThreadLock::UnLock() { pthread_mutex_unlock(&m_Mutex); }
