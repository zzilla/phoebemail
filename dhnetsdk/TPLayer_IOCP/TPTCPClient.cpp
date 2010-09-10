// TPTCPClient.cpp: implementation of the TPTCPClient class.
//
//////////////////////////////////////////////////////////////////////

#include "TPTCPClient.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TPTCPClient::TPTCPClient(ITPListener *tcpclientapp, int engineId)
{
	this->m_listener = tcpclientapp;
	m_engineId = engineId;

	m_pPerHandleData = NULL;
	m_pPerIoRecv = NULL;
	
	m_nLifePacketLen = 0;
	m_lifePacketBuf = NULL;
	m_nKeepLifeTime = 10;
	m_dwLastReceiveTime = GetTickCount();
	m_dwLastHeartBeatTime = 0;

	m_bOnline = FALSE;
	m_bIsPostSend = TRUE;
	m_bCreateEnable = TRUE;
	m_dwLastConTime = 0;

	m_dwLastDealTime = 0;
	m_bLastStatus = TRUE;
	m_nDealData = 0;
}

TPTCPClient::~TPTCPClient()
{
	//退出之前一定要停止回调(完成回调和外层回调)
	Close();
}

/*
 *	摘要：此Heartbeat由外面的状态维护线程实现，开一个线程即可
 */
int TPTCPClient::Heartbeat()
{
	if (m_bOnline)
	{
		if (m_bDetectDisconn && GetTickCount() - m_dwLastReceiveTime > m_nDetectDisconnTime*1000)
		{
			m_bOnline = FALSE;

			if (this->m_listener != NULL && m_bDisCallBack)
			{
				this->m_listener->onDisconnect(m_engineId, m_socket);
				m_bDisCallBack = FALSE;
			}

			m_dwLastReceiveTime = GetTickCount();
		}
		else if (GetTickCount() - m_dwLastHeartBeatTime > m_nKeepLifeTime*1000 || GetTickCount() - m_dwLastHeartBeatTime < 0)
		{
			if (m_nLifePacketLen > 0 && !m_lifePacketBuf.IsEmpty())
			{
				Send(-1, m_lifePacketBuf);	//发送心跳包
			}
			m_dwLastHeartBeatTime = GetTickCount();
		}
	}
	else if (m_bReconnEn)
	{
		//对于出现断线的连接，要想重连必须清除并重新create
		if (m_bCreateEnable)
		{
			closeInside();
			if (0 > Create(opMode_tcp))
			{
				return 0;
			}

			if (m_pPerHandleData != NULL)
			{
				CReadWriteMutexLock lock(m_pPerHandleData->m_pcsCallBack);
				m_pPerHandleData->m_socket = m_socket;
				m_pPerHandleData->m_connId = m_socket;
				m_pPerHandleData->m_listener = this;
			}
			m_bCreateEnable = FALSE;

			//必须先connect再判断是否已连接
			struct sockaddr_in my_addr;
			memset(&my_addr, 0, sizeof(my_addr));
			
			my_addr.sin_family = AF_INET;
			my_addr.sin_addr.s_addr = m_remoteIp;
			my_addr.sin_port = m_remotePort;
			
			connect(m_socket, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));
			
			m_dwLastConTime = GetTickCount();
		}
		
		//检查是否已连上，未连接再重新连接
		if (1 == IsConnected())
		{
			m_bCreateEnable = TRUE;

			//	将socket添加到完成端口中
			int nRet = AddSocketToIOCP(m_socket, m_pPerHandleData);
			if (nRet >= 0)
			{
				m_pPerHandleData->AddRef();
				m_pPerIoRecv->AddRef();
				nRet = PostRecvToIOCP(m_socket, m_pPerIoRecv);
				if (nRet < 0)
				{
					m_bOnline = FALSE;
					m_pPerIoRecv->DecRef();
					m_pPerHandleData->DecRef();
				}
				else
				{
					m_bOnline = TRUE;
					
					if (this->m_listener != NULL)
					{
						this->m_listener->onReconnect(m_engineId, m_socket);
						m_bDisCallBack = TRUE;
					}
				}
			}
		}
		else
		{
			if (GetTickCount() - m_dwLastConTime > 2000 || GetTickCount() - m_dwLastConTime < 0)
			{
				//connect
				struct sockaddr_in my_addr;
				memset(&my_addr, 0, sizeof(my_addr));
				
				my_addr.sin_family = AF_INET;
				my_addr.sin_addr.s_addr = m_remoteIp;
				my_addr.sin_port = m_remotePort;
				
				connect(m_socket, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));

				m_dwLastConTime = GetTickCount();
			}
		}
	}

	return 1;
}

int TPTCPClient::onIOData(int nEngineId, int nConnId, unsigned char* data, int nLen, BOOL bPostSuccess)
{
	m_dwLastReceiveTime = GetTickCount();

	//	先存入缓冲
	if (this->m_listener != NULL && bPostSuccess)
	{
		//	保证可靠传输来牺牲最大性能，但不能死循环以免堵塞。
		int nNum = 1000;
		while (nNum--)
		{
			int ret = this->m_listener->onData(nEngineId, nConnId, data, nLen);
			if (ret < 0)
			{
				//	接收缓冲满
				Sleep(1);
			}
			else
			{
				break;
			}
		}
	}

	//	再投递接收
//	CReadWriteMutexLock lock(m_csOnline, false, true, false);
	if (m_bOnline)
	{
		//post
		m_pPerHandleData->AddRef();
		m_pPerIoRecv->AddRef();
		int nRet = PostRecvToIOCP(m_socket, m_pPerIoRecv);
		if (nRet < 0)
		{
			m_pPerIoRecv->DecRef();
			m_pPerHandleData->DecRef();
		}
	}
//	lock.Unlock();

	return 1;
}

/*
 *	摘要：返回值：0：忙；1：空闲
 */
int TPTCPClient::onIODealData(int nEngineId, int nConnId, unsigned char* buffer, int nLen)
{
	int nRet = 1;

	CReadWriteMutexLock lock(m_csDealData);
	if (m_nDealData >= 1 || (!m_bLastStatus && (GetTickCountEx() - m_dwLastDealTime) < MAX_DEAL_SPANTIME))	//一个线程处理
	{
		return nRet;
	}
	else
	{
		m_nDealData++;
	}
	lock.Unlock();

	if (this->m_listener != NULL)
	{
		nRet = this->m_listener->onDealData(nEngineId, nConnId, buffer, nLen);
	}

	m_dwLastDealTime = GetTickCountEx();
	m_bLastStatus = nRet==0?TRUE:FALSE;

	CReadWriteMutexLock lock1(m_csDealData);
	m_nDealData--;
	lock1.Unlock();

	return nRet;
}

int TPTCPClient::onIODisconnect(int nEngineId, int nConnId)
{
	CReadWriteMutexLock lock(m_csOnline);
	m_bOnline = FALSE;
	lock.Unlock();

	if (this->m_listener != NULL && m_bDisCallBack)
	{
		this->m_listener->onDisconnect(nEngineId, nConnId);
		m_bDisCallBack = FALSE;
	}

	return 1;
}

int TPTCPClient::onIOSendDataAck(int nEngineId, int nConnId, int nId, BOOL bPostSuccess)
{
	CReadWriteMutexLock lock(m_csOnline, false, true, false);

	if (m_bOnline)
	{
		//post
		CReadWriteMutexLock lock1(m_csSendQueue);

		BOOL bPostSendEn = TRUE;
		int nSize = m_lstSendQueue.size();
		if (nSize >= 1)
		{
			CPerIoData *pPerIoSend = new CPerIoData;
			if (pPerIoSend != NULL)
			{
				DataRow* row = m_lstSendQueue.front();

				pPerIoSend->m_wsabuf.buf = row->buf->GetBuf();
				pPerIoSend->m_wsabuf.len = row->buf->BufferSize();
				pPerIoSend->m_operationType = opType_send;
				pPerIoSend->m_operationMode = opMode_tcp;
				pPerIoSend->m_pDataBuf = row->buf;
				pPerIoSend->m_id = row->id;
				pPerIoSend->m_lTotalBytes = row->buf->BufferSize();
				m_pPerHandleData->AddRef();
				if (0 > PostSendToIOCP(row->socket, pPerIoSend))
				{
					pPerIoSend->m_wsabuf.buf = NULL;
					pPerIoSend->DecRef();
					m_pPerHandleData->DecRef();
				}
				else
				{
					bPostSendEn = FALSE;
					m_lstSendQueue.pop();
					delete row;
				}
			}
		}

		m_bIsPostSend = bPostSendEn;
	}
	else
	{
		m_bIsPostSend = TRUE;
	}
	
	lock.Unlock();
	
	if (this->m_listener != NULL && nId != -1)	// -1为心跳包
	{
		this->m_listener->onSendDataAck(nEngineId, nConnId, nId);
	}

	return 1;
}

int TPTCPClient::Connect(const char* szIp, int nPort, int nTimeOut)
{
	//	先create
	int nRet = Create(opMode_tcp);
	if (nRet < 0)
	{
		return -1;
	}
	
	//	再创建客户端环境
	nRet = CreateClientEnvironment();
	if (nRet < 0)
	{
		goto e_clean;
	}

	//	然后connect
    m_remoteIp = inet_addr(szIp);
    m_remotePort = htons(nPort);
	m_bOnline = FALSE;

    struct sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = m_remoteIp;
    my_addr.sin_port = m_remotePort;

    nRet = connect(m_socket, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));

	fd_set fdwrite;
	FD_ZERO(&fdwrite);
	FD_SET(m_socket,&fdwrite);
	timeval tv;
	tv.tv_sec = nTimeOut/1000;
	tv.tv_usec = nTimeOut%1000;
	
	nRet = select(m_socket + 1, NULL, &fdwrite, NULL, &tv);
	if(nRet > 0 && FD_ISSET(m_socket, &fdwrite))
	{
		int error = -1;
		int llen = sizeof(int);

		getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &llen);
		if(error == 0)
		{
			m_bOnline = TRUE;
			m_dwLastReceiveTime = GetTickCount();

			//	将socket添加到完成端口中
			nRet = AddSocketToIOCP(m_socket, m_pPerHandleData);
			if (nRet >= 0)
			{
				m_pPerHandleData->AddRef();
				m_pPerIoRecv->AddRef();
				nRet = PostRecvToIOCP(m_socket, m_pPerIoRecv);
				if (nRet < 0)
				{
					m_bOnline = FALSE;
					m_pPerIoRecv->DecRef();
					m_pPerHandleData->DecRef();
				}
			}
		}
	}

	if (!m_bOnline)
	{
		ClearClientEnvironment();
		goto e_clean;
	}

    return 1;

e_clean:
	DelSocketFromIOCP(m_socket, m_pPerHandleData);
	m_socket = INVALID_SOCKET;
	return -1;
}

int TPTCPClient::Connect(const char* szLocalIp, int nLocalPort, const char* szRemoteIp, int nRemotePort)
{
	m_localIp = inet_addr(szLocalIp);
    m_localPort = htons(nLocalPort);

	return Connect(szRemoteIp, nRemotePort);
}

int TPTCPClient::Send(int id, IBufferRef pSendBuf)
{
	int nRet = -1;

	CReadWriteMutexLock lock(m_csOnline, false, true, false);
	if (m_bOnline)
	{
		CReadWriteMutexLock lock1(m_csSendQueue);
		
		int nSize = m_lstSendQueue.size();
		if (nSize <= m_nSendQueueSize || -1 == id)	// 心跳包优先发送
		{
			if (m_bIsPostSend)
			{
				// post
				CPerIoData *pPerIoSend = new CPerIoData;
				if (pPerIoSend != NULL)
				{
					pPerIoSend->m_wsabuf.buf = pSendBuf->GetBuf();
					pPerIoSend->m_wsabuf.len = pSendBuf->BufferSize();
					pPerIoSend->m_operationType = opType_send;
					pPerIoSend->m_operationMode = opMode_tcp;
					pPerIoSend->m_pDataBuf = pSendBuf;
					pPerIoSend->m_id = id;
					pPerIoSend->m_lTotalBytes = pSendBuf->BufferSize();
					m_pPerHandleData->AddRef();
					if (0 > PostSendToIOCP(m_socket, pPerIoSend))
					{
						pPerIoSend->m_wsabuf.buf = NULL;
						pPerIoSend->DecRef();
						m_pPerHandleData->DecRef();
					}
					else
					{
						m_bIsPostSend = FALSE;
						nRet = 1;
					}
				}
			}
			else
			{
				DataRow* row = new DataRow;
				if (row != NULL)
				{
					row->id = id;
					row->socket = m_socket;
					row->buf = pSendBuf;
					m_lstSendQueue.push(row);

					nRet = 1;
				}
			}
		}
		else
		{
			nRet = -3;
		}
	}
	else
	{
		nRet = -2;
	}

	return nRet;
}

int TPTCPClient::Close()
{
	CReadWriteMutexLock lock(m_csOnline);
	m_bOnline = FALSE;
	m_bReconnEn = FALSE;
	lock.Unlock();

	ClearClientEnvironment();

	CReadWriteMutexLock lock1(m_csSendQueue);
	while (!m_lstSendQueue.empty())
	{
		DataRow *conn = m_lstSendQueue.front();
		if (conn != NULL)
		{
			delete conn;
		}
		m_lstSendQueue.pop();
	}
	lock1.Unlock();

	return 1;
}

int TPTCPClient::closeInside()
{
	DelSocketFromIOCP(INVALID_SOCKET, m_pPerHandleData);
	
	CReadWriteMutexLock lock(m_pPerHandleData->m_pcsCallBack);
	m_pPerHandleData->m_listener = NULL;
	lock.Unlock();

	if (m_socket != INVALID_SOCKET)
	{
		DelSocketFromIOCP(m_socket, NULL);

		m_socket = INVALID_SOCKET;
	}

	CReadWriteMutexLock lock1(m_csSendQueue);
	while (!m_lstSendQueue.empty())
	{
		DataRow *conn = m_lstSendQueue.front();
		if (conn != NULL)
		{
			delete conn;
		}
		m_lstSendQueue.pop();
	}
	m_bIsPostSend = TRUE;
	lock1.Unlock();

	return 1;
}

int	TPTCPClient::IsConnected()
{
	int nIsOnline = 0;

	fd_set fdwrite;
	FD_ZERO(&fdwrite);
	FD_SET(m_socket,&fdwrite);
	timeval tv;
	tv.tv_sec = m_timeout_sec;
	tv.tv_usec = m_timeout_usec;
	
	int ret = select(m_socket + 1, NULL, &fdwrite, NULL, &tv);
	if(ret > 0 && FD_ISSET(m_socket, &fdwrite))
	{
		int error = -1;
		int llen = sizeof(int);

		getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &llen);
		if(error == 0)
		{
			nIsOnline = 1;
		}
	}

	return nIsOnline;
}

int	TPTCPClient::SetKeepLifePacket(unsigned char *pLifePacketBuf, int nBufLen, unsigned int nKeepLifeTime)
{
	if (pLifePacketBuf == NULL || nBufLen <= 0)
	{
		return -1;
	}

	m_nKeepLifeTime = nKeepLifeTime;
	m_nLifePacketLen = nBufLen;
	m_lifePacketBuf = CAutoBuffer::CreateBuffer(nBufLen, (char*)pLifePacketBuf, true);

	return 1;
}

int	TPTCPClient::CreateClientEnvironment()
{
	char *pRecvBuf = new char[RECEIVE_BUFFER_SIZE];
	if (NULL == pRecvBuf)
	{
		return -1;
	}

	IBufferRef pDataBuf = CAutoBuffer::CreateBuffer(RECEIVE_BUFFER_SIZE, pRecvBuf, false);
	if (pDataBuf.IsEmpty())
	{
		goto e_out;
	}
	pRecvBuf = NULL;

	m_pPerIoRecv = new CPerIoData;
	if (NULL == m_pPerIoRecv)
	{
		goto e_out;
	}

	m_pPerHandleData = new CPerHandleData;
	if (NULL == m_pPerHandleData)
	{
		goto e_out;
	}
	
	m_pPerIoRecv->m_wsabuf.buf = pDataBuf->GetBuf();
	m_pPerIoRecv->m_wsabuf.len = pDataBuf->BufferSize();
	m_pPerIoRecv->m_operationType = opType_recv;
	m_pPerIoRecv->m_operationMode = opMode_tcp;
	m_pPerIoRecv->m_pDataBuf = pDataBuf;

	m_pPerHandleData->m_socket = m_socket;
	m_pPerHandleData->m_engineId = m_engineId;
	m_pPerHandleData->m_connId = m_socket;
	m_pPerHandleData->m_listener = this;

	return 1;

e_out:
	if (pRecvBuf != NULL)
	{
		delete[] pRecvBuf;
	}

	if (m_pPerIoRecv != NULL)
	{
		delete m_pPerIoRecv;
		m_pPerIoRecv = NULL;
	}
	
	if (m_pPerHandleData != NULL)
	{
		delete m_pPerHandleData;
		m_pPerHandleData = NULL;
	}

	return -1;
}

int	TPTCPClient::ClearClientEnvironment()
{
	if (m_pPerHandleData != NULL)
	{
		//	必须先停止处理线程处理
		DelSocketFromIOCP(INVALID_SOCKET, m_pPerHandleData);
		
		//	停止工作线程处理
		CReadWriteMutexLock lock(m_pPerHandleData->m_pcsCallBack);
		m_pPerHandleData->m_listener = NULL;
		lock.Unlock();
		
		//	关闭连接
		DelSocketFromIOCP(m_socket, NULL);
		m_socket = INVALID_SOCKET;
		m_pPerHandleData->DecRef();
		m_pPerHandleData = NULL;
	}

	if (m_pPerIoRecv != NULL)
	{
		m_pPerIoRecv->m_operationType = opType_none;
		m_pPerIoRecv->m_operationMode = opMode_none;
	
		m_pPerIoRecv->DecRef();
		m_pPerIoRecv = NULL;
	}

	return 1;
}
