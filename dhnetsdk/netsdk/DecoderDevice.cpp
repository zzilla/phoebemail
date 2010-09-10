// DecoderDevice.cpp: implementation of the CDecoderDevice class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DecoderDevice.h"
#include "Manager.h"
#include "../dvr/dvrdevice/dvr2cfg.h"
#include "../dvr/ParseString.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDecoderDevice::CDecoderDevice(CManager *pManager)
	: m_pManager(pManager)
{

}

CDecoderDevice::~CDecoderDevice()
{
	m_pManager = NULL;
}

int	CDecoderDevice::Init()
{
	return Uninit();
}

int	CDecoderDevice::Uninit()
{
	int nRet = 0;

	return nRet;
}

int	CDecoderDevice::CloseChannelOfDevice(afk_device_s* device)
{
	int nRet = 0;

	return nRet;
}


int __stdcall SynQueryInfoFunc(
							   afk_handle_t object,		/* CRequestChannel */
							   unsigned char *data,		/* 接收数据 */
							   unsigned int datalen,	/* 数据长度 */
							   void *param,				/* 结束标志 */
							   void *udata)				/* MESS_CALLBACK_DATA */
{
	afk_channel_s *pRequestChannel = (afk_channel_s*)object;
	MESS_CALLBACK_DATA *pMessData = (MESS_CALLBACK_DATA*)udata;
	if (pRequestChannel == NULL || data == NULL || pMessData == NULL)
	{
		return -1;
	}
	
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
		*pMessData->pResult = 0;
        SetEventEx(*pRecvEvent);
	}

	return 0;
}

/*
 *	摘要：同步查询通用接口
 */
int CDecoderDevice::SysQueryInfo(LONG lLoginID, int nType, void* pCondition, char *pOutBuffer, int maxlen, int *pRetlen, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == pOutBuffer)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;
	memset(pOutBuffer, 0, maxlen);
	*pRetlen = 0;

	int nResult = -1;
	OS_EVENT recvEvent;
	CreateEventEx(recvEvent, TRUE, FALSE);

	afk_request_channel_param stuRequestParam = {0};
	stuRequestParam.base.func = SynQueryInfoFunc;
	stuRequestParam.base.udata = NULL;
	stuRequestParam.nType = nType;
	stuRequestParam.nSequence = m_pManager->GetPacketSequence();
	stuRequestParam.pCondition = pCondition;
	stuRequestParam.pRecvBuf = pOutBuffer;
	stuRequestParam.nBufLen = maxlen;
	stuRequestParam.pRetLen = pRetlen;
	stuRequestParam.pResultCode = &nResult;
	stuRequestParam.pRecvEvent = &recvEvent;
	afk_channel_s *pRequestChannel = (afk_channel_s*)device->open_channel(device, AFX_CHANNEL_TYPE_REQUEST, &stuRequestParam);
	if (pRequestChannel != NULL)
	{
		DWORD dwRet = WaitForSingleObjectEx(recvEvent, waittime);
		pRequestChannel->close(pRequestChannel);
		ResetEventEx(recvEvent);
		if (dwRet == WAIT_OBJECT_0)
		{
			if (nResult == -1)
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


int __stdcall SysSetupFunc(
							  afk_handle_t object,	/* 数据提供者 */
							  unsigned char *data,	/* 数据体 */
							  unsigned int datalen,	/* 数据长度 */
							  void *param,			/* 回调参数 */
							  void *udata)
{
	afk_channel_s *pRequestChannel = (afk_channel_s*)object;
	MESS_CALLBACK_DATA *pMessData = (MESS_CALLBACK_DATA*)udata;
	if (pRequestChannel == NULL || pMessData == NULL)
	{
		return -1;
	}
	
	OS_EVENT *pRecvEvent = (OS_EVENT*)pMessData->pRecvEvent;
	if (pMessData->nResultCode < 0)
	{
		SetEventEx(*pRecvEvent);
		return -1;
	}
	else
	{
		*pMessData->pResult = pMessData->nResultCode;
        SetEventEx(*pRecvEvent);
	}
	
	return 0;
}

/*
 *	摘要：同步设置通用接口
 */
int CDecoderDevice::SysSetupInfo(LONG lLoginID, int nType, void* pCondition, int waittime)
	
{
 	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == pCondition)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nResult = -1;
	int nRet = -1;
	OS_EVENT recvEvent;
	CreateEventEx(recvEvent, TRUE, FALSE);

	afk_request_channel_param stuRequestParam = {0};
	stuRequestParam.base.func = SysSetupFunc;
	stuRequestParam.base.udata = NULL;
	stuRequestParam.nType = nType;
	stuRequestParam.nSequence = m_pManager->GetPacketSequence();
	stuRequestParam.pCondition = pCondition;
	stuRequestParam.pRecvBuf = NULL;
	stuRequestParam.nBufLen = 0;
	stuRequestParam.pRetLen = NULL;
	stuRequestParam.pResultCode = &nResult;
	stuRequestParam.pRecvEvent = &recvEvent;
	afk_channel_s *pRequestChannel = (afk_channel_s*)device->open_channel(device, AFX_CHANNEL_TYPE_REQUEST, &stuRequestParam);
	if (pRequestChannel != NULL)
	{
		DWORD dwRet = WaitForSingleObjectEx(recvEvent, waittime);
		pRequestChannel->close(pRequestChannel);
		ResetEventEx(recvEvent);
		if (dwRet == WAIT_OBJECT_0)
		{
			if (nResult == -1)
			{
				nRet = NET_RETURN_DATA_ERROR;
			}
			else //add by cqs
			{
				nRet = nResult;

				if (nResult == 1)//reboot the device
				{
					m_pManager->DeviceEvent(device, EVENT_CONFIG_REBOOT, NULL);
				}
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


int	CDecoderDevice::QueryDecoderInfo(LONG lLoginID, LPDEV_DECODER_INFO lpDecInfo, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == lpDecInfo)
	{
		return NET_ILLEGAL_PARAM;
	}

	memset(lpDecInfo, 0, sizeof(DEV_DECODER_INFO));
	device->get_info(device, dit_dev_type, lpDecInfo->szDecType);
	
	int nRetLen = 0;
	char buffer[512] = {0};
	
	int nRet = SysQueryInfo(lLoginID, AFK_REQUEST_DEC_QUERY_SYS, NULL, buffer, 512, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = GetProtocolValue(buffer, "TvNum:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}

		lpDecInfo->nMonitorNum = atoi(szValue);
	
		for (int i = 0; i < lpDecInfo->nMonitorNum; i++)
		{
			p = GetProtocolValue(p, "TVEnable:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				return NET_RETURN_DATA_ERROR;
			}

			if (_stricmp(szValue, "True") == 0)
			{
				lpDecInfo->bMonitorEnable[i] = 1;
			}
			else
			{
				lpDecInfo->bMonitorEnable[i] = 0;
			}
		}

		p = buffer;
		for (int j = 0; j < 16; j++)
		{
			p = GetProtocolValue(p, "TvSplitMode:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				break;
			}
		
			lpDecInfo->szSplitMode[j] = atoi(szValue);	
		}
	}

	nRet = SysQueryInfo(lLoginID, AFK_REQUEST_DEC_QUERY_CHNNUM, NULL, buffer, 512, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = GetProtocolValue(buffer, "ChannelNum:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		
		lpDecInfo->nEncoderNum = atoi(szValue);
	}

	return nRet;
}


int	CDecoderDevice::QueryDecoderTVInfo(LONG lLoginID, int nMonitorID, LPDEV_DECODER_TV lpMonitorInfo, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == lpMonitorInfo)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;
	memset(lpMonitorInfo, 0, sizeof(DEV_DECODER_TV));

	int nRetLen = 0;
	char buffer[512] = {0};

	// 先查询TV信息
	nRet = SysQueryInfo(lLoginID, AFK_REQUEST_DEC_QUERY_TV, (void*)nMonitorID, buffer, 512, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = GetProtocolValue(buffer, "DisTV:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		lpMonitorInfo->nID = atoi(szValue);

		p = GetProtocolValue(buffer, "Enable:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		if (0 == _stricmp(szValue, "True"))
		{
			lpMonitorInfo->bEnable = TRUE;
		}
		
		p = GetProtocolValue(buffer, "DisMode:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		lpMonitorInfo->nSplitType = atoi(szValue);

		p = buffer;
		int nDecoderChannel[16] = {0};
		{
			for (int i = 0; i < lpMonitorInfo->nSplitType; i++)
			{
				p = GetProtocolValue(p, "DisChn:", "\r\n", szValue, 64);
				if (p == NULL)
				{
					return NET_RETURN_DATA_ERROR;
				}
				nDecoderChannel[i] = atoi(szValue);
			}
		}

		for (int i = 0; i < lpMonitorInfo->nSplitType; i++)
		{
			// 再查询解码通道信息
			nRet = QueryDecEncoderInfo(lLoginID, nDecoderChannel[i], &lpMonitorInfo->stuDevInfo[i], waittime);
			if (nRet < 0)
			{
				break;
			}
		}
	}

	return nRet;
}


int CDecoderDevice::QueryDecEncoderInfo(LONG lLoginID, int nEncoderID, LPDEV_ENCODER_INFO lpEncoderInfo, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == lpEncoderInfo)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;
	memset(lpEncoderInfo, 0, sizeof(DEV_ENCODER_INFO));

	int nRetLen = 0;
	char buffer[512] = {0};
	
	// 查询解码通道信息
	nRet = SysQueryInfo(lLoginID, AFK_REQUEST_DEC_QUERY_CHANNEL, (void*)nEncoderID, buffer, 512, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = GetProtocolValue(buffer, "IP:", "\r\n", szValue, 64);
		if (p != NULL)
		{
			int nstrLen = strlen(szValue);
			strncpy(lpEncoderInfo->szDevIp, szValue, nstrLen>MAX_IP_ADDRESS_LEN?MAX_IP_ADDRESS_LEN:nstrLen);
			strncpy(lpEncoderInfo->szDevIpEx, szValue, nstrLen>MAX_IPADDR_OR_DOMAIN_LEN?MAX_IPADDR_OR_DOMAIN_LEN:nstrLen);
		}
		
		p = GetProtocolValue(buffer, "Enable:", "\r\n", szValue, 64);
		if (p != NULL)
		{
			if (0 == _stricmp(szValue, "True"))
			{
				lpEncoderInfo->bDevChnEnable = 1;
			}
		}
		
		p = GetProtocolValue(buffer, "UserName:", "\r\n", szValue, 64);
		if (p != NULL)
		{
			strcpy(lpEncoderInfo->szDevUser, szValue);
		}
		
		p = GetProtocolValue(buffer, "PassWord:", "\r\n", szValue, 64);
		if (p != NULL)
		{
			strcpy(lpEncoderInfo->szDevPwd, szValue);
		}
		
		p = GetProtocolValue(buffer, "Port:", "\r\n", szValue, 64);
		if (p != NULL)
		{
			lpEncoderInfo->wDevPort = atoi(szValue);
		}
		
		p = GetProtocolValue(buffer, "DstChn:", "\r\n", szValue, 64);
		if (p != NULL)
		{
			lpEncoderInfo->nDevChannel = atoi(szValue);
		}
		
		p = GetProtocolValue(buffer, "DstPicType:", "\r\n", szValue, 64);
		if (p != NULL)
		{
			if (0 == _stricmp(szValue, "main"))
			{
				lpEncoderInfo->nStreamType = 0;
			}
			else if (0 == _stricmp(szValue, "second"))
			{
				lpEncoderInfo->nStreamType = 1;
			}
			else if (0 == _stricmp(szValue, "snap"))
			{
				lpEncoderInfo->nStreamType = 2;
			}

		}
		
		p = GetProtocolValue(buffer, "Mode:", "\r\n", szValue, 64);
		if (p != NULL)
		{
			if (0 == _stricmp(szValue, "DirectConn"))
			{
				lpEncoderInfo->byWorkMode = 0;
			}
			else if (0 == _stricmp(szValue, "Transmit"))
			{
				lpEncoderInfo->byWorkMode = 1;
			}
		}

		p = GetProtocolValue(buffer, "ServiceType:", "\r\n", szValue, 64);
		if (p != NULL)
		{
			if (0 == _stricmp(szValue, "TCP"))
			{
				lpEncoderInfo->byConnType = 0;
			}
			else if (0 == _stricmp(szValue, "UDP"))
			{
				lpEncoderInfo->byConnType = 1;
			}
			else if (0 == _stricmp(szValue, "MCAST"))
			{
				lpEncoderInfo->byConnType = 2;
			}
		}

		p = GetProtocolValue(buffer, "ServicePort:", "\r\n", szValue, 64);
		if (p != NULL)
		{
			lpEncoderInfo->wListenPort = atoi(szValue);
		}

		//lpEncoderInfo->byEncoderID = nEncoderID;
		
		p = GetProtocolValue(buffer, "DisChn:", "\r\n", szValue, 64);
		if (p != NULL)
		{
			lpEncoderInfo->byEncoderID = atoi(szValue);
		}


		p = GetProtocolValue(buffer, "ProtoType:", "\r\n", szValue, 64);
		if( p != NULL)
		{
			if( _stricmp(szValue, "DahuaII") == 0)
			{
				lpEncoderInfo->dwProtoType = 1;
			}
			else if( _stricmp(szValue, "DahuaXTJC") == 0)
			{
				lpEncoderInfo->dwProtoType = 2;
			}
			else if( _stricmp(szValue, "DahuaDSS") == 0)
			{
				lpEncoderInfo->dwProtoType = 3;
			}
			else if( _stricmp(szValue, "DahuaRtsp") == 0)
			{
				lpEncoderInfo->dwProtoType = 4;
			}
			else
			{
				lpEncoderInfo->dwProtoType = 0;
			}
		}
			
		char szDevName[132] = {0};
		p = GetProtocolValue(buffer, "DeviceName:", "\r\n", szDevName, 132);
		if(p != NULL)
		{
			int nNameLen = strlen(szDevName);
			nNameLen = nNameLen < 64 ? nNameLen : 63;
			memcpy(lpEncoderInfo->szDevName, szDevName, nNameLen);
		}

		p = GetProtocolValue(buffer, "VideoIn_Type:", "\r\n", szValue, 64);
		if(p != NULL)
		{
			if( _stricmp(szValue, "HD") == 0)
			{
				lpEncoderInfo->byVideoInType = 1;//高清
			}
			else
			{
				lpEncoderInfo->byVideoInType = 0;//标清
			}
		}
		
		p = GetProtocolValue(buffer, "SnapMode:", "\r\n", szValue, 64);
		if(p != NULL)
		{
			lpEncoderInfo->bySnapMode = atoi(szValue);
		}

		p = GetProtocolValue(buffer, "ManuFactory:", "\r\n", szValue, 64);
		if(p != NULL)
		{
			if( _stricmp(szValue, "Private") == 0)
			{
				lpEncoderInfo->byManuFactory = 0;
			}
		}

		p = GetProtocolValue(buffer, "DeviceType:", "\r\n", szValue, 64);
		if(p != NULL)
		{
			if( _stricmp(szValue, "IPC") == 0)
			{
				lpEncoderInfo->byDeviceType = 0;
			}
		}

	}

	return nRet;
}


int CDecoderDevice::SetDecoderTVEnable(LONG lLoginID, BYTE *pDecTVOutEnable, int nBufLen, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == pDecTVOutEnable)
	{
		return NET_ILLEGAL_PARAM;
	}

	REQUEST_DEC_SET_TVENABLE stuCondition = {0};
	stuCondition.pSetTVEnable = pDecTVOutEnable;
	stuCondition.nBuflen = nBufLen;

	return SysSetupInfo(lLoginID, AFK_REQUEST_DEC_SET_TV_ENABLE, &stuCondition, waittime);
}


int CDecoderDevice::SetOperateCallBack(LONG lLoginID, fMessDataCallBack cbMessData, DWORD dwUser)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == cbMessData)
	{
		return NET_ILLEGAL_PARAM;
	}

	DEVICE_MESS_CALLBACK stuCallFunc = {0};
	stuCallFunc.fun = (void*)cbMessData;
	stuCallFunc.data = (void*)dwUser;
	device->set_info(device, dit_set_callback, &stuCallFunc);

	return 0;
}


int __stdcall CtrlDecTVScreenFunc(
							   afk_handle_t object,		/* CRequestChannel */
							   unsigned char *data,		/* 接收数据或回调数据 */
							   unsigned int datalen,	/* 数据长度 */
							   void *param,				/* 标志 */
							   void *udata)				/* CManager */
{
	afk_channel_s *pRequestChannel = (afk_channel_s*)object;
	CManager *pManager = (CManager*)udata;
	if (pRequestChannel == NULL || data == NULL || pManager == NULL)
	{
		return -1;
	}
	
	if ((int)param != -1)
	{
		// 将数据填充到用户缓冲中
	}
	else
	{
		// 应答包回调给用户
		AFK_RESPONSE_DATA *pResponseData = new AFK_RESPONSE_DATA;
		if (pResponseData == NULL)
		{
			return -1;
		}

		NET_CALLBACK_DATA *pCallBackData = new NET_CALLBACK_DATA;
		if (pCallBackData == NULL)
		{
			delete pResponseData;
			return -1;
		}

		MESS_CALLBACK_DATA *pData = (MESS_CALLBACK_DATA *)data;
		pCallBackData->nResultCode = pData->nResultCode;
		pCallBackData->lOperateHandle = pData->lOperateHandle;
		pCallBackData->userdata = pData->userparam;
		pCallBackData->pBuf = NULL;
		pCallBackData->nRetLen = 0;
		pResponseData->lCommand = RESPONSE_DECODER_CTRL_TV;
		pResponseData->lpCallBackData = pCallBackData;
		pResponseData->cbMessFunc = (fMessDataCallBack)pData->cbMessFunc;
		pResponseData->dwUserdata = (DWORD)pData->userdata;
		
		{
			pManager->GetResponseDataCS().Lock();
			pManager->GetResponseDataList().push_back(pResponseData);
			pManager->GetResponseDataCS().UnLock();

			SetEventEx(pManager->m_hResponseEvent);
		}
	}

	return 0;
}

LONG CDecoderDevice::CtrlDecTVScreen(LONG lLoginID, int nMonitorID, BOOL bEnable, int nSplitType, BYTE *pEncoderChannel, int nBufLen, void* userdata)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == pEncoderChannel || nBufLen < nSplitType)
	{
		return NET_ILLEGAL_PARAM;
	}

	REQUEST_DEC_CTRLTV stuCondition = {0};
	stuCondition.bEnable = bEnable;
	stuCondition.nMonitorID = nMonitorID;
	stuCondition.nSplitType = nSplitType;
	stuCondition.pEncoderChannel = pEncoderChannel;
	stuCondition.nBufLen = nBufLen;

	DEVICE_MESS_CALLBACK stuCallFunc = {0};
	device->get_info(device, dit_set_callback, &stuCallFunc);

	afk_request_channel_param stuRequestParam = {0};
	stuRequestParam.base.func = CtrlDecTVScreenFunc;
	stuRequestParam.base.udata = m_pManager;
	stuRequestParam.nType = AFK_REQUEST_DECODER_CTRLTV;
	stuRequestParam.nSequence = m_pManager->GetPacketSequence();
	stuRequestParam.pCondition = &stuCondition;
	stuRequestParam.callbackFunc = stuCallFunc.fun;
	stuRequestParam.userdata = stuCallFunc.data;
	stuRequestParam.pRecvBuf = NULL;
	stuRequestParam.nBufLen = 0;
	stuRequestParam.userparam = userdata;

	afk_channel_s *pRequestChannel = (afk_channel_s*)device->open_channel(device, AFX_CHANNEL_TYPE_REQUEST, &stuRequestParam);

	return (LONG)pRequestChannel;
}


int __stdcall SwitchDecTVEncoderFunc(
							   afk_handle_t object,		/* CRequestChannel */
							   unsigned char *data,		/* 接收数据或回调数据 */
							   unsigned int datalen,	/* 数据长度 */
							   void *param,				/* 标志 */
							   void *udata)				/* CManager */
{
	afk_channel_s *pRequestChannel = (afk_channel_s*)object;
	CManager *pManager = (CManager*)udata;
	if (pRequestChannel == NULL || data == NULL || pManager == NULL)
	{
		return -1;
	}
	
	if ((int)param != -1)
	{
		// 将数据填充到用户缓冲中
	}
	else
	{
		// 应答包回调给用户
		AFK_RESPONSE_DATA *pResponseData = new AFK_RESPONSE_DATA;
		if (pResponseData == NULL)
		{
			return -1;
		}

		NET_CALLBACK_DATA *pCallBackData = new NET_CALLBACK_DATA;
		if (pCallBackData == NULL)
		{
			delete pResponseData;
			return -1;
		}

		MESS_CALLBACK_DATA *pData = (MESS_CALLBACK_DATA *)data;
		pCallBackData->nResultCode = pData->nResultCode;
		pCallBackData->lOperateHandle = pData->lOperateHandle;
		pCallBackData->userdata = pData->userparam;
		pCallBackData->pBuf = NULL;
		pCallBackData->nRetLen = 0;
		pResponseData->lCommand = RESPONSE_DECODER_SWITCH_TV;
		pResponseData->lpCallBackData = pCallBackData;
		pResponseData->cbMessFunc = (fMessDataCallBack)pData->cbMessFunc;
		pResponseData->dwUserdata = (DWORD)pData->userdata;
		
		{
			pManager->GetResponseDataCS().Lock();
			pManager->GetResponseDataList().push_back(pResponseData);
			pManager->GetResponseDataCS().UnLock();

			SetEventEx(pManager->m_hResponseEvent);
		}
	}

	return 0;
}

LONG CDecoderDevice::SwitchDecTVEncoder(LONG lLoginID, int nEncoderID, LPDEV_ENCODER_INFO lpEncoderInfo, void* userdata)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == lpEncoderInfo)
	{
		return NET_ILLEGAL_PARAM;
	}

	REQUEST_DEC_SWITCHTV stuCondition = {0};
	stuCondition.nEncoderID = nEncoderID;
	stuCondition.wDevPort = lpEncoderInfo->wDevPort;
	stuCondition.wEnable = lpEncoderInfo->bDevChnEnable;
	stuCondition.nDevChannel = lpEncoderInfo->nDevChannel;
	stuCondition.nStreamType = lpEncoderInfo->nStreamType;
	stuCondition.byWorkMode = lpEncoderInfo->byWorkMode;
	stuCondition.byConnType = lpEncoderInfo->byConnType;
	stuCondition.wListenPort = lpEncoderInfo->wListenPort;

	if (strlen(lpEncoderInfo->szDevIpEx)>0)
	{
		memcpy(stuCondition.szDevIp, lpEncoderInfo->szDevIpEx, 64);
	}
	else
	{
		memcpy(stuCondition.szDevIp, lpEncoderInfo->szDevIp, 16);

	}

	strcpy(stuCondition.szDevUser, lpEncoderInfo->szDevUser);
	strcpy(stuCondition.szDevPwd, lpEncoderInfo->szDevPwd);
	stuCondition.dwProtoType = lpEncoderInfo->dwProtoType;
	memcpy(stuCondition.szDevName, lpEncoderInfo->szDevName, 63);	//这样不会越界.也不用加结束标志,仍会产生残留字符.
	stuCondition.byVideoInType = lpEncoderInfo->byVideoInType;
	stuCondition.bySnapMode = lpEncoderInfo->bySnapMode;
	stuCondition.byManuFactory = lpEncoderInfo->byManuFactory;
	stuCondition.byDeviceType = lpEncoderInfo->byDeviceType;

	DEVICE_MESS_CALLBACK stuCallFunc = {0};
	device->get_info(device, dit_set_callback, &stuCallFunc);

	afk_request_channel_param stuRequestParam = {0};
	stuRequestParam.base.func = SwitchDecTVEncoderFunc;
	stuRequestParam.base.udata = m_pManager;
	stuRequestParam.nType = AFK_REQUEST_DECODER_SWITCH;
	stuRequestParam.nSequence = m_pManager->GetPacketSequence();
	stuRequestParam.pCondition = &stuCondition;
	stuRequestParam.callbackFunc = stuCallFunc.fun;
	stuRequestParam.userdata = stuCallFunc.data;
	stuRequestParam.pRecvBuf = NULL;
	stuRequestParam.nBufLen = 0;
	stuRequestParam.userparam = userdata;

	afk_channel_s *pRequestChannel = (afk_channel_s*)device->open_channel(device, AFX_CHANNEL_TYPE_REQUEST, &stuRequestParam);

	return (LONG)pRequestChannel;
}


int	CDecoderDevice::AddTourCombin(LONG lLoginID, int nMonitorID, int nSplitType, BYTE *pEncoderChannnel, int nBufLen, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || nMonitorID < 0 || NULL == pEncoderChannnel || nBufLen < nSplitType)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nCombinID = -1;
	int nRetLen = 0;
	char buffer[256] = {0};

	REQUEST_DEC_ADD_COMBIN stuCondition = {0};
	stuCondition.nMonitorID = nMonitorID;
	stuCondition.nSplitType = nSplitType;
	stuCondition.pEncoderChannnel = pEncoderChannnel;
	stuCondition.nBufLen = nBufLen;

	int ret = SysQueryInfo(lLoginID, AFK_REQUEST_DEC_ADD_COMBIN, &stuCondition, buffer, 256, &nRetLen, waittime);
	if (ret >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char*p = GetProtocolValue(buffer, "CombinID:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		nCombinID = atoi(szValue);
	}

	return nCombinID;
}


int	CDecoderDevice::DelTourCombin(LONG lLoginID, int nMonitorID, int nCombinID, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || nMonitorID < 0 || nCombinID < 0)
	{
		return NET_ILLEGAL_PARAM;
	}

	REQUEST_DEC_DEL_COMBIN stuCondition = {0};
	stuCondition.nMonitorID = nMonitorID;
	stuCondition.nCombinID = nCombinID;

	return SysSetupInfo(lLoginID, AFK_REQUEST_DEC_DEL_COMBIN, &stuCondition, waittime);
}


int	CDecoderDevice::SetTourCombin(LONG lLoginID, int nMonitorID, int nCombinID, int nSplitType, BYTE *pEncoderChannel, int nBufLen, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || nMonitorID < 0 || nCombinID < 0 || NULL == pEncoderChannel || nBufLen < nSplitType)
	{
		return NET_ILLEGAL_PARAM;
	}	

	REQUEST_DEC_SET_COMBIN stuCondition = {0};
	stuCondition.nMonitorID = nMonitorID;
	stuCondition.nCombinID = nCombinID;
	stuCondition.nSplitType = nSplitType;
	stuCondition.pEncoderChannnel = pEncoderChannel;
	stuCondition.nBufLen = nBufLen;

	return SysSetupInfo(lLoginID, AFK_REQUEST_DEC_SET_COMBIN, &stuCondition, waittime);
}


int	CDecoderDevice::QueryTourCombin(LONG lLoginID, int nMonitorID, int nCombinID, LPDEC_COMBIN_INFO lpDecCombinInfo, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == lpDecCombinInfo || nMonitorID < 0 || nCombinID < 0)
	{
		return NET_ILLEGAL_PARAM;
	}

	memset(lpDecCombinInfo, 0, sizeof(DEC_COMBIN_INFO));

	int nRet = -1;
	int nRetLen = 0;
	char buffer[512] = {0};

	REQUEST_DEC_DEL_COMBIN stuCondition = {0};
	stuCondition.nMonitorID = nMonitorID;
	stuCondition.nCombinID = nCombinID;

	nRet = SysQueryInfo(lLoginID, AFK_REQUEST_DEC_QUERY_COMBIN, &stuCondition, buffer, 512, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
 		char *p = GetProtocolValue(buffer, "CombinID:", "\r\n", szValue, 64);
 		if (p == NULL)
 		{
 			return NET_RETURN_DATA_ERROR;
 		}
		lpDecCombinInfo->nCombinID = atoi(szValue);

		p = GetProtocolValue(buffer, "DisMode:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		lpDecCombinInfo->nSplitType = atoi(szValue);
		
		for (int i = 0; i < lpDecCombinInfo->nSplitType; i++)
		{
			p = GetProtocolValue(p, "DisChn:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				return NET_RETURN_DATA_ERROR;
			}
			lpDecCombinInfo->bDisChn[i] = atoi(szValue);
		}	
	}

	return nRet;
}


int	CDecoderDevice::SetDecoderTour(LONG lLoginID, int nMonitorID, LPDEC_TOUR_COMBIN lpDecTourInfo, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || nMonitorID < 0 || NULL == lpDecTourInfo)
	{
		return NET_ILLEGAL_PARAM;
	}	
	
	REQUEST_DEC_SET_TOUR stuCondition = {0};
	stuCondition.nMonitorID = nMonitorID;
	stuCondition.nTourTime = lpDecTourInfo->nTourTime;
	stuCondition.pTourCombin = lpDecTourInfo->bCombinID;
	stuCondition.pCombinState = lpDecTourInfo->bCombinState;
	stuCondition.nCombinCount = lpDecTourInfo->nCombinNum;
	
	return SysSetupInfo(lLoginID, AFK_REQUEST_DEC_SET_TOUR, &stuCondition, waittime);
}


int	CDecoderDevice::QueryDecoderTour(LONG lLoginID, int nMonitorID, LPDEC_TOUR_COMBIN lpDecTourInfo, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || nMonitorID < 0 || NULL == lpDecTourInfo)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	memset(lpDecTourInfo, 0, sizeof(DEC_TOUR_COMBIN));

	int nRetLen = 0;
	char buffer[2048] = {0};

	int nRet = SysQueryInfo(lLoginID, AFK_REQUEST_DEC_QUERY_TOUR, (void *)nMonitorID, buffer, 2048, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = GetProtocolValue(buffer, "Interval:", "\r\n", szValue, 64);
 		if (p == NULL)
 		{
 			return NET_RETURN_DATA_ERROR;
 		}
		lpDecTourInfo->nTourTime = atoi(szValue);

		for (int i = 0; i < DEC_COMBIN_NUM; i++)
		{
			p = GetProtocolValue(p, "ComBinID:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				break;
			}
			lpDecTourInfo->bCombinID[i] = atoi(szValue);
			lpDecTourInfo->nCombinNum++;
		}
		
		p = buffer;
		for (int j = 0; j < lpDecTourInfo->nCombinNum; j++)
		{
			p = GetProtocolValue(p, "CombinState:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				break;
			}
			if (_stricmp(szValue, "True") == 0)
			{
				lpDecTourInfo->bCombinState[j] = 1;
			}
			else
			{
				lpDecTourInfo->bCombinState[j] = 0;
			}
		}
	}

	return nRet;
}


int	CDecoderDevice::QuerChannelFlux(LONG lLoginID, int nEncoderID, DEV_DECCHANNEL_STATE * lpChannelState, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || nEncoderID < 0 || NULL == lpChannelState)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	memset(lpChannelState, 0, sizeof(DEV_DECCHANNEL_STATE));
	
	int nRetLen = 0;
	char buffer[2048] = {0};
	
	int nRet = SysQueryInfo(lLoginID, AFK_REQUEST_DEC_CHANNEL_STATE, (void *)nEncoderID, buffer, 2048, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = GetProtocolValue(buffer, "DisChn:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		lpChannelState->byEncoderID = atoi(szValue);
		
		p = GetProtocolValue(buffer, "Data.Traffic:", "\r\n", szValue, 64);
		if (p)
		{
			lpChannelState->nChannelFLux = atoi(szValue);
		}
		
		p = GetProtocolValue(buffer, "Data.Resolution:", "\r\n", szValue, 64);
		if (p)
		{
			strncpy(lpChannelState->szResolution, szValue, 16);
		}
		
		p = GetProtocolValue(buffer, "Data.Frame:", "\r\n", szValue, 64);
		if (p)
		{
			lpChannelState->byFrame = atoi(szValue);
		}
		
		p = GetProtocolValue(buffer, "Decode.Traffic:", "\r\n", szValue, 64);
		if (p)
		{
			lpChannelState->nDecodeFlux = atoi(szValue);
		}
		
		p = GetProtocolValue(buffer, "ChnState:", "\r\n", szValue, 64);
		if (p)
		{
			if (_stricmp(szValue, "Monitor") == 0)
			{
				lpChannelState->byChnState = 1;
			}
			else if (_stricmp(szValue, "PlayBack") == 0)
			{
				lpChannelState->byChnState = 2;
			}
			else if (_stricmp(szValue, "DecodeTour") == 0)
			{
				lpChannelState->byChnState = 3;
			}
			else /*if (_stricmp(szValue, "Stopping") == 0)*/
			{
				lpChannelState->byChnState = 0;
			}
		}
	}
	
	return nRet;
}


LONG CDecoderDevice::DecTVPlayback(LONG lLoginID, int nEncoderID, DEC_PLAYBACK_MODE emPlaybackMode, LPVOID lpInBuffer, DWORD dwInBufferSize, void* userdata)
{
	LONG lOperateHandle = 0;
	switch(emPlaybackMode)
	{
	case Dec_By_Device_File:
		{
			if (dwInBufferSize >= sizeof(DEC_PLAYBACK_FILE_PARAM))
			{
				lOperateHandle = PlayBackByDevByFile(lLoginID, nEncoderID, (LPDEC_PLAYBACK_FILE_PARAM)lpInBuffer, userdata);
			}
		}
		break;
	case Dec_By_Device_Time:
		{
			if (dwInBufferSize >= sizeof(DEC_PLAYBACK_TIME_PARAM))
			{
				lOperateHandle = PlayBackByDevByTime(lLoginID, nEncoderID, (LPDEC_PLAYBACK_TIME_PARAM)lpInBuffer, userdata);
			}
		}
		break;
	default:
	    break;
	}

	return lOperateHandle;
}


int __stdcall DecTVPlaybackFunc(
							   afk_handle_t object,		/* CRequestChannel */
							   unsigned char *data,		/* 接收数据或回调数据 */
							   unsigned int datalen,	/* 数据长度 */
							   void *param,				/* 标志 */
							   void *udata)				/* CManager */
{
	afk_channel_s *pRequestChannel = (afk_channel_s*)object;
	CManager *pManager = (CManager*)udata;
	if (pRequestChannel == NULL || data == NULL || pManager == NULL)
	{
		return -1;
	}
	
	if ((int)param != -1)
	{
		// 将数据填充到用户缓冲中
	}
	else
	{
		// 应答包回调给用户
		AFK_RESPONSE_DATA *pResponseData = new AFK_RESPONSE_DATA;
		if (pResponseData == NULL)
		{
			return -1;
		}

		NET_CALLBACK_DATA *pCallBackData = new NET_CALLBACK_DATA;
		if (pCallBackData == NULL)
		{
			delete pResponseData;
			return -1;
		}

		MESS_CALLBACK_DATA *pData = (MESS_CALLBACK_DATA *)data;
		pCallBackData->nResultCode = pData->nResultCode;
		pCallBackData->lOperateHandle = pData->lOperateHandle;
		pCallBackData->userdata = pData->userparam;
		pCallBackData->pBuf = NULL;
		pCallBackData->nRetLen = 0;
		pResponseData->lCommand = RESPONSE_DECODER_PLAYBACK;
		pResponseData->lpCallBackData = pCallBackData;
		pResponseData->cbMessFunc = (fMessDataCallBack)pData->cbMessFunc;
		pResponseData->dwUserdata = (DWORD)pData->userdata;
		
		{
			pManager->GetResponseDataCS().Lock();
			pManager->GetResponseDataList().push_back(pResponseData);
			pManager->GetResponseDataCS().UnLock();

			SetEventEx(pManager->m_hResponseEvent);
		}
	}

	return 0;
}

LONG CDecoderDevice::PlayBackByDevByFile(LONG lLoginID, int nEncoderID, LPDEC_PLAYBACK_FILE_PARAM lpPlaybackParam, void* userdata)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == lpPlaybackParam)
	{
		return NET_ILLEGAL_PARAM;
	}

	REQUEST_DEC_DEVBYFILE stuCondition = {0};
	stuCondition.nChannelID = lpPlaybackParam->stuRecordInfo.ch;
	stuCondition.nFileSize = lpPlaybackParam->stuRecordInfo.size;
	stuCondition.nRecordFileType = lpPlaybackParam->stuRecordInfo.nRecordFileType;
	stuCondition.driveno = lpPlaybackParam->stuRecordInfo.driveno;
	stuCondition.startcluster = lpPlaybackParam->stuRecordInfo.startcluster;
	stuCondition.bHint = lpPlaybackParam->stuRecordInfo.bHint;
	stuCondition.bImportantRecID = lpPlaybackParam->stuRecordInfo.bImportantRecID;
	stuCondition.startTime.year = lpPlaybackParam->stuRecordInfo.starttime.dwYear - 2000;
	stuCondition.startTime.month = lpPlaybackParam->stuRecordInfo.starttime.dwMonth;
	stuCondition.startTime.day = lpPlaybackParam->stuRecordInfo.starttime.dwDay;
	stuCondition.startTime.hour = lpPlaybackParam->stuRecordInfo.starttime.dwHour;
	stuCondition.startTime.minute = lpPlaybackParam->stuRecordInfo.starttime.dwMinute;
	stuCondition.startTime.second = lpPlaybackParam->stuRecordInfo.starttime.dwSecond;
	stuCondition.endTime.year = lpPlaybackParam->stuRecordInfo.endtime.dwYear - 2000;
	stuCondition.endTime.month = lpPlaybackParam->stuRecordInfo.endtime.dwMonth;
	stuCondition.endTime.day = lpPlaybackParam->stuRecordInfo.endtime.dwDay;
	stuCondition.endTime.hour = lpPlaybackParam->stuRecordInfo.endtime.dwHour;
	stuCondition.endTime.minute = lpPlaybackParam->stuRecordInfo.endtime.dwMinute;
	stuCondition.endTime.second = lpPlaybackParam->stuRecordInfo.endtime.dwSecond;

	stuCondition.nEncoderID = lpPlaybackParam->byEncoderID;
	stuCondition.wEnable = lpPlaybackParam->bDevChnEnable;
	stuCondition.wDevPort = lpPlaybackParam->wDevPort;
	strcpy(stuCondition.szDevIp, lpPlaybackParam->szDevIp);
	strcpy(stuCondition.szDevUser, lpPlaybackParam->szDevUser);
	strcpy(stuCondition.szDevPwd, lpPlaybackParam->szDevPwd);

	DEVICE_MESS_CALLBACK stuCallFunc = {0};
	device->get_info(device, dit_set_callback, &stuCallFunc);

	afk_request_channel_param stuRequestParam = {0};
	stuRequestParam.base.func = DecTVPlaybackFunc;
	stuRequestParam.base.udata = m_pManager;
	stuRequestParam.nType = AFK_REQUEST_DEC_PLAYBACKBYDEC;
	stuRequestParam.nSequence = m_pManager->GetPacketSequence();
	stuRequestParam.nSubType = AFK_REQUEST_DEC_BYDECBYFILE;
	stuRequestParam.pCondition = &stuCondition;
	stuRequestParam.callbackFunc = stuCallFunc.fun;
	stuRequestParam.userdata = stuCallFunc.data;
	stuRequestParam.pRecvBuf = NULL;
	stuRequestParam.nBufLen = 0;
	stuRequestParam.userparam = userdata;

	afk_channel_s *pRequestChannel = (afk_channel_s*)device->open_channel(device, AFX_CHANNEL_TYPE_REQUEST, &stuRequestParam);

	return (LONG)pRequestChannel;
}


LONG CDecoderDevice::PlayBackByDevByTime(LONG lLoginID, int nEncoderID, LPDEC_PLAYBACK_TIME_PARAM lpPlaybackParam, void* userdata)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == lpPlaybackParam)
	{
		return NET_ILLEGAL_PARAM;
	}

	REQUEST_DEC_DEVBYTIME stuCondition = {0};
	stuCondition.nChannelID = lpPlaybackParam->nChannelID;
	stuCondition.startTime.year = lpPlaybackParam->startTime.dwYear - 2000;
	stuCondition.startTime.month = lpPlaybackParam->startTime.dwMonth;
	stuCondition.startTime.day = lpPlaybackParam->startTime.dwDay;
	stuCondition.startTime.hour = lpPlaybackParam->startTime.dwHour;
	stuCondition.startTime.minute = lpPlaybackParam->startTime.dwMinute;
	stuCondition.startTime.second = lpPlaybackParam->startTime.dwSecond;
	stuCondition.endTime.year = lpPlaybackParam->endTime.dwYear - 2000;
	stuCondition.endTime.month = lpPlaybackParam->endTime.dwMonth;
	stuCondition.endTime.day = lpPlaybackParam->endTime.dwDay;
	stuCondition.endTime.hour = lpPlaybackParam->endTime.dwHour;
	stuCondition.endTime.minute = lpPlaybackParam->endTime.dwMinute;
	stuCondition.endTime.second = lpPlaybackParam->endTime.dwSecond;

	stuCondition.nEncoderID = lpPlaybackParam->byEncoderID;
	stuCondition.wEnable = lpPlaybackParam->bDevChnEnable;
	stuCondition.wDevPort = lpPlaybackParam->wDevPort;
	strcpy(stuCondition.szDevIp, lpPlaybackParam->szDevIp);
	strcpy(stuCondition.szDevUser, lpPlaybackParam->szDevUser);
	strcpy(stuCondition.szDevPwd, lpPlaybackParam->szDevPwd);

	DEVICE_MESS_CALLBACK stuCallFunc = {0};
	device->get_info(device, dit_set_callback, &stuCallFunc);

	afk_request_channel_param stuRequestParam = {0};
	stuRequestParam.base.func = DecTVPlaybackFunc;
	stuRequestParam.base.udata = m_pManager;
	stuRequestParam.nType = AFK_REQUEST_DEC_PLAYBACKBYDEC;
	stuRequestParam.nSequence = m_pManager->GetPacketSequence();
	stuRequestParam.nSubType = AFK_REQUEST_DEC_BYDECBYTIME;
	stuRequestParam.pCondition = &stuCondition;
	stuRequestParam.callbackFunc = stuCallFunc.fun;
	stuRequestParam.userdata = stuCallFunc.data;
	stuRequestParam.pRecvBuf = NULL;
	stuRequestParam.nBufLen = 0;
	stuRequestParam.userparam = userdata;

	afk_channel_s *pRequestChannel = (afk_channel_s*)device->open_channel(device, AFX_CHANNEL_TYPE_REQUEST, &stuRequestParam);

	return (LONG)pRequestChannel;
}

int CDecoderDevice::SetDecPlaybackPos(LONG lLoginID, fDecPlayBackPosCallBack cbPlaybackPos, DWORD dwUser)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == cbPlaybackPos)
	{
		return NET_ILLEGAL_PARAM;
	}

	DEVICE_MESS_CALLBACK stuCallFunc = {0};
	stuCallFunc.fun = (void*)cbPlaybackPos;
	stuCallFunc.data = (void*)dwUser;
	device->set_info(device, dit_nvd_playback_flag, &stuCallFunc);

	return 0;
}

int	CDecoderDevice::CtrlDecPlayback(LONG lLoginID, int nEncoderID, DEC_CTRL_PLAYBACK_TYPE emCtrlType, int nValue, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || nEncoderID < 0)
	{
		return NET_ILLEGAL_PARAM;
	}	
	
	int nRet = 0;

	switch(emCtrlType)
	{
	case Dec_Playback_Seek:
		{
			REQUEST_DEC_SET_PLAYBACK stuCondition = {0};
			stuCondition.nType = emCtrlType;
			stuCondition.nEncoderNum = nEncoderID;
			stuCondition.nValue = nValue;
			SysSetupInfo(lLoginID, AFK_REQUEST_DEC_CTRLPLAYBACK, &stuCondition, 0);
		}
		break;
	case Dec_Playback_Play:
	case Dec_Playback_Pause:
	case Dec_Playback_Stop:
		{
			REQUEST_DEC_SET_PLAYBACK stuCondition = {0};
			stuCondition.nType = emCtrlType;
			stuCondition.nEncoderNum = nEncoderID;
			stuCondition.nValue = nValue;
			nRet = SysSetupInfo(lLoginID, AFK_REQUEST_DEC_CTRLPLAYBACK, &stuCondition, waittime);
		}
		break;
	default:
	    break;
	}
	
	return nRet;
}

int CDecoderDevice::CtrlDecTour(LONG lLoginID, int nMonitorID, DEC_CTRL_TOUR_TYPE emCtrlParm, int waittime)
{
	afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device || nMonitorID < 0)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int nRet = 0;
	
	REQUEST_DEC_CONTROL stuCondition = {0};
	stuCondition.nType = 0;
	stuCondition.nMonitorID = nMonitorID;
	stuCondition.nAction = emCtrlParm;
	nRet = SysSetupInfo(lLoginID, AFK_REQUEST_DEC_CONTROL, &stuCondition, waittime);
	if (nRet < 0 )
	{
		nRet = NET_ERROR_CTRL_DECODER_TOUR;
	}
	else
	{
		nRet = NET_NOERROR;
	}

	return nRet;
}










