/*
- Completion Port TCP socket class 
- TcpSocket_CP.cpp
- by lin_jianyan 
- 2007_09_12
*/

#include "TcpSocket_CP.h"
#include <stdio.h>

//global vars
static BOOL		g_bInited = FALSE;
static int		g_iSockNum = 0;

static HANDLE	g_hCompletionPort = NULL;
static int		g_iNumberOfThreads = 0;
static HANDLE*	g_hThreads = NULL;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

typedef enum _opType
{
	opType_none = 0,
	opType_recv,
	opType_send,
}opType;

class CPerIoData
{
public:
	CPerIoData(): m_lLife(1)
	{
		memset(&m_ol, 0, sizeof(OVERLAPPED)); 
		m_wsabuf.buf = NULL;
		m_wsabuf.len = 0;
		m_operationType = opType_none;
		m_lTotalBytes = 0;
		m_lSentBytes = 0;
		m_pcs = NULL;
	}
	
	virtual ~CPerIoData()
	{
	}

	int AddRef()
	{
		InterlockedIncrement(&m_lLife);
		return m_lLife;
	}
	
	int DecRef()
	{		
		if (InterlockedDecrement(&m_lLife) == 0)
		{
			delete this;
			return 0;
		}
		
		return m_lLife;
	}
	
	OVERLAPPED	m_ol;
	WSABUF		m_wsabuf;
	opType		m_operationType;
	LONG		m_lTotalBytes;	//需要发送的总长度
	LONG		m_lSentBytes;	//已发送字节
	LONG		m_lLife;
	CRITICAL_SECTION*	m_pcs;
};

class CPerHandleData
{
public:
	CPerHandleData(): m_lLife(1)
	{
		m_socket = INVALID_SOCKET;
		m_hSndEvt = NULL;
		m_fDisCB = NULL;	
		m_fDataCB = NULL;
		m_dwUD = 0;	
		m_pBuf = NULL;
		m_iBufLen = 0;
		m_pcs = NULL;
	}

	virtual ~CPerHandleData()
	{
		if (m_pBuf != NULL)
		{
			delete[] m_pBuf;
		}
	}
	
	int AddRef()
	{
		InterlockedIncrement(&m_lLife);
		return m_lLife;
	}
	
	int DecRef()
	{
		if (InterlockedDecrement(&m_lLife) == 0)
		{
			delete this;
			return 0;
		}
		
		return m_lLife;
	}
	
	SOCKET				m_socket;
	HANDLE				m_hSndEvt;
    fDisconnectCallBack	m_fDisCB;	
    fTcpDataCallBack	m_fDataCB;
	DWORD				m_dwUD;	
	char*				m_pBuf;
	int					m_iBufLen;
	LONG				m_lLife;
	CRITICAL_SECTION*	m_pcs;
};
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
int InitCompletionPort();
int FintCompletionPort();
DWORD WINAPI RoutineThread(LPVOID completionPortID);


CTcpSocket_CP::CTcpSocket_CP()
{
	//initialize network environment
	if (!g_bInited)
	{
		if (InitializeNetwork() < 0)
		{
			throw -1;
		}
		g_bInited = TRUE;
	}
	
	//count socket numbers
	if (InterlockedIncrement((long*)&g_iSockNum) == 1)
	{
		if (InitCompletionPort() < 0)
		{
			throw -1;
		}
	}
	
	//initialize members
	m_cltSocket = INVALID_SOCKET;
	memset(&m_siCltAddr, 0, sizeof(SOCKADDR_IN));
	m_bEnableTimeWait = TRUE;
	m_pPerIoRecv = NULL;
//	m_pPerIoSend = NULL;
	m_pPerHandleData = NULL;
	m_pBuf = NULL;
	m_iBufSize = DEFAULT_BUFFER_SIZE;
	m_hSendEvent = NULL;
//	m_iSendTimeout = 2000;
	m_fDisconnectCallBack = NULL;
	m_fTcpDataCallBack = NULL;
	m_dwCltUserData = 0;
	
	m_svrSocket = INVALID_SOCKET;
	memset(&m_siSvrAddr, 0, sizeof(SOCKADDR_IN));
	m_hListenThread = NULL;	
	m_fListenCallBack = NULL;
	m_dwSvrUserData = 0;
	m_hListenExitEvent = NULL;
	InitializeCriticalSection(&m_csSend);
}

CTcpSocket_CP::~CTcpSocket_CP()
{		
	Disconnect();
	StopListen();

	//if socket num is zero, clean up network environment
	if (InterlockedDecrement((long*)&g_iSockNum) <= 0)
	{
		if (ClearNetwork() < 0)
		{
			throw -1;
		}
		//g_bInited = FALSE;
		//加上去会有问题
		//g_iSockNum = 0;
	}
	DeleteCriticalSection(&m_csSend);
}

int CTcpSocket_CP::InitializeNetwork()
{
	WSADATA wsa = {0};
	int ret = WSAStartup(MAKEWORD(2,2), &wsa);
	if (ret != 0)
	{
#ifdef DEBUG
		OutputDebugString(" WSAStartup() Failed!\n");
#endif
		return -1;
	}
	return 0;
}


DWORD WINAPI RoutineThread(LPVOID completionPortID)
{
	HANDLE hCompletionPort = (HANDLE)completionPortID;
	
	CPerHandleData* pPerHandleData;
	DWORD dwBytesTransferred;
	CPerIoData* pPerIoData;
	LPOVERLAPPED pOl;
	DWORD dwFlags;
	int iRet;
	BOOL bRet;
	CRITICAL_SECTION* pcsIoData;
	CRITICAL_SECTION* pcsHandleData;
	
	while (TRUE)
	{
		bool bNetnameDeleted = false;
		bRet = GetQueuedCompletionStatus(hCompletionPort, &dwBytesTransferred, (LPDWORD)&pPerHandleData, &pOl, INFINITE);
		if (!bRet) 
		{
			if (GetLastError() == ERROR_INVALID_HANDLE) 
			{
				//the IOCP handle probably has been closed
				break;
			}
			else
			{
				DWORD dw = GetLastError();
				if (ERROR_NETNAME_DELETED == dw || ERROR_OPERATION_ABORTED == dw)
				{
					pPerIoData = CONTAINING_RECORD(pOl, CPerIoData, m_ol);
					pcsIoData = pPerIoData->m_pcs;
					pcsHandleData = pPerHandleData->m_pcs;
					EnterCriticalSection(pcsHandleData);
					EnterCriticalSection(pcsIoData);
// 					pPerIoData->DecRef();
// 					pPerHandleData->DecRef();
					goto _CONTINUE_DEC_IOREF;
				}
				else
				{
					//OutputDebugString("unexpected error GetQueuedCompletionStatus() !\n");
				}
#ifdef DEBUG
				OutputDebugString("Routine thread :: GetQueuedCompletionStatus() Failed ! \n ");
#endif
				continue;
			}
		}

		if (NULL == pPerHandleData) 
		{
			//receive command to exit thread
			break;
		}

		pPerIoData = CONTAINING_RECORD(pOl, CPerIoData, m_ol);

		pcsIoData = pPerIoData->m_pcs;
		pcsHandleData = pPerHandleData->m_pcs;
		
//		OutputDebugString("Entering critical \n");
		EnterCriticalSection(pcsHandleData);
		EnterCriticalSection(pcsIoData);
//		OutputDebugString("Entered critical \n");

		if (dwBytesTransferred == 0 
			&&(pPerIoData->m_operationType == opType_recv 
			||pPerIoData->m_operationType == opType_send))
		{
			//disconnected by remote host or something
			if (pPerHandleData->m_fDisCB != NULL && !bNetnameDeleted)
			{
				pPerHandleData->m_fDisCB(pPerHandleData->m_dwUD);
			}
			goto _CONTINUE_DEC_IOREF;
		}
		
		if (pPerIoData->m_operationType == opType_recv)
		{
			//deal with data
			//Sleep(500);
			if (pPerHandleData->m_fDataCB != NULL)
			{
			//	OutputDebugString("Enter data call back \n");
				pPerHandleData->m_fDataCB(pPerIoData->m_wsabuf.buf, dwBytesTransferred, pPerHandleData->m_dwUD);
			//	OutputDebugString("Leave data call back \n");
			}
			//OutputDebugString("start sleep test\n");
			//Sleep(5000);
			//OutputDebugString("end sleep test\n");
			//deal with data end
			
			//dec reference
			if (pPerIoData->DecRef() <= 0)
			{
				//deleted , or error occured
				LeaveCriticalSection(pcsIoData);
				DeleteCriticalSection(pcsIoData);
				delete pcsIoData;
				pcsIoData = NULL;
				goto _CONTINUE_DEC_HANDLREF;
			}
			//pPerIoData->AddRef();
			//pPerHandleData->AddRef();
			//post another recv request
			ZeroMemory(&pPerIoData->m_ol, sizeof(OVERLAPPED));
			dwFlags = 0;
			iRet = WSARecv(pPerHandleData->m_socket, &pPerIoData->m_wsabuf, 1, &dwBytesTransferred, &dwFlags, &(pPerIoData->m_ol), NULL);
			if (iRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				DWORD dd = WSAGetLastError();
#ifdef _DEBUG
				OutputDebugString("Routine thread :: WSARecv() Failed ! \n ");
#endif
			}
			else
			{
				pPerIoData->AddRef();
				pPerHandleData->AddRef();
			}
			LeaveCriticalSection(pcsIoData);
			goto _CONTINUE_DEC_HANDLREF;
		}
		else if (pPerIoData->m_operationType == opType_send)
		{
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
				iRet = WSASend(pPerHandleData->m_socket, &pPerIoData->m_wsabuf, 1, &dwSendBytes, 0, &pPerIoData->m_ol, NULL);
				if (iRet == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) 
				{
					DWORD dd = WSAGetLastError();
					delete[] pPerIoData->m_wsabuf.buf;
				}
				else
				{
					pPerIoData->AddRef();
					pPerHandleData->AddRef();
				}

				goto _CONTINUE_DEC_IOREF;
			}
			
			delete[] pPerIoData->m_wsabuf.buf;
			//ready to send next wave
			SetEvent(pPerHandleData->m_hSndEvt);
			goto _CONTINUE_DEC_IOREF;
		}
		else if (pPerIoData->m_operationType == opType_none)
		{
			//closed
			goto _CONTINUE_DEC_IOREF;
		}

_CONTINUE_DEC_IOREF:
		iRet = pPerIoData->DecRef();
		LeaveCriticalSection(pcsIoData);
		if (iRet <= 0)
		{
			DeleteCriticalSection(pcsIoData);
			delete pcsIoData;
			pcsIoData = NULL;
		}
_CONTINUE_DEC_HANDLREF:
		iRet = pPerHandleData->DecRef();
		LeaveCriticalSection(pcsHandleData);
		if (iRet <= 0)
		{
			DeleteCriticalSection(pcsHandleData);
			delete pcsHandleData;
			pcsHandleData = NULL;
		}		
		continue;
	}

	return 0;
}

int InitCompletionPort()
{
	if (g_hCompletionPort != NULL)
	{
		//completion port already created
		return 0;
	}
	
	g_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == g_hCompletionPort) 
	{
#ifdef DEBUG
		OutputDebugString(" CreateIoCompletionPort() Failed!\n");
#endif
		return -1;
	}
	
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	//create 2 threads for each processor and 4 extra
	g_iNumberOfThreads = systemInfo.dwNumberOfProcessors*2 + 4; 
	
	if (NULL == g_hThreads)
	{
		g_hThreads = new HANDLE[g_iNumberOfThreads];
		if (NULL == g_hThreads)
		{
			return -1;
		}
	}

	for (int i=0; i<g_iNumberOfThreads; i++)
	{
		g_hThreads[i] = CreateThread(NULL, 0, RoutineThread, g_hCompletionPort, 0, NULL);
		if (NULL == g_hThreads[i])
		{
#ifdef DEBUG
			OutputDebugString(" CreateThread() Failed!\n");
#endif
			return -1;
		}
		//SetThreadPriority(g_hThreads[i], REALTIME_PRIORITY_CLASS);
		//CloseHandle(g_hThreads[i]);
	}
	
	return 0;
}

int FintCompletionPort()
{
	return 0;	//为了防止频繁地线程退出

	if (NULL == g_hCompletionPort)
	{
		return 0;
	}
	
	for (int i=0; i<g_iNumberOfThreads; i++)
	{
		//to end each thread
		PostQueuedCompletionStatus(g_hCompletionPort, 0, 0, 0);	
	}

	DWORD dw = WaitForMultipleObjects(g_iNumberOfThreads, g_hThreads, TRUE, INFINITE);

	CloseHandle(g_hCompletionPort);
	
	for (i=0; i<g_iNumberOfThreads; i++)
	{
		//to end each thread
		CloseHandle(g_hThreads[i]);	
	}
	
	g_hCompletionPort = NULL;
	g_iNumberOfThreads = 0;
//	delete[] g_hThreads;
//	g_hThreads = NULL;
	
	return 0;
}

int CTcpSocket_CP::ClearNetwork()
{
	FintCompletionPort();

//	if (WSACleanup() == SOCKET_ERROR)
//	{
//#ifdef DEBUG
//		OutputDebugString(" WSACleanup() Failed!\n");
//#endif
//		return -1;
//	}
	return 0;
}

int CTcpSocket_CP::Create(int type)
{
	if (m_cltSocket != INVALID_SOCKET || m_svrSocket != INVALID_SOCKET)
	{
		//already created
		return -1;
	}

	if (0 == type)
	{
		m_svrSocket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
		if (m_svrSocket == INVALID_SOCKET)
		{
			return -1;
		}	
	}
	else if (1 == type)
	{
		m_cltSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_cltSocket == INVALID_SOCKET)
		{
			return -1;
		}

		if (!m_bEnableTimeWait)
		{
			BOOL  bDontLinger = FALSE; 
			setsockopt(m_cltSocket, SOL_SOCKET, SO_DONTLINGER, (const char*)&bDontLinger, sizeof(BOOL));
		}

		return CreateClientEnvironment();
	}
	
    return 0;
}

int	CTcpSocket_CP::CreateClientEnvironment()
{
	if (m_iBufSize <= 0)
	{
		goto e_out;
	}
	
	m_pBuf = new char[m_iBufSize];
	if (NULL == m_pBuf)
	{
		goto e_out;
	}
	
	m_hSendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL == m_hSendEvent) 
	{
		goto e_out;
	}
	
	m_pPerIoRecv = new CPerIoData;
	if (NULL == m_pPerIoRecv)
	{
		goto e_out;
	}

	m_pPerIoRecv->m_pcs = new CRITICAL_SECTION;
	if (NULL == m_pPerIoRecv->m_pcs)
	{
		goto e_out;
	}

	InitializeCriticalSection(m_pPerIoRecv->m_pcs);

//	m_pPerIoSend = new CPerIoData;
//	if (NULL == m_pPerIoSend)
//	{
//		goto e_out;
//	}

	m_pPerHandleData = new CPerHandleData;
	if (NULL == m_pPerHandleData)
	{
		goto e_out;
	}
	
	m_pPerHandleData->m_pcs = new CRITICAL_SECTION;
	if (NULL == m_pPerHandleData->m_pcs)
	{
		goto e_out;
	}
	
	InitializeCriticalSection(m_pPerHandleData->m_pcs);
	m_pPerHandleData->m_socket = m_cltSocket;
	m_pPerHandleData->m_fDataCB = m_fTcpDataCallBack;
	m_pPerHandleData->m_fDisCB = m_fDisconnectCallBack;
	m_pPerHandleData->m_dwUD = m_dwCltUserData;
	m_pPerHandleData->m_hSndEvt = m_hSendEvent;
	m_pPerHandleData->m_pBuf = m_pBuf;
	m_pPerHandleData->m_iBufLen = m_iBufSize;
	
//	char tmp[100];
//	sprintf(tmp, "created a new handle: m_pPerHandleData=%d, dataFunc = %d, pcs=%d\n", m_pPerHandleData, m_pPerHandleData->m_fDataCB, m_pPerHandleData->m_pcs);
//	OutputDebugString(tmp);

	return 0;
e_out:
	if (m_pBuf != NULL)
	{
		delete[] m_pBuf;
		m_pBuf = NULL;
	}
	
	if (m_pPerIoRecv != NULL)
	{
		if (m_pPerIoRecv->m_pcs != NULL) 
		{
			DeleteCriticalSection(m_pPerIoRecv->m_pcs);
			delete m_pPerIoRecv->m_pcs;
			m_pPerIoRecv->m_pcs = NULL;
		}
		delete m_pPerIoRecv;
		m_pPerIoRecv = NULL;
	}
		
	if (m_hSendEvent != NULL)
	{
		CloseHandle(m_hSendEvent);
		m_hSendEvent = NULL;
	}
	
	if (m_pPerHandleData != NULL)
	{
		if (m_pPerHandleData->m_pcs != NULL) 
		{
			DeleteCriticalSection(m_pPerHandleData->m_pcs);
			delete m_pPerHandleData->m_pcs;
			m_pPerHandleData->m_pcs = NULL;
		}
		delete m_pPerHandleData;
		m_pPerHandleData = NULL;
	}

	return -1;
}


int	CTcpSocket_CP::ClearClientEnvironment()
{
	int iRet = 0;

	//clear memory	
	m_pBuf = NULL;
	
	//	FintCompletionPort();
	
	if (m_pPerIoRecv != NULL)
	{
		CRITICAL_SECTION* pcs = m_pPerIoRecv->m_pcs;
		EnterCriticalSection(pcs);
		m_pPerIoRecv->m_operationType = opType_none;
		
		iRet = m_pPerIoRecv->DecRef();
		m_pPerIoRecv = NULL;

		LeaveCriticalSection(pcs);

		if (iRet <= 0)
		{
			DeleteCriticalSection(pcs);
			delete pcs;
		}
	}
	
	if (m_pPerHandleData != NULL)
	{
		CRITICAL_SECTION* pcs = m_pPerHandleData->m_pcs;
//		char tmp[100];
//		sprintf(tmp, "disconnecting: m_pPerHandleData=%d, entering pcs\n", m_pPerHandleData);
//		OutputDebugString(tmp);
//
		EnterCriticalSection(pcs);
		m_pPerHandleData->m_fDisCB = NULL;
		m_pPerHandleData->m_fDataCB = NULL;
		m_pPerHandleData->m_dwUD = NULL;
		
		iRet = m_pPerHandleData->DecRef();
		m_pPerHandleData = NULL;
		
//		sprintf(tmp, "disconnecting: pcs=%d, done\n", pcs);
//		OutputDebugString(tmp);
		LeaveCriticalSection(pcs);
		

		if (iRet <= 0)
		{
			DeleteCriticalSection(pcs);
			delete pcs;
		}
	}
	
	if (m_hSendEvent != NULL)
	{
		CloseHandle(m_hSendEvent);
		m_hSendEvent = NULL;
	}

	return 0;
}

int CTcpSocket_CP::ConnectHost(const char *szIp, int iPort, int iTimeOut/* = 1000*/)
{
	if (INVALID_SOCKET == m_cltSocket)
	{
		//not inited
		return -1;
	}
	
	if (strlen(szIp) >= MAX_IPADDR_LEN || iPort < 0)
	{
		//support only ipv4!
		return -1;
	}
	
	int ret = 0;
	
	memset(&m_siCltAddr, 0, sizeof(SOCKADDR_IN));
	
	m_siCltAddr.sin_family = AF_INET;
    m_siCltAddr.sin_port = htons(iPort);
    m_siCltAddr.sin_addr.S_un.S_addr = inet_addr(szIp);
	
    DWORD dwFlag = 1;
    ioctlsocket(m_cltSocket, FIONBIO, &dwFlag);
	
	ret = connect(m_cltSocket, (SOCKADDR*)&m_siCltAddr, sizeof(SOCKADDR));
    if (SOCKET_ERROR == ret && WSAGetLastError() != WSAEWOULDBLOCK)
    {
		DWORD dw = WSAGetLastError();
		//failed
		if (10037 != dw)
		{
			goto e_out;
		}
    }
	
	timeval tv;
	tv.tv_sec = iTimeOut/1000;
	tv.tv_usec = iTimeOut%1000;
	
	FD_SET  fdwrite;
	FD_ZERO(&fdwrite);
	FD_SET(m_cltSocket, &fdwrite);
	
	ret = select(0, NULL, &fdwrite, NULL, iTimeOut == 0 ? NULL : &tv);
	if (SOCKET_ERROR == ret)
	{
		//select failed
		goto e_out;
	}
	
	if (!FD_ISSET(m_cltSocket, &fdwrite))
	{
		//not connected
		goto e_out;
	}
	
	//Initialize CompletionPort
	if (NULL == g_hCompletionPort) 
	{
		ret = InitCompletionPort();
		if (ret < 0)
		{
			goto e_out;
		}
	}
	
	//add this socket into completion port
	CreateIoCompletionPort((HANDLE)m_cltSocket, g_hCompletionPort, (DWORD)m_pPerHandleData, 0);

	memset(&m_pPerIoRecv->m_ol, 0, sizeof(OVERLAPPED));

	m_pPerIoRecv->m_wsabuf.buf = m_pBuf;
	m_pPerIoRecv->m_wsabuf.len = m_iBufSize;
	m_pPerIoRecv->m_operationType = opType_recv;
	DWORD dwBytesReturn;
	DWORD dwFlags;
	dwFlags = 0;
	
	
	//post a recv request
	m_pPerIoRecv->AddRef();
	m_pPerHandleData->AddRef();
	ret = WSARecv(m_cltSocket, &m_pPerIoRecv->m_wsabuf, 1, &dwBytesReturn, &dwFlags, &m_pPerIoRecv->m_ol, NULL);
	if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		DWORD dd = WSAGetLastError();
#ifdef DEBUG
		OutputDebugString("Routine thread :: WSARecv() Failed ! \n ");
#endif
		goto e_out;
	}
	
	return 0;
e_out:
	return -1;
}

int CTcpSocket_CP::Disconnect()
{
    if (m_cltSocket == INVALID_SOCKET)
    {
		return -1;
	}
	
	int result = ClearClientEnvironment();
	
	//close socket
	shutdown(m_cltSocket, SD_BOTH);
	closesocket(m_cltSocket);
	m_cltSocket = INVALID_SOCKET;

	return result;
}

int CTcpSocket_CP::SendData(char *pbuf, int nlen)
{
	if (INVALID_SOCKET == m_cltSocket || NULL == pbuf || nlen <= 0)
	{
		return -1;
	}

	int ret = 0;

	CPerIoData*	pPerIoSend = new CPerIoData;
	if (NULL == pPerIoSend)
	{
		return -1;
	}

	char *pSendBuf = new char[nlen];
	if (NULL == pSendBuf)
	{
		pPerIoSend->DecRef();
		return -1;
	}
	memcpy(pSendBuf, pbuf, nlen);
	memset(&pPerIoSend->m_ol, 0, sizeof(OVERLAPPED));
	pPerIoSend->m_wsabuf.buf = pSendBuf;
	pPerIoSend->m_wsabuf.len = nlen;
	pPerIoSend->m_operationType = opType_send;
	pPerIoSend->m_lSentBytes = 0;
	pPerIoSend->m_lTotalBytes = nlen;
	pPerIoSend->m_pcs = new CRITICAL_SECTION;
	if (NULL == (pPerIoSend->m_pcs))
	{
		delete[] pSendBuf;
		pPerIoSend->DecRef();
		return -1;;
	}
	InitializeCriticalSection(pPerIoSend->m_pcs);

	EnterCriticalSection(&m_csSend);
	
	DWORD dwSendBytes = 0;
	
	m_pPerHandleData->AddRef();
	ret = WSASend(m_cltSocket, &pPerIoSend->m_wsabuf, 1, &dwSendBytes, 0, &pPerIoSend->m_ol, NULL);
	if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) 
	{
		DWORD dd = WSAGetLastError();
		LeaveCriticalSection(&m_csSend);
		delete[] pSendBuf;
		pPerIoSend->DecRef();
		m_pPerHandleData->DecRef();
		return -1;
	}

	LeaveCriticalSection(&m_csSend);
	
	return 0;
}

int CTcpSocket_CP::SetRecvBufferSize(int iSize)
{
	if (m_cltSocket != INVALID_SOCKET || m_svrSocket != INVALID_SOCKET)
	{
		//already created
		return -1;
	}

	if (iSize <= 0)
	{
		return -1;
	}

	m_iBufSize = iSize;

	return 0;
}


int CTcpSocket_CP::SetCallBack(fDisconnectCallBack disCallBack, 
							 fTcpDataCallBack dataCallBack,
							 DWORD dwUserData)
{
	m_fDisconnectCallBack = disCallBack;
	m_fTcpDataCallBack = dataCallBack;
    m_dwCltUserData = dwUserData;

	if (m_pPerHandleData != NULL)
	{
		EnterCriticalSection(m_pPerHandleData->m_pcs);
		m_pPerHandleData->m_fDisCB = disCallBack;
		m_pPerHandleData->m_fDataCB = dataCallBack;
		m_pPerHandleData->m_dwUD = dwUserData;
		LeaveCriticalSection(m_pPerHandleData->m_pcs);
	}

	return 0;
}

int CTcpSocket_CP::EnableTimeWaitState(BOOL bEnableTimeWait)
{
	m_bEnableTimeWait = bEnableTimeWait;
	return 0;
}

DWORD WINAPI ListenThreadFunc_CP(void *parm)
{
	CTcpSocket_CP* pThis = (CTcpSocket_CP*)parm;
	return pThis->ListenSocket();
}

int CTcpSocket_CP::StartListen(const char *ip, int port, fListenCallBack listenFunc, DWORD dwUserData)
{
	if (INVALID_SOCKET == m_svrSocket)
	{
		return -1;
	}
	
	m_siSvrAddr.sin_family = AF_INET;
    m_siSvrAddr.sin_port = htons(port);
    m_siSvrAddr.sin_addr.S_un.S_addr = (ip==NULL) ? INADDR_ANY : inet_addr(ip);
	
	m_fListenCallBack = listenFunc;
	m_dwSvrUserData = dwUserData;
	
	int ret = 0;
	
	ret = bind(m_svrSocket, (SOCKADDR*)&m_siSvrAddr, sizeof(SOCKADDR_IN));
	if (ret == SOCKET_ERROR)
	{
		return -1;
	}
	
	DWORD dwThreadID = 0;
	m_hListenThread = CreateThread(NULL, 0, ListenThreadFunc_CP, this, 0, &dwThreadID);
	if (NULL == m_hListenThread)
	{
		return -1;
	}
	
	m_hListenExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (NULL == m_hListenExitEvent) 
	{
		return -1;
	}
	
	return 0;
}

int CTcpSocket_CP::StopListen()
{
	int result = 0;
	if (INVALID_SOCKET == m_svrSocket || NULL == m_hListenExitEvent)
	{
		result = -1;
	}
	
	int ret = 0;
	shutdown(m_svrSocket, SD_BOTH);
	ret = closesocket(m_svrSocket);
	if (SOCKET_ERROR == ret) 
	{
		result = -1;
	}
	m_svrSocket = INVALID_SOCKET;
	
	SetEvent(m_hListenExitEvent);
	
	DWORD dwRet = WaitForSingleObject(m_hListenThread, 1000);
	if (WAIT_OBJECT_0 != dwRet)
	{
		TerminateThread(m_hListenThread, -1);
	}
	
	CloseHandle(m_hListenThread);
	m_hListenThread = NULL;
	
	CloseHandle(m_hListenExitEvent);
	m_hListenExitEvent = NULL;
	
	return result;
}

int CTcpSocket_CP::ListenSocket()
{
	if (INVALID_SOCKET == m_svrSocket || NULL == m_fListenCallBack)
	{
		return -1;
	}
	
	int ret = 0;
	
	ret = listen(m_svrSocket, 5);
	if (SOCKET_ERROR == ret)
	{
		return -1;
	}
	
	SOCKADDR_IN addr = {0};
	int		 addrLen = sizeof(SOCKADDR_IN);
	while (WaitForSingleObject(m_hListenExitEvent, 0) != WAIT_OBJECT_0) 
	{
		FD_SET  fdread;
		FD_ZERO(&fdread);
		FD_SET(m_svrSocket, &fdread);
		
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 300;
		
		ret = select(0, &fdread, NULL, NULL, &tv);
		if (SOCKET_ERROR == ret)
		{
			return -1;
		}
		
		if (FD_ISSET(m_svrSocket, &fdread))
		{
			SOCKET cltsock = accept(m_svrSocket, (SOCKADDR*)&addr, &addrLen);
			if (INVALID_SOCKET == cltsock)
			{
				//accept failed, not supposed to happen here
				continue;
			}
			
			CTcpSocket_CP *newSocket = new CTcpSocket_CP;
			if (NULL == newSocket)
			{
				return -1;
			}

			m_fListenCallBack(this, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port),
				newSocket, m_dwSvrUserData);

			newSocket->SetSocket(cltsock, &addr, addrLen);
		}
	}
	
	return 0;
}

int CTcpSocket_CP::SetSocket(SOCKET s, SOCKADDR_IN *addr, int addrLen)
{
	if (NULL == addr || addrLen != sizeof(SOCKADDR_IN))
	{
		return -1;
	}
	
	//self-evaluating not allowed!!
	if (s == m_cltSocket)
	{
		return -1;
	}
	
	int ret = 0;

	if (m_cltSocket != INVALID_SOCKET)
	{
		closesocket(m_cltSocket);
	}
	
	m_cltSocket = s;
	memcpy(&m_siCltAddr, addr, sizeof(SOCKADDR_IN));
	
	if (!m_bEnableTimeWait)
	{
		BOOL  bDontLinger = FALSE; 
		setsockopt(m_cltSocket, SOL_SOCKET, SO_DONTLINGER, (const char*)&bDontLinger, sizeof(BOOL));
	}
	
	if (CreateClientEnvironment() < 0)
	{
		goto e_out;
	}

	//Initialize CompletionPort
	if (NULL == g_hCompletionPort) 
	{
		ret = InitCompletionPort();
		if (ret < 0)
		{
			goto e_out;
		}
	}
	
	//add this socket into completion port
	CreateIoCompletionPort((HANDLE)m_cltSocket, g_hCompletionPort, (DWORD)m_pPerHandleData, 0);
	
	memset(&m_pPerIoRecv->m_ol, 0, sizeof(OVERLAPPED));

	m_pPerIoRecv->m_wsabuf.buf = m_pBuf;
	m_pPerIoRecv->m_wsabuf.len = m_iBufSize;
	m_pPerIoRecv->m_operationType = opType_recv;
	DWORD dwBytesReturn;
	DWORD dwFlags;
	dwFlags = 0;
	
	//post a recv request
	m_pPerIoRecv->AddRef();
	m_pPerHandleData->AddRef();
	ret = WSARecv(m_cltSocket, &m_pPerIoRecv->m_wsabuf, 1, &dwBytesReturn, &dwFlags, &m_pPerIoRecv->m_ol, NULL);
	if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		DWORD dd = WSAGetLastError();
#ifdef DEBUG
		OutputDebugString("Routine thread :: WSARecv() Failed ! \n ");
#endif
		goto e_out;
	}

	return 0;
e_out:
	return -1;
}