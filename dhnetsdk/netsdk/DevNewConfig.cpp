// DevNewConfig.cpp: implementation of the CDevNewConfig class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DevNewConfig.h"
#include "Manager.h"
#include "DecoderDevice.h"
#include "Utils_StrParser.h"
#include "../dvr/ParseString.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDevNewConfig::CDevNewConfig(CManager *pManager)
	:m_pManager(pManager)
{

}

CDevNewConfig::~CDevNewConfig()
{
	m_pManager = NULL;
}

int CDevNewConfig::Init()
{
	return 0;
}

int CDevNewConfig::Uninit()
{
	return 0;
}

int CDevNewConfig::CloseChannelOfDevice(afk_device_s* device)
{
	return 0;
}

int __stdcall SynConfigInfoFunc(
							   afk_handle_t object,		/* CNewConfigChannel */
							   unsigned char *data,		/* 接收数据 */
							   unsigned int datalen,	/* 数据长度 */
							   void *param,				/* 结束标志 */
							   void *udata)				/* MESS_CALLBACK_DATA */
{
	afk_channel_s *pNewConfigChannel = (afk_channel_s*)object;
	MESS_CALLBACK_DATA *pMessData = (MESS_CALLBACK_DATA*)udata;
	if (pNewConfigChannel == NULL || pMessData == NULL)
	{
		return -1;
	}

	*pMessData->pResult = pMessData->nResultCode;
	*pMessData->pDvrRestart = pMessData->nDvrRestart;
	
	if (data != NULL)
	{
		OS_EVENT *pRecvEvent = (OS_EVENT*)pMessData->pRecvEvent;
		if (pMessData->nResultCode != 0)
		{
			SetEventEx(*pRecvEvent);
			return -1;
		}
		if ((int)param != -1)
		{
			// 将数据填充到用户缓冲中
			if (pMessData->nBufLen < (*pMessData->pRetLen + datalen))
			{
				SetEventEx(*pRecvEvent);
				return -1;
			}
			
			memcpy(pMessData->pBuf + *pMessData->pRetLen, data, datalen);
			*pMessData->pRetLen += datalen;
		}
		else
		{
			if (pMessData->nBufLen < (*pMessData->pRetLen + datalen))
			{
				SetEventEx(*pRecvEvent);
				return -1;
			}
			
			memcpy(pMessData->pBuf + *pMessData->pRetLen, data, datalen);
			*pMessData->pRetLen += datalen;
			SetEventEx(*pRecvEvent);
		}
	}
	else
	{
		OS_EVENT *pRecvEvent = (OS_EVENT*)pMessData->pRecvEvent;
		if (pMessData->nResultCode < 0)
		{
			SetEventEx(*pRecvEvent);
			return -1;
		}
		else
		{
			SetEventEx(*pRecvEvent);
		}	
	}
	return 0;
}

/*
 *	摘要：配置信息
 */
int CDevNewConfig::SysConfigInfo(LONG lLoginID, int nType, void* pCondition, char *pOutBuffer, int maxlen, int *pRetlen, int *error, int *restart, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == pOutBuffer)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;
	memset(pOutBuffer, 0, maxlen);
	*pRetlen = 0;
	*error = -1;
	*restart = 0;

	int nResult = -1;
	int nRestart = 0;
	OS_EVENT recvEvent;
	CreateEventEx(recvEvent, TRUE, FALSE);

	afk_newconfig_channel_param stuNewConfigParam = {0};
	stuNewConfigParam.base.func = SynConfigInfoFunc;
	stuNewConfigParam.base.udata = NULL;
	stuNewConfigParam.nType = nType;
	stuNewConfigParam.nSequence = m_pManager->GetPacketSequence();
	stuNewConfigParam.pCondition = pCondition;
	stuNewConfigParam.pRecvBuf = pOutBuffer;
	stuNewConfigParam.nBufLen = maxlen;
	stuNewConfigParam.pRetLen = pRetlen;
	stuNewConfigParam.pResultCode = &nResult;
	stuNewConfigParam.pRecvEvent = &recvEvent;
	stuNewConfigParam.pDvrRestart = &nRestart;

	afk_channel_s *pNewConfigChannel = (afk_channel_s*)device->open_channel(device, AFX_CHANNEL_TYPE_NEWCONFIG, &stuNewConfigParam);
	if (pNewConfigChannel != NULL)
	{
		DWORD dwRet = WaitForSingleObjectEx(recvEvent, waittime);
		pNewConfigChannel->close(pNewConfigChannel);
		ResetEventEx(recvEvent);
		if (dwRet == WAIT_OBJECT_0)
		{
			*error = nResult;
			*restart = nRestart;
			if (nResult != 0)
			{
				nRet = NET_RETURN_DATA_ERROR;		
			}
			else
			{
				nRet = nResult;	
			}
		}
		else 
		{
			nRet = NET_NETWORK_ERROR;
		}
	}
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}

	CloseEventEx(recvEvent);

	return nRet;
}


/* 新的获取设备配置（字符串协议）*/
int	CDevNewConfig::GetDevConfig(LONG lLoginID, char* szCommand, int nChannelID, char* szOutBuffer, DWORD dwOutBufferSize, int *error,int waittime/*=500*/)
{
	if (!szCommand || !szOutBuffer)
	{
		return NET_ILLEGAL_PARAM;
	}

	memset(szOutBuffer, 0, dwOutBufferSize);

	afk_device_s* device = (afk_device_s*)lLoginID;

	if (!device || m_pManager->IsDeviceValid(device) < 0)
	{
		return NET_INVALID_HANDLE;
	}

	int nerror = 0;
	int nrestart = 0;
	int nRetLen = 0;
	int nRet = -1;

	//生成json格式
	char szCondition[2048] = {0};
//	Json::Value root;

//	root["Request"]["OperateType"]="GetConfig";
//	root["Request"]["Name"]=szCommand;
//	root["Request"]["Channel"]=nChannelID+1;
//	std::string str;
//	Json::StyledWriter writer(str);
//	writer.write(root);

//	strcpy(szCondition, str.c_str());
	sprintf(szCondition, "{\"Request\":{\"Channel\":%d,\"Name\":\"%s\",\"OperateType\":\"GetConfig\"}}", nChannelID+1, szCommand);

	nRet = SysConfigInfo(lLoginID, 0, szCondition, szOutBuffer, dwOutBufferSize, &nRetLen, &nerror, &nrestart, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		nRet = NET_NOERROR;
	}
	
	if (error)
	{
		*error = nerror;
	}
	
	return nRet;
}	

/* 新的设置设备配置（字符串协议）*/
int	CDevNewConfig::SetDevConfig(LONG lLoginID, char* szCommand, int nChannelID, char* szInBuffer, DWORD dwInBufferSize, int *error, int *restart, int waittime/*=500*/) 
{
	if (!szCommand || !szInBuffer)
	{
		return NET_ILLEGAL_PARAM;
	}

	afk_device_s* device = (afk_device_s*)lLoginID;

	if (!device || m_pManager->IsDeviceValid(device) < 0)
	{
		return NET_INVALID_HANDLE;
	}

	int nerror = 0;
	int nrestart = 0;
	int nRetLen = 0;
	int nRet = -1;

	char szOutBuffer[1024] = {0};

	//生成json格式
	char *pCondition = new char[dwInBufferSize+256];
	if (pCondition)
	{
		memset(pCondition, '0', dwInBufferSize+256);
	}
	else
	{
		return -1;
	}

/*	Json::Value root;
	root["Request"]["OperateType"]="SetConfig";
	root["Request"]["Name"]=szCommand;
	root["Request"]["Channel"]=nChannelID+1;

	std::string str;
	Json::StyledWriter writer(str);
	writer.write(root);
	strcpy(pCondition, str.c_str());
	*/

	sprintf(pCondition, "{\"Request\":{\"Channel\":%d,\"Name\":\"%s\",\"OperateType\":\"SetConfig\"}", nChannelID+1, szCommand);

	if (strlen(szInBuffer) == 0)
	{
		strcat(pCondition, "}");
	}
	else
	{
		std::string str1 = pCondition;
		str1.insert(str1.size(), ",", 1);
		str1.insert(str1.size(), szInBuffer+1, strlen(szInBuffer)-1);
		strcpy(pCondition, str1.c_str());	
	}
	

	nRet = SysConfigInfo(lLoginID, 0, pCondition, szOutBuffer, 1024, &nRetLen, &nerror, &nrestart, waittime);
	if (nRet == 0)
	{
		nRet = NET_NOERROR;
	}
	
	if (error)
	{
		*error = nerror;
	}
	if (restart)
	{
		*restart = nrestart;
	}

	if (pCondition)
	{
		delete []pCondition;
		pCondition = NULL;
	}
	
	return nRet;
}



