/*
* Copyright (c) 2008, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 文件名称：TPUDPClient.h
* 摘　　要：UDP客户端功能类
*
*
* 取代版本：0.1
* 原作者　：Peng Dongfeng
* 完成日期：2008年8月6日
* 修订记录：创建
*/
//////////////////////////////////////////////////////////////////////

#ifndef _TPUDPClient_H_
#define _TPUDPClient_H_

#include "ITPObject.h"


class TPUDPClient : public ITPObject, protected IOCPListener
{
public:
    TPUDPClient(ITPListener *callback, int engineId = 0);
    virtual ~TPUDPClient();


public:	/* 接口 */
    virtual int Close(void);

    virtual int Connect(const char* szIp, int nPort);

	virtual int Connect(const char* szLocalIp, int nLocalPort, const char* szRemoteIp, int nRemotePort);

    virtual int Send(int id, IBufferRef pSendBuf);


public:	/* 内部完成端口回调 */
	virtual int onIOData(int nEngineId, int nConnId, unsigned char* data, int nLen, BOOL bPostSuccess);

	virtual int onIODealData(int nEngineId, int nConnId, unsigned char* buffer, int nLen);

	virtual int onIODisconnect(int nEngineId, int nConnId);

	virtual int onIOSendDataAck(int nEngineId, int nConnId, int nId, BOOL bPostSuccess);


protected:
	int		CreateClientEnvironment();
	int		ClearClientEnvironment();


protected:
	CPerIoData		*m_pPerIoRecv;		// Receive PerIoData
	CPerHandleData	*m_pPerHandleData;	// Socket PerHandle

	BOOL			m_bIsPostSend;		// 是否可以投递发送
	Queue_List		m_lstSendQueue;
	CReadWriteMutex	m_csSendQueue;

	DWORD			m_dwLastDealTime;
	BOOL			m_bLastStatus;
	BOOL			m_bIsDealData;
	CReadWriteMutex m_csDealData;
};

#endif // _TPUDPClient_H_
