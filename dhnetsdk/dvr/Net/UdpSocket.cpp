// UdpSocket.cpp: implementation of the CUdpSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "../StdAfx.h"
#include "UdpSocket.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#pragma   warning(disable:4355)
CUdpSocket::CUdpSocket() : TPUDPClient(this)
{
	m_pPacketBuf = NULL;
	m_nBufSize = 0;
	m_nWritePos = 0;
	m_nReadPos = 0;

	m_pRecvPakcet = NULL;
	m_pDataPacket = NULL;
    m_pUserData = NULL;
}

int	CUdpSocket::CreateRecvBuf(unsigned int nRecvSize)
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

CUdpSocket::~CUdpSocket()
{
	if (m_pPacketBuf != NULL)
	{
		delete m_pPacketBuf;
		m_pPacketBuf = NULL;
	}
	m_nBufSize = 0;
}

void CUdpSocket::SetCallBack(OnReceivePacketFunc cbReceivePacket, OnDataPacketFunc cbDataPacket, void* userdata)
{
	CReadWriteMutexLock lock(m_csOutCallBack);

	m_pUserData = userdata;
	m_pRecvPakcet = cbReceivePacket;
	m_pDataPacket = cbDataPacket;

	lock.Unlock();
}

int	CUdpSocket::ConnectHost(const char* szRemoteIp, int nRemotePort, int nReserved)
{
	return Connect(szRemoteIp, nRemotePort);
}

int	CUdpSocket::ConnectHost(const char* szLocalIp, int nLocalPort, const char* szRemoteIp, int nRemotePort)
{
	return Connect(szLocalIp, nLocalPort, szRemoteIp, nRemotePort);
}

void CUdpSocket::Disconnect()
{
	Close();
}

int	CUdpSocket::WriteData(char *pBuf, int nLen)
{
	IBufferRef pDataBuf = CAutoBuffer::CreateBuffer(nLen, pBuf, true);
	if (pDataBuf.IsEmpty())
	{
		return -1;
	}

	return Send(0, pDataBuf);
}

int	CUdpSocket::GetSockLocalAddr(char *szLocalIp, int &nLocalPort)
{
	return GetSocketBindInfo(szLocalIp, nLocalPort);
}

int CUdpSocket::onData(int nEngineId, int nConnId, unsigned char* data, int nLen)
{
	//	前八个字节是包序号
	if (data == NULL || nLen <= 8 || m_pPacketBuf == NULL)
	{
		return 1;
	}

// 	DWORD dwPacketSeq = *(DWORD*)data;

	CReadWriteMutexLock lock(m_csBuffer);

	int bodylen = nLen - 8;
   	unsigned char * realData = data + 8;

    if (bodylen > 0)
    {
        /***********************缓冲数据***********************/
        //	现在当包长大于存储空间时采取丢包的原则
        //	情况可能有：调试中断太久和收到错误数据包

        int nEndPos = bodylen + m_nWritePos;
        
		//	如果缓冲区足够缓冲数据
        if (nEndPos <= m_nBufSize)
        {
            memcpy(m_pPacketBuf + m_nWritePos, realData, bodylen);
            m_nWritePos = m_nWritePos + bodylen;
        }
		//	如果缓冲区不足以缓冲数据,从头再来
        else
        {   
			//	整个缓冲区都不足以容纳该数据，一般是因为调试中断太久或错误数据包
            if (bodylen + (m_nWritePos-m_nReadPos) >= m_nBufSize)
            {
				bodylen = 0;
            }
			else
            {
				memmove(m_pPacketBuf, m_pPacketBuf + m_nReadPos, m_nWritePos - m_nReadPos);
        
				m_nWritePos = m_nWritePos - m_nReadPos;
				m_nReadPos = 0;				

				if (bodylen > 0)
				{
					memcpy(m_pPacketBuf + m_nWritePos, realData, bodylen);
					m_nWritePos = m_nWritePos + bodylen;
				}
			}
		}
    }

	lock.Unlock();

	return 1;
}

/*
 *	摘要：处理数据
 *	返回值：0：忙；1：空闲
 */
int	CUdpSocket::onDealData(int nEngineId, int nConnId, unsigned char* buffer, int nLen)
{
	int nRet = 1;
	if (m_pPacketBuf == NULL)
	{
		return nRet;
	}

	int nPacketLen = GetData(buffer, nLen);
	if (nPacketLen > 0)
	{
		CReadWriteMutexLock lock(m_csOutCallBack);

		if (m_pDataPacket)
		{
			m_pDataPacket(buffer, nPacketLen, m_pUserData);
		}
		
		if (m_pRecvPakcet)
		{
 			m_pRecvPakcet(buffer, nPacketLen, m_pUserData);
		}

		lock.Unlock();

		nRet = 0;
	}

	return nRet;
}

int CUdpSocket::onSendDataAck(int nEngineId, int nConnId, int nId)
{
	return 1;
}

int CUdpSocket::onConnect(int nEngineId, int nConnId, char* szIp, int nPort)
{
	return 1;
}

int CUdpSocket::onClose(int nEngineId, int nConnId)
{
	return 1;
}

int CUdpSocket::onDisconnect(int nEngineId, int nConnId)
{
	return 1;
}

int CUdpSocket::onReconnect(int nEngineId, int nConnId)
{
	return 1;
}

int	CUdpSocket::GetData(unsigned char* buf, int len)
{
	int nDataLen = 0;

	CReadWriteMutexLock lock(m_csBuffer);

	if ((int)(m_nWritePos - m_nReadPos) >= HEADER_SIZE)
	{
		unsigned int extlen = *(unsigned int*)(m_pPacketBuf + m_nReadPos + 4);

		if ((extlen + HEADER_SIZE) >= len)
		{
			//	指示的扩展数据太长，不正常，清空缓存
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

#ifdef NETSDK_VERSION_SSL
int	CUdpSocket::SetSSL(int nEnable)
{
	TPUDPClient::m_bIsSSL = nEnable;
	return 1;
}
#endif


