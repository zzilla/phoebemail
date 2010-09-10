// BroadCast.cpp: implementation of the BroadCast class.
//
//////////////////////////////////////////////////////////////////////

#include "../StdAfx.h"
#include "Broadcast.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBroadcast::CBroadcast() : TPBroadcast(this)
{
	m_pPacketBuf = NULL;
	m_nBufSize = 0;
	m_nWritePos = 0;
	m_nReadPos = 0;
	
	m_pRecvPakcet = NULL;
    m_pUserData = NULL;
}

int	CBroadcast::CreateRecvBuf(unsigned int nRecvSize)
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

CBroadcast::~CBroadcast()
{
	if (m_pPacketBuf != NULL)
	{
		delete m_pPacketBuf;
		m_pPacketBuf = NULL;
	}
	m_nBufSize = 0;
}

void CBroadcast::SetCallBack(OnBroadcastPacketFunc cbReceivePacket, void *userdata)
{
	m_pRecvPakcet = cbReceivePacket;
    m_pUserData = userdata;
}

int	CBroadcast::ConnectHost(int nRemotePort)
{
	return Connect(NULL, nRemotePort);
}

int	CBroadcast::ConnectHost(const char* szLocalIp, int nLocalPort, int nRemotePort)
{
	return Connect(szLocalIp, nLocalPort, NULL, nRemotePort);
}

void CBroadcast::Disconnect()
{
	Close();
}

int	CBroadcast::WriteData(char *pBuf, int nLen)
{
	IBufferRef pDataBuf = CAutoBuffer::CreateBuffer(nLen, pBuf, true);
	if (pDataBuf.IsEmpty())
	{
		return -1;
	}

	return Send(0, pDataBuf);
}

int CBroadcast::onData(int nEngineId, int nConnId, unsigned char* data, int nLen)
{
	int nRet = 1;
	if (m_pPacketBuf == NULL)
	{
		return nRet;
	}
	
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
int	CBroadcast::onDealData(int nEngineId, int nConnId, unsigned char* buffer, int nLen)
{
	int nRet = 1;
	if (m_pPacketBuf == NULL)
	{
		return nRet;
	}

	int nPacketLen = GetData(buffer, nLen);
	if (nPacketLen > 0)
	{
		if (m_pRecvPakcet)
		{
 			m_pRecvPakcet(this, buffer, nPacketLen, NULL, m_pUserData);
		}

		nRet = 0;
	}

	return nRet;
}

int CBroadcast::onSendDataAck(int nEngineId, int nConnId, int nId)
{
	return 1;
}

int CBroadcast::onConnect(int nEngineId, int nConnId, char* szIp, int nPort)
{
	return 1;
}

int CBroadcast::onClose(int nEngineId, int nConnId)
{
	return 1;
}

int CBroadcast::onDisconnect(int nEngineId, int nConnId)
{
	return 1;
}

int CBroadcast::onReconnect(int nEngineId, int nConnId)
{
	return 1;
}

int	CBroadcast::GetData(unsigned char* buf, int len)
{
	int nDataLen = 0;
	
	CReadWriteMutexLock lock(m_csBuffer);
	
	if ((int)(m_nWritePos - m_nReadPos) >= HEADER_SIZE)
	{
		unsigned int extlen = *(unsigned int*)(m_pPacketBuf + m_nReadPos + 4);
		extlen += *(unsigned char*)(m_pPacketBuf + m_nReadPos + 2);
		
		if ((extlen + HEADER_SIZE) >= len)
		{
			//	指示的扩展数据太长，不正常，清空缓存
#ifdef DEBUG
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


