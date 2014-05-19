#ifndef __EUPUTHREAD_H__
#define __EUPUTHREAD_H__

#include <pthread.h>
#include <signal.h>

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
	sigset_t m_waitSig;
	bool m_bOperate;
	bool m_bIsExit;
};


#endif//__EUPUTHREAD_H__
