// p2ptrans.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "eupulogger4system.h"
#include "eupu.base.pb.h"
#include "eupu_inet.h"
#include "globaldef.h"
#include "netcommon.h"

#define PORT			2300
#define SENDBUF_LEN		1024
#define RECVBUF_LEN		1024
#define MSGHEAD_LEN		64

DWORD dwThreadId = 0;
HANDLE hThread = NULL;

unsigned int WINAPI ReceiveThread(LPVOID lpParam);

SOCKET sockListen;

SOCKADDR_IN addrServer;

char SENDBUF[SENDBUF_LEN];

int _tmain(int argc, _TCHAR* argv[])
{
	// this district for test /////////////////////////////






	///////////////////////////////////////////////////////



	LOG(_ERROR_, "SERVER: main() begin");


	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	WSAEVENT NewEvent = WSACreateEvent();

	int sin_size = 0;

	char tmpbuf[] = {0, 20, 13, 13, 20, 9, 0, 0, 40, 221};
	int tmplen = sizeof(tmpbuf);

	std::string trystr;
	trystr.append(tmpbuf, tmplen);
	int strsize = trystr.size();


	eupu::NetData msgdata;
	msgdata.set_fd(1000);
	msgdata.set_connect_time(34234523);
	msgdata.set_peer_ip("10.10.40.177");
	msgdata.set_peer_port(2300);
	msgdata.set_type(2);
	msgdata.set_datalen(tmplen);

	eupu::NetMessageHead msghead;
	msghead.set_u32msgsize(msgdata.ByteSize());
	msghead.set_u32mainid(200000000);
	msghead.set_u32assisid(200000000);
	msghead.set_u32handlecode(200000000);
	msghead.set_u32reserve(200000000);
	assert(msghead.ByteSize() < MSGHEAD_LEN);

	char* p = SENDBUF;

	if (!msghead.SerializeToArray(p, msghead.ByteSize()))
	{
		LOG(_ERROR_, "SERVER: syz::NetMessageHead::ParseToArray(SENDBUF) error");
	}

	p += MSGHEAD_LEN;

	if (!msgdata.SerializeToArray(p, msgdata.ByteSize()))
	{
		LOG(_ERROR_, "SERVER: syz::NetData::ParseToArray(SENDBUF) error");
	}

	p += msgdata.ByteSize();
	
	memcpy_s(p, SENDBUF_LEN - (p - SENDBUF), tmpbuf, tmplen);

	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err)
	{
		WSACleanup();
		exit(0);
	}

	sockListen = socket(AF_INET, SOCK_STREAM, 0);

	if (INVALID_SOCKET == sockListen)
	{
		LOG(_ERROR_, "SERVER: sockListen == INVALID_SOCKET");
		WSACleanup();
		exit(0);
	}

	hThread = (HANDLE)_beginthreadex(NULL, 0, ReceiveThread, NULL, 0, NULL);

	LOG(_ERROR_, "SERVER: WaitForSingleObject hThread");
	WaitForSingleObject((HANDLE)hThread, INFINITE);

	CEupuLogger4System::Release();

	return 0;
}

unsigned int WINAPI ReceiveThread(LPVOID lpParam)
{

	UNREFERENCED_PARAMETER(lpParam);

	LOG(_ERROR_, "SERVER: ReceiveThread() begin");

	bool bResult = true;
	DWORD nEventTotal = 0;
	WSAEVENT eventArray[WSA_MAXIMUM_WAIT_EVENTS];
	SOCKET sockArray[WSA_MAXIMUM_WAIT_EVENTS];

	memset(eventArray, 0, sizeof(WSAEVENT) * WSA_MAXIMUM_WAIT_EVENTS);
	memset(sockArray, 0, sizeof(SOCKET) * WSA_MAXIMUM_WAIT_EVENTS);

	sockArray[nEventTotal] = sockListen;

	if (WSA_INVALID_EVENT == (eventArray[nEventTotal] = ::WSACreateEvent()))
	{
		LOG(_ERROR_, "SERVER: ReceiveThread() ::WSACreateEvent() == WSA_INVALID_EVENT");
		bResult = false;
	}

	if (bResult)
	{
		if (SOCKET_ERROR == WSAEventSelect(sockListen, eventArray[nEventTotal], FD_ACCEPT | FD_CLOSE))
		{
			LOG(_ERROR_, "SERVER: ReceiveThread() ::WSAEventSelect() == SOCKET_ERROR");
			bResult = false;
		}
		else
		{
			nEventTotal++;
		}
	}

	if (bResult)
	{
		addrServer.sin_family = AF_INET;
		addrServer.sin_port = htons(PORT);
		addrServer.sin_addr.s_addr = INADDR_ANY;
		memset(&(addrServer.sin_zero), 0, sizeof(addrServer.sin_zero));

		if (SOCKET_ERROR == bind(sockListen, (SOCKADDR*)&addrServer, sizeof(SOCKADDR)))
		{
			LOG(_ERROR_, "SERVER: ReceiveThread() bind() == SOCKET_ERROR");
			bResult = false;
		}
	}

	if (bResult)
	{
		if (SOCKET_ERROR == listen(sockListen, 5))
		{
			LOG(_ERROR_, "SERVER: ReceiveThread() listen() == SOCKET_ERROR");
			bResult = false;
		}
	}

	if (bResult)
	{
		DWORD dwIdxSignaled = 0;
		SOCKET sockSignaled = INVALID_SOCKET;
		WSAEVENT eventSignaled = WSA_INVALID_EVENT;

		for(;;)
		{
			LOG(_ERROR_, "SERVER: ReceiveThread() ::WSAWaitForMultipleEvents() begin");
			dwIdxSignaled = ::WSAWaitForMultipleEvents(nEventTotal, eventArray, FALSE, INFINITE, FALSE);
			LOG(_ERROR_, "SERVER: ReceiveThread() ::WSAWaitForMultipleEvents() signaled");

			if (WSA_WAIT_FAILED == dwIdxSignaled)
			{
				LOG(_ERROR_, "SERVER: ReceiveThread() dwIdxSignaled == WSA_WAIT_FAILED");
				break;
			}

			sockSignaled = sockArray[dwIdxSignaled - WSA_WAIT_EVENT_0];
			eventSignaled = eventArray[dwIdxSignaled - WSA_WAIT_EVENT_0];

			WSANETWORKEVENTS networkEvents;
			if (SOCKET_ERROR == ::WSAEnumNetworkEvents(sockSignaled, eventSignaled, &networkEvents))
			{
				LOG(_ERROR_, "SERVER: ReceiveThread() ::WSAEnumNetworkEvents() == SOCKET_ERROR");
				break;
			}

			if (networkEvents.lNetworkEvents & FD_ACCEPT)
			{
				if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
				{
					LOG(_ERROR_, "SERVER: ReceiveThread() networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0");
					break;
				}

				SOCKET sockClient = INVALID_SOCKET;
				SOCKADDR_IN addrClient;
				int sin_size = sizeof(SOCKADDR_IN);

				if (INVALID_SOCKET == (sockClient = accept(sockListen, (SOCKADDR*)&addrClient, &sin_size)))
				{
					LOG(_ERROR_, "SERVER: ReceiveThread() accept() == INVALID_SOCKET");
					break;
				}

				if (SOCKET_ERROR == send(sockClient, SENDBUF, SENDBUF_LEN, 0))
				{
					LOG(_ERROR_, "SERVER: ReceiveThread() send() == SOCKET_ERROR : %d", WSAGetLastError());
					break;
				}

				if (SOCKET_ERROR == closesocket(sockClient))
				{
					break;
				}
			}

			if (networkEvents.lNetworkEvents & FD_READ)
			{
			}

			if (networkEvents.lNetworkEvents & FD_CLOSE)
			{
				LOG(_ERROR_, "SERVER: ReceiveThread() client close the socket");
			}
		}
	}

	for (int i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
	{
		WSACloseEvent(eventArray[i]);
		closesocket(sockArray[i]);
	}
	WSACleanup();

	LOG(_ERROR_, "SERVER: ReceiveThread() end");

	return 0;
}