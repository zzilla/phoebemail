/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：广播类
* 摘　要：用于搜索网络上在线设备。
*
*/
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BROADCAST_H__047412C4_2CB1_4629_9AF2_004E57BF9FB9__INCLUDED_)
#define AFX_BROADCAST_H__047412C4_2CB1_4629_9AF2_004E57BF9FB9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if (defined(WIN32) && !(defined(NETSDK_VERSION_SSL)))
#include "../../TPLayer_IOCP/TPBroadcast.h"
#else
#include "../../TPLayer_Select/TPBroadcast.h"
using namespace NET_TOOL;
#endif

#ifndef HEADER_SIZE
#define HEADER_SIZE  32
#endif


//	接收回调函数
typedef int (__stdcall *OnBroadcastPacketFunc)(void * object, unsigned char *pbuf, unsigned int nlen, void* param, void *userdata);


class CBroadcast : public TPBroadcast, public ITPListener
{
public:
	CBroadcast();
	virtual ~CBroadcast();
	int		CreateRecvBuf(unsigned int nRecvSize); // 创建接收缓冲


public: // 接口
	void	SetCallBack(OnBroadcastPacketFunc cbReceivePacket, void *userdata);
	
	int		ConnectHost(int nRemotePort);
	
	int		ConnectHost(const char* szLocalIp, int nLocalPort, int nRemotePort);
	
	void	Disconnect();
	
	int		WriteData(char *pBuf, int nLen);


public:	// event
	virtual int onData(int nEngineId, int nConnId, unsigned char* data, int nLen);

	virtual int	onDealData(int nEngineId, int nConnId, unsigned char* buffer, int nLen);

	virtual int onSendDataAck(int nEngineId, int nConnId, int nId);

	virtual int onConnect(int nEngineId, int nConnId, char* szIp, int nPort);

	virtual int onClose(int nEngineId, int nConnId);

	virtual int onDisconnect(int nEngineId, int nConnId);

	virtual int onReconnect(int nEngineId, int nConnId);


private:
	int		GetData(unsigned char* buf, int len);

	
private:
	OnBroadcastPacketFunc	m_pRecvPakcet;
    void*				m_pUserData;
	
	// 简易缓冲管理
    unsigned int		m_nWritePos;
    unsigned int		m_nReadPos;
	unsigned char		*m_pPacketBuf;
	unsigned int		m_nBufSize;
	CReadWriteMutex		m_csBuffer;
};

#endif // !defined(AFX_BROADCAST_H__047412C4_2CB1_4629_9AF2_004E57BF9FB9__INCLUDED_)
