
#include "StdAfx.h"
#include "RealPlay.h"
#include "Manager.h"
#include "RenderManager.h"
#include "DevConfigEx.h"
#include "./VideoRender.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CRealPlay::CRealPlay(CManager *pManager)
	: m_pManager(pManager)
{
	m_bIsOpen = FALSE;
}

CRealPlay::~CRealPlay()
{

}

int CRealPlay::Init()
{
	return Uninit();
}

int CRealPlay::Uninit()
{
	int nRet = NET_NOERROR;

	{
		m_csPTZ.Lock();
		
		list<st_PtzControl_Info*>::iterator it = m_lstPTZ.begin();
		for(; it != m_lstPTZ.end(); ++it)
		{
			if (*it)
			{
				int ret = StopPTZControlCommand((*it)->device, (*it)->controlparam.no);
				if (ret < 0)
				{
					nRet = NET_ERROR;
				}
				delete (*it);
			}
		}
		m_lstPTZ.clear();

		m_csPTZ.UnLock();

		if (m_bIsOpen)
		{
			m_timer.KillTimer();
			m_bIsOpen = FALSE;
		}
	}

	{
		m_csMonitors.Lock();
		
		list<st_Monitor_Info*>::iterator it = m_lstMonitors.begin();
		while(it != m_lstMonitors.end())
		{
			if (*it)
			{
				int ret = ProcessStopRealPlay(**it);
				if (ret >= 0)
				{
					delete (*it);
				}
				m_lstMonitors.erase(it++);
			}
			else
			{
				it++;
			}
		}
		
		m_csMonitors.UnLock();
	}
	
	return nRet;
}

int CRealPlay::CloseChannelOfDevice(afk_device_s* device)
{
	if (m_pManager->IsDeviceValid(device) < 0)
	{
		return -1;
	}

	int nRet = NET_NOERROR;
	
	{
		m_csPTZ.Lock();
		list<st_PtzControl_Info*>::iterator it = m_lstPTZ.begin();
		while(it != m_lstPTZ.end())
		{
			if ((*it) && (*it)->device == device)
			{
				delete (*it);
				m_lstPTZ.erase(it++);
			}
			else
			{
				++it;
			}
		}

		m_csPTZ.UnLock();
	}

	{
		m_csMonitors.Lock();
		list<st_Monitor_Info*>::iterator it = m_lstMonitors.begin();
		while(it != m_lstMonitors.end())
		{
			if ((*it) && (*it)->channel)
			{
				afk_device_s* _device = (afk_device_s*)((*it)->channel->get_device((*it)->channel));
				if (!_device)
				{
					nRet = -1;
					++it;
					continue;
				}
				else
				{
					if(_device == device)
					{
						int ret = ProcessStopRealPlay(**it);
						if (ret >= 0)
						{
							delete (*it);
						}
						m_lstMonitors.erase(it++);
					}
					else
					{
						++it;
					}
				}
			}
			else
			{
				++it;
			}
		}

		m_csMonitors.UnLock();
	}

	return nRet;
}

int __stdcall MonitorFunc(afk_handle_t object, unsigned char *data, unsigned int datalen, void *param, void *udata)
{
	st_Monitor_Info *pmoninfo = (st_Monitor_Info*)udata;
	if (pmoninfo == NULL)
	{
		return -1;
	}
	
    if (pmoninfo->Render)
    {
		BOOL bRet = pmoninfo->Render->Play(data, datalen);
    }

    if (pmoninfo->channel == object)
    {
        if (pmoninfo->pRealDataCallBack)
        {
            pmoninfo->pRealDataCallBack((LONG)object, 0, 
                data, datalen, pmoninfo->userdata);
        }

		if (pmoninfo->pRealDataCallBackEx)
		{
			if (pmoninfo->dwDataTypeFlag & 0x00000001) 
			{
				pmoninfo->pRealDataCallBackEx((LONG)object, 0,
					data, datalen, 0, pmoninfo->userdata);
			}
		}

        if (pmoninfo->file)
        {
            fwrite(data, datalen, 1, pmoninfo->file);
            pmoninfo->fileflushflag++;
            if (pmoninfo->fileflushflag%20 == 0)
            {
                fflush(pmoninfo->file);
            }
        }
    }

    return 1;
}

/*
 * 摘要：开始实时监视
 */
LONG CRealPlay::StartRealPlay(LONG lLoginID, int nChannelID, HWND hWnd, UINT subType)
{
    afk_device_s *device = (afk_device_s*)lLoginID;
    if (m_pManager->IsDeviceValid(device) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return 0;
    }

	/* 先申请建立会话 */
	afk_connect_param_t stuConnParam = {0};
	afk_login_device_type stuLoginType = {0};
	device->get_info(device, dit_login_device_flag, &stuLoginType);	// TCP、UDP或组播有登入时指定
	stuConnParam.nConnType = stuLoginType.nLoginMode;
	stuConnParam.nInterfaceType = INTERFACE_REALPLAY;
	int ret = m_pManager->GetDevConfigEx().SetupSession(lLoginID, nChannelID, &stuConnParam);
	if (ret < 0)
	{
		m_pManager->SetLastError(ret);
		return 0;
	}

	/* 准备解码资源 */
	CVideoRender *pRender = m_pManager->GetRenderManager().GetRender(hWnd);
	if (-1 == (int)pRender)
	{
		m_pManager->SetLastError(NET_RENDER_OPEN_ERROR);
		return 0; //系统出错
	}
	
	st_Monitor_Info* pmoninfo = NULL;
	afk_channel_s *channel	  = NULL;

	if (pRender)
	{
		if (pRender->StartDec(false) < 0)
		{
			m_pManager->SetLastError(NET_DEC_OPEN_ERROR);
			goto e_clearup;
		}
	}
	
	/* 建立视频交互通道 */
    pmoninfo = new st_Monitor_Info;
	if (pmoninfo == NULL)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}

    pmoninfo->channel = NULL;
    pmoninfo->channelid = nChannelID;
    pmoninfo->Render = pRender;
    pmoninfo->file = NULL;
    pmoninfo->pRealDataCallBack = NULL;
	pmoninfo->pRealDataCallBackEx = NULL;
	pmoninfo->dwDataTypeFlag = 0;
    pmoninfo->userdata = 0;
    pmoninfo->fileflushflag = 0;
	pmoninfo->bAudioPlay = FALSE;
	pmoninfo->nConnectID = stuConnParam.nConnectID;

    // 创建实时监视通道
    afk_media_channel_param_s parm;
    parm.no = nChannelID;
	parm.subtype = subType;
	parm.conn = stuConnParam;
    parm.base.func = MonitorFunc;
    parm.base.udata = (void*)pmoninfo;

    channel = (afk_channel_s*)device->open_channel(device, AFK_CHANNEL_TYPE_MEDIA, &parm);
    if (channel)
    {
		pmoninfo->channel = channel;
        if (pRender)
        {
            pRender->SetDrawCallBack((void*)m_pManager->GetDrawFunc(), 
                (void*)device, (void*)channel, (void*)m_pManager->GetDrawCallBackUserData());
			pRender->SetMonitorInfo(pmoninfo);
        }

		m_csMonitors.Lock();
		m_lstMonitors.push_back(pmoninfo);
		m_csMonitors.UnLock();
    }
    else
    {
		m_pManager->SetLastError(NET_OPEN_CHANNEL_ERROR);
		goto e_clearup;
    }

	return (LONG)channel;
	
e_clearup:
	
	if (pRender)
	{
		pRender->StopDec();
		m_pManager->GetRenderManager().ReleaseRender(pRender);
	}
	
	if (pmoninfo)
	{
		delete pmoninfo;
		pmoninfo = NULL;
	}

	return 0;
}

/*
 * 摘要：停止实时监视
 */
int CRealPlay::StopRealPlay(LONG lRealHandle)
{
	int nRet = NET_ERROR;

	m_csMonitors.Lock();
	list<st_Monitor_Info*>::iterator it = 
		find_if(m_lstMonitors.begin(),m_lstMonitors.end(),SearchMIbyHandle(lRealHandle));
	
	if (it != m_lstMonitors.end())
	{
		if ((*it))
		{
			//	停止数据回调，关闭通道
			nRet = ProcessStopRealPlay(**it);
			if (nRet >= 0)
			{
				delete (*it);
				m_lstMonitors.erase(it);
				nRet = NET_NOERROR;
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

	m_csMonitors.UnLock();
	return nRet;
}

/*
 *	调整图象播放的流畅性
 */
int CRealPlay::AdjustFluency(LONG lRealHandle, int nLevel)
{
	if(nLevel < 0 || nLevel > 6)
	{
        return NET_ERROR; 
	}

	int nRet = NET_NOERROR;

	m_csMonitors.Lock();

	list<st_Monitor_Info*>::iterator it = 
		find_if(m_lstMonitors.begin(),m_lstMonitors.end(),SearchMIbyHandle(lRealHandle));
	
	if (it != m_lstMonitors.end())
	{
		if ((*it))
		{
			if((*it)->Render)
			{
				BOOL bSuccess = (*it)->Render->AdjustFluency(nLevel);
				if(!bSuccess)
				{
					nRet = NET_ERROR;
				}
			}
			else
			{
				nRet = NET_SYSTEM_ERROR;
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

	m_csMonitors.UnLock();
	return nRet;
}

/*
 * 摘要：开始多画面预览
 */
LONG CRealPlay::StartMultiPlay(LONG lLoginID, int nChannelID, HWND hWnd, UINT subType/* = RType_Multiplay*/)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return 0;
    }

	int nRet = -1;
    afk_device_s *device = (afk_device_s*)lLoginID;
	int nMultWnd = 0;
	int parm1 = 0;
	int parm2 = 0;
	int channelCount = device->channelcount(device);

	if (nChannelID >= channelCount || nChannelID < 0)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return 0;
	}

	if (RType_Multiplay == subType)
	{
		parm1 = channelCount;
		parm2 = 0;
	}
	else if (RType_Multiplay_1 == subType)
	{
		parm1 = 1;
		parm2 = nChannelID;
	}
	else if (RType_Multiplay_4 == subType)
	{
		parm1 = 4;
		parm2 = nChannelID/4;
	}
	else if (RType_Multiplay_6 == subType)
	{
		parm1 = 6;
		parm2 = nChannelID/6;
	}
	else if (RType_Multiplay_8 == subType)
	{
		parm1 = 8;
		parm2 = nChannelID/8;
	}
	else if (RType_Multiplay_9 == subType)
	{
		parm1 = 9;
		parm2 = nChannelID/9;
	}
	else if (RType_Multiplay_12 == subType)
	{
		parm1 = 12;
		parm2 = nChannelID/12;
	}
	else if (RType_Multiplay_16 == subType)
	{
		parm1 = 16;
		parm2 = nChannelID/16;
	}
	else
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return 0;
	}

	/* 先申请建立会话 */
	afk_connect_param_t stuConnParam = {0};
	afk_login_device_type stuLoginType = {0};
	device->get_info(device, dit_login_device_flag, &stuLoginType);	// TCP、UDP或组播有登入时指定
	stuConnParam.nConnType = stuLoginType.nLoginMode;
	stuConnParam.nInterfaceType = INTERFACE_PREVIEW;
	int ret = m_pManager->GetDevConfigEx().SetupSession(lLoginID, channelCount, &stuConnParam);
	if (ret < 0)
	{
		m_pManager->SetLastError(ret);
		return 0;
	}

	/* 准备解码资源 */
	CVideoRender *pRender = m_pManager->GetRenderManager().GetRender(hWnd);
	if (-1 == (int)pRender)
	{
		m_pManager->SetLastError(NET_RENDER_OPEN_ERROR);
		return 0; //系统出错
	}

	st_Monitor_Info *pmoninfo = NULL;
	afk_channel_s *channel	  = NULL;

	if (pRender)
	{
		if (pRender->StartDec(false) < 0)
		{
			m_pManager->SetLastError(NET_DEC_OPEN_ERROR);
			goto e_clearup;
		}
	}
	
	/* 建立视频交互通道 */
    pmoninfo = new st_Monitor_Info;
	if (pmoninfo == NULL)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto e_clearup;
	}

    pmoninfo->channel = NULL;
    pmoninfo->channelid = nMultWnd;
    pmoninfo->Render = pRender;
    pmoninfo->file = NULL;
	pmoninfo->fileflushflag = 0;
    pmoninfo->pRealDataCallBack = NULL;
	pmoninfo->pRealDataCallBackEx = NULL;
	pmoninfo->dwDataTypeFlag = 0;
    pmoninfo->userdata = 0;
	pmoninfo->bAudioPlay = FALSE;
	pmoninfo->nConnectID = stuConnParam.nConnectID;

    // 创建多画面监视通道,只创建多画面数最多的
    afk_preview_channel_param_s parm;
    parm.parm1 = parm1;
	parm.parm2 = parm2;
	parm.conn = stuConnParam;
    parm.base.func = MonitorFunc;
    parm.base.udata = (void*)pmoninfo;

    channel = (afk_channel_s*)device->open_channel(device, AFK_CHANNEL_TYPE_PREVIEW, &parm);
    if (channel)
    {
		pmoninfo->channel = channel;
        if (pRender)
        {
            pRender->SetDrawCallBack((void*)m_pManager->GetDrawFunc(), 
                (void*)device, (void*)channel, (void*)m_pManager->GetDrawCallBackUserData());
			pRender->SetMonitorInfo(pmoninfo);
		}

		m_csMonitors.Lock();
		m_lstMonitors.push_back(pmoninfo);
		m_csMonitors.UnLock();
    }
    else
    {
		m_pManager->SetLastError(NET_OPEN_CHANNEL_ERROR);
		goto e_clearup;
    }

	return (LONG)channel;

e_clearup:
	if (pRender)
	{
		pRender->StopDec();
		m_pManager->GetRenderManager().ReleaseRender(pRender);
	}

	if (pmoninfo)
	{
		delete pmoninfo;
		pmoninfo = NULL;
	}

	return 0;
}

/*
 * 摘要：停止多画面预览
 */
int CRealPlay::StopMultiPlay(LONG lMultiPlay)
{
	int nRet = NET_NOERROR;

	m_csMonitors.Lock();
	list<st_Monitor_Info*>::iterator it = 
		find_if(m_lstMonitors.begin(),m_lstMonitors.end(),SearchMIbyHandle(lMultiPlay));

	if (it != m_lstMonitors.end())
	{
		if ((*it))
		{
			int ret = ProcessStopRealPlay(**it);
			if (ret >= 0)
			{
				delete (*it);
				m_lstMonitors.erase(it);
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

	m_csMonitors.UnLock();
	return nRet;
}

int CRealPlay::Decoder_OpenSound(LONG lRealHandle)
{
	int nRet = NET_NOERROR;
	
	m_csMonitors.Lock();
	list<st_Monitor_Info*>::iterator it = 
		find_if(m_lstMonitors.begin(),m_lstMonitors.end(),SearchMIbyHandle(lRealHandle));

	if (it != m_lstMonitors.end())
	{
        if ((*it) && (*it)->Render)
        {
            BOOL b = (*it)->Render->OpenAudio();
			if (b)
			{
				(*it)->bAudioPlay = TRUE;
				nRet = NET_NOERROR;
			}
			else
			{
				nRet = NET_RENDER_SOUND_ON_ERROR;
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

	m_csMonitors.UnLock();
	return nRet;
}

int	CRealPlay::Decoder_CloseSound()
{
	int nRet = NET_NO_AUDIO;

	m_csMonitors.Lock();
	list<st_Monitor_Info*>::iterator it = m_lstMonitors.begin();
	for(; it != m_lstMonitors.end(); ++it)
	{
		if ((*it) && (*it)->Render)
		{
			if ((*it)->bAudioPlay)
			{
				BOOL b = (*it)->Render->CloseAudio();
				if (b)
				{
					(*it)->bAudioPlay = FALSE;
					nRet = NET_NOERROR;
					break;
				}
				else
				{
					nRet = NET_RENDER_SOUND_OFF_ERROR;
				}
			}
		}
		else
		{
			nRet = NET_SYSTEM_ERROR;
		}
	}

	m_csMonitors.UnLock();
	return nRet;
}

int	CRealPlay::SetVolume(LONG lRealPlay, int nVolume)
{
	int nRet = NET_ERROR;

	m_csMonitors.Lock();
	list<st_Monitor_Info*>::iterator it = 
		find_if(m_lstMonitors.begin(),m_lstMonitors.end(),SearchMIbyHandle(lRealPlay));

	if (it != m_lstMonitors.end())
	{
        if ((*it) && (*it)->Render)
        {
            BOOL b = (*it)->Render->SetAudioVolume(nVolume);
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

	m_csMonitors.UnLock();
	return nRet;
}


int CRealPlay::GetDecoderVideoEffect(
									 LONG lRealHandle, unsigned char *brightness, 
									 unsigned char *contrast, unsigned char *hue, 
									 unsigned char *saturation)
{
	if (!brightness || !contrast || !hue || !saturation)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = NET_ERROR;

	*brightness = 0;
	*contrast = 0;
	*hue = 0;
	*saturation = 0;

	m_csMonitors.Lock();
	list<st_Monitor_Info*>::iterator it = 
		find_if(m_lstMonitors.begin(),m_lstMonitors.end(),SearchMIbyHandle(lRealHandle));

	if (it != m_lstMonitors.end())
	{
		if ((*it) && (*it)->Render)
		{
            (*it)->Render->GetColorParam(brightness, contrast, hue, saturation);
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

	m_csMonitors.UnLock();
	return nRet;
}

int CRealPlay::SetDecoderVideoEffect(
									 LONG lRealHandle, unsigned char brightness, 
									 unsigned char contrast, unsigned char hue, 
									 unsigned char saturation)
{
	int nRet = NET_ERROR;

	m_csMonitors.Lock();
	list<st_Monitor_Info*>::iterator it = 
		find_if(m_lstMonitors.begin(),m_lstMonitors.end(),SearchMIbyHandle(lRealHandle));

	if (it != m_lstMonitors.end())
	{
		if ((*it) && (*it)->Render)
		{
			BOOL b = (*it)->Render->AdjustColor(brightness, contrast, hue, saturation);
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

	m_csMonitors.UnLock();
	return nRet;
}

int	CRealPlay::StartSaveRealData(LONG lRealPlay, const char *pchFileName)
{
	if (NULL == pchFileName)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = NET_ERROR;

	m_csMonitors.Lock();
	list<st_Monitor_Info*>::iterator it = 
		find_if(m_lstMonitors.begin(),m_lstMonitors.end(),SearchMIbyHandle(lRealPlay));

	if (it != m_lstMonitors.end())
	{
        if ((*it)->file)
        {
            nRet = NET_REAL_ALREADY_SAVING;
        }
		else
		{
			(*it)->file = fopen(pchFileName, "wb");
			if ((*it)->file != 0)
			{
				(*it)->fileflushflag = 0;

//				SYSTEMTIME st;
//				GetLocalTime(&st);
//
//				afk_device_s* device = (afk_device_s*)(*it)->channel->get_device((*it)->channel);
//
//				WriteVideoFileHeader((*it)->file, device->device_type(device), 25, 
//					st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
				nRet = NET_NOERROR;
			}
			else
			{
				nRet = NET_OPEN_FILE_ERROR;
			}
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}

	m_csMonitors.UnLock();
	return nRet;
}

int CRealPlay::StopSaveRealData(LONG lRealPlay)
{
	int nRet = NET_ERROR;

	m_csMonitors.Lock();
	list<st_Monitor_Info*>::iterator it = 
		find_if(m_lstMonitors.begin(),m_lstMonitors.end(),SearchMIbyHandle(lRealPlay));

	if (it != m_lstMonitors.end())
	{
		if ((*it))
		{
			if ((*it)->file)
			{
				fclose((*it)->file);
				(*it)->file = 0;
				(*it)->fileflushflag = 0;
				nRet = NET_NOERROR;
			}
			else
			{
				nRet = NET_NOT_SAVING;
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

	m_csMonitors.UnLock();
	return nRet;
}

int CRealPlay::SetRealDataCallBack(LONG lRealPlay, fRealDataCallBack cbRealData, DWORD dwUser)
{
	int nRet = NET_ERROR;

	m_csMonitors.Lock();
	list<st_Monitor_Info*>::iterator it = 
		find_if(m_lstMonitors.begin(),m_lstMonitors.end(),SearchMIbyHandle(lRealPlay));

	if (it != m_lstMonitors.end())
	{
		if ((*it))
		{
			(*it)->userdata = dwUser;
			(*it)->pRealDataCallBack = cbRealData;
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

	m_csMonitors.UnLock();
	return nRet;
}

int CRealPlay::SetRealDataCallBackEx(LONG lRealPlay,fRealDataCallBackEx cbRealData, DWORD dwUser, DWORD dwFlag)
{
	int nRet = NET_ERROR;

	m_csMonitors.Lock();
	list<st_Monitor_Info*>::iterator it = 
		find_if(m_lstMonitors.begin(),m_lstMonitors.end(),SearchMIbyHandle(lRealPlay));

	if (it != m_lstMonitors.end())
	{
		if ((*it))
		{
			(*it)->dwDataTypeFlag = dwFlag;
			(*it)->userdata = dwUser;
			(*it)->pRealDataCallBackEx = cbRealData;
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

	m_csMonitors.UnLock();
	return nRet;
}

int	CRealPlay::CapturePicture(LONG lRealPlay, const char *pchPicFileName)
{
	if (NULL == pchPicFileName)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = NET_ERROR;

	m_csMonitors.Lock();
	list<st_Monitor_Info*>::iterator it = 
		find_if(m_lstMonitors.begin(),m_lstMonitors.end(),SearchMIbyHandle(lRealPlay));

	if (it != m_lstMonitors.end())
	{
		if ((*it) && (*it)->Render)
		{
			BOOL b = (*it)->Render->SnapPicture(pchPicFileName);
			if (b)
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

	m_csMonitors.UnLock();
	return nRet;
}

void CRealPlay::Process_ptzcontrol_proc()
{
	m_csPTZ.Lock();
	list<st_PtzControl_Info*>::iterator it = m_lstPTZ.begin();
	for(; it != m_lstPTZ.end(); ++it)
	{
        StartPTZControlCommand((*it)->device, (*it)->controlparam, FALSE, TRUE);
	}

	m_csPTZ.UnLock();
}

void CRealPlay::ptzcontrol_proc(LPVOID lpPara)
{
    try
    {
		CRealPlay* pThis = reinterpret_cast<CRealPlay*>(lpPara);
		if (pThis)
		{
			pThis->Process_ptzcontrol_proc();
		}
    }
    catch (...)
    {
#ifdef _DEBUG
		OutputDebugString("Error:云台控制定时器发生异常\n");
#endif
    }
}

int	CRealPlay::StartPTZControlCommand(afk_device_s *device, 
                             afk_control_channel_param_s controlparam, 
							 BOOL bOnce, BOOL bInTimer)
{
	if (m_pManager->IsDeviceValid(device) < 0)
	{
		return NET_INVALID_HANDLE;
	}

    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, AFK_CHANNEL_TYPE_CONTROL, &controlparam);
    if (NULL == pchannel)
    {
        return NET_OPEN_CHANNEL_ERROR;
    }
	
    pchannel->close(pchannel);

    if (bOnce)
    {
        return 0;
    }

	if (!bInTimer)
    {
		m_csPTZ.Lock();
		//如已存在该设备和通道号节点，则不需要再压入队列
		list<st_PtzControl_Info*>::iterator it = 
			find_if(m_lstPTZ.begin(),m_lstPTZ.end(),SearchPIbyDevice((LONG)device,controlparam.no));
		if (it == m_lstPTZ.end())
		{
			st_PtzControl_Info* ptzControlInfo = new st_PtzControl_Info;
			ptzControlInfo->device = device;
			ptzControlInfo->controlparam = controlparam;
			m_lstPTZ.push_back(ptzControlInfo);
			
			if (!m_bIsOpen)
			{
				BOOL bSuccess = m_timer.SetTimer(100, ptzcontrol_proc, (LPVOID)this);
				if (bSuccess)
				{
					m_bIsOpen = TRUE;
				}
				else
				{
#ifdef _DEBUG
					OutputDebugString("Error:启动云台控制定时器失败\n");
#endif
					m_csPTZ.UnLock();
					return NET_PTZ_SET_TIMER_ERROR;
				}
			}
		}
		else
		{
			st_PtzControl_Info* pPI = (*it);
			if (pPI->controlparam.type2 != controlparam.type2)
			{
				//不同操作类型，替换之
				m_lstPTZ.erase(it);
				delete pPI;
				st_PtzControl_Info* ptzControlInfo = new st_PtzControl_Info;
				ptzControlInfo->device = device;
				ptzControlInfo->controlparam = controlparam;
				m_lstPTZ.push_back(ptzControlInfo);
			}
			else
			{
				pPI->controlparam.parm1 = controlparam.parm1;
				pPI->controlparam.parm2 = controlparam.parm2;
				pPI->controlparam.parm3 = controlparam.parm3;
			}
		}

		m_csPTZ.UnLock();
    }

    return 0;
}

int	CRealPlay::StopPTZControlCommand(afk_device_s *device, int nChannelID)
{
	int nRet = NET_ERROR;

	m_csPTZ.Lock();
	list<st_PtzControl_Info*>::iterator it = 
		find_if(m_lstPTZ.begin(),m_lstPTZ.end(),SearchPIbyDevice((LONG)device,nChannelID));
	
	if (it != m_lstPTZ.end())
	{
		if ((*it))
		{
			delete (*it);
			m_lstPTZ.erase(it);
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

	m_csPTZ.UnLock();

	return nRet;
}

//本接口也改用亿蛙球机的统一协议
int CRealPlay::PTZControl(LONG lLoginID, int nChannelID, DWORD dwPTZCommand,DWORD dwStep, BOOL dwStop)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
    if (m_pManager->IsDeviceValid(device) < 0)
    {
        return NET_INVALID_HANDLE;
    }
	
	BOOL bOnce = TRUE;
    if (dwStop == FALSE)
    {
        afk_control_channel_param_s parm = {0};
        parm.no = nChannelID;
        parm.type1 = AFK_CONTROL_TYPE_YWPTZ; //AFK_CONTROL_TYPE_PTZ;
		parm.parm1 = 0;
        parm.parm2 = 0;
        parm.parm3 = 0;
        switch(dwPTZCommand)
        {
        case PTZ_UP_CONTROL:
            parm.type2 = PTZ_UP;
             break;
        case PTZ_DOWN_CONTROL:
            parm.type2 = PTZ_DOWN;
            break;
        case PTZ_LEFT_CONTROL:
            parm.type2 = PTZ_LEFT;
            break;
        case PTZ_RIGHT_CONTROL:
            parm.type2 = PTZ_RIGHT;
            break;
        case PTZ_ZOOM_ADD_CONTROL:
            parm.type2 = PTZ_ZOOM_ADD;
            break;
        case PTZ_ZOOM_DEC_CONTROL:
            parm.type2 = PTZ_ZOOM_DEC;
            break;
        case PTZ_FOCUS_ADD_CONTROL:
            parm.type2 = PTZ_FOCUS_ADD;
            break;
        case PTZ_FOCUS_DEC_CONTROL:
            parm.type2 = PTZ_FOCUS_DEC;
            break;
        case PTZ_APERTURE_ADD_CONTROL:
            parm.type2 = PTZ_APERTURE_ADD;
            break;
        case PTZ_APERTURE_DEC_CONTROL:
            parm.type2 = PTZ_APERTURE_DEC;
            break;
        case PTZ_POINT_MOVE_CONTROL:
            parm.type2 = PTZ_POINT_MOVE;
            break;
        case PTZ_POINT_SET_CONTROL:
            parm.type2 = PTZ_POINT_SET;
            break;
        case PTZ_POINT_DEL_CONTROL:
            parm.type2 = PTZ_POINT_DEL;
            break;
        case PTZ_POINT_LOOP_CONTROL:
            parm.type2 = PTZ_POINT_LOOP;
            break;
        case PTZ_LAMP_CONTROL:
            parm.type2 = PTZ_LAMP;
            break;
        default:
            return NET_ERROR;
        }
		if (dwPTZCommand <= PTZ_APERTURE_DEC_CONTROL && dwPTZCommand >= PTZ_UP_CONTROL)
		{
			bOnce = FALSE;
		}
		if(dwPTZCommand <= PTZ_POINT_DEL_CONTROL && dwPTZCommand >= PTZ_UP_CONTROL)
		{
			parm.parm2 = dwStep;
		}
		else
		{
			parm.parm1 = dwStep;
		}
        return StartPTZControlCommand((afk_device_s*)lLoginID, parm, bOnce, FALSE);
    }
    else
    {
        return StopPTZControlCommand((afk_device_s*)lLoginID, nChannelID);
    }

    return NET_ERROR;
}

int CRealPlay::YWPTZControl(LONG lLoginID, int nChannelID, DWORD dwPTZCommand, 
                                    unsigned char param1, unsigned char param2, unsigned char param3, 
                                    BOOL dwStop)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
    if (m_pManager->IsDeviceValid(device) < 0)
    {
        return NET_INVALID_HANDLE;
    }
	
	BOOL bOnce = TRUE;
    if (FALSE == dwStop)
    {
        afk_control_channel_param_s parm;
        parm.no = nChannelID;
        parm.type1 = AFK_CONTROL_TYPE_YWPTZ;

        switch(dwPTZCommand)
        {
        case PTZ_UP_CONTROL:
        case PTZ_DOWN_CONTROL:
        case PTZ_LEFT_CONTROL:
        case PTZ_RIGHT_CONTROL:
        case PTZ_ZOOM_ADD_CONTROL:
        case PTZ_ZOOM_DEC_CONTROL:
			parm.type2 = dwPTZCommand;
            break;
        case PTZ_FOCUS_ADD_CONTROL:
            parm.type2 = PTZ_FOCUS_ADD;
            break;
        case PTZ_FOCUS_DEC_CONTROL:
            parm.type2 = PTZ_FOCUS_DEC;
            break;
        case PTZ_APERTURE_ADD_CONTROL:
            parm.type2 = PTZ_APERTURE_ADD;
            break;
        case PTZ_APERTURE_DEC_CONTROL:
            parm.type2 = PTZ_APERTURE_DEC;
            break;
        case PTZ_POINT_MOVE_CONTROL:
            parm.type2 = PTZ_POINT_MOVE;
            break;
        case PTZ_POINT_SET_CONTROL:
            parm.type2 = PTZ_POINT_SET;
            break;
        case PTZ_POINT_DEL_CONTROL:
            parm.type2 = PTZ_POINT_DEL;
            break;
        case PTZ_POINT_LOOP_CONTROL:
            parm.type2 = PTZ_POINT_LOOP;
            break;
        case PTZ_LAMP_CONTROL:
            parm.type2 = PTZ_LAMP;
            break;
        default:
			if(dwPTZCommand >= EXTPTZ_TOTAL || dwPTZCommand == EXTPTZ_FASTGOTO)
			{
				return NET_ILLEGAL_PARAM;
			}
			else
			{
				parm.type2 = dwPTZCommand;
				break;
			}
        }

        parm.parm1 = param1;
        parm.parm2 = param2;
        parm.parm3 = param3;

		if ((dwPTZCommand <= PTZ_APERTURE_DEC_CONTROL && dwPTZCommand >= PTZ_UP_CONTROL)
			||(dwPTZCommand >= EXTPTZ_LEFTTOP && dwPTZCommand <= EXTPTZ_RIGHTDOWN)
			||(dwPTZCommand >= EXTPTZ_UP_TELE && dwPTZCommand <= EXTPTZ_RIGHTDOWN_WIDE))
		{
			bOnce = FALSE;
		}

        return StartPTZControlCommand((afk_device_s*)lLoginID, parm, bOnce, FALSE);
    }
    else
    {
		afk_control_channel_param_s parm;
		parm.no = nChannelID;
		parm.type1 = AFK_CONTROL_TYPE_YWPTZ;//表示云台控制
		parm.type2 = dwPTZCommand;
		parm.pParm4 = (char*)1;//1表示关闭 

		switch(dwPTZCommand)
        {
        case PTZ_UP_CONTROL:
        case PTZ_DOWN_CONTROL:
        case PTZ_LEFT_CONTROL:
        case PTZ_RIGHT_CONTROL:
        case PTZ_ZOOM_ADD_CONTROL:
        case PTZ_ZOOM_DEC_CONTROL:
			parm.type2 = dwPTZCommand;
            break;
        case PTZ_FOCUS_ADD_CONTROL:
            parm.type2 = PTZ_FOCUS_ADD;
            break;
        case PTZ_FOCUS_DEC_CONTROL:
            parm.type2 = PTZ_FOCUS_DEC;
            break;
        case PTZ_APERTURE_ADD_CONTROL:
            parm.type2 = PTZ_APERTURE_ADD;
            break;
        case PTZ_APERTURE_DEC_CONTROL:
            parm.type2 = PTZ_APERTURE_DEC;
            break;
        case PTZ_POINT_MOVE_CONTROL:
            parm.type2 = PTZ_POINT_MOVE;
            break;
        case PTZ_POINT_SET_CONTROL:
            parm.type2 = PTZ_POINT_SET;
            break;
        case PTZ_POINT_DEL_CONTROL:
            parm.type2 = PTZ_POINT_DEL;
            break;
        case PTZ_POINT_LOOP_CONTROL:
            parm.type2 = PTZ_POINT_LOOP;
            break;
        case PTZ_LAMP_CONTROL:
            parm.type2 = PTZ_LAMP;
            break;
        default:
			if(dwPTZCommand >= EXTPTZ_TOTAL || dwPTZCommand == EXTPTZ_FASTGOTO)
			{
				return NET_ILLEGAL_PARAM;
			}
			else
			{
				parm.type2 = dwPTZCommand;
				break;
			}
        }

		int nRet = StopPTZControlCommand((afk_device_s*)lLoginID, nChannelID);

		if ((dwPTZCommand <= PTZ_APERTURE_DEC_CONTROL && dwPTZCommand >= PTZ_UP_CONTROL)
			||(dwPTZCommand >= EXTPTZ_LEFTTOP && dwPTZCommand <= EXTPTZ_RIGHTDOWN)
			||(dwPTZCommand >= EXTPTZ_UP_TELE && dwPTZCommand <= EXTPTZ_RIGHTDOWN_WIDE))
		{
			// 后发送关闭命令
			afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, AFK_CHANNEL_TYPE_CONTROL, &parm);
			if (NULL == pchannel)
			{
				return NET_OPEN_CHANNEL_ERROR;
			}
			pchannel->close(pchannel);
		}

        return nRet;
    }

    return NET_ERROR;
}

int CRealPlay::YWPTZControlEx(LONG lLoginID, int nChannelID, DWORD dwPTZCommand, 
										   LONG lParam1, LONG lParam2, LONG lParam3, 
										   BOOL dwStop)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
    if (m_pManager->IsDeviceValid(device) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	if (dwPTZCommand != EXTPTZ_FASTGOTO)
	{
		return YWPTZControl(lLoginID, nChannelID, dwPTZCommand,
							lParam1, lParam2, lParam3, dwStop);
	}
	else
	{
		afk_control_channel_param_s parm;
        parm.no = nChannelID;
        parm.type1 = AFK_CONTROL_TYPE_YWPTZ;
		parm.type2 = dwPTZCommand;
		parm.parm1 = lParam1;
        parm.parm2 = lParam2;
        parm.parm3 = lParam3;

		return StartPTZControlCommand((afk_device_s*)lLoginID, parm, TRUE, FALSE);
	}
}

LONG CRealPlay::GetStatiscFlux(LONG lLoginID,LONG lPlayHandle)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
    if (m_pManager->IsDeviceValid(device) < 0)
    {
        return NET_INVALID_HANDLE;
    }
	
	LONG lRet = NET_NOERROR;

	m_csMonitors.Lock();
	list<st_Monitor_Info*>::iterator it = 
		find_if(m_lstMonitors.begin(),m_lstMonitors.end(),SearchMIbyHandle(lPlayHandle));

	if (it != m_lstMonitors.end())
	{
		if ((*it))
		{
			afk_channel_s *stat_channel = (afk_channel_s*)device->open_channel(device, 
				AFK_CHANNEL_TYPE_STATISC, 0);
			
			if (stat_channel)
			{
				lRet = stat_channel->get_info(stat_channel, 0, (*it)->channel);
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
			lRet = NET_SYSTEM_ERROR;
		}
	}
	else
	{
		lRet = NET_INVALID_HANDLE;
	}

	m_csMonitors.UnLock();
	return lRet;
}

int CRealPlay::ProcessStopRealPlay(st_Monitor_Info& moninfo)
{
	int nRet = NET_NOERROR;

	if (moninfo.channel) 
	{
		LONG lLoginID = (LONG)moninfo.channel->get_device(moninfo.channel);

		BOOL bSuccess = moninfo.channel->close(moninfo.channel);
		if (bSuccess)
		{
			moninfo.channel = NULL;
		}
		else
		{
			nRet = NET_CLOSE_CHANNEL_ERROR;
		}

		m_pManager->GetDevConfigEx().DestroySession(lLoginID, moninfo.nConnectID);
	}

	if (moninfo.Render)
	{
		int ret = moninfo.Render->StopDec();
		if (ret >= 0) 
		{
			moninfo.Render->SetDrawCallBack(NULL, NULL, NULL, NULL);
			m_pManager->GetRenderManager().ReleaseRender(moninfo.Render);
			moninfo.Render = NULL;
		}
		else
		{
			nRet = NET_DEC_CLOSE_ERROR;
		}
	}

    if (moninfo.file)
    {
        fclose(moninfo.file);
		moninfo.file = NULL;
    }

	return nRet;
}

/*
 *	获取帧率
 */
int CRealPlay::GetFrameRealPlay(LONG lPlayHandle, int *fileframerate, int *playframerate)
{
	if (!playframerate)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;

	*playframerate = -1;
	
	m_csMonitors.Lock();
	list<st_Monitor_Info*>::iterator it = 
		find_if(m_lstMonitors.begin(),m_lstMonitors.end(),SearchMIbyHandle(lPlayHandle));
	if (it != m_lstMonitors.end())
	{
		if ((*it) && (*it)->Render)
		{
			*playframerate = (*it)->Render->GetFrameRate();
			nRet = 0;
		}
		else
		{
			nRet = NET_INVALID_HANDLE;
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}
	
	m_csMonitors.UnLock();
	return nRet;
}





