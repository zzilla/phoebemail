// DevConfigEx.cpp: implementation of the CDevConfigEx class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DevConfigEx.h"
#include "Manager.h"
#include "DevConfig.h"
#include "DecoderDevice.h"
#include "../dvr/dvrdevice/dvr2cfg.h"
#include "netsdktypes.h"
#include "Utils_StrParser.h"
#include "../dvr/def.h"
#include "../dvr/dvr.h"
#include "../dvr/ParseString.h"
#include "TransBurnFile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDevConfigEx::CDevConfigEx(CManager *pManager)
	:m_pManager(pManager)
{

}

CDevConfigEx::~CDevConfigEx()
{
	m_pManager = NULL;
}

int CDevConfigEx::Init()
{
	return Uninit();
}

int CDevConfigEx::Uninit()
{
	int nRet = 0;

	{
		m_csCfgExport.Lock();
		list<st_ConfigExport_Info*>::iterator it = m_lstCfgExport.begin();
		for(; it != m_lstCfgExport.end(); ++it)
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
		
		m_lstCfgExport.clear();
		m_csCfgExport.UnLock();
	}
	
	{
		m_csCfgImport.Lock();
		list<st_ConfigImport_Info*>::iterator it = m_lstCfgImport.begin();
		for(; it != m_lstCfgImport.end(); ++it)
		{
			if (*it)
			{
				(*it)->channel->close((*it)->channel);
				
				delete (*it);
			}
		}
		
		m_lstCfgImport.clear();
		m_csCfgImport.UnLock();
	}

	return nRet;
}

int CDevConfigEx::CloseChannelOfDevice(afk_device_s* device)
{
	int nRet = 0;

	{
		m_csCfgExport.Lock();
		list<st_ConfigExport_Info*>::iterator it = m_lstCfgExport.begin();
		while(it != m_lstCfgExport.end())
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
						(*it)->file = NULL;
					} 
					
					delete (*it);
					m_lstCfgExport.erase(it++);
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

		m_csCfgExport.UnLock();
	}

	{
		m_csCfgImport.Lock();
		list<st_ConfigImport_Info*>::iterator it = m_lstCfgImport.begin();
		while(it != m_lstCfgImport.end())
		{
			if ((*it) && (*it)->channel)
			{
				afk_device_s* _device = (afk_device_s*)(*it)->channel->get_device((*it)->channel);
				if (_device == device)
				{
					(*it)->channel->close((*it)->channel);
					
					delete (*it);
					m_lstCfgImport.erase(it++);
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

		m_csCfgImport.UnLock();
	}

	return nRet;
}

LONG CDevConfigEx::ExportConfigFile(LONG lLoginID, CONFIG_FILE_TYPE emConfigFileType, char *szSavedFilePath, 
                                      fDownLoadPosCallBack cbDownLoadPos, DWORD dwUserData)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (device == NULL || szSavedFilePath == NULL)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return 0;
	}

	/* 先申请建立会话 */
	afk_connect_param_t stuConnParam = {0};
	stuConnParam.nConnType = channel_connect_tcp;
	int ret = SetupSession(lLoginID, 16, &stuConnParam);
	if (ret < 0)
	{
		m_pManager->SetLastError(ret);
		return 0;
	}
    
	afk_channel_s *channel = NULL;
    st_ConfigExport_Info* pDownloadInfo = new st_ConfigExport_Info;
	if (pDownloadInfo == NULL)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}
	
    pDownloadInfo->channel = NULL;
	pDownloadInfo->file = NULL;
	pDownloadInfo->fileflushflag = 0;
	strcpy(pDownloadInfo->szFilePath, szSavedFilePath);
	pDownloadInfo->nTotalSize = 0;
	pDownloadInfo->nDownLoadSize = 0;
	pDownloadInfo->nTotalFileCount = 0;
	pDownloadInfo->nCurFileNum = -1;
	pDownloadInfo->pDownLoadPosCallBack = cbDownLoadPos;
	pDownloadInfo->userdata = dwUserData;
	pDownloadInfo->nConnectID = stuConnParam.nConnectID;

    afk_download_channel_param_s parm;
	memset(&parm, 0, sizeof(afk_download_channel_param_s));
    parm.base.func = ConfigExportCallBackFunc;
    parm.base.udata = pDownloadInfo;
	parm.conn = stuConnParam;
	parm.info.ch = 16;
	parm.type = AFK_CHANNEL_DOWNLOAD_CONFIGFILE;
	parm.nParam = emConfigFileType;

    channel = (afk_channel_s*)device->open_channel(device, AFK_CHANNEL_TYPE_DOWNLOAD, &parm);
    if (channel)
    {
		pDownloadInfo->channel = channel;
		m_csCfgExport.Lock();
		m_lstCfgExport.push_back(pDownloadInfo);
		m_csCfgExport.UnLock();
    }
	else
	{
		m_pManager->SetLastError(NET_OPEN_CHANNEL_ERROR);
		goto e_clearup;
	}

    return (LONG)channel;

e_clearup:

	if (pDownloadInfo)
	{
		if (pDownloadInfo->file)
		{
			fclose(pDownloadInfo->file);
			pDownloadInfo->file = NULL;
		}
		delete pDownloadInfo;
		pDownloadInfo = 0;
	}

	if (channel)
	{
		channel->close(channel);
		channel = NULL;
	}

    return 0;
}

int __stdcall CDevConfigEx::ConfigExportCallBackFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata
	)
{
    afk_channel_s *channel = (afk_channel_s*)object;
    if (channel == NULL)
	{
		return -1;
	}

	CDevConfigEx::st_ConfigExport_Info* pDownloadInfo = (CDevConfigEx::st_ConfigExport_Info*)udata;
	if (pDownloadInfo == NULL)
	{
		return -1;
	}

	if (datalen != (unsigned int)-1)
	{
		if (data != NULL && datalen == sizeof(CFG_INFO))
		{
			CFG_INFO *pConfigInfo = (CFG_INFO *)data;
			pDownloadInfo->nTotalFileCount = pConfigInfo->TotalConfigFileNum;
			pDownloadInfo->nTotalSize = pConfigInfo->AllConfigLen;
			pDownloadInfo->nDownLoadSize += pConfigInfo->ConfigFile.ThisFrameLen;

			if (pDownloadInfo->nCurFileNum != pConfigInfo->CurrentFileNo)
			{
				if (pDownloadInfo->file != NULL)
				{
					fclose(pDownloadInfo->file);
					pDownloadInfo->file = NULL;
				}
				
				char szFileName[256] = {0};
				sprintf(szFileName, "%s\\%s", pDownloadInfo->szFilePath, pConfigInfo->ConfigFile.FileName);
				pDownloadInfo->file = fopen(szFileName, "wb");
			}
			pDownloadInfo->nCurFileNum = pConfigInfo->CurrentFileNo;

			if (pDownloadInfo->file != NULL)
			{
				fwrite(pConfigInfo->ConfigFile.DataBuffer, pConfigInfo->ConfigFile.ThisFrameLen, 1, pDownloadInfo->file);
			}
			
			if (pDownloadInfo->pDownLoadPosCallBack != NULL)
			{
				pDownloadInfo->pDownLoadPosCallBack((LONG)channel, pDownloadInfo->nTotalSize, pDownloadInfo->nDownLoadSize, pDownloadInfo->userdata);
			}
		}
	}
	else
	{
		//下载结束
		if (pDownloadInfo->file != NULL)
		{
			fclose(pDownloadInfo->file);
			pDownloadInfo->file = NULL;
		}

		if (pDownloadInfo->pDownLoadPosCallBack != NULL)
		{
			pDownloadInfo->pDownLoadPosCallBack((LONG)channel, pDownloadInfo->nTotalSize, -1, pDownloadInfo->userdata);
		}
	}

	return 1;
}

int CDevConfigEx::StopExportCfgFile(LONG lExportHandle)
{
	int nRet = NET_INVALID_HANDLE;

	m_csCfgExport.Lock();

	list<st_ConfigExport_Info*>::iterator it =
		find_if(m_lstCfgExport.begin(),m_lstCfgExport.end(),SearchDLIbyChannel(lExportHandle));

	if (it != m_lstCfgExport.end())
	{
		st_ConfigExport_Info *pDownloadInfo = (*it);
		if (pDownloadInfo != NULL)
		{
			if (pDownloadInfo->channel != NULL)
			{
				LONG lLoginID = (LONG)pDownloadInfo->channel->get_device(pDownloadInfo->channel);

				pDownloadInfo->channel->close(pDownloadInfo->channel);
				
				DestroySession(lLoginID, pDownloadInfo->nConnectID);
			}

			if (pDownloadInfo->file != NULL)
			{
				fclose(pDownloadInfo->file);
				pDownloadInfo->file = NULL;
			}

			pDownloadInfo->pDownLoadPosCallBack = NULL;

			delete pDownloadInfo;
			m_lstCfgExport.remove(pDownloadInfo);
			nRet = 0;
		}
	}

	m_csCfgExport.UnLock();
	return nRet;
}

LONG CDevConfigEx::ImportConfigFile(LONG lLoginID, char *szFileName, fDownLoadPosCallBack cbUploadPos, DWORD dwUserData)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (device == NULL || szFileName == NULL)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return 0;
	}

    st_ConfigImport_Info *pConfigImportInfo = new st_ConfigImport_Info;
	if (pConfigImportInfo == NULL)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		return 0;
	}

	int ret = 0;
	afk_channel_s *pchannel = NULL;
	afk_upgrade_channel_param_s upgradechannel = {0};

	FILE *file = fopen(szFileName, "rb");
    if (file)
    {
        fpos_t pos;
        fseek(file, 0, SEEK_END);
        fgetpos(file, &pos);
#ifdef WIN32
        upgradechannel.size = pos;
#else	//linux
        upgradechannel.size = pos.__pos;
#endif
    }
    else
    {
		delete pConfigImportInfo;
		m_pManager->SetLastError(NET_ERROR);
        return 0;
    }

	fclose(file);
	file = NULL;

	ret = CreateEventEx(pConfigImportInfo->hRecEvent, FALSE, FALSE);
	if (ret < 0)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}
	
	upgradechannel.filetype = 0;
	upgradechannel.type = AFK_CHANNEL_UPLOAD_CONFIGFILE;

	pConfigImportInfo->device = device;
	pConfigImportInfo->channel = NULL;
	pConfigImportInfo->pImportPosCallBack = cbUploadPos;
	pConfigImportInfo->dwUser = dwUserData;
	pConfigImportInfo->bAccept = FALSE;

    upgradechannel.base.func = ConfigImportCallBackFunc; 
    upgradechannel.base.udata = pConfigImportInfo;
    strcpy(upgradechannel.filename, szFileName);

    pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_UPGRADE, &upgradechannel);
    if (pchannel)
    {
		DWORD dwRet = WaitForSingleObjectEx(pConfigImportInfo->hRecEvent, 1000);
        if (dwRet == WAIT_OBJECT_0 && pConfigImportInfo->bAccept)
        {
            pConfigImportInfo->channel = pchannel;
			pchannel->set_info(pchannel, 0, NULL);
			
			m_csCfgImport.Lock();
			m_lstCfgImport.push_back(pConfigImportInfo);
			m_csCfgImport.UnLock();
        }
		else
		{
			goto e_clearup;
		}
    }
	else
	{
		m_pManager->SetLastError(NET_OPEN_CHANNEL_ERROR);
		goto e_clearup;
	}

    return (LONG)pchannel;

e_clearup:
	if (file)
	{
		fclose(file);
		file = NULL;
	}

	if (pchannel)
	{
		pchannel->close(pchannel);
		pchannel = NULL;
	}
	
	if (pConfigImportInfo)
	{
		CloseEventEx(pConfigImportInfo->hRecEvent);

		delete pConfigImportInfo;
		pConfigImportInfo = NULL;
	}

	return 0;
}

int __stdcall CDevConfigEx::ConfigImportCallBackFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata)
{
	afk_channel_s *channel = (afk_channel_s*)object;
    if (channel == NULL)
	{
		return -1;
	}

	CDevConfigEx::st_ConfigImport_Info *pConfigImportInfo = (CDevConfigEx::st_ConfigImport_Info*)udata;
	if (pConfigImportInfo == NULL)
	{
		return -1;
	}

	switch((long)param)
	{
	case 0:	//发送数据中
		{
			if (pConfigImportInfo->pImportPosCallBack != NULL)
			{
				pConfigImportInfo->pImportPosCallBack((LONG)channel, (DWORD)data, datalen, pConfigImportInfo->dwUser);
			}
		}
		break;
	case 1:	//可以开始导入配置文件
		{
			pConfigImportInfo->bAccept = TRUE;
			SetEventEx(pConfigImportInfo->hRecEvent);
		}
		break;
	case 2:	//不允许导入配置文件
		{
			pConfigImportInfo->bAccept = FALSE;
			SetEventEx(pConfigImportInfo->hRecEvent);
		}
		break;
	case 3:	//导入配置文件成功
		{
			if (pConfigImportInfo->pImportPosCallBack != NULL)
			{
				pConfigImportInfo->pImportPosCallBack((LONG)channel, 0, -1, pConfigImportInfo->dwUser);
			}
		}
		break;
	case 4:	//导入配置文件失败
		{
			if (pConfigImportInfo->pImportPosCallBack != NULL)
			{
				pConfigImportInfo->pImportPosCallBack((LONG)channel, 0, -2, pConfigImportInfo->dwUser);
			}
		}
		break;
	case 5:	//导入配置文件出错
		{
			if (pConfigImportInfo->pImportPosCallBack != NULL)
			{
				pConfigImportInfo->pImportPosCallBack((LONG)channel, 0, -3, pConfigImportInfo->dwUser);
			}
		}
		break;
	default:
		break;
	}

    return 1;
}

int CDevConfigEx::StopImportCfgFile(LONG lImportHandle)
{
	int nRet = NET_INVALID_HANDLE;

	m_csCfgImport.Lock();

	list<st_ConfigImport_Info*>::iterator it =
		find_if(m_lstCfgImport.begin(),m_lstCfgImport.end(),SearchUploadbyChannel(lImportHandle));

	if (it != m_lstCfgImport.end())
	{
		st_ConfigImport_Info *pConfigImportInfo = (*it);
		if (pConfigImportInfo != NULL)
		{
			pConfigImportInfo->channel->close(pConfigImportInfo->channel);

			pConfigImportInfo->pImportPosCallBack = NULL;

			delete pConfigImportInfo;
			m_lstCfgImport.remove(pConfigImportInfo);
			nRet = 0;
		}
	}

	m_csCfgImport.UnLock();
	return nRet;
}

int __stdcall QueryDeviceLogWaitFunc(
							   afk_handle_t object,	/* 数据提供者 */
							   unsigned char *data,	/* 数据体 */
							   unsigned int datalen,	/* 数据长度 */
							   void *param,			/* 回调参数 */
							   void *udata)
{
	receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata || false == receivedata->addRef())
    {
		receivedata->decRef();
		return -1;
    }
	
	if (!receivedata->datalen || !receivedata->data)
	{
		receivedata->decRef();
		SetEventEx(receivedata->hRecEvt);
		return -1;
	}
	
	if((int)param == -1 || (int)param > 0)//旧日志结构体。
	{
		int nLogItemNum = datalen/sizeof(LOGINFO_ITEM);//接收到的日志结构体数目	
		long &nStart = receivedata->reserved[0];
		long &nEnd = receivedata->reserved[1];

		LOGINFO_ITEM *pStartItem = (LOGINFO_ITEM *)data;
		DEVICE_LOG_ITEM *pStartStruct = (DEVICE_LOG_ITEM *)receivedata->data;
		
		if(nStart > 0)
		{
			if(nLogItemNum <= nStart)//如果收到的日志条数不到开始条数
			{
				nStart -= nLogItemNum;
				nEnd -= nLogItemNum;
				nLogItemNum = 0;
			}
			else
			{
				nLogItemNum -= nStart;
				pStartItem += nStart;
				nEnd -= nStart;
				nStart = 0;

			}
		}// end if

		if(nStart <= 0)
		{
			int nStructNum = receivedata->maxlen/sizeof(DEVICE_LOG_ITEM) - *receivedata->datalen;//客户开辟的缓冲区长度可以存储的结构体数目		
			int nLoopCount = nStructNum < nLogItemNum ? nStructNum : nLogItemNum;
			nLoopCount = nLoopCount < nEnd ? nLoopCount : nEnd;
			if(nLoopCount <= 0)//没有日志，或者用户开辟缓冲不够
			{
				receivedata->result = 1;
				receivedata->decRef();
				SetEventEx(receivedata->hRecEvt);
				return 1;
			}
			pStartStruct += *receivedata->datalen;//已经接收的日志结构体数目

			char szTime[48] = {0};
			char szContext[64] = {0};
			int nLoop = 0;
			for(nLoop = 0; nLoop < nLoopCount; nLoop++)//将旧的日志结构体转为新的日志结构体
			{
				memset(szTime, 0, 48);
				memset(szContext, 0, 64);

				(pStartStruct+nLoop)->stuOperateTime.year = (pStartItem+nLoop)->time.year; 
				(pStartStruct+nLoop)->stuOperateTime.month = (pStartItem+nLoop)->time.month;
				(pStartStruct+nLoop)->stuOperateTime.day = (pStartItem+nLoop)->time.day;
				(pStartStruct+nLoop)->stuOperateTime.hour = (pStartItem+nLoop)->time.hour;
				(pStartStruct+nLoop)->stuOperateTime.minute = (pStartItem+nLoop)->time.minute;
				(pStartStruct+nLoop)->stuOperateTime.second = (pStartItem+nLoop)->time.second;
				(pStartStruct+nLoop)->nLogType = (pStartItem+nLoop)->type;
				(pStartStruct+nLoop)->bUnionType = 1;
				memcpy(&((pStartStruct+nLoop)->stuOldLog.stuLog), (pStartItem+nLoop), sizeof(LOGINFO_ITEM));
			}//end for
			*(receivedata->datalen) += nLoop;
			nEnd -= nLoop;
		}//end else 

		if((int)param == -1)//如果所有日志都接收完毕
		{
			receivedata->result = 1;
			receivedata->decRef();
			SetEventEx(receivedata->hRecEvt);
			return 1;
		}
	}
	else if((int)param == -2)//新日志结构体.放在回调中进行解析的原因是不用另外开辟缓冲，日志的数量比较多。
	{
		if(datalen <= 0)//如果没有日志信息
		{
			SetEventEx(receivedata->hRecEvt);
			receivedata->decRef();
			return -1;
		}
		
		char *pBuf = new char[datalen];
		if(NULL == pBuf)
		{
			SetEventEx(receivedata->hRecEvt);
			receivedata->decRef();
			return -1;
		}
		memcpy(pBuf, data, datalen);
		CStrParse topParse;
		topParse.setSpliter("|");
		bool bSuccess =  topParse.Parse(pBuf);//解析日志。
		if(!bSuccess)
		{
			delete []pBuf;
			SetEventEx(receivedata->hRecEvt);
			receivedata->decRef();
			return -1;
		}
		int nLogCount = topParse.Size();
		DEVICE_LOG_ITEM *pStartStruct = (DEVICE_LOG_ITEM *)receivedata->data;
		int nStructNum = receivedata->maxlen/sizeof(DEVICE_LOG_ITEM);//客户开辟的缓冲区长度可以存储的结构体数目
		int nLoopCount = nLogCount < nStructNum ? nLogCount : nStructNum;
		
		char szValueBuf[256] = {0};
		int	 nLen = 0;
		int nLoop = 0;
		for(nLoop = 0; nLoop < nLoopCount; nLoop++)//解析日志
		{
			(pStartStruct+nLoop)->bUnionType = 0;
			const char* cpLog = topParse.getWord(nLoop).c_str();

			memset(szValueBuf, 0 , 256);
			nLen = GetValueFormNewPacket(cpLog, "Type", szValueBuf, 256);
			if(nLen > 0)
			{
				(pStartStruct+nLoop)->nLogType = atof(szValueBuf);
			}

			memset(szValueBuf, 0 ,256);
			nLen = GetValueFormNewPacket(cpLog, "Username", szValueBuf, 256);
			if(nLen < 16 && nLen > 0)
			{
				Change_Utf8_Assic((unsigned char *)szValueBuf, (pStartStruct+nLoop)->szOperator);
			}

			memset(szValueBuf, 0, 256);
			nLen = GetValueFormNewPacket(cpLog, "Time", szValueBuf, 256);
			if(nLen > 0)
			{
				CStrParse timeParse;
				timeParse.setSpliter("/");
				bool bSuccess = timeParse.Parse(szValueBuf);
				if(timeParse.Size() == 6)
				{
					(pStartStruct+nLoop)->stuOperateTime.year = timeParse.getValue(0);
					(pStartStruct+nLoop)->stuOperateTime.month = timeParse.getValue(1);
					(pStartStruct+nLoop)->stuOperateTime.day = timeParse.getValue(2);
					(pStartStruct+nLoop)->stuOperateTime.hour = timeParse.getValue(3);
					(pStartStruct+nLoop)->stuOperateTime.minute = timeParse.getValue(4);
					(pStartStruct+nLoop)->stuOperateTime.second = timeParse.getValue(5);
				}
			}

			memset(szValueBuf, 0, 256);
			nLen = GetValueFormNewPacket(cpLog, "Context", szValueBuf, 256);
			if(nLen > 0 && nLen < 64)
			{
				Change_Utf8_Assic((unsigned char *)szValueBuf, (pStartStruct+nLoop)->szLogContext);
			}			
		}
		delete []pBuf;
		receivedata->result = 1;
		*(receivedata->datalen) = nLoop;
		SetEventEx(receivedata->hRecEvt);
		receivedata->decRef();
		return 1;
	}
	
	receivedata->decRef();
	return 1;
}

/*
 *	20万日志查询.
 */
int CDevConfigEx::QueryDeviceLog(LONG lLoginID, QUERY_DEVICE_LOG_PARAM *pQueryParam, char *pLogBuffer, int nLogBufferLen, int *pRecLogNum, int waittime)
{

	if(NULL == pQueryParam || NULL == pLogBuffer || 0 >= waittime)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;
	afk_device_s *device = (afk_device_s *)lLoginID;
 
	afk_search_channel_param_s searchchannel = {0};
	searchchannel.type = AFK_CHANNEL_SEARCH_LOG;
	searchchannel.subtype = 0;//查询日志类型
	searchchannel.base.func = QueryDeviceLogWaitFunc;
	
	//查看能力
	char szPacketBuf[1024] = {0};
	char szPacketLineBuf[128] = {0};
	int nRetLen = 0;
	DEV_ENABLE_INFO stDevEn = {0};
	nRet = m_pManager->GetDevConfig().GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, 1000);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_LOG] != 0)
		{
			searchchannel.param = 2;//表示分页方式查询日志

			//构建字符串
			sprintf(szPacketLineBuf, "Type:%d\r\n", pQueryParam->emLogType);
			strcat(szPacketBuf, szPacketLineBuf);
			memset(szPacketLineBuf, 0, 128);

			sprintf(szPacketLineBuf, "Username:%s\r\n", "");
			strcat(szPacketBuf, szPacketLineBuf);
			memset(szPacketLineBuf, 0, 128);

			sprintf(szPacketLineBuf, "StartTime:%d/%d/%d/%d/%d\r\n", pQueryParam->stuStartTime.dwYear, pQueryParam->stuStartTime.dwMonth, pQueryParam->stuStartTime.dwDay, pQueryParam->stuStartTime.dwHour, pQueryParam->stuStartTime.dwMinute);
			strcat(szPacketBuf, szPacketLineBuf);
			memset(szPacketLineBuf, 0, 128);

			sprintf(szPacketLineBuf, "EndTime:%d/%d/%d/%d/%d\r\n", pQueryParam->stuEndTime.dwYear, pQueryParam->stuEndTime.dwMonth, pQueryParam->stuEndTime.dwDay, pQueryParam->stuEndTime.dwHour, pQueryParam->stuEndTime.dwMinute);
			strcat(szPacketBuf, szPacketLineBuf);
			memset(szPacketLineBuf, 0, 128);

			sprintf(szPacketLineBuf, "StRecordNum:%d\r\n", pQueryParam->nStartNum);
			strcat(szPacketBuf, szPacketLineBuf);
			memset(szPacketLineBuf, 0, 128);

			sprintf(szPacketLineBuf, "EdRecordNum:%d\r\n", pQueryParam->nEndNum);
			strcat(szPacketBuf, szPacketLineBuf);
			//strcat(szPacketBuf, "\r\n");

			searchchannel.webName = szPacketBuf;
			searchchannel.webNamelen = strlen(szPacketBuf);
			searchchannel.subtype = pQueryParam->emLogType;
		}
	}
	   
    *pRecLogNum = 0;
   
	memset(pLogBuffer, 0, nLogBufferLen);

	receivedata_s receivedata;// = {0};
    receivedata.data = pLogBuffer;
    receivedata.datalen = pRecLogNum;
    receivedata.maxlen = nLogBufferLen;
	receivedata.reserved[0] = pQueryParam->nStartNum;
	receivedata.reserved[1] = pQueryParam->nEndNum;
	int nLanguage = 0;
	
    receivedata.result = -1;
    searchchannel.base.udata = &receivedata;
	
	nRet = -1;
	afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
    if (pchannel)
    {
		DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
        pchannel->close(pchannel);
        ResetEventEx(receivedata.hRecEvt);
        if (dwRet == WAIT_OBJECT_0)
        {
            nRet = receivedata.result;
        }
		else
		{
			nRet = NET_NETWORK_ERROR;
		}
	}
	
	return nRet;
}

/************************************************************************
返回值 ： 
	0:	szOutBuf不够；
	>0:	返回拷贝的长度; 
	-1:	表示没有找到szKeyBuf对应的项;
	-2:	参数错误; 
	-3:	包错误
************************************************************************/
int GetValueFormNewPacket(const char *szBuf, const char *szKeyBuf, char *szOutBuf, int nOutBufLen)
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
			if(pValueEnd == NULL)
			{
				pValueEnd = (char *)(szBuf + strlen(szBuf));

			}
			else
			{
				return -3;
			}
		}
		memcpy(szOutBuf, pValueStart, pValueEnd-pValueStart);
		return (pValueEnd-pValueStart);
	}
	else
	{
		return -3;
	}
}

int __stdcall cbSearchDevice(		 afk_handle_t object,	/* 数据提供者 */
									 unsigned char *data,	/* 数据体 */
									 unsigned int datalen,	/* 数据长度 */
									 void *param,			/* 回调参数 */
									 void *udata)
{
	receivedata_s *pReceivedata = (receivedata_s *)udata;
	if(NULL == pReceivedata)
	{
		return -1;
	}
	
	if(*(pReceivedata->datalen)+sizeof(DEVICE_NET_INFO) > pReceivedata->maxlen)
	{
		return -1;
	}

	if(0xb3 == (BYTE)data[0])
	{
		int nUUIdLen = data[2];
		int nExtLen =  *(DWORD*)(data+4);
		if( nExtLen+HEADER_SIZE+nUUIdLen == datalen)
		{
			CONFIG_NET *pCfgNet = (CONFIG_NET *)(data+HEADER_SIZE);
			DEVICE_NET_INFO *pDeviceNetInfo = (DEVICE_NET_INFO *)(pReceivedata->data + *(pReceivedata->datalen));
			
			if(nUUIdLen > 0 && nUUIdLen < MAC_ADDRESS_LEN)
			{
				if(nUUIdLen >= 17)
				{
					memcpy(pDeviceNetInfo->szMac, data+HEADER_SIZE+nExtLen, 17);
				}
				if(nUUIdLen-17 > 0)
				{
					memcpy(pDeviceNetInfo->szDeviceType, data+HEADER_SIZE+nExtLen+17, nUUIdLen-17);
				}
			}

			if(nExtLen == sizeof(CONFIG_NET) && data[16] == 2)
			{
				in_addr ip;
				ip.s_addr = pCfgNet->HostIP;
				strcpy(pDeviceNetInfo->szIP, inet_ntoa(ip));
				pDeviceNetInfo->nPort = pCfgNet->TCPPort;
				ip.s_addr = pCfgNet->GateWayIP;
				strcpy(pDeviceNetInfo->szGateway, inet_ntoa(ip));
				ip.s_addr = pCfgNet->Submask;
				strcpy(pDeviceNetInfo->szSubmask, inet_ntoa(ip));	
			}
			*(pReceivedata->datalen) += sizeof(DEVICE_NET_INFO);
		}
	}
	
	return 1;
}

int CDevConfigEx::SearchDevice(char* szBuf, int nBufLen, int* pRetLen, DWORD dwSearchTime)
{
	if (szBuf == NULL || pRetLen == NULL || m_pManager->m_pDeviceProb == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	*pRetLen = 0;
	memset(szBuf, 0, nBufLen);
	receivedata_s receivedata;
	receivedata.data = szBuf;
	receivedata.datalen = pRetLen;
	receivedata.maxlen = nBufLen;

	LONG lHandle = m_pManager->m_pDeviceProb->start_search_device(cbSearchDevice, &receivedata);
	if(lHandle <= 0)
	{
		return NET_NETWORK_ERROR;
	}

	DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, dwSearchTime);
	m_pManager->m_pDeviceProb->stop_search_device(lHandle);

	return 1;
}



//function for getting new config 
int CDevConfigEx::GetDevNewConfig(LONG lLoginID, DWORD dwCommand, LONG lChannel, LPVOID lpOutBuffer,DWORD dwOutBufferSize,LPDWORD lpBytesReturned,int waittime)
{	
	if (!lpOutBuffer || !lpBytesReturned)
	{
		return NET_ILLEGAL_PARAM;
	}

	if (lChannel < -1 || lChannel >= MAX_CHANNUM)
	{
		return NET_ILLEGAL_PARAM;
	}

	*lpBytesReturned = 0;
	memset(lpOutBuffer, 0, dwOutBufferSize);

	afk_device_s* device = (afk_device_s*)lLoginID;

	if (!device || m_pManager->IsDeviceValid(device) < 0)
	{
		return NET_INVALID_HANDLE;
	}

	int retlen = 0; //data length
	int nRet = -1;  //return value

	switch(dwCommand)
	{
	case DEV_IPFILTER_CFG:				//IP 过滤配置
		{
			if (dwOutBufferSize < sizeof(DEVICE_IPIFILTER_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVICE_IPIFILTER_CFG stIPFilter = {0};

			nRet = GetDevNewConfig_IPFilter(lLoginID, stIPFilter, waittime);
			if (nRet < 0)
			{
				return nRet;
			}

			memcpy(lpOutBuffer, &stIPFilter, sizeof(DEVICE_IPIFILTER_CFG));
			*lpBytesReturned = sizeof(DEVICE_IPIFILTER_CFG);
		}
		break;
	case DEV_TALK_ENCODE_CFG:				//语音对讲编码配置
		{
			if (dwOutBufferSize < sizeof(DEVICE_TALK_ENCODE_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVICE_TALK_ENCODE_CFG stTalkEncode = {0};
			
			nRet = GetDevNewConfig_TalkEncode(lLoginID, stTalkEncode, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			
			memcpy(lpOutBuffer, &stTalkEncode, sizeof(DEVICE_TALK_ENCODE_CFG));
			*lpBytesReturned = sizeof(DEVICE_TALK_ENCODE_CFG);
		}
		break;	
	case DEV_RECORD_PACKET_CFG: //录像打包时间
		{
			if (dwOutBufferSize < sizeof(DEVICE_RECORD_PACKET_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVICE_RECORD_PACKET_CFG stRecordLen = {0};

			nRet = GetDevNewConfig_RecordPacketLen(lLoginID, stRecordLen, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			
			memcpy(lpOutBuffer, &stRecordLen, sizeof(DEVICE_RECORD_PACKET_CFG));
			*lpBytesReturned = sizeof(DEVICE_RECORD_PACKET_CFG);
		}
		break;
	case DEV_ALARM_DECODE_CFG:
		{
			int nChannel = 0;
			const int nMaxChlNum = 8;
			if(lChannel == -1)
			{
				nChannel = nMaxChlNum;
				if (dwOutBufferSize < sizeof(ALARMDECODER_CFG)*nChannel)
				{
					return NET_ILLEGAL_PARAM;
				}
				
				ALARMDECODER_CFG stAlarmDecCfg[nMaxChlNum] = {0};
				for(int i = 0; i < nMaxChlNum; i++)
				{
					nRet = GetDevNewConfig_AlmDecCfg(lLoginID, i, &stAlarmDecCfg[i], waittime);
					if (nRet < 0)
					{
						//break;
					}
				}
				memcpy(lpOutBuffer, &stAlarmDecCfg, sizeof(ALARMDECODER_CFG)*nMaxChlNum);
				*lpBytesReturned = sizeof(ALARMDECODER_CFG)*nMaxChlNum;			
			}
			else if(lChannel >= 0 && lChannel < nMaxChlNum)
			{
				nChannel = 1;
				if (dwOutBufferSize < sizeof(ALARMDECODER_CFG)*nChannel)
				{
					return NET_ILLEGAL_PARAM;
				}
				
				ALARMDECODER_CFG stAlarmDecCfg = {0};
				nRet = GetDevNewConfig_AlmDecCfg(lLoginID, lChannel, &stAlarmDecCfg, waittime);
				if (nRet < 0)
				{
					return nRet;
				}
				
				memcpy(lpOutBuffer, &stAlarmDecCfg, sizeof(ALARMDECODER_CFG));
				*lpBytesReturned = sizeof(ALARMDECODER_CFG);
			}
			else
			{
				return -1;
			}	
		}
		break;
	case DEV_MMS_CFG:				//MMS配置
		{
			if (dwOutBufferSize < sizeof(DEVICE_MMS_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVICE_MMS_CFG stMMS = {0};

			nRet = GetDevNewConfig_MMS(lLoginID, stMMS, waittime);
			if (nRet < 0)
			{
				return nRet;
			}

			memcpy(lpOutBuffer, &stMMS, sizeof(DEVICE_MMS_CFG));
			*lpBytesReturned = sizeof(DEVICE_MMS_CFG);
		}
		break;
	case DEV_SMSACTIVATION_CFG:		// 短信激活无线连接配置
		{
			if (dwOutBufferSize < sizeof(DEVICE_SMSACTIVATION_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVICE_SMSACTIVATION_CFG stSMSActive = {0};
			
			nRet = GetDevNewConfig_SMSACTIVE(lLoginID, stSMSActive, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			
			memcpy(lpOutBuffer, &stSMSActive, sizeof(DEVICE_SMSACTIVATION_CFG));
			*lpBytesReturned = sizeof(DEVICE_SMSACTIVATION_CFG);
		}
		break;	
	case DEV_DIALINACTIVATION_CFG:		// 拨号激活无线连接配置
		{
			if (dwOutBufferSize < sizeof(DEVICE_DIALINACTIVATION_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVICE_DIALINACTIVATION_CFG stDialIn = {0};
			
			nRet = GetDevNewConfig_DIALIN(lLoginID, stDialIn, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			
			memcpy(lpOutBuffer, &stDialIn, sizeof(DEVICE_DIALINACTIVATION_CFG));
			*lpBytesReturned = sizeof(DEVICE_DIALINACTIVATION_CFG);
		}
		break;
	case DEV_SNIFFER_CFG_EX:
		{
			if(dwOutBufferSize < sizeof(YW_ATM_SNIFFER_CFG_EX))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			YW_ATM_SNIFFER_CFG_EX SnifferCfg = {0};			
			nRet = GetDevNewConfig_Sniffer(lLoginID, &SnifferCfg, lChannel, waittime);
			
			memcpy(lpOutBuffer, &SnifferCfg, sizeof(YW_ATM_SNIFFER_CFG_EX));
			*lpBytesReturned = sizeof(YW_ATM_SNIFFER_CFG_EX);
		}
		break;
	case DEV_DOWNLOAD_RATE_CFG:
		{
			if(dwOutBufferSize < sizeof(int))
			{
				return NET_ILLEGAL_PARAM;
			}

			int nDownloadRate = 0;
			nRet = GetDevNewConfig_DownloadRate(lLoginID, nDownloadRate, waittime);
			memcpy(lpOutBuffer, &nDownloadRate, sizeof(int));
			*lpBytesReturned = sizeof(int);
		}
		break;
	case DEV_PANORAMA_SWITCH_CFG:
		{
			if (dwOutBufferSize < sizeof(ALARM_PANORAMA_SWITCH_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			ALARM_PANORAMA_SWITCH_CFG *lpAlarmCfg = (ALARM_PANORAMA_SWITCH_CFG *)lpOutBuffer;
			nRet = GetDevNewConfig_PanoramaSwitch(lLoginID, lpAlarmCfg, waittime);
			if (nRet < 0)
			{
				return nRet;
			}

			*lpBytesReturned = sizeof(ALARM_PANORAMA_SWITCH_CFG);
		}
		break;
	case DEV_LOST_FOCUS_CFG:
		{
			if (dwOutBufferSize < sizeof(ALARM_LOST_FOCUS_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			ALARM_LOST_FOCUS_CFG *lpAlarmCfg = (ALARM_LOST_FOCUS_CFG *)lpOutBuffer;
			nRet = GetDevNewConfig_LostFocus(lLoginID, lpAlarmCfg, waittime);
			if (nRet < 0)
			{
				return nRet;
			}

			*lpBytesReturned = sizeof(ALARM_LOST_FOCUS_CFG);
		}
		break;
	case DEV_VIDEOOUT_CFG:
		{
			if (dwOutBufferSize < sizeof(DEV_VIDEOOUT_INFO))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEV_VIDEOOUT_INFO stVideoOutCfg = {0};
			nRet = GetDevNewConfig_VideoOut(lLoginID, stVideoOutCfg, waittime);
			if (nRet < 0)
			{
				return nRet;
			}

			memcpy(lpOutBuffer, &stVideoOutCfg, sizeof(DEV_VIDEOOUT_INFO));
			*lpBytesReturned = sizeof(DEV_VIDEOOUT_INFO);
		}
		break;
	case DEV_POINT_CFG:
		{
			if(dwOutBufferSize < sizeof(DEVICE_POINT_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEVICE_POINT_CFG stuPointCfg = {0};
			nRet = GetDevNewConfig_Point(lLoginID, stuPointCfg, waittime);
			if(nRet < 0)
			{
				return nRet;
			}

			memcpy(lpOutBuffer, &stuPointCfg, sizeof(DEVICE_POINT_CFG));
			*lpBytesReturned = sizeof(DEVICE_POINT_CFG);
		}
		break;
	case DEV_LOCALALARM_CFG: // 本地报警配置(结构体ALARMIN_CFG_EX)
		{
			int nAlarmNum = device->alarminputcount(device);
			if ( (lChannel >= 0 && lChannel < nAlarmNum && dwOutBufferSize < sizeof(ALARM_IN_CFG_EX)) ||
				(lChannel == -1 && dwOutBufferSize < nAlarmNum*sizeof(ALARM_IN_CFG_EX)) ||
				(lChannel >= nAlarmNum && lChannel != -1) )
			{
				return NET_ILLEGAL_PARAM;
			}
			
			int nChannelNum = nAlarmNum>16?nAlarmNum:16;
			if (-1 == lChannel)
			{
				ALARM_IN_CFG_EX *pstLocalAM_ALL = new ALARM_IN_CFG_EX[nChannelNum];
				if (!pstLocalAM_ALL)
				{
					return NET_SYSTEM_ERROR;
				}
				
				nRet = GetDevNewConfig_AllLocalALMCfg(lLoginID, pstLocalAM_ALL, nChannelNum, waittime);
				if (nRet < 0)
				{
					delete[] pstLocalAM_ALL;
					return nRet;
				}	
				
				memcpy(lpOutBuffer, pstLocalAM_ALL, nAlarmNum*sizeof(ALARM_IN_CFG_EX));
				*lpBytesReturned = nAlarmNum*sizeof(ALARM_IN_CFG_EX);
				delete[] pstLocalAM_ALL;
				pstLocalAM_ALL = NULL;
			}
			else
			{
				ALARM_IN_CFG_EX stLocalAM;	
				nRet = GetDevNewConfig_LocalALMCfg(lLoginID, stLocalAM, lChannel, waittime);
				if (nRet < 0)
				{
					return nRet;
				}
				memcpy(lpOutBuffer, &stLocalAM, sizeof(ALARM_IN_CFG_EX));
				*lpBytesReturned = sizeof(ALARM_IN_CFG_EX);
			}	
		}
		break;
	case DEV_NETALARM_CFG:
		{
			int nAlarmNum = device->alarminputcount(device);
			if ( (lChannel >= 0 && lChannel < nAlarmNum && dwOutBufferSize < sizeof(ALARM_IN_CFG_EX)) ||
				(lChannel == -1 && dwOutBufferSize < nAlarmNum*sizeof(ALARM_IN_CFG_EX)) ||
				(lChannel >= nAlarmNum && lChannel != -1) )
			{
				return NET_ILLEGAL_PARAM;
			}
			
			int nChannelNum = nAlarmNum>16?nAlarmNum:16;
			if (-1 == lChannel)
			{
				ALARM_IN_CFG_EX *pstNetAM_ALL = new ALARM_IN_CFG_EX[nChannelNum];
				if (!pstNetAM_ALL)
				{
					return NET_SYSTEM_ERROR;
				}
				
				nRet = GetDevNewConfig_AllNetALMCfg(lLoginID, pstNetAM_ALL, nChannelNum, waittime);
				if (nRet < 0)
				{
					delete[] pstNetAM_ALL;
					return nRet;
				}	
				
				memcpy(lpOutBuffer, pstNetAM_ALL, nAlarmNum*sizeof(ALARM_IN_CFG_EX));
				*lpBytesReturned = nAlarmNum*sizeof(ALARM_IN_CFG_EX);
				delete[] pstNetAM_ALL;
				pstNetAM_ALL = NULL;
			}
			else
			{
				ALARM_IN_CFG_EX stNetAM;	
				nRet = GetDevNewConfig_NetALMCfg(lLoginID, stNetAM, lChannel, waittime);
				if (nRet < 0)
				{
					return nRet;
				}
				memcpy(lpOutBuffer, &stNetAM, sizeof(ALARM_IN_CFG_EX));
				*lpBytesReturned = sizeof(ALARM_IN_CFG_EX);
			}	
		}
		break;
	case DEV_MOTIONALARM_CFG :
		{
			int nAlarmNum = device->channelcount(device);
			if ( (lChannel >= 0 && lChannel < nAlarmNum && dwOutBufferSize < sizeof(MOTION_DETECT_CFG_EX)) ||
				(lChannel == -1 && dwOutBufferSize < nAlarmNum*sizeof(MOTION_DETECT_CFG_EX)) ||
				(lChannel >= nAlarmNum && lChannel != -1) )
			{
				return NET_ILLEGAL_PARAM;
			}
			
			int nChannelNum = nAlarmNum>16?nAlarmNum:16;
			if (-1 == lChannel)
			{
				MOTION_DETECT_CFG_EX *pstMotionAM_ALL = new MOTION_DETECT_CFG_EX[nChannelNum];
				if (!pstMotionAM_ALL)
				{
					return NET_SYSTEM_ERROR;
				}
				
				nRet = GetDevNewConfig_AllMotionALMCfg(lLoginID, pstMotionAM_ALL, nChannelNum, waittime);
				if (nRet < 0)
				{
					delete[] pstMotionAM_ALL;
					return nRet;
				}	
				
				memcpy(lpOutBuffer, pstMotionAM_ALL, nAlarmNum*sizeof(MOTION_DETECT_CFG_EX));
				*lpBytesReturned = nAlarmNum*sizeof(MOTION_DETECT_CFG_EX);

				delete[] pstMotionAM_ALL;
				pstMotionAM_ALL = NULL;
			}
			else
			{
				MOTION_DETECT_CFG_EX stMotionAM;	
				nRet = GetDevNewConfig_MotionALMCfg(lLoginID, stMotionAM, lChannel, waittime);
				if (nRet < 0)
				{
					return nRet;
				}
				memcpy(lpOutBuffer, &stMotionAM, sizeof(MOTION_DETECT_CFG_EX));
				*lpBytesReturned = sizeof(MOTION_DETECT_CFG_EX);
			}	
		}
		break;
	case DEV_VIDEOLOSTALARM_CFG:
		{
			int nAlarmNum = device->channelcount(device);
			if ( (lChannel >= 0 && lChannel < nAlarmNum && dwOutBufferSize < sizeof(VIDEO_LOST_CFG_EX)) ||
				(lChannel == -1 && dwOutBufferSize < nAlarmNum*sizeof(VIDEO_LOST_CFG_EX)) ||
				(lChannel >= nAlarmNum && lChannel != -1) )
			{
				return NET_ILLEGAL_PARAM;
			}
			
			int nChannelNum = nAlarmNum>16?nAlarmNum:16;
			if (-1 == lChannel)
			{
				VIDEO_LOST_CFG_EX *pstLossAM_ALL = new VIDEO_LOST_CFG_EX[nChannelNum];
				if (!pstLossAM_ALL)
				{
					return NET_SYSTEM_ERROR;
				}
				
				nRet = GetDevNewConfig_AllVideoLostALMCfg(lLoginID, pstLossAM_ALL, nChannelNum, waittime);
				if (nRet < 0)
				{
					delete[] pstLossAM_ALL;
					return nRet;
				}	
				
				memcpy(lpOutBuffer, pstLossAM_ALL, nAlarmNum*sizeof(VIDEO_LOST_CFG_EX));
				*lpBytesReturned = nAlarmNum*sizeof(VIDEO_LOST_CFG_EX);
				
				delete[] pstLossAM_ALL;
				pstLossAM_ALL = NULL;
			}
			else
			{
				VIDEO_LOST_CFG_EX stLossAM;	
				nRet = GetDevNewConfig_VideoLostALMCfg(lLoginID, stLossAM, lChannel, waittime);
				if (nRet < 0)
				{
					return nRet;
				}
				memcpy(lpOutBuffer, &stLossAM, sizeof(VIDEO_LOST_CFG_EX));
				*lpBytesReturned = sizeof(VIDEO_LOST_CFG_EX);
			}	
		}
		break;
	case DEV_BLINDALARM_CFG :
		{
			int nAlarmNum = device->channelcount(device);
			if ( (lChannel >= 0 && lChannel < nAlarmNum && dwOutBufferSize < sizeof(BLIND_CFG_EX)) ||
				(lChannel == -1 && dwOutBufferSize < nAlarmNum*sizeof(BLIND_CFG_EX)) ||
				(lChannel >= nAlarmNum && lChannel != -1) )
			{
				return NET_ILLEGAL_PARAM;
			}
			
			int nChannelNum = nAlarmNum>16?nAlarmNum:16;
			if (-1 == lChannel)
			{
				BLIND_CFG_EX *pstBlindAM_ALL = new BLIND_CFG_EX[nChannelNum];
				if (!pstBlindAM_ALL)
				{
					return NET_SYSTEM_ERROR;
				}
				
				nRet = GetDevNewConfig_AllBlindALMCfg(lLoginID, pstBlindAM_ALL, nChannelNum, waittime);
				if (nRet < 0)
				{
					delete[] pstBlindAM_ALL;
					return nRet;
				}	
				
				memcpy(lpOutBuffer, pstBlindAM_ALL, nAlarmNum*sizeof(BLIND_CFG_EX));
				*lpBytesReturned = nAlarmNum*sizeof(BLIND_CFG_EX);
				
				delete[] pstBlindAM_ALL;
				pstBlindAM_ALL = NULL;
			}
			else
			{
				BLIND_CFG_EX stBlindAM;	
				nRet = GetDevNewConfig_BlindALMCfg(lLoginID, stBlindAM, lChannel, waittime);
				if (nRet < 0)
				{
					return nRet;
				}
				memcpy(lpOutBuffer, &stBlindAM, sizeof(BLIND_CFG_EX));
				*lpBytesReturned = sizeof(BLIND_CFG_EX);
			}	
		}
		break;
	case DEV_DISKALARM_CFG:
		{
			if(dwOutBufferSize < sizeof(DISK_ALARMIN_CFG_EX))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			DISK_ALARMIN_CFG_EX stuDiskCfg = {0};
			nRet = GetDevNewConfig_DiskALMCfg(lLoginID, &stuDiskCfg, waittime);
			if(nRet < 0)
			{
				return nRet;
			}
			
			memcpy(lpOutBuffer, &stuDiskCfg, sizeof(DISK_ALARMIN_CFG_EX));
			*lpBytesReturned = sizeof(DISK_ALARMIN_CFG_EX);
		}
		break;
	case DEV_NETBROKENALARM_CFG:
		{
			if(dwOutBufferSize < sizeof(NET_BROKEN_ALARM_CFG_EX))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			NET_BROKEN_ALARM_CFG_EX stuNetBrokenCfg = {0};
			nRet = GetDevNewConfig_NetBrokenALMCfg(lLoginID, &stuNetBrokenCfg, waittime);
			if(nRet < 0)
			{
				return nRet;
			}
			
			memcpy(lpOutBuffer, &stuNetBrokenCfg, sizeof(NET_BROKEN_ALARM_CFG_EX));
			*lpBytesReturned = sizeof(NET_BROKEN_ALARM_CFG_EX);
		}
		break;
	case DEVICE_ENCODER_CFG:
		{
			if(dwOutBufferSize < sizeof(DEV_ENCODER_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			DEV_ENCODER_CFG stuEncoderCfg = {0};
			nRet = GetDevNewConfig_EncoderCfg(lLoginID, &stuEncoderCfg, waittime);
			if(nRet < 0)
			{
				return nRet;
			}
			
			memcpy(lpOutBuffer, &stuEncoderCfg, sizeof(DEV_ENCODER_CFG));
			*lpBytesReturned = sizeof(DEV_ENCODER_CFG);
		}
		break;
	case DEV_IP_COLLISION_CFG:
		{
			if (dwOutBufferSize < sizeof(ALARM_IP_COLLISION_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			ALARM_IP_COLLISION_CFG *lpAlarmCfg = (ALARM_IP_COLLISION_CFG *)lpOutBuffer;
			nRet = GetDevNewConfig_IPCOllisionDetect(lLoginID, lpAlarmCfg, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			
			*lpBytesReturned = sizeof(ALARM_IP_COLLISION_CFG);
		}
		break;
	case DEV_OSD_ENABLE_CFG:
		{
			if(dwOutBufferSize < sizeof(DEV_OSD_ENABLE_CONFIG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEV_OSD_ENABLE_CONFIG *lpOSDEnabelCfg = (DEV_OSD_ENABLE_CONFIG *)lpOutBuffer;
			nRet = GetDevNewConfig_OSDEnable(lLoginID, lpOSDEnabelCfg, waittime);
			if(nRet < 0)
			{
				return nRet;
			}

			*lpBytesReturned = sizeof(DEV_OSD_ENABLE_CONFIG);
		}
		break;
	case DEV_TV_ADJUST_CFG:
		{
			if(dwOutBufferSize < sizeof(DEVICE_TVADJUST_CFG) || lChannel == -1)
			{
				return NET_ILLEGAL_PARAM;
			}
			
			DEVICE_TVADJUST_CFG *lpTVAdjustCfg = (DEVICE_TVADJUST_CFG *)lpOutBuffer;
			nRet = GetDevNewConfig_TVAdjustCfg(lLoginID, lChannel, lpTVAdjustCfg, waittime);
			if(nRet < 0)
			{
				return nRet;
			}
			
			*lpBytesReturned = sizeof(DEVICE_TVADJUST_CFG);
		}
		break;
	case DEV_ABOUT_VEHICLE_CFG:
		{
			if(dwOutBufferSize < sizeof(DEV_ABOUT_VEHICLE_CONFIG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEV_ABOUT_VEHICLE_CONFIG* pVehicleCfg = (DEV_ABOUT_VEHICLE_CONFIG*)lpOutBuffer;
			nRet = GetDevNewConfig_VehicleCfg(lLoginID, pVehicleCfg, waittime);
			if(nRet < 0)
			{
				return nRet;
			}

			*lpBytesReturned = sizeof(DEV_ABOUT_VEHICLE_CONFIG);
		}
		break;
	case DEV_ATM_OVERLAY_ABILITY:
		{
			if(dwOutBufferSize < sizeof(DEV_ATM_OVERLAY_GENERAL))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			DEV_ATM_OVERLAY_GENERAL* pAtmGeneral = (DEV_ATM_OVERLAY_GENERAL *)lpOutBuffer;
			nRet = GetDevNewConfig_AtmOverlayGeneral(lLoginID, pAtmGeneral, waittime);
			if(nRet < 0)
			{
				return nRet;
			}
			
			*lpBytesReturned = sizeof(DEV_ATM_OVERLAY_GENERAL);
		}
		break;
	case DEV_ATM_OVERLAY_CFG:
		{
			if(dwOutBufferSize < sizeof(DEV_ATM_OVERLAY_CONFIG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEV_ATM_OVERLAY_CONFIG* pAtmConfig = (DEV_ATM_OVERLAY_CONFIG *)lpOutBuffer;
			nRet = GetDevNewConfig_AtmOverlayConfig(lLoginID, pAtmConfig, waittime);
			if(nRet < 0)
			{
				return nRet;
			}

			*lpBytesReturned = sizeof(DEV_ATM_OVERLAY_CONFIG);
		}
		break;
	case DEV_DECODER_TOUR_CFG:
		{
			if(dwOutBufferSize < sizeof(DEVICE_DECODER_TOUR_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEVICE_DECODER_TOUR_CFG* pTourCfg = (DEVICE_DECODER_TOUR_CFG *)lpOutBuffer;
			nRet = GetDevNewConfig_DecoderTourConfig(lLoginID, lChannel, pTourCfg, waittime);
			if(nRet < 0)
			{
				return nRet;
			}

			*lpBytesReturned = sizeof(DEVICE_DECODER_TOUR_CFG);
		}
		break;
	case DEV_SIP_CFG:
		{
			if(dwOutBufferSize < sizeof(DEVICE_SIP_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEVICE_SIP_CFG* pSIPCfg = (DEVICE_SIP_CFG*)lpOutBuffer;
			nRet = GetDevNewConfig_SIPConfig(lLoginID, pSIPCfg, waittime);
			if(nRet < 0)
			{
				return nRet;
			}

			*lpBytesReturned = sizeof(DEVICE_SIP_CFG);
		}
		break;
	case DEV_POS_CFG:
		{
			if ( dwOutBufferSize < sizeof(DEVICE_POS_INFO) ) 
			{
				return NET_ILLEGAL_PARAM ;
			}
			DEVICE_POS_INFO* pPosCfg = (DEVICE_POS_INFO*)lpOutBuffer;
			nRet = GetDevNewConfig_POSConfig(lLoginID, pPosCfg, waittime);
			if(nRet < 0)
			{
				return nRet;
			}
			
			*lpBytesReturned = sizeof(DEVICE_SIP_CFG);

		}
		break ;
	default:
		break;
	}
	return nRet;
} 
//End of get  new config!


////function for setting new config 
int CDevConfigEx::SetDevNewConfig(LONG lLoginID, DWORD dwCommand,LONG lChannel, LPVOID lpInBuffer,DWORD dwInBufferSize,int waittime)
{
	if (!lpInBuffer)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;  //return value
	
	afk_device_s* device = (afk_device_s*)lLoginID;
	
	if (!device || m_pManager->IsDeviceValid(device) < 0)
	{
		return NET_INVALID_HANDLE;
	}

	switch(dwCommand)
	{
	case DEV_IPFILTER_CFG:				//IP 过滤配置
		{
			if (dwInBufferSize < sizeof(DEVICE_IPIFILTER_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVICE_IPIFILTER_CFG *pDevIPFilterCfg = NULL;
			pDevIPFilterCfg = (DEVICE_IPIFILTER_CFG *)lpInBuffer;
			
			nRet = SetDevNewConfig_IPFilter(lLoginID, pDevIPFilterCfg, waittime);
		}
		break;
	case DEV_TALK_ENCODE_CFG:				//语音对讲编码配置
		{
			if (dwInBufferSize < sizeof(DEVICE_TALK_ENCODE_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVICE_TALK_ENCODE_CFG *pDevTalkEncodeCfg = NULL;
			pDevTalkEncodeCfg = (DEVICE_TALK_ENCODE_CFG *)lpInBuffer;
			
			nRet = SetDevNewConfig_TalkEncode(lLoginID, pDevTalkEncodeCfg, waittime);
		}
		break;
	case DEV_RECORD_PACKET_CFG:				//录像打包长度配置
		{
			if (dwInBufferSize < sizeof(DEVICE_RECORD_PACKET_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVICE_RECORD_PACKET_CFG *pRecordLenCfg = NULL;
			pRecordLenCfg = (DEVICE_RECORD_PACKET_CFG *)lpInBuffer;
			
			nRet = SetDevNewConfig_RecordPacketLen(lLoginID, pRecordLenCfg, waittime);
		}
		break;
	case DEV_ALARM_DECODE_CFG:
		{
			int nChannel = 0;
			const int nMaxChlNum = 8;
			if(lChannel == -1)
			{
				nChannel = nMaxChlNum;
				if (dwInBufferSize < sizeof(ALARMDECODER_CFG)*nChannel)
				{
					return NET_ILLEGAL_PARAM;
				}
				
				ALARMDECODER_CFG *pAlarmDecCfg = (ALARMDECODER_CFG *)lpInBuffer;
				for(int i = 0; i < nMaxChlNum; i++)
				{
					nRet = SetDevNewConfig_AlmDecCfg(lLoginID, i, (pAlarmDecCfg+i), waittime);
					if (nRet < 0)
					{
						//break;
					}
				}		
			}
			else if(lChannel >= 0 && lChannel < nMaxChlNum)
			{
				nChannel = 1;
				if (dwInBufferSize < sizeof(ALARMDECODER_CFG)*nChannel)
				{
					return NET_ILLEGAL_PARAM;
				}
				
				ALARMDECODER_CFG *pAlarmDecCfg = (ALARMDECODER_CFG *)lpInBuffer;
				nRet = SetDevNewConfig_AlmDecCfg(lLoginID, lChannel, pAlarmDecCfg, waittime);
				if (nRet < 0)
				{
					return nRet;
				}
			}
			else
			{
				return -1;
			}	
		}
		break;
	case DEV_MMS_CFG:				//MMS配置
		{
			if (dwInBufferSize < sizeof(DEVICE_MMS_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVICE_MMS_CFG *pDevMMSCfg = NULL;
			pDevMMSCfg = (DEVICE_MMS_CFG *)lpInBuffer;
			
			nRet = SetDevNewConfig_MMS(lLoginID, pDevMMSCfg, waittime);
		}
		break;	
	case DEV_SMSACTIVATION_CFG:		// 短信激活无线连接配置
		{
			if (dwInBufferSize < sizeof(DEVICE_SMSACTIVATION_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVICE_SMSACTIVATION_CFG *pDevSMSActiveCfg = NULL;
			pDevSMSActiveCfg = (DEVICE_SMSACTIVATION_CFG *)lpInBuffer;
			
			nRet = SetDevNewConfig_SMSACTIVE(lLoginID, pDevSMSActiveCfg, waittime);
		}
		break;
	case DEV_DIALINACTIVATION_CFG:		// 拨号激活无线连接配置
		{
			if (dwInBufferSize < sizeof(DEVICE_DIALINACTIVATION_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVICE_DIALINACTIVATION_CFG *pDevDialCfg = NULL;
			pDevDialCfg = (DEVICE_DIALINACTIVATION_CFG *)lpInBuffer;
			
			nRet = SetDevNewConfig_DIALIN(lLoginID, pDevDialCfg, waittime);
		}
		break;
	case DEV_SNIFFER_CFG_EX:
		{
			if(dwInBufferSize < sizeof(YW_ATM_SNIFFER_CFG_EX))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			YW_ATM_SNIFFER_CFG_EX *pSnifferCfg = (YW_ATM_SNIFFER_CFG_EX *)lpInBuffer;
			
			nRet = SetDevNewConfig_Sniffer(lLoginID, pSnifferCfg, lChannel, waittime);
		}
		break;
	case DEV_DOWNLOAD_RATE_CFG:
		{
			if(dwInBufferSize < sizeof(int))
			{
				return NET_ILLEGAL_PARAM;
			}

			int *pDownloadRate = (int *)lpInBuffer;
			nRet = SetDevNewConfig_DownloadRate(lLoginID, pDownloadRate, waittime);
		}
		break;
	case DEV_PANORAMA_SWITCH_CFG:
		{
			if (dwInBufferSize < sizeof(ALARM_PANORAMA_SWITCH_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			ALARM_PANORAMA_SWITCH_CFG *lpPanoramaSwitch = (ALARM_PANORAMA_SWITCH_CFG *)lpInBuffer;

			nRet = SetDevNewConfig_PanoramaSwitch(lLoginID, lpPanoramaSwitch, waittime);
		}
		break;
	case DEV_LOST_FOCUS_CFG:
		{
			if (dwInBufferSize < sizeof(ALARM_LOST_FOCUS_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			ALARM_LOST_FOCUS_CFG *lpLostFocus = (ALARM_LOST_FOCUS_CFG *)lpInBuffer;

			nRet = SetDevNewConfig_LostFocus(lLoginID, lpLostFocus, waittime);
		}
		break;
	case DEV_VIDEOOUT_CFG:
		{
			if (dwInBufferSize < sizeof(DEV_VIDEOOUT_INFO))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEV_VIDEOOUT_INFO *lpVideoOut = (DEV_VIDEOOUT_INFO *)lpInBuffer;

			nRet = SetDevNewConfig_VideoOut(lLoginID, lpVideoOut, waittime);
		}
		break;
	case DEV_POINT_CFG:
		{
			if(dwInBufferSize < sizeof(DEVICE_POINT_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			DEVICE_POINT_CFG* pPointCfg = (DEVICE_POINT_CFG *)lpInBuffer;
			
			nRet = SetDevNewConfig_Point(lLoginID, pPointCfg, waittime);
		}
		break;
	case DEV_LOCALALARM_CFG://外部报警（本地报警）
		{
			int iAlarmNum = device->alarminputcount(device);
			if (lChannel < -1 || lChannel >= MAX_ALARM_IN_NUM_EX || lChannel >= iAlarmNum)
			{
				return NET_ILLEGAL_PARAM;
			}
			else if (-1 == lChannel)
			{
				if (dwInBufferSize < iAlarmNum * sizeof(ALARM_IN_CFG_EX))
				{
					return NET_ILLEGAL_PARAM;
				}
			}
			else
			{
				if (dwInBufferSize < sizeof(ALARM_IN_CFG_EX))
				{
					return NET_ILLEGAL_PARAM;
				}
			}
			ALARM_IN_CFG_EX *pLocalALMCfg = (ALARM_IN_CFG_EX *)lpInBuffer;
			
			nRet = SetDevNewConfig_LocalALMCfg(lLoginID, pLocalALMCfg, lChannel, waittime);
		}
		break;
	case DEV_NETALARM_CFG:
		{
			int iAlarmNum = device->alarminputcount(device);
		
			if (lChannel < -1 || lChannel >= MAX_ALARM_IN_NUM_EX || lChannel >= iAlarmNum)
			{
				return NET_ILLEGAL_PARAM;
			}
			else if (-1 == lChannel)
			{
				if (dwInBufferSize < iAlarmNum * sizeof(ALARM_IN_CFG_EX))
				{
					return NET_ILLEGAL_PARAM;
				}
			}
			else
			{
				if (dwInBufferSize < sizeof(ALARM_IN_CFG_EX))
				{
					return NET_ILLEGAL_PARAM;
				}
			}
			ALARM_IN_CFG_EX *pNetALMCfg = (ALARM_IN_CFG_EX *)lpInBuffer;
			
			nRet = SetDevNewConfig_NetALMCfg(lLoginID, pNetALMCfg, lChannel, waittime);
		}
		break;
	case DEV_MOTIONALARM_CFG :
		{
			int iAlarmNum = device->channelcount(device);
			int iChanCount = 1;
			if (lChannel < -1 || lChannel >= MAX_VIDEO_IN_NUM_EX || lChannel >= iAlarmNum)
			{
				return NET_ILLEGAL_PARAM;
			}
			else if (-1 == lChannel)
			{
				if (dwInBufferSize < iAlarmNum * sizeof(MOTION_DETECT_CFG_EX))
				{
					return NET_ILLEGAL_PARAM;
				}
			}
			else
			{
				if (dwInBufferSize < sizeof(MOTION_DETECT_CFG_EX))
				{
					return NET_ILLEGAL_PARAM;
				}
			}
			MOTION_DETECT_CFG_EX *pMotionALMCfg = (MOTION_DETECT_CFG_EX *)lpInBuffer;
			
			nRet = SetDevNewConfig_MotionALMCfg(lLoginID, pMotionALMCfg, lChannel, waittime);
		}
		break;
	case DEV_VIDEOLOSTALARM_CFG:
		{
			int iAlarmNum = device->channelcount(device);
			int iChanCount = 1;
			if (lChannel < -1 || lChannel >= MAX_VIDEO_IN_NUM_EX || lChannel >= iAlarmNum)
			{
				return NET_ILLEGAL_PARAM;
			}
			else if (-1 == lChannel)
			{
				iChanCount = iAlarmNum;
				if (dwInBufferSize < iAlarmNum * sizeof(VIDEO_LOST_CFG_EX))
				{
					return NET_ILLEGAL_PARAM;
				}
			}
			else
			{
				if (dwInBufferSize < sizeof(VIDEO_LOST_CFG_EX))
				{
					return NET_ILLEGAL_PARAM;
				}
			}
			VIDEO_LOST_CFG_EX *pLossALMCfg = (VIDEO_LOST_CFG_EX *)lpInBuffer;
			
			nRet = SetDevNewConfig_VideoLostALMCfg(lLoginID, pLossALMCfg, lChannel, waittime);
		}
		break;
	case DEV_BLINDALARM_CFG :
		{
			int iAlarmNum = device->channelcount(device);
			int iChanCount = 1;
			if (lChannel < -1 || lChannel >= MAX_VIDEO_IN_NUM_EX || lChannel >= iAlarmNum)
			{
				return NET_ILLEGAL_PARAM;
			}
			else if (-1 == lChannel)
			{
				if (dwInBufferSize < iAlarmNum * sizeof(BLIND_CFG_EX))
				{
					return NET_ILLEGAL_PARAM;
				}
			}
			else
			{
				if (dwInBufferSize < sizeof(BLIND_CFG_EX))
				{
					return NET_ILLEGAL_PARAM;
				}
			}
			BLIND_CFG_EX *pBlindALMCfg = (BLIND_CFG_EX *)lpInBuffer;
			
			nRet = SetDevNewConfig_BlindALMCfg(lLoginID, pBlindALMCfg, lChannel, waittime);
		}
		break;
	case DEV_DISKALARM_CFG:
		{
			if (dwInBufferSize < sizeof(DISK_ALARMIN_CFG_EX))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			DISK_ALARMIN_CFG_EX *lpDiskCfg = (DISK_ALARMIN_CFG_EX *)lpInBuffer;
			
			nRet = SetDevNewConfig_DiskALMCfg(lLoginID, lpDiskCfg, waittime);
		}
		break;
	case DEV_NETBROKENALARM_CFG:
		{
			if (dwInBufferSize < sizeof(NET_BROKEN_ALARM_CFG_EX))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			NET_BROKEN_ALARM_CFG_EX *lpNetBrokenCfg = (NET_BROKEN_ALARM_CFG_EX *)lpInBuffer;
			
			nRet = SetDevNewConfig_NetBrokenALMCfg(lLoginID, lpNetBrokenCfg, waittime);
		}
		break;
	case DEVICE_ENCODER_CFG:
		{
			if (dwInBufferSize < sizeof(DEV_ENCODER_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			DEV_ENCODER_CFG *lpEncoderCfg = (DEV_ENCODER_CFG *)lpInBuffer;
			
			nRet = SetDevNewConfig_EncoderCfg(lLoginID, lpEncoderCfg, waittime);
		}
		break;
	case DEV_IP_COLLISION_CFG:
		{
			if (dwInBufferSize < sizeof(ALARM_IP_COLLISION_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			ALARM_IP_COLLISION_CFG *lpIPCollisionCfg = (ALARM_IP_COLLISION_CFG *)lpInBuffer;
			
			nRet = SetDevNewConfig_IPCOllisionDetect(lLoginID, lpIPCollisionCfg, waittime);
		}
		break;
	case DEV_OSD_ENABLE_CFG:
		{
			if(dwInBufferSize < sizeof(DEV_OSD_ENABLE_CONFIG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEV_OSD_ENABLE_CONFIG *lpOSDEnableCfg = (DEV_OSD_ENABLE_CONFIG *)lpInBuffer;
			
			nRet = SetDevNewConfig_OSDEnable(lLoginID, lpOSDEnableCfg, waittime);
		}
		break;
	case DEV_TV_ADJUST_CFG:
		{
			if(dwInBufferSize < sizeof(DEVICE_TVADJUST_CFG) || lChannel == -1)
			{
				return NET_ILLEGAL_PARAM;
			}
			
			DEVICE_TVADJUST_CFG *lpTVAdjustCfg = (DEVICE_TVADJUST_CFG *)lpInBuffer;

			nRet = SetDevNewConfig_TVAdjustCfg(lLoginID, lChannel, lpTVAdjustCfg, waittime);
		
		}
		break;
	case DEV_ABOUT_VEHICLE_CFG:
		{
			if(dwInBufferSize < sizeof(DEV_ABOUT_VEHICLE_CONFIG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEV_ABOUT_VEHICLE_CONFIG* pVehicleCfg = (DEV_ABOUT_VEHICLE_CONFIG *)lpInBuffer;

			nRet = SetDevNewConfig_VehicleCfg(lLoginID, pVehicleCfg, waittime);
		}
		break;
	case DEV_ATM_OVERLAY_CFG:
		{
			if(dwInBufferSize < sizeof(DEV_ATM_OVERLAY_CONFIG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEV_ATM_OVERLAY_CONFIG* pAtmConfig = (DEV_ATM_OVERLAY_CONFIG*)lpInBuffer;

			nRet = SetDevNewConfig_AtmOverlayConfig(lLoginID, pAtmConfig, waittime);
		}
		break;
	case DEV_DECODER_TOUR_CFG:
		{
			if(dwInBufferSize < sizeof(DEVICE_DECODER_TOUR_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEVICE_DECODER_TOUR_CFG* pTourCfg = (DEVICE_DECODER_TOUR_CFG *)lpInBuffer;

			nRet = SetDevNewConfig_DecoderTourConfig(lLoginID, lChannel, pTourCfg, waittime);
		}
		break;
	case DEV_SIP_CFG:
		{
			if(dwInBufferSize < sizeof(DEVICE_SIP_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEVICE_SIP_CFG* pSIPCfg = (DEVICE_SIP_CFG*)lpInBuffer;
			nRet = SetDevNewConfig_SIPConfig(lLoginID, pSIPCfg, waittime);
		}
		break;
	default:
		break;
	}

	return nRet;
}
//End of set  new config!


//IP过滤功能（获取）
int	CDevConfigEx::GetDevNewConfig_IPFilter(LONG lLoginID, DEVICE_IPIFILTER_CFG &stIPFilter, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	memset(&stIPFilter, 0, sizeof(DEVICE_IPIFILTER_CFG));
	stIPFilter.dwSize = sizeof(DEVICE_IPIFILTER_CFG);

	int nRetLen = 0;
	int nRet = -1;
	char buffer[4096] = {0};

	// 先查询IP信息
	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_IPFILTER, NULL, buffer, 4096, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = GetProtocolValue(buffer, "Enable:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		stIPFilter.dwEnable = atoi(szValue);
		
		p = GetProtocolValue(buffer, "Type:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		stIPFilter.dwType = atoi(szValue);
		
		p = buffer;//解析白名单ip
		{
			for (int i = 0; i < IPIFILTER_NUM; i++)
			{
				p = GetProtocolValue(p, "TrustIp:", "\r\n", szValue, 64);
				if (p == NULL)
				{
					break;
				}
				strcpy(stIPFilter.TrustIP.SZIP[i], szValue);
				stIPFilter.TrustIP.dwIPNum++;	
			}
		}

		p = buffer;//解析黑名单ip
		{
			for (int i = 0; i < IPIFILTER_NUM; i++)
			{
				p = GetProtocolValue(p, "BannedIp:", "\r\n", szValue, 64);
				if (p == NULL)
				{
					break;
				}
				strcpy(stIPFilter.BannedIP.SZIP[i], szValue);
				stIPFilter.BannedIP.dwIPNum++;	
			}
		}
	}
	else
	{
		return NET_ERROR_GETCFG_IPFILTER;
	}
			
	return 0;
}

//IP过滤功能（设置）
int	CDevConfigEx::SetDevNewConfig_IPFilter(LONG lLoginID, DEVICE_IPIFILTER_CFG *pstIPFilter, int waittime)
{
	if (NULL == pstIPFilter)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;  //return value
	
	//set ip filter configure
	REQUEST_SET_IPFILTER tmpIpfilterCfg = {0};
	int nIPCount = 0;

	tmpIpfilterCfg.nEnable = pstIPFilter->dwEnable;//使能
	tmpIpfilterCfg.nType = pstIPFilter->dwType;//类型
	
	//白名单
	nIPCount = (pstIPFilter->TrustIP.dwIPNum > IPIFILTER_NUM) ? IPIFILTER_NUM : pstIPFilter->TrustIP.dwIPNum;
	tmpIpfilterCfg.nTrustIpNum = nIPCount;
	for (int i = 0; i < tmpIpfilterCfg.nTrustIpNum; i++)
	{
		strcpy(tmpIpfilterCfg.szTrustIp[i], pstIPFilter->TrustIP.SZIP[i]);
	}
	
	//黑名单
	nIPCount = (pstIPFilter->BannedIP.dwIPNum > IPIFILTER_NUM) ? IPIFILTER_NUM : pstIPFilter->BannedIP.dwIPNum;
	tmpIpfilterCfg.nBlackIpNum = nIPCount;
	for (int j = 0; j < tmpIpfilterCfg.nBlackIpNum; j++)
	{
		strcpy(tmpIpfilterCfg.szBlackIp[j], pstIPFilter->BannedIP.SZIP[j]);
	}
	
	nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_SET_IPFILTER, (void*)&tmpIpfilterCfg, waittime);
			
	if (nRet < 0 )
	{
		nRet = NET_ERROR_SETCFG_IPFILTER;
	}
	else
	{
		nRet = NET_NOERROR;
	}

	return nRet;
}


int CDevConfigEx::GetDevNewConfig_TalkEncode(LONG lLoginID, DEVICE_TALK_ENCODE_CFG &stTalkEncode, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	memset(&stTalkEncode, 0, sizeof(DEVICE_TALK_ENCODE_CFG));
	stTalkEncode.dwSize = sizeof(DEVICE_TALK_ENCODE_CFG);
	
	int nRetLen = 0;
	int nRet = -1;
	char buffer[1024] = {0};
	
	// 查询语音对讲编码配置
	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_AMRENCODE_CFG, NULL, buffer, 1024, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = GetProtocolValue(buffer, "Compression:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		stTalkEncode.nCompression = atoi(szValue);
		
		p = GetProtocolValue(buffer, "Mode:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		stTalkEncode.nMode = atoi(szValue);
	}
	else
	{
		return NET_ERROR_GETCFG_TALKENCODE;
	}
	
	return 0;	
}

int	CDevConfigEx::SetDevNewConfig_TalkEncode(LONG lLoginID, DEVICE_TALK_ENCODE_CFG *pstTalkEncode, int waittime)
{
	if (NULL == pstTalkEncode)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int nRet = -1;  //return value
	int nRetLen = 0;
	char buffer[1024] = {0};
	
	//set talk encode configure
	REQUEST_SET_TALKENCODE tmpTalkEncodeCfg = {0};
	tmpTalkEncodeCfg.nCompression = pstTalkEncode->nCompression;
	if (0 == tmpTalkEncodeCfg.nCompression)//语音对讲采用默认的方式，指定为PCM
	{
		tmpTalkEncodeCfg.nCompression += 1;
	}
	tmpTalkEncodeCfg.nMode = pstTalkEncode->nMode;

	nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_SET_AMRENCODE_CFG, (void*)&tmpTalkEncodeCfg, waittime);
	
	if (nRet < 0 )
	{
		nRet = NET_ERROR_SETCFG_IPFILTER;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	
	return nRet;
}

int CDevConfigEx::GetDevNewConfig_RecordPacketLen(LONG lLoginID, DEVICE_RECORD_PACKET_CFG &stRecordLen, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	memset(&stRecordLen, 0, sizeof(DEVICE_RECORD_PACKET_CFG));
	stRecordLen.dwSize = sizeof(DEVICE_RECORD_PACKET_CFG);
	
	int nRetLen = 0;
	int nRet = -1;
	char buffer[1024] = {0};
	
	// 查询录像打包长度配置
	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_RECORDLEN_CFG, NULL, buffer, 1024, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = GetProtocolValue(buffer, "nType:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		stRecordLen.nType = atoi(szValue);
		
		p = GetProtocolValue(buffer, "nValue:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		stRecordLen.nValue= atoi(szValue);
	}
	else
	{
		return NET_ERROR_GETCFG_RECORDLEN;
	}
	
	return 0;	
}

int	CDevConfigEx::SetDevNewConfig_RecordPacketLen(LONG lLoginID, DEVICE_RECORD_PACKET_CFG *pstRecordLen, int waittime)
{
	if (NULL == pstRecordLen)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int nRet = -1;  //return value
	int nRetLen = 0;
	char buffer[1024] = {0};
	
	//set configure
	REQUEST_SET_RECORDLEN tmpRecordLenCfg = {0};
	tmpRecordLenCfg.nType = pstRecordLen->nType;
	tmpRecordLenCfg.nValue = pstRecordLen->nValue;

	nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_SET_RECORDLEN_CFG, (void*)&tmpRecordLenCfg, waittime);
	
	if (nRet < 0 )
	{
		nRet = NET_ERROR_SETCFG_RECORDLEN;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	
	return nRet;
}

int	CDevConfigEx::GetDevNewConfig_AlmDecCfg(LONG lLoginID, int nChannel, ALARMDECODER_CFG *stAlmDec, int nWaitTime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == stAlmDec)
	{
		return -1;
	}

	int iRecvBufLen = 0;
	if (iRecvBufLen < sizeof(CONFIG_ALARMDECODER))
	{
		iRecvBufLen = sizeof(CONFIG_ALARMDECODER);
	}

	int nRet = -1;
	int retlen = 0;
	char *cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		return -1;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_DECODER, nChannel, cRecvBuf, iRecvBufLen, &retlen, nWaitTime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_ALARMDECODER))
	{
		CONFIG_ALARMDECODER *pCfgAlmDec = (CONFIG_ALARMDECODER *)cRecvBuf;
		stAlmDec->bEnable = pCfgAlmDec->bEnable;
		stAlmDec->dwAddr = pCfgAlmDec->iAddress;
		memset(stAlmDec->dwOutSlots, -1, sizeof(DWORD)*DECODER_OUT_SLOTS_MAX_NUM);
		memcpy(stAlmDec->dwOutSlots, pCfgAlmDec->iOutSlots, sizeof(DWORD)*ALARMDEC_OUT_SLOTS_NUM);
		stAlmDec->nOutSlotNum = ALARMDEC_OUT_SLOTS_NUM;

		ALARM_MSG_HANDLE* pMsg = NULL;
		EVENT_HANDLER* pEvent = NULL;

		int i = 0;
		int j = 0;
		stAlmDec->nMsgHandleNum = ALARMDEC_IN_SLOTS_NUM;
		for(i = 0; i < ALARMDEC_IN_SLOTS_NUM; i++)
		{
			pMsg = &(stAlmDec->struHandle[i]);
			pEvent = &(pCfgAlmDec->InSlots[i].hEvent);

			pMsg->bMessageToNet = pEvent->bMessagetoNet;
			pMsg->dwDuration = pEvent->iAOLatch;
			pMsg->dwEventLatch = pEvent->iEventLatch;
			pMsg->dwRecLatch = pEvent->iRecordLatch;
			
			m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_DECODER, &(pMsg->dwActionMask));
			m_pManager->GetDevConfig().GetAlmActionFlag(*pEvent, &(pMsg->dwActionFlag));	
			
			int index = min(32, MAX_VIDEO_IN_NUM);
			DWORD dwMask = 0x0001;
			for (j = 0; j < index; j++)
			{
				pMsg->byRecordChannel[j] = (pEvent->dwRecord & dwMask) > 0 ? 1 : 0;
				pMsg->bySnap[j] = (pEvent->dwSnapShot & dwMask) > 0 ? 1 : 0;
				pMsg->byTour[j] = (pEvent->dwTour & dwMask) > 0 ? 1 : 0;
				pMsg->struPtzLink[j].iType = pEvent->PtzLink[j].iType;
				pMsg->struPtzLink[j].iValue = pEvent->PtzLink[j].iValue;
				
				dwMask <<= 1;
			}

			index = min(32, MAX_ALARM_OUT_NUM);
			dwMask = 0x0001;
			for(j = 0; j < index; j++)
			{
				pMsg->byRelAlarmOut[j] = (pEvent->dwAlarmOut & dwMask) > 0 ? 1 : 0;
				pMsg->byRelWIAlarmOut[j] = (pEvent->wiAlarmOut & dwMask) > 0 ? 1 : 0;

				dwMask <<= 1;
			}
		}
		
	}
	else
	{
		delete []cRecvBuf;
		return -1;
	}
	
	CONFIG_WORKSHEET WorkSheet = {0};
	nRet = m_pManager->GetDevConfig().GetDevConfig_WorkSheet(lLoginID, WSHEET_DECODER_ALARM, &WorkSheet, nWaitTime, 1, nChannel);
	if (nRet >= 0)
	{
		memcpy(stAlmDec->stSect, WorkSheet.tsSchedule, sizeof(WorkSheet.tsSchedule));
	}
	else
	{
		delete []cRecvBuf;
		return -1;
	}
		
	delete []cRecvBuf;
	return 1;
}

int	CDevConfigEx::SetDevNewConfig_AlmDecCfg(LONG lLoginID, int nChannel, ALARMDECODER_CFG *stAlmDec, int nWaitTime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == stAlmDec)
	{
		return -1;
	}
	
	int iRecvBufLen = 0;
	if (iRecvBufLen < sizeof(CONFIG_ALARMDECODER))
	{
		iRecvBufLen = sizeof(CONFIG_ALARMDECODER);
	}
	
	int nRet = -1;
	int retlen = 0;
	char *cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		return -1;
	}

	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_DECODER, nChannel, cRecvBuf, iRecvBufLen, &retlen, nWaitTime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_ALARMDECODER))
	{
		CONFIG_ALARMDECODER *pCfgAlmDec = (CONFIG_ALARMDECODER *)cRecvBuf;

		pCfgAlmDec->bEnable = stAlmDec->bEnable;
		pCfgAlmDec->iAddress = stAlmDec->dwAddr;

		memcpy(pCfgAlmDec->iOutSlots, stAlmDec->dwOutSlots, sizeof(DWORD)*ALARMDEC_OUT_SLOTS_NUM);
		
		ALARM_MSG_HANDLE* pMsg = NULL;
		EVENT_HANDLER* pEvent = NULL;
		for(int i = 0; i < ALARMDEC_IN_SLOTS_NUM; i++)
		{
			pMsg = &(stAlmDec->struHandle[i]);
			pEvent = &(pCfgAlmDec->InSlots[i].hEvent);
			
			pEvent->bMessagetoNet = pMsg->bMessageToNet;
			pEvent->iAOLatch = pMsg->dwDuration;
			pEvent->iEventLatch = pMsg->dwEventLatch;
			pEvent->iRecordLatch = pMsg->dwRecLatch;
			
			m_pManager->GetDevConfig().SetAlmActionFlag(pEvent, pMsg->dwActionFlag);	
			
			int index = min(32, MAX_VIDEO_IN_NUM);
			pEvent->dwRecord = 0;
			pEvent->dwSnapShot = 0;
			pEvent->dwTour = 0;
			int j = 0;
			for (j = 0; j < index; j++)
			{
				pEvent->dwRecord |= (pMsg->byRecordChannel[j] > 0 ) ? (0x0001<<j) : 0;
				pEvent->dwSnapShot |= (pMsg->bySnap[j] > 0) ? (0x0001<<j) : 0;
				pEvent->dwTour |= (pMsg->byTour[j] > 0) ? (0x0001<<j) : 0;
				pEvent->PtzLink[j].iType = pMsg->struPtzLink[j].iType;
				pEvent->PtzLink[j].iValue = pMsg->struPtzLink[j].iValue;
			}
			
			index = min(32, MAX_ALARM_OUT_NUM);
			pEvent->dwAlarmOut = 0;
			pEvent->wiAlarmOut = 0;
			for(j = 0; j < index; j++)
			{
				pEvent->dwAlarmOut |= (pMsg->byRelAlarmOut[j] > 0) ? (0x0001<<j) : 0;
				pEvent->wiAlarmOut |= (pMsg->byRelWIAlarmOut[j] > 0) ? (0x0001<<j) : 0;
			}
		}

		nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_ALARM_DECODER, nChannel, cRecvBuf, iRecvBufLen, nWaitTime);
		if(nRet < 0)
		{
			delete []cRecvBuf;
			return nRet;
		}
		
	}
	else
	{
		delete []cRecvBuf;
		return -1;
	}
	 
	CONFIG_WORKSHEET WorkSheet = {0};
	WorkSheet.iName = nChannel;
	memcpy(WorkSheet.tsSchedule, stAlmDec->stSect, sizeof(WorkSheet.tsSchedule));
	nRet = m_pManager->GetDevConfig().SetDevConfig_WorkSheet(lLoginID, WSHEET_DECODER_ALARM, &WorkSheet, nWaitTime, 1, nChannel);

	delete[] cRecvBuf;
	return nRet;
}

//MMS（获取）
int	CDevConfigEx::GetDevNewConfig_MMS(LONG lLoginID, DEVICE_MMS_CFG &stMMS, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	memset(&stMMS, 0, sizeof(DEVICE_MMS_CFG));
	stMMS.dwSize = sizeof(DEVICE_MMS_CFG);

	int nRetLen = 0;
	int nRet = -1;
	char buffer[1024] = {0};

	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_MMS_INFO, NULL, buffer, 1024, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = GetProtocolValue(buffer, "Enable:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		stMMS.dwEnable = atoi(szValue);
		
		p = buffer;//解析接收者
		{
			for (int i = 0; i < MMS_RECEIVER_NUM; i++)
			{
				p = GetProtocolValue(p, "Receivers:", "\r\n", szValue, 64);
				if (p == NULL)
				{
					break;
				}
				
				strncpy(stMMS.SZReceiver[i], szValue, 32);
				stMMS.dwReceiverNum++;
			}
		}
		
		p = GetProtocolValue(buffer, "Type:", "\r\n", szValue, 64);
		if (p)
		{
			stMMS.byType = atoi(szValue);
		}

		p = GetProtocolValue(buffer, "Title:", "\r\n", szValue, 64);
		if (p)
		{
			char szTitleName[64];
			memset(szTitleName, 0, sizeof(szTitleName));
			Change_Utf8_Assic((unsigned char *)szValue, szTitleName);
			strncpy(stMMS.SZTitle, szTitleName, 32);
		}

	}
	else
	{
		return NET_ERROR_GETCFG_MMS;
	}
			
	return 0;
}

//MMS（设置）
int	CDevConfigEx::SetDevNewConfig_MMS(LONG lLoginID, DEVICE_MMS_CFG *pstMMS, int waittime)
{
	if (NULL == pstMMS)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;  //return value
	char buffer[1024] = {0};
	int nRetLen = 0;
	
	//set MMS configure
	REQUEST_SET_MMS tmpMMS = {0};
	int nIPCount = 0;

	tmpMMS.nEnable = pstMMS->dwEnable;//使能
	
	nIPCount = (pstMMS->dwReceiverNum > MMS_RECEIVER_NUM) ? MMS_RECEIVER_NUM : pstMMS->dwReceiverNum;
	tmpMMS.nReceiverNum = nIPCount;
	for (int i = 0; i < tmpMMS.nReceiverNum; i++)
	{
		strcpy(tmpMMS.szReceiver[i], pstMMS->SZReceiver[i]);
	}
	tmpMMS.bType = pstMMS->byType;

	int subLen = (strlen(pstMMS->SZTitle)>32) ? 32 : strlen(pstMMS->SZTitle);
	char szTitleUTF8[32*2+2] = {0};
	Change_Assic_UTF8(pstMMS->SZTitle, subLen, szTitleUTF8, 32*2+2);
	subLen = strlen(szTitleUTF8);
//	strcpy(tmpMMS.SZTitle, pstMMS->SZTitle);
	strncpy(tmpMMS.SZTitle, szTitleUTF8, subLen);

	nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_SET_MMS_INFO, (void*)&tmpMMS, waittime);
			
	if (nRet < 0 )
	{
		nRet = NET_ERROR_SETCFG_MMS;
	}
	else
	{
		nRet = NET_NOERROR;
	}

	return nRet;
}


//SMSACTIVE（获取）
int	CDevConfigEx::GetDevNewConfig_SMSACTIVE(LONG lLoginID, DEVICE_SMSACTIVATION_CFG &stSMSActive, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	memset(&stSMSActive, 0, sizeof(DEVICE_SMSACTIVATION_CFG));
	stSMSActive.dwSize = sizeof(DEVICE_SMSACTIVATION_CFG);

	int nRetLen = 0;
	int nRet = -1;
	char buffer[1024] = {0};

	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_SMSACTIVE_INFO, NULL, buffer, 1024, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = GetProtocolValue(buffer, "Enable:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		stSMSActive.dwEnable = atoi(szValue);
		
		p = buffer;//解析发送者
		{
			for (int i = 0; i < MMS_SMSACTIVATION_NUM; i++)
			{
				p = GetProtocolValue(p, "Senders:", "\r\n", szValue, 64);
				if (p == NULL)
				{
					break;
				}
				
				strcpy(stSMSActive.SZSender[i], szValue);
				stSMSActive.dwSenderNum++;
			}
		}
	}
	else
	{
		return NET_ERROR_GETCFG_SMSACTIVATION;
	}
			
	return 0;
}

//SMSACTIVE（设置）
int	CDevConfigEx::SetDevNewConfig_SMSACTIVE(LONG lLoginID, DEVICE_SMSACTIVATION_CFG *pstSMSActive, int waittime)
{
	if (NULL == pstSMSActive)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;  //return value
	char buffer[1024] = {0};
	int nRetLen = 0;
	
	//set SMSActive configure
	REQUEST_SET_SMSACTIVATION tmpSMSActive = {0};
	int nIPCount = 0;

	tmpSMSActive.nEnable = pstSMSActive->dwEnable;//使能
	
	nIPCount = (pstSMSActive->dwSenderNum > MMS_SMSACTIVATION_NUM) ? MMS_SMSACTIVATION_NUM : pstSMSActive->dwSenderNum;
	tmpSMSActive.nSenderNum = nIPCount;
	for (int i = 0; i < tmpSMSActive.nSenderNum; i++)
	{
		strcpy(tmpSMSActive.szSender[i], pstSMSActive->SZSender[i]);
	}
	
	nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_SET_SMSACTIVE_INFO, (void*)&tmpSMSActive, waittime);
			
	if (nRet < 0 )
	{
		nRet = NET_ERROR_SETCFG_SMSACTIVATION;
	}
	else
	{
		nRet = NET_NOERROR;
	}

	return nRet;
}


//DIALIN（获取）
int	CDevConfigEx::GetDevNewConfig_DIALIN(LONG lLoginID, DEVICE_DIALINACTIVATION_CFG &stDIALIN, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	memset(&stDIALIN, 0, sizeof(DEVICE_DIALINACTIVATION_CFG));
	stDIALIN.dwSize = sizeof(DEVICE_DIALINACTIVATION_CFG);

	int nRetLen = 0;
	int nRet = -1;
	char buffer[1024] = {0};

	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_DIALIN_INFO, NULL, buffer, 1024, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = GetProtocolValue(buffer, "Enable:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		stDIALIN.dwEnable = atoi(szValue);
		
		p = buffer;//解析Caller
		{
			for (int i = 0; i < MMS_DIALINACTIVATION_NUM; i++)
			{
				p = GetProtocolValue(p, "Callers:", "\r\n", szValue, 64);
				if (p == NULL)
				{
					break;
				}
				
				strcpy(stDIALIN.SZCaller[i], szValue);
				stDIALIN.dwCallerNum++;
			}
		}
	}
	else
	{
		return NET_ERROR_GETCFG_DIALINACTIVATION;
	}
			
	return 0;
}

//DIALIN（设置）
int	CDevConfigEx::SetDevNewConfig_DIALIN(LONG lLoginID, DEVICE_DIALINACTIVATION_CFG *pstDIALIN, int waittime)
{
	if (NULL == pstDIALIN)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;  //return value
	char buffer[1024] = {0};
	int nRetLen = 0;
	
	//set DIALIN configure
	REQUEST_SET_DIALINACTIVATION tmpDIALIN = {0};
	int nIPCount = 0;

	tmpDIALIN.nEnable = pstDIALIN->dwEnable;//使能
	
	nIPCount = (pstDIALIN->dwCallerNum > MMS_DIALINACTIVATION_NUM) ? MMS_DIALINACTIVATION_NUM : pstDIALIN->dwCallerNum;
	tmpDIALIN.nCallerNum = nIPCount;
	for (int i = 0; i < tmpDIALIN.nCallerNum; i++)
	{
		strcpy(tmpDIALIN.szCaller[i], pstDIALIN->SZCaller[i]);
	}
	
	nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_SET_DIALIN_INFO, (void*)&tmpDIALIN, waittime);
			
	if (nRet < 0 )
	{
		nRet = NET_ERROR_SETCFG_DIALINACTIVATION;
	}
	else
	{
		nRet = NET_NOERROR;
	}

	return nRet;
}


LONG CDevConfigEx::SetDevNewConfig_FileTrans(LONG lLoginID, int nTransType, char* szInBuf, int nInBufLen, fTransFileCallBack cbTransFile, DWORD dwUserData, int waittime)
{
	if(NULL == szInBuf || nInBufLen <= 0)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;
	int nRetLen = 0;
	char buffer[1024] = {0};

	switch(nTransType) 
	{
	case DEV_FILETRANS_BURN:
		{
			if(nInBufLen < sizeof(DEV_BURNFILE_TRANS_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
#ifdef _DEBUG_
			CTaskBurnFileTrans* pTask = (CTaskBurnFileTrans*)m_pManager->GetDynamicThread().CreateTask(
																					CDynamicThread::eBurnFileTrans, 
																					CDynamicThread::eFromTaskDone, 
																					1000*10, 
																					0);

			if(NULL == pTask)
			{
				return -1;
			}
			
			pTask->SetParam(m_pManager, "D:\\test.dat", 1111111, "10.7.5.21", 30001, cbTransFile, dwUserData, waittime);
			m_pManager->GetDynamicThread().Run();
			return (LONG)pTask;
#endif
			DEV_BURNFILE_TRANS_CFG* pFileTrans = (DEV_BURNFILE_TRANS_CFG *)szInBuf;
			//获取文件长度
			if(pFileTrans->nFileSize <= 0)
			{
				FILE* pFile = fopen(pFileTrans->szSourFile, "rb");
				if(NULL == pFile)
				{
					return NET_ILLEGAL_PARAM;
				}
				fseek(pFile, 0, SEEK_END);
				fpos_t fLen;
				fgetpos(pFile, &fLen);
#ifdef WIN32
				pFileTrans->nFileSize = (int)fLen;
#else	//linux
				pFileTrans->nFileSize = fLen.__pos;
#endif
				fclose(pFile);
			}
			
			int nConType = channel_connect_tcp;
			int nConID  = 0;
			char szIP[64] = {0};
			int nPort = 0;

			nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_CONNECT, (void*)nConType, buffer, 1024, &nRetLen, waittime);
			if(nRet >= 0 && nRetLen > 0)
			{
				char szValue[64] = {0};
				char *p = GetProtocolValue(buffer, "ConnectionID:", "\r\n", szValue, 64);
				if (p == NULL)
				{
					return NET_RETURN_DATA_ERROR;
				}
				nConID = atoi(szValue);
				
				p = GetProtocolValue(buffer, "IP:", "\r\n", szValue, 64);
				if(p == NULL)
				{
					return NET_RETURN_DATA_ERROR;
				}

				memcpy(szIP, szValue, 64);
				
				p = GetProtocolValue(buffer, "Port:", "\r\n", szValue, 64);
				if(p == NULL)
				{
					return NET_RETURN_DATA_ERROR;
				}
				nPort = atoi(szValue);			
			}
			else
			{
				return NET_ERROR_CONNECT_FAILED;
			}

			//复制要打包发送的值
			REQUEST_SET_BURNFILE tmpBurnFile = {0};
			tmpBurnFile.nFileSize = pFileTrans->nFileSize;
			tmpBurnFile.dwConID = nConID;
			memcpy(tmpBurnFile.szFileName, pFileTrans->szBurnFile, MAX_PATH_STOR);
			//请求建立连接
			nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_SET_BURNFILE_TRANS, (void*)&tmpBurnFile, buffer, 1024, &nRetLen, waittime);
			if(nRet >= 0 && nRetLen > 0)
			{	// 请求成功

				CTaskBurnFileTrans* pTask = (CTaskBurnFileTrans*)m_pManager->GetDynamicThread().CreateTask(
																					CDynamicThread::eBurnFileTrans, 
																					CDynamicThread::eFromTaskDone, 
																					1000*10, 
																					nConID);
				
				if(NULL == pTask)
				{
					return -1;
				}

				pTask->SetParam(m_pManager, pFileTrans->szSourFile, pFileTrans->nFileSize, szIP, nPort, cbTransFile, dwUserData, waittime);
				m_pManager->GetDynamicThread().Run();
				return (LONG)pTask;
			}
			else
			{
				return NET_ERROR_SETCFG_BURNFILE;
			}
		}
		break;
	case DEV_FILETRANS_STOP:
		{
			if(szInBuf == NULL || nInBufLen < sizeof(LONG))
			{
				return NET_ILLEGAL_PARAM;
			}

			LONG lHandle = *((LONG *)szInBuf);
			CTask *pTask = (CTask *)lHandle;
			DWORD dwConID = 0;
			int nRet = m_pManager->GetDynamicThread().GetUserData(pTask, dwConID);
			if(nRet < 0)
			{
				return 1;//对象找不到，也有可能已经销毁，所以返回成功.
			}

			//set configure
			nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_DISCONNECT, (void*)dwConID, waittime);
			if (nRet < 0 )
			{
				nRet = NET_ERROR_SETCFG_RECORDLEN;
			}
			else
			{
				nRet = NET_NOERROR;
			}
			
			m_pManager->GetDynamicThread().DeleteTask(pTask);
			return 1;
		}
		break;
	default://不存在的类型
		return NET_ILLEGAL_PARAM;
		break;
	}
	return NET_ILLEGAL_PARAM;

}


int CDevConfigEx::GetDevNewConfig_Sniffer(LONG lLoginID, YW_ATM_SNIFFER_CFG_EX* pSnifferCfgEx, int nGroupID, int waittime)
{
	// bulid string
	char szBuf[1024*16];
	memset(szBuf, 0, 1024*16);
	
	sprintf(szBuf+strlen(szBuf), "Method:GetParameterValues\r\n");

	sprintf(szBuf+strlen(szBuf), "<Envelope>\r\n");
	sprintf(szBuf+strlen(szBuf), "ParameterName:Dahua.Device.Network.Sniffer.Cfg.Group\r\n");
	sprintf(szBuf+strlen(szBuf), "GroupID:%d\r\n", nGroupID+1);
	sprintf(szBuf+strlen(szBuf), "</Envelope>\r\n");
	
	for(int i = 0; i < SNIFFER_FRAMEID_NUM; i++)
	{
		sprintf(szBuf+strlen(szBuf), "<Envelope>\r\n");
		sprintf(szBuf+strlen(szBuf), "ParameterName:Dahua.Device.Network.Sniffer.Cfg.Group.Frame\r\n");
		sprintf(szBuf+strlen(szBuf), "GroupID:%d\r\n", nGroupID+1);
		sprintf(szBuf+strlen(szBuf), "FrameID:%d\r\n", i+1);
		sprintf(szBuf+strlen(szBuf), "</Envelope>\r\n");

		for(int j =0; j < SNIFFER_CONTENT_NUM_EX; j++)
		{
			sprintf(szBuf+strlen(szBuf), "<Envelope>\r\n");
			sprintf(szBuf+strlen(szBuf), "ParameterName:Dahua.Device.Network.Sniffer.Cfg.Group.Frame.Content\r\n");
			sprintf(szBuf+strlen(szBuf), "GroupID:%d\r\n", nGroupID+1);
			sprintf(szBuf+strlen(szBuf), "FrameID:%d\r\n", i+1);
			sprintf(szBuf+strlen(szBuf), "ContentID:%d\r\n", j+1);
			sprintf(szBuf+strlen(szBuf), "</Envelope>\r\n");
		}
	}
	sprintf(szBuf+strlen(szBuf), "\r\n");

#ifdef _DEBUG
	int nLen = strlen(szBuf);
#endif

	//
	int nRet = -1;
	int nRetLen = 0;
	char buffer[300*55];
	memset(buffer, 0, 300*55);
 	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_SNIFFER, (void*)&szBuf, buffer, 300*55, &nRetLen, waittime);
	if(nRet < 0  || nRetLen <= 0)
	{
		return NET_ERROR_SNIFFER_GETCFG;
	}

	CStrParse Parser;
	Parser.setSpliter("</Envelope>");
	Parser.Parse(buffer);
	int nSize = Parser.Size();
	for(int j = 0; j < nSize; j++)
	{
		std::string strSection = Parser.getWord();
		strSection += "\r\n";
		char *pszSour = (char *)(strSection.c_str());
		
		char szValue[64] = {0};
		char *p = GetProtocolValue(pszSour, "ParameterName:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			continue;
		}

		if( _stricmp(szValue,"Dahua.Device.Network.Sniffer.Cfg.Group") == 0)
		{
			p = GetProtocolValue(pszSour, "GroupID:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				continue;
			}
			int nGroup = atoi(szValue);

			nRet = GetDevNewConfig_SnifferGroup(p, pSnifferCfgEx);
			if(nRet < 0)
			{
				memset(pSnifferCfgEx, 0, sizeof(YW_ATM_SNIFFER_CFG_EX));
				return nRet;
			}
		}
		else if( _stricmp(szValue, "Dahua.Device.Network.Sniffer.Cfg.Group.Frame") == 0)
		{
			p = GetProtocolValue(pszSour, "GroupID:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				continue;
			}
			int nGroup = atoi(szValue);
			p = GetProtocolValue(pszSour, "FrameID:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				continue;
			}
			int nFrame = atoi(szValue);

			nRet = GetDevNewConfig_SnifferFrame(p, &(pSnifferCfgEx->snifferFrame[nFrame-1].snifferFrameId));
			if(nRet < 0)
			{
				memset(pSnifferCfgEx, 0, sizeof(YW_ATM_SNIFFER_CFG_EX));
				return nRet;
			}
		}
		else if( _stricmp(szValue, "Dahua.Device.Network.Sniffer.Cfg.Group.Frame.Content") == 0)
		{
			p = GetProtocolValue(pszSour, "GroupID:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				continue;
			}
			int nGroup = atoi(szValue);
			p = GetProtocolValue(pszSour, "FrameID:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				continue;
			}
			int nFrame = atoi(szValue);
			p = GetProtocolValue(pszSour, "ContentID:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				continue;
			}
			int nContent = atoi(szValue);

			nRet = GetDevNewConfig_SnifferContent(p, &(pSnifferCfgEx->snifferFrame[nFrame-1].snifferContent[nContent-1]));
			if(nRet < 0)
			{
				memset(pSnifferCfgEx, 0, sizeof(YW_ATM_SNIFFER_CFG_EX));
				return nRet;
			}
		}
	}
	return nRet;
}

int CDevConfigEx::GetDevNewConfig_SnifferGroup(char* szSour, YW_ATM_SNIFFER_CFG_EX* pSnifferCfgEx)
{
	if(NULL == pSnifferCfgEx || NULL == szSour)
	{
		return NET_ILLEGAL_PARAM;
	}

	memset(pSnifferCfgEx, 0 , sizeof(YW_ATM_SNIFFER_CFG_EX));

	char szValue[64] = {0};
	char *p = GetProtocolValue(szSour, "SrcIP:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	strcpy(pSnifferCfgEx->SnifferSrcIP, szValue);

	p = GetProtocolValue(szSour, "SrcPort:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	pSnifferCfgEx->SnifferSrcPort = atoi(szValue);

	p = GetProtocolValue(szSour, "DestIP:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	strcpy(pSnifferCfgEx->SnifferDestIP, szValue);

	p = GetProtocolValue(szSour, "DestPort:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	pSnifferCfgEx->SnifferDestPort = atoi(szValue);

	p = GetProtocolValue(szSour, "ProtocolEnable:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	pSnifferCfgEx->bProtocolEnable = (atoi(szValue) > 0);

	p = GetProtocolValue(szSour, "ProtocolName:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	strcpy(pSnifferCfgEx->szProtocolName, szValue);

	p = GetProtocolValue(szSour, "DateScopeEnable:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	pSnifferCfgEx->bDateScopeEnable = atoi(szValue);

	p = GetProtocolValue(szSour, "DisplayPositon:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	pSnifferCfgEx->displayPosition = atoi(szValue);

	p = GetProtocolValue(szSour, "ChannelMask:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	pSnifferCfgEx->recdChannelMask = atoi(szValue);

	p = GetProtocolValue(szSour, "SnifferMode:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	pSnifferCfgEx->nSnifferMode = atoi(szValue);
	
	return 1;
}

int CDevConfigEx::GetDevNewConfig_SnifferFrame(char* szSour, YW_SNIFFER_FRAMEID* pFrame)
{
	if(NULL == pFrame || NULL == szSour)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	memset(pFrame, 0, sizeof(YW_SNIFFER_FRAMEID));

	char szValue[64] = {0};
	char *p = GetProtocolValue(szSour, "Offset:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	pFrame->Offset = atoi(szValue);

	p = GetProtocolValue(szSour, "Length:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	pFrame->Length = atoi(szValue);

	p = GetProtocolValue(szSour, "Key:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	strcpy(pFrame->Key, szValue);

	
	return 1;
}

int CDevConfigEx::GetDevNewConfig_SnifferContent(char*szSour, YW_SNIFFER_CONTENT* pContent)
{
	if(NULL == pContent || NULL == szSour)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	memset(pContent, 0, sizeof(YW_SNIFFER_CONTENT));

	char szValue[64] = {0};
	char *p = GetProtocolValue(szSour, "Offset:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	pContent->Offset = atoi(szValue);

	p = GetProtocolValue(szSour, "Offset2:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	pContent->Offset2 = atoi(szValue);

	p = GetProtocolValue(szSour, "Length:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	pContent->Length = atoi(szValue);

	p = GetProtocolValue(szSour, "Length2:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	pContent->Length2 = atoi(szValue);

	p = GetProtocolValue(szSour, "KeyTitle:", "\r\n", szValue, 64);
	if (p == NULL)
	{
		return NET_RETURN_DATA_ERROR;
	}
	strcpy(pContent->KeyTitle, szValue);

	return 1;
}



int CDevConfigEx::SetDevNewConfig_Sniffer(LONG lLoginID, YW_ATM_SNIFFER_CFG_EX* pSnifferCfgEx, int nGroupID, int waittime)
{
	int nRetLen = 0;
	char szTmpBuf[512];
	char szSendBuf[300*55];
	memset(szSendBuf, 0, 300*55*sizeof(char));
	
	strcat(szSendBuf, "Method:SetParameterValues\r\n");

	nRetLen = SetDevNewConfig_SnifferGroup(pSnifferCfgEx, nGroupID+1, szTmpBuf, 512);
	if(nRetLen < 512)
	{
		strcat(szSendBuf, szTmpBuf);
	}
	for(int i = 0; i < SNIFFER_FRAMEID_NUM; i++)
	{	
		nRetLen = SetDevNewConfig_SnifferFrame(&(pSnifferCfgEx->snifferFrame[i].snifferFrameId), nGroupID+1, i+1, szTmpBuf, 512);
		if(nRetLen < 512)
		{
			strcat(szSendBuf, szTmpBuf);
		}

		for(int j =0; j < SNIFFER_CONTENT_NUM_EX; j++)
		{
			nRetLen = SetDevNewConfig_SnifferContent(&(pSnifferCfgEx->snifferFrame[i].snifferContent[j]), nGroupID+1, i+1, j+1, szTmpBuf, 512);
			if(nRetLen < 512)
			{
				strcat(szSendBuf, szTmpBuf);
			}
		}
	}
	strcat(szSendBuf, "\r\n");
#ifdef _DEBUG
	int nLen = strlen(szSendBuf);
#endif

	int	nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_SET_SNIFFER, (void*)&szSendBuf, waittime);
	if (nRet < 0 )
	{
		nRet = NET_ERROR_SNIFFER_SETCFG;
	}
	else
	{
		nRet = NET_NOERROR;
	}

	return nRet;
}

int CDevConfigEx::SetDevNewConfig_SnifferGroup(YW_ATM_SNIFFER_CFG_EX* pSnifferCfgEx, int nGroupID,  char* szBuf, int nBufLen)
{
	if(NULL == pSnifferCfgEx || NULL == szBuf)
	{
		return NET_ILLEGAL_PARAM;
	}

	memset(szBuf, 0, nBufLen);
	
	strcat(szBuf, "<Envelope>\r\n");
	strcat(szBuf, "ParameterName:Dahua.Device.Network.Sniffer.Cfg.Group\r\n");
	sprintf(szBuf+strlen(szBuf), "GroupID:%d\r\n", nGroupID);
	pSnifferCfgEx->SnifferSrcIP[MAX_IP_ADDRESS_LEN-1] = '\0';
	sprintf(szBuf+strlen(szBuf), "SrcIP:%s\r\n", pSnifferCfgEx->SnifferSrcIP);
	sprintf(szBuf+strlen(szBuf), "SrcPort:%d\r\n", pSnifferCfgEx->SnifferSrcPort);
	pSnifferCfgEx->SnifferDestIP[MAX_IP_ADDRESS_LEN-1] = '\0';
	sprintf(szBuf+strlen(szBuf), "DestIP:%s\r\n", pSnifferCfgEx->SnifferDestIP);
	sprintf(szBuf+strlen(szBuf), "DestPort:%d\r\n", pSnifferCfgEx->SnifferDestPort);
	sprintf(szBuf+strlen(szBuf), "ProtocolEnable:%d\r\n", pSnifferCfgEx->bProtocolEnable > 0);
	pSnifferCfgEx->szProtocolName[SNIFFER_PROTOCOL_SIZE-1] = '\0';
	sprintf(szBuf+strlen(szBuf), "ProtocolName:%s\r\n", pSnifferCfgEx->szProtocolName);
	sprintf(szBuf+strlen(szBuf), "DateScopeEnable:%d\r\n", pSnifferCfgEx->bDateScopeEnable);
	sprintf(szBuf+strlen(szBuf), "DisplayPositon:%d\r\n", pSnifferCfgEx->displayPosition);
	sprintf(szBuf+strlen(szBuf), "ChannelMask:%d\r\n", pSnifferCfgEx->recdChannelMask);
	sprintf(szBuf+strlen(szBuf), "SnifferMode:%d\r\n", pSnifferCfgEx->nSnifferMode);
	strcat(szBuf, "</Envelope>\r\n");
	return strlen(szBuf);
}

int CDevConfigEx::SetDevNewConfig_SnifferFrame(YW_SNIFFER_FRAMEID* pFrame, int nGroupID, int nFrameID,  char* szBuf, int nBufLen)
{
	if(NULL == pFrame || NULL == szBuf)
	{
		return NET_ILLEGAL_PARAM;
	}

	memset(szBuf, 0, nBufLen);

	
	strcat(szBuf, "<Envelope>\r\n");
	strcat(szBuf, "ParameterName:Dahua.Device.Network.Sniffer.Cfg.Group.Frame\r\n");
	sprintf(szBuf+strlen(szBuf), "GroupID:%d\r\n", nGroupID);
	sprintf(szBuf+strlen(szBuf), "FrameID:%d\r\n", nFrameID);
	sprintf(szBuf+strlen(szBuf), "Offset:%d\r\n", pFrame->Offset);
	sprintf(szBuf+strlen(szBuf), "Length:%d\r\n", pFrame->Length);
	pFrame->Key[15] = '\0';
	sprintf(szBuf+strlen(szBuf), "Key:%s\r\n", pFrame->Key);
	strcat(szBuf, "</Envelope>\r\n");

	return strlen(szBuf);
}

int CDevConfigEx::SetDevNewConfig_SnifferContent(YW_SNIFFER_CONTENT* pContent,int nGroupID, int nFrameID, int nContentID, char* szBuf, int nBufLen)
{
	if(NULL == pContent || szBuf == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	memset(szBuf, 0, nBufLen);
	
	strcat(szBuf, "<Envelope>\r\n");
	strcat(szBuf, "ParameterName:Dahua.Device.Network.Sniffer.Cfg.Group.Frame.Content\r\n");
	sprintf(szBuf+strlen(szBuf), "GroupID:%d\r\n", nGroupID);
	sprintf(szBuf+strlen(szBuf), "FrameID:%d\r\n", nFrameID);
	sprintf(szBuf+strlen(szBuf), "ContentID:%d\r\n", nContentID);
	sprintf(szBuf+strlen(szBuf), "Offset:%d\r\n", pContent->Offset);
	sprintf(szBuf+strlen(szBuf), "Offset2:%d\r\n", pContent->Offset2);
	sprintf(szBuf+strlen(szBuf), "Length:%d\r\n", pContent->Length);
	sprintf(szBuf+strlen(szBuf), "Length2:%d\r\n", pContent->Length2);
	pContent->KeyTitle[23] = '\0';
	sprintf(szBuf+strlen(szBuf), "KeyTitle:%s\r\n", pContent->KeyTitle);
	strcat(szBuf, "</Envelope>\r\n");

	return strlen(szBuf);
}

int CDevConfigEx::GetDevNewConfig_DownloadRate(LONG lLoginID, int& nDownLoadRate, int waittime)
{
	int nRet = -1;
	int nRetLen = 0;
	char buffer[256];
	memset(buffer, 0, 256);
	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_DOWNLOAD_RATE, (void*)NULL, buffer, 256, &nRetLen, waittime);
	if(nRet < 0  || nRetLen <= 0)
	{
		return NET_ERROR_DOWNLOADRATE_GETCFG;
	}

	char szValue[64] = {0};
	char *p = GetProtocolValue(buffer, "DownloadRate:", "\r\n", szValue, 64);
	if(NULL == p)
	{
		return NET_ERROR_DOWNLOADRATE_GETCFG;
	}
	nDownLoadRate = atoi(szValue);

	return 1;
}

int CDevConfigEx::SetDevNewConfig_DownloadRate(LONG lLoginID, int* pDownLoadRate, int waittime)
{
	if(NULL == pDownLoadRate)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;
	int nRetLen = 0;
	char buffer[256];
	memset(buffer, 0, 256);
	nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_SET_DOWNLOAD_RATE, (void*)pDownLoadRate, waittime);
	if(nRet < 0)
	{
		return NET_ERROR_DOWNLOADRATE_GETCFG;
	}
	else
	{
		return NET_NOERROR;
	}
	return 1;
}

int CDevConfigEx::GetDevNewConfig_PanoramaSwitch(LONG lLoginID, ALARM_PANORAMA_SWITCH_CFG *lpAlarmCfg, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (0 == device || NULL == lpAlarmCfg)
	{
		return -1;
	}
	
	int nRetLen = 0;
	int nRet = -1;
	int nSheetNum = 16;

	char *pRecvBuf = NULL;
	int nRecvBufLen = 0;
	if (nRecvBufLen < 16 * sizeof(CONFIG_PANORAMA_SWITCH))
	{
		nRecvBufLen = 16 * sizeof(CONFIG_PANORAMA_SWITCH);
	}
	if (nRecvBufLen < 16 * sizeof(CONFIG_WORKSHEET)) 
	{
		nRecvBufLen = 16 * sizeof(CONFIG_WORKSHEET);
	}

	pRecvBuf = new char [nRecvBufLen];
	if (NULL == pRecvBuf)
	{
		nRet = -1;
		goto END;
	}

	memset(pRecvBuf, 0, nRecvBufLen);
	nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_PANORAMA, 0, pRecvBuf, nRecvBufLen, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen == 16*sizeof(CONFIG_PANORAMA_SWITCH))
	{
		nSheetNum = device->channelcount(device);
		lpAlarmCfg->nAlarmChnNum = device->channelcount(device);

		CONFIG_PANORAMA_SWITCH *pPanoramaSwitchCfg = (CONFIG_PANORAMA_SWITCH *)pRecvBuf;
		for (int i = 0; i < lpAlarmCfg->nAlarmChnNum; i++, pPanoramaSwitchCfg++)
		{
			m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_PANORAMA, &lpAlarmCfg->stuPanoramaSwitch[i].struHandle.dwActionMask);
			lpAlarmCfg->stuPanoramaSwitch[i].bEnable = pPanoramaSwitchCfg->bEnable;

			int index = min(32, MAX_VIDEO_IN_NUM);
			for (int j = 0; j < index; j++)
			{
				lpAlarmCfg->stuPanoramaSwitch[i].struHandle.byRelAlarmOut[j] = (pPanoramaSwitchCfg->hEvent.dwAlarmOut>>j)&1;
				lpAlarmCfg->stuPanoramaSwitch[i].struHandle.byRelWIAlarmOut[j] = (pPanoramaSwitchCfg->hEvent.wiAlarmOut>>j)&1;
				lpAlarmCfg->stuPanoramaSwitch[i].struHandle.byRecordChannel[j] = (pPanoramaSwitchCfg->hEvent.dwRecord>>j)&1;
				lpAlarmCfg->stuPanoramaSwitch[i].struHandle.bySnap[j] = (pPanoramaSwitchCfg->hEvent.dwSnapShot>>j)&1;
				lpAlarmCfg->stuPanoramaSwitch[i].struHandle.byTour[j] = (pPanoramaSwitchCfg->hEvent.dwTour>>j)&1;
				
				lpAlarmCfg->stuPanoramaSwitch[i].struHandle.struPtzLink[j].iValue = pPanoramaSwitchCfg->hEvent.PtzLink[j].iValue;
				lpAlarmCfg->stuPanoramaSwitch[i].struHandle.struPtzLink[j].iType = pPanoramaSwitchCfg->hEvent.PtzLink[j].iType;
			}

			lpAlarmCfg->stuPanoramaSwitch[i].struHandle.dwDuration = pPanoramaSwitchCfg->hEvent.iAOLatch;
			lpAlarmCfg->stuPanoramaSwitch[i].struHandle.dwRecLatch = pPanoramaSwitchCfg->hEvent.iRecordLatch;
			lpAlarmCfg->stuPanoramaSwitch[i].struHandle.dwEventLatch = pPanoramaSwitchCfg->hEvent.iEventLatch;
			lpAlarmCfg->stuPanoramaSwitch[i].struHandle.bMessageToNet = (BYTE)pPanoramaSwitchCfg->hEvent.bMessagetoNet;
			lpAlarmCfg->stuPanoramaSwitch[i].struHandle.bMMSEn = (BYTE)pPanoramaSwitchCfg->hEvent.bMMSEn;
			lpAlarmCfg->stuPanoramaSwitch[i].struHandle.bySnapshotTimes = pPanoramaSwitchCfg->hEvent.SnapshotTimes;
			lpAlarmCfg->stuPanoramaSwitch[i].struHandle.bLog = (BYTE)pPanoramaSwitchCfg->hEvent.bLog;
			lpAlarmCfg->stuPanoramaSwitch[i].struHandle.bMatrixEn = (BYTE)pPanoramaSwitchCfg->hEvent.bMatrixEn;
			lpAlarmCfg->stuPanoramaSwitch[i].struHandle.dwMatrix = pPanoramaSwitchCfg->hEvent.dwMatrix;

			m_pManager->GetDevConfig().GetAlmActionFlag(pPanoramaSwitchCfg->hEvent, &lpAlarmCfg->stuPanoramaSwitch[i].struHandle.dwActionFlag);
		}
	}
	else
	{
		nRet = -1;
		goto END;
	}
	
	memset(pRecvBuf, 0, nRecvBufLen);
	nRet = m_pManager->GetDevConfig().GetDevConfig_WorkSheet(lLoginID, WSHEET_PANORAMA_SWITCH, (CONFIG_WORKSHEET*)pRecvBuf, waittime, nSheetNum);
	if (nRet >= 0)
	{
		CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)pRecvBuf;
		for (int i = 0; i < nSheetNum; i++,pWSheet++)
		{
			memcpy(lpAlarmCfg->stuPanoramaSwitch[i].stSect, pWSheet->tsSchedule, sizeof(pWSheet->tsSchedule));
		}
	}
	else
	{
		nRet = -1;
		goto END;
	}
	
	nRet = 0;

END:
	if (pRecvBuf != NULL)
	{
		delete[] pRecvBuf;
	}
	
	return nRet;
}

int CDevConfigEx::GetDevNewConfig_LostFocus(LONG lLoginID, ALARM_LOST_FOCUS_CFG *lpAlarmCfg, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (0 == device || NULL == lpAlarmCfg)
	{
		return -1;
	}
	
	int nRetLen = 0;
	int nRet = -1;
	int nSheetNum = 16;

	char *pRecvBuf = NULL;
	int nRecvBufLen = 0;
	if (nRecvBufLen < 16 * sizeof(CONFIG_LOST_FOCUS))
	{
		nRecvBufLen = 16 * sizeof(CONFIG_LOST_FOCUS);
	}
	if (nRecvBufLen < 16 * sizeof(CONFIG_WORKSHEET)) 
	{
		nRecvBufLen = 16 * sizeof(CONFIG_WORKSHEET);
	}

	pRecvBuf = new char [nRecvBufLen];
	if (NULL == pRecvBuf)
	{
		nRet = -1;
		goto END;
	}

	memset(pRecvBuf, 0, nRecvBufLen);
	nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_LOSTFOCUS, 0, pRecvBuf, nRecvBufLen, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen == 16*sizeof(CONFIG_LOST_FOCUS))
	{
		nSheetNum = device->channelcount(device);
		lpAlarmCfg->nAlarmChnNum = device->channelcount(device);

		CONFIG_LOST_FOCUS *pLostFocusCfg = (CONFIG_LOST_FOCUS *)pRecvBuf;
		for (int i = 0; i < lpAlarmCfg->nAlarmChnNum; i++, pLostFocusCfg++)
		{
			m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_LOSTFOCUS, &lpAlarmCfg->stuLostFocus[i].struHandle.dwActionMask);
			lpAlarmCfg->stuLostFocus[i].bEnable = pLostFocusCfg->bEnable;

			int index = min(32, MAX_VIDEO_IN_NUM);
			for (int j = 0; j < index; j++)
			{
				lpAlarmCfg->stuLostFocus[i].struHandle.byRelAlarmOut[j] = (pLostFocusCfg->hEvent.dwAlarmOut>>j)&1;
				lpAlarmCfg->stuLostFocus[i].struHandle.byRelWIAlarmOut[j] = (pLostFocusCfg->hEvent.wiAlarmOut>>j)&1;
				lpAlarmCfg->stuLostFocus[i].struHandle.byRecordChannel[j] = (pLostFocusCfg->hEvent.dwRecord>>j)&1;
				lpAlarmCfg->stuLostFocus[i].struHandle.bySnap[j] = (pLostFocusCfg->hEvent.dwSnapShot>>j)&1;
				lpAlarmCfg->stuLostFocus[i].struHandle.byTour[j] = (pLostFocusCfg->hEvent.dwTour>>j)&1;
				
				lpAlarmCfg->stuLostFocus[i].struHandle.struPtzLink[j].iValue = pLostFocusCfg->hEvent.PtzLink[j].iValue;
				lpAlarmCfg->stuLostFocus[i].struHandle.struPtzLink[j].iType = pLostFocusCfg->hEvent.PtzLink[j].iType;
			}

			lpAlarmCfg->stuLostFocus[i].struHandle.dwDuration = pLostFocusCfg->hEvent.iAOLatch;
			lpAlarmCfg->stuLostFocus[i].struHandle.dwRecLatch = pLostFocusCfg->hEvent.iRecordLatch;
			lpAlarmCfg->stuLostFocus[i].struHandle.dwEventLatch = pLostFocusCfg->hEvent.iEventLatch;
			lpAlarmCfg->stuLostFocus[i].struHandle.bMessageToNet = (BYTE)pLostFocusCfg->hEvent.bMessagetoNet;
			lpAlarmCfg->stuLostFocus[i].struHandle.bMMSEn = (BYTE)pLostFocusCfg->hEvent.bMMSEn;
			lpAlarmCfg->stuLostFocus[i].struHandle.bySnapshotTimes = pLostFocusCfg->hEvent.SnapshotTimes;
			lpAlarmCfg->stuLostFocus[i].struHandle.bLog = (BYTE)pLostFocusCfg->hEvent.bLog;
			lpAlarmCfg->stuLostFocus[i].struHandle.bMatrixEn = (BYTE)pLostFocusCfg->hEvent.bMatrixEn;
			lpAlarmCfg->stuLostFocus[i].struHandle.dwMatrix = pLostFocusCfg->hEvent.dwMatrix;
			m_pManager->GetDevConfig().GetAlmActionFlag(pLostFocusCfg->hEvent, &lpAlarmCfg->stuLostFocus[i].struHandle.dwActionFlag);
		}
	}
	else
	{
		nRet = -1;
		goto END;
	}
	
	memset(pRecvBuf, 0, nRecvBufLen);
	nRet = m_pManager->GetDevConfig().GetDevConfig_WorkSheet(lLoginID, WSHEET_LOST_FOCUS, (CONFIG_WORKSHEET*)pRecvBuf, waittime, nSheetNum);
	if (nRet >= 0)
	{
		CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)pRecvBuf;
		for (int i = 0; i < nSheetNum; i++,pWSheet++)
		{
			memcpy(lpAlarmCfg->stuLostFocus[i].stSect, pWSheet->tsSchedule, sizeof(pWSheet->tsSchedule));
		}
	}
	else
	{
		nRet = -1;
		goto END;
	}
	
	nRet = 0;

END:
	if (pRecvBuf != NULL)
	{
		delete[] pRecvBuf;
	}
	
	return nRet;
}

int CDevConfigEx::SetDevNewConfig_PanoramaSwitch(LONG lLoginID, ALARM_PANORAMA_SWITCH_CFG *lpAlarmCfg, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == lpAlarmCfg)
	{
		return -1;
	}

	int nRetLen = 0;
	int nRet = -1;

	char *pRecvBuf = NULL;
	int nRecvBufLen = 0;
	if (nRecvBufLen < 16 * sizeof(CONFIG_PANORAMA_SWITCH))
	{
		nRecvBufLen = 16 * sizeof(CONFIG_PANORAMA_SWITCH);
	}
	if (nRecvBufLen < 16 * sizeof(CONFIG_WORKSHEET)) 
	{
		nRecvBufLen = 16 * sizeof(CONFIG_WORKSHEET);
	}
	
	pRecvBuf = new char [nRecvBufLen];
	if (NULL == pRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(pRecvBuf, 0, nRecvBufLen);
	
	{
		CONFIG_PANORAMA_SWITCH *pPanoramaSwitchCfg = (CONFIG_PANORAMA_SWITCH *)pRecvBuf;
		for (int i = 0; i < lpAlarmCfg->nAlarmChnNum; i++,pPanoramaSwitchCfg++)
		{
			m_pManager->GetDevConfig().SetAlmActionFlag(&pPanoramaSwitchCfg->hEvent, lpAlarmCfg->stuPanoramaSwitch[i].struHandle.dwActionFlag);
			pPanoramaSwitchCfg->bEnable = lpAlarmCfg->stuPanoramaSwitch[i].bEnable;
			
			pPanoramaSwitchCfg->hEvent.dwAlarmOut = 0;
			pPanoramaSwitchCfg->hEvent.wiAlarmOut = 0;
			pPanoramaSwitchCfg->hEvent.dwRecord = 0;
			pPanoramaSwitchCfg->hEvent.dwTour = 0;
			pPanoramaSwitchCfg->hEvent.dwSnapShot = 0;
			int index = min(32, MAX_VIDEO_IN_NUM);
			for (int j = 0; j < index; j++)
			{
				pPanoramaSwitchCfg->hEvent.dwAlarmOut |= (lpAlarmCfg->stuPanoramaSwitch[i].struHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
				pPanoramaSwitchCfg->hEvent.wiAlarmOut |= (lpAlarmCfg->stuPanoramaSwitch[i].struHandle.byRelWIAlarmOut[j]) ? (0x01<<j) : 0;
				
				pPanoramaSwitchCfg->hEvent.dwRecord |= (lpAlarmCfg->stuPanoramaSwitch[i].struHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
				pPanoramaSwitchCfg->hEvent.dwSnapShot |= (lpAlarmCfg->stuPanoramaSwitch[i].struHandle.bySnap[j]) ? (0x01<<j) : 0;
				pPanoramaSwitchCfg->hEvent.dwTour |= (lpAlarmCfg->stuPanoramaSwitch[i].struHandle.byTour[j]) ? (0x01<<j) : 0;
				pPanoramaSwitchCfg->hEvent.PtzLink[j].iValue = lpAlarmCfg->stuPanoramaSwitch[i].struHandle.struPtzLink[j].iValue;
				pPanoramaSwitchCfg->hEvent.PtzLink[j].iType = lpAlarmCfg->stuPanoramaSwitch[i].struHandle.struPtzLink[j].iType;
			}
			
			pPanoramaSwitchCfg->hEvent.iAOLatch = lpAlarmCfg->stuPanoramaSwitch[i].struHandle.dwDuration;
			pPanoramaSwitchCfg->hEvent.iRecordLatch = lpAlarmCfg->stuPanoramaSwitch[i].struHandle.dwRecLatch;
			pPanoramaSwitchCfg->hEvent.iEventLatch = lpAlarmCfg->stuPanoramaSwitch[i].struHandle.dwEventLatch;
			pPanoramaSwitchCfg->hEvent.bMessagetoNet = (BOOL)lpAlarmCfg->stuPanoramaSwitch[i].struHandle.bMessageToNet;
			pPanoramaSwitchCfg->hEvent.bMMSEn = lpAlarmCfg->stuPanoramaSwitch[i].struHandle.bMMSEn;
			pPanoramaSwitchCfg->hEvent.SnapshotTimes = lpAlarmCfg->stuPanoramaSwitch[i].struHandle.bySnapshotTimes;
			pPanoramaSwitchCfg->hEvent.bLog = lpAlarmCfg->stuPanoramaSwitch[i].struHandle.bLog;
			pPanoramaSwitchCfg->hEvent.bMatrixEn = lpAlarmCfg->stuPanoramaSwitch[i].struHandle.bMatrixEn;
			pPanoramaSwitchCfg->hEvent.dwMatrix = lpAlarmCfg->stuPanoramaSwitch[i].struHandle.dwMatrix;
		}
		
		nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_ALARM_PANORAMA, 0, pRecvBuf, 16*sizeof(CONFIG_PANORAMA_SWITCH), waittime);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}

	{
		memset(pRecvBuf, 0, nRecvBufLen);
		CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)pRecvBuf;
		for (int i = 0; i < device->channelcount(device); i++,pWSheet++)
		{
			pWSheet->iName = i;
			memcpy(pWSheet->tsSchedule, lpAlarmCfg->stuPanoramaSwitch[i].stSect, sizeof(pWSheet->tsSchedule));
		}
		nRet = m_pManager->GetDevConfig().SetDevConfig_WorkSheet(lLoginID, WSHEET_PANORAMA_SWITCH, (CONFIG_WORKSHEET*)pRecvBuf, waittime, device->channelcount(device));
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}
	
	nRet = 0;

END:
	delete[] pRecvBuf;
	return nRet;
}

int CDevConfigEx::SetDevNewConfig_LostFocus(LONG lLoginID, ALARM_LOST_FOCUS_CFG *lpAlarmCfg, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == lpAlarmCfg)
	{
		return -1;
	}

	int nRetLen = 0;
	int nRet = -1;

	char *pRecvBuf = NULL;
	int nRecvBufLen = 0;
	if (nRecvBufLen < 16 * sizeof(CONFIG_LOST_FOCUS))
	{
		nRecvBufLen = 16 * sizeof(CONFIG_LOST_FOCUS);
	}
	if (nRecvBufLen < 16 * sizeof(CONFIG_WORKSHEET)) 
	{
		nRecvBufLen = 16 * sizeof(CONFIG_WORKSHEET);
	}
	
	pRecvBuf = new char [nRecvBufLen];
	if (NULL == pRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(pRecvBuf, 0, nRecvBufLen);
	
	{
		CONFIG_LOST_FOCUS *pLostFocusCfg = (CONFIG_LOST_FOCUS *)pRecvBuf;
		for (int i = 0; i < lpAlarmCfg->nAlarmChnNum; i++,pLostFocusCfg++)
		{
			m_pManager->GetDevConfig().SetAlmActionFlag(&pLostFocusCfg->hEvent, lpAlarmCfg->stuLostFocus[i].struHandle.dwActionFlag);
			pLostFocusCfg->bEnable = lpAlarmCfg->stuLostFocus[i].bEnable;
			
			pLostFocusCfg->hEvent.dwAlarmOut = 0;
			pLostFocusCfg->hEvent.wiAlarmOut = 0;
			pLostFocusCfg->hEvent.dwRecord = 0;
			pLostFocusCfg->hEvent.dwTour = 0;
			pLostFocusCfg->hEvent.dwSnapShot = 0;
			int index = min(32, MAX_VIDEO_IN_NUM);
			for (int j = 0; j < index; j++)
			{
				pLostFocusCfg->hEvent.dwAlarmOut |= (lpAlarmCfg->stuLostFocus[i].struHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
				pLostFocusCfg->hEvent.wiAlarmOut |= (lpAlarmCfg->stuLostFocus[i].struHandle.byRelWIAlarmOut[j]) ? (0x01<<j) : 0;
				
				pLostFocusCfg->hEvent.dwRecord |= (lpAlarmCfg->stuLostFocus[i].struHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
				pLostFocusCfg->hEvent.dwSnapShot |= (lpAlarmCfg->stuLostFocus[i].struHandle.bySnap[j]) ? (0x01<<j) : 0;
				pLostFocusCfg->hEvent.dwTour |= (lpAlarmCfg->stuLostFocus[i].struHandle.byTour[j]) ? (0x01<<j) : 0;
				pLostFocusCfg->hEvent.PtzLink[j].iValue = lpAlarmCfg->stuLostFocus[i].struHandle.struPtzLink[j].iValue;
				pLostFocusCfg->hEvent.PtzLink[j].iType = lpAlarmCfg->stuLostFocus[i].struHandle.struPtzLink[j].iType;
			}
			
			pLostFocusCfg->hEvent.iAOLatch = lpAlarmCfg->stuLostFocus[i].struHandle.dwDuration;
			pLostFocusCfg->hEvent.iRecordLatch = lpAlarmCfg->stuLostFocus[i].struHandle.dwRecLatch;
			pLostFocusCfg->hEvent.iEventLatch = lpAlarmCfg->stuLostFocus[i].struHandle.dwEventLatch;
			pLostFocusCfg->hEvent.bMessagetoNet = (BOOL)lpAlarmCfg->stuLostFocus[i].struHandle.bMessageToNet;
			pLostFocusCfg->hEvent.bMMSEn = lpAlarmCfg->stuLostFocus[i].struHandle.bMMSEn;
			pLostFocusCfg->hEvent.SnapshotTimes = lpAlarmCfg->stuLostFocus[i].struHandle.bySnapshotTimes;
			pLostFocusCfg->hEvent.bLog = lpAlarmCfg->stuLostFocus[i].struHandle.bLog;
			pLostFocusCfg->hEvent.bMatrixEn = lpAlarmCfg->stuLostFocus[i].struHandle.bMatrixEn;
			pLostFocusCfg->hEvent.dwMatrix = lpAlarmCfg->stuLostFocus[i].struHandle.dwMatrix;
		}
		
		nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_ALARM_LOSTFOCUS, 0, pRecvBuf, 16*sizeof(CONFIG_LOST_FOCUS), waittime);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}

	{
		memset(pRecvBuf, 0, nRecvBufLen);
		CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)pRecvBuf;
		for (int i = 0; i < device->channelcount(device); i++,pWSheet++)
		{
			pWSheet->iName = i;
			memcpy(pWSheet->tsSchedule, lpAlarmCfg->stuLostFocus[i].stSect, sizeof(pWSheet->tsSchedule));
		}
		nRet = m_pManager->GetDevConfig().SetDevConfig_WorkSheet(lLoginID, WSHEET_LOST_FOCUS, (CONFIG_WORKSHEET*)pRecvBuf, waittime, device->channelcount(device));
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}
	
	nRet = 0;

END:
	delete[] pRecvBuf;
	return nRet;
}

/*
 * 申请建立会话，由服务器端分配ConnectID+IP(server)+Port(server)
 */
int CDevConfigEx::SetupSession(LONG lLoginID, int nChannelID, afk_connect_param_t* pConnParam)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pConnParam == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nProtocolVersion;
	NET_PARAM stuNetParam = {0};

	m_pManager->GetNetParameter(&stuNetParam);
	device->get_info(device, dit_protocol_version, &nProtocolVersion);
	pConnParam->nConnTime = stuNetParam.nConnectTime;
	pConnParam->nTryTime = stuNetParam.nConnectTryNum;
	pConnParam->nConnBufSize = stuNetParam.nConnectBufSize;
	pConnParam->nParam = stuNetParam.nWaittime;

	if (nProtocolVersion >= 6) // 协议版本6以上，IP、Port统一获取
	{
		int nRetLen = 0;
		char buffer[512] = {0};
		
		int nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_CONNECT, (void*)pConnParam->nConnType, buffer, 512, &nRetLen, stuNetParam.nGetConnInfoTime);
		if (nRet >= 0 && nRetLen > 0)
		{
			char szValue[64] = {0};
			char *p = GetProtocolValue(buffer, "IP:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				return NET_RETURN_DATA_ERROR;
			}
			strcpy(pConnParam->szSevIp, szValue);
			if (_stricmp(pConnParam->szSevIp, "0.0.0.0") == 0)
			{
				strcpy(pConnParam->szSevIp, device->device_ip(device));
			}

			p = GetProtocolValue(buffer, "Port:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				return NET_RETURN_DATA_ERROR;
			}
			pConnParam->nSevPort = atoi(szValue);
			if (pConnParam->nConnType == channel_connect_multicast)
			{
				pConnParam->nSevPort += nChannelID;
			}

			p = GetProtocolValue(buffer, "ConnectionID:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				return NET_RETURN_DATA_ERROR;
			}
			pConnParam->nConnectID = atoi(szValue);
		}
		else
		{
			return NET_ERROR_GETCFG_SESSION;
		}
	}
	else // 协议版本6以下，IP、Port分别获取
	{
		afk_login_device_type stuLoginType = {0};
		device->get_info(device, dit_login_device_flag, &stuLoginType);

		if (pConnParam->nConnType == channel_connect_udp)
		{
			strcpy(pConnParam->szSevIp, stuLoginType.szRemoteIp);
			pConnParam->nSevPort = stuLoginType.nRemotePort;
			pConnParam->nConnectID = nChannelID;
		}
		else if (pConnParam->nConnType == channel_connect_multicast)
		{
			strcpy(pConnParam->szSevIp, stuLoginType.szRemoteIp);
			pConnParam->nSevPort = stuLoginType.nRemotePort + nChannelID;
			pConnParam->nConnectID = nChannelID;
		}
		else
		{
			strcpy(pConnParam->szSevIp, device->device_ip(device));
			pConnParam->nSevPort = device->device_port(device);
			pConnParam->nConnectID = nChannelID;
		}
	}
	
	return NET_NOERROR;
}

/*
 * 销毁会话，服务器端删除连接
 */
int CDevConfigEx::DestroySession(LONG lLoginID, int nConnectID)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = 0;
	int nProtocolVersion;
	NET_PARAM stuNetParam = {0};

	m_pManager->GetNetParameter(&stuNetParam);
	device->get_info(device, dit_protocol_version, &nProtocolVersion);

	if (nProtocolVersion >= 6)
	{
		nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_DISCONNECT, (void*)nConnectID, stuNetParam.nGetConnInfoTime);
	}

	return nRet;
}

int CDevConfigEx::GetMulticastAddr(afk_device_s* device, afk_login_device_type *pstMulticastInfo)
{
	if (device == NULL || pstMulticastInfo == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	int retlen = 0;
	CONFIG_NET stuNetCfg;
	NET_PARAM stuNetParam = {0};
	m_pManager->GetNetParameter(&stuNetParam);
	int nRet = m_pManager->GetDevConfig().QueryConfig((LONG)device, CONFIG_TYPE_NET, 0, (char*)&stuNetCfg, sizeof(CONFIG_NET), &retlen, stuNetParam.nGetDevInfoTime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_NET))
	{
		in_addr addr = {0};
		addr.s_addr = stuNetCfg.McastIP;
		strcpy(pstMulticastInfo->szRemoteIp, inet_ntoa(addr));
		pstMulticastInfo->nRemotePort = stuNetCfg.McastPort;
	}
	else
	{
		nRet = NET_ERROR_GETCFG_NETCFG;
	}

	return nRet;
}

int CDevConfigEx::GetUdpPort(afk_device_s* device, afk_login_device_type *pstUdpInfo)
{
	if (device == NULL || pstUdpInfo == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	int retlen = 0;
	CONFIG_NET stuNetCfg;
	NET_PARAM stuNetParam = {0};
	m_pManager->GetNetParameter(&stuNetParam);
	int nRet = m_pManager->GetDevConfig().QueryConfig((LONG)device, CONFIG_TYPE_NET, 0, (char*)&stuNetCfg, sizeof(CONFIG_NET), &retlen, stuNetParam.nGetDevInfoTime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_NET))
	{
		in_addr addr = {0};
		addr.s_addr = stuNetCfg.HostIP;
		strcpy(pstUdpInfo->szRemoteIp, inet_ntoa(addr));
		pstUdpInfo->nRemotePort = stuNetCfg.UDPPort;
	}
	else
	{
		nRet = NET_ERROR_GETCFG_NETCFG;
	}

	return nRet;
}


//VideoOut（获取）
int	CDevConfigEx::GetDevNewConfig_VideoOut(LONG lLoginID, DEV_VIDEOOUT_INFO &stVideoOutCfg, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	memset(&stVideoOutCfg, 0, sizeof(DEV_VIDEOOUT_INFO));


	int nRetLen = 0;
	int nRet = -1;
	char buffer[1024] = {0};

	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_VIDEOFORMAT_INFO, NULL, buffer, 1024, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = buffer;
		while (p)
		{
			p = GetProtocolValue(p, "SupportItem:", "\r\n", szValue, 64);
			if (0 == _stricmp(szValue, "PAL"))
			{
				stVideoOutCfg.dwVideoStandardMask |= 0x01;
			}
			else if(0 == _stricmp(szValue, "NTSC"))
			{
				stVideoOutCfg.dwVideoStandardMask |= 0x02;
			}
		}
		p = GetProtocolValue(buffer, "DefaultItem:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		stVideoOutCfg.nVideoStandard = atoi(szValue);
		
		p = buffer;
		while (p)
		{
			p = GetProtocolValue(p, "SupportResolution:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				break;
			}
			int nIndex = atoi(szValue);
			stVideoOutCfg.dwImageSizeMask |= (1<<nIndex);
		}
		p = GetProtocolValue(buffer, "DefaultResolution:", "\r\n", szValue, 64);
		if (p)
		{
			stVideoOutCfg.nImageSize = atoi(szValue);		
		}			
	}
	else
	{
		return NET_ERROR_GETCFG_VIDEOOUT;
	}
			
	return 0;
}

//VideoOut（设置）
int	CDevConfigEx::SetDevNewConfig_VideoOut(LONG lLoginID, DEV_VIDEOOUT_INFO *lpVideoOutCfg, int waittime)
{
	if (NULL == lpVideoOutCfg)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;  //return value
	char buffer[1024] = {0};
	int nRetLen = 0;
	
	//set VideoOut configure
	REQUEST_SET_VIDEOOUT tmpVideoOut = {0};
	
	tmpVideoOut.nVideoStander = lpVideoOutCfg->nVideoStandard;//视频制式
	tmpVideoOut.nImageSize = lpVideoOutCfg->nImageSize;//分辨率
	
	nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_SET_VIDEOOUT, (void*)&tmpVideoOut, waittime);
			
	if (nRet < 0 )
	{
		nRet = NET_ERROR_SETCFG_VIDEO;
	}
	else
	{
		nRet = NET_NOERROR;
	}

	return nRet;
}

int CDevConfigEx::GetDevNewConfig_Point(LONG lLoginID, DEVICE_POINT_CFG& stuPointCfg, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	memset(&stuPointCfg, 0, sizeof(DEVICE_POINT_CFG));
	
	
	int nRetLen = 0;
	int nRet = -1;
	char buffer[2048] = {0};
	int nChannelNum = device->channelcount(device);
	
	char szRequest[512] = {0};
	sprintf(szRequest+strlen(szRequest), "Method:GetParameterNames\r\n");
	sprintf(szRequest+strlen(szRequest), "ParameterName:Dahua.Device.Oem.JunYi.Intelligent.General\r\n");
	sprintf(szRequest+strlen(szRequest), "\r\n");

	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_POINT_CFG, szRequest, buffer, 2048, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		//解析
		char szValue[64] = {0};
		char *p = buffer;
		p = GetProtocolValue(p, "ChnPresetNum:", "\r\n", szValue, 64);
		if(NULL == p)
		{
			return -1;
		}
		stuPointCfg.nSupportNum = atoi(szValue);

		for( int nChannel = 0; nChannel < nChannelNum; nChannel++)
		{
			//ip和port
			p = GetProtocolValue(p, "ChnIntellInfo:", "\r\n", szValue, 64);
			if(NULL == p)
			{
				return -1;
			}
			CStrParse ChnParser;
			ChnParser.setSpliter("&&");
			if(ChnParser.Parse(szValue) == false)
			{
				return -1;
			}
				
			int nChnIndex = ChnParser.getValue(0) - 1;
			if(nChnIndex >= nChannelNum)
			{
				break;
			}

			if(ChnParser.getWord(1).size() >= MAX_IP_ADDRESS_LEN)
			{
				return -1;
			}
			strcpy(stuPointCfg.stuPointCfg[nChnIndex].szIP, ChnParser.getWord(1).c_str());
			stuPointCfg.stuPointCfg[nChnIndex].nPort = ChnParser.getValue(2);

			for(int nPoint = 0; nPoint < stuPointCfg.nSupportNum; nPoint++)
			{
				CStrParse PointParser;
				PointParser.setSpliter("&&");
				p = GetProtocolValue(p, "ChnPresetInfo:", "\r\n", szValue, 64);
				if(NULL == p)
				{
					return -1;
				}
					
				if(PointParser.Parse(szValue) == false)
				{
					
					return -1;
				}
				
				stuPointCfg.stuPointCfg[nChnIndex].stuPointEnable[nPoint].bEnable = PointParser.getValue(0) > 0 ? 1 : 0;
				stuPointCfg.stuPointCfg[nChnIndex].stuPointEnable[nPoint].bPoint = PointParser.getValue(1);
			}
		}
	}
	else
	{
		return NET_ERROR_GETCFG_POINT;
	}

	return 1;
}

int CDevConfigEx::SetDevNewConfig_Point(LONG lLoginID, DEVICE_POINT_CFG* stuPointCfg, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	int nRet = -1;
	
	char szRequest[2048] = {0};
	sprintf(szRequest+strlen(szRequest), "Method:SetParameterNames\r\n");
	sprintf(szRequest+strlen(szRequest), "ParameterName:Dahua.Device.Oem.JunYi.Intelligent.General\r\n");
	int nChannelNum = device->channelcount(device);
	for(int i = 0; i < nChannelNum; i++)
	{
		stuPointCfg->stuPointCfg[i].szIP[15] = '\0';
		sprintf(szRequest+strlen(szRequest), 
				"ChnIntellInfo:%d&&%s&&%d\r\n",
				i+1,
				stuPointCfg->stuPointCfg[i].szIP,
				stuPointCfg->stuPointCfg[i].nPort);
		for(int j = 0; j < stuPointCfg->nSupportNum; j++)
		{
			sprintf(szRequest+strlen(szRequest), 
					"ChnPresetInfo:%d&&%d\r\n", 
					stuPointCfg->stuPointCfg[i].stuPointEnable[j].bEnable,
					stuPointCfg->stuPointCfg[i].stuPointEnable[j].bPoint);
		}
	}
	sprintf(szRequest+strlen(szRequest), "\r\n");
	nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_SET_POINT_CFG, (void*)&szRequest, waittime);
	if (nRet < 0 )
	{
		nRet = NET_ERROR_SETCFG_POINT;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	
	return nRet;
}

int CDevConfigEx::GetDevNewConfig_OSDEnable(LONG lLoginID, DEV_OSD_ENABLE_CONFIG* stuOSDEnableCfg, int waittime)
{
	afk_device_s * device = (afk_device_s *)lLoginID;
	if (NULL == stuOSDEnableCfg)
	{
		return NET_ILLEGAL_PARAM;
	}	
	if(NULL == device)
	{
		return NET_INVALID_HANDLE;
	}
	
	int nRet = -1;
	int nRetLen = 0;
	char szData[512] = {0};
	sprintf(szData+strlen(szData), "%s", "Method:GetParameterValues\r\n");
	sprintf(szData+strlen(szData), "%s", "ParameterName:Dahua.Device.Oem.WeiKeMu.Video.OSD\r\n");
	sprintf(szData+strlen(szData), "%s", "\r\n");

	char szRetBuf[512] = {0};
	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_OSDENABLE_CFG, szData, szRetBuf, 512, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char* p = GetProtocolValue(szRetBuf, "Enable:", "\r\n", szValue, 64);
		if(NULL == p)
		{
			return -1;
		}
		stuOSDEnableCfg->bEnable = (atoi(szValue) == 0) ? 0 : 1;

		p = GetProtocolValue(p, "Port:", "\r\n", szValue, 64);
		if(NULL == p)
		{
			return -1;
		}
		stuOSDEnableCfg->nPort = atoi(szValue);

		nRet = NET_NOERROR;
	}
	else
	{
		nRet = NET_ERROR_GETCFG_OSDENABLE;
	}

	return nRet;
}

int CDevConfigEx::SetDevNewConfig_OSDEnable(LONG lLoginID, DEV_OSD_ENABLE_CONFIG* stuOSDEnableCfg, int waittime)
{
	afk_device_s * device = (afk_device_s *)lLoginID;
	if (NULL == stuOSDEnableCfg)
	{
		return NET_ILLEGAL_PARAM;
	}	
	if(NULL == device)
	{
		return NET_INVALID_HANDLE;
	}

	int nRet = -1;
	char szData[512] = {0};
	sprintf(szData+strlen(szData), "%s", "Method:SetParameterValues\r\n");
	sprintf(szData+strlen(szData), "%s", "ParameterName:Dahua.Device.Oem.WeiKeMu.Video.OSD\r\n");
	sprintf(szData+strlen(szData), "Enable:%d\r\n", (stuOSDEnableCfg->bEnable == 0) ? 0 : 1);
	sprintf(szData+strlen(szData), "Port:%d\r\n", stuOSDEnableCfg->nPort);
	sprintf(szData+strlen(szData), "%s", "\r\n");
	
	nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_SET_OSDENABLE_CFG, (void*)&szData, waittime);
	if (nRet < 0 )
	{
		nRet = NET_ERROR_SETCFG_POINT;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	return nRet;
}//本地报警
int	CDevConfigEx::GetDevNewConfig_AllLocalALMCfg(LONG lLoginID, ALARM_IN_CFG_EX *pstLocalAlm, LONG lChnNum, int waittime)
{
	afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device || NULL == pstLocalAlm)
	{
		return -1;
	}
	
	memset(pstLocalAlm, 0, lChnNum*sizeof(ALARM_IN_CFG_EX));

	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
		
	int nRet = -1;
	int retlen = 0;
	
	int i = 0, x = 0;
	
	int nAlarmInNum = device->alarminputcount(device);

	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	char *aBuf = NULL;
	
	if ((nAlarmInNum != 0) && (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF]) )
	{
		int iProtocolVersion;
		device->get_info(device, dit_protocol_version, &iProtocolVersion);
		if (iProtocolVersion > 2)//新的报警配置
		{
			int nSheetNum = 16;
			
			if (iRecvBufLen < lChnNum * sizeof(CONFIG_ALARM_X))
			{
				iRecvBufLen = lChnNum * sizeof(CONFIG_ALARM_X);
			}

			cRecvBuf = new char [iRecvBufLen];
			if (NULL == cRecvBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(cRecvBuf, 0, iRecvBufLen);
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_LOCALALM, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_ALARM_X) == 0))
			{
				device->get_info(device, dit_alarmin_num, &nSheetNum);
				
				int j = 0;
				int nCount = retlen/sizeof(CONFIG_ALARM_X);
				if (nCount > nAlarmInNum) 
				{
					nCount = nAlarmInNum;
				}
				CONFIG_ALARM_X *pCfgAlmX = (CONFIG_ALARM_X *)cRecvBuf;
				for (i = 0; i < nCount; i++, pCfgAlmX++)
				{
					m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_LOCALALM, &pstLocalAlm[i].struHandle.dwActionMask);
					pstLocalAlm[i].byAlarmEn = pCfgAlmX->bEnable;
					pstLocalAlm[i].byAlarmType = pCfgAlmX->iSensorType;
					int index = min(32, device->channelcount(device));
					for (j = 0; j < index; j++)
					{
						
						pstLocalAlm[i].struHandle.byRecordChannel[j] = BITRHT(pCfgAlmX->hEvent.dwRecord,j)&1;
						pstLocalAlm[i].struHandle.byTour[j] = BITRHT(pCfgAlmX->hEvent.dwTour,j)&1;
						pstLocalAlm[i].struHandle.bySnap[j] = BITRHT(pCfgAlmX->hEvent.dwSnapShot,j)&1;
					}
					for(j = 0; j < 16; j++)
					{
						pstLocalAlm[i].struHandle.struPtzLink[j].iValue = pCfgAlmX->hEvent.PtzLink[j].iValue;
						pstLocalAlm[i].struHandle.struPtzLink[j].iType = pCfgAlmX->hEvent.PtzLink[j].iType;
					}
					index = min(32, device->alarmoutputcount(device));
					for (j = 0; j < index; j++)
					{
						pstLocalAlm[i].struHandle.byRelAlarmOut[j] = BITRHT(pCfgAlmX->hEvent.dwAlarmOut,j)&1;
						pstLocalAlm[i].struHandle.byRelWIAlarmOut[j] =BITRHT(pCfgAlmX->hEvent.wiAlarmOut,j)&1;
					}
					pstLocalAlm[i].struHandle.dwDuration = pCfgAlmX->hEvent.iAOLatch;
					pstLocalAlm[i].struHandle.dwRecLatch = pCfgAlmX->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
					pstLocalAlm[i].struHandle.dwEventLatch = pCfgAlmX->hEvent.iEventLatch;//报警延时
					pstLocalAlm[i].struHandle.bMessageToNet = (BYTE)pCfgAlmX->hEvent.bMessagetoNet;
					pstLocalAlm[i].struHandle.bMMSEn = (BYTE)pCfgAlmX->hEvent.bMMSEn;
					pstLocalAlm[i].struHandle.bySnapshotTimes = pCfgAlmX->hEvent.SnapshotTimes;//短信发送图片的张数
					pstLocalAlm[i].struHandle.bLog = (BOOL)pCfgAlmX->hEvent.bLog;
					pstLocalAlm[i].struHandle.bMatrixEn = (BOOL)pCfgAlmX->hEvent.bMatrixEn;
					pstLocalAlm[i].struHandle.dwMatrix = pCfgAlmX->hEvent.dwMatrix;
					m_pManager->GetDevConfig().GetAlmActionFlag(pCfgAlmX->hEvent, &pstLocalAlm[i].struHandle.dwActionFlag);
				}
			}
			else
			{
				nRet = -1;
				goto END;
			}
	
			nSheetNum = nSheetNum>lChnNum?lChnNum:nSheetNum;
			for (i=0; i < nSheetNum; i++)
			{
				CONFIG_WORKSHEET stWSheet = {0};
				nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (WSHEET_LOCAL_ALARM<<16)|(i+1), (char*)&stWSheet, sizeof(CONFIG_WORKSHEET), &retlen, waittime);
				if (nRet < 0 || retlen != sizeof(CONFIG_WORKSHEET))
				{
					nRet = -1;
					goto END;
				}

				memcpy(pstLocalAlm[i].stSect, &stWSheet.tsSchedule, sizeof(stWSheet.tsSchedule));

			}

			nRet = 0;
		}
		else //旧的报警
		{
			aBuf = new char [lChnNum*sizeof(CONFIG_ALARM)];
			if (NULL == aBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(aBuf, 0, lChnNum*sizeof(CONFIG_ALARM));
			
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM, 0, aBuf, lChnNum*sizeof(CONFIG_ALARM), &retlen, waittime);
			if((nRet >= 0) && (retlen%sizeof(CONFIG_ALARM) == 0) &&(retlen <= lChnNum * sizeof(CONFIG_ALARM)))
			{
				CONFIG_ALARM *stAlarm = 0;
				int nCount = retlen/sizeof(CONFIG_ALARM);
				for (i = 0; i < nCount; i++)
				{
					pstLocalAlm[i].struHandle.dwActionMask = 0;
					pstLocalAlm[i].struHandle.dwActionMask |= ALARM_TIP;
					pstLocalAlm[i].struHandle.dwActionMask |= ALARM_TOUR;
					pstLocalAlm[i].struHandle.dwActionMask |= ALARM_MAIL;
					pstLocalAlm[i].struHandle.dwActionMask |= ALARM_UPLOAD;
					pstLocalAlm[i].struHandle.dwActionMask |= ALARM_OUT;
					pstLocalAlm[i].struHandle.dwActionMask |= ALARM_RECORD;
					pstLocalAlm[i].struHandle.dwActionMask |= ALARM_FTP_UL;
					
					stAlarm = (CONFIG_ALARM *)(aBuf + i*sizeof(CONFIG_ALARM));
					pstLocalAlm[i].byAlarmType = stAlarm->SensorType;
					pstLocalAlm[i].byAlarmEn = 1; //default enabled
					pstLocalAlm[i].struHandle.dwActionFlag |= ALARM_RECORD; //default record enable
					for (x = 0; x < lChnNum; x++)
					{
						pstLocalAlm[i].struHandle.byRelAlarmOut[x] = (stAlarm->AlarmMask & (0x01<<x)) ? 1 : 0;
						pstLocalAlm[i].struHandle.byRecordChannel[x] = (stAlarm->RecordMask & (0x01<<x)) ? 1 : 0;
					}
					if (stAlarm->AlarmTipEn)
					{
						pstLocalAlm[i].struHandle.dwActionFlag |= ALARM_TIP;
					}
					if (stAlarm->dbTourEn)
					{
						pstLocalAlm[i].struHandle.dwActionFlag |= ALARM_TOUR;
					}
					if (stAlarm->Mailer)
					{
						pstLocalAlm[i].struHandle.dwActionFlag |= ALARM_MAIL;
					}
					if (stAlarm->SendAlarmMsg)
					{
						pstLocalAlm[i].struHandle.dwActionFlag |= ALARM_UPLOAD;
					}
					for (x = 0; x < ALARM_TIME_SECT_NUM; x++)
					{
						for (int j = 0; j < DAYS_PER_WEEK; j++)
						{
							//memcpy(&stAS.struLocalAlmIn[i].stSect[j][x], &stAlarm->sAlarmSects[x], sizeof(TSECT));
						}
						//这里的wEvent是一个“提示”和“报警输出”的使能掩码。
						//	stAS.struLocalAlmIn[i].struHandle.stAlarmOutSect[x].wEvent = ((WORD)stAlarm->dwSectMask[x]) & 0x000F;
					}
					//	stAS.struLocalAlmIn[i].struHandle.dwActionFlag |= (stAlarm->dwSectMask[0] & 0x01)?ALARM_TIP:0;
					//	stAS.struLocalAlmIn[i].struHandle.dwActionFlag |= (stAlarm->dwSectMask[0]>>1) & 0x01?ALARM_OUT:0;
					
					pstLocalAlm[i].struHandle.dwRecLatch = stAlarm->TimePreRec;
					pstLocalAlm[i].struHandle.dwDuration = stAlarm->TimeDelay;
				}
				
				nRet = 0;

			}
			else
			{
				nRet = -1;
				goto END;
			}
			
			ALARM_CONTROL tmpAlarmCfg[CONFIG_ALARM_NUM_EX] = {0};
			int iAlarmIn = 0;
			nRet = m_pManager->GetDevConfig().QueryIOControlState(lLoginID, IOTYPE_ALARMINPUT, NULL, 0, &iAlarmIn, waittime);
			if (nRet >= 0)
			{
				if (iAlarmIn <= CONFIG_ALARM_NUM_EX)
				{
					nRet = m_pManager->GetDevConfig().QueryIOControlState(lLoginID, IOTYPE_ALARMINPUT, tmpAlarmCfg, CONFIG_ALARM_NUM_EX*sizeof(ALARM_CONTROL), &iAlarmIn, waittime);
					iAlarmIn = Min(iAlarmIn, CONFIG_ALARM_NUM);
					if (nRet >= 0)
					{
						for (i = 0; i < iAlarmIn; i++)
						{
							pstLocalAlm[i].byAlarmEn = tmpAlarmCfg[i].state;
						}
					}
				}
			}
			else
			{
				nRet = -1;
				goto END;
			}

			nRet = 0;	
		}
	}
		
END:
	if (cRecvBuf) 
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}

	if (aBuf) 
	{
		delete []aBuf;
		aBuf = NULL;
	}
		
	return nRet;
}

int	CDevConfigEx::GetDevNewConfig_LocalALMCfg(LONG lLoginID, ALARM_IN_CFG_EX &stLocalAlm, LONG lChannel, int waittime)
{
	afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device)
	{
		return -1;
	}
	if (lChannel < 0 || lChannel >= device->alarminputcount(device))
	{
		return -1;
	}

	memset(&stLocalAlm, 0, sizeof(ALARM_IN_CFG_EX));
	
	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
	
	int nRet = -1;
	int retlen = 0;
	
	int i = 0, x = 0;
	
	int nAlarmInNum = device->alarminputcount(device);
	int lChnNum = nAlarmInNum > 16?nAlarmInNum:16;
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	char *aBuf = NULL;
	
	if ((nAlarmInNum != 0) && (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF]) )
	{
		int iProtocolVersion;
		device->get_info(device, dit_protocol_version, &iProtocolVersion);
		if (iProtocolVersion > 2)//新的报警配置
		{
			if (iRecvBufLen < lChnNum * sizeof(CONFIG_ALARM_X))
			{
				iRecvBufLen = lChnNum * sizeof(CONFIG_ALARM_X);
			}
			
			cRecvBuf = new char [iRecvBufLen];
			if (NULL == cRecvBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(cRecvBuf, 0, iRecvBufLen);
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_LOCALALM, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_ALARM_X) == 0) && (retlen/sizeof(CONFIG_ALARM_X) > lChannel))
			{	
				int j = 0;
				CONFIG_ALARM_X *pCfgAlmX = (CONFIG_ALARM_X *)(cRecvBuf + lChannel*sizeof(CONFIG_ALARM_X));
				
				m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_LOCALALM, &stLocalAlm.struHandle.dwActionMask);
				stLocalAlm.byAlarmEn = pCfgAlmX->bEnable;
				stLocalAlm.byAlarmType = pCfgAlmX->iSensorType;
				int index = min(32, device->channelcount(device));
				for (j = 0; j < index; j++)
				{
					
					stLocalAlm.struHandle.byRecordChannel[j] = BITRHT(pCfgAlmX->hEvent.dwRecord,j)&1;
					stLocalAlm.struHandle.byTour[j] = BITRHT(pCfgAlmX->hEvent.dwTour,j)&1;
					stLocalAlm.struHandle.bySnap[j] = BITRHT(pCfgAlmX->hEvent.dwSnapShot,j)&1;
				}
				for(j = 0; j < 16; j++)
				{
					stLocalAlm.struHandle.struPtzLink[j].iValue = pCfgAlmX->hEvent.PtzLink[j].iValue;
					stLocalAlm.struHandle.struPtzLink[j].iType = pCfgAlmX->hEvent.PtzLink[j].iType;
				}
				index = min(32, device->alarmoutputcount(device));
				for (j = 0; j < index; j++)
				{
					stLocalAlm.struHandle.byRelAlarmOut[j] = BITRHT(pCfgAlmX->hEvent.dwAlarmOut,j)&1;
					stLocalAlm.struHandle.byRelWIAlarmOut[j] =BITRHT(pCfgAlmX->hEvent.wiAlarmOut,j)&1;
				}
				stLocalAlm.struHandle.dwDuration = pCfgAlmX->hEvent.iAOLatch;
				stLocalAlm.struHandle.dwRecLatch = pCfgAlmX->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
				stLocalAlm.struHandle.dwEventLatch = pCfgAlmX->hEvent.iEventLatch;//报警延时
				stLocalAlm.struHandle.bMessageToNet = (BYTE)pCfgAlmX->hEvent.bMessagetoNet;
				stLocalAlm.struHandle.bMMSEn = (BYTE)pCfgAlmX->hEvent.bMMSEn;
				stLocalAlm.struHandle.bySnapshotTimes = pCfgAlmX->hEvent.SnapshotTimes;//短信发送图片的张数
				stLocalAlm.struHandle.bLog = (BOOL)pCfgAlmX->hEvent.bLog;
				stLocalAlm.struHandle.bMatrixEn = (BOOL)pCfgAlmX->hEvent.bMatrixEn;
				stLocalAlm.struHandle.dwMatrix = pCfgAlmX->hEvent.dwMatrix;
				m_pManager->GetDevConfig().GetAlmActionFlag(pCfgAlmX->hEvent, &stLocalAlm.struHandle.dwActionFlag);
				
			}
			else
			{
				nRet = -1;
				goto END;
			}
				
			CONFIG_WORKSHEET stWSheet = {0};
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (WSHEET_LOCAL_ALARM<<16)|(lChannel+1), (char*)&stWSheet, sizeof(CONFIG_WORKSHEET), &retlen, waittime);
			if (nRet < 0 || retlen != sizeof(CONFIG_WORKSHEET))
			{
				nRet = -1;
				goto END;
			}
			
			memcpy(stLocalAlm.stSect, &stWSheet.tsSchedule, sizeof(stWSheet.tsSchedule));
				
			nRet = 0;
		}
		else //旧的报警
		{
			aBuf = new char [lChnNum*sizeof(CONFIG_ALARM)];
			if (NULL == aBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(aBuf, 0, lChnNum*sizeof(CONFIG_ALARM));
			
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM, 0, aBuf, lChnNum*sizeof(CONFIG_ALARM), &retlen, waittime);
			if((nRet >= 0) && (retlen%sizeof(CONFIG_ALARM) == 0) &&(retlen <= lChnNum * sizeof(CONFIG_ALARM)))
			{
				CONFIG_ALARM *stAlarm = 0;
				for (i = 0; i < lChnNum; i++)
				{
					stLocalAlm.struHandle.dwActionMask = 0;
					stLocalAlm.struHandle.dwActionMask |= ALARM_TIP;
					stLocalAlm.struHandle.dwActionMask |= ALARM_TOUR;
					stLocalAlm.struHandle.dwActionMask |= ALARM_MAIL;
					stLocalAlm.struHandle.dwActionMask |= ALARM_UPLOAD;
					stLocalAlm.struHandle.dwActionMask |= ALARM_OUT;
					stLocalAlm.struHandle.dwActionMask |= ALARM_RECORD;
					stLocalAlm.struHandle.dwActionMask |= ALARM_FTP_UL;
					
					stAlarm = (CONFIG_ALARM *)(aBuf + lChannel*sizeof(CONFIG_ALARM));
					stLocalAlm.byAlarmType = stAlarm->SensorType;
					stLocalAlm.byAlarmEn = 1; //default enabled
					stLocalAlm.struHandle.dwActionFlag |= ALARM_RECORD; //default record enable
					for (x = 0; x < lChnNum; x++)
					{
						stLocalAlm.struHandle.byRelAlarmOut[x] = (stAlarm->AlarmMask & (0x01<<x)) ? 1 : 0;
						stLocalAlm.struHandle.byRecordChannel[x] = (stAlarm->RecordMask & (0x01<<x)) ? 1 : 0;
					}
					if (stAlarm->AlarmTipEn)
					{
						stLocalAlm.struHandle.dwActionFlag |= ALARM_TIP;
					}
					if (stAlarm->dbTourEn)
					{
						stLocalAlm.struHandle.dwActionFlag |= ALARM_TOUR;
					}
					if (stAlarm->Mailer)
					{
						stLocalAlm.struHandle.dwActionFlag |= ALARM_MAIL;
					}
					if (stAlarm->SendAlarmMsg)
					{
						stLocalAlm.struHandle.dwActionFlag |= ALARM_UPLOAD;
					}
					for (x = 0; x < ALARM_TIME_SECT_NUM; x++)
					{
						for (int j = 0; j < DAYS_PER_WEEK; j++)
						{
							//memcpy(&stAS.struLocalAlmIn[i].stSect[j][x], &stAlarm->sAlarmSects[x], sizeof(TSECT));
						}
						//这里的wEvent是一个“提示”和“报警输出”的使能掩码。
						//	stAS.struLocalAlmIn[i].struHandle.stAlarmOutSect[x].wEvent = ((WORD)stAlarm->dwSectMask[x]) & 0x000F;
					}
					//	stAS.struLocalAlmIn[i].struHandle.dwActionFlag |= (stAlarm->dwSectMask[0] & 0x01)?ALARM_TIP:0;
					//	stAS.struLocalAlmIn[i].struHandle.dwActionFlag |= (stAlarm->dwSectMask[0]>>1) & 0x01?ALARM_OUT:0;
					
					stLocalAlm.struHandle.dwRecLatch = stAlarm->TimePreRec;
					stLocalAlm.struHandle.dwDuration = stAlarm->TimeDelay;
				}
				
				nRet = 0;
				
			}
			else
			{
				nRet = -1;
				goto END;
			}
			
			ALARM_CONTROL tmpAlarmCfg[CONFIG_ALARM_NUM_EX] = {0};
			int iAlarmIn = 0;
			nRet = m_pManager->GetDevConfig().QueryIOControlState(lLoginID, IOTYPE_ALARMINPUT, NULL, 0, &iAlarmIn, waittime);
			if (nRet >= 0)
			{
				if (iAlarmIn <= CONFIG_ALARM_NUM_EX || lChannel < iAlarmIn)
				{
					nRet = m_pManager->GetDevConfig().QueryIOControlState(lLoginID, IOTYPE_ALARMINPUT, tmpAlarmCfg, CONFIG_ALARM_NUM_EX*sizeof(ALARM_CONTROL), &iAlarmIn, waittime);
					iAlarmIn = Min(iAlarmIn, CONFIG_ALARM_NUM);
					if (nRet >= 0)
					{	
						stLocalAlm.byAlarmEn = tmpAlarmCfg[iAlarmIn].state;	
					}
				}
			}
			else
			{
				nRet = -1;
				goto END;
			}
		}
	}
	
END:
	if (cRecvBuf) 
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	if (aBuf) 
	{
		delete []aBuf;
		aBuf = NULL;
	}
	
	return nRet;
}

int	CDevConfigEx::SetDevNewConfig_LocalALMCfg(LONG lLoginID, ALARM_IN_CFG_EX* pLocalAlm, LONG lChannel, int waittime)
{
	afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device)
	{
		return -1;
	}
	
	if (NULL == pLocalAlm)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int retlen = 0; //data length
	int nRet = -1;  //return value
	
	int iAlarmNum = device->alarminputcount(device);
	int iChanNum = iAlarmNum>16?iAlarmNum:16;
	
	int iChanCount = 1;
	int iChanIdx = lChannel;
	
	if (-1 == lChannel)
	{
		iChanIdx = 0;
		iChanCount = iAlarmNum;
	}
	
	ALARM_IN_CFG_EX *const pAlarmCfg = pLocalAlm;
	ALARM_IN_CFG_EX *pTmpAlarmCfg = pAlarmCfg;
	int i = 0,j =0;
	int iTmpIdx = iChanIdx;
	
	int buflen = 0;
	char *vbuf = NULL;

	int oldbuflen = 0;
	char *voldbuf = NULL;

	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
	
	if ((iAlarmNum != 0) && (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF]) )
	{
		int iProtocolVersion;
		device->get_info(device, dit_protocol_version, &iProtocolVersion);
		if (iProtocolVersion > 2)//新的报警配置
		{
			if (buflen < iChanNum * sizeof(CONFIG_ALARM_X))
			{
				buflen = iChanNum * sizeof(CONFIG_ALARM_X);
			}
			if (buflen < iChanNum * sizeof(CONFIG_WORKSHEET)) 
			{
				buflen = iChanNum * sizeof(CONFIG_WORKSHEET);
			}
			vbuf = new char[buflen];
			if (NULL == vbuf)
			{
				nRet = -1;
				goto END;
			}
			
			pTmpAlarmCfg = pAlarmCfg;
			iTmpIdx = iChanIdx;
			memset(vbuf, 0, buflen);
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_LOCALALM, 0, vbuf, buflen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_ALARM_X) == 0) && (retlen/sizeof(CONFIG_ALARM_X) >= iAlarmNum))
			{
				CONFIG_ALARM_X *pCfgAlmX = (CONFIG_ALARM_X *)(vbuf + iTmpIdx*sizeof(CONFIG_ALARM_X));
				int nCount = retlen/sizeof(CONFIG_ALARM_X);
				for (i = 0; i < iChanCount; i++)
				{
					m_pManager->GetDevConfig().SetAlmActionFlag(&pCfgAlmX->hEvent, pTmpAlarmCfg->struHandle.dwActionFlag);
					
					pCfgAlmX->bEnable = pTmpAlarmCfg->byAlarmEn;
					pCfgAlmX->iSensorType = pTmpAlarmCfg->byAlarmType;
					
					pCfgAlmX->hEvent.dwRecord = 0;
					pCfgAlmX->hEvent.dwTour = 0;
					pCfgAlmX->hEvent.dwSnapShot = 0;
					
					int index = min(32, device->channelcount(device));
					for (j = 0; j < index; j++)
					{
						pCfgAlmX->hEvent.dwSnapShot |= (pTmpAlarmCfg->struHandle.bySnap[j]) ? (0x01<<j) : 0;
						pCfgAlmX->hEvent.dwTour |= (pTmpAlarmCfg->struHandle.byTour[j]) ? (0x01<<j) : 0;
						pCfgAlmX->hEvent.dwRecord |= (pTmpAlarmCfg->struHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
						
					}
					
					for(j = 0; j < 16; j++)
					{
						pCfgAlmX->hEvent.PtzLink[j].iValue = pTmpAlarmCfg->struHandle.struPtzLink[j].iValue;
						pCfgAlmX->hEvent.PtzLink[j].iType = pTmpAlarmCfg->struHandle.struPtzLink[j].iType;

					}
					pCfgAlmX->hEvent.dwAlarmOut = 0;
					pCfgAlmX->hEvent.wiAlarmOut = 0;
					index = min(32, device->alarmoutputcount(device));
					for (j = 0; j < index; j++)
					{
						pCfgAlmX->hEvent.dwAlarmOut |= (pTmpAlarmCfg->struHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
						pCfgAlmX->hEvent.wiAlarmOut |= (pTmpAlarmCfg->struHandle.byRelWIAlarmOut[j]) ?(0x01<<j) : 0;
					}
					pCfgAlmX->hEvent.iAOLatch = pTmpAlarmCfg->struHandle.dwDuration;
					pCfgAlmX->hEvent.iRecordLatch = pTmpAlarmCfg->struHandle.dwRecLatch;//把预录时间取消用来表示录像延时，变量名待改
					pCfgAlmX->hEvent.iEventLatch = pTmpAlarmCfg->struHandle.dwEventLatch;
					pCfgAlmX->hEvent.bMessagetoNet = (BOOL)pTmpAlarmCfg->struHandle.bMessageToNet;
					pCfgAlmX->hEvent.bMMSEn = pTmpAlarmCfg->struHandle.bMMSEn;
					pCfgAlmX->hEvent.SnapshotTimes = pTmpAlarmCfg->struHandle.bySnapshotTimes;//短信发送图片的张数
					pCfgAlmX->hEvent.bLog = (BOOL)pTmpAlarmCfg->struHandle.bLog;
					pCfgAlmX->hEvent.bMatrixEn = (BOOL)pTmpAlarmCfg->struHandle.bMatrixEn;
					pCfgAlmX->hEvent.dwMatrix = pTmpAlarmCfg->struHandle.dwMatrix;
					
					iTmpIdx ++;
					pTmpAlarmCfg++;
					pCfgAlmX++;
				}
				
				nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_ALARM_LOCALALM, 0, vbuf, nCount*sizeof(CONFIG_ALARM_X), 2000);
				if (nRet < 0)
				{
					nRet = -1;
					goto END;
				}
			}
			
			Sleep(SETUP_SLEEP);	

			{
				memset(vbuf, 0, buflen);	
				CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)vbuf;
				if (-1 != lChannel) 
				{
					pWSheet->iName = lChannel;
					memcpy(pWSheet->tsSchedule, pLocalAlm->stSect, sizeof(pWSheet->tsSchedule));
					nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (WSHEET_LOCAL_ALARM<<16)|(lChannel+1), (char*)vbuf, sizeof(CONFIG_WORKSHEET), 1000);
					if (nRet < 0)
					{
						nRet = -1;
						goto END;
					}
				}
				else
				{
					for (i = 0; i < iChanCount; i++, pWSheet++)
					{
						pWSheet->iName = i;
						memcpy(pWSheet->tsSchedule, pLocalAlm[i].stSect, sizeof(pWSheet->tsSchedule));
					}

					nRet = m_pManager->GetDevConfig().SetDevConfig_WorkSheet(lLoginID, WSHEET_LOCAL_ALARM, (CONFIG_WORKSHEET*)vbuf, waittime, iChanCount);
					if (nRet < 0)
					{
						nRet = -1;
						goto END;
					}
				}		
			}
			
			nRet = 0;
			
		}
		else//老配置
		{
			if (oldbuflen < iChanNum*sizeof(CONFIG_ALARM))
			{
				oldbuflen = iChanNum*sizeof(CONFIG_ALARM);
			}
			
			voldbuf = new char[oldbuflen];
			if (NULL == voldbuf)
			{
				nRet = -1;
				goto END;
			}

			pTmpAlarmCfg = pAlarmCfg;
			iTmpIdx = iChanIdx;
			memset((void *)voldbuf, 0, oldbuflen);

			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM, 0, voldbuf, oldbuflen, &retlen, 2000);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_ALARM) == 0) &&(retlen <= iChanNum * sizeof(CONFIG_ALARM)))
			{
				CONFIG_ALARM *pAlarmCfg  = (CONFIG_ALARM *)(voldbuf + iTmpIdx*sizeof(CONFIG_ALARM));
				for (i = 0; i < iChanCount; i++)
				{
					pAlarmCfg->AlarmMask = 0;
					for (j = 0; j < MAX_ALARM_OUT_NUM; j++)
					{
						if (pTmpAlarmCfg->struHandle.byRelAlarmOut[j])
						{
							pAlarmCfg->AlarmMask |= (1<<j);
						}
					}
					pAlarmCfg->AlarmTipEn = (pTmpAlarmCfg->struHandle.dwActionFlag&ALARM_TIP)?1:0;
					pAlarmCfg->dbTourEn = (pTmpAlarmCfg->struHandle.dwActionFlag&ALARM_TOUR)?1:0;
					pAlarmCfg->Mailer = (pTmpAlarmCfg->struHandle.dwActionFlag&ALARM_MAIL)?1:0;
					pAlarmCfg->RecordMask = 0;
					for (j = 0; j < MAX_VIDEO_IN_NUM; j++)
					{
						if (pTmpAlarmCfg->struHandle.byRecordChannel[j])
						{
							pAlarmCfg->RecordMask |= (1<<j);
						}
					}
					pAlarmCfg->SendAlarmMsg = (pTmpAlarmCfg->struHandle.dwActionFlag&ALARM_UPLOAD)?1:0;
					pAlarmCfg->SensorType = pTmpAlarmCfg->byAlarmType;
					pAlarmCfg->TimeDelay = pTmpAlarmCfg->struHandle.dwDuration;
					pAlarmCfg->TimePreRec = pTmpAlarmCfg->struHandle.dwRecLatch;
					int iMinIndex = ALARM_TIME_SECT_NUM<ALARM_SECTS?ALARM_TIME_SECT_NUM:ALARM_SECTS; 
					pAlarmCfg[i].dwSectMask[j] = 0;
					for (j = 0; j < iMinIndex; j++)
					{
						if (pTmpAlarmCfg->struHandle.dwActionFlag&ALARM_OUT)
						{
							pAlarmCfg[i].dwSectMask[j] |= 0x02;
						}
						if (pTmpAlarmCfg->struHandle.dwActionFlag&ALARM_TIP)
						{
							pAlarmCfg[i].dwSectMask[j] |= 0x01;
						}
					}

					iTmpIdx ++;
					pTmpAlarmCfg++;
					pAlarmCfg++;
				}
				nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_ALARM, 0, voldbuf, iChanCount*sizeof(CONFIG_ALARM), 2000);
				if (nRet < 0)
				{
					nRet = -1;
					goto END;	
				}
				
				nRet = 0;
			}
			else
			{
				nRet = -1;
				goto END;
			}
			
			ALARM_CONTROL tmpAlarmCfg[CONFIG_ALARM_NUM] = {0};
			int iAlarmIn = 0;
			nRet = m_pManager->GetDevConfig().QueryIOControlState(lLoginID, IOTYPE_ALARMINPUT, NULL, 0, &iAlarmIn, 2000);
			if (nRet >= 0)
			{
				iAlarmIn = Min(iAlarmIn, CONFIG_ALARM_NUM);
				for (i = 0; i < iAlarmIn; i++)
				{
					tmpAlarmCfg[i].index = i;
					tmpAlarmCfg[i].state = pLocalAlm->byAlarmEn; //stAS.struLocalAlmIn[i].byAlarmEn;
				}
				nRet = m_pManager->GetDevConfig().IOControl(lLoginID, IOTYPE_ALARMINPUT, tmpAlarmCfg, iAlarmIn*sizeof(ALARM_CONTROL), waittime);
				if (nRet < 0)
				{
					nRet = -1;
					goto END;
				}
				
			}
			else
			{
				nRet = -1;
				goto END;
			}
		}
	}
END:
	if (vbuf) 
	{
		delete []vbuf;
		vbuf = NULL;
	}
	if (voldbuf) 
	{
		delete []voldbuf;
		voldbuf = NULL;
	}
	
	return nRet;
}

//net alarm
int	CDevConfigEx::GetDevNewConfig_AllNetALMCfg(LONG lLoginID, ALARM_IN_CFG_EX *pstNetlAlm, LONG lChnNum, int waittime)
{
	afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device || NULL == pstNetlAlm)
	{
		return -1;
	}
	
	memset(pstNetlAlm, 0, lChnNum*sizeof(ALARM_IN_CFG_EX));
	
	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
	
	int nRet = -1;
	int retlen = 0;
	
	int i = 0, x = 0;
	
	int nAlarmInNum = device->alarminputcount(device);
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	
	if ((nAlarmInNum != 0) && (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF]) )
	{
		int iProtocolVersion;
		device->get_info(device, dit_protocol_version, &iProtocolVersion);
		if (iProtocolVersion > 2)//新的报警配置
		{
			int nSheetNum = 16;
			
			if (iRecvBufLen < lChnNum * sizeof(CONFIG_ALARM_X))
			{
				iRecvBufLen = lChnNum * sizeof(CONFIG_ALARM_X);
			}
			
			cRecvBuf = new char [iRecvBufLen];
			if (NULL == cRecvBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(cRecvBuf, 0, iRecvBufLen);
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_NETALM, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_ALARM_X) == 0))
			{
				device->get_info(device, dit_alarmin_num, &nSheetNum);
				
				int j = 0;
				int nCount = retlen/sizeof(CONFIG_ALARM_X);
				if (nCount > nAlarmInNum) 
				{
					nCount = nAlarmInNum;
				}
				CONFIG_ALARM_X *pCfgAlmX = (CONFIG_ALARM_X *)cRecvBuf;
				for (i = 0; i < nCount; i++, pCfgAlmX++)
				{
					m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_LOCALALM, &pstNetlAlm[i].struHandle.dwActionMask);
					pstNetlAlm[i].byAlarmEn = pCfgAlmX->bEnable;
					pstNetlAlm[i].byAlarmType = pCfgAlmX->iSensorType;
					int index = min(32, device->channelcount(device));
					for (j = 0; j < index; j++)
					{
						
						pstNetlAlm[i].struHandle.byRecordChannel[j] = BITRHT(pCfgAlmX->hEvent.dwRecord,j)&1;
						pstNetlAlm[i].struHandle.byTour[j] = BITRHT(pCfgAlmX->hEvent.dwTour,j)&1;
						pstNetlAlm[i].struHandle.bySnap[j] = BITRHT(pCfgAlmX->hEvent.dwSnapShot,j)&1;
					}
					for(j = 0; j < 16; j++)
					{
						pstNetlAlm[i].struHandle.struPtzLink[j].iValue = pCfgAlmX->hEvent.PtzLink[j].iValue;
						pstNetlAlm[i].struHandle.struPtzLink[j].iType = pCfgAlmX->hEvent.PtzLink[j].iType;
					}
					index = min(32, device->alarmoutputcount(device));
					for (j = 0; j < index; j++)
					{
						pstNetlAlm[i].struHandle.byRelAlarmOut[j] = BITRHT(pCfgAlmX->hEvent.dwAlarmOut,j)&1;
						pstNetlAlm[i].struHandle.byRelWIAlarmOut[j] = BITRHT(pCfgAlmX->hEvent.wiAlarmOut,j)&1;
					}
					pstNetlAlm[i].struHandle.dwDuration = pCfgAlmX->hEvent.iAOLatch;
					pstNetlAlm[i].struHandle.dwRecLatch = pCfgAlmX->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
					pstNetlAlm[i].struHandle.dwEventLatch = pCfgAlmX->hEvent.iEventLatch;
					pstNetlAlm[i].struHandle.bMessageToNet = (BYTE)pCfgAlmX->hEvent.bMessagetoNet;
					pstNetlAlm[i].struHandle.bMMSEn = (BYTE)pCfgAlmX->hEvent.bMMSEn;
					pstNetlAlm[i].struHandle.bySnapshotTimes = pCfgAlmX->hEvent.SnapshotTimes;//短信发送图片的张数
					pstNetlAlm[i].struHandle.bLog = (BYTE)pCfgAlmX->hEvent.bLog;
					pstNetlAlm[i].struHandle.bMatrixEn = (BYTE)pCfgAlmX->hEvent.bMatrixEn;
					pstNetlAlm[i].struHandle.dwMatrix = pCfgAlmX->hEvent.dwMatrix;
					m_pManager->GetDevConfig().GetAlmActionFlag(pCfgAlmX->hEvent, &pstNetlAlm[i].struHandle.dwActionFlag);
				}
			}
			else
			{
				nRet = -1;
				goto END;
			}

			nSheetNum = nSheetNum>lChnNum?lChnNum:nSheetNum;		
			for (i=0; i < nSheetNum; i++)
			{
				CONFIG_WORKSHEET stWSheet = {0};
				nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (WSHEET_NET_ALARM<<16)|(i+1), (char*)&stWSheet, sizeof(CONFIG_WORKSHEET), &retlen, waittime);
				if (nRet < 0 || retlen != sizeof(CONFIG_WORKSHEET))
				{
					nRet = -1;
					goto END;
				}
				
				memcpy(pstNetlAlm[i].stSect, &stWSheet.tsSchedule, sizeof(stWSheet.tsSchedule));
				
			}
			
			nRet = 0;
		}
		else //旧的报警
		{
			
			nRet = -1;
			goto END;
		}	
	}
	
END:
	if (cRecvBuf) 
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	
	return nRet;
}

int	CDevConfigEx::GetDevNewConfig_NetALMCfg(LONG lLoginID, ALARM_IN_CFG_EX &stNetAlm, LONG lChannel, int waittime)
{
	afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device)
	{
		return -1;
	}
	if (lChannel < 0 || lChannel >= device->alarminputcount(device))
	{
		return -1;
	}

	memset(&stNetAlm, 0, sizeof(ALARM_IN_CFG_EX));
	
	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
	
	int nRet = -1;
	int retlen = 0;
	
	int i = 0, x = 0;
	
	int nAlarmInNum = device->alarminputcount(device);
	int lChnNum = nAlarmInNum > 16?nAlarmInNum:16;
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	
	if ((nAlarmInNum != 0) && (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF]) )
	{
		int iProtocolVersion;
		device->get_info(device, dit_protocol_version, &iProtocolVersion);
		if (iProtocolVersion > 2)//新的报警配置
		{
			if (iRecvBufLen < lChnNum * sizeof(CONFIG_ALARM_X))
			{
				iRecvBufLen = lChnNum * sizeof(CONFIG_ALARM_X);
			}
			
			cRecvBuf = new char [iRecvBufLen];
			if (NULL == cRecvBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(cRecvBuf, 0, iRecvBufLen);
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_NETALM, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_ALARM_X) == 0) && (retlen/sizeof(CONFIG_ALARM_X) > lChannel))
			{	
				int j = 0;
				CONFIG_ALARM_X *pCfgAlmX = (CONFIG_ALARM_X *)(cRecvBuf + lChannel*sizeof(CONFIG_ALARM_X));
				
				m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_LOCALALM, &stNetAlm.struHandle.dwActionMask);
				stNetAlm.byAlarmEn = pCfgAlmX->bEnable;
				stNetAlm.byAlarmType = pCfgAlmX->iSensorType;
				int index = min(32, device->channelcount(device));
				for (j = 0; j < index; j++)
				{
					stNetAlm.struHandle.byRecordChannel[j] = BITRHT(pCfgAlmX->hEvent.dwRecord,j)&1;
					stNetAlm.struHandle.byTour[j] = BITRHT(pCfgAlmX->hEvent.dwTour,j)&1;
					stNetAlm.struHandle.bySnap[j] = BITRHT(pCfgAlmX->hEvent.dwSnapShot,j)&1;
				}
				for(j = 0; j < 16; j++)
				{
					stNetAlm.struHandle.struPtzLink[j].iValue = pCfgAlmX->hEvent.PtzLink[j].iValue;
					stNetAlm.struHandle.struPtzLink[j].iType = pCfgAlmX->hEvent.PtzLink[j].iType;
				}
				index = min(32, device->alarmoutputcount(device));
				for (j = 0; j < index; j++)
				{
					stNetAlm.struHandle.byRelAlarmOut[j] = BITRHT(pCfgAlmX->hEvent.dwAlarmOut,j)&1;
					stNetAlm.struHandle.byRelWIAlarmOut[j] =BITRHT(pCfgAlmX->hEvent.wiAlarmOut,j)&1;
				}
				stNetAlm.struHandle.dwDuration = pCfgAlmX->hEvent.iAOLatch;
				stNetAlm.struHandle.dwRecLatch = pCfgAlmX->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
				stNetAlm.struHandle.dwEventLatch = pCfgAlmX->hEvent.iEventLatch;//报警延时
				stNetAlm.struHandle.bMessageToNet = (BYTE)pCfgAlmX->hEvent.bMessagetoNet;
				stNetAlm.struHandle.bMMSEn = (BYTE)pCfgAlmX->hEvent.bMMSEn;
				stNetAlm.struHandle.bySnapshotTimes = pCfgAlmX->hEvent.SnapshotTimes;//短信发送图片的张数
				stNetAlm.struHandle.bLog = (BOOL)pCfgAlmX->hEvent.bLog;
				stNetAlm.struHandle.bMatrixEn = (BOOL)pCfgAlmX->hEvent.bMatrixEn;
				stNetAlm.struHandle.dwMatrix = pCfgAlmX->hEvent.dwMatrix;
				m_pManager->GetDevConfig().GetAlmActionFlag(pCfgAlmX->hEvent, &stNetAlm.struHandle.dwActionFlag);
				
			}
			else
			{
				nRet = -1;
				goto END;
			}
				
			CONFIG_WORKSHEET stWSheet = {0};
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (WSHEET_NET_ALARM<<16)|(lChannel+1), (char*)&stWSheet, sizeof(CONFIG_WORKSHEET), &retlen, waittime);
			if (nRet < 0 || retlen != sizeof(CONFIG_WORKSHEET))
			{
				nRet = -1;
				goto END;
			}
			
			memcpy(stNetAlm.stSect, &stWSheet.tsSchedule, sizeof(stWSheet.tsSchedule));
				
			nRet = 0;
		}
		else //旧的报警
		{	
			nRet = -1;
			goto END;
		}
	}
	
END:
	if (cRecvBuf) 
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	
	return nRet;
}

int	CDevConfigEx::SetDevNewConfig_NetALMCfg(LONG lLoginID, ALARM_IN_CFG_EX* pNetAlm, LONG lChannel, int waittime)
{
		afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device)
	{
		return -1;
	}
	
	if (NULL == pNetAlm)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int retlen = 0; //data length
	int nRet = -1;  //return value
	
	int iAlarmNum = device->alarminputcount(device);
	int iChanNum = iAlarmNum>16?iAlarmNum:16;
	
	int iChanCount = 1;
	int iChanIdx = lChannel;
	
	if (-1 == lChannel)
	{
		iChanIdx = 0;
		iChanCount = iAlarmNum;
	}
	
	ALARM_IN_CFG_EX *const pAlarmCfg = pNetAlm;
	ALARM_IN_CFG_EX *pTmpAlarmCfg = pAlarmCfg;
	int i = 0,j =0;
	int iTmpIdx = iChanIdx;
	int buflen = 0;
	char *vbuf = NULL;

	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
	
	if ((iAlarmNum != 0) && (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF]) )
	{
		int iProtocolVersion;
		device->get_info(device, dit_protocol_version, &iProtocolVersion);
		if (iProtocolVersion > 2)//新的报警配置
		{
			if (buflen < iChanNum * sizeof(CONFIG_ALARM_X))
			{
				buflen = iChanNum * sizeof(CONFIG_ALARM_X);
			}
			if (buflen < iChanNum * sizeof(CONFIG_WORKSHEET)) 
			{
				buflen = iChanNum * sizeof(CONFIG_WORKSHEET);
			}
			vbuf = new char[buflen];
			if (NULL == vbuf)
			{
				nRet = -1;
				goto END;
			}
			
			pTmpAlarmCfg = pAlarmCfg;
			iTmpIdx = iChanIdx;
			memset(vbuf, 0, buflen);
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_NETALM, 0, vbuf, buflen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_ALARM_X) == 0) && (retlen/sizeof(CONFIG_ALARM_X) >= iAlarmNum))
			{
				CONFIG_ALARM_X *pCfgAlmX = (CONFIG_ALARM_X *)(vbuf + iTmpIdx*sizeof(CONFIG_ALARM_X));
				int nCount = retlen/sizeof(CONFIG_ALARM_X);
				for (i = 0; i < iChanCount; i++)
				{
					m_pManager->GetDevConfig().SetAlmActionFlag(&pCfgAlmX->hEvent, pTmpAlarmCfg->struHandle.dwActionFlag);
					
					pCfgAlmX->bEnable = pTmpAlarmCfg->byAlarmEn;
					pCfgAlmX->iSensorType = pTmpAlarmCfg->byAlarmType;
					
					pCfgAlmX->hEvent.dwRecord = 0;
					pCfgAlmX->hEvent.dwTour = 0;
					pCfgAlmX->hEvent.dwSnapShot = 0;
					
					int index = min(32, device->channelcount(device));
					for (j = 0; j < index; j++)
					{
						pCfgAlmX->hEvent.dwSnapShot |= (pTmpAlarmCfg->struHandle.bySnap[j]) ? (0x01<<j) : 0;
						pCfgAlmX->hEvent.dwTour |= (pTmpAlarmCfg->struHandle.byTour[j]) ? (0x01<<j) : 0;
						pCfgAlmX->hEvent.dwRecord |= (pTmpAlarmCfg->struHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
						
					}

					for(j = 0; j < 16; j++)
					{
						pCfgAlmX->hEvent.PtzLink[j].iValue = pTmpAlarmCfg->struHandle.struPtzLink[j].iValue;
						pCfgAlmX->hEvent.PtzLink[j].iType = pTmpAlarmCfg->struHandle.struPtzLink[j].iType;
					}
					
					pCfgAlmX->hEvent.dwAlarmOut = 0;
					pCfgAlmX->hEvent.wiAlarmOut = 0;
					index = min(32, device->alarmoutputcount(device));
					for (j = 0; j < index; j++)
					{
						pCfgAlmX->hEvent.dwAlarmOut |= (pTmpAlarmCfg->struHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
						pCfgAlmX->hEvent.wiAlarmOut |= (pTmpAlarmCfg->struHandle.byRelWIAlarmOut[j]) ?(0x01<<j) : 0;
					}
					pCfgAlmX->hEvent.iAOLatch = pTmpAlarmCfg->struHandle.dwDuration;
					pCfgAlmX->hEvent.iRecordLatch = pTmpAlarmCfg->struHandle.dwRecLatch;//把预录时间取消用来表示录像延时，变量名待改
					pCfgAlmX->hEvent.iEventLatch = pTmpAlarmCfg->struHandle.dwEventLatch;
					pCfgAlmX->hEvent.bMessagetoNet = (BOOL)pTmpAlarmCfg->struHandle.bMessageToNet;
					pCfgAlmX->hEvent.bMMSEn = pTmpAlarmCfg->struHandle.bMMSEn;
					pCfgAlmX->hEvent.SnapshotTimes = pTmpAlarmCfg->struHandle.bySnapshotTimes;//短信发送图片的张数
					pCfgAlmX->hEvent.bLog = (BOOL)pTmpAlarmCfg->struHandle.bLog;
					pCfgAlmX->hEvent.bMatrixEn = (BOOL)pTmpAlarmCfg->struHandle.bMatrixEn;
					pCfgAlmX->hEvent.dwMatrix = pTmpAlarmCfg->struHandle.dwMatrix;
					
					iTmpIdx ++;
					pTmpAlarmCfg++;
					pCfgAlmX++;
				}
				
				nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_ALARM_NETALM, 0, vbuf, nCount*sizeof(CONFIG_ALARM_X), 2000);
				if (nRet < 0)
				{
					nRet = -1;
					goto END;
				}
			}
			
			Sleep(SETUP_SLEEP);	

			{
				memset(vbuf, 0, buflen);	
				CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)vbuf;
				if (-1 != lChannel) 
				{
					pWSheet->iName = lChannel;
					memcpy(pWSheet->tsSchedule, pNetAlm->stSect, sizeof(pWSheet->tsSchedule));
					nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (WSHEET_NET_ALARM<<16)|(lChannel+1), (char*)vbuf, sizeof(CONFIG_WORKSHEET), 1000);
					if (nRet < 0)
					{
						nRet = -1;
						goto END;
					}
				}
				else
				{
					for (i = 0; i < iChanCount; i++, pWSheet++)
					{
						pWSheet->iName = i;
						memcpy(pWSheet->tsSchedule, pNetAlm[i].stSect, sizeof(pWSheet->tsSchedule));
					}

					nRet = m_pManager->GetDevConfig().SetDevConfig_WorkSheet(lLoginID, WSHEET_NET_ALARM, (CONFIG_WORKSHEET*)vbuf, waittime, iChanCount);
					if (nRet < 0)
					{
						nRet = -1;
						goto END;
					}
				}		
			}
			
			nRet = 0;
			
		}
		else//老配置
		{
			nRet = -1;
			goto END;
		}
	}
END:
	if (vbuf) 
	{
		delete []vbuf;
		vbuf = NULL;
	}
	return nRet;
}

//motion alarm
int	CDevConfigEx::GetDevNewConfig_AllMotionALMCfg(LONG lLoginID, MOTION_DETECT_CFG_EX *pstMotionAlm, LONG lChnNum, int waittime)
{
	afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device || NULL == pstMotionAlm)
	{
		return -1;
	}
	
	memset(pstMotionAlm, 0, lChnNum*sizeof(MOTION_DETECT_CFG_EX));

	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
		
	int nRet = -1;
	int retlen = 0;
	
	int i = 0, x = 0;
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	char *dtBuf = NULL;
	
	if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF] )
	{
		int iProtocolVersion;
		device->get_info(device, dit_protocol_version, &iProtocolVersion);
		if (iProtocolVersion > 2)//新的报警配置
		{
			int nSheetNum = 16;
			
			if (iRecvBufLen < lChnNum * sizeof(CONFIG_MOTIONDETECT))
			{
				iRecvBufLen = lChnNum * sizeof(CONFIG_MOTIONDETECT);
			}
			if (iRecvBufLen < lChnNum * sizeof(CONFIG_WORKSHEET)) 
			{
				iRecvBufLen = lChnNum * sizeof(CONFIG_WORKSHEET);
			}

			cRecvBuf = new char [iRecvBufLen];
			if (NULL == cRecvBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(cRecvBuf, 0, iRecvBufLen);
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_MOTION, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_MOTIONDETECT) == 0))
			{
				nSheetNum = device->channelcount(device);
				int nCount = retlen/sizeof(CONFIG_MOTIONDETECT);
				if (nCount > nSheetNum) 
				{
					nCount = nSheetNum ;
				}
				
				int j = 0;
				CONFIG_MOTIONDETECT *pCfgMtn = (CONFIG_MOTIONDETECT *)cRecvBuf;
				for (i = 0; i < nCount; i++, pCfgMtn++)
				{
					m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_MOTION, &pstMotionAlm[i].struHandle.dwActionMask);
					pstMotionAlm[i].byMotionEn = pCfgMtn->bEnable;
					pstMotionAlm[i].wSenseLevel = pCfgMtn->iLevel;
					
					//Begin: Add by zsc(11402) 2008-1-18
					//	获取动态检测区域的行数和列数
					int nMaxRow = 12;
					int nMaxCol = 16;
					int nSysInfoLen = 0;
					MOTION_DETECT_CAPS SysInfoMotion = {0};
					nRet = m_pManager->GetDevConfig().QuerySystemInfo(lLoginID, SYSTEM_INFO_MOTIONDETECT, (char*)&SysInfoMotion, sizeof(MOTION_DETECT_CAPS), &nSysInfoLen, waittime);
					if (0 == nRet && nSysInfoLen == sizeof(MOTION_DETECT_CAPS))
					{
						if (SysInfoMotion.Enabled == 1)
						{
							nMaxRow = SysInfoMotion.GridRows;
							nMaxCol = SysInfoMotion.GridLines;
						}
					}
					
					pstMotionAlm[i].wMotionRow = nMaxRow;
					pstMotionAlm[i].wMotionCol = nMaxCol;
					//row*col motion region
					for (int x = 0; x < nMaxRow; x++)
					{
						for (int y = 0; y < nMaxCol; y++)
						{
							pstMotionAlm[i].byDetected[x][y] = (pCfgMtn->iRegion[x]&(0x01<<y)) ? 1 : 0;
						}
					}
					//End: zsc(11402)
					
					int index = min(32, device->channelcount(device));
					for (j = 0; j < index; j++)
					{
						
						pstMotionAlm[i].struHandle.byRecordChannel[j] = BITRHT(pCfgMtn->hEvent.dwRecord,j)&1;
						pstMotionAlm[i].struHandle.byTour[j] = BITRHT(pCfgMtn->hEvent.dwTour,j)&1;
						pstMotionAlm[i].struHandle.bySnap[j] = BITRHT(pCfgMtn->hEvent.dwSnapShot,j)&1;
					}
					for(j = 0; j < 16; j++)
					{
						pstMotionAlm[i].struHandle.struPtzLink[j].iValue = pCfgMtn->hEvent.PtzLink[j].iValue;
						pstMotionAlm[i].struHandle.struPtzLink[j].iType = pCfgMtn->hEvent.PtzLink[j].iType;
					}
					index = min(32, device->alarmoutputcount(device));
					for (j = 0; j < index; j++)
					{
						pstMotionAlm[i].struHandle.byRelAlarmOut[j] = BITRHT(pCfgMtn->hEvent.dwAlarmOut,j)&1;
						pstMotionAlm[i].struHandle.byRelWIAlarmOut[j] = BITRHT(pCfgMtn->hEvent.wiAlarmOut,j)&1;
					}
					pstMotionAlm[i].struHandle.dwDuration = pCfgMtn->hEvent.iAOLatch;
					pstMotionAlm[i].struHandle.dwRecLatch = pCfgMtn->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
					pstMotionAlm[i].struHandle.dwEventLatch = pCfgMtn->hEvent.iEventLatch;
					pstMotionAlm[i].struHandle.bMessageToNet = (BYTE)pCfgMtn->hEvent.bMessagetoNet;
					pstMotionAlm[i].struHandle.bMMSEn = (BYTE)pCfgMtn->hEvent.bMMSEn;
					pstMotionAlm[i].struHandle.bySnapshotTimes = pCfgMtn->hEvent.SnapshotTimes;//短信发送图片的张数
					pstMotionAlm[i].struHandle.bLog = (BYTE)pCfgMtn->hEvent.bLog;
					pstMotionAlm[i].struHandle.bMatrixEn = (BYTE)pCfgMtn->hEvent.bMatrixEn;
					pstMotionAlm[i].struHandle.dwMatrix = pCfgMtn->hEvent.dwMatrix;
					m_pManager->GetDevConfig().GetAlmActionFlag(pCfgMtn->hEvent, &pstMotionAlm[i].struHandle.dwActionFlag);
				}
			}
			else
			{
				nRet = -1;
				goto END;
			}

			memset(cRecvBuf, 0, iRecvBufLen);
			nSheetNum = nSheetNum>lChnNum?lChnNum:nSheetNum;
			nRet = m_pManager->GetDevConfig().GetDevConfig_WorkSheet(lLoginID, WSHEET_MOTION, (CONFIG_WORKSHEET*)cRecvBuf, waittime, nSheetNum);
			if (nRet >= 0)
			{
				CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)cRecvBuf;
				for (i = 0; i < nSheetNum; i++,pWSheet++)
				{
					memcpy(pstMotionAlm[i].stSect, pWSheet->tsSchedule, sizeof(pWSheet->tsSchedule));
				}
			}
			else
			{
				nRet = -1;
				goto END;
			}

			nRet = 0;
		}
		else //旧的报警
		{
			dtBuf = new char [lChnNum*sizeof(CONFIG_DETECT)];
			if (NULL == dtBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(dtBuf, 0, lChnNum*sizeof(CONFIG_DETECT));
		
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_DETECT, 0, dtBuf, lChnNum*sizeof(CONFIG_DETECT), &retlen, waittime);
			if ((nRet >=0) && (retlen%sizeof(CONFIG_DETECT) == 0) && (retlen <= lChnNum * sizeof(CONFIG_DETECT)))
			{
				CONFIG_DETECT *stDet = 0;
				int nCount = retlen/sizeof(CONFIG_DETECT);
				for (i = 0; i < nCount; i++)
				{
					pstMotionAlm[i].struHandle.dwActionMask = 0;
					pstMotionAlm[i].struHandle.dwActionMask |= ALARM_OUT;
					pstMotionAlm[i].struHandle.dwActionMask |= ALARM_UPLOAD;
					//pstMotionAlm.struHandle.dwActionMask |= ALARM_TIP;
					pstMotionAlm[i].struHandle.dwActionMask |= ALARM_MAIL;
					pstMotionAlm[i].struHandle.dwActionMask |= ALARM_RECORD;
					
					stDet = (CONFIG_DETECT*)(dtBuf+i*sizeof(CONFIG_DETECT));
					pstMotionAlm[i].byMotionEn = 1; //default enabled
					pstMotionAlm[i].wSenseLevel = stDet->MotionLever;
					
					//Begin: Add by zsc(11402) 2008-1-18
					//	获取动态检测区域的行数和列数
					int nMaxRow = 12;
					int nMaxCol = 16;
					int nSysInfoLen = 0;
					MOTION_DETECT_CAPS SysInfoMotion = {0};
					nRet = m_pManager->GetDevConfig().QuerySystemInfo(lLoginID, SYSTEM_INFO_MOTIONDETECT, (char*)&SysInfoMotion, sizeof(MOTION_DETECT_CAPS), &nSysInfoLen, waittime);
					if (0 == nRet && nSysInfoLen == sizeof(MOTION_DETECT_CAPS))
					{
						if (SysInfoMotion.Enabled == 1)
						{
							nMaxRow = SysInfoMotion.GridRows>18 ? 18 : SysInfoMotion.GridRows;
							nMaxCol = SysInfoMotion.GridLines;
						}
					}
					
					pstMotionAlm[i].wMotionRow = nMaxRow;
					pstMotionAlm[i].wMotionCol = nMaxCol;
					//row*col motion region
					for (x = 0; x < nMaxRow; x++)
					{
						for (int y = 0; y < nMaxCol; y++)
						{
							pstMotionAlm[i].byDetected[x][y] = (stDet->MotionRegion[x] & (0x01<<y)) ? 1 : 0;
						}
					}
					//End: zsc(11402)
					
					for (x = 0; x < device->alarmoutputcount(device); x++)
					{
						pstMotionAlm[i].struHandle.byRelAlarmOut[x] = (stDet->MotionAlarmMask & (0x01<<x)) ? 1 : 0;
						pstMotionAlm[i].struHandle.byRelAlarmOut[x] = (stDet->BlindAlarmMask & (0x01<<x)) ? 1 : 0;
						pstMotionAlm[i].struHandle.byRelAlarmOut[x] = (stDet->LossAlarmMask & (0x01<<x)) ? 1 : 0;
					}
					for (x = 0; x < device->channelcount(device); x++)
					{
						pstMotionAlm[i].struHandle.byRecordChannel[x] = (stDet->MotionRecordMask & (0x01<<x)) ? 1 : 0;
					}
					if (stDet->SendAlarmMsg)
					{
						pstMotionAlm[i].struHandle.dwActionFlag |= ALARM_UPLOAD;
					}
					//default output eanbled
					pstMotionAlm[i].struHandle.dwActionFlag |= ALARM_OUT;
					pstMotionAlm[i].struHandle.dwDuration = stDet->MotionTimeDelay;
					pstMotionAlm[i].struHandle.dwActionFlag |= (stDet->Mailer) ? ALARM_MAIL : 0;
				}
			}
			else
			{
				nRet = -1;
				goto END;;
			}
				
			nRet = 0;
		}
	}
		
END:
	if (cRecvBuf) 
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	if (dtBuf) 
	{
		delete []dtBuf;
		dtBuf = NULL;
	}

	return nRet;	
}
int	CDevConfigEx::GetDevNewConfig_MotionALMCfg(LONG lLoginID, MOTION_DETECT_CFG_EX &stMotionAlm, LONG lChannel, int waittime)
{
	afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device)
	{
		return -1;
	}

	memset(&stMotionAlm, 0, sizeof(MOTION_DETECT_CFG_EX));

	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
		
	int nRet = -1;
	int retlen = 0;
	
	int i = 0, x = 0;
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	char *dtBuf = NULL;

	int lChnNum = device->channelcount(device)>16?device->channelcount(device):16;	
 		
	if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF] )
	{
		int iProtocolVersion;
		device->get_info(device, dit_protocol_version, &iProtocolVersion);
		if (iProtocolVersion > 2)//新的报警配置
		{
			int nSheetNum = 16;
			
			if (iRecvBufLen < lChnNum * sizeof(CONFIG_MOTIONDETECT))
			{
				iRecvBufLen = lChnNum * sizeof(CONFIG_MOTIONDETECT);
			}

			cRecvBuf = new char [iRecvBufLen];
			if (NULL == cRecvBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(cRecvBuf, 0, iRecvBufLen);
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_MOTION, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_MOTIONDETECT) == 0) && (retlen > lChannel * sizeof(CONFIG_MOTIONDETECT)))
			{
				nSheetNum = device->channelcount(device);
			
				int j = 0;
				CONFIG_MOTIONDETECT *pCfgMtn = (CONFIG_MOTIONDETECT *)(cRecvBuf + lChannel*sizeof(CONFIG_MOTIONDETECT));
				
				m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_MOTION, &stMotionAlm.struHandle.dwActionMask);
				stMotionAlm.byMotionEn = pCfgMtn->bEnable;
				stMotionAlm.wSenseLevel = pCfgMtn->iLevel;
				
				//Begin: Add by zsc(11402) 2008-1-18
				//	获取动态检测区域的行数和列数
				int nMaxRow = 12;
				int nMaxCol = 16;
				int nSysInfoLen = 0;
				MOTION_DETECT_CAPS SysInfoMotion = {0};
				nRet = m_pManager->GetDevConfig().QuerySystemInfo(lLoginID, SYSTEM_INFO_MOTIONDETECT, (char*)&SysInfoMotion, sizeof(MOTION_DETECT_CAPS), &nSysInfoLen, waittime);
				if (0 == nRet && nSysInfoLen == sizeof(MOTION_DETECT_CAPS))
				{
					if (SysInfoMotion.Enabled == 1)
					{
						nMaxRow = SysInfoMotion.GridRows;
						nMaxCol = SysInfoMotion.GridLines;
					}
				}
				
				stMotionAlm.wMotionRow = nMaxRow;
				stMotionAlm.wMotionCol = nMaxCol;
				//row*col motion region
				for (int x = 0; x < nMaxRow; x++)
				{
					for (int y = 0; y < nMaxCol; y++)
					{
						stMotionAlm.byDetected[x][y] = (pCfgMtn->iRegion[x]&(0x01<<y)) ? 1 : 0;
					}
				}
				//End: zsc(11402)
				
				int index = min(32, device->channelcount(device));
				for (j = 0; j < index; j++)
				{
					stMotionAlm.struHandle.byRecordChannel[j] = BITRHT(pCfgMtn->hEvent.dwRecord,j)&1;
					stMotionAlm.struHandle.byTour[j] = BITRHT(pCfgMtn->hEvent.dwTour,j)&1;
					stMotionAlm.struHandle.bySnap[j] = BITRHT(pCfgMtn->hEvent.dwSnapShot,j)&1;
				}
				for(j = 0; j < 16; j++)
				{
					stMotionAlm.struHandle.struPtzLink[j].iValue = pCfgMtn->hEvent.PtzLink[j].iValue;
					stMotionAlm.struHandle.struPtzLink[j].iType = pCfgMtn->hEvent.PtzLink[j].iType;
				}
				index = min(32, device->alarmoutputcount(device));
				for (j = 0; j < index; j++)
				{
					stMotionAlm.struHandle.byRelAlarmOut[j] = BITRHT(pCfgMtn->hEvent.dwAlarmOut,j)&1;
					stMotionAlm.struHandle.byRelWIAlarmOut[j] = BITRHT(pCfgMtn->hEvent.wiAlarmOut,j)&1;
				}
				stMotionAlm.struHandle.dwDuration = pCfgMtn->hEvent.iAOLatch;
				stMotionAlm.struHandle.dwRecLatch = pCfgMtn->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
				stMotionAlm.struHandle.dwEventLatch = pCfgMtn->hEvent.iEventLatch;
				stMotionAlm.struHandle.bMessageToNet = (BYTE)pCfgMtn->hEvent.bMessagetoNet;
				stMotionAlm.struHandle.bMMSEn = (BYTE)pCfgMtn->hEvent.bMMSEn;
				stMotionAlm.struHandle.bySnapshotTimes = pCfgMtn->hEvent.SnapshotTimes;//短信发送图片的张数
				stMotionAlm.struHandle.bLog = (BYTE)pCfgMtn->hEvent.bLog;
				stMotionAlm.struHandle.bMatrixEn = (BYTE)pCfgMtn->hEvent.bMatrixEn;
				stMotionAlm.struHandle.dwMatrix = pCfgMtn->hEvent.dwMatrix;
				m_pManager->GetDevConfig().GetAlmActionFlag(pCfgMtn->hEvent, &stMotionAlm.struHandle.dwActionFlag);
				
			}
			else
			{
				nRet = -1;
				goto END;
			}

			CONFIG_WORKSHEET stWSheet = {0};
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (WSHEET_MOTION<<16)|(lChannel+1), (char*)&stWSheet, sizeof(CONFIG_WORKSHEET), &retlen, waittime);
			if (nRet < 0 || retlen != sizeof(CONFIG_WORKSHEET))
			{
				nRet = -1;
				goto END;
			}
			
			memcpy(stMotionAlm.stSect, &stWSheet.tsSchedule, sizeof(stWSheet.tsSchedule));
			
			nRet = 0;
		}
		else //旧的报警
		{
			dtBuf = new char [lChnNum*sizeof(CONFIG_DETECT)];
			if (NULL == dtBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(dtBuf, 0, lChnNum*sizeof(CONFIG_DETECT));
		
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_DETECT, 0, dtBuf, lChnNum*sizeof(CONFIG_DETECT), &retlen, waittime);
			if ((nRet >=0) && (retlen%sizeof(CONFIG_ALARM) == 0)&& (retlen > lChannel * sizeof(CONFIG_ALARM)))
			{
				CONFIG_DETECT *stDet = (CONFIG_DETECT *)(dtBuf + lChannel*sizeof(CONFIG_DETECT));
				
				
				stMotionAlm.struHandle.dwActionMask = 0;
				stMotionAlm.struHandle.dwActionMask |= ALARM_OUT;
				stMotionAlm.struHandle.dwActionMask |= ALARM_UPLOAD;
				//stMotionAlm.struHandle.dwActionMask |= ALARM_TIP;
				stMotionAlm.struHandle.dwActionMask |= ALARM_MAIL;
				stMotionAlm.struHandle.dwActionMask |= ALARM_RECORD;
				
				
				stMotionAlm.byMotionEn = 1; //default enabled
				stMotionAlm.wSenseLevel = stDet->MotionLever;
				
				//Begin: Add by zsc(11402) 2008-1-18
				//	获取动态检测区域的行数和列数
				int nMaxRow = 12;
				int nMaxCol = 16;
				int nSysInfoLen = 0;
				MOTION_DETECT_CAPS SysInfoMotion = {0};
				nRet = m_pManager->GetDevConfig().QuerySystemInfo(lLoginID, SYSTEM_INFO_MOTIONDETECT, (char*)&SysInfoMotion, sizeof(MOTION_DETECT_CAPS), &nSysInfoLen, waittime);
				if (0 == nRet && nSysInfoLen == sizeof(MOTION_DETECT_CAPS))
				{
					if (SysInfoMotion.Enabled == 1)
					{
						nMaxRow = SysInfoMotion.GridRows>18 ? 18 : SysInfoMotion.GridRows;
						nMaxCol = SysInfoMotion.GridLines;
					}
				}
				
				stMotionAlm.wMotionRow = nMaxRow;
				stMotionAlm.wMotionCol = nMaxCol;
				//row*col motion region
				for (x = 0; x < nMaxRow; x++)
				{
					for (int y = 0; y < nMaxCol; y++)
					{
						stMotionAlm.byDetected[x][y] = (stDet->MotionRegion[x] & (0x01<<y)) ? 1 : 0;
					}
				}
				//End: zsc(11402)
				
				for (x = 0; x < device->alarmoutputcount(device); x++)
				{
					stMotionAlm.struHandle.byRelAlarmOut[x] = (stDet->MotionAlarmMask & (0x01<<x)) ? 1 : 0;
					stMotionAlm.struHandle.byRelAlarmOut[x] = (stDet->BlindAlarmMask & (0x01<<x)) ? 1 : 0;
					stMotionAlm.struHandle.byRelAlarmOut[x] = (stDet->LossAlarmMask & (0x01<<x)) ? 1 : 0;
				}
				for (x = 0; x < device->channelcount(device); x++)
				{
					stMotionAlm.struHandle.byRecordChannel[x] = (stDet->MotionRecordMask & (0x01<<x)) ? 1 : 0;
				}
				if (stDet->SendAlarmMsg)
				{
					stMotionAlm.struHandle.dwActionFlag |= ALARM_UPLOAD;
				}
				//default output eanbled
				stMotionAlm.struHandle.dwActionFlag |= ALARM_OUT;
				stMotionAlm.struHandle.dwDuration = stDet->MotionTimeDelay;
				stMotionAlm.struHandle.dwActionFlag |= (stDet->Mailer) ? ALARM_MAIL : 0;
				
			}
			else
			{
				nRet = -1;
				goto END;;
			}
				
			nRet = 0;
		}
	}
		
END:
	if (cRecvBuf) 
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	if (dtBuf) 
	{
		delete []dtBuf;
		dtBuf = NULL;
	}

	return nRet;
}

int	CDevConfigEx::SetDevNewConfig_MotionALMCfg(LONG lLoginID, MOTION_DETECT_CFG_EX* pMotionAlm, LONG lChannel, int waittime)
{
	afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device)
	{
		return -1;
	}
	
	if (NULL == pMotionAlm)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int retlen = 0; //data length
	int nRet = -1;  //return value
	
	int iAlarmNum = device->channelcount(device);
	int iChanNum = iAlarmNum>16?iAlarmNum:16;
	
	int iChanCount = 1;
	int iChanIdx = lChannel;
	
	if (-1 == lChannel)
	{
		iChanIdx = 0;
		iChanCount = iAlarmNum;
	}
	
	MOTION_DETECT_CFG_EX *const pMotionCfg = pMotionAlm;
	MOTION_DETECT_CFG_EX *pTmpMotionCfg = pMotionCfg;
	int i = 0,j =0;
	int iTmpIdx = iChanIdx;
	int buflen = 0;
	char *vbuf = NULL;
	int oldbuflen = 0;
	char *voldbuf = NULL;
	
	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
	
	if ((iAlarmNum != 0) && (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF]) )
	{
		int iProtocolVersion;
		device->get_info(device, dit_protocol_version, &iProtocolVersion);
		if (iProtocolVersion > 2)//新的报警配置
		{
			if (buflen < iChanNum * sizeof(CONFIG_MOTIONDETECT))
			{
				buflen = iChanNum * sizeof(CONFIG_MOTIONDETECT);
			}
			if (buflen < iChanNum * sizeof(CONFIG_WORKSHEET)) 
			{
				buflen = iChanNum * sizeof(CONFIG_WORKSHEET);
			}
			vbuf = new char[buflen];
			if (NULL == vbuf)
			{
				nRet = -1;
				goto END;
			}
			
			pTmpMotionCfg = pMotionCfg;
			iTmpIdx = iChanIdx;
			memset(vbuf, 0, buflen);
			
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_MOTION, 0, vbuf, buflen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_MOTIONDETECT) == 0) && (retlen/sizeof(CONFIG_MOTIONDETECT) >= iAlarmNum))
			{
				int nCount = retlen/sizeof(CONFIG_MOTIONDETECT);
				CONFIG_MOTIONDETECT *pCfgMtn = (CONFIG_MOTIONDETECT *)(vbuf+ iTmpIdx*sizeof(CONFIG_MOTIONDETECT));
				for (i = 0; i < iChanCount; i++)
				{
					m_pManager->GetDevConfig().SetAlmActionFlag(&pCfgMtn->hEvent, pTmpMotionCfg->struHandle.dwActionFlag);
					pCfgMtn->bEnable = pTmpMotionCfg->byMotionEn;
					pCfgMtn->iLevel = pTmpMotionCfg->wSenseLevel;
					
					//Begin: Add by zsc(11402)	2008-1-18
					int nMaxRow	= Min(pTmpMotionCfg->wMotionRow, 32);
					nMaxRow		= Min(nMaxRow, MOTION_ROW_NUM);
					int nMaxCol	= Min(pTmpMotionCfg->wMotionCol, 32);
					nMaxCol		= Min(nMaxCol, MOTION_COL_NUM);
					for (j = 0; j < nMaxRow; j++)
					{
						pCfgMtn->iRegion[j] = 0;
						for (int k = 0; k < nMaxCol; k++)
						{
							if (pTmpMotionCfg->byDetected[j][k])
							{
								pCfgMtn->iRegion[j] |= (1<<k);
							}
						}
					}
					//End:	zsc(11402)	2008-1-18
					
					pCfgMtn->hEvent.dwRecord = 0;
					pCfgMtn->hEvent.dwTour = 0;
					pCfgMtn->hEvent.dwSnapShot = 0;
					
					int index = min(32, device->channelcount(device));
					for (j = 0; j < index; j++)
					{
						pCfgMtn->hEvent.dwSnapShot |= (pTmpMotionCfg->struHandle.bySnap[j]) ? (0x01<<j) : 0;
						pCfgMtn->hEvent.dwTour |= (pTmpMotionCfg->struHandle.byTour[j]) ? (0x01<<j) : 0;
						pCfgMtn->hEvent.dwRecord |= (pTmpMotionCfg->struHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
					}
					for(j = 0; j < 16; j++)
					{
						pCfgMtn->hEvent.PtzLink[j].iValue = pTmpMotionCfg->struHandle.struPtzLink[j].iValue;
						pCfgMtn->hEvent.PtzLink[j].iType = pTmpMotionCfg->struHandle.struPtzLink[j].iType;
					}
					
					pCfgMtn->hEvent.dwAlarmOut = 0;
					pCfgMtn->hEvent.wiAlarmOut = 0;
					index = min(32, device->alarmoutputcount(device));
					for (j = 0; j < index; j++)
					{
						pCfgMtn->hEvent.dwAlarmOut |= (pTmpMotionCfg->struHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
						pCfgMtn->hEvent.wiAlarmOut |= (pTmpMotionCfg->struHandle.byRelWIAlarmOut[j]) ? (0x01<<j) : 0;
					}
					pCfgMtn->hEvent.iAOLatch = pTmpMotionCfg->struHandle.dwDuration;
					pCfgMtn->hEvent.iRecordLatch = pTmpMotionCfg->struHandle.dwRecLatch;
					pCfgMtn->hEvent.iEventLatch = pTmpMotionCfg->struHandle.dwEventLatch;
					pCfgMtn->hEvent.bMessagetoNet = (BOOL)pTmpMotionCfg->struHandle.bMessageToNet;
					pCfgMtn->hEvent.bMMSEn = pTmpMotionCfg->struHandle.bMMSEn;
					pCfgMtn->hEvent.SnapshotTimes = pTmpMotionCfg->struHandle.bySnapshotTimes;//短信发送图片的张数
					pCfgMtn->hEvent.bLog = (BOOL)pTmpMotionCfg->struHandle.bLog;
					pCfgMtn->hEvent.bMatrixEn = (BOOL)pTmpMotionCfg->struHandle.bMatrixEn;
					pCfgMtn->hEvent.dwMatrix = pTmpMotionCfg->struHandle.dwMatrix;
					
					pCfgMtn++;
					pTmpMotionCfg++;
					iTmpIdx++;
					
				}
				
				nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_ALARM_MOTION, 0, vbuf, nCount*sizeof(CONFIG_MOTIONDETECT), 2000);
				if (nRet < 0)
				{
					nRet = -1;
					goto END;
				}
			}
			else
			{
				nRet = -1;
				goto END;

			}
			
			Sleep(SETUP_SLEEP);	
			
			{
				memset(vbuf, 0, buflen);	
				CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)vbuf;
				if (-1 != lChannel) 
				{
					pWSheet->iName = lChannel;
					memcpy(pWSheet->tsSchedule, pMotionAlm->stSect, sizeof(pWSheet->tsSchedule));
					nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (WSHEET_MOTION<<16)|(lChannel+1), (char*)vbuf, sizeof(CONFIG_WORKSHEET), 1000);
					if (nRet < 0)
					{
						nRet = -1;
						goto END;
					}
				}
				else
				{
					for (i = 0; i < iChanCount; i++, pWSheet++)
					{
						pWSheet->iName = i;
						memcpy(pWSheet->tsSchedule, pMotionAlm[i].stSect, sizeof(pWSheet->tsSchedule));
					}
					
					nRet = m_pManager->GetDevConfig().SetDevConfig_WorkSheet(lLoginID, WSHEET_MOTION, (CONFIG_WORKSHEET*)vbuf, waittime, iChanCount);
					if (nRet < 0)
					{
						nRet = -1;
						goto END;
					}
				}		
			}
			
			nRet = 0;	
		}
		else//老配置
		{
			if (oldbuflen < iChanNum*sizeof(CONFIG_DETECT))
			{
				oldbuflen = iChanNum*sizeof(CONFIG_DETECT);
			}
			
			voldbuf = new char[oldbuflen];
			if (NULL == voldbuf)
			{
				nRet = -1;
				goto END;
			}
			
			pTmpMotionCfg = pMotionCfg;
			iTmpIdx = iChanIdx;
			memset((void *)voldbuf, 0, oldbuflen);
			
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_DETECT, 0, voldbuf, oldbuflen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_DETECT) == 0) && (retlen/sizeof(CONFIG_DETECT) >= iAlarmNum))
			{
				int j = 0;
				CONFIG_DETECT * pDetCfg = (CONFIG_DETECT *)(voldbuf + iTmpIdx * sizeof(CONFIG_DETECT));
				for (i = 0; i < iChanCount; i++)
				{
					pDetCfg->MotionAlarmMask = 0;
					for (j = 0; j < device->alarmoutputcount(device); j++)
					{
						if (pTmpMotionCfg->struHandle.byRelAlarmOut[j])
						{
							pDetCfg->MotionAlarmMask |= (1<<j);
						}
					}
					pDetCfg[i].MotionRecordMask = 0;
					for (j = 0; j < device->channelcount(device); j++)
					{
						if (pTmpMotionCfg->struHandle.byRecordChannel[j])
						{
							pDetCfg->MotionRecordMask |= (1<<j);
						}
					}
					pDetCfg->MotionTimeDelay = pTmpMotionCfg->struHandle.dwDuration;
					pDetCfg->Mailer = 0;
					
					if (pTmpMotionCfg->struHandle.dwActionFlag&ALARM_MAIL)
					{
						pDetCfg[i].Mailer = 1;
					}
					pDetCfg->MotionLever = pTmpMotionCfg->wSenseLevel;
					
					//Begin: Add by zsc(11402)	2008-1-18
					int nMaxRow = Min(pTmpMotionCfg->wMotionRow, 18);
					nMaxRow		= Min(nMaxRow, MOTION_ROW_NUM);
					int nMaxCol = Min(pTmpMotionCfg->wMotionCol, 32);
					nMaxCol		= Min(nMaxCol, MOTION_COL_NUM);
					for (j = 0; j < nMaxRow; j++)
					{
						pDetCfg->MotionRegion[j] = 0;
						for (int k = 0; k < nMaxCol; k++)
						{
							if (pTmpMotionCfg->byDetected[j][k])
							{
								pDetCfg->MotionRegion[j] |= (1<<k);
							}
						}
					}
					//End: zsc(11402)
					
					pDetCfg->SendAlarmMsg = 0;
					if (pTmpMotionCfg->struHandle.dwActionFlag&ALARM_UPLOAD)
					{
						pDetCfg->SendAlarmMsg = 1;
					}
					
					iTmpIdx ++;
					pTmpMotionCfg++;
					pDetCfg++;
					
				}
				
				nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_DETECT, 0, voldbuf, oldbuflen, 2000);
				if (nRet < 0)
				{
					nRet = -1;
					goto END;	
				}
				
				nRet = 0;
			}
			else
			{
				nRet = -1;
				goto END;
			}	
		}
	}
END:
	if (vbuf) 
	{
		delete []vbuf;
		vbuf = NULL;
	}
	if (voldbuf) 
	{
		delete voldbuf;
		voldbuf = NULL;			
	}
	return nRet;
}

//video lost alarm
int	CDevConfigEx::GetDevNewConfig_AllVideoLostALMCfg(LONG lLoginID, VIDEO_LOST_CFG_EX *pstVideoLostAlm, LONG lChnNum, int waittime)
{
	afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device || NULL == pstVideoLostAlm)
	{
		return -1;
	}
	
	memset(pstVideoLostAlm, 0, lChnNum*sizeof(VIDEO_LOST_CFG_EX));

	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
		
	int nRet = -1;
	int retlen = 0;
	
	int i = 0, x = 0;
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	char *dtBuf = NULL;
	
	if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF] )
	{
		int iProtocolVersion;
		device->get_info(device, dit_protocol_version, &iProtocolVersion);
		if (iProtocolVersion > 2)//新的报警配置
		{
			int nSheetNum = 16;
			
			if (iRecvBufLen < lChnNum * sizeof(CONFIG_LOSSDETECT))
			{
				iRecvBufLen = lChnNum * sizeof(CONFIG_LOSSDETECT);
			}
			if (iRecvBufLen < lChnNum * sizeof(CONFIG_WORKSHEET)) 
			{
				iRecvBufLen = lChnNum * sizeof(CONFIG_WORKSHEET);
			}

			cRecvBuf = new char [iRecvBufLen];
			if (NULL == cRecvBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(cRecvBuf, 0, iRecvBufLen);
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_LOSS, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_LOSSDETECT) == 0))
			{
				nSheetNum = device->channelcount(device);

				int nCount = retlen/sizeof(CONFIG_LOSSDETECT);
				if (nCount > lChnNum) 
				{
					nCount = lChnNum;
				}
				
				int j = 0;
				CONFIG_LOSSDETECT *pCfgLos = (CONFIG_LOSSDETECT *)cRecvBuf;
				for (i = 0; i < nCount; i++, pCfgLos++)
				{
					m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_LOSS, &pstVideoLostAlm[i].struHandle.dwActionMask);
					pstVideoLostAlm[i].byAlarmEn = pCfgLos->bEnable;
					int index = min(32, device->channelcount(device));
					for (j = 0; j < index; j++)
					{
						pstVideoLostAlm[i].struHandle.byRecordChannel[j] = BITRHT(pCfgLos->hEvent.dwRecord,j)&1;
						pstVideoLostAlm[i].struHandle.byTour[j] = BITRHT(pCfgLos->hEvent.dwTour,j)&1;
						pstVideoLostAlm[i].struHandle.bySnap[j] = BITRHT(pCfgLos->hEvent.dwSnapShot,j)&1;
					}
					for (j = 0; j < 16; j++) 
					{
						pstVideoLostAlm[i].struHandle.struPtzLink[j].iValue = pCfgLos->hEvent.PtzLink[j].iValue;
						pstVideoLostAlm[i].struHandle.struPtzLink[j].iType = pCfgLos->hEvent.PtzLink[j].iType;
					}
					index = min(32, device->alarmoutputcount(device));
					for (j = 0; j < index; j++)
					{
						pstVideoLostAlm[i].struHandle.byRelAlarmOut[j] = BITRHT(pCfgLos->hEvent.dwAlarmOut,j)&1;
						pstVideoLostAlm[i].struHandle.byRelWIAlarmOut[j] =BITRHT(pCfgLos->hEvent.wiAlarmOut,j)&1;
					}
					pstVideoLostAlm[i].struHandle.dwDuration = pCfgLos->hEvent.iAOLatch;
					pstVideoLostAlm[i].struHandle.dwRecLatch = pCfgLos->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
					pstVideoLostAlm[i].struHandle.dwEventLatch = pCfgLos->hEvent.iEventLatch;
					pstVideoLostAlm[i].struHandle.bMessageToNet = (BYTE)pCfgLos->hEvent.bMessagetoNet;
					pstVideoLostAlm[i].struHandle.bMMSEn = (BYTE)pCfgLos->hEvent.bMMSEn;
					pstVideoLostAlm[i].struHandle.bySnapshotTimes = pCfgLos->hEvent.SnapshotTimes;//短信发送图片的张数
					pstVideoLostAlm[i].struHandle.bLog = (BYTE)pCfgLos->hEvent.bLog;
					pstVideoLostAlm[i].struHandle.bMatrixEn = (BYTE)pCfgLos->hEvent.bMatrixEn;
					pstVideoLostAlm[i].struHandle.dwMatrix = pCfgLos->hEvent.dwMatrix;
					m_pManager->GetDevConfig().GetAlmActionFlag(pCfgLos->hEvent, &pstVideoLostAlm[i].struHandle.dwActionFlag);
				}		
			}
			else
			{
				nRet = -1;
				goto END;
			}

			memset(cRecvBuf, 0, iRecvBufLen);
			nRet = m_pManager->GetDevConfig().GetDevConfig_WorkSheet(lLoginID, WSHEET_LOSS, (CONFIG_WORKSHEET*)cRecvBuf, waittime, nSheetNum);
			if (nRet >= 0)
			{
				CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)cRecvBuf;
				for (i = 0; i < nSheetNum; i++,pWSheet++)
				{
					memcpy(pstVideoLostAlm[i].stSect, pWSheet->tsSchedule, sizeof(pWSheet->tsSchedule));
				}
			}
			else
			{
				nRet = -1;
				goto END;
			}

			nRet = 0;
		}
		else //旧的报警
		{
			dtBuf = new char [lChnNum*sizeof(CONFIG_DETECT)];
			if (NULL == dtBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(dtBuf, 0, lChnNum*sizeof(CONFIG_DETECT));
		
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_DETECT, 0, dtBuf, lChnNum*sizeof(CONFIG_DETECT), &retlen, waittime);
			if ((nRet >=0) && (retlen%sizeof(CONFIG_DETECT) == 0) && (retlen <= lChnNum * sizeof(CONFIG_DETECT)))
			{
				CONFIG_DETECT *stDet = 0;
				int nCount = retlen/sizeof(CONFIG_DETECT);
				for (i = 0; i < nCount; i++)
				{
					pstVideoLostAlm[i].struHandle.dwActionMask = 0;
					pstVideoLostAlm[i].struHandle.dwActionMask |= ALARM_OUT;
					pstVideoLostAlm[i].struHandle.dwActionMask |= ALARM_TIP;
					pstVideoLostAlm[i].struHandle.dwActionMask |= ALARM_MAIL;
					
					stDet = (CONFIG_DETECT*)(dtBuf+i*sizeof(CONFIG_DETECT));
					pstVideoLostAlm[i].byAlarmEn = 1; //default enabled

					for (x = 0; x < device->alarmoutputcount(device); x++)
					{
						pstVideoLostAlm[i].struHandle.byRelAlarmOut[x] = (stDet->LossAlarmMask & (0x01<<x)) ? 1 : 0;
					}
					if (stDet->LossTipEn) 
					{
						pstVideoLostAlm[i].struHandle.dwActionFlag |= ALARM_TIP;
					}
					
					//default output eanbled
					pstVideoLostAlm[i].struHandle.dwActionFlag |= ALARM_OUT;
				}
			}
			else
			{
				nRet = -1;
				goto END;;
			}
				
			nRet = 0;
		}
	}
		
END:
	if (cRecvBuf) 
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	if (dtBuf) 
	{
		delete []dtBuf;
		dtBuf = NULL;
	}

	return nRet;	
}


int	CDevConfigEx::GetDevNewConfig_VideoLostALMCfg(LONG lLoginID, VIDEO_LOST_CFG_EX &stVideoLostAlm, LONG lChannel, int waittime)
{
	afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device)
	{
		return -1;
	}

	memset(&stVideoLostAlm, 0, sizeof(VIDEO_LOST_CFG_EX));

	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
		
	int nRet = -1;
	int retlen = 0;
	
	int i = 0, x = 0;
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	char *dtBuf = NULL;

	int lChnNum = device->channelcount(device)>16?device->channelcount(device):16;	
 		
	if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF] )
	{
		int iProtocolVersion;
		device->get_info(device, dit_protocol_version, &iProtocolVersion);
		if (iProtocolVersion > 2)//新的报警配置
		{
			if (iRecvBufLen < lChnNum * sizeof(CONFIG_LOSSDETECT))
			{
				iRecvBufLen = lChnNum * sizeof(CONFIG_LOSSDETECT);
			}

			cRecvBuf = new char [iRecvBufLen];
			if (NULL == cRecvBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(cRecvBuf, 0, iRecvBufLen);
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_LOSS, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_LOSSDETECT) == 0) && (retlen/sizeof(CONFIG_LOSSDETECT) > lChannel))
			{
				int j = 0;
				CONFIG_LOSSDETECT *pCfgLos = (CONFIG_LOSSDETECT *)(cRecvBuf + lChannel*sizeof(CONFIG_LOSSDETECT));
				
				m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_LOSS, &stVideoLostAlm.struHandle.dwActionMask);
				stVideoLostAlm.byAlarmEn = pCfgLos->bEnable;
				int index = min(32, device->channelcount(device));
				for (j = 0; j < index; j++)
				{
					stVideoLostAlm.struHandle.byRecordChannel[j] = BITRHT(pCfgLos->hEvent.dwRecord,j)&1;
					stVideoLostAlm.struHandle.byTour[j] = BITRHT(pCfgLos->hEvent.dwTour,j)&1;
					stVideoLostAlm.struHandle.bySnap[j] = BITRHT(pCfgLos->hEvent.dwSnapShot,j)&1;
				}
				for(j = 0; j <16; j++)
				{
					stVideoLostAlm.struHandle.struPtzLink[j].iValue = pCfgLos->hEvent.PtzLink[j].iValue;
					stVideoLostAlm.struHandle.struPtzLink[j].iType = pCfgLos->hEvent.PtzLink[j].iType;
				}

				index = min(32, device->alarmoutputcount(device));
				for (j = 0; j < index; j++)
				{
					stVideoLostAlm.struHandle.byRelAlarmOut[j] = BITRHT(pCfgLos->hEvent.dwAlarmOut,j)&1;
					stVideoLostAlm.struHandle.byRelWIAlarmOut[j] =BITRHT(pCfgLos->hEvent.wiAlarmOut,j)&1;
				}
				stVideoLostAlm.struHandle.dwDuration = pCfgLos->hEvent.iAOLatch;
				stVideoLostAlm.struHandle.dwRecLatch = pCfgLos->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
				stVideoLostAlm.struHandle.dwEventLatch = pCfgLos->hEvent.iEventLatch;
				stVideoLostAlm.struHandle.bMessageToNet = (BYTE)pCfgLos->hEvent.bMessagetoNet;
				stVideoLostAlm.struHandle.bMMSEn = (BYTE)pCfgLos->hEvent.bMMSEn;
				stVideoLostAlm.struHandle.bySnapshotTimes = pCfgLos->hEvent.SnapshotTimes;//短信发送图片的张数
				stVideoLostAlm.struHandle.bLog = (BYTE)pCfgLos->hEvent.bLog;
				stVideoLostAlm.struHandle.bMatrixEn = (BYTE)pCfgLos->hEvent.bMatrixEn;
				stVideoLostAlm.struHandle.dwMatrix = pCfgLos->hEvent.dwMatrix;
				m_pManager->GetDevConfig().GetAlmActionFlag(pCfgLos->hEvent, &stVideoLostAlm.struHandle.dwActionFlag);				
			}
			else
			{
				nRet = -1;
				goto END;
			}

			CONFIG_WORKSHEET stWSheet = {0};
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (WSHEET_LOSS<<16)|(lChannel+1), (char*)&stWSheet, sizeof(CONFIG_WORKSHEET), &retlen, waittime);
			if (nRet < 0 || retlen != sizeof(CONFIG_WORKSHEET))
			{
				nRet = -1;
				goto END;
			}
			
			memcpy(stVideoLostAlm.stSect, &stWSheet.tsSchedule, sizeof(stWSheet.tsSchedule));
			
			nRet = 0;
		}
		else //旧的报警
		{
			dtBuf = new char [lChnNum*sizeof(CONFIG_DETECT)];
			if (NULL == dtBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(dtBuf, 0, lChnNum*sizeof(CONFIG_DETECT));
		
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_DETECT, 0, dtBuf, lChnNum*sizeof(CONFIG_DETECT), &retlen, waittime);
			if ((nRet >=0) && (retlen%sizeof(CONFIG_ALARM) == 0)&& (retlen/sizeof(CONFIG_ALARM) > lChannel))
			{
				CONFIG_DETECT *stDet = (CONFIG_DETECT *)(dtBuf + lChannel*sizeof(CONFIG_DETECT));
				stVideoLostAlm.struHandle.dwActionMask = 0;
				stVideoLostAlm.struHandle.dwActionMask |= ALARM_OUT;
				stVideoLostAlm.struHandle.dwActionMask |= ALARM_TIP;
				stVideoLostAlm.struHandle.dwActionMask |= ALARM_MAIL;
				
				stVideoLostAlm.byAlarmEn = 1; //default enabled
				
				for (x = 0; x < device->alarmoutputcount(device); x++)
				{
					stVideoLostAlm.struHandle.byRelAlarmOut[x] = (stDet->LossAlarmMask & (0x01<<x)) ? 1 : 0;
				}
				if (stDet->LossTipEn) 
				{
					stVideoLostAlm.struHandle.dwActionFlag |= ALARM_TIP;
				}
				
				//default output eanbled
				stVideoLostAlm.struHandle.dwActionFlag |= ALARM_OUT;
			}
			else
			{
				nRet = -1;
				goto END;;
			}
				
			nRet = 0;
		}
	}
		
END:
	if (cRecvBuf) 
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	if (dtBuf) 
	{
		delete []dtBuf;
		dtBuf = NULL;
	}

	return nRet;
}
int	CDevConfigEx::SetDevNewConfig_VideoLostALMCfg(LONG lLoginID, VIDEO_LOST_CFG_EX* pVideoLostAlm, LONG lChannel, int waittime)
{
	afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device)
	{
		return -1;
	}
	
	if (NULL == pVideoLostAlm)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int retlen = 0; //data length
	int nRet = -1;  //return value
	
	int iAlarmNum = device->channelcount(device);
	int iChanNum = iAlarmNum>16?iAlarmNum:16;
	
	int iChanCount = 1;
	int iChanIdx = lChannel;
	
	if (-1 == lChannel)
	{
		iChanIdx = 0;
		iChanCount = iAlarmNum;
	}
	
	VIDEO_LOST_CFG_EX *const pLossCfg = pVideoLostAlm;
	VIDEO_LOST_CFG_EX *pTmpLossCfg = pLossCfg;
	int i = 0,j =0;
	int iTmpIdx = iChanIdx;
	int buflen = 0;
	char *vbuf = NULL;
	int oldbuflen = 0;
	char *voldbuf = NULL;
	
	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
	
	if ((iAlarmNum != 0) && (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF]) )
	{
		int iProtocolVersion;
		device->get_info(device, dit_protocol_version, &iProtocolVersion);
		if (iProtocolVersion > 2)//新的报警配置
		{
			if (buflen < iChanNum * sizeof(CONFIG_LOSSDETECT))
			{
				buflen = iChanNum * sizeof(CONFIG_LOSSDETECT);
			}
			if (buflen < iChanNum * sizeof(CONFIG_WORKSHEET)) 
			{
				buflen = iChanNum * sizeof(CONFIG_WORKSHEET);
			}
			vbuf = new char[buflen];
			if (NULL == vbuf)
			{
				nRet = -1;
				goto END;
			}
			
			pTmpLossCfg = pLossCfg;
			iTmpIdx = iChanIdx;
			memset(vbuf, 0, buflen);
			
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_LOSS, 0, vbuf, buflen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_LOSSDETECT) == 0) && (retlen/sizeof(CONFIG_LOSSDETECT) >= iAlarmNum))
			{
				CONFIG_LOSSDETECT *pCfgLos = (CONFIG_LOSSDETECT *)(vbuf+ iTmpIdx*sizeof(CONFIG_LOSSDETECT));
				int nCount = retlen/sizeof(CONFIG_LOSSDETECT);
				for (i = 0; i < iChanCount; i++)
				{
					m_pManager->GetDevConfig().SetAlmActionFlag(&pCfgLos->hEvent, pTmpLossCfg->struHandle.dwActionFlag);
					pCfgLos->bEnable = pTmpLossCfg->byAlarmEn;
					
					pCfgLos->hEvent.dwRecord = 0;
					pCfgLos->hEvent.dwTour = 0;
					pCfgLos->hEvent.dwSnapShot = 0;
					
					int index = min(32, device->channelcount(device));
					for (j = 0; j < index; j++)
					{
						pCfgLos->hEvent.dwSnapShot |= (pTmpLossCfg->struHandle.bySnap[j]) ? (0x01<<j) : 0;
						pCfgLos->hEvent.dwTour |= (pTmpLossCfg->struHandle.byTour[j]) ? (0x01<<j) : 0;
						pCfgLos->hEvent.dwRecord |= (pTmpLossCfg->struHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
					}

					for(j = 0; j < 16; j++)
					{
						pCfgLos->hEvent.PtzLink[j].iValue = pTmpLossCfg->struHandle.struPtzLink[j].iValue;
						pCfgLos->hEvent.PtzLink[j].iType = pTmpLossCfg->struHandle.struPtzLink[j].iType;
					}
					
					pCfgLos->hEvent.dwAlarmOut = 0;
					pCfgLos->hEvent.wiAlarmOut = 0;
					index = min(32, device->alarmoutputcount(device));
					for (j = 0; j < index; j++)
					{
						pCfgLos->hEvent.dwAlarmOut |= (pTmpLossCfg->struHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
						pCfgLos->hEvent.wiAlarmOut |= (pTmpLossCfg->struHandle.byRelWIAlarmOut[j]) ? (0x01<<j) : 0;
					}
					pCfgLos->hEvent.iAOLatch = pTmpLossCfg->struHandle.dwDuration;
					pCfgLos->hEvent.iRecordLatch = pTmpLossCfg->struHandle.dwRecLatch;
					pCfgLos->hEvent.iEventLatch = pTmpLossCfg->struHandle.dwEventLatch;
					pCfgLos->hEvent.bMessagetoNet = (BOOL)pTmpLossCfg->struHandle.bMessageToNet;
					pCfgLos->hEvent.bMMSEn = pTmpLossCfg->struHandle.bMMSEn;
					pCfgLos->hEvent.SnapshotTimes = pTmpLossCfg->struHandle.bySnapshotTimes;//短信发送图片的张数
					pCfgLos->hEvent.bLog = (BOOL)pTmpLossCfg->struHandle.bLog;
					pCfgLos->hEvent.bMatrixEn = (BOOL)pTmpLossCfg->struHandle.bMatrixEn;
					pCfgLos->hEvent.dwMatrix = pTmpLossCfg->struHandle.dwMatrix;

					pTmpLossCfg++;
					pCfgLos++;
					iTmpIdx++;
				}
			
				nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_ALARM_LOSS, 0, vbuf, nCount*sizeof(CONFIG_LOSSDETECT), 2000);
				if (nRet < 0)
				{
					nRet = -1;
					goto END;
				}
			}
			else
			{
				nRet = -1;
				goto END;
			}
			
			Sleep(SETUP_SLEEP);	
			
			{
				memset(vbuf, 0, buflen);	
				CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)vbuf;
				if (-1 != lChannel) 
				{
					pWSheet->iName = lChannel;
					memcpy(pWSheet->tsSchedule, pVideoLostAlm->stSect, sizeof(pWSheet->tsSchedule));
					nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (WSHEET_LOSS<<16)|(lChannel+1), (char*)vbuf, sizeof(CONFIG_WORKSHEET), 1000);
					if (nRet < 0)
					{
						nRet = -1;
						goto END;
					}
				}
				else
				{
					for (i = 0; i < iChanCount; i++, pWSheet++)
					{
						pWSheet->iName = i;
						memcpy(pWSheet->tsSchedule, pVideoLostAlm[i].stSect, sizeof(pWSheet->tsSchedule));
					}
					
					nRet = m_pManager->GetDevConfig().SetDevConfig_WorkSheet(lLoginID, WSHEET_LOSS, (CONFIG_WORKSHEET*)vbuf, waittime, iChanCount);
					if (nRet < 0)
					{
						nRet = -1;
						goto END;
					}
				}		
			}
			
			nRet = 0;	
		}
		else//老配置
		{
			if (oldbuflen < iChanNum*sizeof(CONFIG_DETECT))
			{
				oldbuflen = iChanNum*sizeof(CONFIG_DETECT);
			}
			
			voldbuf = new char[oldbuflen];
			if (NULL == voldbuf)
			{
				nRet = -1;
				goto END;
			}
			
			pTmpLossCfg = pLossCfg;
			iTmpIdx = iChanIdx;
			memset((void *)voldbuf, 0, oldbuflen);
			
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_DETECT, 0, voldbuf, oldbuflen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_DETECT) == 0) && (retlen/sizeof(CONFIG_DETECT) >= iAlarmNum))
			{
				int j = 0;
				CONFIG_DETECT * pDetCfg = (CONFIG_DETECT *)(voldbuf + iTmpIdx * sizeof(CONFIG_DETECT));
				for (i = 0; i < iChanCount; i++)
				{
					pDetCfg[i].LossAlarmMask = 0;
					for (j = 0; j < device->alarmoutputcount(device); j++)
					{
						if (pTmpLossCfg->struHandle.byRelAlarmOut[j])
						{
							pDetCfg[i].LossAlarmMask |= (1<<j);
						}
					}

					pDetCfg[i].LossTipEn = (pTmpLossCfg->struHandle.dwActionFlag&ALARM_TIP)?1:0;
					pDetCfg[i].Mailer = 0;
					if (pTmpLossCfg->struHandle.dwActionFlag&ALARM_MAIL)
					{
						pDetCfg[i].Mailer = 1;
					}
					
					pDetCfg[i].SendAlarmMsg = 0;
					if (pTmpLossCfg->struHandle.dwActionFlag&ALARM_UPLOAD)
					{
						pDetCfg[i].SendAlarmMsg = 1;
					}
					
					iTmpIdx ++;
					pTmpLossCfg++;
					pDetCfg++;
					
				}
				
				nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_DETECT, 0, voldbuf, oldbuflen, 2000);
				if (nRet < 0)
				{
					nRet = -1;
					goto END;	
				}
				
				nRet = 0;
			}
			else
			{
				nRet = -1;
				goto END;
			}	
		}
	}
END:
	if (vbuf) 
	{
		delete []vbuf;
		vbuf = NULL;
	}
	if (voldbuf) 
	{
		delete voldbuf;
		voldbuf = NULL;			
	}
	return nRet;
}

//blind alarm
int	CDevConfigEx::GetDevNewConfig_AllBlindALMCfg(LONG lLoginID, BLIND_CFG_EX *pstBlindAlm, LONG lChnNum, int waittime)
{
	afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device || NULL == pstBlindAlm)
	{
		return -1;
	}
	
	memset(pstBlindAlm, 0, lChnNum*sizeof(BLIND_CFG_EX));

	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
		
	int nRet = -1;
	int retlen = 0;
	
	int i = 0, x = 0;
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	char *dtBuf = NULL;
	
	if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF] )
	{
		int iProtocolVersion;
		device->get_info(device, dit_protocol_version, &iProtocolVersion);
		if (iProtocolVersion > 2)//新的报警配置
		{
			int nSheetNum = 16;
			
			if (iRecvBufLen < lChnNum * sizeof(CONFIG_BLINDDETECT))
			{
				iRecvBufLen = lChnNum * sizeof(CONFIG_BLINDDETECT);
			}
			if (iRecvBufLen < lChnNum * sizeof(CONFIG_WORKSHEET)) 
			{
				iRecvBufLen = lChnNum * sizeof(CONFIG_WORKSHEET);
			}

			cRecvBuf = new char [iRecvBufLen];
			if (NULL == cRecvBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(cRecvBuf, 0, iRecvBufLen);
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_BLIND, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_BLINDDETECT) == 0))
			{
				nSheetNum = device->channelcount(device);

				int nCount = retlen/sizeof(CONFIG_BLINDDETECT);
				if (nCount > nSheetNum) 
				{
					nCount = nSheetNum;
				}
				
				int j = 0;
				CONFIG_BLINDDETECT *pCfgBld = (CONFIG_BLINDDETECT *)cRecvBuf;
				for (i = 0; i < nCount; i++, pCfgBld++)
				{
					m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_BLIND, &pstBlindAlm[i].struHandle.dwActionMask);
					pstBlindAlm[i].byBlindEnable = pCfgBld->bEnable;
					pstBlindAlm[i].byBlindLevel = pCfgBld->iLevel;
					int index = min(32, device->channelcount(device));
					for (j = 0; j < index; j++)
					{
						pstBlindAlm[i].struHandle.byRecordChannel[j] = BITRHT(pCfgBld->hEvent.dwRecord,j)&1;
						pstBlindAlm[i].struHandle.byTour[j] = BITRHT(pCfgBld->hEvent.dwTour,j)&1;
						pstBlindAlm[i].struHandle.bySnap[j] = BITRHT(pCfgBld->hEvent.dwSnapShot,j)&1;
					}
					for(j = 0; j < 16; j++)
					{
						pstBlindAlm[i].struHandle.struPtzLink[j].iValue = pCfgBld->hEvent.PtzLink[j].iValue;
						pstBlindAlm[i].struHandle.struPtzLink[j].iType = pCfgBld->hEvent.PtzLink[j].iType;
					}
					index = min(32, device->alarmoutputcount(device));
					for (j = 0; j < index; j++)
					{
						pstBlindAlm[i].struHandle.byRelAlarmOut[j] = BITRHT(pCfgBld->hEvent.dwAlarmOut,j)&1;
						pstBlindAlm[i].struHandle.byRelWIAlarmOut[j] =BITRHT(pCfgBld->hEvent.wiAlarmOut,j)&1;
					}
					pstBlindAlm[i].struHandle.dwDuration = pCfgBld->hEvent.iAOLatch;
					pstBlindAlm[i].struHandle.dwRecLatch = pCfgBld->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
					pstBlindAlm[i].struHandle.dwEventLatch = pCfgBld->hEvent.iEventLatch;
					pstBlindAlm[i].struHandle.bMessageToNet = (BYTE)pCfgBld->hEvent.bMessagetoNet;
					pstBlindAlm[i].struHandle.bMMSEn = (BYTE)pCfgBld->hEvent.bMMSEn;
					pstBlindAlm[i].struHandle.bySnapshotTimes = pCfgBld->hEvent.SnapshotTimes;//短信发送图片的张数
					pstBlindAlm[i].struHandle.bLog = (BYTE)pCfgBld->hEvent.bLog;
					pstBlindAlm[i].struHandle.bMatrixEn = (BYTE)pCfgBld->hEvent.bMatrixEn;
					pstBlindAlm[i].struHandle.dwMatrix = pCfgBld->hEvent.dwMatrix;
					m_pManager->GetDevConfig().GetAlmActionFlag(pCfgBld->hEvent, &pstBlindAlm[i].struHandle.dwActionFlag);
				}		
			}
			else
			{
				nRet = -1;
				goto END;
			}

			memset(cRecvBuf, 0, iRecvBufLen);
			nSheetNum = nSheetNum>lChnNum?lChnNum:nSheetNum;
			nRet = m_pManager->GetDevConfig().GetDevConfig_WorkSheet(lLoginID, WSHEET_BLIND, (CONFIG_WORKSHEET*)cRecvBuf, waittime, nSheetNum);
			if (nRet >= 0)
			{
				CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)cRecvBuf;
				for (i = 0; i < nSheetNum; i++,pWSheet++)
				{
					memcpy(pstBlindAlm[i].stSect, pWSheet->tsSchedule, sizeof(pWSheet->tsSchedule));
				}
			}
			else
			{
				nRet = -1;
				goto END;
			}

			nRet = 0;
		}
		else //旧的报警
		{
			dtBuf = new char [lChnNum*sizeof(CONFIG_DETECT)];
			if (NULL == dtBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(dtBuf, 0, lChnNum*sizeof(CONFIG_DETECT));
		
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_DETECT, 0, dtBuf, lChnNum*sizeof(CONFIG_DETECT), &retlen, waittime);
			if ((nRet >=0) && (retlen%sizeof(CONFIG_DETECT) == 0) && (retlen <= lChnNum * sizeof(CONFIG_DETECT)))
			{
				CONFIG_DETECT *stDet = 0;
				int nCount = retlen/sizeof(CONFIG_DETECT);
				for (i = 0; i < nCount; i++)
				{
					pstBlindAlm[i].struHandle.dwActionMask = 0;
					pstBlindAlm[i].struHandle.dwActionMask |= ALARM_OUT;
					pstBlindAlm[i].struHandle.dwActionMask |= ALARM_TIP;
					pstBlindAlm[i].struHandle.dwActionMask |= ALARM_MAIL;
				
					stDet = (CONFIG_DETECT*)(dtBuf+i*sizeof(CONFIG_DETECT));
					pstBlindAlm[i].byBlindEnable = 1; //default enabled
		
					for (x = 0; x < MAX_ALARM_OUT_NUM; x++)
					{
						pstBlindAlm[i].struHandle.byRelAlarmOut[x] = (stDet->BlindAlarmMask & (0x01<<x)) ? 1 : 0;
					}
					
					if (stDet->BlindTipEn) 
					{
						pstBlindAlm[i].struHandle.dwActionFlag |= ALARM_TIP;
					}
					//default output eanbled
					pstBlindAlm[i].struHandle.dwActionFlag |= ALARM_OUT;
				}
			}
			else
			{
				nRet = -1;
				goto END;;
			}
				
			nRet = 0;
		}
	}
		
END:
	if (cRecvBuf) 
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	if (dtBuf) 
	{
		delete []dtBuf;
		dtBuf = NULL;
	}

	return nRet;
}


int	CDevConfigEx::GetDevNewConfig_BlindALMCfg(LONG lLoginID, BLIND_CFG_EX &stBlindAlm, LONG lChannel, int waittime)
{
	afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device)
	{
		return -1;
	}

	memset(&stBlindAlm, 0, sizeof(BLIND_CFG_EX));

	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
		
	int nRet = -1;
	int retlen = 0;
	
	int i = 0, x = 0;
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	char *dtBuf = NULL;

	int lChnNum = device->channelcount(device)>16?device->channelcount(device):16;	
 		
	if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF] )
	{
		int iProtocolVersion;
		device->get_info(device, dit_protocol_version, &iProtocolVersion);
		if (iProtocolVersion > 2)//新的报警配置
		{
			if (iRecvBufLen < lChnNum * sizeof(CONFIG_BLINDDETECT))
			{
				iRecvBufLen = lChnNum * sizeof(CONFIG_BLINDDETECT);
			}

			cRecvBuf = new char [iRecvBufLen];
			if (NULL == cRecvBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(cRecvBuf, 0, iRecvBufLen);
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_BLIND, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_BLINDDETECT) == 0) && (retlen/sizeof(CONFIG_BLINDDETECT) > lChannel))
			{
				int j = 0;
				CONFIG_BLINDDETECT *pCfgBld = (CONFIG_BLINDDETECT *)(cRecvBuf + lChannel*sizeof(CONFIG_BLINDDETECT));
			
				m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_BLIND, &stBlindAlm.struHandle.dwActionMask);
				stBlindAlm.byBlindEnable = pCfgBld->bEnable;
				stBlindAlm.byBlindLevel = pCfgBld->iLevel;
				int index = min(32, device->channelcount(device));
				for (j = 0; j < index; j++)
				{
					stBlindAlm.struHandle.byRecordChannel[j] = BITRHT(pCfgBld->hEvent.dwRecord,j)&1;
					stBlindAlm.struHandle.byTour[j] = BITRHT(pCfgBld->hEvent.dwTour,j)&1;
					stBlindAlm.struHandle.bySnap[j] = BITRHT(pCfgBld->hEvent.dwSnapShot,j)&1;
				}
				for(j = 0; j < 16; j++)
				{
					stBlindAlm.struHandle.struPtzLink[j].iValue = pCfgBld->hEvent.PtzLink[j].iValue;
					stBlindAlm.struHandle.struPtzLink[j].iType = pCfgBld->hEvent.PtzLink[j].iType;
				}
				index = min(32, device->alarmoutputcount(device));
				for (j = 0; j < index; j++)
				{
					stBlindAlm.struHandle.byRelAlarmOut[j] = BITRHT(pCfgBld->hEvent.dwAlarmOut,j)&1;
					stBlindAlm.struHandle.byRelWIAlarmOut[j] =BITRHT(pCfgBld->hEvent.wiAlarmOut,j)&1;
				}
				stBlindAlm.struHandle.dwDuration = pCfgBld->hEvent.iAOLatch;
				stBlindAlm.struHandle.dwRecLatch = pCfgBld->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
				stBlindAlm.struHandle.dwEventLatch = pCfgBld->hEvent.iEventLatch;
				stBlindAlm.struHandle.bMessageToNet = (BYTE)pCfgBld->hEvent.bMessagetoNet;
				stBlindAlm.struHandle.bMMSEn = (BYTE)pCfgBld->hEvent.bMMSEn;
				stBlindAlm.struHandle.bySnapshotTimes = pCfgBld->hEvent.SnapshotTimes;//短信发送图片的张数
				stBlindAlm.struHandle.bLog = (BYTE)pCfgBld->hEvent.bLog;
				stBlindAlm.struHandle.bMatrixEn = (BYTE)pCfgBld->hEvent.bMatrixEn;
				stBlindAlm.struHandle.dwMatrix = pCfgBld->hEvent.dwMatrix;
				m_pManager->GetDevConfig().GetAlmActionFlag(pCfgBld->hEvent, &stBlindAlm.struHandle.dwActionFlag);				
			}
			else
			{
				nRet = -1;
				goto END;
			}

			CONFIG_WORKSHEET stWSheet = {0};
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (WSHEET_BLIND<<16)|(lChannel+1), (char*)&stWSheet, sizeof(CONFIG_WORKSHEET), &retlen, waittime);
			if (nRet < 0 || retlen != sizeof(CONFIG_WORKSHEET))
			{
				nRet = -1;
				goto END;
			}
			
			memcpy(stBlindAlm.stSect, &stWSheet.tsSchedule, sizeof(stWSheet.tsSchedule));
			
			nRet = 0;
		}
		else //旧的报警
		{
			dtBuf = new char [lChnNum*sizeof(CONFIG_DETECT)];
			if (NULL == dtBuf)
			{
				nRet = -1;
				goto END;
			}
			memset(dtBuf, 0, lChnNum*sizeof(CONFIG_DETECT));
		
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_DETECT, 0, dtBuf, lChnNum*sizeof(CONFIG_DETECT), &retlen, waittime);
			if ((nRet >=0) && (retlen%sizeof(CONFIG_ALARM) == 0)&& (retlen/sizeof(CONFIG_ALARM) > lChannel))
			{
				CONFIG_DETECT *stDet = (CONFIG_DETECT *)(dtBuf + lChannel*sizeof(CONFIG_DETECT));
				stBlindAlm.struHandle.dwActionMask = 0;
				stBlindAlm.struHandle.dwActionMask |= ALARM_OUT;
				stBlindAlm.struHandle.dwActionMask |= ALARM_TIP;
				stBlindAlm.struHandle.dwActionMask |= ALARM_MAIL;
				
				stBlindAlm.byBlindEnable = 1; //default enabled
				
				for (x = 0; x < MAX_ALARM_OUT_NUM; x++)
				{
					stBlindAlm.struHandle.byRelAlarmOut[x] = (stDet->BlindAlarmMask & (0x01<<x)) ? 1 : 0;
				}
				
				if (stDet->BlindTipEn) 
				{
					stBlindAlm.struHandle.dwActionFlag |= ALARM_TIP;
				}
				//default output eanbled
				stBlindAlm.struHandle.dwActionFlag |= ALARM_OUT;
			}
			else
			{
				nRet = -1;
				goto END;;
			}
				
			nRet = 0;
		}
	}
		
END:
	if (cRecvBuf) 
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	if (dtBuf) 
	{
		delete []dtBuf;
		dtBuf = NULL;
	}

	return nRet;
}
int	CDevConfigEx::SetDevNewConfig_BlindALMCfg(LONG lLoginID, BLIND_CFG_EX* pBlindAlm, LONG lChannel, int waittime)
{
	afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device)
	{
		return -1;
	}
	
	if (NULL == pBlindAlm)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int retlen = 0; //data length
	int nRet = -1;  //return value
	
	int iAlarmNum = device->channelcount(device);
	int iChanNum = iAlarmNum>16?iAlarmNum:16;
	
	int iChanCount = 1;
	int iChanIdx = lChannel;
	
	if (-1 == lChannel)
	{
		iChanIdx = 0;
		iChanCount = iAlarmNum;
	}
	
	BLIND_CFG_EX *const pBlindCfg = pBlindAlm;
	BLIND_CFG_EX *pTmpBlindCfg = pBlindCfg;
	int i = 0,j =0;
	int iTmpIdx = iChanIdx;
	int buflen = 0;
	char *vbuf = NULL;
	int oldbuflen = 0;
	char *voldbuf = NULL;
	
	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
	
	if ((iAlarmNum != 0) && (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF]) )
	{
		int iProtocolVersion;
		device->get_info(device, dit_protocol_version, &iProtocolVersion);
		if (iProtocolVersion > 2)//新的报警配置
		{
			if (buflen < iChanNum * sizeof(CONFIG_BLINDDETECT))
			{
				buflen = iChanNum * sizeof(CONFIG_BLINDDETECT);
			}
			if (buflen < iChanNum * sizeof(CONFIG_WORKSHEET)) 
			{
				buflen = iChanNum * sizeof(CONFIG_WORKSHEET);
			}
			vbuf = new char[buflen];
			if (NULL == vbuf)
			{
				nRet = -1;
				goto END;
			}
			
			pTmpBlindCfg = pBlindCfg;
			iTmpIdx = iChanIdx;
			memset(vbuf, 0, buflen);
			
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_BLIND, 0, vbuf, buflen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_BLINDDETECT) == 0) && (retlen/sizeof(CONFIG_BLINDDETECT) >= iAlarmNum))
			{
				CONFIG_BLINDDETECT *pCfgBld = (CONFIG_BLINDDETECT *)(vbuf+ iTmpIdx*sizeof(CONFIG_BLINDDETECT));
				int nCount = retlen/sizeof(CONFIG_BLINDDETECT);
				for (i = 0; i < iChanCount; i++)
				{
					m_pManager->GetDevConfig().SetAlmActionFlag(&pCfgBld->hEvent, pTmpBlindCfg->struHandle.dwActionFlag);
					pCfgBld->bEnable = pTmpBlindCfg->byBlindEnable;
					pCfgBld->iLevel = pTmpBlindCfg->byBlindLevel;
					
					pCfgBld->hEvent.dwRecord = 0;
					pCfgBld->hEvent.dwTour = 0;
					pCfgBld->hEvent.dwSnapShot = 0;
					
					int index = min(32, device->channelcount(device));
					for (j = 0; j < index; j++)
					{
						pCfgBld->hEvent.dwSnapShot |= (pTmpBlindCfg->struHandle.bySnap[j]) ? (0x01<<j) : 0;
						pCfgBld->hEvent.dwTour |= (pTmpBlindCfg->struHandle.byTour[j]) ? (0x01<<j) : 0;
						pCfgBld->hEvent.dwRecord |= (pTmpBlindCfg->struHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
					}

					for(j = 0; j < 16; j++)
					{
						pCfgBld->hEvent.PtzLink[j].iValue = pTmpBlindCfg->struHandle.struPtzLink[j].iValue;
						pCfgBld->hEvent.PtzLink[j].iType = pTmpBlindCfg->struHandle.struPtzLink[j].iType;
					}
					
					pCfgBld->hEvent.dwAlarmOut = 0;
					pCfgBld->hEvent.wiAlarmOut = 0;
					index = min(32, device->alarmoutputcount(device));
					for (j = 0; j < index; j++)
					{
						pCfgBld->hEvent.dwAlarmOut |= (pTmpBlindCfg->struHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
						pCfgBld->hEvent.wiAlarmOut |= (pTmpBlindCfg->struHandle.byRelWIAlarmOut[j]) ? (0x01<<j) : 0;
					}
					pCfgBld->hEvent.iAOLatch = pTmpBlindCfg->struHandle.dwDuration;
					pCfgBld->hEvent.iRecordLatch = pTmpBlindCfg->struHandle.dwRecLatch;
					pCfgBld->hEvent.iEventLatch = pTmpBlindCfg->struHandle.dwEventLatch;
					pCfgBld->hEvent.bMessagetoNet = (BOOL)pTmpBlindCfg->struHandle.bMessageToNet;
					pCfgBld->hEvent.bMMSEn = pTmpBlindCfg->struHandle.bMMSEn;
					pCfgBld->hEvent.SnapshotTimes = pTmpBlindCfg->struHandle.bySnapshotTimes;//短信发送图片的张数
					pCfgBld->hEvent.bLog = (BOOL)pTmpBlindCfg->struHandle.bLog;
					pCfgBld->hEvent.bMatrixEn = (BOOL)pTmpBlindCfg->struHandle.bMatrixEn;
					pCfgBld->hEvent.dwMatrix = pTmpBlindCfg->struHandle.dwMatrix;

					pTmpBlindCfg++;
					pCfgBld++;
					iTmpIdx++;
				}
				
				nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_ALARM_BLIND, 0, vbuf, nCount*sizeof(CONFIG_BLINDDETECT), 2000);
				if (nRet < 0)
				{
					nRet = -1;
					goto END;
				}
			}
			else
			{
				nRet = -1;
				goto END;
			}
			
			Sleep(SETUP_SLEEP);	
			
			{
				memset(vbuf, 0, buflen);	
				CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)vbuf;
				if (-1 != lChannel) 
				{
					pWSheet->iName = lChannel;
					memcpy(pWSheet->tsSchedule, pBlindAlm->stSect, sizeof(pWSheet->tsSchedule));
					nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (WSHEET_LOSS<<16)|(lChannel+1), (char*)vbuf, sizeof(CONFIG_WORKSHEET), 1000);
					if (nRet < 0)
					{
						nRet = -1;
						goto END;
					}
				}
				else
				{
					for (i = 0; i < iChanCount; i++, pWSheet++)
					{
						pWSheet->iName = i;
						memcpy(pWSheet->tsSchedule, pBlindAlm[i].stSect, sizeof(pWSheet->tsSchedule));
					}
					
					nRet = m_pManager->GetDevConfig().SetDevConfig_WorkSheet(lLoginID, WSHEET_LOSS, (CONFIG_WORKSHEET*)vbuf, waittime, iChanCount);
					if (nRet < 0)
					{
						nRet = -1;
						goto END;
					}
				}		
			}
			
			nRet = 0;	
		}
		else//老配置
		{
			if (oldbuflen < iChanNum*sizeof(CONFIG_DETECT))
			{
				oldbuflen = iChanNum*sizeof(CONFIG_DETECT);
			}
			
			voldbuf = new char[oldbuflen];
			if (NULL == voldbuf)
			{
				nRet = -1;
				goto END;
			}
			
			pTmpBlindCfg = pBlindCfg;
			iTmpIdx = iChanIdx;
			memset((void *)voldbuf, 0, oldbuflen);
			
			nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_DETECT, 0, voldbuf, oldbuflen, &retlen, waittime);
			if ((nRet >= 0) && (retlen%sizeof(CONFIG_DETECT) == 0) && (retlen >= iAlarmNum * sizeof(CONFIG_DETECT)))
			{
				int j = 0;
				CONFIG_DETECT * pDetCfg = (CONFIG_DETECT *)(voldbuf + iTmpIdx * sizeof(CONFIG_DETECT));
				for (i = 0; i < iChanCount; i++)
				{
					pDetCfg[i].BlindAlarmMask = 0;
					for (j = 0; j < MAX_ALARM_OUT_NUM; j++)
					{
						if (pTmpBlindCfg->struHandle.byRelAlarmOut[j])
						{
							pDetCfg[i].BlindAlarmMask |= (1<<j);
						}
					}
					
					pDetCfg[i].BlindTipEn = (pTmpBlindCfg->struHandle.dwActionFlag&ALARM_TIP)?1:0;
				
					pDetCfg[i].Mailer = 0;
					if (pTmpBlindCfg->struHandle.dwActionFlag&ALARM_MAIL)
					{
						pDetCfg[i].Mailer = 1;
					}
					pDetCfg[i].SendAlarmMsg = 0;
					if (pTmpBlindCfg->struHandle.dwActionFlag&ALARM_UPLOAD)
					{
						pDetCfg[i].SendAlarmMsg = 1;
					}
					
					iTmpIdx ++;
					pTmpBlindCfg++;
					pDetCfg++;
					
				}
				
				nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_DETECT, 0, voldbuf, oldbuflen, 2000);
				if (nRet < 0)
				{
					nRet = -1;
					goto END;	
				}
				
				nRet = 0;
			}
			else
			{
				nRet = -1;
				goto END;
			}	
		}
	}
END:
	if (vbuf) 
	{
		delete []vbuf;
		vbuf = NULL;
	}
	if (voldbuf) 
	{
		delete voldbuf;
		voldbuf = NULL;			
	}
	return nRet;
}

//disk alarm
int	CDevConfigEx::GetDevNewConfig_DiskALMCfg(LONG lLoginID, DISK_ALARMIN_CFG_EX *pDiskAlmCfg, int waittime)
{
	afk_device_s * device = (afk_device_s *)lLoginID;
	if (NULL == device || NULL == pDiskAlmCfg)
	{
		return -1;
	}
	
	int retlen = 0;
	int nRet = -1;

	int i = 0, x = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < sizeof(CONFIG_DISKNOSPACE))
	{
		iRecvBufLen = sizeof(CONFIG_DISKNOSPACE);
	}
	if (iRecvBufLen < sizeof(CONFIG_GENERIC_EVENT))
	{
		iRecvBufLen = sizeof(CONFIG_GENERIC_EVENT);
	}
	if(iRecvBufLen < sizeof(CONFIG_STORAGE_NUMBER_EVENT))
	{
		iRecvBufLen = sizeof(CONFIG_STORAGE_NUMBER_EVENT);
	}

	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_DISKFULL, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_DISKNOSPACE))
	{
		int j = 0;
		CONFIG_DISKNOSPACE *pCfgDiskNoSpace = (CONFIG_DISKNOSPACE *)cRecvBuf;
		m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_DISKFULL, &pDiskAlmCfg->struLCHandle.dwActionMask);
		pDiskAlmCfg->byLowCapEn = pCfgDiskNoSpace->bEnable;
		pDiskAlmCfg->byLowerLimit = pCfgDiskNoSpace->iLowerLimit;
		int index = min(32, device->channelcount(device));
		for (j = 0; j < index; j++)
		{
			
			pDiskAlmCfg->struLCHandle.byRecordChannel[j] = BITRHT(pCfgDiskNoSpace->hEvent.dwRecord,j)&1;
			pDiskAlmCfg->struLCHandle.byTour[j] = BITRHT(pCfgDiskNoSpace->hEvent.dwTour,j)&1;
			pDiskAlmCfg->struLCHandle.bySnap[j] = BITRHT(pCfgDiskNoSpace->hEvent.dwSnapShot,j)&1;
		}
		for( j = 0; j < 16; j++)
		{
			pDiskAlmCfg->struLCHandle.struPtzLink[j].iValue = pCfgDiskNoSpace->hEvent.PtzLink[j].iValue;
			pDiskAlmCfg->struLCHandle.struPtzLink[j].iType = pCfgDiskNoSpace->hEvent.PtzLink[j].iType;

		}

		index = min(32, device->alarmoutputcount(device));
		for (j = 0; j < index; j++)
		{
			pDiskAlmCfg->struLCHandle.byRelAlarmOut[j] = BITRHT(pCfgDiskNoSpace->hEvent.dwAlarmOut,j)&1;
			pDiskAlmCfg->struLCHandle.byRelWIAlarmOut[j] =BITRHT(pCfgDiskNoSpace->hEvent.wiAlarmOut,j)&1;
		}
		pDiskAlmCfg->struLCHandle.dwDuration = pCfgDiskNoSpace->hEvent.iAOLatch;
		pDiskAlmCfg->struLCHandle.dwRecLatch = pCfgDiskNoSpace->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
		pDiskAlmCfg->struLCHandle.dwEventLatch = pCfgDiskNoSpace->hEvent.iEventLatch;
		pDiskAlmCfg->struLCHandle.bMessageToNet = (BYTE)pCfgDiskNoSpace->hEvent.bMessagetoNet;
		
		pDiskAlmCfg->struLCHandle.bMMSEn = (BYTE)pCfgDiskNoSpace->hEvent.bMMSEn;
		pDiskAlmCfg->struLCHandle.bySnapshotTimes = pCfgDiskNoSpace->hEvent.SnapshotTimes;//短信发送图片的张数
		pDiskAlmCfg->struLCHandle.bLog = (BYTE)pCfgDiskNoSpace->hEvent.bLog;
		pDiskAlmCfg->struLCHandle.bMatrixEn = (BYTE)pCfgDiskNoSpace->hEvent.bMatrixEn;
		pDiskAlmCfg->struLCHandle.dwMatrix = pCfgDiskNoSpace->hEvent.dwMatrix;
		m_pManager->GetDevConfig().GetAlmActionFlag(pCfgDiskNoSpace->hEvent, &pDiskAlmCfg->struLCHandle.dwActionFlag);
	}
	else
	{
		nRet = -1;
		goto END;
	}

	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_NODISK, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_GENERIC_EVENT))
	{
		int j = 0;
		CONFIG_GENERIC_EVENT *pCfgNoDisk = (CONFIG_GENERIC_EVENT *)cRecvBuf;
		m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_NODISK, &pDiskAlmCfg->struHdvrHandle.dwActionMask);
		pDiskAlmCfg->byNoDiskEn = pCfgNoDisk->bEnable;
		int index = min(32, device->channelcount(device));
		for (j = 0; j < index; j++)
		{
			pDiskAlmCfg->struHdvrHandle.byRecordChannel[j] = BITRHT(pCfgNoDisk->hEvent.dwRecord,j)&1;
			pDiskAlmCfg->struHdvrHandle.byTour[j] = BITRHT(pCfgNoDisk->hEvent.dwTour,j)&1;
			pDiskAlmCfg->struHdvrHandle.bySnap[j] = BITRHT(pCfgNoDisk->hEvent.dwSnapShot,j)&1;
		}
		for(j = 0; j < 16; j++)
		{
			pDiskAlmCfg->struHdvrHandle.struPtzLink[j].iValue = pCfgNoDisk->hEvent.PtzLink[j].iValue;
			pDiskAlmCfg->struHdvrHandle.struPtzLink[j].iType = pCfgNoDisk->hEvent.PtzLink[j].iType;
		}
		index = min(32, device->alarmoutputcount(device));
		for (j = 0; j < index; j++)
		{
			pDiskAlmCfg->struHdvrHandle.byRelAlarmOut[j] = BITRHT(pCfgNoDisk->hEvent.dwAlarmOut,j)&1;
			pDiskAlmCfg->struHdvrHandle.byRelWIAlarmOut[j] =BITRHT(pCfgNoDisk->hEvent.dwAlarmOut,j)&1;
		}
		pDiskAlmCfg->struHdvrHandle.dwDuration = pCfgNoDisk->hEvent.iAOLatch;
		pDiskAlmCfg->struHdvrHandle.dwRecLatch = pCfgNoDisk->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
		pDiskAlmCfg->struHdvrHandle.dwEventLatch = pCfgNoDisk->hEvent.iEventLatch;
		pDiskAlmCfg->struHdvrHandle.bMessageToNet = (BYTE)pCfgNoDisk->hEvent.bMessagetoNet;
		pDiskAlmCfg->struHdvrHandle.bMMSEn = (BYTE)pCfgNoDisk->hEvent.bMMSEn;
		pDiskAlmCfg->struHdvrHandle.bySnapshotTimes = pCfgNoDisk->hEvent.SnapshotTimes;//短信发送图片的张数
		pDiskAlmCfg->struHdvrHandle.bLog = (BYTE)pCfgNoDisk->hEvent.bLog;
		pDiskAlmCfg->struHdvrHandle.bMatrixEn = (BYTE)pCfgNoDisk->hEvent.bMatrixEn;
		pDiskAlmCfg->struHdvrHandle.dwMatrix = pCfgNoDisk->hEvent.dwMatrix;
		m_pManager->GetDevConfig().GetAlmActionFlag(pCfgNoDisk->hEvent, &pDiskAlmCfg->struHdvrHandle.dwActionFlag);
	}
	else
	{
		nRet = -1;
		goto END;
	}
	
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_DISKERR, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_GENERIC_EVENT))
	{
		int j = 0;
		CONFIG_GENERIC_EVENT *pCfgDiskErr = (CONFIG_GENERIC_EVENT *)cRecvBuf;
		m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_DISKERR, &pDiskAlmCfg->struMsgHandle.dwActionMask);
		pDiskAlmCfg->byDiskErrEn = pCfgDiskErr->bEnable;
		int index = min(32, device->channelcount(device));
		for (j = 0; j < index; j++)
		{
			
			pDiskAlmCfg->struMsgHandle.byRecordChannel[j] = BITRHT(pCfgDiskErr->hEvent.dwRecord,j)&1;
			pDiskAlmCfg->struMsgHandle.byTour[j] = BITRHT(pCfgDiskErr->hEvent.dwTour,j)&1;
			pDiskAlmCfg->struMsgHandle.bySnap[j] = BITRHT(pCfgDiskErr->hEvent.dwSnapShot,j)&1;
		}
		for (j = 0; j < 16; j++)
		{
			pDiskAlmCfg->struMsgHandle.struPtzLink[j].iValue = pCfgDiskErr->hEvent.PtzLink[j].iValue;
			pDiskAlmCfg->struMsgHandle.struPtzLink[j].iType = pCfgDiskErr->hEvent.PtzLink[j].iType;
		}
		index = min(32, device->alarmoutputcount(device));
		for (j = 0; j < index; j++)
		{
			pDiskAlmCfg->struMsgHandle.byRelAlarmOut[j] = BITRHT(pCfgDiskErr->hEvent.dwAlarmOut,j)&1;
			pDiskAlmCfg->struMsgHandle.byRelWIAlarmOut[j] =BITRHT(pCfgDiskErr->hEvent.wiAlarmOut,j)&1;
		}
		pDiskAlmCfg->struMsgHandle.dwDuration = pCfgDiskErr->hEvent.iAOLatch;
		pDiskAlmCfg->struMsgHandle.dwRecLatch = pCfgDiskErr->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
		pDiskAlmCfg->struMsgHandle.dwEventLatch = pCfgDiskErr->hEvent.iEventLatch;
		pDiskAlmCfg->struMsgHandle.bMessageToNet = (BYTE)pCfgDiskErr->hEvent.bMessagetoNet;
		pDiskAlmCfg->struMsgHandle.bMMSEn = (BYTE)pCfgDiskErr->hEvent.bMMSEn;
		pDiskAlmCfg->struMsgHandle.bySnapshotTimes = pCfgDiskErr->hEvent.SnapshotTimes;//短信发送图片的张数
		pDiskAlmCfg->struMsgHandle.bLog = (BYTE)pCfgDiskErr->hEvent.bLog;
		pDiskAlmCfg->struMsgHandle.bMatrixEn = (BYTE)pCfgDiskErr->hEvent.bMatrixEn;
		pDiskAlmCfg->struMsgHandle.dwMatrix = pCfgDiskErr->hEvent.dwMatrix;
		m_pManager->GetDevConfig().GetAlmActionFlag(pCfgDiskErr->hEvent, &pDiskAlmCfg->struMsgHandle.dwActionFlag);

		memset(cRecvBuf, 0, iRecvBufLen);
		nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_DISKNUM, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
		if(nRet >=0 && retlen == sizeof(CONFIG_STORAGE_NUMBER_EVENT))
		{
			CONFIG_STORAGE_NUMBER_EVENT *pCfgDiskNum = (CONFIG_STORAGE_NUMBER_EVENT *)cRecvBuf;
			pDiskAlmCfg->bDiskNum = pCfgDiskNum->iHddNumber;
		}
	}
	
	else
	{
		nRet = -1;
		goto END;
	}
	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
}


int	CDevConfigEx::SetDevNewConfig_DiskALMCfg(LONG lLoginID, DISK_ALARMIN_CFG_EX* pDiskAlmCfg, int waittime)
{
	afk_device_s * device = (afk_device_s *)lLoginID;
	if (NULL == device || NULL == pDiskAlmCfg)
	{
		return -1;
	}

	int retlen = 0;
	int nRet = -1;

	int i = 0, j = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < sizeof(CONFIG_DISKNOSPACE))
	{
		iRecvBufLen = sizeof(CONFIG_DISKNOSPACE);
	}
	if (iRecvBufLen < sizeof(CONFIG_GENERIC_EVENT))
	{
		iRecvBufLen = sizeof(CONFIG_GENERIC_EVENT);
	}
	if (iRecvBufLen < sizeof(CONFIG_STORAGE_NUMBER_EVENT))
	{
		iRecvBufLen = sizeof(CONFIG_STORAGE_NUMBER_EVENT);
	}
	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
//	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_DISKFULL, 0, cRecvBuf, iRecvBufLen, &retlen, g_query_wait);
//	if (nRet >= 0 && retlen == sizeof(CONFIG_DISKNOSPACE))
	{
		CONFIG_DISKNOSPACE *pCfgDiskNoSpace = (CONFIG_DISKNOSPACE *)cRecvBuf;
		m_pManager->GetDevConfig().SetAlmActionFlag(&pCfgDiskNoSpace->hEvent, pDiskAlmCfg->struLCHandle.dwActionFlag);
		pCfgDiskNoSpace->bEnable = pDiskAlmCfg->byLowCapEn;
		pCfgDiskNoSpace->iLowerLimit = pDiskAlmCfg->byLowerLimit;
		
		pCfgDiskNoSpace->hEvent.dwRecord = 0;
		pCfgDiskNoSpace->hEvent.dwTour = 0;
		pCfgDiskNoSpace->hEvent.dwSnapShot = 0;

		int index = min(32, device->channelcount(device));
		for (j = 0; j < index; j++)
		{
			pCfgDiskNoSpace->hEvent.dwSnapShot |= (pDiskAlmCfg->struLCHandle.bySnap[j]) ? (0x01<<j) : 0;
			pCfgDiskNoSpace->hEvent.dwTour |= (pDiskAlmCfg->struLCHandle.byTour[j]) ? (0x01<<j) : 0;
			pCfgDiskNoSpace->hEvent.dwRecord |= (pDiskAlmCfg->struLCHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
		}

		for(j = 0; j < 16; j++)
		{
			pCfgDiskNoSpace->hEvent.PtzLink[j].iValue = pDiskAlmCfg->struLCHandle.struPtzLink[j].iValue;
			pCfgDiskNoSpace->hEvent.PtzLink[j].iType = pDiskAlmCfg->struLCHandle.struPtzLink[j].iType;
		}

		pCfgDiskNoSpace->hEvent.dwAlarmOut = 0;
		pCfgDiskNoSpace->hEvent.wiAlarmOut = 0;
		index = min(32, device->alarmoutputcount(device));
		for (j = 0; j < index; j++)
		{
			pCfgDiskNoSpace->hEvent.dwAlarmOut |= (pDiskAlmCfg->struLCHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
			pCfgDiskNoSpace->hEvent.wiAlarmOut |= (pDiskAlmCfg->struLCHandle.byRelWIAlarmOut[j]) ? (0x01<<j) : 0;
		}
		pCfgDiskNoSpace->hEvent.iAOLatch = pDiskAlmCfg->struLCHandle.dwDuration;
		pCfgDiskNoSpace->hEvent.iRecordLatch = pDiskAlmCfg->struLCHandle.dwRecLatch;
		pCfgDiskNoSpace->hEvent.iEventLatch = pDiskAlmCfg->struLCHandle.dwEventLatch;
		pCfgDiskNoSpace->hEvent.bMessagetoNet = (BOOL)pDiskAlmCfg->struLCHandle.bMessageToNet;
		pCfgDiskNoSpace->hEvent.bMMSEn = pDiskAlmCfg->struLCHandle.bMMSEn;
		pCfgDiskNoSpace->hEvent.SnapshotTimes = pDiskAlmCfg->struLCHandle.bySnapshotTimes;//短信发送图片的张数
		pCfgDiskNoSpace->hEvent.bLog = (BOOL)pDiskAlmCfg->struLCHandle.bLog;
		pCfgDiskNoSpace->hEvent.bMatrixEn = (BOOL)pDiskAlmCfg->struLCHandle.bMatrixEn;
		pCfgDiskNoSpace->hEvent.dwMatrix = pDiskAlmCfg->struLCHandle.dwMatrix;
		nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_ALARM_DISKFULL, 0, cRecvBuf, sizeof(CONFIG_DISKNOSPACE), 2000);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}
//	else
//	{
//		nRet = -1;
//		goto END;
//	}

	Sleep(SETUP_SLEEP);

	memset(cRecvBuf, 0, iRecvBufLen);
//	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_NODISK, 0, cRecvBuf, iRecvBufLen, &retlen, g_query_wait);
//	if (nRet >= 0 && retlen == sizeof(CONFIG_GENERIC_EVENT))
	{
		CONFIG_GENERIC_EVENT *pCfgNoDisk = (CONFIG_GENERIC_EVENT *)cRecvBuf;

		m_pManager->GetDevConfig().SetAlmActionFlag(&pCfgNoDisk->hEvent, pDiskAlmCfg->struHdvrHandle.dwActionFlag);
		pCfgNoDisk->bEnable = pDiskAlmCfg->byNoDiskEn;
		
		pCfgNoDisk->hEvent.dwRecord = 0;
		pCfgNoDisk->hEvent.dwTour = 0;
		pCfgNoDisk->hEvent.dwSnapShot = 0;

		int index = min(32, device->channelcount(device));
		for (j = 0; j < index; j++)
		{
			pCfgNoDisk->hEvent.dwSnapShot |= (pDiskAlmCfg->struHdvrHandle.bySnap[j]) ? (0x01<<j) : 0;
			pCfgNoDisk->hEvent.dwTour |= (pDiskAlmCfg->struHdvrHandle.byTour[j]) ? (0x01<<j) : 0;
			pCfgNoDisk->hEvent.dwRecord |= (pDiskAlmCfg->struHdvrHandle.byRecordChannel[j]) ? (0x01<<j) : 0;	
		}
		for(j = 0; j < 16; j++)
		{
			pCfgNoDisk->hEvent.PtzLink[j].iValue = pDiskAlmCfg->struHdvrHandle.struPtzLink[j].iValue;
			pCfgNoDisk->hEvent.PtzLink[j].iType = pDiskAlmCfg->struHdvrHandle.struPtzLink[j].iType;
		}

		pCfgNoDisk->hEvent.dwAlarmOut = 0;
		pCfgNoDisk->hEvent.wiAlarmOut = 0;
		index = min(32, device->alarmoutputcount(device));
		for (j = 0; j < index; j++)
		{
			pCfgNoDisk->hEvent.dwAlarmOut |= (pDiskAlmCfg->struHdvrHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
			pCfgNoDisk->hEvent.wiAlarmOut |= (pDiskAlmCfg->struHdvrHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
		}
		pCfgNoDisk->hEvent.iAOLatch = pDiskAlmCfg->struHdvrHandle.dwDuration;
		pCfgNoDisk->hEvent.iRecordLatch = pDiskAlmCfg->struHdvrHandle.dwRecLatch;
		pCfgNoDisk->hEvent.iEventLatch = pDiskAlmCfg->struHdvrHandle.dwEventLatch;
		pCfgNoDisk->hEvent.bMessagetoNet = (BOOL)pDiskAlmCfg->struHdvrHandle.bMessageToNet;
		pCfgNoDisk->hEvent.bMMSEn = pDiskAlmCfg->struHdvrHandle.bMMSEn;
		pCfgNoDisk->hEvent.SnapshotTimes = pDiskAlmCfg->struHdvrHandle.bySnapshotTimes;//短信发送图片的张数
		pCfgNoDisk->hEvent.bLog = (BOOL)pDiskAlmCfg->struHdvrHandle.bLog;
		pCfgNoDisk->hEvent.bMatrixEn = (BOOL)pDiskAlmCfg->struHdvrHandle.bMatrixEn;
		pCfgNoDisk->hEvent.dwMatrix = pDiskAlmCfg->struHdvrHandle.dwMatrix;
		
		nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_ALARM_NODISK, 0, cRecvBuf, sizeof(CONFIG_GENERIC_EVENT), 2000);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}
//	else
//	{
//		nRet = -1;
//		goto END;
//	}

	Sleep(SETUP_SLEEP);

	memset(cRecvBuf, 0, iRecvBufLen);
//	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_DISKERR, 0, cRecvBuf, iRecvBufLen, &retlen, g_query_wait);
//	if (nRet >= 0 && retlen == sizeof(CONFIG_GENERIC_EVENT))
	{
		CONFIG_GENERIC_EVENT *pCfgDiskErr = (CONFIG_GENERIC_EVENT *)cRecvBuf;

		m_pManager->GetDevConfig().SetAlmActionFlag(&pCfgDiskErr->hEvent, pDiskAlmCfg->struMsgHandle.dwActionFlag);
		pCfgDiskErr->bEnable = pDiskAlmCfg->byDiskErrEn;
		
		pCfgDiskErr->hEvent.dwRecord = 0;
		pCfgDiskErr->hEvent.dwTour = 0;
		pCfgDiskErr->hEvent.dwSnapShot = 0;

		int index = min(32, device->channelcount(device));
		for (j = 0; j < index; j++)
		{
			pCfgDiskErr->hEvent.dwSnapShot |= (pDiskAlmCfg->struMsgHandle.bySnap[j]) ? (0x01<<j) : 0;
			pCfgDiskErr->hEvent.dwTour |= (pDiskAlmCfg->struMsgHandle.byTour[j]) ? (0x01<<j) : 0;
			pCfgDiskErr->hEvent.dwRecord |= (pDiskAlmCfg->struMsgHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
						
		}

		for(j = 0; j < 16; j++)
		{
			pCfgDiskErr->hEvent.PtzLink[j].iValue = pDiskAlmCfg->struMsgHandle.struPtzLink[j].iValue;
			pCfgDiskErr->hEvent.PtzLink[j].iType = pDiskAlmCfg->struMsgHandle.struPtzLink[j].iType;
		}

		pCfgDiskErr->hEvent.dwAlarmOut = 0;
		pCfgDiskErr->hEvent.wiAlarmOut = 0;
		index = min(32, device->alarmoutputcount(device));
		for (j = 0; j < index; j++)
		{
			pCfgDiskErr->hEvent.dwAlarmOut |= (pDiskAlmCfg->struMsgHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
			pCfgDiskErr->hEvent.wiAlarmOut |= (pDiskAlmCfg->struMsgHandle.byRelWIAlarmOut[j]) ? (0x01<<j) : 0;
		}
		pCfgDiskErr->hEvent.iAOLatch = pDiskAlmCfg->struMsgHandle.dwDuration;
		pCfgDiskErr->hEvent.iRecordLatch = pDiskAlmCfg->struMsgHandle.dwRecLatch;
		pCfgDiskErr->hEvent.iEventLatch = pDiskAlmCfg->struMsgHandle.dwEventLatch;
		pCfgDiskErr->hEvent.bMessagetoNet = (BOOL)pDiskAlmCfg->struMsgHandle.bMessageToNet;
		pCfgDiskErr->hEvent.bMMSEn = pDiskAlmCfg->struMsgHandle.bMMSEn;
		pCfgDiskErr->hEvent.SnapshotTimes = pDiskAlmCfg->struMsgHandle.bySnapshotTimes;
		pCfgDiskErr->hEvent.bLog = (BOOL)pDiskAlmCfg->struMsgHandle.bLog;
		pCfgDiskErr->hEvent.bMatrixEn = (BOOL)pDiskAlmCfg->struMsgHandle.bMatrixEn;
		pCfgDiskErr->hEvent.dwMatrix = pDiskAlmCfg->struMsgHandle.dwMatrix;
		
		nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_ALARM_DISKERR, 0, cRecvBuf, sizeof(CONFIG_GENERIC_EVENT), 2000);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}

		/*
		 *	先读取配置，再将硬盘数目保存过去。
		 */
		memset(cRecvBuf, 0, iRecvBufLen);
		nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_DISKNUM, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
		if (nRet >= 0 && retlen == sizeof(CONFIG_STORAGE_NUMBER_EVENT))
		{
			CONFIG_STORAGE_NUMBER_EVENT *pCfgDiskNum = (CONFIG_STORAGE_NUMBER_EVENT *)cRecvBuf;
			pCfgDiskNum->iHddNumber = pDiskAlmCfg->bDiskNum;

			nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_ALARM_DISKNUM, 0, cRecvBuf, sizeof(CONFIG_STORAGE_NUMBER_EVENT), 2000);
			if(nRet < 0)
			{
				nRet = -1;
				goto END;
			}
		}
	}
//	else
//	{
//		nRet = -1;
//		goto END;
//	}
	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
}

//net broken alarm
int	CDevConfigEx::GetDevNewConfig_NetBrokenALMCfg(LONG lLoginID, NET_BROKEN_ALARM_CFG_EX* pNetBrkCfg, int waittime)
{
	afk_device_s * device = (afk_device_s *)lLoginID;
	if (NULL == device || NULL == pNetBrkCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;
	
	int i = 0, j = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < sizeof(CONFIG_GENERIC_EVENT))
	{
		iRecvBufLen = sizeof(CONFIG_GENERIC_EVENT);
	}
	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_NETBROKEN, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_GENERIC_EVENT))
	{
		CONFIG_GENERIC_EVENT *pCfgNetBrk = (CONFIG_GENERIC_EVENT *)cRecvBuf;
		m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_NETBROKEN, &pNetBrkCfg->struHandle.dwActionMask);
		pNetBrkCfg->byEnable = pCfgNetBrk->bEnable;
		int index = min(32, device->channelcount(device));
		for (j = 0; j < index; j++)
		{
			
			pNetBrkCfg->struHandle.byRecordChannel[j] = BITRHT(pCfgNetBrk->hEvent.dwRecord,j)&1;
			pNetBrkCfg->struHandle.byTour[j] = BITRHT(pCfgNetBrk->hEvent.dwTour,j)&1;
			pNetBrkCfg->struHandle.bySnap[j] = BITRHT(pCfgNetBrk->hEvent.dwSnapShot,j)&1;
		}
		for(j = 0; j < 16; j++)
		{
			pNetBrkCfg->struHandle.struPtzLink[j].iValue = pCfgNetBrk->hEvent.PtzLink[j].iValue;
			pNetBrkCfg->struHandle.struPtzLink[j].iType = pCfgNetBrk->hEvent.PtzLink[j].iType;
		}
		index = min(32, device->alarmoutputcount(device));
		for (j = 0; j < index; j++)
		{
			pNetBrkCfg->struHandle.byRelAlarmOut[j] = BITRHT(pCfgNetBrk->hEvent.dwAlarmOut,j)&1;
			pNetBrkCfg->struHandle.byRelWIAlarmOut[j] =BITRHT(pCfgNetBrk->hEvent.wiAlarmOut,j)&1;
		}
		pNetBrkCfg->struHandle.dwDuration = pCfgNetBrk->hEvent.iAOLatch;
		pNetBrkCfg->struHandle.dwRecLatch = pCfgNetBrk->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
		pNetBrkCfg->struHandle.dwEventLatch = pCfgNetBrk->hEvent.iEventLatch;
		pNetBrkCfg->struHandle.bMessageToNet = (BYTE)pCfgNetBrk->hEvent.bMessagetoNet;
		pNetBrkCfg->struHandle.bMMSEn = (BYTE)pCfgNetBrk->hEvent.bMMSEn;
		pNetBrkCfg->struHandle.bySnapshotTimes = pCfgNetBrk->hEvent.SnapshotTimes;//短信发送图片的张数
		pNetBrkCfg->struHandle.bLog = (BYTE)pCfgNetBrk->hEvent.bLog;
		pNetBrkCfg->struHandle.bMatrixEn = (BYTE)pCfgNetBrk->hEvent.bMatrixEn;
		pNetBrkCfg->struHandle.dwMatrix = pCfgNetBrk->hEvent.dwMatrix;
		m_pManager->GetDevConfig().GetAlmActionFlag(pCfgNetBrk->hEvent, &pNetBrkCfg->struHandle.dwActionFlag);
	}
	else
	{
		nRet = -1;
		goto END;
	}
	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
	
}
int	CDevConfigEx::SetDevNewConfig_NetBrokenALMCfg(LONG lLoginID, NET_BROKEN_ALARM_CFG_EX* pNetBrkCfg, int waittime)
{
	afk_device_s * device = (afk_device_s *)lLoginID;
	if (NULL == device || NULL == pNetBrkCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;
	
	int i = 0, j = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < sizeof(CONFIG_GENERIC_EVENT))
	{
		iRecvBufLen = sizeof(CONFIG_GENERIC_EVENT);
	}
	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
	//	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_NETBROKEN, 0, cRecvBuf, iRecvBufLen, &retlen, g_query_wait);
	//	if (nRet >= 0 && retlen == sizeof(CONFIG_GENERIC_EVENT))
	{
		CONFIG_GENERIC_EVENT *pCfgNetBrk = (CONFIG_GENERIC_EVENT *)cRecvBuf;
		
		m_pManager->GetDevConfig().SetAlmActionFlag(&pCfgNetBrk->hEvent, pNetBrkCfg->struHandle.dwActionFlag);
		pCfgNetBrk->bEnable = pNetBrkCfg->byEnable;
		
		pCfgNetBrk->hEvent.dwRecord = 0;
		pCfgNetBrk->hEvent.dwTour = 0;
		pCfgNetBrk->hEvent.dwSnapShot = 0;
		
		int index = min(32, device->channelcount(device));
		for (j = 0; j < index; j++)
		{
			pCfgNetBrk->hEvent.dwSnapShot |= (pNetBrkCfg->struHandle.bySnap[j]) ? (0x01<<j) : 0;
			pCfgNetBrk->hEvent.dwTour |= (pNetBrkCfg->struHandle.byTour[j]) ? (0x01<<j) : 0;
			pCfgNetBrk->hEvent.dwRecord |= (pNetBrkCfg->struHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
			
		}
		for(j = 0; j < 16; j++)
		{
			pCfgNetBrk->hEvent.PtzLink[j].iValue = pNetBrkCfg->struHandle.struPtzLink[j].iValue;
			pCfgNetBrk->hEvent.PtzLink[j].iType = pNetBrkCfg->struHandle.struPtzLink[j].iType;
		}
		
		pCfgNetBrk->hEvent.dwAlarmOut = 0;
		//		pCfgNetBrk->hEvent.wiAlarmOut = 0;
		index = min(32, device->alarmoutputcount(device));
		for (j = 0; j < index; j++)
		{
			pCfgNetBrk->hEvent.dwAlarmOut |= (pNetBrkCfg->struHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
			//			pCfgNetBrk->hEvent.wiAlarmOut |= (pNetBrkCfg->struHandle.byRelWIAlarmOut[j]) ? (0x01<<j) : 0;
		}
		pCfgNetBrk->hEvent.iAOLatch = pNetBrkCfg->struHandle.dwDuration;
		pCfgNetBrk->hEvent.iRecordLatch = pNetBrkCfg->struHandle.dwRecLatch;
		//		pCfgNetBrk->hEvent.iEventLatch = pNetBrkCfg->struHandle.dwEventLatch;
		pCfgNetBrk->hEvent.bMessagetoNet = (BOOL)pNetBrkCfg->struHandle.bMessageToNet;
		pCfgNetBrk->hEvent.bMMSEn = pNetBrkCfg->struHandle.bMMSEn;
		pCfgNetBrk->hEvent.SnapshotTimes = pNetBrkCfg->struHandle.bySnapshotTimes;//短信发送图片的张数
		pCfgNetBrk->hEvent.bLog = (BOOL)pNetBrkCfg->struHandle.bLog;
		pCfgNetBrk->hEvent.bMatrixEn = (BOOL)pNetBrkCfg->struHandle.bMatrixEn;
		pCfgNetBrk->hEvent.dwMatrix = pNetBrkCfg->struHandle.dwMatrix;
		nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_ALARM_NETBROKEN, 0, cRecvBuf, sizeof(CONFIG_GENERIC_EVENT), 2000);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}
	//	else
	//	{
	//		nRet = -1;
	//		goto END;
	//	}
	
	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
}	


int	CDevConfigEx::GetDevNewConfig_EncoderCfg(LONG lLoginID, DEV_ENCODER_CFG *pEncoderCfg, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == pEncoderCfg)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int nRet = -1;
	memset(pEncoderCfg, 0, sizeof(DEV_ENCODER_CFG));
	
	int nRetLen = 0;
	char buffer[512] = {0};
	
	// 先查询解码通道个数
	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_DEC_QUERY_CHNNUM, NULL, buffer, 512, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = GetProtocolValue(buffer, "ChannelNum:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		
		pEncoderCfg->nChannels = atoi(szValue);

		p = GetProtocolValue(buffer, "HDAbility:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			pEncoderCfg->byHDAbility = 0;
		}
		else
		{
			pEncoderCfg->byHDAbility = atoi(szValue);
		}	 

		p = GetProtocolValue(buffer, "TVAdjust:", "\r\n", szValue, 64);
		if(NULL == p)
		{
			pEncoderCfg->bTVAdjust = 1;//默认为支持
		}
		else
		{
			if( _stricmp("True", szValue) == 0)
			{
				pEncoderCfg->bTVAdjust = 1; 
			}
		}

		//默认为0,前面已经memset为0.
		p = GetProtocolValue(buffer, "DecodeTour:", "\r\n", szValue, 64);
		if(NULL != p)
		{
			p = GetProtocolValue(buffer, "TourMaxChn:", "\r\n", szValue, 64);
			if(NULL != p)
			{
				pEncoderCfg->bDecodeTour = atoi(szValue);
			}
		}

		p = GetProtocolValue(buffer, "RemotePTZControl:", "\r\n", szValue, 64);
		if(NULL != p)
		{
			pEncoderCfg->bRemotePTZCtl = ( 0 == _stricmp(szValue, "True")) ? 1 : 0;
		}
	}
	
	// 再查询解码通道信息
	for(int i = 0; i < pEncoderCfg->nChannels; i++)
	{
		nRet = m_pManager->GetDecoderDevice().QueryDecEncoderInfo(lLoginID, i, &pEncoderCfg->stuDevInfo[i], waittime);
		if (nRet < 0)
		{
			break;
		}
	}
		
	return nRet;	
}

int	CDevConfigEx::SetDevNewConfig_EncoderCfg(LONG lLoginID, DEV_ENCODER_CFG *pEncoderCfg, int waittime)
{
	afk_device_s * device = (afk_device_s *)lLoginID;
	if (NULL == device || NULL == pEncoderCfg)
	{
		return -1;
	}
	
	int nRet = -1;  //return value
	for(int i = 0; i < pEncoderCfg->nChannels; i++)
	{
		nRet = m_pManager->GetDecoderDevice().SwitchDecTVEncoder(lLoginID, i, &pEncoderCfg->stuDevInfo[i], NULL);
		if (nRet < 0 )
		{
			nRet = NET_ERROR_SETCFG_ENCODERINFO;
		}
		else
		{
			nRet = NET_NOERROR;
		}
	}
	
	return nRet;
}

//IP collision detection alarm
int CDevConfigEx::GetDevNewConfig_IPCOllisionDetect(LONG lLoginID, ALARM_IP_COLLISION_CFG *lpAlarmCfg, int waittime)
{
	afk_device_s * device = (afk_device_s *)lLoginID;
	if (NULL == device || NULL == lpAlarmCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;
	
	int i = 0, j = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < sizeof(CONFIG_GENERIC_EVENT))
	{
		iRecvBufLen = sizeof(CONFIG_GENERIC_EVENT);
	}
	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = m_pManager->GetDevConfig().QueryConfig(lLoginID, CONFIG_TYPE_ALARM_IPCOLLISION, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_GENERIC_EVENT))
	{
		CONFIG_GENERIC_EVENT *pCfgIPCollision = (CONFIG_GENERIC_EVENT *)cRecvBuf;
		m_pManager->GetDevConfig().GetAlmActionMsk(CONFIG_TYPE_ALARM_IPCOLLISION, &lpAlarmCfg->struHandle.dwActionMask);
		lpAlarmCfg->bEnable = pCfgIPCollision->bEnable;
		int index = min(32, device->channelcount(device));
		for (j = 0; j < index; j++)
		{
			
			lpAlarmCfg->struHandle.byRecordChannel[j] = BITRHT(pCfgIPCollision->hEvent.dwRecord,j)&1;
			lpAlarmCfg->struHandle.byTour[j] = BITRHT(pCfgIPCollision->hEvent.dwTour,j)&1;
			lpAlarmCfg->struHandle.bySnap[j] = BITRHT(pCfgIPCollision->hEvent.dwSnapShot,j)&1;
		}
		for(j = 0; j < 16; j++)
		{
			lpAlarmCfg->struHandle.struPtzLink[j].iValue = pCfgIPCollision->hEvent.PtzLink[j].iValue;
			lpAlarmCfg->struHandle.struPtzLink[j].iType = pCfgIPCollision->hEvent.PtzLink[j].iType;
		}
		index = min(32, device->alarmoutputcount(device));
		for (j = 0; j < index; j++)
		{
			lpAlarmCfg->struHandle.byRelAlarmOut[j] = BITRHT(pCfgIPCollision->hEvent.dwAlarmOut,j)&1;
			lpAlarmCfg->struHandle.byRelWIAlarmOut[j] =BITRHT(pCfgIPCollision->hEvent.wiAlarmOut,j)&1;
		}
		lpAlarmCfg->struHandle.dwDuration = pCfgIPCollision->hEvent.iAOLatch;
		lpAlarmCfg->struHandle.dwRecLatch = pCfgIPCollision->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
		lpAlarmCfg->struHandle.dwEventLatch = pCfgIPCollision->hEvent.iEventLatch;
		lpAlarmCfg->struHandle.bMessageToNet = (BYTE)pCfgIPCollision->hEvent.bMessagetoNet;
		lpAlarmCfg->struHandle.bMMSEn = (BYTE)pCfgIPCollision->hEvent.bMMSEn;
		lpAlarmCfg->struHandle.bySnapshotTimes = pCfgIPCollision->hEvent.SnapshotTimes;//短信发送图片的张数
		lpAlarmCfg->struHandle.bLog = (BYTE)pCfgIPCollision->hEvent.bLog;
		lpAlarmCfg->struHandle.bMatrixEn = (BYTE)pCfgIPCollision->hEvent.bMatrixEn;
		lpAlarmCfg->struHandle.dwMatrix = pCfgIPCollision->hEvent.dwMatrix;
		m_pManager->GetDevConfig().GetAlmActionFlag(pCfgIPCollision->hEvent, &lpAlarmCfg->struHandle.dwActionFlag);
	}
	else
	{
		nRet = -1;
		goto END;
	}
	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
	
}
int	CDevConfigEx::SetDevNewConfig_IPCOllisionDetect(LONG lLoginID, ALARM_IP_COLLISION_CFG *lpAlarmCfg, int waittime)
{
	afk_device_s * device = (afk_device_s *)lLoginID;
	if (NULL == device || NULL == lpAlarmCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;
	
	int i = 0, j = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < sizeof(CONFIG_GENERIC_EVENT))
	{
		iRecvBufLen = sizeof(CONFIG_GENERIC_EVENT);
	}
	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
	//	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_IPCOLLISION, 0, cRecvBuf, iRecvBufLen, &retlen, g_query_wait);
	//	if (nRet >= 0 && retlen == sizeof(CONFIG_GENERIC_EVENT))
	{
		CONFIG_GENERIC_EVENT *pCfgIPCollision = (CONFIG_GENERIC_EVENT *)cRecvBuf;
		
		m_pManager->GetDevConfig().SetAlmActionFlag(&pCfgIPCollision->hEvent, lpAlarmCfg->struHandle.dwActionFlag);
		pCfgIPCollision->bEnable = lpAlarmCfg->bEnable;
		
		pCfgIPCollision->hEvent.dwRecord = 0;
		pCfgIPCollision->hEvent.dwTour = 0;
		pCfgIPCollision->hEvent.dwSnapShot = 0;
		
		int index = min(32, device->channelcount(device));
		for (j = 0; j < index; j++)
		{
			pCfgIPCollision->hEvent.dwSnapShot |= (lpAlarmCfg->struHandle.bySnap[j]) ? (0x01<<j) : 0;
			pCfgIPCollision->hEvent.dwTour |= (lpAlarmCfg->struHandle.byTour[j]) ? (0x01<<j) : 0;
			pCfgIPCollision->hEvent.dwRecord |= (lpAlarmCfg->struHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
			
		}
		for(j = 0; j < 16; j++)
		{
			pCfgIPCollision->hEvent.PtzLink[j].iValue = lpAlarmCfg->struHandle.struPtzLink[j].iValue;
			pCfgIPCollision->hEvent.PtzLink[j].iType = lpAlarmCfg->struHandle.struPtzLink[j].iType;
		}
		
		pCfgIPCollision->hEvent.dwAlarmOut = 0;
		index = min(32, device->alarmoutputcount(device));
		for (j = 0; j < index; j++)
		{
			pCfgIPCollision->hEvent.dwAlarmOut |= (lpAlarmCfg->struHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
		}
		pCfgIPCollision->hEvent.iAOLatch = lpAlarmCfg->struHandle.dwDuration;
		pCfgIPCollision->hEvent.iRecordLatch = lpAlarmCfg->struHandle.dwRecLatch;
		pCfgIPCollision->hEvent.bMessagetoNet = (BOOL)lpAlarmCfg->struHandle.bMessageToNet;
		pCfgIPCollision->hEvent.bMMSEn = lpAlarmCfg->struHandle.bMMSEn;
		pCfgIPCollision->hEvent.SnapshotTimes = lpAlarmCfg->struHandle.bySnapshotTimes;//短信发送图片的张数
		pCfgIPCollision->hEvent.bLog = (BOOL)lpAlarmCfg->struHandle.bLog;
		pCfgIPCollision->hEvent.bMatrixEn = (BOOL)lpAlarmCfg->struHandle.bMatrixEn;
		pCfgIPCollision->hEvent.dwMatrix = lpAlarmCfg->struHandle.dwMatrix;
		nRet = m_pManager->GetDevConfig().SetupConfig(lLoginID, CONFIG_TYPE_ALARM_IPCOLLISION, 0, cRecvBuf, sizeof(CONFIG_GENERIC_EVENT), 2000);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}
	//	else
	//	{
	//		nRet = -1;
	//		goto END;
	//	}
	
	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
}	

int CDevConfigEx::GetDevNewConfig_TVAdjustCfg(LONG lLoginID, LONG lTVID, DEVICE_TVADJUST_CFG* pTVAdjustCfg, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == pTVAdjustCfg)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int nRet = -1;
	memset(pTVAdjustCfg, 0, sizeof(DEVICE_TVADJUST_CFG));
	
	int nRetLen = 0;
	char buffer[512] = {0};
	
	int nTVID = lTVID;
	
	// 查询TV调节参数
	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_TVADJUST_CFG, (void *)lTVID, buffer, 512, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = GetProtocolValue(buffer, "Top:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		pTVAdjustCfg->nTop = atoi(szValue);

		p = GetProtocolValue(buffer, "Bottom:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		pTVAdjustCfg->nBotton = atoi(szValue);
		
		p = GetProtocolValue(buffer, "Right:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		pTVAdjustCfg->nRight = atoi(szValue);

		p = GetProtocolValue(buffer, "Left:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		
		pTVAdjustCfg->nLeft = atoi(szValue);	
	}
	else
	{
		nRet = NET_ERROR_GETCFG_TVADJUST;		
	}
	
	return nRet;	
}

int CDevConfigEx::SetDevNewConfig_TVAdjustCfg(LONG lLoginID, LONG lTVID, DEVICE_TVADJUST_CFG* pTVAdjustCfg, int waittime)
{
	if (NULL == pTVAdjustCfg)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int nRet = -1;  //return value
	char buffer[1024] = {0};
	int nRetLen = 0;
	
	//set TV adjust configure
	REQUEST_SET_TVADJUST tmpTvAdjust = {0};
	tmpTvAdjust.nTop = pTVAdjustCfg->nTop;
	tmpTvAdjust.nBotton = pTVAdjustCfg->nBotton;
	tmpTvAdjust.nLeft = pTVAdjustCfg->nLeft;
	tmpTvAdjust.nRight = pTVAdjustCfg->nRight;
	tmpTvAdjust.nTVID = lTVID;

	
	nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_SET_TVADJUST_CFG, (void*)&tmpTvAdjust, waittime);
	
	if (nRet < 0 )
	{
		nRet = NET_ERROR_SETCFG_TVADJUST;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	
	return nRet;
}

int CDevConfigEx::GetDevNewConfig_VehicleCfg(LONG lLoginID, DEV_ABOUT_VEHICLE_CONFIG* pVehicleCfg, int waittime)
{
	if(NULL == pVehicleCfg)
	{
		return NET_ILLEGAL_PARAM;
	}

	char pszRequest[512] = {0};
	char pszAck[1024] = {0};
	int nRetLen = 0;
	int nRet = -1;

	sprintf(pszRequest, "%s", "Method:GetParameterValues\r\n");
	sprintf(pszRequest+strlen(pszRequest), "%s" ,"ParameterName:Dahua.Device.OnVehicle.General\r\n");
	sprintf(pszRequest+strlen(pszRequest), "%s", "\r\n");
	
	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_VEHICLE_CFG, (LPVOID)pszRequest, pszAck, 1024, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char* p = GetProtocolValue(pszAck, "AutoBoot.Enable:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		pVehicleCfg->bAutoBootEnable = atoi(szValue) > 0 ? TRUE : FALSE;

		p = GetProtocolValue(pszAck, "AutoBoot.Day:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		pVehicleCfg->dwAutoBootTime = atoi(szValue);

		p = GetProtocolValue(pszAck, "AutoShutDown.Enable:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		pVehicleCfg->bAutoShutDownEnable = atoi(szValue) > 0 ? TRUE : FALSE;

		p = GetProtocolValue(pszAck, "AutoShutDown.Day:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		pVehicleCfg->dwAutoShutDownTime = atoi(szValue);

		p = GetProtocolValue(pszAck, "AutoShutDown.DelayTime:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		pVehicleCfg->dwShutDownDelayTime = atoi(szValue);

		p = GetProtocolValue(pszAck, "Event.NoDisk:", "\r\n", szValue, 64);
		if (p == NULL)
		{
			return NET_RETURN_DATA_ERROR;
		}
		if(_stricmp(szValue, "Ignore") == 0)
		{
			pVehicleCfg->bEventNoDisk = 1;
		}
		else if(_stricmp(szValue, "Reboot") ==0)
		{
			pVehicleCfg->bEventNoDisk = 2;
		}
		else
		{
			return NET_RETURN_DATA_ERROR; 
		}
	}
	else
	{
		nRet = NET_ERROR_GET_VEHICLE_CFG;;
	}

	return nRet;
}

int CDevConfigEx::SetDevNewConfig_VehicleCfg(LONG lLoginID, DEV_ABOUT_VEHICLE_CONFIG* pVehicleCfg, int waittime)
{
	if(NULL == pVehicleCfg)
	{
		return NET_ILLEGAL_PARAM;
	}

	char pszSetCfg[1024] = {0};
	sprintf(pszSetCfg, "%s", "Method:SetParameterValues\r\n");
	sprintf(pszSetCfg+strlen(pszSetCfg), "%s", "ParameterName:Dahua.Device.OnVehicle.General\r\n");
	sprintf(pszSetCfg+strlen(pszSetCfg), "AutoBoot.Enable:%d\r\n", pVehicleCfg->bAutoBootEnable > 0 ? 1 : 0);
	sprintf(pszSetCfg+strlen(pszSetCfg), "AutoBoot.Day:%d\r\n", pVehicleCfg->dwAutoBootTime);
	sprintf(pszSetCfg+strlen(pszSetCfg), "AutoShutDown.Enable:%d\r\n", pVehicleCfg->bAutoShutDownEnable > 0 ? 1 : 0);
	sprintf(pszSetCfg+strlen(pszSetCfg), "AutoShutDown.Day:%d\r\n", pVehicleCfg->dwAutoShutDownTime);
	sprintf(pszSetCfg+strlen(pszSetCfg), "AutoShutDown.DelayTime:%d\r\n", pVehicleCfg->dwShutDownDelayTime);
	if(pVehicleCfg->bEventNoDisk != 1 && pVehicleCfg->bEventNoDisk != 2)
	{
		return NET_ILLEGAL_PARAM;
	}
	char* pszEventNoDisk[] = {"Ignore","Reboot"};
	sprintf(pszSetCfg+strlen(pszSetCfg), "Event.NoDisk:%s\r\n", pszEventNoDisk[pVehicleCfg->bEventNoDisk-1]);
	sprintf(pszSetCfg+strlen(pszSetCfg), "%s", "\r\n");

	int nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_SET_VEHICLE_CFG, (LPVOID)pszSetCfg, waittime);
	if (nRet < 0 )
	{
		nRet = NET_ERROR_SET_VEHICLE_CFG;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	
	return nRet;
}

int CDevConfigEx::GetDevNewConfig_AtmOverlayGeneral(LONG lLoginID, DEV_ATM_OVERLAY_GENERAL* pAtmGeneral, int waittime)
{
	if(NULL == pAtmGeneral)
	{
		return NET_ILLEGAL_PARAM;
	}

	
	char pszRequest[512] = {0};
	char pszAck[1024] = {0};
	int nRetLen = 0;
	int nRet = -1;

	sprintf(pszRequest+strlen(pszRequest), "Method:GetParameterValues\r\n");
	sprintf(pszRequest+strlen(pszRequest), "ParameterName:Dahua.Device.ATM.Overlap.General\r\n");
	sprintf(pszRequest+strlen(pszRequest), "\r\n");

	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_ATM_OVERLAY_ABILITY, (LPVOID)pszRequest, pszAck, 1024, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64];
		//data sourece
		char *p = pszAck;
		pAtmGeneral->dwDataSource = 0;
		while(p != NULL)
		{
			p = GetProtocolValue(p, "DataSource:", "\r\n", szValue, 64);
			if(p != NULL)
			{
				if(_stricmp(szValue, "Net") == 0)
				{
					pAtmGeneral->dwDataSource += 1;
				}
				if(_stricmp(szValue, "Com") == 0)
				{
					pAtmGeneral->dwDataSource += 2;
				}
			}
		}
		//protocols
		int nIndex = 0;
		for(p = pszAck; p != NULL; )
		{
			p = GetProtocolValue(p, "Protocol:", "\r\n", szValue, 64);
			if(p != NULL)
			{
				if(strlen(szValue) <= 32)
				{
					memcpy(pAtmGeneral->szProtocol[nIndex], szValue, 32);
					nIndex++;
				}
			}
		}
	}
	else
	{
		nRet = NET_ERROR_GET_ATM_OVERLAY_ABILITY;
	}

	return nRet;
}

int CDevConfigEx::SetDevNewConfig_AtmOverlayConfig(LONG lLoginID, DEV_ATM_OVERLAY_CONFIG* pAtmConfig, int waittime)
{
	if(NULL == pAtmConfig)
	{
		return NET_ILLEGAL_PARAM;
	}

	char pszSetCfg[1024] = {0};
	sprintf(pszSetCfg+strlen(pszSetCfg), "Method:SetParameterValues\r\n");
	sprintf(pszSetCfg+strlen(pszSetCfg), "ParameterName:Dahua.Device.ATM.Overlap.Config\r\n");
	//data sources;
	char* szDataSoure[] = {"Net", "Com"};
	if(pAtmConfig->dwDataSource < 1 || pAtmConfig->dwDataSource > 2)
	{
		return NET_ILLEGAL_PARAM;
	}
	sprintf(pszSetCfg+strlen(pszSetCfg), "DataSource:%s\r\n", szDataSoure[pAtmConfig->dwDataSource-1]);
	//protocol
	sprintf(pszSetCfg+strlen(pszSetCfg), "Protocol:%s\r\n", pAtmConfig->szProtocol);
	//Channel
	DWORD dwMask = 0x00000001;
	for(int i = 0; i < 32; i++)//32 bits
	{
		if((pAtmConfig->dwChannelMask & (dwMask << i)) > 0)
		{
			sprintf(pszSetCfg+strlen(pszSetCfg), "Channel:%d\r\n", i);
		}
	}
	//Mode
	if((pAtmConfig->bMode&0x01) > 0)
	{
		sprintf(pszSetCfg+strlen(pszSetCfg), "Mode:Preview\r\n");
	}
	if((pAtmConfig->bMode&0x02) > 0)
	{
		sprintf(pszSetCfg+strlen(pszSetCfg), "Mode:Encode\r\n");
	}
	//location
	char* szLocation[] = {"LeftTop", "LeftBottom", "RightTop", "RightBottom"};
	if(pAtmConfig->bLocation < 1 || pAtmConfig->bLocation > 4)
	{
		return NET_ILLEGAL_PARAM;
	}
	sprintf(pszSetCfg+strlen(pszSetCfg), "Location:%s\r\n", szLocation[pAtmConfig->bLocation-1]);
	//
	int nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_SET_ATM_OVERLAY_CFG, (LPVOID)pszSetCfg, waittime);
	if (nRet < 0 )
	{
		nRet = NET_ERROR_SET_ATM_OVERLAY_CFG;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	
	return nRet;
}

int CDevConfigEx::GetDevNewConfig_AtmOverlayConfig(LONG lLoginID, DEV_ATM_OVERLAY_CONFIG* pAtmConfig, int waittime)
{
	if(NULL == pAtmConfig)
	{
		return NET_ILLEGAL_PARAM;
	}

	char pszRequest[512] = {0};
	char pszAck[1024] = {0};
	int nRetLen = 0;
	int nRet = -1;

	sprintf(pszRequest+strlen(pszRequest), "Method:GetParameterValues\r\n");
	sprintf(pszRequest+strlen(pszRequest), "ParameterName:Dahua.Device.ATM.Overlap.Config\r\n");
	sprintf(pszRequest+strlen(pszRequest), "\r\n");

	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_ATM_OVERLAY_CFG, (LPVOID)pszRequest, pszAck, 1024, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64];
		//data source
		char* p = GetProtocolValue(pszAck, "DataSource:", "\r\n", szValue, 64);
		if(NULL == p)
		{
			return -1;
		}
		if(_stricmp(szValue, "Net") == 0)
		{
			pAtmConfig->dwDataSource = 1;
		}
		else if(_stricmp(szValue, "Com") == 0)
		{
			pAtmConfig->dwDataSource = 2;
		}
		else
		{
			return -1;
		}
		//Protocol
		p = GetProtocolValue(pszAck, "Protocol:", "\r\n", szValue, 64);
		if(NULL == p)
		{
			return -1;
		}
		memcpy(pAtmConfig->szProtocol, szValue, 32);
		//channel
		p = pszAck;
		DWORD dwMask = 0x00000001;
		while(p != NULL)
		{
			p = GetProtocolValue(p, "Channel:", "\r\n", szValue, 64);
			if (p != NULL)
			{
				pAtmConfig->dwChannelMask |= (dwMask << atoi(szValue));
			}
		}
		//mode
		p = pszAck;
		while(p != NULL)
		{
			p = GetProtocolValue(p, "Mode:", "\r\n", szValue, 64);
			if (p != NULL)
			{
				if(_stricmp(szValue, "Preview") == 0)
				{
					pAtmConfig->bMode += 1;
				}
				if(_stricmp(szValue, "Encode") == 0)
				{
					pAtmConfig->bMode += 2;
				}
			}
		}
		//Location
		p = GetProtocolValue(pszAck, "Location:", "\r\n", szValue, 64);
		if(NULL == p)
		{
			return -1;
		}
		char* szLocation[] = {"LeftTop", "LeftBottom", "RightTop", "RightBottom"};
		for(int i = 0; i < 4; i++)
		{
			if(_stricmp(szValue, szLocation[i]) == 0)
			{
				pAtmConfig->bLocation = i+1;
				break;
			}
		}

	}
	else
	{
		nRet = NET_ERROR_GET_ATM_OVERLAY_CFG;
	}

	return nRet;
}


int CDevConfigEx::GetDevNewConfig_DecoderTourConfig(LONG lLoginID, int nChannel, DEVICE_DECODER_TOUR_CFG* pTourCfg, int waittime)
{
	if(NULL == pTourCfg)
	{
		return NET_ILLEGAL_PARAM;
	}

	char pszRequest[512] = {0};
	char pszAck[4096] = {0};
	int nRetLen = 0;
	int nRet = -1;

	sprintf(pszRequest+strlen(pszRequest), "Method:GetParameterValues\r\n");
	sprintf(pszRequest+strlen(pszRequest), "ParameterName:Dahua.Device.Decode.TourCfg\r\n");
	sprintf(pszRequest+strlen(pszRequest), "DisChn:%d\r\n", nChannel+1);
	sprintf(pszRequest+strlen(pszRequest), "\r\n");

	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_DECODER_TOUR_CFG, (LPVOID)pszRequest, pszAck, 4096, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		memset(pTourCfg, 0, sizeof(pTourCfg));
		//prase string here
		char szValue[64] = {0};
		int nIndex = 0;
		char * p = NULL;
		//ip
		p = pszAck;
		for(nIndex = 0; p != NULL; nIndex++)
		{
			p = GetProtocolValue(p, "IP:", "\r\n", szValue, 64);
			if(NULL != p && (strlen(szValue) < MAX_IP_ADDRESS_LEN))
			{
				strcpy(pTourCfg->tourCfg[nIndex].szIP, szValue);
			}
		}
		//port
		p = pszAck;
		for(nIndex = 0; p != NULL; nIndex++)
		{
			p = GetProtocolValue(p, "Port:", "\r\n", szValue, 64);
			if(NULL != p)
			{
				pTourCfg->tourCfg[nIndex].nPort = atoi(szValue);
			}
		}
		//DstChn
		p = pszAck;
		for(nIndex = 0; p != NULL; nIndex++)
		{
			p = GetProtocolValue(p, "DstChn:", "\r\n", szValue, 64);
			if(NULL != p)
			{
				pTourCfg->tourCfg[nIndex].nPlayChn = atoi(szValue);
			}
		}
		//DstPicType
		p = pszAck;
		for(nIndex = 0; p != NULL; nIndex++)
		{
			p = GetProtocolValue(p, "DstPicType:", "\r\n", szValue, 64);
			if(NULL != p)
			{
				if(_stricmp("main", szValue) == 0)
					pTourCfg->tourCfg[nIndex].nPlayType = 0;
				else if(_stricmp("second", szValue) == 0)
					pTourCfg->tourCfg[nIndex].nPlayType = 1;
			}
		}
		//UserName
		p = pszAck;
		for(nIndex = 0; p != NULL; nIndex++)
		{
			p = GetProtocolValue(p, "UserName:", "\r\n", szValue, 64);
			if(NULL != p && (strlen(szValue) < 32))
			{
				strcpy(pTourCfg->tourCfg[nIndex].szUserName, szValue);
			}
		}
		//PassWord
		p = pszAck;
		for(nIndex = 0; p != NULL; nIndex++)
		{
			p = GetProtocolValue(p, "PassWord:", "\r\n", szValue, 64);
			if(NULL != p && (strlen(szValue) < 32))
			{
				strcpy(pTourCfg->tourCfg[nIndex].szPassword, szValue);
			}
		}

		//Interval
		p = pszAck;
		for(nIndex = 0; p != NULL; nIndex++)
		{
			p = GetProtocolValue(p, "Interval:", "\r\n", szValue, 64);
			if(NULL != p && (strlen(szValue) < 32))
			{
				pTourCfg->tourCfg[nIndex].nInterval = atoi(szValue);
			}
		}

		pTourCfg->nCfgNum = nIndex-1;//上面每一项数目应该相等,所以取其中一项数目即可.
	}
	else
	{
		nRet = NET_ERROR_GET_DECODER_TOUR_CFG;
	}

	return nRet;
	
}

int CDevConfigEx::SetDevNewConfig_DecoderTourConfig(LONG lLoginID, int nChannel, DEVICE_DECODER_TOUR_CFG* pTourCfg, int waittime)
{
	if(NULL == pTourCfg)
	{
		return NET_ILLEGAL_PARAM;
	}
	int nRet = -1;
	int nRetLen = 0;

	//first, Query the upper limit supported.
	int nDecoderTour = 0;
	char buffer[512] = {0};
	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_DEC_QUERY_CHNNUM, NULL, buffer, 512, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		//默认为0,前面已经memset为0.
		char szValue[64] = {0};
		char *p = GetProtocolValue(buffer, "DecodeTour:", "\r\n", szValue, 64);
		if(NULL != p)
		{
			p = GetProtocolValue(buffer, "TourMaxChn:", "\r\n", szValue, 64);
			if(NULL != p)
			{
				nDecoderTour = atoi(szValue);
			}
		}
	}

	//calculate the struct number according to upper limit supported, which probability expands to 64. 
	int nStuNum = (nDecoderTour < pTourCfg->nCfgNum) ? nDecoderTour : pTourCfg->nCfgNum; 
	
	char pszSetCfg[4096] = {0};
	sprintf(pszSetCfg, "Method:SetParameterValues\r\n");
	sprintf(pszSetCfg+strlen(pszSetCfg), "ParameterName:Dahua.Device.Decode.TourCfg\r\n");
	sprintf(pszSetCfg+strlen(pszSetCfg), "DisChn:%d\r\n", nChannel+1);
	for(int i = 0; i < nStuNum; i++)
	{
		DEV_DECODER_TOUR_SINGLE_CFG* pTourSingleCfg = &(pTourCfg->tourCfg[i]);
		//avoid exception.
		pTourSingleCfg->szIP[127] = '\0';
		pTourSingleCfg->szPassword[31] = '\0';
		pTourSingleCfg->szUserName[31] = '\0';
		
		char* szStreamType[] = {"main", "second"};
		if(pTourSingleCfg->nPlayType < 0 || pTourSingleCfg->nPlayType > 1)
		{
			return NET_ILLEGAL_PARAM;
		}

		sprintf(pszSetCfg+strlen(pszSetCfg), "IP:%s\r\n", pTourSingleCfg->szIP);
		sprintf(pszSetCfg+strlen(pszSetCfg), "Port:%d\r\n", pTourSingleCfg->nPort);
		sprintf(pszSetCfg+strlen(pszSetCfg), "DstChn:%d\r\n", pTourSingleCfg->nPlayChn);
		sprintf(pszSetCfg+strlen(pszSetCfg), "DstPicType:%s\r\n", szStreamType[pTourSingleCfg->nPlayType]);
		sprintf(pszSetCfg+strlen(pszSetCfg), "UserName:%s\r\n", pTourSingleCfg->szUserName);
		sprintf(pszSetCfg+strlen(pszSetCfg), "PassWord:%s\r\n", pTourSingleCfg->szPassword);
		sprintf(pszSetCfg+strlen(pszSetCfg), "Interval:%d\r\n", pTourSingleCfg->nInterval);
	}
	sprintf(pszSetCfg+strlen(pszSetCfg), "\r\n");

	nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_SET_DECODER_TOUR_CFG, (LPVOID)pszSetCfg, waittime);
	if (nRet < 0 )
	{
		nRet = NET_ERROR_SET_DECODER_TOUR_CFG;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	

	return nRet;
}

int CDevConfigEx::GetDevNewConfig_SIPAbility(LONG lLoginID, STREAM_MEDIA_EN* pStreamMediaEn, int waittime)
{
	if(NULL == pStreamMediaEn || waittime <= 0)
	{
		return NET_ILLEGAL_PARAM;
	}

	char szAck[1024] = {0};
	char szRequest[1024] = {0};
	int nRetLen = 0;
	sprintf(szRequest, "Method:GetParameterNames\r\n");
	sprintf(szRequest+strlen(szRequest), "ParameterName:Dahua.Device.StreamMedia.Info\r\n\r\n");
	int nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_SIP_ABILITY, (LPVOID)szRequest, szAck, 1024, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char* p =GetProtocolValue(szAck, "ProtocolType:", "\r\n", szValue, 64);
		if(NULL == p)
		{
			return NET_ERROR_GET_SIP_ABILITY;
		}
		char *szProType[] = {"", "SIP", "RTSP"};
		for(int i = 0; i < sizeof(szProType)/sizeof(char*); i++)
		{
			if(_stricmp(szValue, szProType[i]) == 0)
			{
				pStreamMediaEn->nStreamType = i;
				return NET_NOERROR;
			}
		}

		
		return NET_ERROR_GET_SIP_ABILITY;
	}

	return NET_ERROR_GET_SIP_ABILITY;
}

int CDevConfigEx::GetDevNewConfig_SIPConfig(LONG lLoginID, DEVICE_SIP_CFG* pSIPCfg, int waittime)
{
	if(pSIPCfg == NULL || waittime <= 0)
	{
		return NET_ILLEGAL_PARAM;
	}

	char szAck[4096] = {0};
	char szRequest[1024] = {0};
	int nRetLen = 0;
	sprintf(szRequest, "Method:GetParameterValues\r\n");
	sprintf(szRequest+strlen(szRequest), "ParameterName:Dahua.Device.StreamMedia.SIP.Cfg\r\n\r\n");
	int nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_SIP_CFG, (LPVOID)szRequest, szAck, 4096, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		memset(pSIPCfg, 0, sizeof(DEVICE_SIP_CFG));

		char szValue[128];
		char* p = GetProtocolValue(szAck, "UnregisterOnReboot:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_SIP_CFG;
		}
		pSIPCfg->bUnregOnBoot = atoi(szValue);

		p = GetProtocolValue(szAck, "AccoutName:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_SIP_CFG;
		}
		strncpy(pSIPCfg->szAccoutName, szValue, 63);
		
		p = GetProtocolValue(szAck, "SIPServer:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_SIP_CFG;
		}
		strncpy(pSIPCfg->szSIPServer, szValue, 127);

		p = GetProtocolValue(szAck, "Proxy:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_SIP_CFG;
		}
		strncpy(pSIPCfg->szOutBoundProxy, szValue, 127);
		
		p = GetProtocolValue(szAck, "UserID:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_SIP_CFG;
		}
		pSIPCfg->dwSIPUsrID = atoi(szValue);
		
		p = GetProtocolValue(szAck, "AuthID:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_SIP_CFG;
		}
		pSIPCfg->dwAuthID = atoi(szValue);

		p = GetProtocolValue(szAck, "AuthPassword:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_SIP_CFG;
		}
		strncpy(pSIPCfg->szAuthPsw, szValue, 63);
		
		p = GetProtocolValue(szAck, "STUNServer:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_SIP_CFG;
		}
		strncpy(pSIPCfg->szSTUNServer, szValue, 127);

		p = GetProtocolValue(szAck, "RegExpiration:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_SIP_CFG;
		}
		pSIPCfg->dwRegExp = atoi(szValue);

		p = GetProtocolValue(szAck, "LocalSIPPort:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_SIP_CFG;
		}
		pSIPCfg->dwLocalSIPPort = atoi(szValue);

		p = GetProtocolValue(szAck, "LocalRTPPort:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_SIP_CFG;
		}
		pSIPCfg->dwLocalRTPPort = atoi(szValue);
	}

	return NET_NOERROR;
}
//REQUEST_POS_INFO
int CDevConfigEx::GetDevNewConfig_POSConfig( LONG lLoginID , DEVICE_POS_INFO* pPosCfg , int waittime ) 
{
	if(pPosCfg == NULL || waittime <= 0)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	char szAck[4096] = {0};
	char szRequest[1024] = {0};
	int nRetLen = 0;
	sprintf(szRequest, "Method:GetParameterValues\r\n");
	sprintf(szRequest+strlen(szRequest), "ParameterName:ZenoIntel.Device.OnPos.Pos.Cfg\r\n\r\n");
	int nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_POS_CFG, (LPVOID)szRequest, szAck, 4096, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		memset(pPosCfg, 0, sizeof(DEVICE_POS_INFO));
		
		char szValue[128];
		char* p = GetProtocolValue(szAck, "FontColor:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_POS_CFG;
		}
		pPosCfg->fontColor = atoi(szValue);
		
		p = GetProtocolValue(szAck, "FontSize:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_POS_CFG;
		}
		pPosCfg->fontSize = atoi( szValue ) ;
		
		p = GetProtocolValue(szAck, "FontX:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_POS_CFG;
		}
		pPosCfg->fontX = atoi( szValue ) ;
		
		p = GetProtocolValue(szAck, "FontY:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_POS_CFG;
		}
		pPosCfg->fontY = atoi( szValue ) ;
		
		p = GetProtocolValue(szAck, "FontShow:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_POS_CFG;
		}
		pPosCfg->show = atoi(szValue);
		
		p = GetProtocolValue(szAck, "HoldTime:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_POS_CFG;
		}
		pPosCfg->holdTime = atoi(szValue);
		
		p = GetProtocolValue(szAck, "POSAlarmServerAddr:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_POS_CFG;
		}
		strncpy(pPosCfg->posAlarmHostAddr , szValue, 63);
		
		p = GetProtocolValue(szAck, "POSAlarmServerPort:", "\r\n", szValue, 128);
		if(NULL == p)
		{
			return NET_ERROR_GET_POS_CFG;
		}
		pPosCfg->posAlarmHostPort = atoi( szValue ) ;
	}
	
	return NET_NOERROR;
}

int CDevConfigEx::SetDevNewConfig_SIPConfig(LONG lLoginID, DEVICE_SIP_CFG* pSIPCfg, int waittime)
{
	if(pSIPCfg == NULL || waittime <= 0)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	char szCfg[4096] = {0};
	char szAck[1024] = {0};
	int nRet = 0;
	//avoid exception
	pSIPCfg->szAccoutName[63] = '\0';
	pSIPCfg->szAuthPsw[63] = '\0';
	pSIPCfg->szOutBoundProxy[127] = '\0';
	pSIPCfg->szSIPServer[127] = '\0';
	pSIPCfg->szSTUNServer[127] = '\0';
	
	sprintf(szCfg, "Method:SetParameterValues\r\n");
	sprintf(szCfg+strlen(szCfg), "ParameterName:Dahua.Device.StreamMedia.SIP.Cfg\r\n");
	sprintf(szCfg+strlen(szCfg), "UnregisterOnReboot:%d\r\n", pSIPCfg->bUnregOnBoot);
	sprintf(szCfg+strlen(szCfg), "AccoutName:%s\r\n", pSIPCfg->szAccoutName);
	sprintf(szCfg+strlen(szCfg), "SIPServer:%s\r\n", pSIPCfg->szSIPServer);
	sprintf(szCfg+strlen(szCfg), "Proxy:%s\r\n", pSIPCfg->szOutBoundProxy);
	sprintf(szCfg+strlen(szCfg), "UserID:%d\r\n", pSIPCfg->dwSIPUsrID);
	sprintf(szCfg+strlen(szCfg), "AuthID:%d\r\n", pSIPCfg->dwAuthID);
	sprintf(szCfg+strlen(szCfg), "AuthPassword:%s\r\n", pSIPCfg->szAuthPsw);
	sprintf(szCfg+strlen(szCfg), "STUNServer:%s\r\n", pSIPCfg->szSTUNServer);
	sprintf(szCfg+strlen(szCfg), "RegExpiration:%d\r\n", pSIPCfg->dwRegExp);
	sprintf(szCfg+strlen(szCfg), "LocalSIPPort:%d\r\n", pSIPCfg->dwLocalSIPPort);
	sprintf(szCfg+strlen(szCfg), "LocalRTPPort:%d\r\n", pSIPCfg->dwLocalRTPPort);
	sprintf(szCfg+strlen(szCfg), "\r\n");
	
	nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_SET_SIP_CFG, (LPVOID)szCfg, waittime);
	if (nRet < 0 )
	{
		nRet = NET_ERROR_SET_SIP_CFG;
	}
	else
	{
		nRet = NET_NOERROR;
	}

	return NET_NOERROR;
}

int CDevConfigEx::SetDevNetConfig_POSConfig( LONG lLoginID , DEVICE_POS_INFO *pPosCfg , int waittime ) 
{
	if(pPosCfg == NULL || waittime <= 0)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	char szCfg[4096] = {0};
	char szAck[1024] = {0};
	int nRet = 0;
	
	sprintf(szCfg, "Method:SetParameterValues\r\n");
	sprintf(szCfg+strlen(szCfg), "ParameterName:ZenoIntel.Device.OnPos.Pos.Cfg\r\n");
	sprintf(szCfg+strlen(szCfg), "FontColor:%d\r\n", pPosCfg->fontColor );
	sprintf(szCfg+strlen(szCfg), "FontSize:%d\r\n", pPosCfg->fontSize );
	sprintf(szCfg+strlen(szCfg), "FontX:%d\r\n", pPosCfg->fontX );
	sprintf(szCfg+strlen(szCfg), "FontY:%d\r\n", pPosCfg->fontY );
	sprintf(szCfg+strlen(szCfg), "FontShow:%d\r\n", pPosCfg->show );
	sprintf(szCfg+strlen(szCfg), "HoldTime:%d\r\n", pPosCfg->holdTime );
	sprintf(szCfg+strlen(szCfg), "POSAlarmServerAddr:%s\r\n", pPosCfg->posAlarmHostAddr );
	sprintf(szCfg+strlen(szCfg), "POSAlarmServerPort:%d\r\n", pPosCfg->posAlarmHostPort );
	sprintf(szCfg+strlen(szCfg), "\r\n");
	
	nRet = m_pManager->GetDecoderDevice().SysSetupInfo(lLoginID, AFK_REQUEST_SET_POS_CFG, (LPVOID)szCfg, waittime);
	if (nRet < 0 )
	{
		nRet = NET_ERROR_SET_POS_CFG;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	
	return NET_NOERROR;
}