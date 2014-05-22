#ifdef OS_WINDOWS

#include "wsathread.h"
#include "sysqueue.h"
#include "common.h"
#include "globalmgr.h"
#include "eupulogger4system.h"
#include "sprotocol.h"

CWSAThread::CWSAThread()
{
}

CWSAThread::~CWSAThread()
{
}

void CWSAThread::run()
{
}

void CWSAThread::reset()
{
}

bool CWSAThread::startup()
{
	return false;
}

time_t CWSAThread::getIndex()
{
	m_index++;
	if (m_index == 0xFFFFFFFF)
		m_index = 1;
	return m_index;
}

void CWSAThread::doWSAEvent()
{
}

bool CWSAThread::stop()
{
	return false;
}

void CWSAThread::doKeepaliveTimeout()
{
}

void CWSAThread::doSendKeepaliveToServer()
{
}

bool CWSAThread::doListen()
{
	return false;
}

bool CWSAThread::doAccept(int fd)
{
	return false;
}

bool CWSAThread::addClientToWSA(SOCKET_SET* psockset)
{
	return false;
}

void CWSAThread::doRecvMessage(SOCKET_KEY* pkey)
{
}

int CWSAThread::doSendMessage(SOCKET_KEY* pkey)
{
	return 0;
}

bool CWSAThread::parsePacketToRecvQueue(SOCKET_SET *psockset, char *buf, int buflen)
{
	return false;
}

void CWSAThread::closeClient(int fd, time_t conn_time)
{
}

void CWSAThread::createClientCloseMsg(SOCKET_SET *psockset)
{
}

bool CWSAThread::addSocketToMap(SOCKET_SET *psockset)
{
	return false;
}

void CWSAThread::deleteSendMsgFromSendMap(int fd)
{
}

void CWSAThread::doSystemEvent()
{
}

bool CWSAThread::createConnectServerMsg(SOCKET_SET *psockset)
{
	return false;
}

#endif//OS_WINDOWS