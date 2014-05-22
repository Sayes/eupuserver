#include "euputhread.h"
#include "string.h"

CEupuThread::CEupuThread()
: m_bOperate(false)
, m_bIsExit(true)
{
    setMaskSIGUSR1();
#ifdef OS_LINUX
	m_pid = 0;
    sigemptyset(&m_waitSig);
    sigaddset(&m_waitSig, SIGUSR1);
#elif OS_WINDOWS
	//TODO
	memset(&m_pid, 0, sizeof(pthread_t));
#endif
}

CEupuThread::~CEupuThread()
{
    m_bOperate = false;
    m_bIsExit = false;
}

void* CEupuThread::ThreadFunc(void* arg)
{
#ifdef OS_LINUX
    CEupuThread* pthread = (CEupuThread*)arg;
    pthread->run();
    return NULL;
#elif OS_WINDOWS
	//TODO
    CEupuThread* pthread = (CEupuThread*)arg;
    pthread->run();
    return NULL;
#endif
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
#ifdef OS_LINUX
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
#elif OS_WINDOWS
	//TODO
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
#endif
}

void CEupuThread::pause()
{
    int sig;
#ifdef OS_LINUX
    sigwait(&m_waitSig, &sig);
#elif OS_WINDOWS
	//TODO
	sem_wait(&m_waitSig);
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
