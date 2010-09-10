/*
* Copyright (c) 2008, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 文件名称：ITPObject.h
* 摘　　要：传输层接口抽象类，由完成端口模型来实现，只用于Windows系统。
*
*
* 取代版本：0.1
* 原作者　：Peng Dongfeng
* 完成日期：2008年6月16日
* 修订记录：创建
*/
//////////////////////////////////////////////////////////////////////

#ifndef _ITPOBJECT_H_
#define _ITPOBJECT_H_

#include "TPTypedef.h"
#include "ITPListener.h"


#define RECEIVE_BUFFER_SIZE	(64*1024)
#define MAX_PACKET_SIZE		(50*1024)
#define MAX_DEAL_SPANTIME	(10)	//ms

class ITPObject  
{
public:
	ITPObject();
	virtual ~ITPObject();


public:
	//	传输层初始化，在主程序入口
	static int Startup(void);
	static int Cleanup(void);
	
	static long GetNewClientId();
	static long GetNewPort();

	//	增加socket到完成端口
	int	AddSocketToIOCP(SOCKET socket, CPerHandleData *pPerHandleData);

	//	从完成端口中移除socket
	int	DelSocketFromIOCP(SOCKET socket, CPerHandleData *pPerHandleData);

	//	投递一个接收
	int	PostRecvToIOCP(SOCKET socket, CPerIoData *pPerIoRecv);

	//	投递一个发送
	int	PostSendToIOCP(SOCKET socket, CPerIoData *pPerIoSend);


public:	/* 控制类方法 */

	//	设置底层套接字缓冲区大小，type表示是发送缓冲区还是接收缓冲区
	int SetSocketBufferSize(TP_DATA_TYPE type, int size);

	//	获取底层套接字缓冲区大小，type表示是发送缓冲区还是接收缓冲区
	int GetSocketBufferSize(TP_DATA_TYPE type);

	//	设置传输层调用select时的超时值。如果不赋值则默认设sec=0;usec=10;
	//...如果设置为0则表示为轮循状态
	int SetSelectTimeout(long sec, long usec);

	//	设置是否断线重连
	int	SetIsReconnect(int nEnable);

	//	设置是否断线检测
	int	SetIsDetectDisconn(int nEnable);

	//	设置发送列队最大长度，当超过此长度会发送失败
	int	SetMaxSendQueue(int size);

	//	获取底层套接字绑定的地址和端口号
	int	GetSocketBindInfo(char *szIp, int &nPort);


protected:
	int	Create(opMode type);

	int	GetLocalIP(char *szIp);

protected:
	unsigned long	m_remoteIp;			// IP,PORT 都是网络字节序
	unsigned short	m_remotePort;
	unsigned long	m_localIp;
	unsigned short	m_localPort;
	SOCKET			m_socket;

	int				m_engineId;
	ITPListener		*m_listener;

	int				m_recvBuffSize;
	int				m_sendBuffSize;

	BOOL			m_bDisCallBack;	// 是否可以断线回调
	BOOL			m_bReconnEn;	// 是否断线重连
	BOOL			m_bDetectDisconn; // 是否断线检测

	long			m_timeout_sec;
	long			m_timeout_usec;

	int				m_nSendQueueSize;
	unsigned int	m_nDetectDisconnTime;	//断线检测时间
};

#endif // _ITPOBJECT_H_
