// AutoRegister.cpp: implementation of the CAutoRegister class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AutoRegister.h"
#include "DecoderDevice.h"
#include "Manager.h"
#include "../dvr/dvrdevice/dvr2cfg.h"
#include "../dvr/ParseString.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAutoRegister::CAutoRegister(CManager *pManager)
	: m_pManager(pManager)
{

}

CAutoRegister::~CAutoRegister()
{
	m_pManager = NULL;
}

int	CAutoRegister::Init()
{
	return Uninit();
}

int	CAutoRegister::Uninit()
{
	int nRet = 0;

	return nRet;
}

int	CAutoRegister::CloseChannelOfDevice(afk_device_s* device)
{
	int nRet = 0;

	return nRet;
}

LONG CAutoRegister::ConnectRegServer(LONG lLoginID, char* RegServerIP, WORD RegServerPort, int TimeOut)
{
	afk_device_s* device = (afk_device_s*)lLoginID;

	if (!device || m_pManager->IsDeviceValid(device) < 0 || !RegServerIP || strlen(RegServerIP)>16)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return 0;
	}

	int nRet = -1;  //return value
	int nRetLen = 0;
	char buffer[512] = {0};
	LONG lConnectionID = 0;
	
	//Request Connect RegServer
	char szConnectParam[128];
	sprintf(szConnectParam, "%s\r\n%s\r\n%d", "Port", RegServerIP, RegServerPort);

	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_CONNECT, (void*)szConnectParam, buffer, 512, &nRetLen, 1000);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = GetProtocolValue(buffer, "ConnectionID:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return 0;
		}

		lConnectionID = atol(szValue);

		DWORD dwCreateTime = GetTickCountEx();//TimeOut时间内查询连接的状态

		while (GetTickCountEx() - dwCreateTime < TimeOut)
		{
			//查询连接的状态
			BOOL nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_AUTOREGISTER_GETSERVER,
																	(void*)lConnectionID, buffer, 512, &nRetLen, 1000);
			if (nRet >= 0 && nRetLen > 0)
			{
				p = GetProtocolValue(buffer, "State:", "\r\n", szValue, 64);
				if (!p || (_stricmp(szValue, "Success") == 0))
				{	
					break;
				}
				else
				{
					lConnectionID = 0;//连接失败继续查询
				}
			}
		}
	}
	
	return lConnectionID>0?lConnectionID:0;
}

int CAutoRegister::QueryRegServerInfo(LONG lLoginID, LPDEV_SERVER_AUTOREGISTER lpRegServerInfo, int waittime)
{
	afk_device_s* device = (afk_device_s*)lLoginID;
	
	if (!device || NULL == lpRegServerInfo)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;

	memset(lpRegServerInfo, 0, sizeof(DEV_SERVER_AUTOREGISTER));
	
	int nRetLen = 0;
	char buffer[512] = {0};
	int  nConnectID[CONTROL_AUTO_REGISTER_NUM] = {0};
	int  nConnectCount = 0;
	
	// 主动注册服务器的连接ID
	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_AUTOREGISTER_GETCONID,
														NULL, buffer, 512, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = buffer;//解析连接ID
		{
			for (int i = 0; i < CONTROL_AUTO_REGISTER_NUM; i++)//解析ID
			{
				p = GetProtocolValue(p, "ConnectionID:", "\r\n", szValue, 64);
				if (NULL ==p)
				{
					break;
				}
				else
				{
					nConnectID[i] = atoi(szValue);
					nConnectCount++;	
				}		
			}
		}

		{//解析注册状态
			p = buffer;
			for (int j = 0; j < CONTROL_AUTO_REGISTER_NUM; j++)
			{
				p = GetProtocolValue(p, "RegState:", "\r\n", szValue, 64);
				if (NULL == p)
				{
					break;	
				}
				else
				{
					if (_stricmp(szValue, "Success") == 0)
					{
						lpRegServerInfo->stuDevRegisterSeverInfo[j].nState = 1;//注册成功
					}
					else
					{
						lpRegServerInfo->stuDevRegisterSeverInfo[j].nState = 0;//注册失败
					}	
				}	
			}		
		}
		
		//根据连接ID取主动注册服务器的状态信息
		for (int k = 0; k < nConnectCount; k++)
		{
			// 再查询主动注册服务器的状态信息
			nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_AUTOREGISTER_GETSERVER,
																(void*)nConnectID[k], buffer, 512, &nRetLen, waittime);
			if (nRet >= 0 && nRetLen > 0)
			{
				p = GetProtocolValue(buffer, "Addr:", "\r\n", szValue, 64);
				if (p == NULL)
				{
					return NET_RETURN_DATA_ERROR;
				}
				strcpy(lpRegServerInfo->stuDevRegisterSeverInfo[k].szServerIp, szValue);
				
				p = GetProtocolValue(buffer, "port:", "\r\n", szValue, 64);
				if (p == NULL)
				{
					return NET_RETURN_DATA_ERROR;
				}
				lpRegServerInfo->stuDevRegisterSeverInfo[k].nPort = atoi(szValue);
			
				p = GetProtocolValue(buffer, "State:", "\r\n", szValue, 64);
				if (p)
				{
					if (_stricmp(szValue, "Success") != 0)
					{
						lpRegServerInfo->stuDevRegisterSeverInfo[k].nState = 2; //连接失败
					}
				}
				else
				{
					return NET_RETURN_DATA_ERROR;
				}

				lpRegServerInfo->nRegisterSeverCount +=1;
				lpRegServerInfo->stuDevRegisterSeverInfo[k].lConnectionID = nConnectID[k];
			}
		}
	}
	else
	{
		return NET_ERROR_GET_AUTOREGSERVER;
	}
	
	return nRet;
	
}

int CAutoRegister::ControlRegister(LONG lLoginID, LONG ConnectionID, int waittime)
{
	afk_device_s* device = (afk_device_s*)lLoginID;
	
	if (!device || ConnectionID <= 0)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return NET_ILLEGAL_PARAM;
	}
	int nRet = -1;  //return value

	//控制主动注册服务器
	nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_AUTOREGISTER_REGSERVER, (void*)ConnectionID, waittime);
	if (nRet < 0 )
	{
		nRet = NET_ERROR_CONTROL_AUTOREGISTER;
	}
	else
	{
		nRet = NET_NOERROR;
	}		
	return nRet;

}

int CAutoRegister::DisConnectRegServer(LONG lLoginID, LONG ConnectionID)
{
	afk_device_s* device = (afk_device_s*)lLoginID;
	
	if (!device || ConnectionID < 0)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return NET_ILLEGAL_PARAM;
	}
	int nRet = -1;  //return value

	//断开主动注册服务器
	char szDisconnectParam[128] = {0};
	sprintf(szDisconnectParam, "%s\r\n%d", "Port", ConnectionID);
	nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_DISCONNECT, (void*)szDisconnectParam, 500);
	
	if (nRet < 0 )
	{
		nRet = NET_ERROR_DISCONNECT_AUTOREGISTER;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	
	return nRet;
}

