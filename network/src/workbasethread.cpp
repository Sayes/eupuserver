#include "eupulogger4system.h"
#include "sysqueue.h"
#include "globalmgr.h"
#include "workbasethread.h"

CWorkBaseThread::CWorkBaseThread()
: CEupuThread()
{
	LOG(_INFO_, "CWorkBaseThread::CWorkBaseThread() begin");
}

CWorkBaseThread::~CWorkBaseThread()
{
	LOG(_INFO_, "CWorkBaseThread::~CWorkBaseThread() begin");
}

void CWorkBaseThread::reset()
{
	LOG(_INFO_, "CWorkBaseThread::reset() begin");
}

bool CWorkBaseThread::stop()
{
	LOG(_INFO_, "CWorkBaseThread::stop() begin");

	m_bOperate = false;

	if (m_bIsExit == false)
	{
#ifdef OS_LINUX
		usleep(10);
#elif OS_WINDOWS
		Sleep(10);
#endif
	}
	return true;
}

void CWorkBaseThread::run()
{
	LOG(_INFO_, "CWorkBaseThread::run() begin");

	pause();

	m_bIsExit = false;

	CSysQueue<NET_DATA>* precvlist = CGlobalMgr::getInstance()->getRecvQueue();
	CSysQueue<NET_EVENT>* peventlist = CGlobalMgr::getInstance()->getEventQueue();
	if (precvlist == NULL || peventlist == NULL)
	{
		LOG(_ERROR_, "CWorkBaseThread::run() error, recvlist == NULL || eventlist == NULL");
		m_bOperate = false;
		m_bIsExit = true;
		return;
	}

	int nret = 0;

	while (m_bOperate)
	{
		if (precvlist->isEmptyWithoutLock())
		{
#ifdef OS_LINUX
			usleep(500);
#elif OS_WINDOWS
			Sleep(500);
#endif
			continue;
		}

		NET_DATA* pdata = NULL;

		while (precvlist->outQueue(pdata, true))
		{
			if (pdata == NULL)
			{
				LOG(_ERROR_, "CWorkBaseThread::run() error, precvlist->outQueue(pdata) get NULL data");
				continue;
			}

			nret = processMessage(pdata);

			if (nret < 0)
			{
				LOG(_ERROR_, "CWorkBaseThread::run() error, processMessage() failed");
				LOGHEX(_ERROR_, "message:", pdata->pdata, pdata->data_len);

				if (!CGlobalMgr::getInstance()->createCloseConnectEvent(pdata->fd, pdata->connect_time))
				{
					LOG(_ERROR_, "CWorkBaseThread::run() error, createCloseConnectEvent() failed, fd=%d, time=%u", pdata->fd, pdata->connect_time);
				}
			}
			delete pdata;
			//pdata = NULL;
		}
		loop();
	}
}

bool CWorkBaseThread::start()
{
	LOG(_INFO_, "CWorkBaseThread::start() begin");

	if (!CEupuThread::start())
	{
		LOG(_ERROR_, "CWorkBaseThread::loop() error, CEupuThread::start() failed");
		return false;
	}

	continues();
	return true;
}

void CWorkBaseThread::loop()
{
	LOG(_INFO_, "CWorkBaseThread::loop() begin");
	return;
}

