
#include "StdAfx.h"
#include "Net/TcpSocket.h"
#include "Net/UdpSocket.h"
#include "Net/MulticastSocket.h"
#include "Net/TcpSockServer.h"
#include "Net/Broadcast.h"
#include "Net/SocketCallBack.h"
#include "dvrinterface.h"
#include "dvrdevice/dvrdevice.h"
#include "dvrdevice/dvrdevice_DDNS.h"
#include "dvrcap.h"
#include "Des.h"


/*
 * 创建登录包
 */
void build_login_packet(unsigned char *pBuf, const char *szUser, const char *szPassWord, int nSpecCap, void *pCapParam, unsigned int *pExtLen)
{
	*pExtLen = 0;
    memset(pBuf, 0, HEADER_SIZE);

	// fill buffer.
	pBuf[0] = 0xA0;
	pBuf[24]=4; // 主版本号
	pBuf[25]=1; // 副版本号

	//protocol version ——协议版本号，0－以前版本，2－二代，5－多连接，6－支持动态多连接(解决版本5多连接的缺陷)
	pBuf[3] = 6<<4;

    // 一般类型
    pBuf[26] = 0;

#ifdef NETSDK_VERSION_ENCRYPT
	pBuf[27] = 1;
#endif

	switch(nSpecCap) 
	{
	case 1:	// 无条件登录
		pBuf[28] = 0x3C; // 0x3C或0x3A
		break;
	default:
		break;
	}
	
    if (szUser != NULL && szPassWord != NULL)
    {
#ifdef NETSDK_VERSION_OEM
		pBuf[27] = 1;
		char szUserBuf[256] = {0};
		char szPwdBuf[256] = {0};
		char szOrigUser[64] = {0};
		char szOrigPwd[64] = {0};
		strcpy(szOrigUser, szUser);
		strcpy(szOrigPwd, szPassWord);
		DesEncrypt(szUserBuf, (char*)szOrigUser, strlen(szOrigUser), NETSDK_OEM_KEY, strlen(NETSDK_OEM_KEY));
		DesEncrypt(szPwdBuf, (char*)szOrigPwd, strlen(szOrigPwd), NETSDK_OEM_KEY, strlen(NETSDK_OEM_KEY));

		sprintf((char*)(pBuf+HEADER_SIZE), "%s&&%s", szUserBuf, szPwdBuf);
		*pExtLen = strlen(szUserBuf) + strlen(szPwdBuf) + 2;
		*(unsigned int*)(pBuf + 4) = *pExtLen;
#else
		if (strlen(szUser) <= 8 && strlen(szPassWord) <= 8)
		{
			memcpy(&pBuf[8], szUser, strlen(szUser));
			memcpy(&pBuf[16], szPassWord, strlen(szPassWord));
		}
        else
		{
			sprintf((char*)(pBuf+HEADER_SIZE), "%s&&%s", szUser, szPassWord);
			*pExtLen = strlen(szUser) + strlen(szPassWord) + 2;
			*(unsigned int*)(pBuf + 4) = *pExtLen;
		}
#endif
    }
	
	// for atm?
	pBuf[30] = 0xa1;
	pBuf[31] = 0xaa;
}


/*
 * 解析登录包
 */
BOOL parse_login_respond(unsigned char *pBuf, int nLen, afk_dvrdevice_info &stuDevInfo, int *pResultCode, int *pProtocolVer,int *pCard_Query_Info, int *pMultiPlayEnable, int *pIsQuickConfig)
{	
	if (NULL == pResultCode)
	{
		return FALSE;
	}

	*pResultCode = *(pBuf + 9);	// 偏移9为登录返回结果保存位置.
    if (*(pBuf + 8) != 0)
    {
        return FALSE;
    }

    memset(&stuDevInfo, 0, sizeof(afk_dvrdevice_info));
    
    int nType = PRODUCT_NONE;
	switch(*(pBuf + 12)) 
	{
	case DVR_PRODUCT_H:
		nType = PRODUCT_DVR_MEPG4_SH2;
		strcpy(stuDevInfo.szDevType,"PRODUCTDVR_HB");
		break;
	case DVR_PRODUCT_G:
		nType = PRODUCT_DVR_MEPG4_ST2;
		strcpy(stuDevInfo.szDevType,"PRODUCTDVR_GB");
		break;
	case DVR_PRODUCT_L:
		nType = PRODUCT_DVR_MPEG4_SX2;
		strcpy(stuDevInfo.szDevType,"PRODUCTDVR_LB");
		break;
	case DVR_PRODUCT_N:
		nType = PRODUCT_DVR_NONREALTIME;
		strcpy(stuDevInfo.szDevType,"PRODUCTDVR_NONREALTIME");
		break;
	case DVR_PRODUCT_ATM:
		nType = PRODUCT_DVR_ATM;
		strcpy(stuDevInfo.szDevType,"DVR_PRODUCT_ATM");
		break;
	case NVS_PRODUCT_S:
		nType = PRODUCT_NVS_S;
		strcpy(stuDevInfo.szDevType,"PRODUCTNVS_NVS_S");
		break;
	case NVS_PRODUCT_E: //nvs 0404e
		nType = PRODUCT_NVS_E;
		strcpy(stuDevInfo.szDevType,"PRODUCTNVS_NVS_E");
		break;
	case DVR_PRODUCT_NB:
		nType = PRODUCT_NB_SERIAL;
		strcpy(stuDevInfo.szDevType,"DVR_PRODUCT_NB");
		break;
	case DVR_PRODUCT_LN:
		nType = PRODUCT_LN_SERIAL;
		strcpy(stuDevInfo.szDevType,"DVR_PRODUCT_LN");
		break;
	case DVR_PRODUCT_BAV:
		nType = PRODUCT_BAV_SERIAL;
		strcpy(stuDevInfo.szDevType,"DVR_PRODUCT_BAV");
		break;
	case NVS_PRODUCT_B:
		nType = PRODUCT_NVS_B;
		strcpy(stuDevInfo.szDevType,"PRODUCTDVR_NVS_B");
		break;
	case NVS_PRODUCT_C:
		nType = PRODUCT_NVS_C;
		strcpy(stuDevInfo.szDevType,"PRODUCTNVS_NVS_H");
		break;
	case NVD_PRODUCT_SERIAL:
		nType = PRODUCT_NVD_SERIAL;
		strcpy(stuDevInfo.szDevType,"PRODUCT_NVD_SERIAL");
		break;
	case SDIP_PRODUCT:
		nType = PRODUCT_SDIP_SERIAL;
		strcpy(stuDevInfo.szDevType,"SDIP_PRODUCT");
		break;
	case IPC_PRODUCT:
		nType = PRODUCT_IPC_SERIAL;
		strcpy(stuDevInfo.szDevType,"IPC_PRODUCT");
		break;
	case IPC_PRODUCT_400:
		nType = PRODUCT_IPC_SERIAL;
		strcpy(stuDevInfo.szDevType,"IPC_PRODUCT_400");
		break;
	case IPC_PRODUCT_A6:
		nType = PRODUCT_IPC_SERIAL;
		strcpy(stuDevInfo.szDevType,"IPC_PRODUCT_A6");
		break;
	case N5_PRODUCT:
		nType = PRODUCT_DVR_N5;
		strcpy(stuDevInfo.szDevType, "PRODUCT_DVR_N5");
		break;
	case MIX_DVR_PRODUCT:
		nType = PRODUCT_DVR_MIX_DVR;
		strcpy(stuDevInfo.szDevType, "PRODUCT_DVR_MIX_DVR");
		break;
	case SVR_PRODUCT:
		if (*(pBuf + 13) == 0x01)
		{
			nType = PRODUCT_SVR_BS;
			strcpy(stuDevInfo.szDevType, "PRODUCT_SVR_BS");
		}
		else
		{
			nType = PRODUCT_SVR_SERIAL;
			strcpy(stuDevInfo.szDevType, "PRODUCT_SVR_SERIAL");
		}
		break;
	case NVR_PRODUCT:
		nType = PRODUCT_NVR_SERIAL;
		strcpy(stuDevInfo.szDevType, "PRODUCT_NVR_SERIAL");
		break;
	case DEV_NORMAL_TYPE_PRODUCT:
		{
			nType = PRODUCT_DVR_NEW_PROTOCOL;
			memcpy(stuDevInfo.szDevType,(pBuf+HEADER_SIZE),pBuf[4]);
		}
		break;
	default:
		nType = PRODUCT_DVR_NEW_PROTOCOL; // 默认为LB
		strcpy(stuDevInfo.szDevType,"Unknown device type.");
	}

	*pProtocolVer = pBuf[3]>>4;
    stuDevInfo.type = nType;
    stuDevInfo.channelcount = pBuf[10];
	stuDevInfo.ispreview = pBuf[1]!=0?true:false;
    stuDevInfo.ispal = (pBuf[28] == 0);
    stuDevInfo.version = 0/*pBuf[24] << 8 | pBuf[25]*/;
    stuDevInfo.enctype = pBuf[11];
    stuDevInfo.alarminputcount = alarminputCount(nType, pBuf);
    stuDevInfo.alarmoutputcount = alarmoutputCount(nType, pBuf);
    stuDevInfo.istrans = transability(nType);
    stuDevInfo.istalk = talkability(nType);
	*pCard_Query_Info = pBuf[31]&0x08 ? 1 : 0;//第3位：是否允许卡号查询标志
	*pMultiPlayEnable = pBuf[1];
	*pIsQuickConfig = pBuf[20];  //add by cqs 是否支持不支持的命令快速返回，如果支持建议用户配置的超时时间设置60S，以保证窄带下正常获取配置
    return TRUE;
}


/*
 * 模仿SSL发送数据，用在特殊版本
 */
#ifdef NETSDK_VERSION_BOGUSSSL
int device_ssl_send_data(CTcpSocket* pTcpSocket, int nTimeout)
{
	pTcpSocket->WriteData((char*)ssl1, 130);
	DWORD dwRet = WaitForSingleObjectEx(pTcpSocket->m_hSpecialEvent, nTimeout);
	if (dwRet != WAIT_OBJECT_0)
	{
		return -1;
	}
	
	pTcpSocket->WriteData((char*)ssl2, 182);
	dwRet = WaitForSingleObjectEx(pTcpSocket->m_hSpecialEvent, nTimeout);
	if (dwRet != WAIT_OBJECT_0)
	{
		return -1;
	}
	
	pTcpSocket->WriteData((char*)ssl3, 49);
	dwRet = WaitForSingleObjectEx(pTcpSocket->m_hSpecialEvent, nTimeout);
	if (dwRet != WAIT_OBJECT_0)
	{
		return -1;
	}
	
	pTcpSocket->WriteData((char*)ssl4, 35);
	dwRet = WaitForSingleObjectEx(pTcpSocket->m_hSpecialEvent, nTimeout);
	if (dwRet != WAIT_OBJECT_0)
	{
		return -1;
	}
	
	pTcpSocket->WriteData((char*)ssl5, 27);
	dwRet = WaitForSingleObjectEx(pTcpSocket->m_hSpecialEvent, nTimeout);
	if (dwRet != WAIT_OBJECT_0)
	{
		return -1;
	}
	
	pTcpSocket->WriteData((char*)ssl6, 27);
	dwRet = WaitForSingleObjectEx(pTcpSocket->m_hSpecialEvent, nTimeout);
	if (dwRet != WAIT_OBJECT_0)
	{
		return -1;
	}
	
	pTcpSocket->WriteData((char*)ssl7, 27);
	dwRet = WaitForSingleObjectEx(pTcpSocket->m_hSpecialEvent, nTimeout);
	if (dwRet != WAIT_OBJECT_0)
	{
		return -1;
	}

	return 0;
}
#endif


CDvrDevice* create_dvr_device(afk_dvrdevice_info stuDevInfo, CTcpSocket *pTcpSocket, 
                             const char *szIp, int nPort, const char *szUser, const char *szPassWord, 
                             fDisconnectCallBack cbDisconnect, fDeviceEventCallBack cbDevEvent, void *userdata)
{
    CDvrDevice *pDvrDevice = NULL;
    switch(stuDevInfo.type)
    {
    case PRODUCT_DVR_NONREALTIME_MACE:	// 非实时MACE
    case PRODUCT_DVR_NONREALTIME:		// 非实时
    case PRODUCT_NVS_MPEG1:				// 网络视频服务器
    case PRODUCT_DVR_MPEG1_2:			// MPEG1二路录像机
    case PRODUCT_DVR_MPEG1_8:			// MPEG1八路录像机
		break;
    case PRODUCT_DVR_MPEG4_8:			// MPEG4八路录像机
    case PRODUCT_DVR_MPEG4_16:			// MPEG4十六路录像机
    case PRODUCT_DVR_MPEG4_SX2:			// MPEG4视新十六路录像机
    case PRODUCT_DVR_MEPG4_ST2:			// MPEG4视通录像机
    case PRODUCT_DVR_MEPG4_SH2:			// MPEG4视豪录像机
    case PRODUCT_DVR_MPEG4_GBE:			// MPEG4视通二代增强型录像机
    case PRODUCT_DVR_MPEG4_NVSII:		// MPEG4网络视频服务器II代
	case PRODUCT_DVR_ATM:				// ATM机
	case PRODUCT_NB_SERIAL:				// 二代非实时NB系列机器
	case PRODUCT_LN_SERIAL:				// LN系列产品
	case PRODUCT_BAV_SERIAL:				// BAV系列产品
	case PRODUCT_SDIP_SERIAL:			// SDIP系列产品
	case PRODUCT_IPC_SERIAL:				// IPC系列产品
	case PRODUCT_NVS_B:					// NVS B系列
	case PRODUCT_NVS_C:					// NVS H系列
	case PRODUCT_NVS_S:					// NVS S系列
	case PRODUCT_NVS_E:					// NVS E系列
	case PRODUCT_DVR_NEW_PROTOCOL:		// 设备类型
	case PRODUCT_NVD_SERIAL:				// 解码器
	case PRODUCT_DVR_N5:                 // N5
	case PRODUCT_DVR_MIX_DVR:            // 混合DVR
	case PRODUCT_SVR_SERIAL:				// SVR
	case PRODUCT_SVR_BS:					// SVR-BS
	case PRODUCT_NVR_SERIAL:				// NVR
        pDvrDevice = new CDvrDevice(stuDevInfo, pTcpSocket, szIp, nPort, szUser, szPassWord, cbDisconnect, cbDevEvent, userdata);
        break;
    case PRODUCT_DVR_STD_NEW: /*!< 新标准配置协议 */
        break;
	case PRODUCT_DVR_DDNS:  /*!< DDNS服务器 */
		pDvrDevice = new CDvrDevice_DDNS(stuDevInfo, pTcpSocket, szIp, nPort, szUser, szPassWord, cbDisconnect, cbDevEvent, userdata);
        break;
    default:
        pDvrDevice = NULL;
    }

    return (CDvrDevice*)pDvrDevice;
}


void* try_connect_dvr(const char *szIp, int nPort, const char *szUser, const char *szPassWord, 
                  int nSpecCap, void *pCapParam, fDisconnectCallBack cbDisconnect, fDeviceEventCallBack cbDevEvent, void *userdata, 
				  int *pErrorCode, int nTimeout, int nTryTime, BOOL bIsReConn, int nConnHostTime, int nSubConnSpaceTime, int nConnBufLen)
{
#ifdef NETSDK_VERSION_SSL
	BOOL bSSL = FALSE;
	// SSL
	if (nSpecCap == 7)
	{
		nPort -= 1;
		bSSL = TRUE;
	}
#endif

	/* 创建主连接 */
	CTcpSocket *pTcpSocket = NULL;
	afk_connect_param_t stuConnParam = {0};

	stuConnParam.nConnType = channel_connect_tcp;
	stuConnParam.nConnTime = nConnHostTime;
	stuConnParam.nTryTime = nTryTime;
	stuConnParam.nConnBufSize = nConnBufLen;
	strcpy(stuConnParam.szSevIp, szIp);
	stuConnParam.nSevPort = nPort;

	if (2 == nSpecCap)
	{
		pTcpSocket = (CTcpSocket*)pCapParam;
		if (NULL == pTcpSocket)
		{
			*pErrorCode = 0;
			return NULL;
		}
	}
	else
	{
#ifdef NETSDK_VERSION_SSL
		pTcpSocket = device_create_connect<CTcpSocket>(&stuConnParam, bSSL);
#else
		pTcpSocket = device_create_connect<CTcpSocket>(&stuConnParam);
#endif
		if (pTcpSocket == NULL)
		{
			*pErrorCode = 9;	// 主连接失败
			return NULL;
		}
	}

#ifdef NETSDK_VERSION_SSL
	// SSL
	if (nSpecCap == 7)
	{
		nPort += 1;
		stuConnParam.nSevPort = nPort;
	}
#endif

	/* DDNS服务器连接 */
	if (!szUser && !szPassWord)
	{
		afk_dvrdevice_info stuDevInfo = {0};
		stuDevInfo.type = PRODUCT_DVR_DDNS;

		CDvrDevice *pDvrDevice = create_dvr_device(stuDevInfo, pTcpSocket, szIp, nPort, szUser, szPassWord, cbDisconnect, cbDevEvent, userdata);
		if (pDvrDevice == NULL)
		{
			pTcpSocket->Disconnect();
			delete pTcpSocket;
			*pErrorCode = 7;
			return NULL;	
		}
		
		pTcpSocket->SetIsDetectDisconn(0);
		pTcpSocket->SetCallBack(OnDisconnect, OnReConnect, OnOtherPacket, OnReceivePacket, pDvrDevice);
		return (afk_device_s*)pDvrDevice;
	}

#ifdef NETSDK_VERSION_BOGUSSSL
	int ret = device_ssl_send_data(pTcpSocket, nTimeout);
	if (ret < 0)
	{
		pTcpSocket->Disconnect();
		delete pTcpSocket;
		*pErrorCode = 3;
		return NULL;
	}
#endif

	/* 登入设备 */
	unsigned int nCommandLen = 0;
	unsigned char szCommand[160] = {0};
	
	build_login_packet(szCommand, szUser, szPassWord, nSpecCap, pCapParam, &nCommandLen);
	Sleep(50);
	pTcpSocket->WriteData((char*)szCommand, HEADER_SIZE+nCommandLen);

	// 同步等待相应包
	DWORD dwRet = WaitForSingleObjectEx(pTcpSocket->m_hRecEvent, nTimeout);
	ResetEventEx(pTcpSocket->m_hRecEvent);
	if (dwRet != WAIT_OBJECT_0)
	{
		pTcpSocket->Disconnect();
		delete pTcpSocket;
		*pErrorCode = 3;
		return NULL;
	}

	afk_dvrdevice_info stuDevInfo = {0};
	int result= 0;
	int protocol_version = 0;
	int Card_Enable_Info = 0;
	int	nMultiPlayEn = 0;
	unsigned int nSessionId = 0;
	int IsQuickConfig = 0;
	
	// 分析返回包
	if (!parse_login_respond(pTcpSocket->m_registerAck, pTcpSocket->m_nRegisterLen, stuDevInfo, &result, &protocol_version, &Card_Enable_Info, &nMultiPlayEn, &IsQuickConfig))
	{
		pTcpSocket->Disconnect();
		delete pTcpSocket;
		*pErrorCode = result + 1;
		return NULL;
	}

	/* 创建设备对象 */
	stuDevInfo.bAutoRegister = (2 == nSpecCap) ? TRUE : FALSE;
	CDvrDevice *pDvrDevice = create_dvr_device(stuDevInfo, pTcpSocket, szIp, nPort, szUser, szPassWord, cbDisconnect, cbDevEvent, userdata);
	if (pDvrDevice == NULL)
	{
		pTcpSocket->Disconnect();
		delete pTcpSocket;
        *pErrorCode = 7;
		return NULL;	
	}

	nSessionId = *(unsigned int*)(pTcpSocket->m_registerAck + 16);
	pDvrDevice->device_set_info(dit_protocol_version, &protocol_version);
	pDvrDevice->device_set_info(dit_session_id, &nSessionId);
	pDvrDevice->device_set_info(dit_card_query, &Card_Enable_Info);
	pDvrDevice->device_set_info(dit_multiplay_flag, &nMultiPlayEn);
	pDvrDevice->device_set_info(dit_quick_config, &IsQuickConfig);

	{
		unsigned char heartbeat[HEADER_SIZE];
		memset(heartbeat, 0, HEADER_SIZE);
		heartbeat[0] = 0xA1;
		
#ifdef NETSDK_VERSION_INVERSE
		for (int i = 0; i < HEADER_SIZE; i++)
		{
			unsigned char *p = (unsigned char*)(heartbeat + i);
			*p = ~(*p);
		}
#endif
		pTcpSocket->SetKeepLife(heartbeat, HEADER_SIZE);
	}

	pTcpSocket->SetIsReConn(bIsReConn?1:0);
	pTcpSocket->SetIsDetectDisconn(1);
	pTcpSocket->SetCallBack(OnDisconnect, OnReConnect, OnOtherPacket, OnReceivePacket, pDvrDevice);

	if (protocol_version < 5 || protocol_version >= 6 || 2 == nSpecCap || 6 == nSpecCap)
	{
		return (afk_device_s*)pDvrDevice;
	}

	memset(szCommand, 0, 32);
	szCommand[0] = 0xA1;

	/*********************多连接，每个通道一个连接*********************/
	// 协议版本号==5时，建立子连接
	int i = 0;
	if (3 == nSpecCap || 4 == nSpecCap || IsQuickConfig == 1) // 支持快速配置的视频通道子连接在后面创建
	{
		i = stuDevInfo.channelcount;
	}
	for (i; i < stuDevInfo.channelcount + 1; i++)
	{
		pTcpSocket->WriteData((char*)szCommand, HEADER_SIZE);

		stuConnParam.nConnectID = i;
		stuConnParam.userdata = pDvrDevice;
		if (result == 8) // 子连接要求等待注册应答包
		{
			stuConnParam.nParam = nTimeout;
		}
		else
		{
			stuConnParam.nParam = 0;
		}

		CTcpSocket* pSubTcpSocket = (CTcpSocket*)pDvrDevice->CreateSubConn(&stuConnParam);
		if (NULL == pSubTcpSocket)
		{
			pDvrDevice->device_close();
			pDvrDevice->device_DecRef();
			*pErrorCode = 8;
			return NULL;
		}

		if (0 == stuConnParam.nParam)
		{
			Sleep(nSubConnSpaceTime);
		}
	}
	
	return (afk_device_s*)pDvrDevice;
}


/*
 * 主动侦听设备
 */
void* start_listen_dvr(char *szIp, int nPort, fListenCallback cbListen, void *userdata, int nConnBufLen)
{
	CTcpSocket *pTcpSocket = new CTcpSocket;
	if (pTcpSocket == NULL)
	{
		return NULL;
	}
	if (pTcpSocket->CreateRecvBuf(nConnBufLen) < 0)
	{
		delete pTcpSocket;
		return NULL;
	}

	int ret = pTcpSocket->StartListen(szIp, nPort, cbListen, userdata);
	if (ret < 0)
	{
		delete pTcpSocket;
		return NULL;
	}

	return (void*)pTcpSocket;
}


/*
 * 停止侦听设备
 */
BOOL stop_listen_dvr(void *pListenHandle)
{
	CTcpSocket *pTcpSocket = (CTcpSocket*)pListenHandle;
	if (NULL == pTcpSocket)
	{
		return FALSE;
	}

	int ret = pTcpSocket->StopListen();
	delete pTcpSocket;

	return ret>=0?TRUE:FALSE;
}


/*
 * 关闭客户端连接
 */
BOOL close_client_dvr(void *pClientHandle)
{
	CTcpSocket *pTcpSocket = (CTcpSocket*)pClientHandle;
	if (NULL == pTcpSocket)
	{
		return FALSE;
	}

	pTcpSocket->Disconnect();
	delete pTcpSocket;

	return TRUE;
}


BOOL response_reg_dvr(void *pClientHandle, BOOL bAccept)
{
	CTcpSocket *pTcpSocket = (CTcpSocket*)pClientHandle;
	if (NULL == pTcpSocket)
	{
		return FALSE;
	}
	
	pTcpSocket->ResponseReg(bAccept);
	
	return TRUE;
}


/*
 * 初始化网络环境
 */
BOOL network_startup(void)
{
	int ret = CTcpSocket::InitNetwork();
	return ret >= 0 ? TRUE:FALSE;
}


/*
 * 清理网络环境
 */
BOOL network_cleanup(void)
{
	int ret = CTcpSocket::ClearNetwork();
	return ret >= 0 ? TRUE:FALSE;
}


LONG _start_alarm_server(char *szIp, int nPort, fEventCallBack RecAlarmCallBack, unsigned long user_data)
{
	CTcpSockServer *pAlarmServer = new CTcpSockServer;
	if(NULL == pAlarmServer)
	{
		return 0;
	}

	int nRet = pAlarmServer->StartListen(szIp, nPort, RecAlarmCallBack, user_data);
	if(nRet <= 0)
	{
		delete pAlarmServer;
		return 0;
	}

	return (LONG)pAlarmServer;
}

BOOL _stop_alarm_server(LONG lHandler)
{
	if (lHandler <= 0)
	{
		return FALSE;
	}

	CTcpSockServer *pAlarmServer = (CTcpSockServer *)lHandler;
	bool bReturn = pAlarmServer->StopListen();
	delete pAlarmServer;
	
	return bReturn;
}

int _heart_beat(LONG lHandler)
{
	if(lHandler <= 0)
	{
		return -1;
	}

	CTcpSockServer *pAlarmServer = (CTcpSockServer *)lHandler;
	return pAlarmServer->HeartBeat();
}

LONG _start_search_device(afk_callback cbReceived, void* pUserData)
{
	CBroadcast *pBroadcast = new CBroadcast;
	if(NULL == pBroadcast)
	{
		return -1;
	}

	if (pBroadcast->CreateRecvBuf(200*1024) < 0)
	{
		delete pBroadcast;
		return -1;
	}

	pBroadcast->SetCallBack(cbReceived, pUserData);
	if (pBroadcast->ConnectHost(NULL, 5050, 5050) < 0)
	{
		return -1;
	}
	
	unsigned char szData[32];
	memset(szData, 0, sizeof(szData));
	szData[0] = 0xa3;
	szData[1] = 1;
	szData[16] = 2;
	pBroadcast->WriteData((char *)szData, 32);
	
	return (LONG)pBroadcast;
}


int _stop_search_device(LONG lHandle)
{
	CBroadcast *pBroadcast = (CBroadcast *)lHandle;
	if(NULL == pBroadcast)
	{
		return -1;
	}
	
	pBroadcast->Disconnect();
	delete pBroadcast;
	return 1;
}

LONG _new_tcp_socket(char* szIP, int nPort, int nWaitTime)
{
	CTcpSocket *pTcpSocket = new CTcpSocket;
	if(NULL == pTcpSocket)
	{
		return NULL;
	}

	pTcpSocket->ConnectHost(szIP, nPort, nWaitTime);

	return (LONG)pTcpSocket;
}

int _delete_tcp_socket(LONG lHandle)
{
	CTcpSocket *pTcpSocket = (CTcpSocket *)lHandle;
	if(NULL != pTcpSocket)
	{
		pTcpSocket->Disconnect();
		delete pTcpSocket;
		return 1;
	}
	else
	{
		return -1;
	}
}

int _write_data(LONG lHandle, char *pBuf, int nBufLen)
{
	CTcpSocket *pTcpSocket = (CTcpSocket *)lHandle;
	if(NULL == pTcpSocket)
	{
		return -1;
	}

	return pTcpSocket->WriteData(pBuf, nBufLen);
}
