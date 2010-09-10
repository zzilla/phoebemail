// TcpSocket.cpp: implementation of the CTcpSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TcpSocket.h"
#include "DataCenter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#pragma   warning(disable:4355)
CTcpSocket::CTcpSocket() : TPTCPServer(this), TPTCPClient(this)
{
	m_pDataCenter = NULL;
	m_nSessionId = 0;
	m_bConnType = TRUE;

	m_nWritePos = 0;
	m_nReadPos = 0;
}

CTcpSocket::~CTcpSocket()
{
	
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
int CTcpSocket::onData(int engineId, int connId, const char* data, int len)
{
	CReadWriteMutexLock lock(m_csBuffer);

    if (len > 0)
    {
        /***********************缓冲数据***********************/
        //	现在当包长大于存储空间时采取丢包的原则
        //	情况可能有：调试中断太久和收到错误数据包

        int nEndPos = len + m_nWritePos;
        
		//	如果缓冲区足够缓冲数据
        if (nEndPos <= MAX_BUF_SIZE)
        {
            memcpy(m_packetBuf + m_nWritePos, data, len);
            m_nWritePos = m_nWritePos + len;
        }
		//	如果缓冲区不足以缓冲数据,从头再来
        else
        {   
			//	整个缓冲区都不足以容纳该数据，一般是因为调试中断太久或错误数据包
            if (len + (m_nWritePos-m_nReadPos) >= MAX_BUF_SIZE)
            {
				len = 0;
            }
			else
            {
				memmove(m_packetBuf, m_packetBuf + m_nReadPos, m_nWritePos - m_nReadPos);
        
				m_nWritePos = m_nWritePos - m_nReadPos;
				m_nReadPos = 0;				

				if (len > 0)
				{
					memcpy(m_packetBuf + m_nWritePos, data, len);
					m_nWritePos = m_nWritePos + len;
				}
			}
		}
    }

	return 1;
}

/*
 *	摘要：处理数据
 *	返回值：0：忙；1：空闲
 */
int	CTcpSocket::onDealData(int engineId, int connId)
{
	int nRet = 1;

	int len = GetData();
	if (len > 0)
	{
		if (0xA0 == m_dealBuf[0])
		{
			//	登入包
			if (m_nSessionId == 0)
			{
				m_nSessionId = TPTCPClient::m_socket;
				m_bConnType = TRUE;

				//	控制权从连接列表转到用户连接列表中
				m_pDataCenter->AddUserConn(m_nSessionId, this);
			}
			
			BYTE *pCommand = new BYTE[32];
			memset(pCommand, 0, 32);
			pCommand[0] = 0xB0;
			pCommand[3] = 0x50;
			pCommand[10] = m_pDataCenter->m_nDevChannel;
			pCommand[11] = 0x09;
			pCommand[12] = 0x02;
			*(DWORD*)(pCommand+16) = m_nSessionId;
			
			IBufferRef pCommandData = CAutoBuffer::CreateBuffer(32, (char*)pCommand, false);
			WriteData(pCommandData);
		}
		else if (0x11 == m_dealBuf[0])
		{
			//	监视包
			if (m_nSessionId != 0)
			{
				m_pDataCenter->UserConnPlay(m_nSessionId, m_dealBuf, len);
			}
		}
		else if (0xF1 == m_dealBuf[0])
		{
			//	子连接
			m_nSessionId = *(DWORD*)(m_dealBuf+8);
			m_bConnType = FALSE;
			int nChannel = m_dealBuf[13] - 1;
			
			m_pDataCenter->UserConnSubConn(m_nSessionId, this, TPTCPClient::m_socket, nChannel);
		}
		else if (0xA1 == m_dealBuf[0])
		{
			//	心跳包
			BYTE *pCommand = new BYTE[32];
			memset(pCommand, 0, 32);
			pCommand[0] = 0xB1;
			
			IBufferRef pCommandData = CAutoBuffer::CreateBuffer(32, (char*)pCommand, false);
			WriteData(pCommandData);
		}
		
		nRet = 0;
	}

	return nRet;
}

/*
 *	摘要：发送数据完成回调，从发送队列中取数据继续发送
 */
int CTcpSocket::onSendDataAck(int engineId, int connId, int id)
{
	return 1;
}

/*
 *	摘要：作服务器用才回调，有新的client连接，返回值为0表示接受此连接，返回值为1表示拒绝接受
 */
int CTcpSocket::onConnect(int engineId, int connId, const char* ip, int port)
{
	return 1;
}

/*
 *	摘要：作服务器用才回调，client退出回调，删除客户端连接列表
 */
int CTcpSocket::onClose(int engineId, int connId)
{
	return 1;
}

/*
 *	摘要：作客户端用才回调，断线回调，内部可能重连，由应用层决定是否close
 */
int CTcpSocket::onDisconnect(int engineId, int connId)
{
	if (m_nSessionId != 0 && m_bConnType)
	{
		m_pDataCenter->DelUserConn(m_nSessionId, this);
	}
	else
	{
		m_pDataCenter->DelConn(TPTCPClient::m_socket, this);
	}
	
	return 1;
}

/*
 *	摘要：作客户端用才回调，断线后重连成功回调，恢复所有的业务(操作)
 */
int CTcpSocket::onReconnect(int engineId, int connId)
{
	return 1;
}

//////////////////////////////////////////////////////////////////////////

int	CTcpSocket::WriteData(IBufferRef pSendBuf)
{
	return TPTCPClient::Send(0, pSendBuf);
}

void CTcpSocket::Disconnect()
{
	TPTCPClient::Close();
}

int	CTcpSocket::Heartbeat()
{
	return TPTCPClient::Heartbeat();
}

int	CTcpSocket::GetData()
{
	int nDataLen = 0;

	CReadWriteMutexLock lock(m_csBuffer);

	if ((int)(m_nWritePos - m_nReadPos) >= HEADER_SIZE)
	{
		unsigned int extlen = *(unsigned int*)(m_packetBuf + m_nReadPos + 4);

		if (extlen >= MAX_PACKET_SIZE)
		{
			//	指示的扩展数据太长，不正常，清空缓存
			m_nWritePos = m_nReadPos = 0;

			return 0;
		}

		if (m_nWritePos - m_nReadPos >= extlen + HEADER_SIZE)
        {
			nDataLen = extlen + HEADER_SIZE;
			memcpy(m_dealBuf, m_packetBuf + m_nReadPos, nDataLen);

			m_nReadPos += nDataLen;
        }
	}

	return nDataLen;
}

//////////////////////////////////////////////////////////////////////////

int	CTcpSocket::StartServer(const char *szIp, int nPort, CDataCenter *pDataCenter)
{
	m_pDataCenter = pDataCenter;
	return TPTCPServer::Listen((char*)szIp, nPort);
}

int	CTcpSocket::StopServer()
{
	return TPTCPServer::Close();
}

int	CTcpSocket::DealNewSocket(SOCKET newsock, int connId, const char* ip, int port)
{
	CTcpSocket *pNewSocket = new CTcpSocket();
	if (pNewSocket == NULL || m_pDataCenter == NULL)
	{
		closesocket(newsock);
		return -1;
	}

	//	变成客户端的连接类
	int ret = pNewSocket->SetSocket(newsock, connId, ip, port, m_pDataCenter);
	if (ret >= 0)
	{
		CReadWriteMutexLock lock(m_pDataCenter->m_csConn);
		m_pDataCenter->m_lstConns[newsock] = pNewSocket;
	}

	return 1;
}

int	CTcpSocket::SetSocket(SOCKET newsock, int connId, const char* ip, int port, CDataCenter *pDataCenter)
{
	int nRet = -1;
	CPerHandleData *pPerHandleData = NULL;

	TPTCPClient::m_bReconnEn = FALSE;
	TPTCPClient::m_socket = newsock;
	TPTCPClient::m_remoteIp = inet_addr(ip);
	TPTCPClient::m_remotePort = htons(port);
	TPTCPClient::m_bOnline = TRUE;
	m_pDataCenter = pDataCenter;

	int ret = TPTCPClient::CreateClientEnvironment();
	if (ret >= 0)
	{
		//	将socket添加到完成端口中
		ret = TPTCPClient::AddSocketToIOCP(TPTCPClient::m_socket, TPTCPClient::m_pPerHandleData);
		if (ret >= 0)
		{
			pPerHandleData = TPTCPClient::m_pPerHandleData;

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
	}
	
	if (nRet < 0)
	{
		TPTCPClient::DelSocketFromIOCP(newsock, pPerHandleData);
	}

	return nRet;
}