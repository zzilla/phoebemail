
#include "ITPObject.h"

DWORD WINAPI RoutineThread(LPVOID completionPortID);
DWORD WINAPI RoutineDealThread(LPVOID lpParam);

//////////////////////////////////////////////////////////////////////////

static HANDLE	g_hCompletionPort = NULL;
static int		g_nNumberOfThreads = 0;
static HANDLE*	g_hThreads = NULL;
static long		g_clientId_new = 0;
static long		g_port_new = 10000;

static int		g_nDealThreadsNum = 0;
static HANDLE*	g_hDealThreads = NULL;
static HANDLE	g_hExitEvent;
static DEAL_LIST g_lstDealSocks;
static CReadWriteMutex g_csDealSock;

//////////////////////////////////////////////////////////////////////////

ITPObject::ITPObject()
{
	m_socket	= INVALID_SOCKET;
    m_remoteIp	= INADDR_ANY;
    m_remotePort= 0;
	m_localIp	= INADDR_ANY;
	m_localPort	= 0;

	m_listener = NULL;
	m_engineId = 0;

	m_recvBuffSize = 64 * 1024;
	m_sendBuffSize = 64 * 1024;

	m_bReconnEn = FALSE;
	m_bDisCallBack = TRUE;
	m_bDetectDisconn = TRUE;
	m_nSendQueueSize = 1000; // 超出这个值认为发送超时，SDK发送的数据包都较小，应该设大点
	m_nDetectDisconnTime = 60;

	m_timeout_sec = 0;
	m_timeout_usec = 0;
}

ITPObject::~ITPObject()
{

}

int ITPObject::Startup(void)
{
	WSADATA wsa = {0};
	int ret = WSAStartup(MAKEWORD(2,2), &wsa);
	if (ret != 0)
	{
		return -1;
	}

	if (g_hCompletionPort != NULL)
	{
		// 完成端口已经创建
		return 1;
	}
	
	g_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == g_hCompletionPort) 
	{
		return -1;
	}
	
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	//create 1 threads for each processor and 1 extra
	g_nNumberOfThreads = systemInfo.dwNumberOfProcessors + 1; 
	
	if (NULL == g_hThreads)
	{
		g_hThreads = new HANDLE[g_nNumberOfThreads];
		if (NULL == g_hThreads)
		{
			return -1;
		}
	}

	for (int i = 0; i < g_nNumberOfThreads; i++)
	{
		DWORD dwThreadID = 0;
		g_hThreads[i] = CreateThread(NULL, 0, RoutineThread, g_hCompletionPort, 0, &dwThreadID);
		if (NULL == g_hThreads[i])
		{
			return -1;
		}
		//	对于实时且流量较大的数据，需要提高线程优先级
		//SetThreadPriority(g_hThreads[i], REALTIME_PRIORITY_CLASS);
	}

	g_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (NULL == g_hExitEvent) 
	{
		return -1;
	}

	g_nDealThreadsNum = systemInfo.dwNumberOfProcessors + 3;
	if (NULL == g_hDealThreads)
	{
		g_hDealThreads = new HANDLE[g_nDealThreadsNum];
		if (NULL == g_hDealThreads)
		{
			return -1;
		}
	}

	for (int j = 0; j < g_nDealThreadsNum; j++)
	{
		DWORD dwThreadID = 0;
		g_hDealThreads[j] = CreateThread(NULL, 0, RoutineDealThread, NULL, 0, &dwThreadID);
		if (NULL == g_hDealThreads[j])
		{
			return -1;
		}
	}
	
	return 1;
}

int ITPObject::Cleanup(void)
{
	if (NULL != g_hCompletionPort)
	{
		int i = 0;
		for (i = 0; i < g_nNumberOfThreads; i++)
		{
			//	关闭工作线程
			PostQueuedCompletionStatus(g_hCompletionPort, 0, 0, 0);	
		}
		
		DWORD dw = WaitForMultipleObjects(g_nNumberOfThreads, g_hThreads, TRUE, INFINITE);
		
		CloseHandle(g_hCompletionPort);
		g_hCompletionPort = NULL;
		
		for (i = 0; i < g_nNumberOfThreads; i++)
		{
			//to end each thread
			CloseHandle(g_hThreads[i]);	
		}
		
		g_nNumberOfThreads = 0;
		delete[] g_hThreads;
		g_hThreads = NULL;

		SetEvent(g_hExitEvent);
		DWORD dwRet = WaitForMultipleObjects(g_nDealThreadsNum, g_hDealThreads, TRUE, INFINITE);

		CloseHandle(g_hExitEvent);
		g_hExitEvent = NULL;

		for (i = 0; i < g_nDealThreadsNum; i++)
		{
			CloseHandle(g_hDealThreads[i]);
		}

		g_nDealThreadsNum = 0;
		delete[] g_hDealThreads;
		g_hDealThreads = NULL;
	}

	if (WSACleanup() == SOCKET_ERROR)
	{
		return -1;
	}

	return 1;
}

/*
 *	摘要：完成端口的工作线程
 */
DWORD WINAPI RoutineThread(LPVOID completionPortID)
{
	HANDLE hCompletionPort = (HANDLE)completionPortID;
	
	DWORD dwBytesTransferred = 0;
	LPOVERLAPPED pOl = NULL;
	CPerHandleData* pPerHandleData = NULL;
	CPerIoData* pPerIoData = NULL;
	
	DWORD dwFlags = 0;
	int nRet = 0;
	BOOL bRet = FALSE;
	
	while (TRUE)
	{
		bRet = GetQueuedCompletionStatus(hCompletionPort, &dwBytesTransferred, (LPDWORD)&pPerHandleData, &pOl, INFINITE);
		if (!bRet) 
		{
			// socket has closed, post request immediately complete
			if (GetLastError() == ERROR_INVALID_HANDLE) 
			{
				//the IOCP handle probably has been closed
				break;
			}
			else if (pOl != NULL && pPerHandleData != NULL)
			{
				pPerIoData = CONTAINING_RECORD(pOl, CPerIoData, m_ol);

				//	失败的原因可能有：1.调用了closesocket()；2.投递的线程退出(需要再重新投递，否则会一直不能接收)
				DWORD dwError = GetLastError();
				if (ERROR_OPERATION_ABORTED == dwError)
				{
					pPerIoData->m_nErrorCount ++;

					// receive
					if (pPerIoData->m_operationType == opType_recv && pPerIoData->m_nErrorCount < 2)
					{
						CReadWriteMutexLock lock(pPerHandleData->m_pcsCallBack, false, true, false);
						if (pPerHandleData->m_listener != NULL)
						{
							pPerHandleData->m_listener->onIOData(pPerHandleData->m_engineId, pPerHandleData->m_connId, (unsigned char*)pPerIoData->m_wsabuf.buf, dwBytesTransferred, FALSE);
						}
						lock.Unlock();
					}
					// send
					else if (pPerIoData->m_operationType == opType_send && pPerIoData->m_nErrorCount < 2)
					{
						CReadWriteMutexLock lock(pPerHandleData->m_pcsCallBack, false, true, false);
						if (pPerHandleData->m_listener != NULL)
						{
							pPerHandleData->m_listener->onIOSendDataAck(pPerHandleData->m_engineId, pPerHandleData->m_connId, pPerIoData->m_id, FALSE);
						}
						lock.Unlock();
					}
				}

				pPerIoData->DecRef();
				pPerHandleData->DecRef();
			}

			continue;
		}

		if (NULL == pPerHandleData) 
		{
			//receive command to exit thread
			break;
		}

		pPerIoData = CONTAINING_RECORD(pOl, CPerIoData, m_ol);

		// disconnect
		if (dwBytesTransferred == 0 
			&&(pPerIoData->m_operationType == opType_recv 
			||pPerIoData->m_operationType == opType_send))
		{
			//disconnected by remote host or something
			CReadWriteMutexLock lock(pPerHandleData->m_pcsCallBack);
			if (pPerHandleData->m_listener != NULL)
			{
				pPerHandleData->m_listener->onIODisconnect(pPerHandleData->m_engineId, pPerHandleData->m_connId);
			}
			lock.Unlock();

			//dec reference
			pPerIoData->DecRef();
			pPerHandleData->DecRef();
			continue;
		}

		// receive
		if (pPerIoData->m_operationType == opType_recv)
		{
			//deal with data
			CReadWriteMutexLock lock(pPerHandleData->m_pcsCallBack, false, true, false);
			if (pPerHandleData->m_listener != NULL)
			{
				//OutputDebugString("Enter recv data call back \n");
				pPerHandleData->m_listener->onIOData(pPerHandleData->m_engineId, pPerHandleData->m_connId, (unsigned char*)pPerIoData->m_wsabuf.buf, dwBytesTransferred, TRUE);
				//OutputDebugString("Leave recv data call back \n");
			}
			lock.Unlock();
			
			//dec reference
			pPerIoData->DecRef();
			pPerHandleData->DecRef();
		}
		// send
		else if (pPerIoData->m_operationType == opType_send)
		{
			CReadWriteMutexLock lock(pPerHandleData->m_pcsCallBack, false, true, false);

			pPerIoData->m_lSentBytes += dwBytesTransferred;
			if (pPerIoData->m_lSentBytes < pPerIoData->m_lTotalBytes)
			{
				//   
				//   the   previous   write   operation   didn't   send   all   the   data,   
				//   post   another   send   to   complete   the   operation   
				//   
				memset(&pPerIoData->m_ol, 0, sizeof(OVERLAPPED));
				pPerIoData->m_wsabuf.buf += dwBytesTransferred;
				pPerIoData->m_wsabuf.len -= dwBytesTransferred;
				pPerIoData->m_operationType = opType_send;
				DWORD dwSendBytes = 0;

				if (pPerIoData->m_operationMode == opMode_tcp)
				{
					nRet = WSASend(pPerHandleData->m_socket, &pPerIoData->m_wsabuf, 1, &dwSendBytes, 0, &pPerIoData->m_ol, NULL);
					if (nRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) 
					{
						DWORD dd = WSAGetLastError();
					}
					else
					{
						lock.Unlock();
						continue;
					}
				}
				else if (pPerIoData->m_operationMode == opMode_udp)
				{
					int to_len = sizeof(struct sockaddr_in);
					nRet = WSASendTo(pPerHandleData->m_socket, &pPerIoData->m_wsabuf, 1, &dwSendBytes, 0, (struct sockaddr*)&pPerIoData->m_remoteAddr, to_len, &pPerIoData->m_ol, NULL);
					if (nRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) 
					{
						DWORD dd = WSAGetLastError();
					}
					else
					{
						lock.Unlock();
						continue;
					}
				}
			}
			
			if (pPerHandleData->m_listener != NULL)
			{
				//OutputDebugString("Enter send data call back \n");
				pPerHandleData->m_listener->onIOSendDataAck(pPerHandleData->m_engineId, pPerHandleData->m_connId, pPerIoData->m_id, TRUE);
				//OutputDebugString("Leave send data call back \n");
			}
			lock.Unlock();

			//dec reference
			pPerIoData->DecRef();
			pPerHandleData->DecRef();
		}
		else if (pPerIoData->m_operationType == opType_none)
		{
			//closed
			pPerIoData->DecRef();
			pPerHandleData->DecRef();
		}

		continue;
	}

	return 0;
}

/*
 *	摘要：线程用于处理接收数据
 */
DWORD WINAPI RoutineDealThread(LPVOID lpParam)
{
	unsigned char dealBuf[MAX_PACKET_SIZE];

	while (WaitForSingleObject(g_hExitEvent, 0) != WAIT_OBJECT_0) 
	{
		BOOL bSleepThread = TRUE;
		int nRet = -1;

		CReadWriteMutexLock lock(g_csDealSock, false, true, false);

		if (!g_lstDealSocks.empty())
		{
			list<CPerHandleData*>::iterator it = g_lstDealSocks.begin();
			while(it != g_lstDealSocks.end())
			{
				CPerHandleData *pPerHandleData = (*it);
				if (pPerHandleData != NULL)
				{
					if (pPerHandleData->m_listener != NULL)
					{
						nRet = pPerHandleData->m_listener->onIODealData(pPerHandleData->m_engineId, pPerHandleData->m_connId, dealBuf, MAX_PACKET_SIZE);
						if (nRet == 0)
						{
							bSleepThread = FALSE;
						}
					}
				}

				++it;
			}
		}
		
		lock.Unlock();

		if (bSleepThread)
		{
			Sleep(10);
		}
	}

	return 0;
}

int ITPObject::AddSocketToIOCP(SOCKET socket, CPerHandleData *pPerHandleData)
{
	if (g_hCompletionPort == NULL || socket == INVALID_SOCKET || pPerHandleData == NULL)
	{
		return -1;
	}

	HANDLE hCompletionPort = CreateIoCompletionPort((HANDLE)socket, g_hCompletionPort, (DWORD)pPerHandleData, 0);
	if (hCompletionPort != g_hCompletionPort)
	{
		// failed
		return -1;
	}

	CReadWriteMutexLock lock(g_csDealSock);

	g_lstDealSocks.push_back(pPerHandleData);

	return 1;
}

int	ITPObject::DelSocketFromIOCP(SOCKET socket, CPerHandleData *pPerHandleData)
{
	CReadWriteMutexLock lock(g_csDealSock);

	if (pPerHandleData != NULL)
	{
		g_lstDealSocks.remove(pPerHandleData);
	}

	lock.Unlock();

	if (socket != INVALID_SOCKET)
	{
		shutdown(socket, SD_BOTH);
		closesocket(socket);
	}

	return 1;
}

int ITPObject::PostRecvToIOCP(SOCKET socket, CPerIoData *pPerIoRecv)
{
	if (g_hCompletionPort == NULL || socket == INVALID_SOCKET || pPerIoRecv == NULL)
	{
		return -1;
	}

	DWORD dwBytesReturn = 0;
	DWORD dwFlags = 0;

	ZeroMemory(&pPerIoRecv->m_ol, sizeof(OVERLAPPED));
	
	if (pPerIoRecv->m_operationMode == opMode_tcp)
	{
		int ret = WSARecv(socket, &pPerIoRecv->m_wsabuf, 1, &dwBytesReturn, &dwFlags, &pPerIoRecv->m_ol, NULL);
		if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			// post failed
			return -1;
		}
	}
	else if (pPerIoRecv->m_operationMode == opMode_udp)
	{
		int ret = WSARecvFrom(socket, &pPerIoRecv->m_wsabuf, 1, &dwBytesReturn, &dwFlags, (struct sockaddr*)&pPerIoRecv->m_remoteAddr, &pPerIoRecv->m_nFromLen, &pPerIoRecv->m_ol, NULL);
		if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			// post failed
			return -1;
		}
	}
	else
	{
		return -1;
	}

	return 1;
}

int	ITPObject::PostSendToIOCP(SOCKET socket, CPerIoData *pPerIoSend)
{
	if (g_hCompletionPort == NULL || socket == INVALID_SOCKET || pPerIoSend == NULL)
	{
		return -1;
	}

	DWORD dwSendBytes = 0;

	ZeroMemory(&pPerIoSend->m_ol, sizeof(OVERLAPPED));

	if (pPerIoSend->m_operationMode == opMode_tcp)
	{
		int nRet = WSASend(socket, &pPerIoSend->m_wsabuf, 1, &dwSendBytes, 0, &pPerIoSend->m_ol, NULL);
		if (nRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) 
		{
			// post failed
			return -1;
		}
	}
	else if (pPerIoSend->m_operationMode == opMode_udp)
	{
		int to_len = sizeof(struct sockaddr_in);
		int nRet = WSASendTo(socket, &pPerIoSend->m_wsabuf, 1, &dwSendBytes, 0, (struct sockaddr*)&pPerIoSend->m_remoteAddr, to_len, &pPerIoSend->m_ol, NULL);
		if (nRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) 
		{
			// post failed
			return -1;
		}
	}
	else
	{
		return -1;
	}

	return 1;
}

int ITPObject::Create(opMode type)
{
	if (m_socket != INVALID_SOCKET)
	{
		return -1;
	}

	if (type == opMode_tcp)
	{
		m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	else if (type == opMode_udp)
	{
		m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}
	
	if (m_socket == INVALID_SOCKET)
	{
		return -1;
	}

	//	绑定套接字端口
	if (m_localPort != 0)
	{
		struct sockaddr_in local_addr;
		memset(&local_addr, 0, sizeof(struct sockaddr_in));
		local_addr.sin_family = AF_INET;
		local_addr.sin_port = m_localPort;
		local_addr.sin_addr.s_addr = m_localIp;

		if (INVALID_SOCKET == bind(m_socket, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)))
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			return -1;
		}
	}
	else if (type == opMode_udp)
	{
		//	随机生成端口号，直到绑定成功为止
		char szLocalIp[32] = {0};
		long nLocalPort = GetNewPort();
		BOOL bIsBind = FALSE;

		int ret = GetLocalIP(szLocalIp);
		if (ret < 0)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			return -1;
		}
		m_localIp = inet_addr(szLocalIp);
		m_localPort = htons(nLocalPort);

		struct sockaddr_in local_addr;
		memset(&local_addr, 0, sizeof(struct sockaddr_in));
		local_addr.sin_family = AF_INET;
		local_addr.sin_port = m_localPort;
		local_addr.sin_addr.s_addr = m_localIp;

		for (int n = 0; n < 10000; n++)
		{
			if (INVALID_SOCKET != bind(m_socket, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)))
			{
				bIsBind = TRUE;
				break;
			}

			nLocalPort = GetNewPort();
			m_localPort = htons(nLocalPort);
			local_addr.sin_port = m_localPort;
		}

		if (!bIsBind)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			return -1;
		}
	}

	//	设为非阻塞模式
	unsigned long flags = 1;
	int nRet = ioctlsocket(m_socket, FIONBIO, &flags);
	if (nRet != 0)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return -1;
	}

	if(m_recvBuffSize > 0)
		setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&m_recvBuffSize, sizeof(int));
	if(m_sendBuffSize > 0)
		setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char*)&m_sendBuffSize, sizeof(int));

// 	
// 	int bNodelay = 1; 
// 	int err; 
// 	err = setsockopt( m_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&bNodelay, sizeof(bNodelay));//不采用延时算法
// 
// 	int sndbuf = 0;        /* Send buffer size */
// 	int rcvbuf = 0;        /* Receive buffer size */
// 	int sndout = 0;   /* send timeout */
// 	int rcvout = 0;   /* receive timeout */
// 	int optlen;        /* Option length */
// 	
// 	sndbuf = 10000;    /* Send buffer size */
// 	optlen = sizeof(sndbuf);
// 	setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&sndbuf,optlen);
// 
// 	sndout = 10000;
// 	optlen = sizeof(sndout);
// 	setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&sndout,optlen);
// 

	return 1;
}

//////////////////////////////////////////////////////////////////////////

int ITPObject::SetSocketBufferSize(TP_DATA_TYPE type, int size)
{
	int nRet = 1;

	if(size >= 0 && size <= 65535)
	{
		switch(type)
		{
		case TP_SEND:
			m_sendBuffSize = size;
			break;
		case TP_RECEIVE:
			m_recvBuffSize = size;
			break;
		default:
			nRet = -1;
		}
	}
	else
	{
		nRet = -2;
	}

	return nRet;
}

int ITPObject::GetSocketBufferSize(TP_DATA_TYPE type)
{
	int nRet = -1;
	switch(type)
	{
	case TP_SEND:
		{
			nRet = m_sendBuffSize;
		}
		break;
	case TP_RECEIVE:
		{
			nRet = m_recvBuffSize;
		}
		break;
	default:
		break;
	}

	return nRet;
}

int ITPObject::SetSelectTimeout(long sec, long usec)
{
	int nRet = 1;

	if(sec >=0 && usec >= 0)
	{
		m_timeout_sec = sec;
		m_timeout_usec = usec;
	}
	else
	{
		nRet = -1;
	}

	return nRet;
}

int ITPObject::SetIsReconnect(int nEnable)
{
	if (nEnable != 0)
	{
		m_bReconnEn = TRUE;
	}
	else
	{
		m_bReconnEn = FALSE;
	}

	return 1;
}

int	ITPObject::SetIsDetectDisconn(int nEnable)
{
	if (nEnable != 0)
	{
		m_bDetectDisconn = TRUE;
	}
	else
	{
		m_bDetectDisconn = FALSE;
	}

	return 1;
}

int	ITPObject::SetMaxSendQueue(int size)
{
	int nRet = 1;

	if (size >0 && size < 65535)
	{
		m_nSendQueueSize = size;
	}
	else
	{
		nRet = -1;
	}

	return nRet;
}

int	ITPObject::GetSocketBindInfo(char *szIp, int &nPort)
{
	if (szIp == NULL)
	{
		return -1;
	}

	in_addr localIp = {0};
	localIp.s_addr = m_localIp;

	strcpy(szIp, inet_ntoa(localIp));
	nPort = ntohs(m_localPort);

	return 1;
}

int	ITPObject::GetLocalIP(char *szIp)
{
	if (szIp == NULL)
	{
		return -1;
	}

	int nRet = -1;
	char sHost[228];
	PHOSTENT hostinfo;

	gethostname(sHost,228);
	if((hostinfo = gethostbyname(sHost)) != NULL)
	{
		if (hostinfo->h_addr_list[0])
		{
			strcpy(szIp, inet_ntoa(*(struct in_addr *)hostinfo->h_addr_list[0]));
			nRet = 1;
		}
	}

	return nRet;
}

long ITPObject::GetNewClientId()
{
	return InterlockedIncrement(&g_clientId_new);
}

long ITPObject::GetNewPort()
{
	InterlockedIncrement(&g_port_new);
	if (g_port_new >= 20000)
	{
		g_port_new = 10000;
	}

	return g_port_new;
}