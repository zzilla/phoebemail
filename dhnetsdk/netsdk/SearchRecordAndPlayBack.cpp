
#include "StdAfx.h"
#include "SearchRecordAndPlayBack.h"
#include "Manager.h"
#include "netsdktypes.h"
#include "RenderManager.h"
#include "Utils.h"
#include "./NetPlayBack/NetPlayBack.h"
#include "VideoRender.h"
#include "DevConfig.h"
#include "DevConfigEx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSearchRecordAndPlayBack::CSearchRecordAndPlayBack(CManager *pManager)
	: m_pManager(pManager)
{

}

CSearchRecordAndPlayBack::~CSearchRecordAndPlayBack()
{

}

int CSearchRecordAndPlayBack::Init()
{
	return Uninit();
}

int CSearchRecordAndPlayBack::Uninit()
{
	int nRet = 0;

	ReleaseAllSearchRecordInfo();
	
	{
		m_csNPI.Lock();
		list<st_NetPlayBack_Info*>::iterator it = m_lstNPI.begin();
		while(it != m_lstNPI.end())
		{
			if (*it)
			{
				int nRet = Process_stopplayback(**it);
				if (nRet >= 0)
				{
					delete (*it);
				}
				m_lstNPI.erase(it++);
			}
			else
			{
				it++;
			}
		}

		m_lstNPI.clear();
		m_csNPI.UnLock();
	}
	
	{
		m_csDLI.Lock();
		list<st_DownLoad_Info*>::iterator it = m_lstDLI.begin();
		for(; it != m_lstDLI.end(); ++it)
		{
			if (*it)
			{
				(*it)->channel->close((*it)->channel);
				if ((*it)->file)
				{
					fclose((*it)->file);
					(*it)->file = NULL;
				} 
				
				delete (*it);
			}
		}
		
		m_lstDLI.clear();
		m_csDLI.UnLock();
	}

	return 0;
}


int CSearchRecordAndPlayBack::CloseChannelOfDevice(afk_device_s* device)
{
	int nRet = 0;

	{
		m_csSRI.Lock();
		list<st_SearchRecord_Info*>::iterator it = m_lstSRI.begin();
		while(it != m_lstSRI.end())
		{
			if ((*it))
			{
				if ((*it)->device == device)
				{
					ReleaseRecordFileInfo(**it);
					delete (*it);
					m_lstSRI.erase(it++);
				}
				else
				{
					++it;
				}
			}
			else
			{
				++it;
			}
		}

		m_csSRI.UnLock();
	}

	{
		m_csNPI.Lock();
		list<st_NetPlayBack_Info*>::iterator it = m_lstNPI.begin();
		while(it != m_lstNPI.end())
		{
			if ((*it) && (*it)->channel)
			{
				afk_device_s* _device = (afk_device_s*)(*it)->channel->get_device((*it)->channel);
				if (_device == device)
				{
					int nRet = Process_stopplayback(**it);
					if (nRet >= 0)
					{
						delete (*it);
					}
					m_lstNPI.erase(it++);
				}
				else
				{
					++it;
				}
			}
			else
			{
				++it;
			}
		}

		m_csNPI.UnLock();
	}

	{
		m_csDLI.Lock();
		list<st_DownLoad_Info*>::iterator it = m_lstDLI.begin();
		while(it != m_lstDLI.end())
		{
			if ((*it) && (*it)->channel)
			{
				afk_device_s* _device = (afk_device_s*)(*it)->channel->get_device((*it)->channel);
				if (_device == device)
				{
					(*it)->channel->close((*it)->channel);
					if ((*it)->file)
					{
						fclose((*it)->file);
						(*it)->file = 0;
					} 
					
					delete (*it);
					m_lstDLI.erase(it++);
				}
				else
				{
					++it;
				}
			}
			else
			{
				++it;
			}
		}

		m_csDLI.UnLock();
	}

	return nRet;
}

int __stdcall CSearchRecordAndPlayBack::QueryRecordFileInfoFunc(
												afk_handle_t object,	/* 数据提供者 */
												unsigned char *data,	/* 数据体 */
												unsigned int datalen,	/* 数据长度 */
												void *param,			/* 回调参数 */
												void *udata)
{
	int iRet = -1;
    receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata || false == receivedata->addRef())
    {
        return -1;
    }

	if (!receivedata->datalen || !receivedata->data)
	{
		SetEventEx(receivedata->hRecEvt);
        iRet = -1;
		goto END;
	}
	
	*receivedata->datalen = datalen/sizeof(afk_record_file_info_s);
    //缓冲区不够大
    if (datalen > (unsigned int)receivedata->maxlen)
    {
        SetEventEx(receivedata->hRecEvt);
        iRet = -1;
		goto END;
    }

    receivedata->result = 0;
    memcpy(receivedata->data, data, datalen);
    SetEventEx(receivedata->hRecEvt);
	iRet = 1;

END:
	receivedata->decRef();
	return iRet;
}

BOOL	CSearchRecordAndPlayBack::QueryRecordTime(LONG lLoginID, int nChannelId, int nRecordFileType, 
											LPNET_TIME tmStart, LPNET_TIME tmEnd, char* pchCardid,
											BOOL *bResult, int waittime)
{
	if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return FALSE;
    }
	
	if (NULL == tmStart || NULL == tmEnd || NULL == bResult)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return FALSE;
	}

	if (*tmStart >= *tmEnd)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return FALSE;
	}
	
	int nRet = -1;

    int filecount = 0;
    NET_RECORDFILE_INFO recordfiletemp[MAX_RECORD_NUM];
    int                 recordfilecounttemp = 0;

    afk_device_s *device = (afk_device_s*)lLoginID;

    afk_search_channel_param_s searchchannel = {0};
    searchchannel.no = searchchannel.queryrecord.ch = nChannelId;
    searchchannel.base.func = QueryRecordFileInfoFunc;
    searchchannel.queryrecord.starttime.year = tmStart->dwYear;
    searchchannel.queryrecord.starttime.month = tmStart->dwMonth;
    searchchannel.queryrecord.starttime.day = tmStart->dwDay;
    searchchannel.queryrecord.starttime.hour = tmStart->dwHour;
    searchchannel.queryrecord.starttime.minute = tmStart->dwMinute;
    searchchannel.queryrecord.starttime.second = tmStart->dwSecond;
    
    searchchannel.queryrecord.endtime.year = tmEnd->dwYear;
    searchchannel.queryrecord.endtime.month = tmEnd->dwMonth;
    searchchannel.queryrecord.endtime.day = tmEnd->dwDay;
    searchchannel.queryrecord.endtime.hour = tmEnd->dwHour;
    searchchannel.queryrecord.endtime.minute = tmEnd->dwMinute;
    searchchannel.queryrecord.endtime.second = tmEnd->dwSecond;
	if (4 == nRecordFileType) //卡号查询
	{
		if (!pchCardid || strlen(pchCardid) > 59) //暂限制为59字节
		{
			return NET_ILLEGAL_PARAM;
		}
		strcpy(searchchannel.queryrecord.cardid, pchCardid);
	}

    receivedata_s receivedata;
    receivedata.data = (char*)recordfiletemp;
    receivedata.datalen = &recordfilecounttemp;
    receivedata.maxlen = MAX_RECORD_NUM*sizeof(NET_RECORDFILE_INFO);
	//receivedata.hRecEvt = m_hRecEvent;
    receivedata.result = -1;

	searchchannel.base.udata = &receivedata;
    searchchannel.type = AFK_CHANNEL_SEARCH_RECORD;
    searchchannel.queryrecord.type = nRecordFileType;
    searchchannel.queryrecord.bytime = true;	//bytime

    unsigned long endDay = tmEnd->dwYear*10000 + tmEnd->dwMonth*100 + tmEnd->dwDay;
    unsigned long endTime = tmEnd->dwHour*10000 + tmEnd->dwMinute*100 + tmEnd->dwSecond;

    recordfilecounttemp = 0;
    receivedata.result = -1;

    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
    if (pchannel)
    {
		DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
		pchannel->close(pchannel);
		ResetEventEx(receivedata.hRecEvt);
		
		if (dwRet == WAIT_OBJECT_0)
		{
			if (receivedata.result == -1)
			{
				m_pManager->SetLastError(NET_RETURN_DATA_ERROR);
				return FALSE;
			}
			else
			{
				if (recordfilecounttemp == 0)
				{
					//无记录
					*bResult = FALSE;
					return TRUE;
				}
				
				LPNET_RECORDFILE_INFO ptemp = &recordfiletemp[0];
				unsigned long startDay = ptemp->starttime.dwYear*10000 + 
					ptemp->starttime.dwMonth*100 + ptemp->starttime.dwDay;
				unsigned long startTime = ptemp->starttime.dwHour*10000 +
					ptemp->starttime.dwMinute*100 + ptemp->starttime.dwSecond;

				if (endDay > startDay || (endDay == startDay && endTime > startTime)) 
				{
					//有记录
					*bResult = TRUE;
					return TRUE;
				}
			}
		} //if (dwRet == WAIT_OBJECT_0)
		else
		{
			m_pManager->SetLastError(NET_NETWORK_ERROR);
			return FALSE;
		}
    } //if (pchannel)
    else
    {
		m_pManager->SetLastError(NET_OPEN_CHANNEL_ERROR);
        return FALSE;
    }
	
	m_pManager->SetLastError(NET_ERROR);
    return FALSE;
}

/*
 *	对图片查询的能力级判断
 */
 BOOL	CSearchRecordAndPlayBack::SearchRecordProtocol(LONG lLoginID)
{
	 if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
	 {
		 m_pManager->SetLastError(NET_INVALID_HANDLE);
		 return FALSE;
	 }
	 
	 //查看能力集
	 BOOL bRecordProto = FALSE;
	 
	 afk_device_s *device = (afk_device_s*)lLoginID;
	 int nSpecial = 0;
	 device->get_info(device, dit_recordprotocol_type, &nSpecial);
	 if(-1 == nSpecial)	//没获取过
	 {
		 int nSp = 0;
		 char pBuf[sizeof(RecordEnable_T)] = {0};
		 int nBufSize = sizeof(RecordEnable_T);
		 int nLen = 0;
		 int nQueryResult = m_pManager->GetDevConfig().QuerySystemInfo(lLoginID, SYSTEM_INFO_PICTURE, pBuf, nBufSize, &nLen, 3000);
		 if (nQueryResult ==0 && nLen == sizeof(RecordEnable_T))
		 {
			 RecordEnable_T *pEnable_T= (RecordEnable_T *)pBuf;
			 if(0==nQueryResult && 1==pEnable_T->isSupportNewA5Query)
			 {
				 bRecordProto = TRUE;
				 nSp = SP_NEW_RECORDPROTOCOL;
				 device->set_info(device, dit_recordprotocol_type, &nSp);
			 }
			 else
			 {
				 device->set_info(device, dit_recordprotocol_type, &nSp);
			 }
		 }
	 }
	 else if(nSpecial == SP_NEW_RECORDPROTOCOL)
	 {
		 bRecordProto = TRUE;
	 }

	 return bRecordProto;
}

BOOL	CSearchRecordAndPlayBack::QueryRecordFile(LONG lLoginID, 
											int nChannelId, 
											int nRecordFileType, 
											LPNET_TIME time_start,
											LPNET_TIME time_end,
											char* cardid, 
											LPNET_RECORDFILE_INFO fileinfo,
											int maxlen,
											int *filecount,
											int waittime,
											BOOL bTime)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return FALSE;
    }

	if (!time_start || !time_end || !fileinfo || !filecount)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return FALSE;
	}

	if (*time_start >= *time_end)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return FALSE;
	}

	//add by cqs 2009.1.9
	//查询重要录像能力集
	bool bSupport = false;	
	int nRetLen = -1;
	int nret = -1;
	DEV_ENABLE_INFO stDevEn = {0};
	//查看能力
	nret = m_pManager->GetDevConfig().GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, 1000);
	if (nret >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_MARK_IMPORTANTRECORD] != 0)
		{
			bSupport = true;
		}
		else
		{
			bSupport = false;
		}
	}
	int nProtocol = 0;
	//对查询图片功能进行能力级判断
	if(8 == nRecordFileType)
	{
		if(SearchRecordProtocol(lLoginID))
		{
			nProtocol = 1;
		}
		else
		{
			return FALSE;
		}
	}
	if(9==nRecordFileType)
	{
		if(SearchRecordProtocol(lLoginID))
		{
			nProtocol = 1;
		}
	}

	int nRet = -1;

    *filecount = 0;
    NET_RECORDFILE_INFO recordfiletemp[MAX_RECORD_NUM];
    int                 recordfilecounttemp = 0;

    afk_device_s *device = (afk_device_s*)lLoginID;

    afk_search_channel_param_s searchchannel = {0};
    searchchannel.no = searchchannel.queryrecord.ch = nChannelId;
    searchchannel.base.func = QueryRecordFileInfoFunc;
    searchchannel.queryrecord.starttime.year = time_start->dwYear;
    searchchannel.queryrecord.starttime.month = time_start->dwMonth;
    searchchannel.queryrecord.starttime.day = time_start->dwDay;
    searchchannel.queryrecord.starttime.hour = time_start->dwHour;
    searchchannel.queryrecord.starttime.minute = time_start->dwMinute;
    searchchannel.queryrecord.starttime.second = time_start->dwSecond;
    
    searchchannel.queryrecord.endtime.year = time_end->dwYear;
    searchchannel.queryrecord.endtime.month = time_end->dwMonth;
    searchchannel.queryrecord.endtime.day = time_end->dwDay;
    searchchannel.queryrecord.endtime.hour = time_end->dwHour;
    searchchannel.queryrecord.endtime.minute = time_end->dwMinute;
    searchchannel.queryrecord.endtime.second = time_end->dwSecond;
	if (4 == nRecordFileType || 5 == nRecordFileType ||10 == nRecordFileType) //卡号查询，组合查询，按字段查询
	{
		if (!cardid || strlen(cardid) > 256) //暂限制为256字节
		{
			return NET_ILLEGAL_PARAM;
		}
		strcpy(searchchannel.queryrecord.cardid, cardid);
	}

	if (8 == nRecordFileType) //卡号查询图片(针对金桥网吧和上海公安)
	{
		if(cardid)
		{
			if(strlen(cardid) > 20) //卡号限制为20字节
			{
				return NET_ILLEGAL_PARAM;
			}
			
			strcpy(searchchannel.queryrecord.cardid, cardid);
		}
		else
		{
			strcpy(searchchannel.queryrecord.cardid, "");
		}
	}

    receivedata_s receivedata;
    receivedata.data = (char*)recordfiletemp;
    receivedata.datalen = &recordfilecounttemp;
    receivedata.maxlen = MAX_RECORD_NUM*sizeof(NET_RECORDFILE_INFO);
	//receivedata.hRecEvt = m_hRecEvent;
    receivedata.result = -1;

	searchchannel.base.udata = &receivedata;
    searchchannel.type = AFK_CHANNEL_SEARCH_RECORD;
    searchchannel.queryrecord.type = nRecordFileType;
    searchchannel.queryrecord.bytime = bTime?true:false;
	searchchannel.param = nProtocol;

	
    bool bFindEnd = false;
    unsigned long endDay = time_end->dwYear*10000 + time_end->dwMonth*100 + time_end->dwDay;
    unsigned long endTime = time_end->dwHour*10000 + time_end->dwMinute*100 + time_end->dwSecond;
	bool bFirstQueryrecord = true;//用于重要录像查询标识是否第一次查询
    while (!bFindEnd)
    {
        recordfilecounttemp = 0;
        receivedata.result = -1;
		//add 2009.1.9 cqs
		int nImportantRecord = 0;
		if (!bFirstQueryrecord && bSupport)//重要录像的非第一次查询标识
		{
			nImportantRecord = 1;	
		}
		else
		{
			nImportantRecord = 0;	
		}
		device->set_info(device, dit_firstqueryrecord_flag, &nImportantRecord);

        afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
            AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
        if (pchannel)
        {
			DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
			pchannel->close(pchannel);
			ResetEventEx(receivedata.hRecEvt);
			
			if (dwRet == WAIT_OBJECT_0)
			{
				if (receivedata.result == -1)
				{
					m_pManager->SetLastError(NET_RETURN_DATA_ERROR);
					return FALSE;
				}
				else
				{
					//如果查询完毕
					if (recordfilecounttemp == 0)
					{
						return TRUE;
					}

					if (recordfilecounttemp < 16)
					{
						bFindEnd = TRUE;
					}
					
					for (int i=0; i<recordfilecounttemp; i++)
					{
						LPNET_RECORDFILE_INFO ptemp = (LPNET_RECORDFILE_INFO)&recordfiletemp[i];
						unsigned long startDay = ptemp->starttime.dwYear*10000 + 
							ptemp->starttime.dwMonth*100 + ptemp->starttime.dwDay;
						unsigned long startTime = ptemp->starttime.dwHour*10000 +
							ptemp->starttime.dwMinute*100 + ptemp->starttime.dwSecond;
						if (endDay > startDay || (endDay == startDay && endTime > startTime))
						{
							//缓冲区不够
							if (maxlen >= (int)(sizeof(NET_RECORDFILE_INFO)*(*filecount + 1)))
							{
								memcpy(fileinfo + *filecount, ptemp, sizeof(NET_RECORDFILE_INFO));
								(*filecount)++;
							}
							else
							{
								m_pManager->SetLastError(NET_INSUFFICIENT_BUFFER);
								return TRUE; 
							}
						}
						else
						{
							bFindEnd = true;
							continue;
						}
						
						bFirstQueryrecord = false;//继续查询
						searchchannel.queryrecord.starttime.year = ptemp->endtime.dwYear;
						searchchannel.queryrecord.starttime.month = ptemp->endtime.dwMonth;
						searchchannel.queryrecord.starttime.day = ptemp->endtime.dwDay;
						searchchannel.queryrecord.starttime.hour = ptemp->endtime.dwHour;
						searchchannel.queryrecord.starttime.minute = ptemp->endtime.dwMinute;
						searchchannel.queryrecord.starttime.second = ptemp->endtime.dwSecond + 1;
					}
					continue;
				}
			} //if (dwRet == WAIT_OBJECT_0)
			else
			{
				m_pManager->SetLastError(NET_NETWORK_ERROR);
				return FALSE;
			}
        } //if (pchannel)
        else
        {
			m_pManager->SetLastError(NET_OPEN_CHANNEL_ERROR);
            return FALSE;
        }
    }
	
	m_pManager->SetLastError(NET_ERROR);
    return bFindEnd;
}

BOOL CSearchRecordAndPlayBack::QueryFurthestRecordTime(LONG lLoginID, int nRecordFileType, char *pchCardid, NET_FURTHEST_RECORD_TIME* pFurthrestTime, int nWaitTime)
{
	if(NULL == pFurthrestTime)
	{
		return FALSE;
	}

	if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return FALSE;
    }

	DEV_DISK_RECORD_INFO stuDiskRecordInfo = {0};
	int	nRetLen = 0;
	int nRet = m_pManager->GetDevConfig().QueryDevState(lLoginID, DEVSTATE_RECORD_TIME, (char *)&stuDiskRecordInfo, sizeof(DEV_DISK_RECORD_INFO), &nRetLen, nWaitTime);
	if(nRet <= 0 && nRetLen == sizeof(DEV_DISK_RECORD_INFO))
	{
		memset(pFurthrestTime, 0, sizeof(NET_FURTHEST_RECORD_TIME));

		afk_device_s *device = (afk_device_s *)lLoginID;
		int nChnCount = device->channelcount(device);
		pFurthrestTime->nChnCount = nChnCount;
		NET_RECORDFILE_INFO stuRecordInfo = {0};
		int nFileCount = 0;
		for(int i = 0; i < nChnCount; i++)
		{
			memset(&stuRecordInfo, 0 , sizeof(NET_RECORDFILE_INFO));
			BOOL bSuccess = QueryRecordFile(lLoginID,
											i, 
											nRecordFileType, 
											&stuDiskRecordInfo.stuBeginTime, 
											&stuDiskRecordInfo.stuEndTime, 
											pchCardid,
											&stuRecordInfo,
											sizeof(NET_RECORDFILE_INFO),
											&nFileCount,
											nWaitTime,
											FALSE);

			if(bSuccess && nFileCount > 0)// 成功
			{
				memcpy(&pFurthrestTime->stuFurthestTime[i], &stuRecordInfo.starttime, sizeof(NET_TIME));
			}
			else //认为没录像
			{
				memset(&pFurthrestTime->stuFurthestTime[i], 0, sizeof(NET_TIME));
			}
		}
		
		return TRUE;
	}

	return FALSE;
}


/************************************************************************
 ** 释放录像文件的句柄列表
 ***********************************************************************/
void CSearchRecordAndPlayBack::ReleaseRecordFileInfo(st_SearchRecord_Info& sr)
{
	list<NET_RECORDFILE_INFO*>::iterator it = sr.lstrf.begin();
	for(; it != sr.lstrf.end(); ++it)
	{
		delete (*it);
	}
	sr.lstrf.clear();
}

/************************************************************************
 ** 处理查找录像文件：返回值 <0 失败， 0 成功
 ***********************************************************************/
int CSearchRecordAndPlayBack::Process_QueryRecordfile(afk_device_s* device,
											int nChannelId, 
											int nRecordFileType, 
											LPNET_TIME time_start,
											LPNET_TIME time_end,
											char* cardid, 
											int waittime,
											BOOL bTime,
											list<NET_RECORDFILE_INFO*>& lstRecordFile)
{
    if (m_pManager->IsDeviceValid(device) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	if (!time_start || !time_end)
	{
		return NET_ILLEGAL_PARAM;
	}

	if (*time_start >= *time_end)
	{
		return NET_ILLEGAL_PARAM;
	}

	//add by cqs 2009.1.9
	//查询重要录像能力集
	BOOL bSupport = FALSE;	
	int nRetLen = -1;
	int nret = -1;
	DEV_ENABLE_INFO stDevEn = {0};
	//查看能力
	nret = m_pManager->GetDevConfig().GetDevFunctionInfo((LONG)device, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, 1000);
	if (nret >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_MARK_IMPORTANTRECORD] != 0)
		{
			bSupport = TRUE;
		}
		else
		{
			bSupport = FALSE;
		}
	}

	//对按卡号查询图片功能进行能力级判断
	if(8 == nRecordFileType)
	{
		if(!SearchRecordProtocol((LONG)device))
		{
			return FALSE;
		}
	}

	lstRecordFile.clear();

    NET_RECORDFILE_INFO recordfiletemp[MAX_RECORD_NUM];
    int                 recordfilecounttemp = 0;

    afk_search_channel_param_s searchchannel = {0};
    searchchannel.no = searchchannel.queryrecord.ch = nChannelId;
    searchchannel.base.func = QueryRecordFileInfoFunc;
    searchchannel.queryrecord.starttime.year = time_start->dwYear;
    searchchannel.queryrecord.starttime.month = time_start->dwMonth;
    searchchannel.queryrecord.starttime.day = time_start->dwDay;
    searchchannel.queryrecord.starttime.hour = time_start->dwHour;
    searchchannel.queryrecord.starttime.minute = time_start->dwMinute;
    searchchannel.queryrecord.starttime.second = time_start->dwSecond;
    
    searchchannel.queryrecord.endtime.year = time_end->dwYear;
    searchchannel.queryrecord.endtime.month = time_end->dwMonth;
    searchchannel.queryrecord.endtime.day = time_end->dwDay;
    searchchannel.queryrecord.endtime.hour = time_end->dwHour;
    searchchannel.queryrecord.endtime.minute = time_end->dwMinute;
    searchchannel.queryrecord.endtime.second = time_end->dwSecond;
	if (4 == nRecordFileType || 5 == nRecordFileType ||10 == nRecordFileType) //卡号查询
	{
		if (!cardid || strlen(cardid) > 256) //暂限制为256字节 
		{
			return NET_ILLEGAL_PARAM;
		}
		strcpy(searchchannel.queryrecord.cardid, cardid);
	}

	if (8 == nRecordFileType) //卡号查询图片(针对金桥网吧)
	{
		if(cardid)
		{
			if(strlen(cardid) > 20) //卡号限制为20字节
			{
				return NET_ILLEGAL_PARAM;
			}
			
			strcpy(searchchannel.queryrecord.cardid, cardid);
		}
		else
		{
			strcpy(searchchannel.queryrecord.cardid, "");
		}
	}
	
    receivedata_s receivedata;
    receivedata.data = (char*)recordfiletemp;
    receivedata.datalen = &recordfilecounttemp;
    receivedata.maxlen = MAX_RECORD_NUM*sizeof(NET_RECORDFILE_INFO);
	//receivedata.hRecEvt = m_hRecEvent;
    receivedata.result = -1;
	
    searchchannel.base.udata = &receivedata;
    searchchannel.type = AFK_CHANNEL_SEARCH_RECORD;
    searchchannel.queryrecord.type = nRecordFileType;
    searchchannel.queryrecord.bytime = bTime?true:false;

	int nRet = 0;
    BOOL bFindEnd = FALSE;
    unsigned long endDay = time_end->dwYear*10000 + time_end->dwMonth*100 + time_end->dwDay;
    unsigned long endTime = time_end->dwHour*10000 + time_end->dwMinute*100 + time_end->dwSecond;
	bool bFirstQueryrecord = TRUE;//用于重要录像查询标识是否第一次查询
    while (!bFindEnd)
    {
        recordfilecounttemp = 0;
        receivedata.result = -1;
		//add 2009.1.9 cqs
		int nImportantRecord = 0;
		if (!bFirstQueryrecord && bSupport)//重要录像的非第一次查询标识
		{
			nImportantRecord = 1;
		}
		else
		{
			nImportantRecord = 0;
		}
		device->set_info(device, dit_firstqueryrecord_flag, &nImportantRecord);

        afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
            AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
		/* 打开通道成功 */
        if (pchannel)
        {
			DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
			pchannel->close(pchannel);
			ResetEventEx(receivedata.hRecEvt);
			
			/* 查询数据返回 */
			if (dwRet == WAIT_OBJECT_0)
			{
				/* 返回数据失败 */
				if (receivedata.result == -1)
				{
					nRet = NET_RETURN_DATA_ERROR;
					bFindEnd = TRUE;
				}
				/* 成功返回数据：处理数据 */
				else
				{
					/* 查询完毕 */
					if (recordfilecounttemp == 0)
					{
						bFindEnd = TRUE;
					}

					if (recordfilecounttemp < 16)
					{
						bFindEnd = TRUE;
					}

					if (lstRecordFile.size() > 50000) // 异常处理
					{
						nRet = NET_RETURN_DATA_ERROR;
						bFindEnd = TRUE;
					}
					
					/* 处理数据 */
					for (int i=0; i<recordfilecounttemp; i++)
					{
						LPNET_RECORDFILE_INFO ptemp = (LPNET_RECORDFILE_INFO)&recordfiletemp[i];
						unsigned long startDay = ptemp->starttime.dwYear*10000 + 
							ptemp->starttime.dwMonth*100 + ptemp->starttime.dwDay;
						unsigned long startTime = ptemp->starttime.dwHour*10000 +
							ptemp->starttime.dwMinute*100 + ptemp->starttime.dwSecond;
						/* 在查询时间范围内：加载数据 */
						if (endDay > startDay ||
							(endDay == startDay && endTime > startTime))
						{
							NET_RECORDFILE_INFO* prf = new NET_RECORDFILE_INFO;
							/* 申请内存出错 */
							if (!prf)
							{
								nRet = NET_SYSTEM_ERROR;
								bFindEnd = TRUE;
							}
							else
							{
								memcpy(prf, ptemp, sizeof(NET_RECORDFILE_INFO));
								lstRecordFile.push_back(prf);
							}
						}
						else
						{
							/* 在查找时间范围之外：退出，返回成功 */
							bFindEnd = TRUE;
						}
						bFirstQueryrecord = FALSE;//继续查询

						searchchannel.queryrecord.starttime.year = ptemp->endtime.dwYear;
						searchchannel.queryrecord.starttime.month = ptemp->endtime.dwMonth;
						searchchannel.queryrecord.starttime.day = ptemp->endtime.dwDay;
						searchchannel.queryrecord.starttime.hour = ptemp->endtime.dwHour;
						searchchannel.queryrecord.starttime.minute = ptemp->endtime.dwMinute;
						searchchannel.queryrecord.starttime.second = ptemp->endtime.dwSecond + 1; 
					}// end of for (int i=0; i<recordfilecounttemp; i++)
				}/* 成功返回数据：处理数据 */
			}//end of if (dwRet == WAIT_OBJECT_0)
			else
			{
				/* 等待超时 */
				nRet = NET_NETWORK_ERROR;
				//	bResult = false;
				bFindEnd = TRUE;
			}
        }// end of if (pchannel)
        else
        {
		/* 打开通道失败 */
			nRet = NET_OPEN_CHANNEL_ERROR;
			bFindEnd = TRUE;
        }
    }// end of while (!bFindEnd)

	//重要录像进行排序
	if (bSupport)
	{
		SortRecordFileList(lstRecordFile);
	}
	
	if (nRet < 0)
	{
		st_SearchRecord_Info sr;
		sr.lstrf = lstRecordFile;
		ReleaseRecordFileInfo(sr);
		lstRecordFile.clear();
	}

	return nRet;
}

/************************************************************************
 ** 开始查找录像文件：返回值 0 失败， >0 查找句柄
 ***********************************************************************/
LONG CSearchRecordAndPlayBack::FindFile(LONG lLoginID,
								int nChannelId,
								int nRecordFileType, 
								char* cardid, 
								LPNET_TIME time_start,
								LPNET_TIME time_end,
								BOOL bTime, 
								int waittime)
{
    afk_device_s *device = (afk_device_s*)lLoginID;

	st_SearchRecord_Info * psr = new st_SearchRecord_Info;
	if (!psr)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		return 0;
	}
	psr->device = device;
	
	int r = Process_QueryRecordfile(device, nChannelId, nRecordFileType,
					time_start, time_end, cardid, waittime, bTime, psr->lstrf);

	if (r < 0)
	{
		if (psr)
		{
			delete psr;
		}
		m_pManager->SetLastError(r);
		return 0;
	}

	m_csSRI.Lock();
	m_lstSRI.push_back(psr);
	m_csSRI.UnLock();

	return (LONG)psr;
}

/************************************************************************
 ** 查找录像文件：返回值 -1 参数出错，0 录像文件信息数据取完，1 取回一条录像文件信息
 ***********************************************************************/
int	CSearchRecordAndPlayBack::FindNextFile(LONG lFindHandle,LPNET_RECORDFILE_INFO lpFindData)
{
	if (!lpFindData)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return -1;	//参数出错
	}
	else
	{
		memset(lpFindData, 0x00, sizeof(NET_RECORDFILE_INFO));
	}

	int iRet = 0;

	m_csSRI.Lock();
	list<st_SearchRecord_Info*>::iterator it = 
		find(m_lstSRI.begin(), m_lstSRI.end(), (st_SearchRecord_Info*)lFindHandle);

	/* 查找句柄有效 */
	if (it != m_lstSRI.end())
	{
		while (1)
		{
			size_t s = (*it)->lstrf.size();
			/* 还有录像文件信息数据 */
			if (s > 0)
			{
				NET_RECORDFILE_INFO* p = (*it)->lstrf.front();
				if (p)
				{
					memcpy(lpFindData, p, sizeof(NET_RECORDFILE_INFO));
					(*it)->lstrf.pop_front();
					delete p;
					iRet = 1;	//取回数据，返回成功
					goto END;
				}
				else
				{
					(*it)->lstrf.pop_front();
					continue;
				}
			}
			/* 录像文件信息数据取完 */
			else
			{
				iRet = 0;
				goto END;	//录像文件信息数据取完
			}
		}
	}
	/* 句柄无效 */
	else
	{
		m_pManager->SetLastError(NET_INVALID_HANDLE);
		iRet = -1; //参数出错
		goto END;
	}

END:
	m_csSRI.UnLock();

	return iRet;	//录像文件信息数据取完
}

/************************************************************************
 ** 结束录像文件查找：返回值 TRUE 成功， FALSE 失败
 ***********************************************************************/
int CSearchRecordAndPlayBack::FindClose(LONG lFindHandle)
{
	int iRet = 0;
	m_csSRI.Lock();
	list<st_SearchRecord_Info*>::iterator it = 
		find(m_lstSRI.begin(), m_lstSRI.end(), (st_SearchRecord_Info*)lFindHandle);

	if (it != m_lstSRI.end())
	{
		ReleaseRecordFileInfo(**it);
		delete (*it);
		m_lstSRI.erase(it);
	}
	else
	{
		m_pManager->SetLastError(NET_INVALID_HANDLE);
		iRet = -1;
	}

	m_csSRI.UnLock();
	return iRet;
}

int __stdcall CSearchRecordAndPlayBack::NetPlayBackCallBackFunc(
													afk_handle_t object,	/* 数据提供者 */
													unsigned char *data,	/* 数据体 */
													unsigned int datalen,	/* 数据长度 */
													void *param,			/* 回调参数 */
													void *udata
												)
{
	int nRet = -1;
    afk_channel_s *channel = (afk_channel_s*)object;
	st_NetPlayBack_Info* netplaybackinfo = (st_NetPlayBack_Info*)udata;
	if (channel == NULL || netplaybackinfo == NULL)
	{
		return nRet;
	}

	if (channel && netplaybackinfo && (netplaybackinfo->channel == channel))
	{
		if (datalen != -1)
        {
            netplaybackinfo->nReceiveSize += datalen;
            if (netplaybackinfo->PlayBack)
            {
                nRet = netplaybackinfo->PlayBack->AddData(data, datalen);
            }

            if ((unsigned int)param > 0 &&netplaybackinfo->nFrameRate != (unsigned int)param &&
                netplaybackinfo->Render)
            {
			//	播放ＳＤＫ自适应，不需要调用接口改帧率。
            //	netplaybackinfo->Render->SetFrameRate((unsigned int)param);
                netplaybackinfo->nFrameRate = (unsigned int)param;
            }
        }
        else
        {
			netplaybackinfo->ncurrf++;
			if (netplaybackinfo->prf && (netplaybackinfo->ncurrf < netplaybackinfo->nrflen))
			{
				NET_RECORDFILE_INFO* p = netplaybackinfo->prf+netplaybackinfo->ncurrf;
				afk_download_channel_param_s parm = {0};
				memcpy(&parm.info, netplaybackinfo->prf+netplaybackinfo->ncurrf, sizeof(NET_RECORDFILE_INFO));

				parm.nByTime = 1;
				parm.type = AFK_CHANNEL_DOWNLOAD_RECORD;
				channel->set_info(channel, 1, (void*)(&parm));
				if (netplaybackinfo->PlayBack)
				{
					netplaybackinfo->PlayBack->IsRePause();
				}
				nRet = 1;
			}
			else
			{
				netplaybackinfo->bDownLoadEnd = TRUE;
				nRet = 1;
			}
        }
	}

    return nRet;
}

bool __stdcall NetPlayBack_ReadDataPauseFunc(bool bPause, void *userdata)
{
    afk_channel_s *channel = (afk_channel_s*)userdata;
    return channel->pause(channel, bPause);
}

DWORD GetOffsetTimeByByte(const st_NetPlayBack_Info *pNetPlayBackInfo, unsigned int nOffsetByte, DWORD dwTotalTime)
{
	DWORD dwOffsetTime = 0;

	if (pNetPlayBackInfo->prf != NULL && pNetPlayBackInfo->nrflen > 0)
	{
		DWORD dwFileSize = 0;
		DWORD offset = 0;
		for (int i = 0; i < pNetPlayBackInfo->nrflen; i++)
		{
			dwFileSize += pNetPlayBackInfo->prf[i].size;
			offset = GetOffsetTime(pNetPlayBackInfo->prf[i].starttime, pNetPlayBackInfo->prf[i].endtime);
			if (nOffsetByte <= dwFileSize)
			{
				if (pNetPlayBackInfo->prf[i].size != 0)
				{
					dwOffsetTime += (pNetPlayBackInfo->prf[i].size + nOffsetByte - dwFileSize)*offset/pNetPlayBackInfo->prf[i].size;
				}
				break;
			}
			dwOffsetTime += offset;
		}
	}

	return dwOffsetTime;
}

void* WINAPI pbthreadproc(LPVOID pPara)
{
	st_NetPlayBack_Info *netplaybacktemp = (st_NetPlayBack_Info*)pPara;
	if (netplaybacktemp == NULL)
	{
		return (void*)0xFFFFFFFF;
	}

	BOOL bReceiveData = FALSE;		// 异常处理，如果设备一直没数据过来直接回调进度结束
	DWORD dwStartTime = GetTickCountEx();
	
	while(TRUE)
	{
		DWORD dwRet = WaitForSingleObjectEx(netplaybacktemp->hPBExit, 0);
		if (WAIT_OBJECT_0 == dwRet)
		{
			break;
		}

		int nStat = 1;
        unsigned char buffer[1024];
		int whileCount = 0;
        while (whileCount++ <= 10)
        {
            int readlen = netplaybacktemp->PlayBack->GetData(buffer, 1024);
            if (readlen > 0)
            {
				bReceiveData = TRUE;
				if (netplaybacktemp->Render)
                {
					BOOL bret = netplaybacktemp->Render->Play(buffer, readlen);
					if (bret) 
					{
						//数据回调函数
						if (netplaybacktemp->fNetDataCallBack)
						{
							int nret = netplaybacktemp->fNetDataCallBack((LONG)netplaybacktemp->channel, 0, buffer, readlen, netplaybacktemp->dwDataUser);
							/*
							if (nret > 0) 
							{
								//>0正常 
								netplaybacktemp->PlayBack->DecDataLength(readlen);
							}
							else if (0 == nret) 
							{
								//0-阻塞
								nStat = 0;
								break;
							}
							else
							{
								//<0系统出错
								nStat = -1;
								break;
							}
							*/
							netplaybacktemp->PlayBack->DecDataLength(readlen);
						}
						else
						{
							//无数据回调
							netplaybacktemp->PlayBack->DecDataLength(readlen);
						}
					}
                    else
					{
						nStat = 0;
						break;
					}
                }
				else if (netplaybacktemp->fNetDataCallBack)
				{
					int nret = netplaybacktemp->fNetDataCallBack((LONG)netplaybacktemp->channel, 0, buffer, readlen, netplaybacktemp->dwDataUser);
					if (nret > 0) 
					{
						//>0正常 
						netplaybacktemp->PlayBack->DecDataLength(readlen);
					}
					else if (0 == nret) 
					{
						//0-阻塞
						nStat = 0;
						break;
					}
					else
					{
						//<0系统出错
						nStat = -1;
						break;
					}
				}
				else
				{
					//没人接收数据了，扔掉
					netplaybacktemp->PlayBack->DecDataLength(readlen);
					break;
				}
            }//if (readlen > 0)
            else
            {
				nStat = 0; //block;
                break;
            }
        }

		DWORD dwPlayRemainBufLen = 0;
		if (netplaybacktemp->Render)
		{
			dwPlayRemainBufLen = netplaybacktemp->Render->GetSourceBufferRemain();
		}

		netplaybacktemp->nPlayBackSize = netplaybacktemp->nOffsetSize + (int)(netplaybacktemp->nReceiveSize - 
            netplaybacktemp->PlayBack->GetSize() - dwPlayRemainBufLen)/(int)1024;
		
        if (netplaybacktemp->pNetPlayBackPosCallBack)
        {
			if (netplaybacktemp->nPlayBackSize < netplaybacktemp->nTotalSize) 
			{
				if (0 == netplaybacktemp->nPlayBackType)
				{
					netplaybacktemp->pNetPlayBackPosCallBack((LONG)netplaybacktemp->channel, 
					netplaybacktemp->nTotalSize, netplaybacktemp->nPlayBackSize, netplaybacktemp->dwPosUser);
				}
				else if (1 == netplaybacktemp->nPlayBackType)
				{
					DWORD dwOffsetTime = GetOffsetTimeByByte(netplaybacktemp, netplaybacktemp->nPlayBackSize, netplaybacktemp->dwTotalTime);
					netplaybacktemp->pNetPlayBackPosCallBack((LONG)netplaybacktemp->channel, 
					netplaybacktemp->dwTotalTime, dwOffsetTime, netplaybacktemp->dwPosUser);
				}
			}
			//下载已结束
            if ((netplaybacktemp->bDownLoadEnd && netplaybacktemp->PlayBack->GetSize() <= 0) || (!bReceiveData && GetTickCountEx() - dwStartTime > 60000))
            {
				if (netplaybacktemp->Render) 
				{
					if (netplaybacktemp->Render->IsEmpty())
					{
						netplaybacktemp->nPlayBackSize = netplaybacktemp->nTotalSize;
						if (0 == netplaybacktemp->nPlayBackType)
						{
							netplaybacktemp->pNetPlayBackPosCallBack((LONG)netplaybacktemp->channel, 
							netplaybacktemp->nTotalSize, -1, netplaybacktemp->dwPosUser);
						}
						else if (1 == netplaybacktemp->nPlayBackType)
						{
							netplaybacktemp->pNetPlayBackPosCallBack((LONG)netplaybacktemp->channel, 
							netplaybacktemp->dwTotalTime, -1, netplaybacktemp->dwPosUser);
						}
						
						nStat = 0;
						break;
					}
				}
				else if (netplaybacktemp->fNetDataCallBack)
				{
					netplaybacktemp->nPlayBackSize = netplaybacktemp->nTotalSize;
					if (0 == netplaybacktemp->nPlayBackType)
					{
						netplaybacktemp->pNetPlayBackPosCallBack((LONG)netplaybacktemp->channel, 
						netplaybacktemp->nTotalSize, -1, netplaybacktemp->dwPosUser);
					}
					else if (1 == netplaybacktemp->nPlayBackType)
					{
						netplaybacktemp->pNetPlayBackPosCallBack((LONG)netplaybacktemp->channel, 
						netplaybacktemp->dwTotalTime, -1, netplaybacktemp->dwPosUser);
					}
					
					nStat = 0;
					break;
				}   
            }
        }

		if (0 == nStat)
		{
			Sleep(5);  //block
		}
		else if(nStat < 0)
		{
			break;
		}
	}

	return 0;
}

LONG CSearchRecordAndPlayBack::PlayBackByRecordFile(LONG lLoginID, 
									  LPNET_RECORDFILE_INFO lpRecordFile, 
									  HWND hWnd, fDownLoadPosCallBack cbDownLoadPos, DWORD dwUserData)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
    if (m_pManager->IsDeviceValid(device) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return 0;
    }

	if (lpRecordFile == NULL || hWnd == NULL)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return 0;
	}

	/* 先申请建立会话 */
	afk_connect_param_t stuConnParam = {0};
	stuConnParam.nConnType = channel_connect_tcp;
	stuConnParam.nInterfaceType = INTERFACE_PLAYBACK;
	int ret = m_pManager->GetDevConfigEx().SetupSession(lLoginID, lpRecordFile->ch, &stuConnParam);
	if (ret < 0)
	{
		m_pManager->SetLastError(ret);
		return 0;
	}

	/* 准备解码资源 */
	CVideoRender *pRender = m_pManager->GetRenderManager().GetRender(hWnd);
	if (-1 == (int)pRender)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		return 0; //系统出错
	}
	
	st_NetPlayBack_Info* ppb = NULL;
	afk_channel_s *channel = NULL;
	CNetPlayBack *pNetPlayBack = NULL;
	NET_RECORDFILE_INFO* myFile = NULL;

	if (pRender)
	{
		if (pRender->StartDec(true) < 0)
		{
			m_pManager->SetLastError(NET_DEC_OPEN_ERROR);
			goto e_clearup;
		}
	}

	//store record file information
	myFile = new NET_RECORDFILE_INFO;
	if (myFile == NULL)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}
	memcpy(myFile, lpRecordFile, sizeof(NET_RECORDFILE_INFO));

    // 增加网络回放列表
    ppb = new st_NetPlayBack_Info;
	if (ppb == NULL)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}

    ppb->channelid = lpRecordFile->ch;
    ppb->Render = pRender;
    ppb->PlayBack = NULL;
	ppb->channel = NULL;
	ppb->nPlayBackType = 0;
    ppb->nTotalSize = lpRecordFile->size;
    ppb->timeStart = lpRecordFile->starttime;
    ppb->timeEnd = lpRecordFile->endtime;
    ppb->nOffsetSize = 0;
    ppb->nReceiveSize = 0;
    ppb->nPlayBackSize = 0;
    ppb->bDownLoadEnd = FALSE;
    ppb->nFrameRate = 25;
    ppb->bAudioPlay = FALSE;
    ppb->pNetPlayBackPosCallBack = cbDownLoadPos;
    ppb->dwPosUser = dwUserData;
	ppb->fNetDataCallBack = NULL;
	ppb->dwDataUser = 0;
	ppb->pFileInfo = myFile;
	ppb->prf = NULL;
	ppb->nrflen = 0;
	ppb->ncurrf = 0;
	ppb->dwThreadID = 0;
	ppb->nConnectID = stuConnParam.nConnectID;

    //创建下载通道
    afk_download_channel_param_s parm;
	memset(&parm, 0, sizeof(afk_download_channel_param_s));
    parm.base.func = CSearchRecordAndPlayBack::NetPlayBackCallBackFunc;
    parm.base.udata = ppb;
	parm.conn = stuConnParam;
    memcpy(&parm.info, lpRecordFile, sizeof(NET_RECORDFILE_INFO));
	parm.info.ch = lpRecordFile->ch;
	parm.nByTime = 0;
	parm.nParam = 0; //回放
	parm.type = AFK_CHANNEL_DOWNLOAD_RECORD;

    channel = (afk_channel_s*)device->open_channel(device, AFK_CHANNEL_TYPE_DOWNLOAD, &parm);
    if (channel)
    {
        if (pRender)
        {
            pRender->SetDrawCallBack((void*)m_pManager->GetDrawFunc(), 
                (void*)device, (void*)channel, (void*)m_pManager->GetDrawCallBackUserData());
        }
		//创建网络回放缓冲
		NetPlayBack_CallBack netPlayBackCallBack;
		netPlayBackCallBack.ReadDataPauseFunc = NetPlayBack_ReadDataPauseFunc;
		netPlayBackCallBack.pUserData = channel;
		pNetPlayBack = new CNetPlayBack(netPlayBackCallBack);
		if (!pNetPlayBack)
		{
			m_pManager->SetLastError(NET_SYSTEM_ERROR);
			goto e_clearup;
		}

	    ppb->channel = channel;
	    ppb->PlayBack = pNetPlayBack;

		ret = CreateEventEx(ppb->hPBExit, FALSE, FALSE);
		if (ret < 0)
		{
			m_pManager->SetLastError(NET_SYSTEM_ERROR);
			goto e_clearup;
		}
		
		ret = CreateThreadEx(ppb->hThread, 0, (LPTHREAD_START_ROUTINE)pbthreadproc, (void*)ppb, /*CREATE_SUSPENDED*/0, &ppb->dwThreadID);
		if (ret < 0)
		{
			m_pManager->SetLastError(NET_SYSTEM_ERROR);
			goto e_clearup;
		}

		m_csNPI.Lock();
		m_lstNPI.push_back(ppb);
		m_csNPI.UnLock();
    }
    else
    {
		m_pManager->SetLastError(NET_OPEN_CHANNEL_ERROR);
		goto e_clearup;
    }

	return (LONG)channel;

e_clearup:
	if (ppb)
	{
		TerminateThreadEx(ppb->hThread, 0);
		CloseEventEx(ppb->hPBExit);
		CloseThreadEx(ppb->hThread);
		
		delete ppb;
		ppb = NULL;
	}

	if (channel)
	{
		channel->close(channel);
		channel = NULL;
	}

	if (pRender)
	{
		pRender->StopDec();
		m_pManager->GetRenderManager().ReleaseRender(pRender);
	}

	if (myFile)
	{
		delete myFile;
		myFile = NULL;
	}

	if (pNetPlayBack)
	{
		delete pNetPlayBack;
		pNetPlayBack = NULL;
	}

    return 0;
}

LONG	CSearchRecordAndPlayBack::PlayBackByRecordFileEx(LONG lLoginID, LPNET_RECORDFILE_INFO lpRecordFile, 
												   HWND hWnd, fDownLoadPosCallBack cbDownLoadPos, DWORD dwPosData, 
												   fDataCallBack fDownLoadDataCallBack, DWORD dwDataUser)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (m_pManager->IsDeviceValid(device) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return 0;
    }

	if(!lpRecordFile || (!hWnd && !fDownLoadDataCallBack))
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return 0;
	}

	/* 先申请建立会话 */
	afk_connect_param_t stuConnParam = {0};
	stuConnParam.nConnType = channel_connect_tcp;
	stuConnParam.nInterfaceType = INTERFACE_PLAYBACK;
	int ret = m_pManager->GetDevConfigEx().SetupSession(lLoginID, lpRecordFile->ch, &stuConnParam);
	if (ret < 0)
	{
		m_pManager->SetLastError(ret);
		return 0;
	}

	/* 准备解码资源 */
	CVideoRender *pRender = m_pManager->GetRenderManager().GetRender(hWnd);
	if (-1 == (int)pRender)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		return 0; //系统出错
	}

	st_NetPlayBack_Info* ppb = NULL;
	afk_channel_s *channel = NULL;
	CNetPlayBack *pNetPlayBack = NULL;
	NET_RECORDFILE_INFO* myFile = NULL;

	if (pRender)
	{
		if (pRender->StartDec(true) < 0)
		{
			m_pManager->SetLastError(NET_DEC_OPEN_ERROR);
			goto e_clearup;
		}
	}
	
	//store record file information
	myFile = new NET_RECORDFILE_INFO;
	if (myFile == NULL)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}
	memcpy(myFile, lpRecordFile, sizeof(NET_RECORDFILE_INFO));

    // 增加网络回放列表
    ppb = new st_NetPlayBack_Info;
	if (ppb == NULL)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}

    ppb->channelid = lpRecordFile->ch;
    ppb->Render = pRender;
    ppb->PlayBack = NULL;
	ppb->channel = NULL;
	ppb->nPlayBackType = 0;
    ppb->nTotalSize = lpRecordFile->size;
    ppb->timeStart = lpRecordFile->starttime;
    ppb->timeEnd = lpRecordFile->endtime;
    ppb->nOffsetSize = 0;
    ppb->nReceiveSize = 0;
    ppb->nPlayBackSize = 0;
    ppb->bDownLoadEnd = FALSE;
    ppb->nFrameRate = 25;
    ppb->bAudioPlay = FALSE;
    ppb->pNetPlayBackPosCallBack = cbDownLoadPos;
    ppb->dwPosUser = dwPosData;
	ppb->pFileInfo = myFile;
	ppb->prf = NULL;
	ppb->nrflen = 0;
	ppb->ncurrf = 0;
	ppb->fNetDataCallBack = fDownLoadDataCallBack;
	ppb->dwDataUser = dwDataUser;
	ppb->dwThreadID = 0;
	ppb->nConnectID = stuConnParam.nConnectID;
	
    //创建下载通道
    afk_download_channel_param_s parm;
	memset(&parm, 0, sizeof(afk_download_channel_param_s));
    parm.base.func = CSearchRecordAndPlayBack::NetPlayBackCallBackFunc;
    parm.base.udata = ppb;
	parm.conn = stuConnParam;
    memcpy(&parm.info, lpRecordFile, sizeof(NET_RECORDFILE_INFO));
	parm.info.ch = lpRecordFile->ch;
	ppb->pFileInfo->ch = parm.info.ch;
	parm.nByTime = 0;
	parm.nParam = 0; //回放
	parm.type = AFK_CHANNEL_DOWNLOAD_RECORD;

    channel = (afk_channel_s*)device->open_channel(device, AFK_CHANNEL_TYPE_DOWNLOAD, &parm);
    if (channel)
    {
        if (pRender)
        {
            pRender->SetDrawCallBack((void*)m_pManager->GetDrawFunc(), 
                (void*)device, (void*)channel, (void*)m_pManager->GetDrawCallBackUserData());
        }
		//创建网络回放缓冲
		NetPlayBack_CallBack netPlayBackCallBack;
		netPlayBackCallBack.ReadDataPauseFunc = NetPlayBack_ReadDataPauseFunc;
		netPlayBackCallBack.pUserData = channel;
		pNetPlayBack = new CNetPlayBack(netPlayBackCallBack);
		if (!pNetPlayBack)
		{
			m_pManager->SetLastError(NET_SYSTEM_ERROR);
			goto e_clearup;
		}

	    ppb->channel = channel;
	    ppb->PlayBack = pNetPlayBack;

		ret = CreateEventEx(ppb->hPBExit, FALSE, FALSE);
		if (ret < 0)
		{
			m_pManager->SetLastError(NET_SYSTEM_ERROR);
			goto e_clearup;
		}
		
		ret = CreateThreadEx(ppb->hThread, 0, (LPTHREAD_START_ROUTINE)pbthreadproc, (void*)ppb, /*CREATE_SUSPENDED*/0, &ppb->dwThreadID);
		if (ret < 0)
		{
			m_pManager->SetLastError(NET_SYSTEM_ERROR);
			goto e_clearup;
		}

		m_csNPI.Lock();
		m_lstNPI.push_back(ppb);
		m_csNPI.UnLock();
    }
    else
    {
		m_pManager->SetLastError(NET_OPEN_CHANNEL_ERROR);
		goto e_clearup;
    }

	return (LONG)channel;

e_clearup:
	if (ppb)
	{
		TerminateThreadEx(ppb->hThread, 0);
		CloseEventEx(ppb->hPBExit);
		CloseThreadEx(ppb->hThread);

		delete ppb;
		ppb = NULL;
	}
	
	if (channel)
	{
		channel->close(channel);
		channel = NULL;
	}

	if (pRender)
	{
		pRender->StopDec();
		m_pManager->GetRenderManager().ReleaseRender(pRender);
	}

	if (myFile)
	{
		delete myFile;
		myFile = NULL;
	}
	
	if (pNetPlayBack)
	{
		delete pNetPlayBack;
		pNetPlayBack = NULL;
	}

    return 0;
}


LONG CSearchRecordAndPlayBack::PlayBackByTime(LONG lLoginID, int nChannelID, LPNET_TIME lpStartTime, LPNET_TIME lpStopTIme, 
						fDownLoadPosCallBack cbDownLoadPos, DWORD dwPosUser, HWND hWnd)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (m_pManager->IsDeviceValid(device) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return 0;
    }

	if (!lpStartTime || !lpStopTIme || !hWnd)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return 0;
	}

	/* 先申请建立会话 */
	afk_connect_param_t stuConnParam = {0};
	stuConnParam.nConnType = channel_connect_tcp;
	stuConnParam.nInterfaceType = INTERFACE_PLAYBACK;
	int ret = m_pManager->GetDevConfigEx().SetupSession(lLoginID, nChannelID, &stuConnParam);
	if (ret < 0)
	{
		m_pManager->SetLastError(ret);
		return 0;
	}

	/* 再查询时间段内的文件列表 */
	list<NET_RECORDFILE_INFO*> lstrf;
	ret = Process_QueryRecordfile(device, nChannelID, 0/*全部类型*/,
					lpStartTime, lpStopTIme, 0, 3000, true, lstrf);
	if (ret < 0)
	{
		m_pManager->SetLastError(ret);
		return 0;
	}
	else if (lstrf.size() <= 0)
	{
		m_pManager->SetLastError(NET_NO_RECORD_FOUND);
		return 0;
	}

	st_NetPlayBack_Info* ppb = NULL;
	afk_channel_s *channel = NULL;
	CNetPlayBack *pNetPlayBack = NULL;
	NET_RECORDFILE_INFO* prf = NULL;
	CVideoRender *pRender = NULL;
	DWORD dwTotalSize = 0;
	DWORD dwTotalTime = 0;

	int nrflen = lstrf.size();
	prf = new NET_RECORDFILE_INFO[nrflen];
	if (prf == NULL)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		st_SearchRecord_Info sr;
		sr.lstrf = lstrf;
		ReleaseRecordFileInfo(sr);
		return 0;
	}

	list<NET_RECORDFILE_INFO*>::iterator it = lstrf.begin();
	for(int i=0; (it!=lstrf.end())&& (i<nrflen); it++,i++)
	{
		memcpy(prf+i, *it, sizeof(NET_RECORDFILE_INFO));
		dwTotalSize += prf[i].size;
		dwTotalTime += GetOffsetTime(prf[i].starttime, prf[i].endtime);
		
		delete *it;
	}

	lstrf.clear();

	/* 准备解码资源 */
	pRender = m_pManager->GetRenderManager().GetRender(hWnd);
	if (-1 == (int)pRender)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}

	if (pRender)
	{
		if (pRender->StartDec(true) < 0)
		{
			m_pManager->SetLastError(NET_DEC_OPEN_ERROR);
			goto e_clearup;
		}
	}

    //增加网络回放列表
    ppb = new st_NetPlayBack_Info;
	if (ppb == NULL)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}

    ppb->channelid = prf->ch;
    ppb->Render = pRender;
    ppb->PlayBack = 0;
	ppb->channel = 0;
	ppb->nPlayBackType = 1;
	ppb->dwTotalTime = dwTotalTime;
    ppb->nTotalSize = dwTotalSize;
    ppb->timeStart = *lpStartTime;
    ppb->timeEnd = *lpStopTIme;
    ppb->nOffsetSize = 0;
    ppb->nReceiveSize = 0;
    ppb->nPlayBackSize = 0;
    ppb->bDownLoadEnd = FALSE;
    ppb->nFrameRate = 25;
    ppb->bAudioPlay = FALSE;
    ppb->pNetPlayBackPosCallBack = cbDownLoadPos;
    ppb->dwPosUser = dwPosUser;
	ppb->fNetDataCallBack = 0;
	ppb->dwDataUser = 0;
	ppb->pFileInfo = 0;
	ppb->prf = prf;
	ppb->nrflen = nrflen;
	ppb->ncurrf = 0;
	ppb->dwThreadID = 0;
	ppb->nConnectID = stuConnParam.nConnectID;
	
    //创建下载通道
    afk_download_channel_param_s parm;
	memset(&parm, 0, sizeof(afk_download_channel_param_s));
    parm.base.func = CSearchRecordAndPlayBack::NetPlayBackCallBackFunc;
    parm.base.udata = ppb;
	parm.conn = stuConnParam;
    memcpy(&parm.info, prf, sizeof(NET_RECORDFILE_INFO));
	parm.info.ch = prf->ch;
	parm.nByTime = 1;
	parm.nParam = 0; //回放
	parm.type = AFK_CHANNEL_DOWNLOAD_RECORD;

    channel = (afk_channel_s*)device->open_channel(device, AFK_CHANNEL_TYPE_DOWNLOAD, &parm);
    if (channel)
    {
        if (pRender)
        {
            pRender->SetDrawCallBack((void*)m_pManager->GetDrawFunc(), 
                (void*)device, (void*)channel, (void*)m_pManager->GetDrawCallBackUserData());
        }
		//创建网络回放缓冲
		NetPlayBack_CallBack netPlayBackCallBack;
		netPlayBackCallBack.ReadDataPauseFunc = NetPlayBack_ReadDataPauseFunc;
		netPlayBackCallBack.pUserData = channel;
		pNetPlayBack = new CNetPlayBack(netPlayBackCallBack);
		if (!pNetPlayBack)
		{
			m_pManager->SetLastError(NET_SYSTEM_ERROR);
			goto e_clearup;
		}

	    ppb->channel = channel;
	    ppb->PlayBack = pNetPlayBack;

		ret = CreateEventEx(ppb->hPBExit, FALSE, FALSE);
		if (ret < 0)
		{
			m_pManager->SetLastError(NET_SYSTEM_ERROR);
			goto e_clearup;
		}
		
		ret = CreateThreadEx(ppb->hThread, 0, (LPTHREAD_START_ROUTINE)pbthreadproc, (void*)ppb, /*CREATE_SUSPENDED*/0, &ppb->dwThreadID);
		if (ret < 0)
		{
			m_pManager->SetLastError(NET_SYSTEM_ERROR);
			goto e_clearup;
		}

		m_csNPI.Lock();
		m_lstNPI.push_back(ppb);
		m_csNPI.UnLock();
    }
    else
    {
		m_pManager->SetLastError(NET_OPEN_CHANNEL_ERROR);
		goto e_clearup;
    }

	return (LONG)channel;

e_clearup:
	if (ppb)
	{
		TerminateThreadEx(ppb->hThread, 0);
		CloseEventEx(ppb->hPBExit);
		CloseThreadEx(ppb->hThread);

		delete ppb;
		ppb = NULL;
	}
	
	if (channel)
	{
		channel->close(channel);
		channel = NULL;
	}
	
	if (pRender)
	{
		pRender->StopDec();
		m_pManager->GetRenderManager().ReleaseRender(pRender);
	}
	
	if (prf)
	{
		delete[] prf;
		prf = NULL;
	}
	
	if (pNetPlayBack)
	{
		delete pNetPlayBack;
		pNetPlayBack = NULL;
	}
	
    return 0;
}

LONG	CSearchRecordAndPlayBack::PlayBackByTimeEx(LONG lLoginID, int nChannelID, LPNET_TIME lpStartTime, LPNET_TIME lpStopTIme, 
											 fDownLoadPosCallBack cbDownLoadPos, DWORD dwPosUser, HWND hWnd, 
											 fDataCallBack fDownLoadDataCallBack, DWORD dwDataUser)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (m_pManager->IsDeviceValid(device) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return 0;
    }

	if (!lpStartTime || !lpStopTIme || (!hWnd && !fDownLoadDataCallBack)) 
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return 0;
	}

	/* 先申请建立会话 */
	afk_connect_param_t stuConnParam = {0};
	stuConnParam.nConnType = channel_connect_tcp;
	stuConnParam.nInterfaceType = INTERFACE_PLAYBACK;
	int ret = m_pManager->GetDevConfigEx().SetupSession(lLoginID, nChannelID, &stuConnParam);
	if (ret < 0)
	{
		m_pManager->SetLastError(ret);
		return 0;
	}

	/* 再查询时间段内的文件列表 */
	list<NET_RECORDFILE_INFO*> lstrf;
	ret = Process_QueryRecordfile(device, nChannelID, 0/*全部类型*/,
					lpStartTime, lpStopTIme, 0, 3000, true, lstrf);
	if (ret < 0)
	{
		m_pManager->SetLastError(ret);
		return 0;
	}
	else if (lstrf.size() <= 0) 
	{
		m_pManager->SetLastError(NET_NO_RECORD_FOUND);
		return 0;
	}

	st_NetPlayBack_Info* ppb = NULL;
	afk_channel_s *channel = NULL;
	CNetPlayBack  *pNetPlayBack = NULL;
	NET_RECORDFILE_INFO* prf = NULL;
	CVideoRender *pRender = NULL;
	DWORD dwTotalSize = 0;
	DWORD dwTotalTime = 0;

	int nrflen = lstrf.size();
	prf = new NET_RECORDFILE_INFO[nrflen];
	if (!prf)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		st_SearchRecord_Info sr;
		sr.lstrf = lstrf;
		ReleaseRecordFileInfo(sr);
		return 0;
	}

	list<NET_RECORDFILE_INFO*>::iterator it = lstrf.begin();
	for(int i=0; (it!=lstrf.end())&& (i<nrflen); it++,i++)
	{
		memcpy(prf+i, *it, sizeof(NET_RECORDFILE_INFO));
		dwTotalSize += prf[i].size;
		dwTotalTime += GetOffsetTime(prf[i].starttime, prf[i].endtime);

		delete *it;
	}

	lstrf.clear();

	/* 准备解码资源 */
	pRender = m_pManager->GetRenderManager().GetRender(hWnd);
	if (-1 == (int)pRender)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}

	if (pRender)
	{
		if (pRender->StartDec(true) < 0)
		{
			m_pManager->SetLastError(NET_DEC_OPEN_ERROR);
			goto e_clearup;
		}
	}
	
    //增加网络回放列表
    ppb = new st_NetPlayBack_Info;
	if (!ppb)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}
	
    ppb->channelid = prf->ch;
    ppb->Render = pRender;
    ppb->PlayBack = 0;
	ppb->channel = 0;
	ppb->nPlayBackType = 1;
	ppb->dwTotalTime = dwTotalTime;
    ppb->nTotalSize = dwTotalSize;
    ppb->timeStart = *lpStartTime;
    ppb->timeEnd = *lpStopTIme;
    ppb->nOffsetSize = 0;
    ppb->nReceiveSize = 0;
    ppb->nPlayBackSize = 0;
    ppb->bDownLoadEnd = FALSE;
    ppb->nFrameRate = 25;
    ppb->bAudioPlay = FALSE;
    ppb->pNetPlayBackPosCallBack = cbDownLoadPos;
    ppb->dwPosUser = dwPosUser;
	ppb->fNetDataCallBack = /*pRender ? 0 : */fDownLoadDataCallBack;
	ppb->dwDataUser = dwDataUser;
	ppb->pFileInfo = 0;
	ppb->prf = prf;
	ppb->nrflen = nrflen;
	ppb->ncurrf = 0;
	ppb->dwThreadID = 0;
	ppb->nConnectID = stuConnParam.nConnectID;
	
    //创建下载通道
    afk_download_channel_param_s parm;
	memset(&parm, 0, sizeof(afk_download_channel_param_s));
    parm.base.func = CSearchRecordAndPlayBack::NetPlayBackCallBackFunc;
    parm.base.udata = ppb;
	parm.conn = stuConnParam;
    memcpy(&parm.info, prf, sizeof(NET_RECORDFILE_INFO));
	parm.info.ch = prf->ch;
	parm.nByTime = 1;
	parm.nParam = 0; //回放
	parm.type = AFK_CHANNEL_DOWNLOAD_RECORD;

    channel = (afk_channel_s*)device->open_channel(device, AFK_CHANNEL_TYPE_DOWNLOAD, &parm);
    if (channel)
    {
        if (pRender)
        {
            pRender->SetDrawCallBack((void*)m_pManager->GetDrawFunc(), 
                (void*)device, (void*)channel, (void*)m_pManager->GetDrawCallBackUserData());
        }
		//创建网络回放缓冲
		NetPlayBack_CallBack netPlayBackCallBack;
		netPlayBackCallBack.ReadDataPauseFunc = NetPlayBack_ReadDataPauseFunc;
		netPlayBackCallBack.pUserData = channel;
		pNetPlayBack = new CNetPlayBack(netPlayBackCallBack);
		if (!pNetPlayBack)
		{
			m_pManager->SetLastError(NET_SYSTEM_ERROR);
			goto e_clearup;
		}

	    ppb->channel = channel;
	    ppb->PlayBack = pNetPlayBack;

		ret = CreateEventEx(ppb->hPBExit, FALSE, FALSE);
		if (ret < 0)
		{
			m_pManager->SetLastError(NET_SYSTEM_ERROR);
			goto e_clearup;
		}
		
		ret = CreateThreadEx(ppb->hThread, 0, (LPTHREAD_START_ROUTINE)pbthreadproc, (void*)ppb, /*CREATE_SUSPENDED*/0, &ppb->dwThreadID);
		if (ret < 0)
		{
			m_pManager->SetLastError(NET_SYSTEM_ERROR);
			goto e_clearup;
		}
		
		m_csNPI.Lock();
		m_lstNPI.push_back(ppb);
		m_csNPI.UnLock();
    }
    else
    {
		m_pManager->SetLastError(NET_OPEN_CHANNEL_ERROR);
		goto e_clearup;
    }

	return (LONG)channel;

e_clearup:
	if (ppb)
	{
		TerminateThreadEx(ppb->hThread, 0);
		CloseEventEx(ppb->hPBExit);
		CloseThreadEx(ppb->hThread);

		delete ppb;
		ppb = NULL;
	}

	if (channel)
	{
		channel->close(channel);
		channel = NULL;
	}

	if (pRender)
	{
		pRender->StopDec();
		m_pManager->GetRenderManager().ReleaseRender(pRender);
	}

	if (prf)
	{
		delete[] prf;
		prf = NULL;
	}

	if (pNetPlayBack)
	{
		delete pNetPlayBack;
		pNetPlayBack = NULL;
	}

    return 0;
}

st_NetPlayBack_Info* CSearchRecordAndPlayBack::GetNetPlayBackInfo(LONG lPlayHandle)
{
	st_NetPlayBack_Info* p = NULL;

	list<st_NetPlayBack_Info*>::iterator it = 
		find_if(m_lstNPI.begin(),m_lstNPI.end(),SearchNPIbyChannel(lPlayHandle));

	if (it != m_lstNPI.end())
	{
		p = (*it);
	}

	return p;
}

int CSearchRecordAndPlayBack::PausePlayBack(LONG lPlayHandle, BOOL bPause)
{
	int nRet = -1;

	m_csNPI.Lock();

	st_NetPlayBack_Info* pNPI = GetNetPlayBackInfo(lPlayHandle);
	if (pNPI)
	{
		if (pNPI->Render) 
		{
			BOOL b = pNPI->Render->Pause(bPause == TRUE);
			if (b)
			{
				nRet = 0;
			}
			else
			{
				nRet = NET_RENDER_PAUSE_ERROR;
			}
		}
		else
		{
			bool b = pNPI->channel->pause(pNPI->channel, bPause?true:false);
			if (b)
			{
				nRet = 0;
			}
			else
			{
				nRet = NET_RENDER_PAUSE_ERROR;
			}
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}
	
	m_csNPI.UnLock();

	return nRet;
}

/* parameter dwOffsetSize for output size */
int GetSeekFileBySeekTime(const st_NetPlayBack_Info& npi, unsigned int offsettime, DWORD& dwOffsetSize, DWORD& dwOffsetTime)
{
	int nFileID = 0;

	if (npi.prf && npi.nrflen)
	{
		DWORD dwTotalTime = 0;
		for (int i = 0; i < npi.nrflen; i++)
		{
			DWORD offset = GetOffsetTime(npi.prf[i].starttime, npi.prf[i].endtime);
			dwTotalTime += offset;
			if (offsettime <= dwTotalTime)
			{
				dwOffsetTime = offset + offsettime - dwTotalTime;
				if (offset != 0)
				{
					dwOffsetSize += (DWORD)(dwOffsetTime*1.0/offset*npi.prf[i].size);
				}
				nFileID = i;
				break;
			}

			dwOffsetSize += npi.prf[i].size;
		}
	}

	return nFileID;
}
 
int CSearchRecordAndPlayBack::SeekPlayBack(LONG lPlayHandle, unsigned int offsettime, unsigned int offsetbyte)
{
	int nRet = NET_NOERROR;
	
	m_csNPI.Lock();
	
	st_NetPlayBack_Info* pNPI = GetNetPlayBackInfo(lPlayHandle);
	if (pNPI)
	{
		if (INVALID_OFFSET_TIME != offsettime && pNPI->prf && pNPI->nrflen)
		{
			DWORD dwOffsetSize = 0;	// 拖动位置的文件偏移大小
			DWORD dwOffsetTime = 0; // 定位后的文件偏移时间
			int nSeekrf = GetSeekFileBySeekTime(*pNPI, offsettime, dwOffsetSize, dwOffsetTime);
			if (nSeekrf >= 0)
			{
				if (nSeekrf != pNPI->ncurrf) // 要定位的文件不是当前的文件，打开此文件
				{
					NET_RECORDFILE_INFO* p = pNPI->prf+nSeekrf;
					
					afk_download_channel_param_s parm = {0};
					memcpy(&parm.info, pNPI->prf+nSeekrf, sizeof(NET_RECORDFILE_INFO));

					parm.type = AFK_CHANNEL_DOWNLOAD_RECORD;
					parm.nByTime = 1;
					int r = pNPI->channel->set_info(pNPI->channel, 1, (void*)(&parm));
					if (r == 0)
					{
						nRet = NET_NETWORK_ERROR;
					}
					else
					{
						pNPI->ncurrf = nSeekrf;
					}						
				}
				else
				{
					// 要定位的文件就是当前文件
					if (pNPI->bDownLoadEnd) 
					{
						// 已结束，重新打开文件
						afk_download_channel_param_s parm = {0};
						memcpy(&parm.info, pNPI->prf+pNPI->ncurrf, sizeof(NET_RECORDFILE_INFO));
						parm.nByTime = 0;
						parm.type = AFK_CHANNEL_DOWNLOAD_RECORD;
						
						int r = pNPI->channel->set_info(pNPI->channel, 1, (void*)(&parm));
						if (r == 0)
						{
							nRet = NET_NETWORK_ERROR;
						}
					}
				}
			}
			else
			{
				nRet = NET_ILLEGAL_PARAM;
			}

			if (nRet == NET_NOERROR)
			{
				// 按时间定位
				afk_download_control_param_s parm = {0};
				pNPI->PlayBack->Pause(BUFFER_RESET);
				//Sleep(100);
				parm.offsettime = dwOffsetTime;
				parm.offsetdata = INVAlID_OFFSET_BYTE;
				int r = pNPI->channel->set_info(pNPI->channel, 0, &parm);
				if (r == 0)
				{
					nRet = NET_NETWORK_ERROR;
				}
				else
				{
					pNPI->bDownLoadEnd = FALSE;
					if (pNPI->PlayBack)
					{
						pNPI->PlayBack->Reset();
					}
					if (pNPI->Render)
					{
						pNPI->Render->Reset();
					}
					pNPI->nOffsetSize = dwOffsetSize;
					pNPI->nReceiveSize = 0;
					
					nRet = 0;
				}
				
				pNPI->PlayBack->Resume(BUFFER_RESET);
			}
		}
		else if (INVAlID_OFFSET_BYTE != offsetbyte && !pNPI->nrflen)
		{
			if (pNPI->bDownLoadEnd) 
			{
				// 已结束，重新打开文件
				afk_download_channel_param_s parm = {0};
				memcpy(&parm.info, pNPI->pFileInfo, sizeof(NET_RECORDFILE_INFO));
				parm.nByTime = 0;
				parm.type = AFK_CHANNEL_DOWNLOAD_RECORD;

				int r = pNPI->channel->set_info(pNPI->channel, 1, (void*)(&parm));
				if (r == 0)
				{
					nRet = NET_NETWORK_ERROR;
				}
			}
			pNPI->PlayBack->Pause(BUFFER_RESET);
			//Sleep(100);
			afk_download_control_param_s parm = {0};
			parm.offsettime = INVALID_OFFSET_TIME;
			parm.offsetdata = offsetbyte;
			int r = pNPI->channel->set_info(pNPI->channel, 0, &parm);
			if (r == 0)
			{
				nRet = NET_NETWORK_ERROR;
			}
			else
			{
				pNPI->bDownLoadEnd = FALSE;

				if (pNPI->Render)
				{
					pNPI->Render->Reset();
				}
				if (pNPI->PlayBack)
				{
					pNPI->PlayBack->Reset();
				}
				
				pNPI->nOffsetSize = offsetbyte;
				pNPI->nReceiveSize = 0;
				nRet = 0;
			}
			pNPI->PlayBack->Resume(BUFFER_RESET);
		}
		else
		{
			nRet = NET_ILLEGAL_PARAM;
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}
	
	m_csNPI.UnLock();
	
	return nRet;
}

int CSearchRecordAndPlayBack::StopPlayBack(LONG lPlayHandle)
{	
	int nRet = NET_NOERROR;
	
	m_csNPI.Lock();
	st_NetPlayBack_Info* pNPI = GetNetPlayBackInfo(lPlayHandle);
	if (pNPI)
	{
		nRet = Process_stopplayback(*pNPI);
		if (nRet >= 0)
		{
			delete pNPI;
			m_lstNPI.remove(pNPI);
			nRet = NET_NOERROR;
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}

	m_csNPI.UnLock();
	return nRet;
}

int CSearchRecordAndPlayBack::StepPlayBack(LONG lPlayHandle, BOOL bStop)
{
	int nRet = NET_ERROR;

	m_csNPI.Lock();

	st_NetPlayBack_Info* pNPI = GetNetPlayBackInfo(lPlayHandle);
	if (pNPI)
	{
		if (pNPI->Render) 
		{
			BOOL b = pNPI->Render->Step(bStop);
			if (b)
			{
				nRet = NET_NOERROR; //success
			}
			else
			{
				nRet = NET_RENDER_STEP_ERROR;
			}
		}
		else
		{
			nRet = NET_SYSTEM_ERROR;
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}
	
	m_csNPI.UnLock();
	return nRet;
}

int	CSearchRecordAndPlayBack::FastPlayBack(LONG lPlayHandle)
{
	int nRet = NET_ERROR;
	m_csNPI.Lock();
	
	st_NetPlayBack_Info* pNPI = GetNetPlayBackInfo(lPlayHandle);
	if (pNPI)
	{
		if (pNPI->Render) 
		{
			BOOL b = pNPI->Render->Fast();
			if (b)
			{
				nRet = NET_NOERROR; //success
			}
			else
			{
				nRet = NET_RENDER_FRAMERATE_ERROR;
			}
		}
		else
		{
			nRet = NET_SYSTEM_ERROR;
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}
	
	m_csNPI.UnLock();
	return nRet;
}

int CSearchRecordAndPlayBack::SlowPlayBack(LONG lPlayHandle)
{
	int nRet = NET_ERROR;
	m_csNPI.Lock();

	st_NetPlayBack_Info* pNPI = GetNetPlayBackInfo(lPlayHandle);
	if (pNPI)
	{
		if (pNPI->Render) 
		{
			BOOL b = pNPI->Render->Slow();
			if (b)
			{
				nRet = NET_NOERROR; //success
			}
			else
			{
				nRet = NET_RENDER_FRAMERATE_ERROR;
			}
		}
		else
		{
			nRet = NET_SYSTEM_ERROR;
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}
	
	m_csNPI.UnLock();
	return nRet;
}

int	CSearchRecordAndPlayBack::NormalPlayBack(LONG lPlayHandle)
{
	int nRet = NET_ERROR;
	m_csNPI.Lock();
	
	st_NetPlayBack_Info* pNPI = GetNetPlayBackInfo(lPlayHandle);
	if (pNPI)
	{
		if (pNPI->Render) 
		{
			BOOL b = pNPI->Render->PlayNormal();
			if (b)
			{
				nRet = NET_NOERROR; //success
			}
			else
			{
				nRet = NET_RENDER_FRAMERATE_ERROR;
			}
		}
		else
		{
			nRet = NET_SYSTEM_ERROR;
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}
	
	m_csNPI.UnLock();
	return nRet;
}

int CSearchRecordAndPlayBack::SetFramePlayBack(LONG lPlayHandle, int framerate)
{
	int nRet = NET_ERROR;
	m_csNPI.Lock();
	
	st_NetPlayBack_Info* pNPI = GetNetPlayBackInfo(lPlayHandle);
	if (pNPI)
	{
		if (pNPI->Render)
		{
			BOOL b = pNPI->Render->SetFrameRate(framerate);
			if (b)
			{
				nRet = NET_NOERROR; //success
			}
			else
			{
				nRet = NET_RENDER_FRAMERATE_ERROR;
			}
		}
		else
		{
			nRet = NET_SYSTEM_ERROR;
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}
	
	m_csNPI.UnLock();
	return nRet;
}

int CSearchRecordAndPlayBack::GetFramePlayBack(LONG lPlayHandle, int *fileframerate, int *playframerate)
{
	if (!fileframerate || !playframerate)
	{
		return NET_ILLEGAL_PARAM;
	}
	else
	{
		*fileframerate = -1;
		*playframerate = -1;
	}

	int nRet = -1;
	m_csNPI.Lock();
	
	st_NetPlayBack_Info* pNPI = GetNetPlayBackInfo(lPlayHandle);
	if (pNPI && pNPI->Render)
	{
		*playframerate = pNPI->Render->GetFrameRate();
        *fileframerate = pNPI->channel->get_info(pNPI->channel, 0, 0);
		nRet = 0;
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}
	
	m_csNPI.UnLock();
	return nRet;
}

int CSearchRecordAndPlayBack::GetPlayBackOsdTime(LONG lPlayHandle, LPNET_TIME lpOsdTime, LPNET_TIME lpStartTime, LPNET_TIME lpEndTime)
{
	if (!lpOsdTime || !lpStartTime || !lpEndTime)
	{
		return NET_ILLEGAL_PARAM;
	}
	else
	{
		memset(lpOsdTime,0x00,sizeof(NET_TIME));
	}

	int nRet = -1;
	m_csNPI.Lock();
	
	st_NetPlayBack_Info* pNPI = GetNetPlayBackInfo(lPlayHandle);
	if (pNPI)
	{
		if (pNPI->Render)
		{
			pNPI->Render->GetOSDTime(&lpOsdTime->dwYear, &lpOsdTime->dwMonth, 
					&lpOsdTime->dwDay, &lpOsdTime->dwHour, 
					&lpOsdTime->dwMinute, &lpOsdTime->dwSecond);
			*lpStartTime = pNPI->timeStart;
			*lpEndTime = pNPI->timeEnd;
			nRet = 0;
		}
        else
		{
			nRet = NET_SYSTEM_ERROR;
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}
	
	m_csNPI.UnLock();
	return nRet;
}

int __stdcall DownLoadRecordFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata
)
{
    afk_channel_s *channel = (afk_channel_s*)object;
    if (!channel)
	{
		return -1;
	}

	st_DownLoad_Info* pDLI = (st_DownLoad_Info*)udata;
	if (!pDLI)
	{
		return -1;
	}
	
    if (pDLI->file)
    {
        if (datalen != -1)
        {
			if (data)
			{
				if (0 == fwrite(data, datalen, 1, pDLI->file))
				{
#ifdef WIN32
					if (ERROR_DISK_FULL == GetLastError())
#else	//linux
#endif
					{
						if (pDLI->prf) 
						{
							if (pDLI->pTimeDownLoadPosCallBack) 
							{
								pDLI->pTimeDownLoadPosCallBack((LONG)channel,
									pDLI->nTotalSize, -2, pDLI->ncurrf, *(pDLI->prf+pDLI->ncurrf), pDLI->userdata);
							}
						}
						else
						{
							if (pDLI->pDownLoadPosCallBack)
							{
								pDLI->pDownLoadPosCallBack((LONG)channel, 
									pDLI->nTotalSize, -2, pDLI->userdata);
							}
						}
					}
				}
				pDLI->fileflushflag++;
				if (pDLI->fileflushflag%40 == 0)
				{
					fflush(pDLI->file);
				}
				pDLI->nDownLoadSize += datalen;
				
				if (pDLI->prf) 
				{
					if (pDLI->pTimeDownLoadPosCallBack) 
					{
						pDLI->pTimeDownLoadPosCallBack((LONG)channel,
							pDLI->nTotalSize, pDLI->nDownLoadSize/1024, pDLI->ncurrf, *(pDLI->prf+pDLI->ncurrf), pDLI->userdata);
					}
				}
				else
				{
					if (pDLI->pDownLoadPosCallBack)
					{
						pDLI->pDownLoadPosCallBack((LONG)channel, 
							pDLI->nTotalSize, pDLI->nDownLoadSize/1024, pDLI->userdata);
					}
				}
			}
        }
        else
        {
			pDLI->ncurrf++;
			if (pDLI->prf && (pDLI->ncurrf < pDLI->nrflen))
			{
				NET_RECORDFILE_INFO* p = pDLI->prf+pDLI->ncurrf;
				afk_download_channel_param_s parm = {0};
				memcpy(&parm.info, pDLI->prf+pDLI->ncurrf, sizeof(NET_RECORDFILE_INFO));

				parm.type = AFK_CHANNEL_DOWNLOAD_RECORD;
				parm.nByTime = 1;
				channel->set_info(channel, 1, (void*)(&parm));
			}
			else
			{
				fclose(pDLI->file);
				pDLI->file = 0;
				pDLI->nDownLoadSize = -1;

				if (pDLI->prf) 
				{
					if (pDLI->pTimeDownLoadPosCallBack) 
					{
						pDLI->pTimeDownLoadPosCallBack((LONG)channel,
							pDLI->nTotalSize, -1, pDLI->ncurrf, *(pDLI->prf+pDLI->ncurrf), pDLI->userdata);
					}
				}
				else
				{
					if (pDLI->pDownLoadPosCallBack)
					{
#ifdef _DEBUG
						OutputDebugString("CLIENT_DownloadByRecordFile: end!\n");
#endif
						pDLI->pDownLoadPosCallBack((LONG)channel, 
							pDLI->nTotalSize, -1, pDLI->userdata);
					}
				}
			}
        }
    }//end of if (pDLI->file)
	
	return 1;
}

LONG CSearchRecordAndPlayBack::DownloadByRecordFile(LONG lLoginID,LPNET_RECORDFILE_INFO lpRecordFile, char *sSavedFileName, 
                                      fDownLoadPosCallBack cbDownLoadPos, DWORD dwUserData)
{
	if (!lpRecordFile || !sSavedFileName)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return 0;
	}

    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return 0;
    }

	/* 先申请建立会话 */
	afk_connect_param_t stuConnParam = {0};
	stuConnParam.nConnType = channel_connect_tcp;
	stuConnParam.nInterfaceType = INTERFACE_DOWNLOAD;
	int ret = m_pManager->GetDevConfigEx().SetupSession(lLoginID, lpRecordFile->ch, &stuConnParam);
	if (ret < 0)
	{
		m_pManager->SetLastError(ret);
		return 0;
	}

    afk_device_s *device = (afk_device_s*)lLoginID;
	afk_channel_s *channel = 0;
    st_DownLoad_Info* pDLI = new st_DownLoad_Info;
	if (!pDLI)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}
	
    pDLI->channel = 0;
    pDLI->channelid = lpRecordFile->ch;
    pDLI->file = fopen(sSavedFileName, "wb");
	if (!pDLI->file)
	{
		m_pManager->SetLastError(NET_OPEN_FILE_ERROR);
		goto e_clearup;
	}
	if (device->device_type(device) != NET_NB_SERIAL) 
	{
		/*WriteVideoFileHeader(pDLI->file, device->device_type(device), 25, 
			lpRecordFile->starttime.dwYear, 
			lpRecordFile->starttime.dwMonth, 
			lpRecordFile->starttime.dwDay, 
			lpRecordFile->starttime.dwHour,
			lpRecordFile->starttime.dwMinute, 
			lpRecordFile->starttime.dwSecond);*/
	}

	pDLI->fileflushflag = 0;
	pDLI->nTotalSize = lpRecordFile->size;
	pDLI->nDownLoadSize = 0;
	pDLI->pDownLoadPosCallBack = cbDownLoadPos;
	pDLI->pTimeDownLoadPosCallBack = 0;
	pDLI->userdata = dwUserData;
	memset (&(pDLI->timeStart), 0 , sizeof(NET_TIME));
	memset (&(pDLI->timeEnd), 0 , sizeof(NET_TIME));
	pDLI->prf = 0;
	pDLI->nrflen = 0;
	pDLI->ncurrf = 0;
	pDLI->nConnectID = stuConnParam.nConnectID;

    afk_download_channel_param_s parm;
	memset(&parm, 0, sizeof(afk_download_channel_param_s));
    parm.base.func = DownLoadRecordFunc;
    parm.base.udata = pDLI;
	parm.conn = stuConnParam;
    memcpy(&parm.info, lpRecordFile, sizeof(NET_RECORDFILE_INFO));
	parm.nByTime = 0;
	parm.nParam = 1; //下载
	parm.type = AFK_CHANNEL_DOWNLOAD_RECORD;

    channel = (afk_channel_s*)device->open_channel(device, AFK_CHANNEL_TYPE_DOWNLOAD, &parm);
    if (channel)
    {
		pDLI->channel = channel;
		m_csDLI.Lock();
		m_lstDLI.push_back(pDLI);
		m_csDLI.UnLock();
    }
	else
	{
		m_pManager->SetLastError(NET_OPEN_CHANNEL_ERROR);
		goto e_clearup;
	}

    return (LONG)channel;

e_clearup:

	if (pDLI)
	{
		if (pDLI->file)
		{
			fclose(pDLI->file);
			pDLI->file = 0;
		}
		delete pDLI;
		pDLI = 0;
	}

	if (channel)
	{
		channel->close(channel);
		channel = 0;
	}

    return 0;
}

st_DownLoad_Info* CSearchRecordAndPlayBack::GetDownLoadInfo(LONG lFileHandle)
{
	st_DownLoad_Info* p = 0;
	list<st_DownLoad_Info*>::iterator it =
		find_if(m_lstDLI.begin(),m_lstDLI.end(),SearchDLIbyChannel(lFileHandle));

	if (it != m_lstDLI.end())
	{
		p = (*it);
	}
	
	return p;
}

int CSearchRecordAndPlayBack::StopDownload(LONG lFileHandle)
{
	int nRet = -1;

	m_csDLI.Lock();
	st_DownLoad_Info * pDLI = GetDownLoadInfo(lFileHandle);
	if (pDLI)
	{
		LONG lLoginID = (LONG)pDLI->channel->get_device(pDLI->channel);
		pDLI->channel->close(pDLI->channel);
		m_pManager->GetDevConfigEx().DestroySession(lLoginID, pDLI->nConnectID);

		if (pDLI->file)
		{
			fclose(pDLI->file);
			pDLI->file = 0;
		}
		
		pDLI->pDownLoadPosCallBack = NULL;
		pDLI->pTimeDownLoadPosCallBack = NULL;
		
		delete pDLI;
		m_lstDLI.remove(pDLI);
		nRet = 0;
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}

	m_csDLI.UnLock();
	return nRet;
}

int CSearchRecordAndPlayBack::GetDownloadPos(LONG lFileHandle, int *nTotalSize, int *nDownLoadSize)
{
	if (!nTotalSize || !nDownLoadSize)
	{
		return NET_ILLEGAL_PARAM;
	}
	else
	{
		*nTotalSize = 0;
		*nDownLoadSize = 0;
	}

	int nRet = -1;

	m_csDLI.Lock();

	st_DownLoad_Info* pDLI = GetDownLoadInfo(lFileHandle);
	if (pDLI)
	{
		*nTotalSize = pDLI->nTotalSize;
		if (-1 == pDLI->nDownLoadSize)
		{
			*nDownLoadSize = pDLI->nTotalSize;
		}
		else
		{
			*nDownLoadSize = pDLI->nDownLoadSize / 1024;
		}
		nRet = 0;
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}

	m_csDLI.UnLock();
	return nRet;
}

int	CSearchRecordAndPlayBack::CapturePicture(LONG lPlayHandle, const char *pchPicFileName)
{
	int nRet = NET_ERROR;
	m_csNPI.Lock();

	st_NetPlayBack_Info* pNPI = GetNetPlayBackInfo(lPlayHandle);
	if (pNPI)
	{
		if (pNPI->Render)
		{
			BOOL b = pNPI->Render->SnapPicture(pchPicFileName);
			if(b)
			{
				nRet = NET_NOERROR;
			}
			else
			{
				nRet = NET_RENDER_SNAP_ERROR;
			}
		}
		else
		{
			nRet = NET_SYSTEM_ERROR;
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}
	
	m_csNPI.UnLock();
	return nRet;
}


int CSearchRecordAndPlayBack::Decoder_OpenSound(LONG lPlayHandle)
{
	int nRet = NET_ERROR;
	m_csNPI.Lock();

	st_NetPlayBack_Info* pNPI = GetNetPlayBackInfo(lPlayHandle);
	if (pNPI && pNPI->Render)
	{
        BOOL b = pNPI->Render->OpenAudio();
		if (b)
		{
			pNPI->bAudioPlay = TRUE;
			nRet = NET_NOERROR;
		}	
		else
		{
			nRet = NET_RENDER_SOUND_ON_ERROR;
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}
	
	m_csNPI.UnLock();
	return nRet;
}

int CSearchRecordAndPlayBack::Decoder_CloseSound()
{
	int nRet = NET_ERROR;
	m_csNPI.Lock();

	list<st_NetPlayBack_Info*>::iterator it = m_lstNPI.begin();
	for(; it != m_lstNPI.end(); ++it)
	{
		if ((*it) && (*it)->Render && (*it)->bAudioPlay)
		{
			BOOL b = (*it)->Render->CloseAudio();
			if (b)
			{
				(*it)->bAudioPlay = FALSE;
				nRet = NET_NOERROR;
			}
			else
			{
				nRet = NET_RENDER_SOUND_OFF_ERROR;
			}
		}
	}

	m_csNPI.UnLock();

	return nRet;
}

int CSearchRecordAndPlayBack::GetDecoderVideoEffect(LONG lPlayHandle, 
									unsigned char *brightness, unsigned char *contrast, 
									unsigned char *hue, unsigned char *saturation)
{
	if (!brightness || !contrast || !hue || !saturation)
	{
		return NET_ILLEGAL_PARAM;
	}
	else
	{
		*brightness = 0;
		*contrast = 0;
		*hue = 0;
		*saturation = 0;
	}

	int nRet = NET_ERROR;
	m_csNPI.Lock();
	
	st_NetPlayBack_Info* pNPI = GetNetPlayBackInfo(lPlayHandle);

	if (pNPI)
	{
		if (pNPI->Render)
		{
            pNPI->Render->GetColorParam(brightness, contrast, hue, saturation);
            nRet = NET_NOERROR;
		}
		else
		{
			nRet = NET_SYSTEM_ERROR;
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}
	
	m_csNPI.UnLock();
	return nRet;
}

int CSearchRecordAndPlayBack::SetDecoderVideoEffect(LONG lPlayHandle, 
									unsigned char brightness, unsigned char contrast, 
									unsigned char hue, unsigned char saturation)
{
	int nRet = NET_ERROR;
	m_csNPI.Lock();

	st_NetPlayBack_Info* pNPI = GetNetPlayBackInfo(lPlayHandle);

	if (pNPI)
	{
		if (pNPI->Render)
		{
			BOOL b = pNPI->Render->AdjustColor(brightness, contrast, hue, saturation);
            if (b)
            {
                nRet = NET_NOERROR;
            }
            else
            {
				nRet = NET_RENDER_ADJUST_ERROR;
            }		
		}
		else
		{
			nRet = NET_SYSTEM_ERROR;
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}
	
	m_csNPI.UnLock();
	return nRet;
}

int CSearchRecordAndPlayBack::SetVolume(LONG lPlayHandle, int nVolume)
{
	int nRet = NET_ERROR;
	m_csNPI.Lock();

	st_NetPlayBack_Info* pNPI = GetNetPlayBackInfo(lPlayHandle);

	if (pNPI)
	{
        if (pNPI->Render)
        {
            BOOL b = pNPI->Render->SetAudioVolume(nVolume);
			if (b)
			{
				nRet = NET_NOERROR;
			}
			else
			{
				nRet = NET_RENDER_SET_VOLUME_ERROR;
			}
        }	
		else
		{
			nRet = NET_SYSTEM_ERROR;
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}
	
	m_csNPI.UnLock();
	return nRet;	
}


LONG CSearchRecordAndPlayBack::GetStatiscFlux(LONG lLoginID,LONG lPlayHandle)
{
	LONG lRet = NET_ERROR;
	m_csNPI.Lock();

	st_NetPlayBack_Info* pNPI = GetNetPlayBackInfo(lPlayHandle);
	if (pNPI)
	{
		afk_device_s *device = (afk_device_s*)lLoginID;

		afk_channel_s *stat_channel = (afk_channel_s*)device->open_channel(device, 
				AFK_CHANNEL_TYPE_STATISC, 0);

		if (stat_channel)
		{
			lRet = stat_channel->get_info(stat_channel, 0, pNPI->channel);
			bool b = stat_channel->close(stat_channel);
			if (!b)
			{
				lRet = NET_CLOSE_CHANNEL_ERROR;
			}
		}
		else
		{
			lRet = NET_OPEN_CHANNEL_ERROR;
		}
	}
	else
	{
		lRet = NET_INVALID_HANDLE;
	}
	
	m_csNPI.UnLock();
	return lRet;
}


/************************************************************************
 ** 释放录像文件的句柄列表
 ***********************************************************************/
void CSearchRecordAndPlayBack::ReleaseAllSearchRecordInfo(void)
{
	m_csSRI.Lock();
	list<st_SearchRecord_Info*>::iterator it = m_lstSRI.begin();
	for(; it != m_lstSRI.end(); ++it)
	{
		ReleaseRecordFileInfo(**it);
		delete (*it);
	}

	m_lstSRI.clear();
	m_csSRI.UnLock();
}


int CSearchRecordAndPlayBack::Process_stopplayback(st_NetPlayBack_Info& npi)
{
	int nRet = NET_NOERROR;
	
	SetEventEx(npi.hPBExit);
	
#ifdef WIN32
	DWORD hdl = GetCurrentThreadId();
	if (hdl == npi.dwThreadID)
	{
		//当前线程
	}
#else	//linux
	pthread_t self = pthread_self();
	if (self == npi.hThread.m_hThread)
	{
		//当前线程
	}
#endif
	else
	{
		DWORD dw = WaitForSingleObjectEx(npi.hThread, 1000*10);
		if (WAIT_OBJECT_0 != dw)
		{
			TerminateThreadEx(npi.hThread, 1);
		}
	}
	
	CloseThreadEx(npi.hThread);
	CloseEventEx(npi.hPBExit);

	if (npi.Render)
	{
		int ret = npi.Render->StopDec();
		if (ret >= 0)
		{
			npi.Render->SetDrawCallBack(0,0,0,0);
			m_pManager->GetRenderManager().ReleaseRender(npi.Render);
		}
		else
		{
			nRet = NET_DEC_CLOSE_ERROR;
		}
	}

	if (npi.channel)
	{
		LONG lLoginID = (LONG)npi.channel->get_device(npi.channel);
		
		bool bSuccess = npi.channel->close(npi.channel);
		if (!bSuccess)
		{
			nRet = NET_CLOSE_CHANNEL_ERROR;
		}

		m_pManager->GetDevConfigEx().DestroySession(lLoginID, npi.nConnectID);
	}

    if (npi.PlayBack)
    {
        delete npi.PlayBack;
		npi.PlayBack = NULL;
    }

	if(npi.pFileInfo)
	{
		delete npi.pFileInfo;
		npi.pFileInfo = NULL;
	}

	if (npi.prf)
	{
		delete[] npi.prf;
		npi.prf = NULL;
	}
	
	return nRet;
}

LONG CSearchRecordAndPlayBack::DownloadByTime(LONG lLoginID, int nChannelId, int nRecordFileType,
											LPNET_TIME tmStart, LPNET_TIME tmEnd, char *sSavedFileName,
											fTimeDownLoadPosCallBack cbTimeDownLoadPos, DWORD dwUserData)
{
	if (!sSavedFileName)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return 0;
	}
	
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return 0;
    }
	
    afk_device_s *device = (afk_device_s*)lLoginID;
	if (device->device_type(device) == NET_NB_SERIAL) 
	{
		return NET_UNSUPPORTED;
	}

	/* 先申请建立会话 */
	afk_connect_param_t stuConnParam = {0};
	stuConnParam.nConnType = channel_connect_tcp;
	stuConnParam.nInterfaceType = INTERFACE_DOWNLOAD;
	int ret = m_pManager->GetDevConfigEx().SetupSession(lLoginID, nChannelId, &stuConnParam);
	if (ret < 0)
	{
		m_pManager->SetLastError(ret);
		return 0;
	}

	list<NET_RECORDFILE_INFO*> lstrf;
	
	int r = Process_QueryRecordfile(device, nChannelId, nRecordFileType,
					tmStart, tmEnd, 0, 3000, true, lstrf);
	if (r < 0)
	{
		m_pManager->SetLastError(r);
		return 0;
	}
	else if (lstrf.size() <= 0) 
	{
		m_pManager->SetLastError(NET_NO_RECORD_FOUND);
		return 0;
	}
	
	DWORD dwTotalSize = 0;
	int nrflen = lstrf.size();
	NET_RECORDFILE_INFO* prf = new NET_RECORDFILE_INFO[nrflen];
	if (!prf)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		st_SearchRecord_Info sr;
		sr.lstrf = lstrf;
		ReleaseRecordFileInfo(sr);
		return 0;
	}

	list<NET_RECORDFILE_INFO*>::iterator it = lstrf.begin();
	for(int i=0; (it!=lstrf.end())&& (i<nrflen); it++,i++)
	{
		memcpy(prf+i, *it, sizeof(NET_RECORDFILE_INFO));
		dwTotalSize += prf[i].size;
		delete *it;
	}

	lstrf.clear();

	afk_channel_s *channel = 0;
    st_DownLoad_Info* pDLI = new st_DownLoad_Info;
	if (!pDLI)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}

    pDLI->channel = 0;
    pDLI->channelid = nChannelId;
    pDLI->file = fopen(sSavedFileName, "wb");
	if (!pDLI->file)
	{
		m_pManager->SetLastError(NET_OPEN_FILE_ERROR);
		goto e_clearup;
	}

//     WriteVideoFileHeader(pDLI->file, device->device_type(device), 25, 
// 		tmStart->dwYear, 
// 		tmStart->dwMonth, 
// 		tmStart->dwDay, 
// 		tmStart->dwHour,
// 		tmStart->dwMinute, 
// 		tmStart->dwSecond);
	
	pDLI->fileflushflag = 0;
	pDLI->nTotalSize = dwTotalSize;
	pDLI->timeStart = *tmStart;
	pDLI->timeEnd = *tmEnd;
	pDLI->nDownLoadSize = 0;
	pDLI->pDownLoadPosCallBack = 0;
	pDLI->pTimeDownLoadPosCallBack = cbTimeDownLoadPos;
	pDLI->userdata = dwUserData;
	pDLI->prf = prf;
	pDLI->nrflen = nrflen;
	pDLI->ncurrf = 0;
	pDLI->nConnectID = stuConnParam.nConnectID;
	
    afk_download_channel_param_s parm;
	memset(&parm, 0, sizeof(afk_download_channel_param_s));
    parm.base.func = DownLoadRecordFunc;
    parm.base.udata = pDLI;
	parm.conn = stuConnParam;
    memcpy(&parm.info, prf, sizeof(NET_RECORDFILE_INFO));
	parm.nByTime = 1;
	parm.nParam = 1; //下载
	parm.type = AFK_CHANNEL_DOWNLOAD_RECORD;
	
    channel = (afk_channel_s*)device->open_channel(device, AFK_CHANNEL_TYPE_DOWNLOAD, &parm);
    if (channel)
    {
		pDLI->channel = channel;
		m_csDLI.Lock();
		m_lstDLI.push_back(pDLI);
		m_csDLI.UnLock();
    }
	else
	{
		m_pManager->SetLastError(NET_OPEN_CHANNEL_ERROR);
#ifdef DEBUG
		OutputDebugString("open channel failed");
#endif
		goto e_clearup;
	}

	return (LONG)channel;

e_clearup:
	if (prf)
	{
		delete[] prf;
		prf = 0;
	}

	if (pDLI)
	{
		if (pDLI->file)
		{
			fclose(pDLI->file);
			pDLI->file = 0;
		}
		delete pDLI;
		pDLI = 0;
	}

	if (channel)
	{
		channel->close(channel);
		channel = 0;
	}
	
    return 0;	
}

