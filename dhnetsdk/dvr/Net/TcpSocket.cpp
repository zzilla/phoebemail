// TcpSocket.cpp: implementation of the CTcpSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "../StdAfx.h"
#include "TcpSocket.h"
#include "../ParseString.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#pragma   warning(disable:4355)
CTcpSocket::CTcpSocket() : TPTCPServer(this), TPTCPClient(this)
{
	m_pPacketBuf = NULL;
	m_nBufSize = 0;
	m_nWritePos = 0;
	m_nReadPos = 0;

	m_pDisConnect = NULL;
    m_pReconnect = NULL;
    m_pNormalPacket = NULL;
	m_pRecvPakcet = NULL;
    m_pUserData = NULL;

	m_pListenSockFunc = NULL;
	m_pListenUserData = NULL;
	m_pListenSocket = NULL;

	CreateEventEx(m_hRecEvent, TRUE, FALSE);
#ifdef NETSDK_VERSION_BOGUSSSL
	CreateEventEx(m_hSpecialEvent, FALSE, FALSE);
	m_nSSL = 0;
#endif
}

int	CTcpSocket::CreateRecvBuf(unsigned int nRecvSize)
{
	if (nRecvSize > 0 && m_pPacketBuf == NULL)
	{
		m_pPacketBuf = new unsigned char[nRecvSize];
		if (m_pPacketBuf != NULL)
		{
			m_nBufSize = nRecvSize;
			return 1;
		}
	}

	return -1;
}

CTcpSocket::~CTcpSocket()
{
	CloseEventEx(m_hRecEvent);
	
#ifdef NETSDK_VERSION_BOGUSSSL
	CloseEventEx(m_hSpecialEvent);
#endif

	if (m_pPacketBuf != NULL)
	{
		delete[] m_pPacketBuf;
		m_pPacketBuf = NULL;
	}
	m_nBufSize = 0;
}

int CTcpSocket::InitNetwork()
{
	return TPTCPClient::Startup();
}

int CTcpSocket::ClearNetwork()
{
	return TPTCPClient::Cleanup();
}

/*
 *	摘要：接收数据回调
 */
int CTcpSocket::onData(int nEngineId, int nConnId, unsigned char* data, int nLen)
{
	int nRet = 1;
	if (m_pPacketBuf == NULL)
	{
		return nRet;
	}

#ifdef NETSDK_VERSION_INVERSE
	for (int i = 0; i < nLen; i++)
	{
		unsigned char *p = (unsigned char*)(data + i);
		*p = ~(*p);
	}
#endif

	CReadWriteMutexLock lock(m_csBuffer);

    if (nLen > 0)
    {
        /***********************缓冲数据***********************/
        //	现在当包长大于存储空间时采取丢包的原则
        //	情况可能有：调试中断太久和收到错误数据包

        int nEndPos = nLen + m_nWritePos;
        
		//	如果缓冲区足够缓冲数据
        if (nEndPos <= m_nBufSize)
        {
            memcpy(m_pPacketBuf + m_nWritePos, data, nLen);
            m_nWritePos = m_nWritePos + nLen;
        }
		//	如果缓冲区不足以缓冲数据,从头再来
        else
        {   
			//	整个缓冲区都不足以容纳该数据，一般是因为调试中断太久或错误数据包
            if (nLen + (m_nWritePos-m_nReadPos) >= m_nBufSize)
            {
				nLen = 0;
				nRet = -1;
            }
			else
            {
				memmove(m_pPacketBuf, m_pPacketBuf + m_nReadPos, m_nWritePos - m_nReadPos);
        
				m_nWritePos = m_nWritePos - m_nReadPos;
				m_nReadPos = 0;				

				if (nLen > 0)
				{
					memcpy(m_pPacketBuf + m_nWritePos, data, nLen);
					m_nWritePos = m_nWritePos + nLen;
				}
			}
		}
    }

	lock.Unlock();

	return nRet;
}

/*
 *	摘要：处理数据
 *	返回值：0：忙；1：空闲
 */
int	CTcpSocket::onDealData(int nEngineId, int nConnId, unsigned char* buffer, int nLen)
{
#ifdef NETSDK_VERSION_BOGUSSSL

	if (m_nSSL < 7)
	{
		//SSL
		DealSSL();

		return 1;
	}

#endif

	int nRet = 1;
	if (m_pPacketBuf == NULL)
	{
		return nRet;
	}

	int nPacketLen = GetData(buffer, nLen);
	if (nPacketLen > 0)
	{
		CReadWriteMutexLock lock(m_csOutCallBack);

		if (m_pRecvPakcet)
		{
			m_pRecvPakcet(buffer, nPacketLen, m_pUserData);
		}

		DealSpecialPacket((unsigned char*)buffer, nPacketLen);

		if (m_pNormalPacket != NULL)
		{
			m_pNormalPacket(buffer, nPacketLen, m_pUserData);
		}

		lock.Unlock();

		nRet = 0;
	}

	return nRet;
}

/*
 *	摘要：发送数据完成回调，从发送队列中取数据继续发送
 */
int CTcpSocket::onSendDataAck(int nEngineId, int nConnId, int nId)
{
	return 1;
}

/*
 *	摘要：作服务器用才回调，有新的client连接，返回值为0表示接受此连接，返回值为1表示拒绝接受
 */
int CTcpSocket::onConnect(int nEngineId, int nConnId, char* szIp, int nPort)
{
	return 1;
}

/*
 *	摘要：作服务器用才回调，client退出回调，删除客户端连接列表
 */
int CTcpSocket::onClose(int nEngineId, int nConnId)
{
	return 1;
}

/*
 *	摘要：作客户端用才回调，断线回调，内部可能重连，由应用层决定是否close
 */
int CTcpSocket::onDisconnect(int nEngineId, int nConnId)
{
	m_nWritePos = m_nReadPos = 0;
	ResetEventEx(m_hRecEvent);

	CReadWriteMutexLock lock(m_csOutCallBack);

	if (m_pDisConnect != NULL)
	{
		m_pDisConnect(m_pUserData);
	}

	//	主动注册
	if (m_pListenSockFunc != NULL)
	{
		in_addr ip = {0};
		ip.s_addr = TPTCPClient::m_remoteIp;
		m_pListenSockFunc(m_pListenSocket, inet_ntoa(ip), ntohs(TPTCPClient::m_remotePort), -1, (void*)this, m_pListenUserData);
	}

	lock.Unlock();

	return 1;
}

/*
 *	摘要：作客户端用才回调，断线后重连成功回调，恢复所有的业务(操作)
 */
int CTcpSocket::onReconnect(int nEngineId, int nConnId)
{
	ResetEventEx(m_hRecEvent);

	CReadWriteMutexLock lock(m_csOutCallBack);

	if (m_pReconnect != NULL)
	{
		m_pReconnect(m_pUserData);
	}

	lock.Unlock();
	
	return 1;
}

//////////////////////////////////////////////////////////////////////////

void CTcpSocket::SetCallBack(OnDisConnectFunc cbDisConnect, OnReConnectFunc cbReconnect, 
							 OnNormalPacketFunc cbNormalPacket, OnRecPacketFunc cbReceivePacket, void* userdata)
{
	CReadWriteMutexLock lock(m_csOutCallBack);

	m_pUserData = userdata;
	m_pDisConnect = cbDisConnect;
    m_pReconnect = cbReconnect;
    m_pNormalPacket = cbNormalPacket;
	m_pRecvPakcet = cbReceivePacket;

	m_pListenSockFunc = NULL;
	m_pListenUserData = NULL;
	m_pListenSocket = NULL;

	lock.Unlock();
}

void CTcpSocket::SetKeepLife(unsigned char *szLifePacket, int nBufLen, unsigned int nKeepLifeTime)
{
	TPTCPClient::SetKeepLifePacket(szLifePacket, nBufLen, nKeepLifeTime);
}

int	CTcpSocket::ConnectHost(const char *szIp, int nPort, int nTimeOut)
{
	return TPTCPClient::Connect(szIp, nPort, nTimeOut);
}

void CTcpSocket::Disconnect()
{
	TPTCPClient::Close();
}

int	CTcpSocket::WriteData(char *pBuf, int nLen)
{
#ifdef NETSDK_VERSION_INVERSE
	for (int i = 0; i < nLen; i++)
	{
		unsigned char *p = (unsigned char*)(pBuf + i);
		*p = ~(*p);
	}
#endif

	IBufferRef pDataBuf = CAutoBuffer::CreateBuffer(nLen, pBuf, true);

#ifdef NETSDK_VERSION_INVERSE
	for (int j = 0; j < nLen; j++)
	{
		unsigned char *p = (unsigned char*)(pBuf + j);
		*p = ~(*p);
	}
#endif

	if (pDataBuf.IsEmpty())
	{
		return -1;
	}
	
	return TPTCPClient::Send(0, pDataBuf);
}

int	CTcpSocket::Heartbeat()
{
	return TPTCPClient::Heartbeat();
}

//////////////////////////////////////////////////////////////////////////

int	CTcpSocket::StartListen(const char *szIp, int nPort, OnListenSockFunc cbListenSock, void *userdata)
{
	m_pListenSockFunc = cbListenSock;
	m_pListenUserData = userdata;

	return TPTCPServer::Listen((char*)szIp, nPort);
}

int	CTcpSocket::StopListen()
{
	return TPTCPServer::Close();
}

int	CTcpSocket::DealNewSocket(SOCKET newsock, int connId, char* ip, int port)
{
	CTcpSocket *pNewSocket = new CTcpSocket();
	if (pNewSocket == NULL)
	{
		closesocket(newsock);
		return -1;
	}

	if (pNewSocket->CreateRecvBuf(m_nBufSize) < 0)
	{
		delete pNewSocket;
		closesocket(newsock);
		return -1;
	}

	if (m_pListenSockFunc != NULL)
	{
		m_pListenSockFunc(this, (char*)ip, port, 0, pNewSocket, m_pListenUserData);
	}

	//	变成客户端的连接类
	int ret = pNewSocket->SetSocket(newsock, connId, ip, port, m_pListenSockFunc, m_pListenUserData, (void*)this);
	if (ret < 0)
	{
		if (m_pListenSockFunc != NULL)
		{
			m_pListenSockFunc(this, (char*)ip, port, -1, pNewSocket, m_pListenUserData);
		}
	}

	return 1;
}

int	CTcpSocket::SetSocket(SOCKET newsock, int connId, const char* ip, int port, OnListenSockFunc cbListen, void* listenuserdata, void* pListenSocket)
{
	int nRet = -1;

	TPTCPClient::m_bReconnEn = FALSE;
	TPTCPClient::m_socket = newsock;
	TPTCPClient::m_remoteIp = inet_addr(ip);
	TPTCPClient::m_remotePort = htons(port);
	TPTCPClient::m_bOnline = TRUE;
	m_pListenSockFunc = cbListen;
	m_pListenUserData = listenuserdata;
	m_pListenSocket = pListenSocket;

	int ret = TPTCPClient::CreateClientEnvironment();
	if (ret >= 0)
	{
#if (defined(WIN32) && !(defined(NETSDK_VERSION_SSL)))
		//	将socket添加到完成端口中
		ret = TPTCPClient::AddSocketToIOCP(TPTCPClient::m_socket, TPTCPClient::m_pPerHandleData);
		if (ret >= 0)
		{
			TPTCPClient::m_pPerHandleData->AddRef();
			TPTCPClient::m_pPerIoRecv->AddRef();
			ret = TPTCPClient::PostRecvToIOCP(TPTCPClient::m_socket, TPTCPClient::m_pPerIoRecv);
			if (ret < 0)
			{
				TPTCPClient::m_pPerIoRecv->DecRef();
				TPTCPClient::m_pPerHandleData->DecRef();
				TPTCPClient::ClearClientEnvironment();
			}
			else
			{
				nRet = 1;
			}
		}
		else
		{
			TPTCPClient::ClearClientEnvironment();
		}
#else
		ret = TPTCPClient::AddSocketToThread(TPTCPClient::m_socket, TPTCPClient::m_pPerHandleData);
		if (ret >= 0)
		{
			nRet = 1;
		}
		else
		{
			TPTCPClient::ClearClientEnvironment();
		}
#endif
	}
	
	if (nRet < 0)
	{
#if (defined(WIN32) && !(defined(NETSDK_VERSION_SSL)))
		TPTCPClient::DelSocketFromIOCP(TPTCPClient::m_socket, TPTCPClient::m_pPerHandleData);
#else
		TPTCPClient::DelSocketFromThread(TPTCPClient::m_socket, TPTCPClient::m_pPerHandleData);
#endif
	}

	return nRet;
}

int	CTcpSocket::ResponseReg(bool bAccept)
{
//	ack 现在无须回应

//
//		BYTE ack[HEADER_SIZE] = {0};
//		ack[0] = 0x82;
//		ack[8] = 102;
//		ack[12] = bAccept?0:1;


//	WriteData((char*)ack, HEADER_SIZE);

	return 1;
}

//////////////////////////////////////////////////////////////////////////

int	CTcpSocket::SetIsReConn(int nEnable)
{
	return TPTCPClient::SetIsReconnect(nEnable);
}

int	CTcpSocket::SetIsDetectDisconn(int nEnable)
{
	return TPTCPClient::SetIsDetectDisconn(nEnable);
}

int	CTcpSocket::GetIsOnline()
{
	int nIsOnline = 0;

	CReadWriteMutexLock lock(TPTCPClient::m_csOnline, false, true, false);
	if (TPTCPClient::m_bOnline)
	{
		nIsOnline = 1;
	}
	else
	{
		nIsOnline = 0;
	}
	lock.Unlock();

	return nIsOnline;
}

void CTcpSocket::DealSpecialPacket(unsigned char *pbuf, int nlen)
{
	if (0xB0 == (unsigned char)*pbuf && nlen < 64)
	{
		memcpy(m_registerAck, pbuf, nlen);
		m_nRegisterLen = nlen;

		SetEventEx(m_hRecEvent);
	}
	else if (0xF4 == (unsigned char)*pbuf && nlen > HEADER_SIZE)
	{
		pbuf[nlen-1] = '\0';
		pbuf[nlen-2] = '\0';
		char szCommand[64] = {0};
		char szResultCode[64] = {0};
		char *p = GetProtocolValue((char*)(pbuf+HEADER_SIZE), "ParameterName:", "\r\n", szCommand, 64);
		if (p)
		{
			if (_stricmp(szCommand, "Dahua.Device.Network.ControlConnection.AckSubChannel") == 0)
			{
				p = GetProtocolValue((char*)(pbuf+HEADER_SIZE), "FaultCode:", "\r\n", szResultCode, 64);
				if (p)
				{
					if (_stricmp(szResultCode, "OK") == 0)
					{
						m_registerAck[8] = 0;
					}
					else
					{
						m_registerAck[8] = 1;
					}

					SetEventEx(m_hRecEvent);
				}
			}
		}
	}
	else if (m_pListenSockFunc != NULL && 0xB4 == (unsigned char)*pbuf && 0x07 == (unsigned char)*(pbuf + 8) && nlen < 1024)
	{
		// 主动注册包
		DWORD extlen = *(DWORD*)(pbuf+4);
		char *serial = new char[extlen+1];
		memset(serial, 0, extlen+1);
		memcpy(serial, pbuf+HEADER_SIZE, extlen);

		if (m_pListenSockFunc != NULL)
		{
			in_addr ip = {0};
			ip.s_addr = TPTCPClient::m_remoteIp;
			m_pListenSockFunc(m_pListenSocket, inet_ntoa(ip), ntohs(TPTCPClient::m_remotePort), 1, (void*)serial, m_pListenUserData);
		}
		
		delete[] serial;
	}
	else if(0xf1 == (unsigned char)*pbuf && nlen < 64)
	{
		m_registerAck[0] = pbuf[14];
		nlen = 1;

		SetEventEx(m_hRecEvent);
	}
	else if(0x0b == (unsigned char)*pbuf && nlen == 32)
	{
		// 登出命令返回
		SetEventEx(m_hRecEvent);
	}
}

int	CTcpSocket::GetData(unsigned char* buf, int len)
{
	int nDataLen = 0;

	CReadWriteMutexLock lock(m_csBuffer);

	if ((int)(m_nWritePos - m_nReadPos) >= HEADER_SIZE)
	{
		unsigned int extlen = *(unsigned int*)(m_pPacketBuf + m_nReadPos + 4);

		if ((extlen + HEADER_SIZE) >= len)
		{
			//	指示的扩展数据太长，不正常，清空缓存
#ifdef _DEBUG
		OutputDebugString("指示的扩展数据太长，不正常，清空缓存\n");
#endif			
			m_nWritePos = m_nReadPos = 0;
			return 0;
		}

		if (m_nWritePos - m_nReadPos >= extlen + HEADER_SIZE)
        {
			nDataLen = extlen + HEADER_SIZE;
			memcpy(buf, m_pPacketBuf + m_nReadPos, nDataLen);

			m_nReadPos += nDataLen;
        }
	}

	lock.Unlock();

	return nDataLen;
}

int	CTcpSocket::CloseSubConn()
{
#if (defined(WIN32) && !(defined(NETSDK_VERSION_SSL)))
	IOCPListener* listener = TPTCPClient::m_pPerHandleData->m_listener;
	TPTCPClient::closeInside();
	if (0 > TPTCPClient::Create(opMode_tcp))
	{
		return -1;
	}
	
	if (TPTCPClient::m_pPerHandleData != NULL)
	{
		CReadWriteMutexLock lock(TPTCPClient::m_pPerHandleData->m_pcsCallBack);
		TPTCPClient::m_pPerHandleData->m_socket = TPTCPClient::m_socket;
		TPTCPClient::m_pPerHandleData->m_connId = TPTCPClient::m_socket;
		TPTCPClient::m_pPerHandleData->m_listener = listener;
	}
#else
	TPTCPClient::m_bOnline = FALSE;
	TPTCPClient::m_bReconnEn = TRUE;
	TPTCPClient::m_emSockStatus = TP_TCP_CLOSE;
#endif

	return 1;
}

int	CTcpSocket::ConnectSubConn()
{
#if (defined(WIN32) && !(defined(NETSDK_VERSION_SSL)))
	int nOnline = TPTCPClient::IsConnected();

	if (nOnline == 1)
	{
		m_nWritePos = m_nReadPos = 0;
		int nRet = TPTCPClient::AddSocketToIOCP(TPTCPClient::m_socket, TPTCPClient::m_pPerHandleData);
		if (nRet >= 0)
		{
			TPTCPClient::m_bOnline = TRUE;
			TPTCPClient::m_pPerHandleData->AddRef();
			TPTCPClient::m_pPerIoRecv->AddRef();
			nRet = TPTCPClient::PostRecvToIOCP(TPTCPClient::m_socket, TPTCPClient::m_pPerIoRecv);
			if (nRet < 0)
			{
				TPTCPClient::m_pPerIoRecv->DecRef();
				TPTCPClient::m_pPerHandleData->DecRef();
			}
		}
	}
	else
	{
		if (GetTickCount() - TPTCPClient::m_dwLastConTime > 3000)
		{
			//connect
			struct sockaddr_in my_addr;
			memset(&my_addr, 0, sizeof(my_addr));
			
			my_addr.sin_family = AF_INET;
			my_addr.sin_addr.s_addr = TPTCPClient::m_remoteIp;
			my_addr.sin_port = TPTCPClient::m_remotePort;
			
			connect(TPTCPClient::m_socket, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));
			
			TPTCPClient::m_dwLastConTime = GetTickCount();
		}
	}

	return nOnline;
#else
	TPTCPClient::Heartbeat();
	if (TPTCPClient::m_bOnline)
	{
		return 1;
	}

	return -1;
#endif
}

#ifdef NETSDK_VERSION_BOGUSSSL
int	CTcpSocket::DealSSL()
{
	CReadWriteMutexLock lock(m_csBuffer);
	
	int nRevLen = (int)(m_nWritePos - m_nReadPos);
	
	switch(m_nSSL)
	{
	case 0:
		{
			if (nRevLen >= 840)
			{
				m_nSSL++;
				m_nWritePos = m_nReadPos = 0;
				SetEventEx(m_hSpecialEvent);
			}
		}
		break;
	case 1:
		{
			if (nRevLen >= 130)
			{
				m_nSSL++;
				m_nWritePos = m_nReadPos = 0;
				SetEventEx(m_hSpecialEvent);
			}
		}
		break;
	case 2:
		{
			if (nRevLen >= 36)
			{
				m_nSSL++;
				m_nWritePos = m_nReadPos = 0;
				SetEventEx(m_hSpecialEvent);
			}
		}
		break;
	case 3:
		{
			if (nRevLen >= 35)
			{
				m_nSSL++;
				m_nWritePos = m_nReadPos = 0;
				SetEventEx(m_hSpecialEvent);
			}
		}
		break;
	case 4:
		{
			if (nRevLen >= 35)
			{
				m_nSSL++;
				m_nWritePos = m_nReadPos = 0;
				SetEventEx(m_hSpecialEvent);
			}
		}
		break;
	case 5:
		{
			if (nRevLen >= 30)
			{
				m_nSSL++;
				m_nWritePos = m_nReadPos = 0;
				SetEventEx(m_hSpecialEvent);
			}
		}
		break;
	case 6:
		{
			if (nRevLen >= 36)
			{
				m_nSSL++;
				m_nWritePos = m_nReadPos = 0;
				SetEventEx(m_hSpecialEvent);
			}
		}
		break;
	default:
		break;
	}
	
	return 1;
}
#endif

#ifdef NETSDK_VERSION_SSL
int	CTcpSocket::SetSSL(int nEnable)
{
	TPTCPClient::m_bIsSSL = nEnable;
	return 1;
}
#endif





