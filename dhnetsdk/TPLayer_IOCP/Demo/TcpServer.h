// TcpServer.h: interface for the CTcpServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCPSERVER_H__60051960_ECA8_426C_91D9_7A4BBF66415F__INCLUDED_)
#define AFX_TCPSERVER_H__60051960_ECA8_426C_91D9_7A4BBF66415F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../TPLayer/TPTCPServer.h"

class CTcpServer : public TPTCPServer, public ITPListener
{
public:
	CTcpServer();
	virtual ~CTcpServer();

public:
	virtual int onData(int engineId, int connId, const char* data, int len);

	virtual int onConnect(int engineId, int connId, const char* ip, int port);

	virtual int onClose(int engineId, int connId);

	virtual int onDisconnect(int engineId, int connId);

	virtual int onReconnect(int engineId, int connId);

	virtual int onSendDataAck(int engineId, int connId, int id);
};

#endif // !defined(AFX_TCPSERVER_H__60051960_ECA8_426C_91D9_7A4BBF66415F__INCLUDED_)
