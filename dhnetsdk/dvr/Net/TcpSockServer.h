/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：TCP服务器类
* 摘　要：TCP服务器。
*
*/
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ALARMSERVER_H__9376B537_0FCB_40E1_BD4C_DD1B60FFBD41__INCLUDED_)
#define AFX_ALARMSERVER_H__9376B537_0FCB_40E1_BD4C_DD1B60FFBD41__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if (defined(WIN32) && !(defined(NETSDK_VERSION_SSL)))
#include "../../TPLayer_IOCP/TPTCPServer.h"
#else
#include "../../TPLayer_Select/TPTCPServer.h"
using namespace NET_TOOL;
#endif

//	接收回调函数
typedef int (CALLBACK *fEventCallBack)(LONG lHandle, int connId, char *szIp, WORD wPort, LONG lCommand, void *pParam, DWORD dwParamLen, DWORD dwUserData);


//////////////////////////////////////////////////////////////////////////

class CTcpSockServer :public TPTCPServer, public ITPListener
{
public:
	CTcpSockServer();
	virtual ~CTcpSockServer();

public:
	static int InitNetwork();
	static int ClearNetwork();
	
public:	// interface
	int		StartListen(const char *szIp, int nPort, fEventCallBack cbRecAlarmFunc, DWORD dwUserdata);
	
	int		StopListen();

	int		HeartBeat();
	
public:	// event
	virtual int onData(int nEngineId, int nConnId, unsigned char* data, int nLen);

	virtual int	onDealData(int nEngineId, int nConnId, unsigned char* buffer, int nLen);

	virtual int onSendDataAck(int nEngineId, int nConnId, int nId);

	virtual int onConnect(int nEngineId, int nConnId, char* szIp, int nPort);

	virtual int onClose(int nEngineId, int nConnId);

	virtual int onDisconnect(int nEngineId, int nConnId);

	virtual int onReconnect(int nEngineId, int nConnId);

private:
	fEventCallBack	m_cbServer;
	DWORD			m_dwUserData;
};

#endif // !defined(AFX_ALARMSERVER_H__9376B537_0FCB_40E1_BD4C_DD1B60FFBD41__INCLUDED_)
