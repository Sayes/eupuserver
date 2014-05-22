#ifndef __EUPUTHREAD_H__
#define __EUPUTHREAD_H__

#include <pthread.h>
#ifdef OS_LINUX
#include <signal.h>
#elif OS_WINDOWS
#include <semaphore.h>
#endif

class CEupuThread {
public:
	CEupuThread();
	virtual ~CEupuThread();

	pthread_t GetThreadID()
	{
		return m_pid;
	}

	virtual bool start();
	virtual void pause();
	virtual void continues();
	virtual bool stop();
	virtual void reset() = 0;
	virtual bool isStarted();

protected:
    void setMaskSIGUSR1();
	static void* ThreadFunc(void *arg);
	virtual void run() = 0;

protected:
	pthread_t m_pid;
#ifdef OS_LINUX
	sigset_t m_waitSig;
#elif OS_WINDOWS
	sem_t m_waitSig;
#endif
	bool m_bOperate;
	bool m_bIsExit;
};


#endif//__EUPUTHREAD_H__
