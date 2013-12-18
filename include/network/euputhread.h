#ifndef __EUPUTHREAD_H__
#define __EUPUTHREAD_H__

#include <pthread.h>

class CEupuThread {
public:
	CEupuThread();
	virtual ~CEupuThread();

	pthread_t GetThreadID()
	{
		return m_pid;
	}

	virtual bool start();
	virtual bool pause();
	virtual bool continues();
	virtual bool stop();
	virtual void reset() = 0;
	virtual bool isstarted();

protected:
	static void* ThreadFun(void *arg);
	virtual void run() = 0;

protected:
	pthread_t m_pid;
	sigset_t m_waitSig;
	bool m_bOperate;
	bool m_bIsExist;
};


#endif//__EUPUTHREAD_H__
