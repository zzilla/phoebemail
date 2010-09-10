/*
* Copyright (c) 2008, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 文件名称：TPTypedef.h
* 摘　　要：传输层预定义
*
*
* 取代版本：0.1
* 原作者　：Peng Dongfeng
* 完成日期：2008年6月16日
* 修订记录：创建
*/
//////////////////////////////////////////////////////////////////////

#ifndef _TPETYPEDEF_H_
#define _TPETYPEDEF_H_

#pragma   warning(disable:4996)
#pragma   warning(disable:4786)

#include "winsock2i.h"
#include "./include/ReadWriteMutex.h"
using namespace SPTools;
#include "./include/AutoBuffer.h"

#include <queue>
#include <map>


typedef enum
{
	TP_SEND = 1,
	TP_RECEIVE
} TP_DATA_TYPE;

typedef struct __DataRow 
{
	unsigned int	id;
	int				socket;
	IBufferRef		buf;
} DataRow;


//////////////////////////////////////////////////////////////////////////

typedef enum _opType
{
	opType_none = 0,
	opType_recv,
	opType_send,
	opType_accept
} opType;

typedef enum _opMode
{
	opMode_none = 0,
	opMode_tcp,
	opMode_udp
} opMode;

class IOCPListener
{
public:
	virtual int onIOData(int nEngineId, int nConnId, unsigned char* data, int nLen, BOOL bPostSuccess) = 0;

	virtual int onIODealData(int nEngineId, int nConnId, unsigned char* buffer, int nLen) = 0;

	virtual int onIODisconnect(int nEngineId, int nConnId) = 0;

	virtual int onIOSendDataAck(int nEngineId, int nConnId, int nId, BOOL bPostSuccess) = 0;
};

class CPerIoData
{
public:
	CPerIoData(): m_lLifeCount(1)
	{
		memset(&m_ol, 0, sizeof(OVERLAPPED));
		memset(&m_remoteAddr, 0, sizeof(struct sockaddr_in));
		m_nFromLen		= sizeof(struct sockaddr_in);
		m_wsabuf.buf	= NULL;
		m_wsabuf.len	= 0;
		m_operationType = opType_none;
		m_operationMode = opMode_none;
		m_pDataBuf		= NULL;

		m_lTotalBytes	= 0;
		m_lSentBytes	= 0;
		m_id			= 0;
		m_nErrorCount	= 0;
	}
	
	virtual ~CPerIoData()
	{
	}

	//	只有对象存在，才能增加计数；可以通过链表来管理
	int AddRef()
	{
		if (InterlockedIncrement(&m_lLifeCount) <= 1)
		{
			InterlockedDecrement(&m_lLifeCount);
			return 0;
		}
		
		return m_lLifeCount;
	}
	
	int DecRef()
	{
		if (m_lLifeCount <= 0)
		{
			return 0;
		}
		
		if (InterlockedDecrement(&m_lLifeCount) <= 0)
		{
			delete this;
			return 0;
		}
		
		return m_lLifeCount;
	}
	
	OVERLAPPED		m_ol;
	WSABUF			m_wsabuf;
	opType			m_operationType;
	opMode			m_operationMode;
	IBufferRef		m_pDataBuf;
	struct sockaddr_in m_remoteAddr;	// 发送地址或接收地址
	int				m_nFromLen;
	LONG			m_lLifeCount;
	LONG			m_lTotalBytes;		// 须发送的总字节数
	LONG			m_lSentBytes;		// 已经发送的字节数
	LONG			m_id;
	int				m_nErrorCount;
};

class CPerHandleData
{
public:
	CPerHandleData(): m_lLifeCount(1)
	{
		m_socket	= INVALID_SOCKET;
		m_engineId	= 0;
		m_connId	= 0;
		m_listener	= NULL;
	}

	virtual ~CPerHandleData()
	{

	}
	
	int AddRef()
	{
		if (InterlockedIncrement(&m_lLifeCount) <= 1)
		{
			InterlockedDecrement(&m_lLifeCount);
			return 0;
		}
		
		return m_lLifeCount;
	}
	
	int DecRef()
	{
		if (m_lLifeCount <= 0)
		{
			return 0;
		}
		
		if (InterlockedDecrement(&m_lLifeCount) <= 0)
		{
			delete this;
			return 0;
		}
		
		return m_lLifeCount;
	}
	
	SOCKET			m_socket;
	LONG			m_lLifeCount;
	int				m_engineId;
	int				m_connId;
	IOCPListener*	m_listener;
	CReadWriteMutex	m_pcsCallBack;	//	上层用来对回调停止的控制
};

//////////////////////////////////////////////////////////////////////////


class CConnClientData
{
public:
	CConnClientData(): m_lLifeCount(1)
	{
		m_socket	= INVALID_SOCKET;
		m_localIp	= INADDR_ANY;
		m_localPort	= 0;
		m_remoteIp	= INADDR_ANY;
		m_remotePort = 0;

		m_pPerIoRecv = NULL;
		m_pPerHandleData = NULL;
		
		m_id		= 0;
		m_dwLastReceiveTime = 0;
		m_bIsOnline = TRUE;
		m_bPostSend = TRUE;
		m_dwLastDealTime = 0;
		m_bLastStatus = TRUE;
		m_bIsDealData = FALSE;
	}

	virtual ~CConnClientData()
	{
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
	}
	
	int AddRef()
	{
		if (InterlockedIncrement(&m_lLifeCount) <= 1)
		{
			InterlockedDecrement(&m_lLifeCount);
			return 0;
		}
		
		return m_lLifeCount;
	}
	
	int DecRef()
	{
		if (m_lLifeCount <= 0)
		{
			return 0;
		}
		
		if (InterlockedDecrement(&m_lLifeCount) <= 0)
		{
			delete this;
			return 0;
		}
		
		return m_lLifeCount;
	}
	
	LONG			m_lLifeCount;
	SOCKET			m_socket;
	unsigned long	m_localIp;
	unsigned short	m_localPort;
	unsigned long	m_remoteIp;
	unsigned short	m_remotePort;

	CReadWriteMutex	m_pcsData;
	unsigned int	m_id;
	unsigned long	m_dwLastReceiveTime;
	BOOL			m_bIsOnline;

	CPerIoData		*m_pPerIoRecv;
	CPerHandleData	*m_pPerHandleData;

	BOOL			m_bPostSend;
	std::queue<DataRow*> m_lstSendQueue;
	CReadWriteMutex	m_csSendQueue;

	DWORD			m_dwLastDealTime;
	BOOL			m_bLastStatus;
	BOOL			m_bIsDealData;
	CReadWriteMutex m_csDealData;
};

//////////////////////////////////////////////////////////////////////////

typedef std::queue<DataRow*> Queue_List;
typedef std::map<unsigned int, CConnClientData*> CONN_MAP;
typedef std::list<CPerHandleData*> DEAL_LIST;

//////////////////////////////////////////////////////////////////////////


#endif	//	_TPETYPEDEF_H_