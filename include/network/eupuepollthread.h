#ifndef __EUPUEPOLLTHREAD_H__
#define __EUPUEPOLLTHREAD_H__

#include <euputhread.h>
#include <netcommon.h>


class CEupuEpollThread : public CEupuThread {
public:
	CEupuEpollThread();

	virtual ~CEupuEpollThread();

	virtual bool Stop();
	virtual void Run();
	virtual void Reset();

	bool IsStop();
	{
		return false;
	}

	bool StartUp();
};











#endif
