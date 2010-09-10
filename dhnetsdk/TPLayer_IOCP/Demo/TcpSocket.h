// TcpSocket.h: interface for the CTcpSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCPSOCKET_H__8E1B116D_3800_4C99_A666_8413ABE8E9F7__INCLUDED_)
#define AFX_TCPSOCKET_H__8E1B116D_3800_4C99_A666_8413ABE8E9F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma comment( lib, "Ws2_32.lib")

#include "TPLayer/TPTCPServer.h"
#include "TPLayer/TPTCPClient.h"
class CDataCenter;

#define HEADER_SIZE  32
#define MAX_BUF_SIZE  (5*1024)
#define MAX_PACKET_SIZE	 (128)

//////////////////////////////////////////////////////////////////////////

class CTcpSocket : virtual public TPTCPServer, virtual public TPTCPClient, public ITPListener
{
public:
	CTcpSocket();
	virtual ~CTcpSocket();

public:
	static int InitNetwork();
	static int ClearNetwork();


public:	// client
	int		WriteData(IBufferRef pSendBuf);

	void	Disconnect();

	int		Heartbeat();

	
public:	// server
	int		StartServer(const char *szIp, int nPort, CDataCenter *pDataCenter);
	
	int		StopServer();

	virtual int	DealNewSocket(SOCKET newsock, int connId, const char* ip, int port);

	int		SetSocket(SOCKET newsock, int connId, const char* ip, int port, CDataCenter *pDataCenter);


	
public:	// event
	virtual int onData(int engineId, int connId, const char* data, int len);

	virtual int	onDealData(int engineId, int connId);

	virtual int onSendDataAck(int engineId, int connId, int id);

	virtual int onConnect(int engineId, int connId, const char* ip, int port);

	virtual int onClose(int engineId, int connId);

	virtual int onDisconnect(int engineId, int connId);

	virtual int onReconnect(int engineId, int connId);


private:
	int		GetData();

private:
	CDataCenter*		m_pDataCenter;

	unsigned int		m_nSessionId;
	BOOL				m_bConnType;	//主/子连接

	// 简易缓冲管理
    unsigned int		m_nWritePos;
    unsigned int		m_nReadPos;
	unsigned char		m_packetBuf[MAX_BUF_SIZE];
	unsigned char		m_dealBuf[MAX_PACKET_SIZE];
	CReadWriteMutex		m_csBuffer;

};

#endif // !defined(AFX_TCPSOCKET_H__8E1B116D_3800_4C99_A666_8413ABE8E9F7__INCLUDED_)
