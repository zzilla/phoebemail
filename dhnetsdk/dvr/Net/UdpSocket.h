/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：UDP类
* 摘　要：UDP方式传输。
*
*/
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_UDPSOCKET_H__7F3A8338_7091_44DE_844A_D4750FC492F3__INCLUDED_)
#define AFX_UDPSOCKET_H__7F3A8338_7091_44DE_844A_D4750FC492F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if (defined(WIN32) && !(defined(NETSDK_VERSION_SSL)))
#include "../../TPLayer_IOCP/TPUDPClient.h"
#else
#include "../../TPLayer_Select/TPUDPClient.h"
using namespace NET_TOOL;
#endif

#ifndef HEADER_SIZE
#define HEADER_SIZE  32
#endif


//	UDP接收回调函数
typedef int (__stdcall *OnReceivePacketFunc)(unsigned char *pbuf, int nlen, void *userdata);

//	数据回调函数(用在码流统计)
typedef int (__stdcall *OnDataPacketFunc)(unsigned char *pbuf, int nlen, void *userdata);


class CUdpSocket : public TPUDPClient, public ITPListener
{
public:
	CUdpSocket();
	virtual ~CUdpSocket();
	int		CreateRecvBuf(unsigned int nRecvSize); // 创建接收缓冲


public:
    void	SetCallBack(OnReceivePacketFunc cbReceivePacket, OnDataPacketFunc cbDataPacket, void* userdata);

	int		ConnectHost(const char* szRemoteIp, int nRemotePort, int nReserved=0);

	int		ConnectHost(const char* szLocalIp, int nLocalPort, const char* szRemoteIp, int nRemotePort);

	void	Disconnect();

	int		WriteData(char *pBuf, int nLen);

	int		GetSockLocalAddr(char *szLocalIp, int &nLocalPort);


public:	// event
	virtual int onData(int nEngineId, int nConnId, unsigned char* data, int nLen);

	virtual int	onDealData(int nEngineId, int nConnId, unsigned char* buffer, int nLen);

	virtual int onSendDataAck(int nEngineId, int nConnId, int nId);

	virtual int onConnect(int nEngineId, int nConnId, char* szIp, int nPort);

	virtual int onClose(int nEngineId, int nConnId);

	virtual int onDisconnect(int nEngineId, int nConnId);

	virtual int onReconnect(int nEngineId, int nConnId);

public:
#ifdef NETSDK_VERSION_SSL
	int		SetSSL(int nEnable);
#endif


private:
	int		GetData(unsigned char* buf, int len);
	

private:
	OnReceivePacketFunc	m_pRecvPakcet;
	OnDataPacketFunc	m_pDataPacket;
    void*				m_pUserData;

	// 简易缓冲管理
    unsigned int		m_nWritePos;
    unsigned int		m_nReadPos;
	unsigned char		*m_pPacketBuf;
	unsigned int		m_nBufSize;
	CReadWriteMutex		m_csBuffer;
	CReadWriteMutex		m_csOutCallBack;
};

#endif // !defined(AFX_UDPSOCKET_H__7F3A8338_7091_44DE_844A_D4750FC492F3__INCLUDED_)
