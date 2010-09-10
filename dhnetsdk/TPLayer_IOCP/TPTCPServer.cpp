// TPTCPServer.cpp: implementation of the TPTCPServer class.
//
//////////////////////////////////////////////////////////////////////

#include "TPTCPServer.h"

DWORD WINAPI ListenThreadFunc(void *param);
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TPTCPServer::TPTCPServer(ITPListener* tcpserverapp, int engineId)
{
	this->m_listener = tcpserverapp;
	m_engineId = engineId;

	m_hListenThread = NULL;
	m_hListenExitEvent = NULL;
}

TPTCPServer::~TPTCPServer()
{
	//	退出之前一定要停止所有回调(完成回调和外层回调)
	Close();
}

int TPTCPServer::onIOData(int nEngineId, int nConnId, unsigned char* data, int nLen, BOOL bPostSuccess)
{
	CConnClientData *pConnClient = NULL;

	CReadWriteMutexLock lock(m_csClientsMap, false, true, false);
	CONN_MAP::iterator it = m_lstConnClients.find(nConnId);
	if(it != m_lstConnClients.end())
	{
		pConnClient = (*it).second;
		if (pConnClient != NULL)
		{
			pConnClient->m_dwLastReceiveTime = GetTickCount();
			pConnClient->AddRef();
		}
	}
	lock.Unlock();

	if (pConnClient != NULL)
	{
		if (this->m_listener != NULL && bPostSuccess)
		{
			this->m_listener->onData(nEngineId, nConnId, data, nLen);
		}

//		CReadWriteMutexLock lock2(pConnClient->m_pcsData, false, true, false);
		if (pConnClient->m_bIsOnline)
		{
			//post
			pConnClient->m_pPerHandleData->AddRef();
			pConnClient->m_pPerIoRecv->AddRef();
			int nRet = PostRecvToIOCP(pConnClient->m_socket, pConnClient->m_pPerIoRecv);
			if (nRet < 0)
			{
				pConnClient->m_pPerIoRecv->DecRef();
				pConnClient->m_pPerHandleData->DecRef();
			}
		}
//		lock.Unlock();

		pConnClient->DecRef();
	}

	return 1;
}

/*
 *	摘要：返回值：0：忙；1：空闲
 */
int TPTCPServer::onIODealData(int nEngineId, int nConnId, unsigned char* buffer, int nLen)
{
	int nRet = 1;
	CConnClientData *pConnClient = NULL;

	CReadWriteMutexLock lock(m_csClientsMap, false, true, false);
	CONN_MAP::iterator it = m_lstConnClients.find(nConnId);
	if(it != m_lstConnClients.end())
	{
		pConnClient = (*it).second;
		if (pConnClient != NULL)
		{
			pConnClient->AddRef();
		}
	}
	lock.Unlock();
	
	if (pConnClient != NULL)
	{
		CReadWriteMutexLock lockData(pConnClient->m_csDealData);
		if (pConnClient->m_bIsDealData || (!pConnClient->m_bLastStatus && (GetTickCountEx() - pConnClient->m_dwLastDealTime) < MAX_DEAL_SPANTIME))
		{
			lockData.Unlock();
			pConnClient->DecRef();
			return nRet;
		}
		else
		{
			pConnClient->m_bIsDealData = TRUE;
		}
		lockData.Unlock();
		
		if (this->m_listener != NULL)
		{
			nRet = this->m_listener->onDealData(nEngineId, nConnId, buffer, nLen);
		}

		pConnClient->m_dwLastDealTime = GetTickCountEx();
		pConnClient->m_bLastStatus = nRet==0?TRUE:FALSE;
		
		CReadWriteMutexLock lockData1(pConnClient->m_csDealData);
		pConnClient->m_bIsDealData = FALSE;
		lockData1.Unlock();
		
		pConnClient->DecRef();
	}

	return nRet;
}

int TPTCPServer::onIODisconnect(int nEngineId, int nConnId)
{
	CConnClientData *pConnClient = NULL;

	CReadWriteMutexLock lock(m_csClientsMap, false, true, false);
	CONN_MAP::iterator it = m_lstConnClients.find(nConnId);
	if(it != m_lstConnClients.end())
	{
		pConnClient = (*it).second;
		if (pConnClient != NULL)
		{
			pConnClient->AddRef();
		}
	}
	lock.Unlock();

	if (pConnClient != NULL)
	{
		CReadWriteMutexLock lock1(pConnClient->m_pcsData);
		pConnClient->m_bIsOnline = FALSE;
		lock1.Unlock();

		pConnClient->DecRef();

		//close
		CloseClient(nConnId);

		if (this->m_listener != NULL)
		{
			this->m_listener->onClose(nEngineId, nConnId);
		}
	}

	return 1;
}

int TPTCPServer::onIOSendDataAck(int nEngineId, int nConnId, int nId, BOOL bPostSuccess)
{
	CConnClientData *pConnClient = NULL;

	CReadWriteMutexLock lock(m_csClientsMap, false, true, false);
	CONN_MAP::iterator it = m_lstConnClients.find(nConnId);
	if(it != m_lstConnClients.end())
	{
		pConnClient = (*it).second;
		if (pConnClient != NULL)
		{
			pConnClient->AddRef();
		}
	}
	lock.Unlock();

	if (pConnClient != NULL)
	{
		CReadWriteMutexLock lock1(pConnClient->m_pcsData, false, true, false);

		if (pConnClient->m_bIsOnline)
		{
			//post
			CReadWriteMutexLock lock2(pConnClient->m_csSendQueue);

			BOOL bPostSendEn = TRUE;
			int nSize = pConnClient->m_lstSendQueue.size();
			if (nSize >= 1)
			{
				CPerIoData *pPerIoSend = new CPerIoData;
				if (pPerIoSend != NULL)
				{
					DataRow* row = pConnClient->m_lstSendQueue.front();
					
					pPerIoSend->m_wsabuf.buf = row->buf->GetBuf();
					pPerIoSend->m_wsabuf.len = row->buf->BufferSize();
					pPerIoSend->m_operationType = opType_send;
					pPerIoSend->m_operationMode = opMode_tcp;
					pPerIoSend->m_pDataBuf = row->buf;
					pPerIoSend->m_id = row->id;
					pPerIoSend->m_lTotalBytes = row->buf->BufferSize();
					pConnClient->m_pPerHandleData->AddRef();
					if (0 > PostSendToIOCP(row->socket, pPerIoSend))
					{
						pPerIoSend->m_wsabuf.buf = NULL;
						pPerIoSend->DecRef();
						pConnClient->m_pPerHandleData->DecRef();
					}
					else
					{
						bPostSendEn = FALSE;
						pConnClient->m_lstSendQueue.pop();
						delete row;
					}
				}
			}

			pConnClient->m_bPostSend = bPostSendEn;
		}
		else
		{
			pConnClient->m_bPostSend = TRUE;
		}
		
		lock1.Unlock();

		pConnClient->DecRef();

		if (this->m_listener != NULL && nId != -1)
		{
			this->m_listener->onSendDataAck(nEngineId, nConnId, nId);
		}
	}

	return 1;
}

int TPTCPServer::Send(int connId, int id, IBufferRef pSendBuf)
{
	int nRet = -1;
	CConnClientData *pConnClient = NULL;

	CReadWriteMutexLock lock(m_csClientsMap, false, true, false);
	CONN_MAP::iterator it = m_lstConnClients.find(connId);
	if(it != m_lstConnClients.end())
	{
		pConnClient = (*it).second;
		if (pConnClient != NULL)
		{
			pConnClient->AddRef();
		}
	}
	lock.Unlock();

	if (pConnClient != NULL)
	{
		CReadWriteMutexLock lock1(pConnClient->m_pcsData, false, true, false);

		if (pConnClient->m_bIsOnline)
		{
			CReadWriteMutexLock lock2(pConnClient->m_csSendQueue);
			
			int nSize = pConnClient->m_lstSendQueue.size();
			if (nSize <= m_nSendQueueSize)
			{
				if (pConnClient->m_bPostSend)
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
						pConnClient->m_pPerHandleData->AddRef();
						if (0 > PostSendToIOCP(pConnClient->m_socket, pPerIoSend))
						{
							pPerIoSend->m_wsabuf.buf = NULL;
							pPerIoSend->DecRef();
							pConnClient->m_pPerHandleData->DecRef();
						}
						else
						{
							pConnClient->m_bPostSend = FALSE;
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
						pConnClient->m_lstSendQueue.push(row);
						
						nRet = 1;
					}
				}
			}
		}
		
		lock1.Unlock();

		pConnClient->DecRef();
	}

	return nRet;
}

int TPTCPServer::Heartbeat()
{
	int connId = -1;

	//	断线检测，关闭操作时间可能较长，每次处理一个
	CReadWriteMutexLock lock(m_csClientsMap, false, true, false);

	for (CONN_MAP::iterator it = m_lstConnClients.begin(); it != m_lstConnClients.end(); it++)
	{
		CConnClientData *pConnClient = (*it).second;
		if (pConnClient != NULL)
		{
			if (GetTickCount() - pConnClient->m_dwLastReceiveTime > m_nDetectDisconnTime*1000 && pConnClient->m_dwLastReceiveTime != 0)
			{
				pConnClient->m_bIsOnline = FALSE;
				connId = (*it).first;
				break;
			}
		}
	}
	
	lock.Unlock();

	//	关闭
	if (connId != -1)
	{
		CloseClient(connId);
		
		if (this->m_listener != NULL)
		{
			this->m_listener->onClose(m_engineId, connId);
		}
	}

	return 1;
}


int TPTCPServer::Listen(const char* szIp, int nPort)
{
	if (szIp == NULL)
    {
        m_localIp = INADDR_ANY;
    }
    else
    {
        m_localIp = inet_addr(szIp);
    }

    m_localPort = htons(nPort);

	//	先create
	int nRet = Create(opMode_tcp);
	if (nRet < 0)
	{
		return -1;
	}

	DWORD dwThreadID = 0;

	if (SOCKET_ERROR == listen(m_socket, 5))
    {
		goto e_clean;
    }

	m_hListenExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (NULL == m_hListenExitEvent) 
	{
		goto e_clean;
	}

	m_hListenThread = CreateThread(NULL, 0, ListenThreadFunc, this, 0, &dwThreadID);
	if (NULL == m_hListenThread)
	{
		CloseHandle(m_hListenExitEvent);
		m_hListenExitEvent = NULL;
		goto e_clean;
	}

	return 1;

e_clean:
	closesocket(m_socket);
	m_socket = INVALID_SOCKET;
	return -1;
}

DWORD WINAPI ListenThreadFunc(void *param)
{
	TPTCPServer* pThis = (TPTCPServer*)param;
	return pThis->ListenSocket();
}

int	TPTCPServer::ListenSocket()
{
	if (INVALID_SOCKET == m_socket)
	{
		return -1;
	}

	SOCKADDR_IN addr = {0};
	int		 addrLen = sizeof(SOCKADDR_IN);
	while (WaitForSingleObject(m_hListenExitEvent, 0) != WAIT_OBJECT_0) 
	{
		FD_SET  fdread;
		FD_ZERO(&fdread);
		FD_SET(m_socket, &fdread);
		
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 300;
		
		int ret = select(0, &fdread, NULL, NULL, &tv);
		if (SOCKET_ERROR == ret)
		{
			return -1;
		}
		
		if (FD_ISSET(m_socket, &fdread))
		{
			SOCKET cltsock = accept(m_socket, (SOCKADDR*)&addr, &addrLen);
			if (INVALID_SOCKET == cltsock)
			{
				//accept failed, not supposed to happen here
				continue;
			}
			
			int connId = ITPObject::GetNewClientId();
			char *ip = inet_ntoa(addr.sin_addr);
			int port = ntohs(addr.sin_port);

			unsigned long flags = 1;
			ret = ioctlsocket(cltsock, FIONBIO, &flags);
			
			if(m_recvBuffSize > 0)
				setsockopt(cltsock, SOL_SOCKET, SO_RCVBUF, (char*)&m_recvBuffSize, sizeof(int));
			if(m_sendBuffSize > 0)
				setsockopt(cltsock, SOL_SOCKET, SO_SNDBUF, (char*)&m_sendBuffSize, sizeof(int));
			
			
			DealNewSocket(cltsock, connId, ip, port);
		}
	}
	
	return 1;
}

int	TPTCPServer::DealNewSocket(SOCKET newsock, int nConnId, char* szIp, int nPort)
{
	BOOL bAccept = FALSE;
	int ret = 1;
	
	if (this->m_listener != NULL)
	{
		ret = this->m_listener->onConnect(m_engineId, nConnId, szIp, nPort);
	}
	
	if (ret == 0)
	{
		CConnClientData *pConn = new CConnClientData();
		if (pConn != NULL)
		{
			ret = CreateClientEnvironment(pConn);
			if (ret >= 0)
			{
				pConn->m_pPerHandleData->m_socket = newsock;
				pConn->m_pPerHandleData->m_engineId = m_engineId;
				pConn->m_pPerHandleData->m_connId = nConnId;
				pConn->m_pPerHandleData->m_listener = this;
				pConn->m_id = nConnId;
				pConn->m_socket = newsock;
				pConn->m_remoteIp = inet_addr(szIp);
				pConn->m_remotePort = htons(nPort);
				
				//	将socket添加到完成端口中
				ret = AddSocketToIOCP(newsock, pConn->m_pPerHandleData);
				if (ret >= 0)
				{
					pConn->m_pPerHandleData->AddRef();
					pConn->m_pPerIoRecv->AddRef();
					ret = PostRecvToIOCP(newsock, pConn->m_pPerIoRecv);
					if (ret < 0)
					{
						pConn->m_pPerIoRecv->DecRef();
						pConn->m_pPerHandleData->DecRef();
						ClearClientEnvironment(pConn);
					}
					else
					{
						bAccept = TRUE;
						pConn->m_bIsOnline = TRUE;
						pConn->m_dwLastReceiveTime = GetTickCount();

						CReadWriteMutexLock lock(m_csClientsMap);
						m_lstConnClients[nConnId] = pConn;
					}
				}
				else
				{
					ClearClientEnvironment(pConn);
				}
			}
			else
			{
				DelSocketFromIOCP(newsock, NULL);
			}
		}
		else
		{
			DelSocketFromIOCP(newsock, NULL);
		}
		
		if (!bAccept)
		{
			if (this->m_listener != NULL)
			{
				this->m_listener->onClose(m_engineId, nConnId);
			}
		}
	}
	else
	{
		DelSocketFromIOCP(newsock, NULL);
	}

	return 1;
}

int TPTCPServer::Close()
{
	if (m_socket != INVALID_SOCKET)
	{
		DelSocketFromIOCP(m_socket, NULL);
		m_socket = INVALID_SOCKET;
	}

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

	//	为了避免锁套锁，将链表拷到临时链表中
	CONN_MAP lstConnClients;

	{
		CReadWriteMutexLock lock(m_csClientsMap);
		
		for (CONN_MAP::iterator it = m_lstConnClients.begin(); it != m_lstConnClients.end(); it++)
		{
			lstConnClients[(*it).first] = (*it).second;
		}
		m_lstConnClients.clear();
		
		lock.Unlock();
	}

	{
		for (CONN_MAP::iterator it = lstConnClients.begin(); it != lstConnClients.end(); it++)
		{
			CConnClientData *pConnClient = (*it).second;
			if (pConnClient != NULL)
			{
				CReadWriteMutexLock lock1(pConnClient->m_pcsData);
				pConnClient->m_bIsOnline = FALSE;
				lock1.Unlock();
				
				ClearClientEnvironment(pConnClient);
				
				pConnClient->DecRef();
			}
		}
		lstConnClients.clear();
	}

	return 1;
}

int TPTCPServer::CloseClient(int connId)
{
	CConnClientData *pConnClient = NULL;

	CReadWriteMutexLock lock(m_csClientsMap);
	CONN_MAP::iterator it = m_lstConnClients.find(connId);
	if(it != m_lstConnClients.end())
	{
		pConnClient = (*it).second;
		m_lstConnClients.erase(connId);
	}
	lock.Unlock();

	if (pConnClient != NULL)
	{
// 		CReadWriteMutexLock lock2(pConnClient->m_pcsData);
		pConnClient->m_bIsOnline = FALSE;
// 		lock2.Unlock();

		ClearClientEnvironment(pConnClient);

		pConnClient->DecRef();
	}

	return 1;
}

int	TPTCPServer::CreateClientEnvironment(CConnClientData *pClientConn)
{
	if (pClientConn == NULL)
	{
		return -1;
	}

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

	pClientConn->m_pPerIoRecv = new CPerIoData;
	if (NULL == pClientConn->m_pPerIoRecv)
	{
		goto e_out;
	}

	pClientConn->m_pPerHandleData = new CPerHandleData;
	if (NULL == pClientConn->m_pPerHandleData)
	{
		goto e_out;
	}
	
	pClientConn->m_pPerIoRecv->m_wsabuf.buf = pDataBuf->GetBuf();
	pClientConn->m_pPerIoRecv->m_wsabuf.len = pDataBuf->BufferSize();
	pClientConn->m_pPerIoRecv->m_operationType = opType_recv;
	pClientConn->m_pPerIoRecv->m_operationMode = opMode_tcp;
	pClientConn->m_pPerIoRecv->m_pDataBuf = pDataBuf;

	pClientConn->m_pPerHandleData->m_engineId = m_engineId;
	pClientConn->m_pPerHandleData->m_listener = this;

	return 1;

e_out:
	if (pRecvBuf != NULL)
	{
		delete[] pRecvBuf;
	}

	if (pClientConn->m_pPerIoRecv != NULL)
	{
		delete pClientConn->m_pPerIoRecv;
		pClientConn->m_pPerIoRecv = NULL;
	}
	
	if (pClientConn->m_pPerHandleData != NULL)
	{
		delete pClientConn->m_pPerHandleData;
		pClientConn->m_pPerHandleData = NULL;
	}

	return -1;
}

int	TPTCPServer::ClearClientEnvironment(CConnClientData *pClientConn)
{
	if (pClientConn == NULL)
	{
		return -1;
	}

	if (pClientConn->m_pPerHandleData != NULL)
	{
		//	停止处理线程处理
		DelSocketFromIOCP(INVALID_SOCKET, pClientConn->m_pPerHandleData);

		//	停止工作线程处理
		CReadWriteMutexLock lock(pClientConn->m_pPerHandleData->m_pcsCallBack);
		pClientConn->m_pPerHandleData->m_listener = NULL;
		lock.Unlock();
		
		//	关闭连接
		DelSocketFromIOCP(pClientConn->m_socket, NULL);
		pClientConn->m_socket = INVALID_SOCKET;
		pClientConn->m_pPerHandleData->DecRef();
		pClientConn->m_pPerHandleData = NULL;
	}

	if (pClientConn->m_pPerIoRecv != NULL)
	{
		pClientConn->m_pPerIoRecv->m_operationType = opType_none;
		pClientConn->m_pPerIoRecv->m_operationMode = opMode_none;
	
		pClientConn->m_pPerIoRecv->DecRef();
		pClientConn->m_pPerIoRecv = NULL;
	}

	return 1;
}