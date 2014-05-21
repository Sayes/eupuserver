#include "euputhread.h"

CEupuThread::CEupuThread()
: m_bOperate(false)
, m_bIsExit(true)
#ifdef OS_LINUX
, m_pid(0)
#endif
{
    setMaskSIGUSR1();
#ifdef OS_LINUX
    sigemptyset(&m_waitSig);
    sigaddset(&m_waitSig, SIGUSR1);
#elif OS_WINDOWS
	//TODO
#endif
}

CEupuThread::~CEupuThread()
{
    m_bOperate = false;
    m_bIsExit = false;
}

void* CEupuThread::ThreadFunc(void* arg)
{
    CEupuThread* pthread = (CEupuThread*)arg;
    pthread->run();
    return NULL;
}

void CEupuThread::setMaskSIGUSR1()
{
#ifdef OS_LINUX
    sigset_t sig;
    sigemptyset(&sig);
    sigaddset(&sig, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &sig, NULL);
#elif OS_WINDOWS
	//TODO
#endif
}

bool CEupuThread::start()
{
    int nret = pthread_create(&m_pid, NULL, ThreadFunc, this);
    
    if (0 == nret)
    {
        nret = pthread_detach(m_pid);
        if (0 == nret)
        {
            m_bOperate = true;
            m_bIsExit = false;
            return true;
        }
    }
    else
    {
        return false;
    }
    return true;
}

void CEupuThread::pause()
{
    int sig;
#ifdef OS_LINUX
    sigwait(&m_waitSig, &sig);
#elif OS_WINDOWS
	//TODO
#endif
}

//continue the thread, the real object needn't implement this method
void CEupuThread::continues()
{
    //send SIGUSR1 signal to current thread to continue
#ifdef OS_LINUX
    pthread_kill(m_pid, SIGUSR1);
#elif OS_WINDOWS
	//TODO
#endif
}

bool CEupuThread::stop()
{
    m_bOperate = false;
	return true;
}

bool CEupuThread::isStarted()
{
    return m_bOperate;
}
