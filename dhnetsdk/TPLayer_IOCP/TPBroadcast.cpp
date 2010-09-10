// TPBroadcast.cpp: implementation of the TPBroadcast class.
//
//////////////////////////////////////////////////////////////////////

#include "TPBroadcast.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TPBroadcast::TPBroadcast(ITPListener *callback, int engineId)
	: TPUDPClient(callback, engineId)
{
	
}

TPBroadcast::~TPBroadcast()
{

}

int TPBroadcast::Connect(const char* szIp, int nPort)
{
	if (szIp == NULL)
    {
        m_remoteIp = INADDR_BROADCAST;
    }
    else
    {
        m_remoteIp = inet_addr(szIp);
    }
	
    m_remotePort = htons(nPort);

	//	先create
	int nRet = Create(opMode_udp);
	if (nRet < 0)
	{
		return -1;
	}

	//	socket属性设为广播方式	
	bool opt = true;
	setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(bool));

	//	再创建客户端环境
	nRet = CreateClientEnvironment();
	if (nRet < 0)
	{
		goto e_clean;
	}
	
	//	然后将socket添加到完成端口中
	nRet = AddSocketToIOCP(m_socket, m_pPerHandleData);
	if (nRet >= 0)
	{
		m_pPerHandleData->AddRef();
		m_pPerIoRecv->AddRef();
		nRet = PostRecvToIOCP(m_socket, m_pPerIoRecv);
		if (nRet < 0)
		{
			m_pPerIoRecv->DecRef();
			m_pPerHandleData->DecRef();
			
			ClearClientEnvironment();
			goto e_clean;
		}
	}
	else
	{
		ClearClientEnvironment();
		goto e_clean;
	}
	
    return 1;
	
e_clean:
	DelSocketFromIOCP(m_socket, m_pPerHandleData);
	m_socket = INVALID_SOCKET;
	return -1;
}

int TPBroadcast::Connect(const char* szLocalIp, int nLocalPort, const char* szRemoteIp, int nRemotePort)
{
	if (szLocalIp == NULL)
    {
        m_localIp = INADDR_ANY;
    }
    else
    {
        m_localIp = inet_addr(szLocalIp);
    }

    m_localPort = htons(nLocalPort);
	
	return Connect(szRemoteIp, nRemotePort);
}


