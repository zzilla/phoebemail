/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：TCP客户端类
* 摘　要：TCP方式传输。
*
*/
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCPSOCKET_H__8E1B116D_3800_4C99_A666_8413ABE8E9F7__INCLUDED_)
#define AFX_TCPSOCKET_H__8E1B116D_3800_4C99_A666_8413ABE8E9F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32
#pragma comment( lib, "Ws2_32.lib")
#ifdef NETSDK_VERSION_SSL
#pragma comment( lib, "libeay32.lib")
#pragma comment( lib, "ssleay32.lib")
#endif
#endif

#if (defined(WIN32) && !(defined(NETSDK_VERSION_SSL)))
#include "../../TPLayer_IOCP/TPTCPClient.h"
#include "../../TPLayer_IOCP/TPTCPServer.h"
#else
#include "../../TPLayer_Select/TPTCPClient.h"
#include "../../TPLayer_Select/TPTCPServer.h"
using namespace NET_TOOL;
#endif

#ifndef HEADER_SIZE
#define HEADER_SIZE  32
#endif


//	断线回调函数
typedef int (__stdcall *OnDisConnectFunc)(void *userdata);

//	重新已连接回调函数
typedef int	(__stdcall *OnReConnectFunc)(void *userdata);

//	非指定消息回调处理函数
typedef int (__stdcall *OnNormalPacketFunc)(unsigned char *pbuf, int nlen, void *userdata);

//	接收回调函数(用在码流统计)
typedef int (__stdcall *OnRecPacketFunc)(unsigned char *pbuf, int nlen, void *userdata);

//	侦听回调函数
typedef void (__stdcall *OnListenSockFunc)(void* caller, char *ip, int port, int type, void *value, void *userdata);


//////////////////////////////////////////////////////////////////////////

class CTcpSocket : virtual public TPTCPServer, virtual public TPTCPClient, public ITPListener
{
public:
	CTcpSocket();
	virtual ~CTcpSocket();
	int		CreateRecvBuf(unsigned int nRecvSize); // 创建接收缓冲

	
public:
	static int InitNetwork();
	static int ClearNetwork();


public:	// client
    void	SetCallBack(OnDisConnectFunc cbDisConnect, OnReConnectFunc cbReconnect, OnNormalPacketFunc cbNormalPacket, OnRecPacketFunc cbReceivePacket, void* userdata);

	void	SetKeepLife(unsigned char *szLifePacket, int nBufLen, unsigned int nKeepLifeTime=10);

	int		ConnectHost(const char *szIp, int nPort, int nTimeOut = 1500);

	void	Disconnect();

	int		WriteData(char *pBuf, int nLen);

	int		Heartbeat();


public:	// server
	int		StartListen(const char *szIp, int nPort, OnListenSockFunc cbListenSock, void *userdata);
	
	int		StopListen();

	virtual int	DealNewSocket(SOCKET newsock, int connId, char* ip, int port);

	int		SetSocket(SOCKET newsock, int connId, const char* ip, int port, OnListenSockFunc cbListen, void* listenuserdata, void* pListenSocket);

	int		ResponseReg(bool bAccept);

	
public:	// event
	virtual int onData(int nEngineId, int nConnId, unsigned char* data, int nLen);

	virtual int	onDealData(int nEngineId, int nConnId, unsigned char* buffer, int nLen);

	virtual int onSendDataAck(int nEngineId, int nConnId, int nId);

	virtual int onConnect(int nEngineId, int nConnId, char* szIp, int nPort);

	virtual int onClose(int nEngineId, int nConnId);

	virtual int onDisconnect(int nEngineId, int nConnId);

	virtual int onReconnect(int nEngineId, int nConnId);


public:

	int		SetIsReConn(int nEnable);

	int		SetIsDetectDisconn(int nEnable);

	int		GetIsOnline();

	void	DealSpecialPacket(unsigned char *pbuf, int nlen);

	int		CloseSubConn();	// 关闭子连接

	int		ConnectSubConn(); // 子连接重连

#ifdef NETSDK_VERSION_SSL
	int		SetSSL(int nEnable);
#endif


private:
	int		GetData(unsigned char* buf, int len);

#ifdef NETSDK_VERSION_BOGUSSSL
	int		DealSSL();
#endif


public:
	OS_EVENT			m_hRecEvent;		// 同步接收数据的事件
	unsigned char		m_registerAck[64];
	int					m_nRegisterLen;
		
#ifdef NETSDK_VERSION_BOGUSSSL
	OS_EVENT			m_hSpecialEvent;
	int					m_nSSL;	
#endif


private:
	OnDisConnectFunc	m_pDisConnect;
    OnReConnectFunc		m_pReconnect;
    OnNormalPacketFunc	m_pNormalPacket;
	OnRecPacketFunc		m_pRecvPakcet;
    void*				m_pUserData;

	OnListenSockFunc	m_pListenSockFunc;
	void*				m_pListenUserData;
	void*				m_pListenSocket;

	// 简易缓冲管理
    unsigned int		m_nWritePos;
    unsigned int		m_nReadPos;
	unsigned char		*m_pPacketBuf;
	unsigned int		m_nBufSize;
	CReadWriteMutex		m_csBuffer;
	CReadWriteMutex		m_csOutCallBack;
};

#endif // !defined(AFX_TCPSOCKET_H__8E1B116D_3800_4C99_A666_8413ABE8E9F7__INCLUDED_)
