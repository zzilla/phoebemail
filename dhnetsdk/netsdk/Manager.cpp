
#include "StdAfx.h"
#include "../dvr/dvr.h"
#include "../dvr/dvrdevice/dvr2cfg.h"
#include "Manager.h"
#include "RealPlay.h"
#include "SearchRecordAndPlayBack.h"
#include "AlarmDeal.h"
#include "Talk.h" 
#include "DevConfig.h"
#include "DevConfigEx.h"
#include "DevControl.h"
#include "RenderManager.h"
#include "ServerSet.h"
#include "SnapPicture.h"
#include "GPSSubcrible.h"
#include "DecoderDevice.h"
#include "AutoRegister.h"
#include "TransBurnFile.h"
#include "Utils.h"
#include "DevNewConfig.h"


CManager::CManager()
{
	m_pRealPlayMdl		= new CRealPlay(this);
	m_pPlayBackMdl		= new CSearchRecordAndPlayBack(this);
	m_pRenderManager	= new CRenderManager(this);
	m_pAlarmMdl			= new CAlarmDeal(this);
	m_pTalkMdl			= new CTalk(this);
	m_pConfigMdl		= new CDevConfig(this);
	m_pConfigExMdl		= new CDevConfigEx(this);
	m_pControlMdl		= new CDevControl(this);
	m_pAlarmServerMdl	= new CServerSet(this);
	m_pSnapPicMdl		= new CSnapPicture(this);
	m_pGPSMdl			= new CGPSSubcrible(this);
	m_pNVDMdle			= new CDecoderDevice(this);
	m_pAutoRegiMdl		= new CAutoRegister(this);
	m_pDynamicThread	= new CDynamicThread();
	m_pNewConfigMdl     = new CDevNewConfig(this);

	if (NULL == m_pRealPlayMdl || NULL == m_pPlayBackMdl || NULL == m_pRenderManager || 
		NULL == m_pAlarmMdl || NULL == m_pTalkMdl || NULL == m_pConfigMdl || NULL == m_pConfigExMdl || 
		NULL == m_pControlMdl || NULL == m_pAlarmServerMdl || NULL == m_pSnapPicMdl || NULL == m_pGPSMdl || 
		NULL == m_pNVDMdle || NULL == m_pAutoRegiMdl || NULL == m_pDynamicThread || NULL == m_pNewConfigMdl)
	{
		throw 0xFFFFFFFF;
	}

	int ret = CreateEventEx(m_hDisEvent, FALSE, FALSE);
	if (ret < 0)
	{
		throw 0xFFFFFFFF;
	}
	
	ret = CreateEventEx(m_hExitThread, TRUE, FALSE);
	if (ret < 0)
	{
		throw 0xFFFFFFFF;
	}
	
	ret = CreateEventEx(m_hAlarmDataEvent, FALSE, FALSE);
	if (ret < 0)
	{
		throw 0xFFFFFFFF;
	}

	ret = CreateEventEx(m_hCleanResEvent, FALSE, FALSE);
	if (ret < 0)
	{
		throw 0xFFFFFFFF;
	}

	ret = CreateEventEx(m_hAutoRegiEvent, FALSE, FALSE);
	if (ret < 0)
	{
		throw 0xFFFFFFFF;
	}

	ret = CreateEventEx(m_hAutoRegiDisEvent, FALSE, FALSE);
	if (ret < 0)
	{
		throw 0xFFFFFFFF;
	}

	ret = CreateEventEx(m_hResponseEvent, FALSE, FALSE);
	if (ret < 0)
	{
		throw 0xFFFFFFFF;
	}

	ret = CreateEventEx(m_hNVDPlaybackEvent, FALSE, FALSE);
	if (ret < 0)
	{
		throw 0xFFFFFFFF;
	}

	m_dwErrorCode = 0;
	m_pDeviceProb = NULL;
	m_dwPacketSequence = 0;
	
	m_nLoginTimeout = 5000;
	m_nConnTryNum = 1;
	m_nConnHostTime = 1500;
	m_nSubConnSpaceTime = 10;
	m_nConnBufLen = 250*1024;
	m_nGetDevInfoTime = 1000;
	m_nGetSubConnTime = 1000;

	m_pfDisConnect = NULL;
	m_pfReConnect = NULL;
	m_pfSubDisConnect = NULL;
	m_pfMessCallBack = NULL;
	m_pfDrawCallBack = NULL;
	m_dwDisConnectUser = 0;
	m_dwHaveReconnUser = 0;
	m_dwSubDisconnUser = 0;
	m_dwDrawUser = 0;

	InterlockedSetEx(&m_nLifeCount, 0);
}

CManager::~CManager()
{
	if (m_pRealPlayMdl != NULL)
	{
		delete m_pRealPlayMdl;
		m_pRealPlayMdl = NULL;
	}

	if (m_pPlayBackMdl != NULL)
	{
		delete m_pPlayBackMdl;
		m_pPlayBackMdl = NULL;
	}

	if (m_pRenderManager != NULL)
	{
		delete m_pRenderManager;
		m_pRenderManager = NULL;
	}

	if (m_pAlarmMdl != NULL)
	{
		delete m_pAlarmMdl;
		m_pAlarmMdl = NULL;
	}
	
	if (m_pTalkMdl != NULL)
	{
		delete m_pTalkMdl;
		m_pTalkMdl = NULL;
	}

	if (m_pConfigMdl != NULL)
	{
		delete m_pConfigMdl;
		m_pConfigMdl = NULL;
	}

	if (m_pConfigExMdl != NULL)
	{
		delete m_pConfigExMdl;
		m_pConfigExMdl = NULL;
	}

	if (m_pNewConfigMdl != NULL)
	{
		delete m_pNewConfigMdl;
		m_pNewConfigMdl = NULL;
	}

	if (m_pControlMdl != NULL)
	{
		delete m_pControlMdl;
		m_pControlMdl = NULL;
	}

	if (m_pAlarmServerMdl != NULL)
	{
		delete m_pAlarmServerMdl;
		m_pAlarmServerMdl = NULL;
	}

	if (m_pSnapPicMdl != NULL)
	{
		delete m_pSnapPicMdl;
		m_pSnapPicMdl = NULL;
	}

	if (m_pGPSMdl != NULL)
	{
		delete m_pGPSMdl;
		m_pGPSMdl = NULL;
	}

	if (m_pNVDMdle != NULL)
	{
		delete m_pNVDMdle;
		m_pNVDMdle = NULL;
	}

	if (m_pAutoRegiMdl != NULL)
	{
		delete m_pAutoRegiMdl;
		m_pAutoRegiMdl = NULL;
	}

	if (m_pDynamicThread != NULL)
	{
		delete m_pDynamicThread;
		m_pDynamicThread = NULL;
	}

	std::list<AFK_LAST_ERROR *>::iterator it = m_lstLastError.begin();
	while (it != m_lstLastError.end())
	{
		if ((*it))
		{
			delete (*it);
		}
		m_lstLastError.erase(it++);
	}

	CloseEventEx(m_hDisEvent);
	CloseEventEx(m_hExitThread);
	CloseEventEx(m_hAlarmDataEvent);
	CloseEventEx(m_hCleanResEvent);
	CloseEventEx(m_hAutoRegiEvent);
	CloseEventEx(m_hAutoRegiDisEvent);
	CloseEventEx(m_hResponseEvent);
	CloseEventEx(m_hNVDPlaybackEvent);
}

//////////////////////////////////////////////////////////////////////////

/*
 * 摘要：处理回调线程
 */
void* WINAPI RoutineThreadProc(LPVOID lpParam)
{
	CManager *pThis = (CManager*)lpParam;
	if (pThis == NULL)
	{
		return NULL;
	}

#ifdef WIN32
	HANDLE hWait[] = {pThis->m_hDisEvent.m_hEvent,pThis->m_hAlarmDataEvent.m_hEvent, pThis->m_hAutoRegiEvent.m_hEvent, pThis->m_hAutoRegiDisEvent.m_hEvent, pThis->m_hResponseEvent.m_hEvent, pThis->m_hNVDPlaybackEvent.m_hEvent, pThis->m_hExitThread.m_hEvent};
#endif

	BOOL bExit = FALSE;
	while(!bExit)
	{
#ifdef WIN32
		DWORD dwRet = WaitForMultipleObjects(sizeof(hWait)/sizeof(HANDLE),hWait,FALSE,100);
#else	//linux
		DWORD dwRet = -1;

		int timeOut = 20;
		while (--timeOut > 0)
		{
			if (0 == WaitForSingleObjectEx(pThis->m_hDisEvent, 0))
			{
				dwRet = WAIT_OBJECT_0;
				break;
			}
			if (0 == WaitForSingleObjectEx(pThis->m_hAlarmDataEvent, 0))
			{
				dwRet = WAIT_OBJECT_0+1;
				break;
			}
			if (0 == WaitForSingleObjectEx(pThis->m_hAutoRegiEvent, 0))
			{
				dwRet = WAIT_OBJECT_0+2;
			}
			if (0 == WaitForSingleObjectEx(pThis->m_hAutoRegiDisEvent, 0))
			{
				dwRet = WAIT_OBJECT_0+3;
			}
			if (0 == WaitForSingleObjectEx(pThis->m_hResponseEvent, 0))
			{
				dwRet = WAIT_OBJECT_0+4;
				break;
			}
			if (0 == WaitForSingleObjectEx(pThis->m_hNVDPlaybackEvent, 0))
			{
				dwRet = WAIT_OBJECT_0+5;
				break;
			}
			if (0 == WaitForSingleObjectEx(pThis->m_hExitThread, 0))
			{
				dwRet = WAIT_OBJECT_0+6;
				break;
			}

			usleep(5*1000);
		}
#endif
		switch(dwRet) {
		case WAIT_OBJECT_0:
			{
				pThis->DealDisconnCallback();
			}
			break;
		case WAIT_OBJECT_0+1:	
			{
				pThis->DealAlarmDataCallback();
			}
			break;
		case WAIT_OBJECT_0+2:
			{
				pThis->DealAutoRegiCallback();
			}
			break;
		case WAIT_OBJECT_0+3:
			{
				pThis->DealAutoRegiDisCallback();
			}
			break;
		case WAIT_OBJECT_0+4:
			{
				pThis->DealResponseDataCallback();
			}
			break;
		case WAIT_OBJECT_0+5:
			{
				pThis->DealNVDPosCallback();
			}
			break;
		case WAIT_OBJECT_0+6:
			{
				bExit = TRUE;
			}
			break;
		default:
			break;
		}
	}

	return NULL;
}

void* WINAPI HeatBeatThreadProc(LPVOID lpParam)
{
	CManager *pThis = (CManager*)lpParam;
	if (pThis == NULL)
	{
		return NULL;
	}

	BOOL bSignal = TRUE;
	int nAsynSpanTime = 0;
	int nTimeCount = 0;
	while (TRUE)
	{
		DWORD dwRet = WaitForSingleObjectEx(pThis->m_hExitThread, 500);
		if (WAIT_OBJECT_0 == dwRet)
		{
			break;
		}

		if (bSignal)
		{
			bSignal = FALSE;
			
			//send heart beat
			pThis->AllSendHeartBeat();
		}
		else
		{
			bSignal = TRUE;
			
			if (pThis->GetHaveReconnFunc() != NULL)
			{
				//login and realplay
				pThis->AllDevExecuteTask();
			}
		}

		// 异步接口，超时则删除资源
		if (++nAsynSpanTime > 20)
		{
			nAsynSpanTime = 0;
			pThis->AllAsynTimeoutDetect();
		}

		// 报警服务器心跳
		nTimeCount = (++nTimeCount) > 2 ? 2 : nTimeCount;
		if(nTimeCount >= 2)
		{
			nTimeCount = 0;
			pThis->GetASCS().Lock();
			pThis->m_pDeviceProb->server_heart_beat(pThis->GetAlarmServer().GetHandle());
			pThis->GetASCS().UnLock();
		}
	}
	
	return NULL;
}

void* WINAPI CleanResThreadProc(LPVOID lpParam)
{
	CManager *pThis = (CManager*)lpParam;
	if (pThis == NULL)
	{
		return NULL;
	}

	HANDLE hWait[] = {pThis->m_hCleanResEvent.m_hEvent, pThis->m_hExitThread.m_hEvent};
	BOOL bExit = FALSE;
	while(!bExit)
	{
		DWORD dwRet = WaitForMultipleObjects(sizeof(hWait)/sizeof(HANDLE), hWait, FALSE, INFINITE);
		switch(dwRet)
		{
		case WAIT_OBJECT_0:
			{
				pThis->DealCleanResource();
			}
			break;
		case WAIT_OBJECT_0+1:
			{
				bExit = TRUE;
			}
			break;
		default:
			break;
		}
	}

	return NULL;
}

/*
 * 摘要：处理断线信息回调
 */
void CManager::DealDisconnCallback()
{
	AFK_DISCONN_INFO *pstDisconnInfo = NULL;
	
	m_csDisconn.Lock();
	std::list<AFK_DISCONN_INFO*>::iterator it = m_lstDisconnDev.begin();
	if (it != m_lstDisconnDev.end())
	{
		pstDisconnInfo = (*it);
		m_lstDisconnDev.erase(it);
	}
	m_csDisconn.UnLock();
	
	if (pstDisconnInfo != NULL)
	{
		// 登出后不回调
		if (IsDeviceValid(pstDisconnInfo->device, 0) >= 0)
		{
			if (pstDisconnInfo->channel == NULL) // 主连接
			{
				if (!pstDisconnInfo->bOnline && m_pfDisConnect != NULL)
				{
					m_pfDisConnect((LONG)pstDisconnInfo->device, pstDisconnInfo->szIp, pstDisconnInfo->nPort, m_dwDisConnectUser);
				}
				else if (pstDisconnInfo->bOnline && m_pfReConnect != NULL)
				{
					m_pfReConnect((LONG)pstDisconnInfo->device, pstDisconnInfo->szIp, pstDisconnInfo->nPort, m_dwHaveReconnUser);
				}
			}
			else // 子连接
			{
				if (m_pfSubDisConnect != NULL)
				{
					m_pfSubDisConnect(pstDisconnInfo->emInterfaceType, pstDisconnInfo->bOnline, (LONG)pstDisconnInfo->channel, (LONG)pstDisconnInfo->device, m_dwSubDisconnUser);
				}
			}
		}
		
		delete pstDisconnInfo;
		SetEventEx(m_hDisEvent);
	}
}

/*
 * 摘要：处理报警数据回调
 */
void CManager::DealAlarmDataCallback()
{
	AFK_ALARM_DATA *pstAlarmData = NULL;
	
	m_csAlarmData.Lock();
	std::list<AFK_ALARM_DATA*>::iterator it = m_lstAlarmData.begin();
	if (it != m_lstAlarmData.end())
	{
		pstAlarmData = (*it);
		m_lstAlarmData.erase(it);
	}
	m_csAlarmData.UnLock();
	
	if (pstAlarmData != NULL)
	{
		if (pstAlarmData->device != NULL && m_pfMessCallBack != NULL)
		{
			m_pfMessCallBack(pstAlarmData->dwAlarmType, (LONG)pstAlarmData->device, 
				(char*)pstAlarmData->data, pstAlarmData->datalen, pstAlarmData->szIp, 
				pstAlarmData->nPort, m_dwMessUser);
		}
		
		DelAlarmData(pstAlarmData);
		
		SetEventEx(m_hAlarmDataEvent);
	}
}

/*
 * 摘要：处理主动注册连接回调
 */
void CManager::DealAutoRegiCallback()
{
	AFK_CLIENT_REGISTER_INFO *pstCltRegInfo = NULL;
	
	m_csCltRegInfo.Lock();
	std::list<AFK_CLIENT_REGISTER_INFO*>::iterator it = m_lstCltRegInfo.begin();
	if (it != m_lstCltRegInfo.end())
	{
		pstCltRegInfo = (*it);
		m_lstCltRegInfo.erase(it);
	}
	m_csCltRegInfo.UnLock();
	
	if (pstCltRegInfo != NULL)
	{
		fServiceCallBack cbListen = NULL;
		DWORD dwUser = 0;
		m_csListen.Lock();
		
		std::list<AFK_LISTER_DATA*>::iterator item = find_if(m_lstListenHandle.begin(), m_lstListenHandle.end(), CManager::SearchLSIbyHandle((LONG)pstCltRegInfo->caller));
		if (item != m_lstListenHandle.end() && NULL != (*item))
		{
			std::list<AFK_CLIENT_NODE*>::iterator itClt = find_if((*item)->lstClients.begin(), (*item)->lstClients.end(), CManager::SearchLCIbyIpPort(pstCltRegInfo->ip, pstCltRegInfo->port));
			if (itClt != (*item)->lstClients.end() && NULL != (*itClt))
			{
				(*itClt)->status = 1; //标志为已上传序列号
				strcpy((char*)(*itClt)->serial, pstCltRegInfo->serial);
				
				cbListen = (*item)->pCb;
				dwUser = (*item)->lsnUser;
			}
		}
		
		m_csListen.UnLock();
		
		if (cbListen != NULL)
		{
			char szDevSerial[DEV_SERIAL_NUM_LEN] = {0};
			Change_Utf8_Assic((unsigned char*)pstCltRegInfo->serial, szDevSerial);
			szDevSerial[DEV_SERIAL_NUM_LEN-1] = '\0';
			
			cbListen((LONG)pstCltRegInfo->caller, pstCltRegInfo->ip, pstCltRegInfo->port, DVR_SERIAL_RETURN_CB, szDevSerial, DEV_SERIAL_NUM_LEN, dwUser);
		}
		
		delete pstCltRegInfo;
		
		SetEventEx(m_hAutoRegiEvent);
	}
}

/*
 * 摘要：处理主动注册断线回调
 */
void CManager::DealAutoRegiDisCallback()
{
	AFK_CLIENT_DISCONN_INFO *pstCltDisconn = NULL;
	
	m_csCltDisconn.Lock();
	std::list<AFK_CLIENT_DISCONN_INFO*>::iterator it = m_lstCltDisconn.begin();
	if (it != m_lstCltDisconn.end())
	{
		pstCltDisconn = (*it);
		m_lstCltDisconn.erase(it);
	}
	m_csCltDisconn.UnLock();
	
	if (pstCltDisconn != NULL)
	{
		fServiceCallBack cbListen = NULL;
		DWORD dwUser = 0;
		BOOL bCltExit = FALSE;
		m_csListen.Lock();
		
		std::list<AFK_LISTER_DATA*>::iterator item = find_if(m_lstListenHandle.begin(), m_lstListenHandle.end(), CManager::SearchLSIbyHandle((LONG)pstCltDisconn->caller));
		if (item != m_lstListenHandle.end() && NULL != (*item))
		{
			list<AFK_CLIENT_NODE*>::iterator itClt = find_if((*item)->lstClients.begin(), (*item)->lstClients.end(), CManager::SearchLCIbyIpPort(pstCltDisconn->ip, pstCltDisconn->port));
			if (itClt != (*item)->lstClients.end() && NULL != (*itClt))
			{
				if (1 == (*itClt)->status)
				{
					cbListen = (*item)->pCb;
					dwUser = (*item)->lsnUser;
				}
				
				bCltExit = TRUE;
				delete (*itClt);
				(*item)->lstClients.erase(itClt);
			}
		}
		
		m_csListen.UnLock();
		
		if (bCltExit) 
		{
			m_pDeviceProb->close_client(pstCltDisconn->clnHandle);
		}
		
		if (cbListen != NULL)
		{
			cbListen((LONG)pstCltDisconn->caller, pstCltDisconn->ip, pstCltDisconn->port, DVR_DISCONNECT_CB, pstCltDisconn->clnHandle, 0, dwUser);
		}
		
		delete pstCltDisconn;
		
		SetEventEx(m_hAutoRegiDisEvent);
	}
}

/*
 * 摘要：处理异步信息回调
 */
void CManager::DealResponseDataCallback()
{
	AFK_RESPONSE_DATA *pResponseData = NULL;
	
	m_csResponse.Lock();
	std::list<AFK_RESPONSE_DATA*>::iterator it = m_lstResponseData.begin();
	if (it != m_lstResponseData.end())
	{
		pResponseData = (*it);
		m_lstResponseData.erase(it);
	}	
	m_csResponse.UnLock();
	
	if (pResponseData != NULL)
	{
		if (pResponseData->cbMessFunc != NULL)
		{
			pResponseData->cbMessFunc(pResponseData->lCommand, pResponseData->lpCallBackData, pResponseData->dwUserdata);
		}
		
		if (pResponseData->lpCallBackData != NULL)
		{
			delete pResponseData->lpCallBackData;
		}
		delete pResponseData;
		
		SetEventEx(m_hResponseEvent);
	}
}

/*
 * 摘要：处理NVD进度回调
 */
void CManager::DealNVDPosCallback()
{
	AFK_NVD_PLAYBACK *pPlaybackPosInfo = NULL;
	
	m_csNVDPlayback.Lock();
	std::list<AFK_NVD_PLAYBACK*>::iterator it = m_lstNVDPlayback.begin();
	if (it != m_lstNVDPlayback.end())
	{
		pPlaybackPosInfo = (*it);
		m_lstNVDPlayback.erase(it);
	}
	m_csNVDPlayback.UnLock();
	
	if (pPlaybackPosInfo != NULL)
	{
		if (pPlaybackPosInfo->cbPlayBackPos != NULL)
		{
			pPlaybackPosInfo->cbPlayBackPos((LONG)pPlaybackPosInfo->device, pPlaybackPosInfo->nEncoderNum, pPlaybackPosInfo->dwTotalSize, pPlaybackPosInfo->dwCurSize, pPlaybackPosInfo->dwUser);
		}
		
		delete pPlaybackPosInfo;
		
		SetEventEx(m_hNVDPlaybackEvent);
	}
}

/*
 * 摘要：删除报警数据
 */
void CManager::DelAlarmData(AFK_ALARM_DATA *pstAlarmData)
{
	if (pstAlarmData == NULL)
	{
		return;
	}
	
	switch(pstAlarmData->dwAlarmType)
	{
	case COMM_ALARM:
		{
			NET_CLIENT_STATE* pcs = (NET_CLIENT_STATE*)pstAlarmData->data;
			if (pcs)
			{
				delete pcs;
			}
			break;
		}
	case SHELTER_ALARM:
		{
			BYTE *pby = (BYTE*)pstAlarmData->data;
			if (pby)
			{
				delete[] pby;
			}
			break;
		}
	case DISK_FULL_ALARM:
		{
			DWORD* pdw = (DWORD*)pstAlarmData->data;
			if (pdw)
			{
				delete pdw;
			}
			break;
		}
	case DISK_ERROR_ALARM:
		{
			DWORD* pdw = (DWORD*)pstAlarmData->data;
			if (pdw)
			{
				delete pdw;
			}
			break;
		}
	case SOUND_DETECT_ALARM:
		{
			BYTE *pby = (BYTE *)pstAlarmData->data;
			if (pby)
			{
				delete[] pby;
			}
			break;
		}
	case ALARMDECODER_ALARM:
		{
			BYTE *pby = (BYTE *)pstAlarmData->data;
			if (pby)
			{
				delete[] pby;
			}
		}
		break;
	case POS_TICKET_ALARM:
		{
			BYTE *pby = (BYTE *)pstAlarmData->data;
			if (pby)
			{
				delete[] pby;
			}
			break;
		}
	case POS_TICKET_RAWDATA:
		{
			BYTE *pby = (BYTE *)pstAlarmData->data;
			if (pby)
			{
				delete[] pby;
			}
			break;
		}

		//以下是新协议报警
	case OUT_ALARM_EX:
	case MOTION_ALARM_EX:
	case VIDEOLOST_ALARM_EX:
	case SHELTER_ALARM_EX:
	case SOUND_DETECT_ALARM_EX:
	case DISKFULL_ALARM_EX:
	case DISKERROR_ALARM_EX:
	case ENCODER_ALARM_EX:
	case URGENCY_ALARM_EX:
	case WIRELESS_ALARM_EX:
	case REBOOT_EVENT_EX:
	case AUTO_TALK_START_EX:
	case AUTO_TALK_STOP_EX:
	case EVENT_CONFIG_CHANGE:
	case PANORAMA_SWITCH_ALARM_EX:
	case LOSTFOCUS_ALARM_EX:
	case DEVICE_REBOOT_EX:
	case CONFIG_CHANGED_EX:
		{
			BYTE *pby = (BYTE *)pstAlarmData->data;
			if (pby)
			{
				delete[] pby;
			}
		}
		break;
	case NEW_SOUND_DETECT_ALARM_EX:
		{
			NEW_SOUND_ALARM_STATE *pSoundAlarm = (NEW_SOUND_ALARM_STATE*)pstAlarmData->data;
			if (pSoundAlarm != NULL) 
			{
				delete pSoundAlarm;
			}
		}
		break;
	case CONFIG_RESULT_EVENT_EX:
		{
			DEV_SET_RESULT *pSetRet=(DEV_SET_RESULT *)pstAlarmData->data;
			if(pSetRet != NULL)
			{
				delete pSetRet;
			}
		}
		break;
	case ALARM_DECODER_ALARM_EX:
		{
			ALARM_DECODER_ALARM *pAlarmDecoderAlarm = (ALARM_DECODER_ALARM *)pstAlarmData->data;
			if(pAlarmDecoderAlarm != NULL)
			{
				delete pAlarmDecoderAlarm;
			}
		}
		break;
	case DECODER_DECODE_ABILITY:
		{
			BYTE *pby = (BYTE *)pstAlarmData->data;
			if (pby)
			{
				delete pby;
			}
		}
		break;
	case FDDI_DECODER_ABILITY:
		{
			ALARM_FDDI_ALARM * pAlarm = (ALARM_FDDI_ALARM *)pstAlarmData->data;
			if(NULL != pAlarm)
			{
				delete pAlarm;
			}
		}
		break;
	case OEMSTATE_EX:
	case ATMPOS_BROKEN_EX:
		{	
			BYTE *bData = (BYTE *)pstAlarmData->data;
			if(NULL != bData)
			{
				delete bData;
			}
		}
		break;
	case DSP_ALARM_EX:
		{
			DSP_ALARM * pAlarm = (DSP_ALARM *)pstAlarmData->data;
			if(NULL != pAlarm)
			{
				delete pAlarm;
			}
		}
		break;
	case RECORD_CHANGED_EX:
		{
			ALARM_RECORDING_CHANGED *pAlarmInfo = (ALARM_RECORDING_CHANGED*)pstAlarmData->data;
			if (pAlarmInfo != NULL)
			{
				delete[] pAlarmInfo;
			}
		}
		break;
		
	case POS_TICKET_ALARM_EX:
		{
			BYTE *pby = (BYTE *)pstAlarmData->data;
			if (pby)
			{
				delete[] pby;
			}
		}
		break;
	case POS_TICKET_RADATA_EX:
		{
			BYTE *pby = (BYTE *)pstAlarmData->data;
			if (pby)
			{
				delete[] pby;
			}
		}
		break;
	default:
		break;
	}
	
	delete pstAlarmData;
}

int	CManager::ClearAlarm(afk_device_s* device)
{
	if (device == NULL)
	{
		return -1;
	}

	std::list<AFK_ALARM_DATA*>::iterator it = m_lstAlarmData.begin();
	while(it != m_lstAlarmData.end())
	{
		if (*it && (*it)->device)
		{
			if ((*it)->device == device)
			{
				DelAlarmData((*it));

				m_lstAlarmData.erase(it++);
				continue;
			}
		}
		it++;
	}
	m_lstAlarmData.clear();

	return 0;
}

/*
 * 摘要：发送心跳包和断线检测
 */
void CManager::AllSendHeartBeat()
{
	afk_device_s *device = NULL;

	m_csDevices.Lock();
	std::list<afk_device_s*>::iterator it = m_lstDevices.begin();
	while (it != m_lstDevices.end() && (*it))
	{
		device = (*it);
		if (device != NULL)
		{
			device->set_info(device, dit_heart_beat, NULL);
		}
		
		it++;
	}
	m_csDevices.UnLock();
}

/*
 * 摘要：执行断线重连任务
 */
void CManager::AllDevExecuteTask()
{
	afk_device_s *device = NULL;

	m_csDevices.Lock();
	std::list<afk_device_s*>::iterator it = m_lstDevices.begin();
	while (it != m_lstDevices.end() && (*it))
	{
		device = (*it);
		if (device != NULL)
		{
			int nIsHaveTask = 0;
			device->get_info(device, dit_device_task, &nIsHaveTask);
			if (nIsHaveTask == 1)
			{
				device->set_info(device, dit_execute_task, NULL);
			}
		}
		
		it++;
	}
	m_csDevices.UnLock();
}

/*
 * 摘要：异步接口超时检测
 */
void CManager::AllAsynTimeoutDetect()
{
	afk_device_s *device = NULL;

	m_csDevices.Lock();
	std::list<afk_device_s*>::iterator it = m_lstDevices.begin();
	while (it != m_lstDevices.end() && (*it))
	{
		device = (*it);
		if (device != NULL)
		{
			device->set_info(device, dit_asyn_timeout, NULL);
		}
		
		it++;
	}
	m_csDevices.UnLock();
}

/*
 * 摘要：清理断线连接资源
 */
void CManager::DealCleanResource()
{
	afk_device_s* device = NULL;

	m_csCleanRes.Lock();

	std::list<afk_device_s*>::iterator it = m_lstCleanRes.begin();
	if (it != m_lstCleanRes.end())
	{
		device = (*it);
		m_lstCleanRes.erase(it);
	}

	m_csCleanRes.UnLock();
	
	if (device != NULL)
	{
		m_csDevices.Lock();
		
		std::list<afk_device_s*>::iterator it = find(m_lstDevices.begin(),m_lstDevices.end(),device);
		if (it != m_lstDevices.end())
		{
			device->set_info(device, dit_cleanres_flag, NULL);
		}
		
		m_csDevices.UnLock();
		
		SetEventEx(m_hCleanResEvent);
	}
}

//////////////////////////////////////////////////////////////////////////

void __stdcall onDisConnectFunc(void *device, void *channel, int nOnline, char *szIp, int nPort, int nInterfaceType, void *userdata)
{
	CManager *pThis = (CManager*)userdata;
    if (pThis != NULL)
    {
		pThis->DeviceDisConnect((afk_device_s*)device, (afk_channel_s*)channel, nOnline, szIp, nPort, nInterfaceType);
    }
}

/*
 * 摘要：设备主动断开内部回调函数
 */
void CManager::DeviceDisConnect(afk_device_s* device, afk_channel_s* channel, int nOnline, char *szIp, int nPort, int nInterfaceType)
{
	if (device != NULL && szIp != NULL)
	{
		AFK_DISCONN_INFO *pstDisconnInfo = new AFK_DISCONN_INFO;
		if (pstDisconnInfo != NULL)
		{
			strcpy(pstDisconnInfo->szIp, szIp);
			pstDisconnInfo->nPort = nPort;
			pstDisconnInfo->device = device;
			pstDisconnInfo->channel = channel;
			pstDisconnInfo->bOnline = nOnline;
			pstDisconnInfo->emInterfaceType = (EM_INTERFACE_TYPE)nInterfaceType;

			m_csDisconn.Lock();
			m_lstDisconnDev.push_back(pstDisconnInfo);
			m_csDisconn.UnLock();

			SetEventEx(m_hDisEvent);
		}
	}
}

BOOL ConvertConfigType(WORD wInType, DWORD &dwOutType);

void __stdcall onDeviceEventFunc(void *device, int nEventType, void *param, void *userdata)
{
	CManager *pThis = (CManager*)userdata;
    if (pThis != NULL)
    {
		pThis->DeviceEvent((afk_device_s*)device, nEventType, param);
    }
}

/*
 * 摘要：设备事件内部回调函数
 */
void CManager::DeviceEvent(afk_device_s* device, int nEventType, void *param)
{
	if (device != NULL)
	{
		switch(nEventType)
		{
		case EVENT_CONFIG_REBOOT:
			{
				AFK_ALARM_DATA *pstAlarmData = new AFK_ALARM_DATA;
				if (pstAlarmData != NULL)
				{
					memset(pstAlarmData, 0, sizeof(AFK_ALARM_DATA));
					pstAlarmData->device = device;
					strcpy(pstAlarmData->szIp, device->device_ip(device));
					pstAlarmData->nPort = device->device_port(device);
					pstAlarmData->dwAlarmType = REBOOT_EVENT_EX;
					pstAlarmData->data = NULL;
					pstAlarmData->datalen = 0;
					
					m_csAlarmData.Lock();
					m_lstAlarmData.push_back(pstAlarmData);
					m_csAlarmData.UnLock();

					SetEventEx(m_hAlarmDataEvent);
				}
			}
			break;
		case EVENT_CONFIG_ACK:
			{
				CONFIG_ACK_INFO *pstCfgAckInfo = (CONFIG_ACK_INFO *)param;
				if (pstCfgAckInfo != NULL)
				{
					AFK_ALARM_DATA *pstAlarmData = new AFK_ALARM_DATA;
					if (pstAlarmData != NULL)
					{
						DEV_SET_RESULT *pData = new DEV_SET_RESULT;
						if (pData != NULL)
						{
							memset(pstAlarmData, 0, sizeof(AFK_ALARM_DATA));
							pstAlarmData->device = device;
							strcpy(pstAlarmData->szIp, device->device_ip(device));
							pstAlarmData->nPort = device->device_port(device);
							pstAlarmData->dwAlarmType = CONFIG_RESULT_EVENT_EX;

							memset(pData , 0, sizeof(DEV_SET_RESULT));
							ConvertConfigType(pstCfgAckInfo->type, pData->dwType);
							pData->wResultCode = pstCfgAckInfo->nResultCode;
							pData->wRebootSign = pstCfgAckInfo->bReboot;
							pstAlarmData->data = (void *)pData;
							pstAlarmData->datalen = sizeof(DEV_SET_RESULT);

							m_csAlarmData.Lock();
							m_lstAlarmData.push_back(pstAlarmData);
							m_csAlarmData.UnLock();

							SetEventEx(m_hAlarmDataEvent);
						}
						else
						{
							delete pstAlarmData;
						}
					}
				}
			}
			break;
		case EVENT_TALK_START:
			{
				AFK_ALARM_DATA *pstAlarmData = new AFK_ALARM_DATA;
				if (pstAlarmData != NULL)
				{
					memset(pstAlarmData, 0, sizeof(AFK_ALARM_DATA));
					pstAlarmData->device = device;
					strcpy(pstAlarmData->szIp, device->device_ip(device));
					pstAlarmData->nPort = device->device_port(device);
					pstAlarmData->dwAlarmType = AUTO_TALK_START_EX;
					pstAlarmData->data = NULL;
					pstAlarmData->datalen = 0;
					
					m_csAlarmData.Lock();
					m_lstAlarmData.push_back(pstAlarmData);
					m_csAlarmData.UnLock();

					SetEventEx(m_hAlarmDataEvent);
				}
			}
			break;
		case EVENT_TALK_STOP:
			{
				AFK_ALARM_DATA *pstAlarmData = new AFK_ALARM_DATA;
				if (pstAlarmData != NULL)
				{
					memset(pstAlarmData, 0, sizeof(AFK_ALARM_DATA));
					pstAlarmData->device = device;
					strcpy(pstAlarmData->szIp, device->device_ip(device));
					pstAlarmData->nPort = device->device_port(device);
					pstAlarmData->dwAlarmType = AUTO_TALK_STOP_EX;
					pstAlarmData->data = NULL;
					pstAlarmData->datalen = 0;
					
					m_csAlarmData.Lock();
					m_lstAlarmData.push_back(pstAlarmData);
					m_csAlarmData.UnLock();

					SetEventEx(m_hAlarmDataEvent);
				}
			}
			break;
		case EVENT_CONFIG_CHANGE:
			{
				AFK_ALARM_DATA *pstAlarmData = new AFK_ALARM_DATA;
				memset(pstAlarmData, 0, sizeof(AFK_ALARM_DATA));
				pstAlarmData->device = device;
				strcpy(pstAlarmData->szIp, device->device_ip(device));
				pstAlarmData->nPort = device->device_port(device);
				pstAlarmData->dwAlarmType = CONFIG_CHANGE_EX;
				pstAlarmData->data = NULL;
				pstAlarmData->datalen = 0;
				
				m_csAlarmData.Lock();
				m_lstAlarmData.push_back(pstAlarmData);
				m_csAlarmData.UnLock();

				SetEventEx(m_hAlarmDataEvent);
			}
			break;
		case EVENT_NVD_PLAYBACK:
			{
				NVD_PLAYBACK_EVENT *pstAckInfo = (NVD_PLAYBACK_EVENT *)param;
				if (pstAckInfo != NULL)
				{
					AFK_NVD_PLAYBACK *pstPlaybackPos = new AFK_NVD_PLAYBACK;
					if (pstPlaybackPos != NULL)
					{
						memset(pstPlaybackPos, 0, sizeof(AFK_NVD_PLAYBACK));
						pstPlaybackPos->device = device;
						pstPlaybackPos->nEncoderNum = pstAckInfo->nEncoderNum;
						pstPlaybackPos->dwTotalSize = pstAckInfo->nTotalSize;
						pstPlaybackPos->dwCurSize = pstAckInfo->nCurSize;

						DEVICE_MESS_CALLBACK stuCallFunc = {0};
						device->get_info(device, dit_nvd_playback_flag, &stuCallFunc);
						pstPlaybackPos->cbPlayBackPos = (fDecPlayBackPosCallBack)stuCallFunc.fun;
						pstPlaybackPos->dwUser = (DWORD)stuCallFunc.data;

						m_csNVDPlayback.Lock();
						m_lstNVDPlayback.push_back(pstPlaybackPos);
						m_csNVDPlayback.UnLock();

						SetEventEx(m_hNVDPlaybackEvent);
					}
				}
			}
			break;
		case EVENT_DEVICE_DEAL:
			{
				afk_device_s* device = (afk_device_s*)param;
				if (device != NULL)
				{
					m_csCleanRes.Lock();
					m_lstCleanRes.push_back(device);
					m_csCleanRes.UnLock();

					SetEventEx(m_hCleanResEvent);
				}
			}
			break;
		default:
		    break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

/*
 * 摘要：初始化环境
 */
int	CManager::Init(fDisConnect cbDisConnect, DWORD dwUser)
{
	// 设置定时器精度，有利于提高读配置信息的速度
#ifdef WIN32
	timeBeginPeriod(1);
#endif

	if (InterlockedIncrementEx(&m_nLifeCount) > 1)
	{
		return 0;
	}

	int nLastError = NET_SDK_INIT_ERROR;
	int nRet = -1;
	DWORD dwThreadID = 0;

	m_dwErrorCode = 0;
	m_pfDisConnect		= cbDisConnect;
	m_dwDisConnectUser	= dwUser;

	m_pDeviceProb = (afk_device_prob_t*)plugin_info.init(0);
	if (m_pDeviceProb == NULL)
	{
		goto e_clearup;
	}

	if (!m_pDeviceProb->net_startup())
	{
		goto e_clearup;
	}

	nRet = m_pRealPlayMdl->Init();
	if (nRet < 0)
	{
		goto e_clearup;
	}

	nRet = m_pPlayBackMdl->Init();
	if (nRet < 0)
	{
		goto e_clearup;
	}

	if (!m_pRenderManager->Init())
	{
		goto e_clearup;	
	}

	nRet = m_pAlarmMdl->Init();
	if (nRet < 0)
	{
		goto e_clearup;
	}

	nRet = m_pTalkMdl->Init();
	if (nRet < 0)
	{
		goto e_clearup;
	}

	nRet = m_pConfigMdl->Init();
	if (nRet < 0)
	{
		goto e_clearup;
	}

	nRet = m_pConfigExMdl->Init();
	if (nRet < 0)
	{
		goto e_clearup;
	}

	nRet = m_pControlMdl->Init();
	if (nRet < 0)
	{
		goto e_clearup;
	}

	if(!m_pAlarmServerMdl->Init())
	{
		goto e_clearup;
	}

	nRet = m_pNVDMdle->Init();
	if (nRet < 0)
	{
		goto e_clearup;
	}

	nRet = m_pAutoRegiMdl->Init();
	if (nRet < 0)
	{
		goto e_clearup;
	}
	
	nRet = CreateThreadEx(m_hThread, 0, (LPTHREAD_START_ROUTINE)RoutineThreadProc, (void*)this, 0, &dwThreadID);
	if (nRet < 0)
	{
		goto e_clearup;
	}

	nRet = CreateThreadEx(m_hHeatBeatThread, 0, (LPTHREAD_START_ROUTINE)HeatBeatThreadProc, (void*)this, 0, &dwThreadID);
	if (nRet < 0)
	{
		goto e_clearup;
	}

	nRet = CreateThreadEx(m_hCleanResThread, 0, (LPTHREAD_START_ROUTINE)CleanResThreadProc, (void*)this, 0, &dwThreadID);
	if (nRet < 0)
	{
		goto e_clearup;
	}

	return 0;

e_clearup:
	Uninit(); //这里不太可能失败，为保存之前的失败原因，这里不对返回值做处理
	return nLastError;
}

/*
 * 摘要：释放资源
 */
int CManager::Uninit()
{
	if (InterlockedDecrementEx(&m_nLifeCount) > 0)
	{
		return 0;
	}

	int nRet = 0;

	// 登出
	{
		std::list<afk_device_s*>::iterator it = m_lstDevices.begin();
		/* 因为logout里要删除登陆句柄，所以才用这么怪的方法 */
		for(; it != m_lstDevices.end(); it = m_lstDevices.begin())
		{
			if (*it)
			{
				if (Logout_Dev((LONG)*it) < 0)
				{
					nRet = -1;
				}
			}
		}
	}

	// 主动注册停止服务
	{
		m_csListen.Lock();
		
		std::list<AFK_LISTER_DATA*>::iterator itLsn;
		std::list<AFK_CLIENT_NODE*>::iterator itCln;
		itLsn = m_lstListenHandle.begin();
		for (;itLsn != m_lstListenHandle.end();itLsn = m_lstListenHandle.begin())
		{
			if ((*itLsn))
			{
				if (!StopListenServer((LONG)(*itLsn)))
				{
					nRet = -1;
				}
			}
		}
		m_lstListenHandle.clear();
		
		m_csListen.UnLock();
	}

	// 关闭线程
	{
		SetEventEx(m_hExitThread);
		DWORD dwRet = WaitForSingleObjectEx(m_hThread,1000*10);
		if (dwRet != WAIT_OBJECT_0)
		{
			TerminateThreadEx(m_hThread, 0xFFFFFFFF);
		}
		
		SetEventEx(m_hExitThread);
		dwRet = WaitForSingleObjectEx(m_hHeatBeatThread, 1000*10);
		if (dwRet != WAIT_OBJECT_0)
		{
			TerminateThreadEx(m_hHeatBeatThread, 0xFFFFFFFF);
		}

		SetEventEx(m_hExitThread);
		dwRet = WaitForSingleObjectEx(m_hCleanResThread, 1000*10);
		if (dwRet != WAIT_OBJECT_0)
		{
			TerminateThreadEx(m_hCleanResThread, 0xFFFFFFFF);
		}
		
		CloseThreadEx(m_hThread);
		CloseThreadEx(m_hHeatBeatThread);
		CloseThreadEx(m_hCleanResThread);
		
		ResetEventEx(m_hExitThread);
	}

	// 退出各个模块
	{
		if (m_pRealPlayMdl->Uninit() < 0)
		{
			nRet = -1;
		}
		
		if (m_pPlayBackMdl->Uninit() < 0)
		{
			nRet = -1;
		}

		if (!m_pRenderManager->Uninit())
		{
			nRet = -1;
		}

		if (m_pAlarmMdl->Uninit() < 0)
		{
			nRet = -1;
		}

		if (m_pTalkMdl->UnInit() < 0)
		{
			nRet = -1;
		}
		
		if (m_pConfigMdl->Uninit() < 0)
		{
			nRet = -1;
		}
		
		if (m_pConfigExMdl->Uninit() < 0)
		{
			nRet = -1;
		}
		
		if (m_pControlMdl->Uninit() < 0)
		{
			nRet = -1;
		}
		
		if(m_pAlarmServerMdl->Uninit() < 0)
		{
			nRet = -1;
		}

		if (m_pNVDMdle->Uninit() < 0)
		{
			nRet = -1;
		}

		if (m_pAutoRegiMdl->Uninit() < 0)
		{
			nRet = -1;
		}
	}

	// 清理网络
	{
		if (m_pDeviceProb != NULL)
		{
			m_pDeviceProb->net_cleanup();
			m_pDeviceProb = NULL;
		}
	}

	{
		std::list<AFK_DISCONN_INFO *>::iterator it = m_lstDisconnDev.begin();
		while (it != m_lstDisconnDev.end())
		{
			if ((*it))
			{
				delete (*it);
			}
			it++;
		}
		m_lstDisconnDev.clear();
	}

	{
		std::list<AFK_RESPONSE_DATA *>::iterator it = m_lstResponseData.begin();
		while (it != m_lstResponseData.end())
		{
			AFK_RESPONSE_DATA *pstResponseData = (*it);
			if (pstResponseData != NULL)
			{
				if (pstResponseData->lpCallBackData != NULL)
				{
					delete pstResponseData->lpCallBackData;
				}
				delete pstResponseData;
			}
			it++;
		}
		m_lstResponseData.clear();
	}

	{
		std::list<AFK_NVD_PLAYBACK *>::iterator it = m_lstNVDPlayback.begin();
		while (it != m_lstNVDPlayback.end())
		{
			if ((*it))
			{
				delete (*it);
			}
			it++;
		}
		m_lstNVDPlayback.clear();
	}

	m_pfDisConnect		= NULL;
	m_pfReConnect		= NULL;
	m_dwDisConnectUser	= 0;
	m_dwHaveReconnUser	= 0;
	
	if (nRet < 0)
	{
		nRet = NET_SDK_UNINIT_ERROR;
	}

	return nRet;
}

/*
 * 摘要：登入设备
 */
LONG CManager::Login_Dev(char *szDevIp, int nPort, char *szUser, char *szPassword, LPNET_DEVICEINFO lpDeviceInfo, int *pErrorCode)
{
	if (m_pDeviceProb == NULL)
	{
		SetLastError(NET_NO_INIT);
		return 0;
	}

	if (szDevIp == NULL || szUser == NULL || szPassword == NULL || strlen(szUser)>64)
	{
		SetLastError(NET_ILLEGAL_PARAM);
		return 0;
	}

	int nErrorCode = 0;

	// 将用户名转换为UTF8格式
	char *pUTF8 = new char[64*2+2];
	if (NULL == pUTF8)
	{
		SetLastError(NET_SYSTEM_ERROR);
		return 0;
	}
	memset(pUTF8, 0, 64*2+2);
#ifdef NETSDK_VERSION_ENCRYPT
	strcpy(pUTF8, szUser);
#else
	Change_Assic_UTF8(szUser, strlen(szUser), pUTF8, strlen(szUser)*2+2);
#endif

	// 登入
    afk_device_s *device = (afk_device_s*)m_pDeviceProb->try_connect(szDevIp, nPort, pUTF8, szPassword, 0, NULL, 
							onDisConnectFunc, onDeviceEventFunc, this, &nErrorCode, m_nLoginTimeout, m_nConnTryNum, 
							m_pfReConnect!=NULL?TRUE:FALSE, m_nConnHostTime, m_nSubConnSpaceTime, m_nConnBufLen);

	delete[] pUTF8;

	// 获取设备基本信息
    if (device != NULL) 
    {
		m_csDevices.Lock();
		m_lstDevices.push_back(device);
		m_csDevices.UnLock();

		NET_DEVICEINFO stuDeviceInfo = {0};
		if (GetDeviceInfo(device, &stuDeviceInfo, m_nGetDevInfoTime) >= 0)
		{
			if (lpDeviceInfo != NULL) 
			{
				memcpy(lpDeviceInfo, &stuDeviceInfo, sizeof(NET_DEVICEINFO));
			}
		}
		
		// 如果在登入接口返回之前断线的话，增加断线回调。
		int nIsOnline = 1;
		device->get_info(device, dit_online_flag, &nIsOnline);
		if (0 == nIsOnline)
		{
			DeviceDisConnect(device, NULL, FALSE, szDevIp, nPort, 0);
		}
    }
	else
	{
		int nLastError = GetLoginError(nErrorCode);
		SetLastError(nLastError);
	}

	if (pErrorCode != NULL)
	{
		*pErrorCode = nErrorCode;
	}

    return (LONG)device;
}

/*
 * 摘要：登入设备
 */
LONG CManager::Login_DevEx(char *szDevIp, int nPort, char *szUser, char *szPassword, int nSpecCap, void* pCapParam, LPNET_DEVICEINFO lpDeviceInfo, int *pErrorCode)
{
	if (m_pDeviceProb == NULL)
	{
		SetLastError(NET_NO_INIT);
		return 0;
	}

	if (szDevIp == NULL || szUser == NULL || szPassword == NULL || strlen(szUser)>64)
	{
		SetLastError(NET_ILLEGAL_PARAM);
		return 0;
	}

	int nErrorCode = 0;
    afk_device_s *device = NULL;

	char *pUTF8 = new char[64*2+2];
	if (NULL == pUTF8)
	{
		SetLastError(NET_SYSTEM_ERROR);
		return 0;
	}
	memset(pUTF8, 0, 64*2+2);
#ifdef NETSDK_VERSION_ENCRYPT
	strcpy(pUTF8, szUser);
#else
	Change_Assic_UTF8(szUser, strlen(szUser), pUTF8, strlen(szUser)*2+2);
#endif

	void *pCapParam_2 = pCapParam;
	if (2 == nSpecCap)		// 连接已存在的登录
	{
		char *serial = (char*)pCapParam;
		if (NULL == serial)
		{
			SetLastError(NET_ILLEGAL_PARAM);
			delete[] pUTF8;
			return 0;
		}
		if (strlen(serial) > DEV_SERIAL_NUM_LEN)
		{
			SetLastError(NET_ILLEGAL_PARAM);
			delete[] pUTF8;
			return 0;
		}

		char szDevSerialUTF8[2*DEV_SERIAL_NUM_LEN+2] = {0};
		Change_Assic_UTF8(serial, strlen(serial), szDevSerialUTF8, 2*DEV_SERIAL_NUM_LEN+2);
		szDevSerialUTF8[2*DEV_SERIAL_NUM_LEN+1] = '\0';

		BOOL bExistClient = FALSE;

		m_csListen.Lock();

		list<AFK_LISTER_DATA*>::iterator itLsn;
		list<AFK_CLIENT_NODE*>::iterator itCln;
		itLsn = m_lstListenHandle.begin();
		
		while (itLsn != m_lstListenHandle.end() && (*itLsn))
		{
			itCln = (*itLsn)->lstClients.begin();
			while (itCln != (*itLsn)->lstClients.end() && (*itCln))
			{
				if (0 == (*itCln)->status) 
				{
					itCln++;
					continue;
				}
				if (_stricmp((*itCln)->serial, szDevSerialUTF8) == 0
					&& _stricmp((*itCln)->ip, szDevIp) == 0
					&& (*itCln)->port == nPort)
				{
					//found!
					bExistClient = TRUE;
					pCapParam_2 = ((*itCln)->clnHandle);
					break;
				}
				itCln++;
			}

			if (bExistClient)
			{			
				break;
			}

			itLsn++;
		}

		if (!bExistClient)
		{
			SetLastError(NET_LISTER_INCORRECT_SERIAL);
			m_csListen.UnLock();
			delete[] pUTF8;
			return 0;
		}
		
		device = (afk_device_s*)m_pDeviceProb->try_connect(szDevIp, nPort, pUTF8, szPassword, nSpecCap, pCapParam_2, 
						onDisConnectFunc, onDeviceEventFunc, (void*)this, &nErrorCode, m_nLoginTimeout, m_nConnTryNum, 
						m_pfReConnect!=NULL?TRUE:FALSE, m_nConnHostTime, m_nSubConnSpaceTime, m_nConnBufLen);

		delete[] pUTF8;

		if (device != NULL)
		{
			// 该连接交由登入管理，这里可删掉
			delete(*itCln);
			(*itLsn)->lstClients.erase(itCln);
		}
		else
		{
			delete(*itCln);
			(*itLsn)->lstClients.erase(itCln);
		}

		m_csListen.UnLock();
	}
	else
	{
		device = (afk_device_s*)m_pDeviceProb->try_connect(szDevIp, nPort, pUTF8, szPassword, nSpecCap, pCapParam_2, 
					onDisConnectFunc, onDeviceEventFunc, (void*)this, &nErrorCode, m_nLoginTimeout, m_nConnTryNum, 
					m_pfReConnect!=NULL?TRUE:FALSE, m_nConnHostTime, m_nSubConnSpaceTime, m_nConnBufLen);
		
		delete[] pUTF8;
	}

    if (device != NULL) 
    {
		m_csDevices.Lock();
		m_lstDevices.push_back(device);
		m_csDevices.UnLock();
		
		if (3 == nSpecCap)		// 组播方式
		{
			// 获取组播地址
			afk_login_device_type stuMultiInfo = {0};
			int ret = GetDevConfigEx().GetMulticastAddr(device, &stuMultiInfo);
			if (ret < 0)
			{
				device->close(device);
				m_csDevices.Lock();
				device->dev_decref(device);
				m_lstDevices.remove(device);
				m_csDevices.UnLock();
				return 0;
			}
			
			stuMultiInfo.nLoginMode = channel_connect_multicast;
			device->set_info(device, dit_login_device_flag, &stuMultiInfo);
		}
		else if (4 == nSpecCap)		// UDP方式
		{
			// 获取UDP地址
			afk_login_device_type stuUdpInfo = {0};
			int ret = GetDevConfigEx().GetUdpPort(device, &stuUdpInfo);
			if (ret < 0)
			{
				device->close(device);
				m_csDevices.Lock();
				device->dev_decref(device);
				m_lstDevices.remove(device);
				m_csDevices.UnLock();
				return 0;
			}
			
			stuUdpInfo.nLoginMode = channel_connect_udp;
			device->set_info(device, dit_login_device_flag, &stuUdpInfo);
		}

		// 尝试成功，查询系统配置信息
		if (lpDeviceInfo) 
		{
			GetDeviceInfo(device, lpDeviceInfo, m_nGetDevInfoTime);
		}

		// 如果在登入接口返回之前断线的话，增加断线回调。
		int nIsOnline = 1;
		device->get_info(device, dit_online_flag, &nIsOnline);
		if (0 == nIsOnline)
		{
			DeviceDisConnect(device, NULL, FALSE, szDevIp, nPort, 0);
		}
    }
	else
	{
		int nLastError = GetLoginError(nErrorCode);
		SetLastError(nLastError);
	}

	if (pErrorCode != NULL)
	{
		*pErrorCode = nErrorCode;
	}

    return (LONG)device;
}

/*
 * 摘要：查询设备基本信息
 */
int CManager::GetDeviceInfo(afk_device_s* device, NET_DEVICEINFO *pstDevInfo, int nWaitTime)
{
	if (device == NULL || pstDevInfo == NULL)
	{
		return -1;
	}
	
	// 获取设备信息
	SYSATTR_T stuDevSysInfo = {0};
	int nRetLen = 0;
	
	int nRet = GetDevConfig().QuerySystemInfo((LONG)device, SYSTEM_INFO_DEV_ATTR, (char*)&stuDevSysInfo, sizeof(SYSATTR_T), &nRetLen, nWaitTime);
	if (nRet >= 0)
	{
		pstDevInfo->byAlarmInPortNum = stuDevSysInfo.iAlarmInCaps;
		pstDevInfo->byAlarmOutPortNum = stuDevSysInfo.iAlarmOutCaps;
		int nEtherFlag = stuDevSysInfo.iIsMutiEthIf;
		int nAlarmInNum = stuDevSysInfo.iAlarmInCaps;
		device->set_info(device, dit_ether_flag, &nEtherFlag);
		device->set_info(device, dit_alarmin_num, &nAlarmInNum);
	}
	else
	{
		pstDevInfo->byAlarmInPortNum = device->alarminputcount(device);
		pstDevInfo->byAlarmOutPortNum = device->alarmoutputcount(device);
	}
	
	char szDevSearial[32];
	memset(szDevSearial, 0, 32);
	nRetLen = 0;

	nRet = GetDevConfig().QuerySystemInfo((LONG)device, SYSTEM_INFO_DEV_ID, szDevSearial, 32, &nRetLen, nWaitTime);
	if (nRet >= 0)
	{
		memcpy(pstDevInfo->sSerialNumber, szDevSearial, 32);
		pstDevInfo->sSerialNumber[31] = '\0';
	}
	else
	{
		pstDevInfo->sSerialNumber[0] = '\0';
	}

	// 获取硬盘状态信息
	nRetLen = 0;
	int nBufSize = sizeof(IDE_INFO64) + 256*sizeof(DRIVER_INFO);
	char *pDiskBuf = new char[nBufSize];
	if (NULL == pDiskBuf)
	{
		return NET_SYSTEM_ERROR;
	}
	
	nRet = GetDevConfig().QuerySystemInfo((LONG)device, SYSTEM_INFO_DISK_DRIVER, pDiskBuf, nBufSize, &nRetLen, 500);
	if (nRet >=0)
	{
		if (nRetLen >= sizeof(IDE_INFO) && nRetLen-sizeof(IDE_INFO)==((IDE_INFO *)pDiskBuf)->ide_num*sizeof(DRIVER_INFO))
		{
			IDE_INFO *pIDE_Info = (IDE_INFO *)pDiskBuf;

			if (pIDE_Info->ide_num <= 128)
			{
				pstDevInfo->byDiskNum = pIDE_Info->ide_num;
			}
		}
		else if (nRetLen >= sizeof(IDE_INFO64) && nRetLen-sizeof(IDE_INFO64)==((IDE_INFO64 *)pDiskBuf)->ide_num*sizeof(DRIVER_INFO))
		{
			IDE_INFO64 *pIDE_Info= (IDE_INFO64 *)pDiskBuf;

			if (pIDE_Info->ide_num <= 128)
			{
				pstDevInfo->byDiskNum = pIDE_Info->ide_num;
			}
		}
	}

	if (pDiskBuf != NULL)
	{
		delete[] pDiskBuf;
		pDiskBuf = NULL;
	}

	pstDevInfo->byDVRType = device->device_type(device);
	pstDevInfo->byChanNum = device->channelcount(device);

	return 0;
}

int CManager::GetLoginError(int nErrorCode)
{
	int nLastError = NET_LOGIN_ERROR_NETWORK;
	switch(nErrorCode) 
	{
	case 1: // 密码不正确
		nLastError = NET_LOGIN_ERROR_PASSWORD;
		break;
	case 2: //用户名不存在
		nLastError = NET_LOGIN_ERROR_USER;
		break;
	case 3: // 登录超时
		nLastError = NET_LOGIN_ERROR_TIMEOUT;
		break;
	case 4: // 重复登录
		nLastError = NET_LOGIN_ERROR_RELOGGIN;
		break;
	case 5: // 帐号被锁定
		nLastError = NET_LOGIN_ERROR_LOCKED;
		break;
	case 6: // 帐号被列入黑名单
		nLastError = NET_LOGIN_ERROR_BLACKLIST;
		break;
	case 7: // 系统忙，资源不足
		nLastError = NET_LOGIN_ERROR_BUSY;
		break;
	case 8: // 子连接失败
		nLastError = NET_LOGIN_ERROR_SUBCONNECT;
		break;
	case 9: // 主连接失败
		nLastError = NET_LOGIN_ERROR_CONNECT;
		break;
	default:
		nLastError = NET_LOGIN_ERROR_NETWORK;
		break;
	}

	return nLastError;
}

/*
 * 摘要：登出设备
 */
int CManager::Logout_Dev(LONG lLoginID)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
    if (IsDeviceValid(device) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	int nLastError = 0;

    if (device != NULL)
    {
		int nRet = DeleteDevice(device);
		if (nRet >= 0)
		{
			nRet = device->close(device);
			if (nRet != 0)
			{
				nLastError = NET_SDK_LOGOUT_ERROR;
			}
		
			m_csDevices.Lock();
			m_lstDevices.remove(device);
			device->dev_decref(device);
			m_csDevices.UnLock();
		}
		else
		{
			nLastError = NET_SDK_UNINIT_ERROR;
		}
    }

    return nLastError;	
}

/*
 * 摘要：删除设备
 */
int CManager::DeleteDevice(afk_device_s* device)
{
	int nLastError = 0;
	int nRet1 = m_pRealPlayMdl->CloseChannelOfDevice(device);
	int nRet2 = m_pPlayBackMdl->CloseChannelOfDevice(device);
	int nRet3 = m_pAlarmMdl->StopListen((LONG)device);
	int nRet4 = m_pTalkMdl->CloseChannelOfDevice(device);
	int nRet5 = m_pControlMdl->CloseChannelOfDevice(device);
	int nRet6 = m_pConfigMdl->CloseChannelOfDevice(device);
	int nRet7 = m_pConfigExMdl->CloseChannelOfDevice(device);

	m_csAlarmData.Lock();
	ClearAlarm(device);
	m_csAlarmData.UnLock();
	
	if (nRet1 < 0 || nRet2 < 0 /*|| nRet3 < 0*/ || nRet4 < 0 || nRet5 < 0 || nRet6 < 0 || nRet7 < 0)
	{
		nLastError = NET_SDK_UNINIT_ERROR;
	}
	
	return nLastError;
}

/*
 * 摘要：检查设备是否有效；specFlag标志：0表示没有特殊操作，1表示有特殊操作，目前是给设备资源增加生命计数
 */
int CManager::IsDeviceValid(afk_device_s* device, int specFlag)
{
	int nRet = -1;
	m_csDevices.Lock();

	std::list<afk_device_s*>::iterator it = find(m_lstDevices.begin(),m_lstDevices.end(),device);
	if (it != m_lstDevices.end())
	{
		if (1 == specFlag && device->dev_addref(device) <= 0)
		{
		}
		else
		{
			nRet = 0;
		}
	}
	else
	{
#ifdef _DEBUG
		OutputDebugString("错误:设备指针无效---->CManager::IsDeviceValid\n");
#endif
	}

	m_csDevices.UnLock();
	return nRet;
}

void CManager::EndDeviceUse(afk_device_s* device)
{
	device->dev_decref(device);
}

/*
 * 摘要：获取设备码流统计
 */
LONG CManager::GetStatiscFlux(LONG lLoginID, LONG lPlayHandle)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
    if (IsDeviceValid(device) < 0)
    {
        return NET_INVALID_HANDLE;
    }
    
	LONG lRet = NET_ERROR;
	
    if (lPlayHandle == 0 && device)
    {    
		afk_channel_s *stat_channel = (afk_channel_s*)device->open_channel(device, AFK_CHANNEL_TYPE_STATISC, 0);
		if (stat_channel) 
		{
			LONG statFlux = stat_channel->get_info(stat_channel, 0, 0);
			BOOL bSuccess = stat_channel->close(stat_channel);
			if (!bSuccess)
			{
				lRet = NET_CLOSE_CHANNEL_ERROR;
			}
			else
			{
				return statFlux >= 0 ? statFlux : NET_ERROR;
			}
		}
		else
		{
			lRet = NET_OPEN_CHANNEL_ERROR;
		}
    }
    else
    {
		lRet = m_pRealPlayMdl->GetStatiscFlux(lLoginID,lPlayHandle);
		if (lRet < 0)
		{
			lRet = m_pPlayBackMdl->GetStatiscFlux(lLoginID,lPlayHandle);
		}
	}

    return lRet;
}

/*该函数只能被用户线程调用*/
DWORD CManager::GetLastError(void)
{
	m_csError.Lock();
	
	DWORD dwRet = NET_NOERROR;
#ifdef WIN32
	DWORD dwID = GetCurrentThreadId();
#else	//linux
	pthread_t dwID = pthread_self();
#endif

	std::list<AFK_LAST_ERROR*>::iterator it = find_if(m_lstLastError.begin(),m_lstLastError.end(),SearchLEbyThreadID(dwID));
	if (it != m_lstLastError.end() && (*it))
	{
		dwRet = (*it)->dwError;
	}
	
	m_csError.UnLock();
	return dwRet; 
}

/*该函数只能被用户线程调用*/
int CManager::SetLastError(DWORD dwError)
{
	m_csError.Lock();
	
	int nRet = -1;

#ifdef WIN32
	DWORD dwID = GetCurrentThreadId();
#else	//linux
	pthread_t dwID = pthread_self();
#endif

	std::list<AFK_LAST_ERROR*>::iterator it = find_if(m_lstLastError.begin(),m_lstLastError.end(),SearchLEbyThreadID(dwID));
	if (it != m_lstLastError.end() && (*it))
	{
		(*it)->dwError = dwError;
		nRet = 0;
	}
	else
	{
		AFK_LAST_ERROR *pLe = new AFK_LAST_ERROR;
		if (pLe)
		{
			pLe->dwError = dwError;
			pLe->dwThreadID = dwID;
			m_lstLastError.push_back(pLe);
			nRet = 0;
		}
		else
		{
			nRet = -1;
		}
	}
	
	m_csError.UnLock();
	return nRet;
}

//////////////////////////////////////////////////////////////////////////

int __stdcall GetDVRIPFunc(afk_handle_t object, unsigned char *data, unsigned int datalen, void *param, void *udata)
{
	receivedata_s *receivedata = (receivedata_s*)udata;
	if (!receivedata || false == receivedata->addRef())
	{
		return -1;
	}
	
	AFK_DDNS_DOMAIN *struDM = 0;
	if (data && (int)datalen >= sizeof(struDM))
	{
		struDM = (AFK_DDNS_DOMAIN *)data;
		if (!struDM)
		{
			SetEventEx(receivedata->hRecEvt);
			return -1;
		}
		memcpy(receivedata->data, (char *)struDM->dvr_Ip, sizeof(struDM->dvr_Ip));
		receivedata->result = 0;
		SetEventEx(receivedata->hRecEvt);
		return 1;
	}
	
	SetEventEx(receivedata->hRecEvt);
	return -1;
}

BOOL CManager::GetDVRIPByResolveSvr(char *szDevIp, int nPort, unsigned char *szDVRName, int nDVRNameLen, unsigned char *szDVRSerialNumber, int nDVRSerialLen, char *szGetIp)
{
	if (m_pDeviceProb == NULL)
	{
		SetLastError(NET_NETWORK_ERROR);
		return FALSE;
	}

	// 目前服务器端只实现了按名字查询
	if ((szDVRName == NULL /*&& !sDVRSerialNumber*/) || szGetIp == NULL)
	{
		SetLastError(NET_ILLEGAL_PARAM);
		return FALSE;
	}
	
	BOOL bRet = FALSE;
	int nError = 0;
	
    afk_device_s *server_DDNS = (afk_device_s*)m_pDeviceProb->try_connect(szDevIp, nPort, 0, 0, 0, 0, 
									onDisConnectFunc, onDeviceEventFunc, (void*)this, &nError, m_nLoginTimeout, 
									m_nConnTryNum, false, m_nConnHostTime, m_nSubConnSpaceTime, m_nConnBufLen);

    if (server_DDNS != NULL) 
    {
		Sleep(50); // 发太快的话，ddns服务器来不及响应

		afk_search_channel_param_s DDNSchannel = {0};
		DDNSchannel.type = AFK_CHANNEL_SEARCH_DDNSIP_BYNAME; //通过名称
		DDNSchannel.base.func = GetDVRIPFunc;
		
		int nBufLen = 0;
		receivedata_s receivedata;
		receivedata.data = szGetIp; // 用于存放IP
		receivedata.datalen = &nBufLen;
		receivedata.maxlen = 32;
		receivedata.result = -1;
		
		DDNSchannel.base.udata = &receivedata;
		DDNSchannel.webName = (char*)szDVRName;
		DDNSchannel.webNamelen = nDVRNameLen;
		
		afk_channel_s *pchannel = (afk_channel_s*)server_DDNS->open_channel(server_DDNS, AFK_CHANNEL_TYPE_SEARCH, &DDNSchannel);
		if (pchannel)
		{
			DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, m_nLoginTimeout);
			BOOL bSuccess = pchannel->close(pchannel);
			if (!bSuccess)
			{
				SetLastError(NET_CLOSE_CHANNEL_ERROR);
			}
			
			if (dwRet == WAIT_OBJECT_0)
			{    
				if (receivedata.result == -1)
				{
					SetLastError(NET_RETURN_DATA_ERROR);
				}
				else
				{
					bRet = TRUE;
				}
			}
			else
			{
				SetLastError(NET_NETWORK_ERROR);
			}
		}
		else
		{
			SetLastError(NET_OPEN_CHANNEL_ERROR);
		}
		
		// 在删除TCP连接类之前，必须停止CTcpSocket类内的一切回调。
		server_DDNS->close(server_DDNS);
		int nRet = server_DDNS->dev_decref(server_DDNS);
		if (nRet < 0)
		{
			SetLastError(NET_NETWORK_ERROR);
		}		
	}
	else
	{
		SetLastError(NET_NETWORK_ERROR);
	}
	
	return bRet;
}

//////////////////////////////////////////////////////////////////////////

void __stdcall onListenServerFunc(void* pServerHandle, char *szClientIp, int nClientPort, int nCommand, void *pParam, void *userdata)
{
	CManager *pThis = (CManager*)userdata;
	if (NULL != pThis)
	{
		pThis->DealListenServer(pServerHandle, szClientIp, nClientPort, nCommand, pParam);
	}
}

/*
 * 摘要：主动注册侦听处理
 */
void CManager::DealListenServer(void* pServerHandle, char *szClientIp, int nClientPort, int nCommand, void *pParam)
{
	if (NULL == m_pDeviceProb || NULL == pServerHandle || NULL == szClientIp)
	{
		return;
	}

	/* "设备连接上来回调"和"连接断线回调"统一放到一个线程去处理，防止出现死锁问题 */
	switch(nCommand)
	{
	case 0:	// 新的设备连接上来了，由侦听线程过来
		{
			m_csListen.Lock();
			std::list<AFK_LISTER_DATA*>::iterator it = find_if(m_lstListenHandle.begin(), m_lstListenHandle.end(), SearchLSIbyHandle((LONG)pServerHandle));
			if (it != m_lstListenHandle.end() && NULL != (*it))
			{
				AFK_CLIENT_NODE* newClient = new AFK_CLIENT_NODE;
				if (newClient != NULL)
				{
					memset(newClient, 0, sizeof(AFK_CLIENT_NODE));
					newClient->clnHandle = pParam;
					strcpy(newClient->ip, szClientIp);
					newClient->port = nClientPort;
					newClient->status = 0;	//未上传序列号
					(*it)->lstClients.push_back(newClient);
				}
			}
			m_csListen.UnLock();
		}
		break;
	case 1:	// 设备传上来了序列号，由数据处理线程过来
		{
			char *serial = (char*)pParam;
			if (NULL == serial || strlen(serial) > DEV_SERIAL_NUM_LEN)
			{
				break;
			}

			AFK_CLIENT_REGISTER_INFO *pCltRegInfo = new AFK_CLIENT_REGISTER_INFO;
			if (pCltRegInfo != NULL)
			{
				memset(pCltRegInfo, 0, sizeof(AFK_CLIENT_REGISTER_INFO));
				pCltRegInfo->caller = pServerHandle;
				strcpy(pCltRegInfo->ip, szClientIp);
				pCltRegInfo->port = nClientPort;
				strcpy(pCltRegInfo->serial, serial);
				
				m_csCltRegInfo.Lock();
				m_lstCltRegInfo.push_back(pCltRegInfo);
				m_csCltRegInfo.UnLock();

				SetEventEx(m_hAutoRegiEvent);
			}
		}
		break;
	case -1: // 连接断线，由接收线程过来
		{
			AFK_CLIENT_DISCONN_INFO *pCltDisconn = new AFK_CLIENT_DISCONN_INFO;
			if (pCltDisconn != NULL)
			{
				memset(pCltDisconn, 0, sizeof(AFK_CLIENT_DISCONN_INFO));
				pCltDisconn->caller = pServerHandle;
				strcpy(pCltDisconn->ip, szClientIp);
				pCltDisconn->port = nClientPort;
				pCltDisconn->clnHandle = pParam;

				m_csCltDisconn.Lock();
				m_lstCltDisconn.push_back(pCltDisconn);
				m_csCltDisconn.UnLock();

				SetEventEx(m_hAutoRegiDisEvent);
			}
		}
	    break;
	default:
	    break;
	}
}

/*
 * 摘要：开始侦听设备
 */
LONG CManager::ListenServer(char *szIp, int nPort, fServiceCallBack cbListen, DWORD dwUserData)
{
	if (NULL == m_pDeviceProb)
	{
		SetLastError(NET_NO_INIT);
		return 0;
	}

	if (NULL == cbListen)
	{
		SetLastError(NET_ILLEGAL_PARAM);
		return 0;
	}
	
	void *pServerHandle = m_pDeviceProb->start_listen(szIp, nPort, onListenServerFunc, this, m_nConnBufLen);
	if (NULL == pServerHandle)
	{
		SetLastError(NET_ERROR);
		return 0;
	}

	AFK_LISTER_DATA *pstListenData = new AFK_LISTER_DATA;
	if (NULL == pstListenData)
	{
		m_pDeviceProb->stop_listen(pServerHandle);
		SetLastError(NET_SYSTEM_ERROR);
		return 0;
	}
	
	pstListenData->lsnHandle = pServerHandle;
	pstListenData->pCb = cbListen;
	pstListenData->lsnUser = dwUserData;

	m_csListen.Lock();
	m_lstListenHandle.push_back(pstListenData);
	m_csListen.UnLock();

	return (LONG)pstListenData;
}

/*
 * 摘要：停止侦听设备
 */
BOOL CManager::StopListenServer(LONG lServerHandle)
{
	AFK_LISTER_DATA *pstListenData = (AFK_LISTER_DATA *)lServerHandle;
	if (pstListenData == NULL)
	{
		return FALSE;
	}

	BOOL bRet = FALSE;
	m_csListen.Lock();

	std::list<AFK_LISTER_DATA*>::iterator itLsn;
	std::list<AFK_CLIENT_NODE*>::iterator itClnt;

	// 检验句柄有效性
	itLsn = find(m_lstListenHandle.begin(), m_lstListenHandle.end(), pstListenData);
	if (itLsn == m_lstListenHandle.end())
	{
		SetLastError(NET_INVALID_HANDLE);
		goto e_out;
	}
	
	// 停止侦听
	bRet =  m_pDeviceProb->stop_listen(pstListenData->lsnHandle);
	if (!bRet)
	{
		SetLastError(NET_ERROR);
		goto e_out;
	}

	// 关闭可能的客户端连接
	itClnt = pstListenData->lstClients.begin();
	while (itClnt != pstListenData->lstClients.end())
	{
		m_pDeviceProb->close_client((*itClnt)->clnHandle);
		delete (*itClnt);
		itClnt++;
	}
	pstListenData->lstClients.clear();

	delete pstListenData;
	m_lstListenHandle.erase(itLsn);

	m_csListen.UnLock();
	return bRet;

e_out:
	m_csListen.UnLock();
	return FALSE;
}

BOOL CManager::ResponseDevReg(char *szDevSerial, char *szIp, int nPort, BOOL bAccept)
{
	if (NULL == szDevSerial || NULL == szIp)
	{
		SetLastError(NET_ILLEGAL_PARAM);
		return FALSE;
	}

	if (strlen(szDevSerial) > DEV_SERIAL_NUM_LEN)
	{
		SetLastError(NET_ILLEGAL_PARAM);
		return FALSE;
	}

	char szDevSerialUTF8[2*DEV_SERIAL_NUM_LEN+2] = {0};
	Change_Assic_UTF8(szDevSerial, strlen(szDevSerial), szDevSerialUTF8, 2*DEV_SERIAL_NUM_LEN+2);
	szDevSerialUTF8[2*DEV_SERIAL_NUM_LEN+1] = '\0';

	std::list<AFK_LISTER_DATA*>::iterator itLsn;
	std::list<AFK_CLIENT_NODE*>::iterator itCln;
	itLsn = m_lstListenHandle.begin();

	BOOL bRet = FALSE;
	
	BOOL bExistClient = FALSE;

	while (itLsn != m_lstListenHandle.end() && (*itLsn))
	{
		itCln = (*itLsn)->lstClients.begin();
		while (itCln != (*itLsn)->lstClients.end() && (*itCln))
		{
			if (0 == (*itCln)->status) 
			{
				itCln++;
				continue;
			}

			if (_stricmp((*itCln)->serial, szDevSerialUTF8) == 0
				&& _stricmp((*itCln)->ip, szIp) == 0
				&& (*itCln)->port == nPort)
			{
				//found!
				bExistClient = TRUE;
				break;
			}
			itCln++;
		}

		if (bExistClient)
		{
			break;
		}

		itLsn++;
	}

	if (!bExistClient)
	{
		SetLastError(NET_ILLEGAL_PARAM);
		return FALSE;
	}

	m_pDeviceProb->response_reg((*itCln)->clnHandle, bAccept?TRUE:FALSE);
	
	if (!bAccept)
	{
		m_pDeviceProb->close_client((*itCln)->clnHandle);
		delete (*itCln);
		(*itLsn)->lstClients.erase(itCln);
	}
	else
	{
		Sleep(100); //damn..
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

BOOL CManager::SetDeviceMode(LONG lLoginID, EM_USEDEV_MODE emType, void* pValue)
{
	if (IsDeviceValid((afk_device_s *)lLoginID) < 0)
    {
		SetLastError(NET_INVALID_HANDLE);
        return FALSE;
    }
	
	int nRet = -1;
	afk_device_s *device = (afk_device_s *)lLoginID;

	switch(emType)
	{
	case TALK_CODE_CLIENT_MODE:
		{
			int nTalkMode = 0;
			nRet = device->set_info(device, dit_talk_use_mode, &nTalkMode);
		}
		break;
	case TALK_CODE_SERVER_MODE:
		{
			int nTalkMode = 1;
			nRet = device->set_info(device, dit_talk_use_mode, &nTalkMode);
		}
	    break;
	case TALK_CODE_ENCODE_TYPE:
		{
			DEV_TALK_DECODE_INFO *pEncodeType = (DEV_TALK_DECODE_INFO*)pValue;
			if (pEncodeType == NULL)
			{
				return FALSE;
			}

			AUDIO_ATTR_T stAudioAttr = {0};
			stAudioAttr.iAudioType = (WORD)pEncodeType->encodeType;
			stAudioAttr.iAudioBit = (WORD)pEncodeType->nAudioBit;
			stAudioAttr.dwSampleRate = pEncodeType->dwSampleRate;
			nRet = device->set_info(device, dit_talk_encode_type, &stAudioAttr);
		}
		break;
	case TALK_CODE_LISTEN_MODE:
		{
			nRet = device->set_info(device, dit_alarm_listen_use_mode, pValue);		
		}
		break;
	case TALK_CONFIG_AUTHORITY_MODE:
		{
			int nAuthorityMode = *(int*)pValue;

			if (0 == nAuthorityMode)//不采用配置权限
			{
				DVR_AUTHORITY_INFO nAuthorityInfo;
				memset(&nAuthorityInfo, 0, sizeof(nAuthorityInfo));
				for (int i = 0; i < MAX_AUTHORITY_NUM; i++)
				{
					nAuthorityInfo.bAuthorityInfo[i] = 1;
				}

				nRet = device->set_info(device, dit_config_authority_mode, &nAuthorityInfo);
			}
			
			else if (1 == nAuthorityMode)//采用配置权限
			{
				nRet = AnalyzeAuthorityInfo(device);
			}
	
		}
		break;
	default:
	    break;
	}

	return nRet>=0? TRUE:FALSE;
}

// 协议的配置类型转换为接口的配置类型
BOOL ConvertConfigType(WORD wInType, DWORD &dwOutType)
{
	switch(wInType)
	{
	case 0:
		dwOutType = DEV_DEVICECFG;
		break;
	case 1:
		dwOutType = DEV_COMMCFG;
		break;
	case 2:
		dwOutType = DEV_NETCFG;
		break;
	case 3:
		dwOutType = DEV_RECORDCFG;
		break;
	case 4:
		dwOutType = DEV_CHANNELCFG;
		break;
	case 5:
		dwOutType = DEV_COMMCFG;
		break;
	case 6:
		dwOutType = DEV_ALARMCFG;
		break;
	case 7:
		dwOutType = DEV_ALARMCFG;
		break;
		//case 8:
	case 10:
		dwOutType = DEV_CHANNELCFG;
		break;
	case 11:
		dwOutType = DEV_MAIL_CFG;
		break;
	case 12:
		dwOutType = DEV_PREVIEWCFG;
		break;
	case 13:
		dwOutType = DEV_NETCFG;
		break;
	case 14:
		dwOutType = DEV_NETCFG;
		break;
	case 15:
		dwOutType = DEV_SNIFFER_CFG;
		break;
		
		//case 16:
		//case 17:
	case 18:
		dwOutType = DEV_AUTOMTCFG;
		break;
	case 19:
		dwOutType = DEV_NTP_CFG;
		break;
		//case 20:
		//case 22:
		//case 23:
		//case 24:
		//case 25:
		//case 26:
		//case 27:
		//case 29:
		dwOutType = DEV_WATERMAKE_CFG;
		break;
	case 30:
		dwOutType = DEV_VEDIO_MARTIX;
		break;
		//case 31:
		//case 32:
		//case 33:
		//case 34:
		dwOutType = DEV_VIDEO_COVER;
		break;
	case 35:
		dwOutType = DEV_NETCFG;
		break;
	case 36:
		dwOutType = DEV_WEB_URL_CFG;
		break;
	case 37:
		dwOutType = DEV_FTP_PROTO_CFG;
		break;
	case 38:
		dwOutType = DEV_CAMERA_CFG;
		break;
	case 39:
		dwOutType = DEV_NETCFG;
		break;
	case 40:
		dwOutType = DEV_DNS_CFG;
		break;
	case 41:
		dwOutType = DEV_STORAGE_STATION_CFG;
		break;
	case 42:
		dwOutType = DEV_DOWNLOAD_STRATEGY;
		break;
		//case 43:
	case 123 :
		dwOutType = DEV_SNAP_CFG;
		break;
	case 124:
		dwOutType = DEV_ALARMCFG;
		break;
	case 125:
		dwOutType = DEV_RECORDCFG;
		break;
	case 126:
		dwOutType = DEV_CHANNELCFG;
		break;
	case 127:
		dwOutType = DEV_SNAP_CFG;
		break;
	case 128:
		dwOutType = DEV_CHANNELCFG;
		break;
		//case 130://
	case 131:
		dwOutType = DEV_WLAN_CFG;
		break;
		//case 132:
	case 133:
		dwOutType = DEV_TRANS_STRATEGY;
		break;
		//case 135 : DEV_WLAN_DEVICE_CFG
	case 140:
		dwOutType = DEV_MULTI_DDNS;
		break;
		//case 151:
		//case 189:
	case 190:
		dwOutType = DEV_INTERVIDEO_CFG;
		break;
		//case 200:
		//case 201:
		break;
	case 241:
		dwOutType = DEV_REGISTER_CFG;
		break;
	default:
		return FALSE;	
	}
			
	return TRUE;
}

int CManager::AnalyzeAuthorityInfo(afk_device_s* device)
{
	if (NULL == device)
	{
		return -1;
	}
	
	char usrname[20] = {0};
	device->get_info(device, dit_user_name, &usrname);
	
    USER_MANAGE_INFO  UserInfo = {0};	
	DVR_AUTHORITY_INFO AuthorityInfo = {0};
	
	for (int i = 0; i < MAX_AUTHORITY_NUM; i++)
	{
		AuthorityInfo.bAuthorityInfo[i] = 1;
	}
	
	int nRet = -1;
	nRet = GetDevConfig().QueryUserInfo((LONG)device, &UserInfo, 500);
	
	if (nRet >= 0) //数据解析
	{
		
		char FunctionType[MAX_AUTHORITY_NUM][20]={"CtrPanel","Shutdown","Monitor","Monitor_01",	"Monitor_02",	
													"Monitor_03","Monitor_04","Monitor_05","Monitor_06","Monitor_07",
													"Monitor_08","Monitor_09","Monitor_10","Monitor_11","Monitor_12","Monitor_13",
													"Monitor_14","Monitor_15","Monitor_16","Replay","Replay_01",	
													"Replay_02","Replay_03","Replay_04","Replay_05","Replay_06",
													"Replay_07","Replay_08","Replay_09","Replay_10","Replay_11",
													"Replay_12","Replay_13","Replay_14","Replay_15","Replay_16",	
													"NetPreview","NetPreview_01","NetPreview_02","NetPreview_03","NetPreview_04",
													"NetPreview_05","NetPreview_06","NetPreview_07","NetPreview_08",
													"NetPreview_09","NetPreview_10","NetPreview_11","NetPreview_12",
													"NetPreview_13","NetPreview_14","NetPreview_15","NetPreview_16",
													"Record","Backup","MHardisk","MPTZ","Account",	"Sysinfo","Alarm",
													"Config","QueryLog","DelLog","SysUpdate","Control","AutoMaintain",
													"GeneralConfig","EncodeConfig","RecordConfig","ComConfig","NetConfig",
													"AlarmConfig","VideoConfig","PtzConfig","OutputConfig","DefaultConfig",
													 "DataFormat"};
		
		//权限
		for (int i = 0; i < UserInfo.dwRightNum; i++)
		{
			for (int j = 0; j < MAX_AUTHORITY_NUM; j++)
			{
				if (0 == FunctionType[j][0])//结束
				{
					break;
				}
				
				//找到设备的权限
				if (0 == _stricmp(UserInfo.rightList[i].name, FunctionType[j]))
				{
					AuthorityInfo.bAuthorityInfo[j] = 0;
					
					for (int userindex = 0; userindex < UserInfo.dwUserNum; userindex++)
					{
						//找到当前用户
						if (0 == _stricmp(UserInfo.userList[userindex].name, usrname))
						{
							for (int curuser = 0; curuser < UserInfo.userList[userindex].dwRightNum ; curuser++ )
							{
								//判断当前用户是否有该权限
								if (UserInfo.rightList[i].dwID == UserInfo.userList[userindex].rights[curuser])
								{
									AuthorityInfo.bAuthorityInfo[j] = 1;
									break;
								}
							}
							
							break;
						}
					}
					
					break;
				}
			}
		}
	}
	
	nRet = device->set_info(device, dit_config_authority_mode, &AuthorityInfo);

	return nRet>=0?0:-1;
}

void CManager::SetNetParameter(NET_PARAM *pNetParam)
{
	if (NULL == pNetParam)
	{
		return;
	}

	m_nLoginTimeout = pNetParam->nWaittime > 0 ? pNetParam->nWaittime : m_nLoginTimeout;
	m_nConnTryNum = pNetParam->nConnectTryNum > 0 ? pNetParam->nConnectTryNum : m_nConnTryNum;
	m_nConnHostTime = pNetParam->nConnectTime > 0 ? pNetParam->nConnectTime : m_nConnHostTime;
	m_nSubConnSpaceTime = pNetParam->nSubConnectSpaceTime > 0 ? pNetParam->nSubConnectSpaceTime : m_nSubConnSpaceTime;
	m_nConnBufLen = pNetParam->nConnectBufSize > 0 ? pNetParam->nConnectBufSize : m_nConnBufLen;
	m_nGetDevInfoTime = pNetParam->nGetDevInfoTime > 0 ? pNetParam->nGetDevInfoTime : m_nGetDevInfoTime;
	m_nGetSubConnTime = pNetParam->nGetConnInfoTime > 0 ? pNetParam->nGetConnInfoTime : m_nGetSubConnTime;
}

void CManager::GetNetParameter(NET_PARAM *pNetParam)
{
	if (pNetParam == NULL)
	{
		return;
	}

	pNetParam->nWaittime = m_nLoginTimeout;
	pNetParam->nConnectTryNum = m_nConnTryNum;
	pNetParam->nConnectTime = m_nConnHostTime;
	pNetParam->nSubConnectSpaceTime = m_nSubConnSpaceTime;
	pNetParam->nConnectBufSize = m_nConnBufLen;
	pNetParam->nGetDevInfoTime = m_nGetDevInfoTime;
	pNetParam->nGetConnInfoTime = m_nGetSubConnTime;
}
	
DWORD CManager::GetPacketSequence()
{
	InterlockedIncrementEx(&m_dwPacketSequence);
	if (m_dwPacketSequence > 0x7FFFFFF0)
	{
		m_dwPacketSequence = 1;
	}
	
	return m_dwPacketSequence;
}




