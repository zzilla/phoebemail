
#include "serverset.h"
#include "Manager.h"
#include "utils.h"

#ifndef HEADER_SIZE
#define HEADER_SIZE 32
#endif

typedef enum
{
	alarm_reserved,
	alarm_normal,
	alarm_motion,
	alarm_loss,
	alarm_blind,
	alarm_audio,
	alarm_diskfull,
	alarm_diskbreak,
	alarm_nodisk,
	alarm_decoder,
	alarm_posticket,
	alarm_posrawdata,
	alarm_upload_cfg = 120,
	alarm_spel = 150,
} ALARM_TYPE;
//////////////////////////////////////////////////////////////////////////
//class CAlarmClient
CAlarmClient::CAlarmClient():m_lLifeCount(1)
{
	memset(m_szIP, 0 , 64);
	m_nPort = 0;

	m_nWritePos = 0;
	m_nReadPos = 0;
}

CAlarmClient::~CAlarmClient()
{

}

int CAlarmClient::GetData(char *szBuf, int nBufLen)
{
	int nDataLen = 0;
	
	m_csBuf.Lock();
	
	if ((int)(m_nWritePos - m_nReadPos) >= HEADER_SIZE)
	{
		unsigned int extlen = *(unsigned int*)(m_szBuf + m_nReadPos + 4);
		
		if ((extlen + HEADER_SIZE) >= ALARM_BUFFER_SIZE)
		{
			//	指示的扩展数据太长，不正常，清空缓存
			m_nWritePos = m_nReadPos = 0;
			
			m_csBuf.UnLock();
			return 0;
		}
		
		if (m_nWritePos - m_nReadPos >= extlen + HEADER_SIZE)
        {
			nDataLen = extlen + HEADER_SIZE;
			if(nBufLen >= nDataLen)
			{
				memcpy(szBuf, m_szBuf + m_nReadPos, nDataLen);
			}
			m_nReadPos += nDataLen;
        }
	}
	
	m_csBuf.UnLock();
	
	return nDataLen;

}

int CAlarmClient::PutData(const char* szData, int nDataLen)
{
	m_csBuf.Lock();

    if (nDataLen > 0)
    {
        /***********************缓冲数据***********************/
        //	现在当包长大于存储空间时采取丢包的原则
        //	情况可能有：调试中断太久和收到错误数据包
		
        int nEndPos = nDataLen + m_nWritePos;
        
		//	如果缓冲区足够缓冲数据
        if (nEndPos <= ALARM_BUFFER_SIZE)
        {
            memcpy(m_szBuf + m_nWritePos, szData, nDataLen);
            m_nWritePos = m_nWritePos + nDataLen;
        }
		//	如果缓冲区不足以缓冲数据,从头再来
        else
        {   
			//	整个缓冲区都不足以容纳该数据，一般是因为调试中断太久或错误数据包
            if (nDataLen + (m_nWritePos-m_nReadPos) >= ALARM_BUFFER_SIZE)
            {
				nDataLen = 0;
            }
			else
            {
				memmove(m_szBuf, m_szBuf + m_nReadPos, m_nWritePos - m_nReadPos);
				
				m_nWritePos = m_nWritePos - m_nReadPos;
				m_nReadPos = 0;				
				
				if (nDataLen > 0)
				{
					memcpy(m_szBuf + m_nWritePos, szData, nDataLen);
					m_nWritePos = m_nWritePos + nDataLen;
				}
			}
		}
    }
	
	m_csBuf.UnLock();
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//class CServerSet
int CServerSet::DealAlarmPacket(char* szIP, int nPort, char* szBuf, int nBufLen)
{
	if(m_cbAlarm && nBufLen >= 36 && szBuf[0] == 0x69 && szBuf != NULL)
	{
		char alarmData[32] = {0};
		DWORD dwExtData = *(DWORD *)(szBuf+HEADER_SIZE);//扩展数据
		switch(szBuf[12])//报警类型
		{
		case alarm_normal:
			if (m_cbAlarm != NULL)
			{
				for (int i = 0; i < 16; i++)
				{
					if (dwExtData&0x01)
					{
						alarmData[i] = 1;
					}
					dwExtData >>= 1;
				}
				m_cbAlarm(m_lHandle, szIP, nPort, OUT_ALARM_EX, alarmData, 16, m_dwUserData);
			}
			break;
		case alarm_motion:
			if (m_cbAlarm)
			{
				for (int i = 0; i < 16; i++)
				{
					if (dwExtData&0x01)
					{
						alarmData[i] = 1;
					}
					dwExtData >>= 1;
				}
				m_cbAlarm(m_lHandle, szIP, nPort, MOTION_ALARM_EX, alarmData, 16, m_dwUserData);
			}
			break;
		case alarm_loss:
			if (m_cbAlarm)
			{
				for (int i = 0; i < 16; i++)
				{
					if (dwExtData&0x01)
					{
						alarmData[i] = 1;
					}
					dwExtData >>= 1;
				}
				m_cbAlarm(m_lHandle, szIP, nPort, VIDEOLOST_ALARM_EX, alarmData, 16, m_dwUserData);
			}
			break;
		case alarm_blind:
			if (m_cbAlarm)
			{
				for (int i = 0; i < 16; i++)
				{
					if (dwExtData&0x01)
					{
						alarmData[i] = 1;
					}
					dwExtData >>= 1;
				}
				m_cbAlarm(m_lHandle, szIP, nPort, SHELTER_ALARM_EX, alarmData, 16, m_dwUserData);
			}
			break;
		case alarm_audio:
			if (m_cbAlarm)
			{
				for (int i = 0; i < 16; i++)
				{
					if (dwExtData&0x01)
					{
						alarmData[i] = 1;
					}
					dwExtData >>= 1;
				}
				m_cbAlarm(m_lHandle, szIP, nPort, SOUND_DETECT_ALARM_EX, alarmData, 16, m_dwUserData);
			}
			break;
		case alarm_diskfull:
			if (m_cbAlarm)
			{
				if (dwExtData)
				{
					alarmData[0] = 1;
					m_cbAlarm(m_lHandle, szIP, nPort, DISKFULL_ALARM_EX, alarmData, 1, m_dwUserData);
				}
			}
			break;
		case alarm_diskbreak:
			if (m_cbAlarm)
			{
				for (int i = 0; i < 32; i++)
				{
					if (dwExtData&0x01)
					{
						alarmData[i] = 1;
					}
					dwExtData >>= 1;
				}
				m_cbAlarm(m_lHandle, szIP, nPort, DISKERROR_ALARM_EX, alarmData, 32, m_dwUserData);
			}
			break;
		case alarm_decoder:
			if(m_cbAlarm)
			{
				ALARM_DECODER_ALARM stuAlarmDecoderAlarm = {0};
				if((nBufLen-HEADER_SIZE)%sizeof(ALARM_DECODER) == 0)
				{
					stuAlarmDecoderAlarm.nAlarmDecoderNum = (nBufLen-HEADER_SIZE)/sizeof(ALARM_DECODER);
					memcpy(stuAlarmDecoderAlarm.stuAlarmDecoder, szBuf+HEADER_SIZE, nBufLen-HEADER_SIZE);
					
					m_cbAlarm(m_lHandle, szIP, nPort, ALARM_DECODER_ALARM_EX,
						&stuAlarmDecoderAlarm, sizeof(ALARM_DECODER_ALARM), m_dwUserData);
				}
			}
			break;
			
		case alarm_posticket:
			if(m_cbAlarm)
			{
				if (dwExtData)
				{
					alarmData[0] = 1;
					m_cbAlarm(m_lHandle, szIP, nPort, POS_TICKET_ALARM_EX, alarmData, 1, m_dwUserData);
				}
			}
			break;
		case alarm_posrawdata:
			if(m_cbAlarm)
			{
				if (dwExtData)
				{
					alarmData[0] = 1;
					m_cbAlarm(m_lHandle, szIP, nPort, POS_TICKET_RADATA_EX, alarmData, 1, m_dwUserData);
				}
			}
			break;
			
		default:
			{
#ifdef _DEBUG
				OutputDebugString("Unknown alarmType ok \n");
#endif
				break;
			}
		}
	}
	return 1;
}

int CALLBACK ServiceCallBack(LONG lHandle, int connId, char *szIp, WORD wPort, LONG lCommand, void *pParam, DWORD dwParamLen, DWORD dwUserData)
{
	CServerSet *pServer = (CServerSet *)dwUserData;
	if(NULL == pServer || pServer->GetHandle() != lHandle)
	{
		return -1;
	}
	
	switch(lCommand)
	{
	case 1://on connect
		{
			CAlarmClient *pClient = new CAlarmClient();
			strcpy(pClient->m_szIP, szIp);
			pClient->m_nPort = wPort;

			pServer->AddTcpServerConnect(connId, pClient);
		}
		break;
	case 2://on data
		{
			pServer->FindTcpServerConnect(connId, (char *)pParam, dwParamLen);
		}
		break;
	case 3://on deal data
		{
//			pServer->DealTcpServerConnect(connId);
		}
		break;
	case 4://on disconnect
		{
			pServer->DelTcpServerConnect(connId);
		}
		break;
	default:
		{
#ifdef _DEBUG
			OutputDebugString("command error!\n");	
#endif
		}
		break;
	}
	
	return 1;
}



CServerSet::CServerSet(CManager *pManager)
	: m_pManager(pManager)
{
	m_lHandle = 0;
	m_cbAlarm = NULL;
}

CServerSet::~CServerSet()
{
	CAlarmClient *pClientInfo = NULL;
}

BOOL CServerSet::Init(void)
{
	return TRUE;
}

BOOL CServerSet::Uninit(void)
{
	if(m_lHandle > 0)
	{
		StopServer(m_lHandle);
	}
	return TRUE;
}

LONG CServerSet::StartServer(WORD wPort, char *pIp, fServiceCallBack pfcb, DWORD dwTimeOut, DWORD dwUserData)
{
	if(m_lHandle > 0)
	{
		return 0;
	}

	LONG lHandle = m_pManager->m_pDeviceProb->start_alarm_server(pIp, wPort, ServiceCallBack, (DWORD)this);
	if(lHandle <= 0)
	{
		return -1;
	}
	
	m_csClientList.Lock();
	m_cbAlarm = pfcb;
	m_dwUserData = dwUserData;
	m_lHandle = lHandle;
	m_csClientList.UnLock();
	return lHandle;
}

BOOL CServerSet::StopServer(LONG lHandle)
{
	if(m_lHandle == lHandle)
	{
		m_csClientList.Lock();
		map<int, CAlarmClient *>::iterator i = m_clientList.begin();
		for(; i != m_clientList.end(); i++)
		{
			CAlarmClient *pClientInfo = (*i).second;
			if(pClientInfo != NULL)
			{
				delete pClientInfo;
			}
		}
		m_clientList.clear();
		m_csClientList.UnLock();

		m_pManager->GetASCS().Lock();
		m_pManager->m_pDeviceProb->stop_alarm_server(lHandle);
		m_lHandle = 0;
		m_pManager->GetASCS().UnLock();
		
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void CServerSet::AddTcpServerConnect(int connId, CAlarmClient *pClient)
{
	m_csClientList.Lock();
	m_clientList[connId] = pClient;
	m_csClientList.UnLock();
}

void CServerSet::DelTcpServerConnect(int connId)
{
	m_csClientList.Lock();

	map<int, CAlarmClient *>::iterator it = m_clientList.find(connId);
	if(it != m_clientList.end())
	{
		CAlarmClient *pClient = (*it).second;
		if(NULL != pClient)
		{
			pClient->DecRef();
		}
		m_clientList.erase(it);
	}

	m_csClientList.UnLock();
}

void CServerSet::FindTcpServerConnect(int connId, char *szData, int nDataLen)
{
	CAlarmClient *pClient = NULL;

	m_csClientList.Lock();
	map<int, CAlarmClient *>::iterator it = m_clientList.find(connId);
	if(it != m_clientList.end())
	{
		pClient = (*it).second;
		if(NULL != pClient)
		{
			pClient->PutData(szData, nDataLen);
			pClient->AddRef();
		}
	}
	m_csClientList.UnLock();	

	if (pClient != NULL) 
	{
		char szBuf[512] = {0};
		int nLen = pClient->GetData(szBuf, 512);
		while(nLen > 0)
		{		
			if(szData[17] == 0)
			{
				DealAlarmPacket(pClient->m_szIP, pClient->m_nPort, szBuf, nLen);
			}
			else if(szData[17] == 1) //新的报警上传包
			{
				DealNewAlarmPacket(pClient->m_szIP, pClient->m_nPort, szBuf, nLen);
			}
			nLen = pClient->GetData(szBuf, 512);
		}

		pClient->DecRef();
	}
}

void CServerSet::DealTcpServerConnect(int connId)
{

}

int  CServerSet::DealNewAlarmPacket(char* szIP, int nPort, char* szBuf, int nBufLen)
{
	if(NULL == szIP || NULL == szBuf || nBufLen < 32)//亿蛙协议包不会小于32长度
	{
		return -1;
	}

	if(szBuf[0] == 0x69)
	{
		NEW_ALARM_UPLOAD	stuAlarmUpload = {0};
		char szPacketBuf[512] = {0};
		char szOutBuf[256] = {0};
		int nRet = -1;
		memcpy(szPacketBuf, szBuf+HEADER_SIZE, nBufLen-HEADER_SIZE);
		strcpy(stuAlarmUpload.szGlobalIP, szIP);
		stuAlarmUpload.nPort = nPort;
		//nRet 返回实际长度，所以memcpy之前没有memset(..., 0, ...);
		nRet = GetValueFormNewPacket(szPacketBuf, "alarm channel", szOutBuf, 256);
		if(nRet > 0)
		{
			stuAlarmUpload.dwAlarmMask = atoi(szOutBuf);
		}
		memset(szOutBuf, 0 ,256);
		nRet = GetValueFormNewPacket(szPacketBuf, "Domain name", szOutBuf, 256);
		if(nRet > 0 && nRet < 256)
		{	
			Change_Utf8_Assic((unsigned char *)szOutBuf, stuAlarmUpload.szDomainName);
		}
		nRet = GetValueFormNewPacket(szPacketBuf, "Time", szOutBuf, 256);
		if(nRet > 0 && nRet < 40)
		{
			memcpy(stuAlarmUpload.szAlarmOccurTime, szOutBuf, nRet);
		}
		switch(szBuf[12]) 
		{
		case alarm_normal:	//1	外部报警
			{
				stuAlarmUpload.dwAlarmType = UPLOAD_GENERAL_ALARM;
			}
			break;
		case alarm_motion:	//2	动态检测报警
			{
				stuAlarmUpload.dwAlarmType = UPLOAD_MOTION_ALARM;
			}
			break;
		case alarm_loss:	//3 视频丢失报警
			{
				stuAlarmUpload.dwAlarmType = UPLOAD_VIDEOLOST_ALARM;
			}
			break;
		case alarm_blind:	//4 视频遮挡报警
			{
				stuAlarmUpload.dwAlarmType = UPLOAD_SHELTER_ALARM;
			}
			break;
		case alarm_audio:	//5 音频检测报警
			{
				stuAlarmUpload.dwAlarmType = UPLOAD_SOUND_DETECT_ALARM;
			}
			break;
		case alarm_diskfull://6 硬盘满报警
			{
				stuAlarmUpload.dwAlarmType = UPLOAD_DISKFULL_ALARM;
			}
			break;
		case alarm_diskbreak://7 坏硬盘报警
			{
				stuAlarmUpload.dwAlarmType = UPLOAD_DISKERROR_ALARM;
			}
			break;
		case alarm_nodisk:	//8 
			break;
		case alarm_decoder:	//9 解码器报警
			{
				stuAlarmUpload.dwAlarmType = UPLOAD_DECODER_ALARM;
				memset(szOutBuf, 0, 256);
				nRet = GetValueFormNewPacket(szPacketBuf, "DeAlarm index", szOutBuf, 256);
				if(nRet > 0)
				{
					stuAlarmUpload.bAlarmDecoderIndex = atoi(szOutBuf);
				}
			}
			break;
		case alarm_upload_cfg://定时上传配置
			{
				stuAlarmUpload.dwAlarmType = UPLOAD_EVENT_ONTIME;
				nRet = GetValueFormNewPacket(szPacketBuf, "Global IP", szOutBuf, 256);
				if(nRet > 0 && nRet < MAX_IP_ADDRESS_LEN)
				{
					memset(stuAlarmUpload.szGlobalIP, 0, MAX_IP_ADDRESS_LEN);//前面有可能赋值，所以清零.
					memcpy(stuAlarmUpload.szGlobalIP, szOutBuf, nRet);
				}
			}
			break;
		case alarm_posticket:
			{
				stuAlarmUpload.dwAlarmType = UPLOAD_POSTICKET_ALARM;
			}
			break;
		case alarm_spel:	//150
			break;
		default:
			{
#ifdef _DEBUG
			OutputDebugString("CServerSet::DealNewAlarmPacket, wrong packet");
#endif
			return 0;
			}
			break;
		}//end switch
		
		m_cbAlarm(m_lHandle, szIP, nPort, stuAlarmUpload.dwAlarmType, (char *)&stuAlarmUpload, sizeof(NEW_ALARM_UPLOAD), m_dwUserData);
	}//end if
	return 0;
}

/************************************************************************
返回值 ： 
	0:	szOutBuf不够；
	>0:	返回拷贝的长度; 
	-1:	表示没有找到szKeyBuf对应的项;
	-2:	参数错误; 
	-3:	包错误
************************************************************************/
int CServerSet::GetValueFormNewPacket(char *szBuf, char *szKeyBuf, char *szOutBuf, int nOutBufLen)
{
	if(NULL == szOutBuf || NULL == szBuf || NULL == szKeyBuf || 0 >= nOutBufLen)
	{
		return -2;
	}

	char *pKeyWord = strstr(szBuf, szKeyBuf);
	if(NULL == pKeyWord)
	{
		return -1;
	}
	int nKeyLen = strlen(szKeyBuf);
	if(*(pKeyWord+nKeyLen) == ':' && 
		(pKeyWord == szBuf || (*(pKeyWord-1) == '\n' && *(pKeyWord-2) == '\r')))//如果为第一项或者前面为“\r\n”
	{
		char *pValueStart = pKeyWord+nKeyLen+1;
		char *pValueEnd = strstr(pValueStart, "\r\n");
		if((pValueEnd - pValueStart) > nOutBufLen)
		{
			return 0;
		}
		if((pValueEnd - pValueStart) < 0)
		{
#ifdef _DEBUG
			OutputDebugString("error, occur on \"CServerSet::GetValueFormNewPacket\"\n");
#endif
			return -3;
		}
		memcpy(szOutBuf, pValueStart, pValueEnd-pValueStart);
		return (pValueEnd-pValueStart);
	}
	else
	{
		return -3;
	}
}
 		
 
