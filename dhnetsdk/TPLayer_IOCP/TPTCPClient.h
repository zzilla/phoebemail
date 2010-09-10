/*
* Copyright (c) 2008, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 文件名称：TPTCPClient.h
* 摘　　要：TCP客户端功能类
*
*
* 取代版本：0.1
* 原作者　：Peng Dongfeng
* 完成日期：2008年6月16日
* 修订记录：创建
*/
//////////////////////////////////////////////////////////////////////

#ifndef _TPTCPClient_H_
#define _TPTCPClient_H_

#include "ITPObject.h"


class TPTCPClient : public ITPObject, protected IOCPListener
{
public:
    TPTCPClient(ITPListener *tcpclientapp, int engineId = 0);
    virtual ~TPTCPClient();


public:	/* 接口 */
    virtual int Close(void);

    virtual int Connect(const char* szIp, int nPort, int nTimeOut=1500);

	virtual int Connect(const char* szLocalIp, int nLocalPort, const char* szRemoteIp, int nRemotePort);

    virtual int Send(int id, IBufferRef pSendBuf);

	//	设置心跳包信息
	virtual int	SetKeepLifePacket(unsigned char *pLifePacketBuf, int nBufLen, unsigned int nKeepLifeTime);

	//	发送心跳包，检测断线，断线重连
    virtual int Heartbeat(void);


public:	/* 内部完成端口回调 */

	virtual int onIOData(int nEngineId, int nConnId, unsigned char* data, int nLen, BOOL bPostSuccess);

	virtual int onIODealData(int nEngineId, int nConnId, unsigned char* buffer, int nLen);

	virtual int onIODisconnect(int nEngineId, int nConnId);

	virtual int onIOSendDataAck(int nEngineId, int nConnId, int nId, BOOL bPostSuccess);


protected:
	int		CreateClientEnvironment();
	int		ClearClientEnvironment();
	int		closeInside();
	int		IsConnected();


protected:
	CPerIoData		*m_pPerIoRecv;		// Receive PerIoData
	CPerHandleData	*m_pPerHandleData;	// Socket PerHandle

	BOOL			m_bIsPostSend;		// 是否可以投递发送
	Queue_List		m_lstSendQueue;
	CReadWriteMutex	m_csSendQueue;
	
	int				m_nLifePacketLen;
	IBufferRef		m_lifePacketBuf;						// 心跳包数据
	unsigned int	m_nKeepLifeTime;						// 心跳间隔
	unsigned long	m_dwLastReceiveTime;					// 最近一次接收数据时间
	unsigned long	m_dwLastHeartBeatTime;					// 最近一次发送心跳包时间

	BOOL			m_bCreateEnable;
	unsigned long	m_dwLastConTime;	// 最近一次connect时间
	BOOL			m_bOnline;			// 是否在线
	CReadWriteMutex	m_csOnline;

	DWORD			m_dwLastDealTime;
	BOOL			m_bLastStatus;
	int				m_nDealData;
	CReadWriteMutex m_csDealData;
};

#endif // _TPTCPClient_H_
