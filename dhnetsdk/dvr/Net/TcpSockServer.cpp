// AlarmServer.cpp: implementation of the CAlarmServer class.
//
//////////////////////////////////////////////////////////////////////

#include "../StdAfx.h"
#include "TcpSockServer.h"

/*************************************************************************/
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CTcpSockServer::CTcpSockServer(): TPTCPServer(this)
{
	m_cbServer		= NULL;
	m_dwUserData	= 0;
}

CTcpSockServer::~CTcpSockServer()
{
	
}

int CTcpSockServer::InitNetwork()
{
	return TPTCPServer::Startup();
}

int CTcpSockServer::ClearNetwork()
{
	return TPTCPServer::Cleanup();
}

int CTcpSockServer::onData(int nEngineId, int nConnId, unsigned char* data, int nLen)
{
	if(m_cbServer != NULL)
	{
		m_cbServer((LONG)this, nConnId, NULL, -1, 2, (LPVOID)data, nLen, m_dwUserData);
	}

	return 1;
}

int	CTcpSockServer::onDealData(int nEngineId, int nConnId, unsigned char* buffer, int nLen)
{
	if(m_cbServer != NULL)
	{
		m_cbServer((LONG)this, nConnId, NULL, -1, 3, NULL, 0, m_dwUserData);
	}
	
	return 1;
}

int CTcpSockServer::onSendDataAck(int nEngineId, int nConnId, int nId)
{
	return 1;
}

int CTcpSockServer::onConnect(int nEngineId, int nConnId, char* szIp, int nPort)
{
	if(m_cbServer != NULL)
	{
		m_cbServer((LONG)this, nConnId, (char *)szIp, nPort, 1, NULL, 0, m_dwUserData);
	}

	return 0;//0表示接受，1表示拒绝.
}

int CTcpSockServer::onClose(int nEngineId, int nConnId)
{
	if(m_cbServer != NULL)
	{
		m_cbServer((LONG)this, nConnId, NULL, -1, 4, NULL, 0, m_dwUserData);
	}
	
	return 1;
}

int CTcpSockServer::onDisconnect(int nEngineId, int nConnId)
{
	return 1;
}

int CTcpSockServer::onReconnect(int nEngineId, int nConnId)
{
	return 1;
}
	


int	CTcpSockServer::StartListen(const char *szIp, 
					int nPort,
					fEventCallBack cbServer, 
					DWORD dwUserdata)
{
	m_cbServer = cbServer;
	m_dwUserData = dwUserdata;
	
	return TPTCPServer::Listen((char*)szIp, nPort);
}

int	CTcpSockServer::StopListen()
{
	return TPTCPServer::Close();
}

int CTcpSockServer::HeartBeat()
{
	return this->Heartbeat();
}