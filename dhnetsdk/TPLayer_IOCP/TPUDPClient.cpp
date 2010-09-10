// TPUDPClient.cpp: implementation of the TPUDPClient class.
//
//////////////////////////////////////////////////////////////////////

#include "TPUDPClient.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TPUDPClient::TPUDPClient(ITPListener *callback, int engineId)
{
	this->m_listener = callback;
	m_engineId = engineId;

	m_pPerHandleData = NULL;
	m_pPerIoRecv = NULL;

	m_bIsPostSend = TRUE;
	m_bIsDealData = FALSE;
	m_dwLastDealTime = 0;
	m_bLastStatus = TRUE;
}

TPUDPClient::~TPUDPClient()
{
	//	退出之前一定要停止回调(完成回调和外层回调)
	this->m_listener = NULL;
	Close();

	Sleep(1);
}

int TPUDPClient::onIOData(int nEngineId, int nConnId, unsigned char* data, int nLen, BOOL bPostSuccess)
{
	//	先存入缓冲队列里
	if (this->m_listener != NULL && bPostSuccess)
	{
		this->m_listener->onData(nEngineId, nConnId, data, nLen);
	}

	//	再投递接收
	m_pPerHandleData->AddRef();
	m_pPerIoRecv->AddRef();
	int nRet = PostRecvToIOCP(m_socket, m_pPerIoRecv);
	if (nRet < 0)
	{
		m_pPerIoRecv->DecRef();
		m_pPerHandleData->DecRef();
	}

	return 1;
}

/*
 *	摘要：返回值：0：忙；1：空闲
 */
int TPUDPClient::onIODealData(int nEngineId, int nConnId, unsigned char* buffer, int nLen)
{
	int nRet = 1;

	CReadWriteMutexLock lock(m_csDealData);
	if (m_bIsDealData || (!m_bLastStatus && (GetTickCountEx() - m_dwLastDealTime) < MAX_DEAL_SPANTIME))
	{
		return nRet;
	}
	else
	{
		m_bIsDealData = TRUE;
	}
	lock.Unlock();

	if (this->m_listener != NULL)
	{
		nRet = this->m_listener->onDealData(nEngineId, nConnId, buffer, nLen);
	}

	m_dwLastDealTime = GetTickCountEx();
	m_bLastStatus = nRet==0?TRUE:FALSE;

	CReadWriteMutexLock lock1(m_csDealData);
	m_bIsDealData = FALSE;
	lock1.Unlock();

	return nRet;
}

int TPUDPClient::onIODisconnect(int nEngineId, int nConnId)
{
	if (this->m_listener != NULL && m_bDisCallBack)
	{
		this->m_listener->onDisconnect(nEngineId, nConnId);
		m_bDisCallBack = FALSE;
	}

	return 1;
}

int TPUDPClient::onIOSendDataAck(int nEngineId, int nConnId, int nId, BOOL bPostSuccess)
{
	CReadWriteMutexLock lock(m_csSendQueue);

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
			pPerIoSend->m_operationMode = opMode_udp;
			pPerIoSend->m_pDataBuf = row->buf;
			pPerIoSend->m_remoteAddr.sin_family = AF_INET;
			pPerIoSend->m_remoteAddr.sin_addr.s_addr = m_remoteIp;
			pPerIoSend->m_remoteAddr.sin_port = m_remotePort;
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

	lock.Unlock();
	
	if (this->m_listener != NULL)
	{
		this->m_listener->onSendDataAck(nEngineId, nConnId, nId);
	}

	return 1;
}

int TPUDPClient::Connect(const char* szIp, int nPort)
{
	//	先create
	int nRet = Create(opMode_udp);
	if (nRet < 0)
	{
		return -1;
	}

	if (szIp == NULL)
    {
        m_remoteIp = INADDR_ANY;
    }
    else
    {
        m_remoteIp = inet_addr(szIp);
    }

    m_remotePort = htons(nPort);
	
	//	再创建客户端环境
	nRet = CreateClientEnvironment();
	if (nRet < 0)
	{
		goto e_clean;
	}

	//	然后将socket添加到完成端口中
	nRet = AddSocketToIOCP(m_socket, m_pPerHandleData);
	if (nRet >= 0)
	{
		m_pPerHandleData->AddRef();
		m_pPerIoRecv->AddRef();
		nRet = PostRecvToIOCP(m_socket, m_pPerIoRecv);
		if (nRet < 0)
		{
			m_pPerIoRecv->DecRef();
			m_pPerHandleData->DecRef();

			ClearClientEnvironment();
			goto e_clean;
		}
	}
	else
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

int TPUDPClient::Connect(const char* szLocalIp, int nLocalPort, const char* szRemoteIp, int nRemotePort)
{
	m_localIp = inet_addr(szLocalIp);
    m_localPort = htons(nLocalPort);

	return Connect(szRemoteIp, nRemotePort);
}

int TPUDPClient::Send(int id, IBufferRef pSendBuf)
{
	int nRet = -1;
	
	CReadWriteMutexLock lock(m_csSendQueue);
	
	int nSize = m_lstSendQueue.size();
	if (nSize <= m_nSendQueueSize)
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
				pPerIoSend->m_operationMode = opMode_udp;
				pPerIoSend->m_pDataBuf = pSendBuf;
				pPerIoSend->m_remoteAddr.sin_family = AF_INET;
				pPerIoSend->m_remoteAddr.sin_addr.s_addr = m_remoteIp;
				pPerIoSend->m_remoteAddr.sin_port = m_remotePort;
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

	return nRet;
}

int TPUDPClient::Close()
{
	ClearClientEnvironment();

	CReadWriteMutexLock lock(m_csSendQueue);
	while (!m_lstSendQueue.empty())
	{
		DataRow *conn = m_lstSendQueue.front();
		if (conn != NULL)
		{
			delete conn;
		}
		m_lstSendQueue.pop();
	}
	lock.Unlock();

	return 1;
}

int	TPUDPClient::CreateClientEnvironment()
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
	m_pPerIoRecv->m_operationMode = opMode_udp;
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

int	TPUDPClient::ClearClientEnvironment()
{
	if (m_pPerHandleData != NULL)
	{
		//	停止处理线程处理
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