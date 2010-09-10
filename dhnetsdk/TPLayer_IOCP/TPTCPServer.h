/*
* Copyright (c) 2008, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 文件名称：TPTCPServer.h
* 摘　　要：TCP服务器功能类
*
*
* 取代版本：0.1
* 原作者　：Peng Dongfeng
* 完成日期：2008年6月16日
* 修订记录：创建
*/
//////////////////////////////////////////////////////////////////////

#ifndef TPTCPSERVER
#define TPTCPSERVER

#include "ITPObject.h"


class TPTCPServer : public ITPObject, protected IOCPListener
{
public:
    TPTCPServer(ITPListener* tcpserverapp, int engineId = 0);
    virtual ~TPTCPServer();


public: /* 接口 */
    virtual int CloseClient(int connId);

    virtual int Close();
    
	virtual int Listen(const char* szIp, int nPort);
    
    virtual int Send(int connId, int id, IBufferRef pSendBuf);

    virtual int Heartbeat();


public:	//内部函数
	virtual int onIOData(int nEngineId, int nConnId, unsigned char* data, int nLen, BOOL bPostSuccess);

	virtual int onIODealData(int nEngineId, int nConnId, unsigned char* buffer, int nLen);

	virtual int onIODisconnect(int nEngineId, int nConnId);

	virtual int onIOSendDataAck(int nEngineId, int nConnId, int nId, BOOL bPostSuccess);

	virtual	int	ListenSocket();

	//	虚函数，可以重写Accept处理方式
	virtual int	DealNewSocket(SOCKET newsock, int nConnId, char* szIp, int nPort);

	
protected:
	int		CreateClientEnvironment(CConnClientData *pClientConn);
	int		ClearClientEnvironment(CConnClientData *pClientConn);


protected:
	HANDLE			m_hListenThread;
	HANDLE			m_hListenExitEvent;

	CONN_MAP		m_lstConnClients;
	CReadWriteMutex	m_csClientsMap;
};

#endif
