// TcpClient.h: interface for the CTcpClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCPCLIENT_H__0BE8B2BE_A838_4CEB_BDD6_A5188E644376__INCLUDED_)
#define AFX_TCPCLIENT_H__0BE8B2BE_A838_4CEB_BDD6_A5188E644376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../TPLayer/TPTCPClient.h"

class CTcpClient : public TPTCPClient, public ITPListener
{
public:
	CTcpClient();
	virtual ~CTcpClient();

public:
	virtual int onData(int engineId, int connId, const char* data, int len);

	virtual int onConnect(int engineId, int connId, const char* ip, int port);

	virtual int onClose(int engineId, int connId);

	virtual int onDisconnect(int engineId, int connId);

	virtual int onReconnect(int engineId, int connId);

	virtual int onSendDataAck(int engineId, int connId, int id);
};

#endif // !defined(AFX_TCPCLIENT_H__0BE8B2BE_A838_4CEB_BDD6_A5188E644376__INCLUDED_)
