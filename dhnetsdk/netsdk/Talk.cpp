
#include "Talk.h"
#include "RenderManager.h"
#include "VideoRender.h"
#include "DevConfig.h"

#ifdef WIN32

#include "Alaw_encoder.c"
#include "./depend/hisamr.h"

/*	$FXN :	CTalk()
==	======================================================================
==	$DSC :	构造函数，在其中对类的一些成员变量进行了初始化
==	$ARG :	
==		 :	
==	$RET :	
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
CTalk::CTalk(CManager *pManager)
	: m_pManager(pManager)
{
//	InitializeCriticalSection(&m_cs);

	m_ARStartSuc = FALSE;

	m_iEncodeFlag = 0;
	m_render = NULL;
	
	m_hisEncodeHandle = 0;
	m_hisDecodeHandle = 0;
}

/*	$FXN :	~CTalk()
==	======================================================================
==	$DSC :	析构函数，一些善后工作
==	$ARG :	
==		 :	
==	$RET :	
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
CTalk::~CTalk()
{
//	DeleteCriticalSection(&m_cs);
}

/*	$FXN :	Init()
==	======================================================================
==	$DSC :	模块工作前的初始化工作,初始化的工作可能会失败,所以要从构造函数中分离出来
==	$ARG :	
==		 :	
==	$RET :	0表示成功,非零失败
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
int CTalk::Init()
{
	{
		list<LPTALKHANDLEINFO>::iterator it = m_talkhandlelist.begin();
		for (; it != m_talkhandlelist.end(); it++)
		{
			if (*it)
			{
				if ((*it)->pTalkChannel)
				{
					(*it)->pTalkChannel->close((*it)->pTalkChannel);
				}
				delete (*it);
			}
		}
		m_talkhandlelist.clear();
	}
	
	{
		list<LPTALKBROADCASTINFO>::iterator it = m_broadcastlist.begin();
		for (; it != m_broadcastlist.end(); it++)
		{
			if (*it)
			{
				if ((*it)->pTalkChannel)
				{
					(*it)->pTalkChannel->close((*it)->pTalkChannel);
				}
				delete (*it);
			}
		}
		m_broadcastlist.clear();
	}
	
	
	//Begin: Modify by li_deming(11517) 2008-1-14	
	/*
	memset((void *)&m_AudioFormat, 0, sizeof(AUDIOFORMAT));
	m_AudioFormat.dwFrameLen = 1024*2;
		m_AudioFormat.nAudioBitrate = 64;
		m_AudioFormat.nAudioLayer = 5;
		m_AudioFormat.wChannels = 1;
		m_AudioFormat.dwSamplesPerSec = 8000;
		m_AudioFormat.wBitsPerSample = 8;*/
	m_FrameLength=2048;//1024*2
	//End:li_deming(11517)
	m_ARStartSuc = FALSE;

	m_dwAudioEncodeType = INTERNEL_ENCODE_TYPE_8K8BPCM;
	
	m_render = m_pManager->GetRenderManager().GetRender((HWND)0xFFFFFFFF);
	if (0 == (int)m_render || -1 == (int)m_render)
	{
		m_render = NULL;
	}

	if (0 == m_hisEncodeHandle) 
	{
		if (HI_SUCCESS != AMR_Encode_Init(&m_hisEncodeHandle, 0)) 
		{
			m_iEncodeFlag = 0;
			m_hisEncodeHandle = 0;
			m_pManager->SetLastError(NET_ILLEGAL_PARAM);
			return NET_ILLEGAL_PARAM;
		}
	}
/*
	其实在render_open之前一定要调用render_init，已经和render库的开发者约定好AUDIO_RENDER_INDEX为对讲模块所用
	if (render_open(AUDIO_RENDER_INDEX, 0, 0, 0, NULL, 0, ByDDOffscreen, 0) == 0)
	{
		m_irenderindex = AUDIO_RENDER_INDEX;
	}
*/
	m_iEncodeFlag = 0;

	return 0;
}

/*	$FXN :	UnInit()
==	======================================================================
==	$DSC :	模块功能不再使用时释放资源.
==	$ARG :	
==		 :	
==	$RET :	0表示成功,非零失败
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
int CTalk::UnInit()
{
//	EnterCriticalSection(&m_cs);
	m_cs.Lock();
	{
		list<LPTALKHANDLEINFO>::iterator it = m_talkhandlelist.begin();
		for (; it != m_talkhandlelist.end(); it++)
		{
			if (*it)
			{
				if ((*it)->pTalkChannel)
				{
					(*it)->pTalkChannel->close((*it)->pTalkChannel);
				}
				delete (*it);
			}
		}
		m_talkhandlelist.clear();
	}
	
	{
		list<LPTALKBROADCASTINFO>::iterator it = m_broadcastlist.begin();
		for (; it != m_broadcastlist.end(); it++)
		{
			if (*it)
			{
				if ((*it)->pTalkChannel)
				{
					(*it)->pTalkChannel->close((*it)->pTalkChannel);
				}
				delete (*it);
			}
		}
		m_broadcastlist.clear();
	}
	
//	LeaveCriticalSection(&m_cs);
	m_cs.UnLock();
		
	//Begin: Delete by li_deming(11517) 2008-1-14
	/*
	if (m_RecordHandle != NULL)
		{
			CloseAudioRecordEx(m_RecordHandle);
			m_RecordHandle = 0;
		}*/
	//End:li_deming(11517)
	
	if (TRUE == m_ARStartSuc)
	{		
		//Begin: Modify by li_deming(11517) 2008-1-14
		//UnInitAudioRecordEx();
		m_render->CloseAudioRecord();
		//End:li_deming(11517)
		m_ARStartSuc = FALSE;
	}

	if (m_render != NULL)
	{
		m_pManager->GetRenderManager().ReleaseRender(m_render);
		m_render = NULL;
	}

	if (m_hisEncodeHandle != 0) 
	{
		AMR_Encode_Exit(&m_hisEncodeHandle);
	}
/*
	if (AUDIO_RENDER_INDEX == m_irenderindex)
	{
		render_close(m_irenderindex);
		m_irenderindex = INVALID_RENDER_INDEX;
	}
*/
	
	return 0;
}

/*	$FXN :	StartTalk(LONG lLoginID, pfAudioDataCallBack pfcb, DWORD dwUser)
==	======================================================================
==	$DSC :	启动和设备的对讲，实现了和设备之间建立对讲通道
==	$ARG :	(in)lLoginID就是设备的句柄，(in)pfcb通过它把设备的数据和录音数据(如果在录音的话)回调给用户，
==		 :	(in)dwUser是用户自定义信息
==	$RET :	-1 or 0表示失败，否则返回一个注册的句柄，通过它调用下面的TalkSendData和StopTalk
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
LONG CTalk::StartTalk(LONG lLoginID, pfAudioDataCallBack pfcb, DWORD dwUser)
{
	afk_device_s *device = (afk_device_s *)lLoginID;

	int nTalkMode = 0;
	BOOL bServerMode = FALSE;
	AUDIO_ATTR_T stTalkEncodeType = {0};

	device->get_info(device, dit_talk_use_mode, &nTalkMode);
	bServerMode = nTalkMode==1? TRUE:FALSE;

	device->get_info(device, dit_talk_encode_type, &stTalkEncodeType);

	m_dwAudioEncodeType = INTERNEL_ENCODE_TYPE_8K8BPCM;
	m_FrameLength=1024;
	m_dwSampleRate = 8000;
	m_nAudioBit = 8;
	
	if (stTalkEncodeType.iAudioType == 0xFFFF)	// 为了兼容性，默认方式就为PCM
	{
		stTalkEncodeType.iAudioType = 0;
		int	bufLen = sizeof(DIALOG_CAPS)+16*sizeof(AUDIO_ATTR_T);
		char* capBuf = new char[bufLen];
		if (NULL == capBuf)
		{
			return NET_SYSTEM_ERROR;
		}
		
		int retlen = 0;
		int iRet = m_pManager->GetDevConfig().QuerySystemInfo(lLoginID, SYSTEM_INFO_TALK_ATTR, capBuf, bufLen, &retlen, QUERY_WAIT);
		if (iRet >= 0 && retlen >= sizeof(DIALOG_CAPS) && ((retlen-sizeof(DIALOG_CAPS))%sizeof(AUDIO_ATTR_T))==0) 
		{
			DIALOG_CAPS* pDialogCap = (DIALOG_CAPS*)capBuf;
			if (0 == pDialogCap->iAudioTypeNum)
			{
				delete[] capBuf;
				return NET_UNSUPPORTED;
			}
			AUDIO_ATTR_T* pAAT = NULL;
			for (int i = 0; i < pDialogCap->iAudioTypeNum; i++)
			{
				pAAT = (AUDIO_ATTR_T*)(capBuf+sizeof(DIALOG_CAPS)+i*sizeof(AUDIO_ATTR_T));
				
				if(1==pAAT->iAudioType && m_dwAudioEncodeType<INTERNEL_ENCODE_TYPE_8K16BNEWPCM)
				{
					//设备支持带头信息的PCM 8K 16 Bit
					m_dwAudioEncodeType = INTERNEL_ENCODE_TYPE_8K16BNEWPCM;
					m_FrameLength=1024;
					m_dwSampleRate = pAAT->dwSampleRate;
					m_nAudioBit = pAAT->iAudioBit;
				}
			}
		}
		
		if (capBuf != NULL)
		{
			delete[] capBuf;
			capBuf = NULL;
		}
	}
	else if (stTalkEncodeType.iAudioType == 0)
	{
		m_dwAudioEncodeType = INTERNEL_ENCODE_TYPE_8K8BPCM;
		m_FrameLength=1024;
		m_dwSampleRate = stTalkEncodeType.dwSampleRate;
		m_nAudioBit = stTalkEncodeType.iAudioBit;
	}
	else if (stTalkEncodeType.iAudioType == 1)
	{
		m_dwAudioEncodeType = INTERNEL_ENCODE_TYPE_8K16BNEWPCM;
		m_FrameLength=1024;
		m_dwSampleRate = stTalkEncodeType.dwSampleRate;
		m_nAudioBit = stTalkEncodeType.iAudioBit;
	}
	else if (stTalkEncodeType.iAudioType == 2)
	{
		m_dwAudioEncodeType = INTERNEL_ENCODE_TYPE_G711A8K16B;
		m_FrameLength=1280;
		m_dwSampleRate = stTalkEncodeType.dwSampleRate;
		m_nAudioBit = stTalkEncodeType.iAudioBit;
	}
	else if (stTalkEncodeType.iAudioType == 3)
	{
		m_dwAudioEncodeType = INTERNEL_ENCODE_TYPE_AMR8K16B;
		m_FrameLength=320;
		m_dwSampleRate = stTalkEncodeType.dwSampleRate;
		m_nAudioBit = stTalkEncodeType.iAudioBit;
	}
	else if (stTalkEncodeType.iAudioType == 4)
	{
		m_dwAudioEncodeType = INTERNEL_ENCODE_TYPE_G711U8K16B;
		m_FrameLength=320;
		m_dwSampleRate = stTalkEncodeType.dwSampleRate;
		m_nAudioBit = stTalkEncodeType.iAudioBit;
	}
	else if (stTalkEncodeType.iAudioType == 5)
	{
		m_dwAudioEncodeType = INTERNEL_ENCODE_TYPE_G7268K16B;
		m_FrameLength=320;
		m_dwSampleRate = stTalkEncodeType.dwSampleRate;
		m_nAudioBit = stTalkEncodeType.iAudioBit;
	}


	if (NULL == m_render) 
	{
		return NET_NO_TALK_CHANNEL;
	}

	if (!bServerMode)
	{
		if (m_render->StartDec(FALSE, 
			(INTERNEL_ENCODE_TYPE_8K16BNEWPCM==m_dwAudioEncodeType || INTERNEL_ENCODE_TYPE_G711A8K16B==m_dwAudioEncodeType || INTERNEL_ENCODE_TYPE_AMR8K16B==m_dwAudioEncodeType || INTERNEL_ENCODE_TYPE_G711U8K16B==m_dwAudioEncodeType || INTERNEL_ENCODE_TYPE_G7268K16B==m_dwAudioEncodeType) ? 2 : 1) < 0)
		{
			return NET_RENDER_SOUND_ON_ERROR;
		}
	}
	
	m_cs.Lock();

	LONG lRet = 0;
	list<LPTALKHANDLEINFO>::iterator it = m_talkhandlelist.begin();
	for (;it != m_talkhandlelist.end(); it++)
	{
		if ((*it)->lDevHandle == lLoginID)
		{
			m_cs.UnLock();
			return 0;
		}
	}

	afk_talk_channel_param_s talkchannelpar;
	LPTALKHANDLEINFO pstTalkInfo = NULL;
	pstTalkInfo = new TALKHANDLEINFO;
	if (NULL == pstTalkInfo)
	{
		lRet = NET_SYSTEM_ERROR;
		goto FAILED;
	}
	memset((void *)pstTalkInfo, 0, sizeof(TALKHANDLEINFO));
	memset((void *)&talkchannelpar, 0, sizeof(afk_channel_param_s));
	talkchannelpar.base.udata = (void *)pstTalkInfo;
	talkchannelpar.base.func = CTalk::TalkFunc;
	talkchannelpar.no = pstTalkInfo->iChannelId;
	talkchannelpar.nEncodeType = stTalkEncodeType.iAudioType==0?1:stTalkEncodeType.iAudioType;
	talkchannelpar.nAudioBit = stTalkEncodeType.iAudioBit;
	talkchannelpar.dwSampleRate = stTalkEncodeType.dwSampleRate;
	
	pstTalkInfo->pTalkChannel = (afk_channel_s *)device->open_channel(device, AFK_CHANNEL_TYPE_TALK, &talkchannelpar);
	if (NULL == pstTalkInfo->pTalkChannel)
	{
		lRet = NET_OPEN_CHANNEL_ERROR;
		goto FAILED;
	}
	pstTalkInfo->lDevHandle = lLoginID;
	pstTalkInfo->pfcb = pfcb;
	pstTalkInfo->dwUser = dwUser;
	m_talkhandlelist.push_back(pstTalkInfo);
	lRet = (LONG)pstTalkInfo;
	
	goto END;

FAILED:
	if (pstTalkInfo != NULL)
	{
		if (pstTalkInfo->pTalkChannel)
		{
			pstTalkInfo->pTalkChannel->close(pstTalkInfo->pTalkChannel);
		}
		delete pstTalkInfo;
	}

	if (!bServerMode)
	{
		if (m_render != NULL)
		{
			m_render->CloseAudio();
			m_render->StopDec();
		}
	}

END:
	m_cs.UnLock();
	return lRet;
}

/*	$FXN :	SetVolume(LONG lTalkHandle, WORD wVolume)
==	======================================================================
==	$DSC :	设置语音对讲的音量，这里的lTalkHandle是通过上面的StartTalk得来的。
==	$ARG :	(in)lTalkHandle是权限凭证，是上面的StartTalk的返回值，
==		 :	(in)wVolume是要设置的音量值，范围: 0 -- 0xFFFF, 0 音量最小;
==	$RET :	成功返回TRUE，否则返回FALSE
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
BOOL CTalk::SetVolume(LONG lTalkHandle, WORD wVolume)
{
//	EnterCriticalSection(&m_cs);
	m_cs.Lock();

	BOOL bRet = FALSE;
	list<LPTALKHANDLEINFO>::iterator it = m_talkhandlelist.begin();
	for (; it != m_talkhandlelist.end(); it++)
	{
		if (lTalkHandle == (LONG)(*it))
		{
			break;
		}
	}
	if (it == m_talkhandlelist.end())
	{
		m_pManager->SetLastError(NET_INVALID_HANDLE);
		bRet = FALSE;
		goto END;
	}
	
	if (NULL == m_render) 
	{
		m_pManager->SetLastError(NET_NO_TALK_CHANNEL);
		bRet = FALSE;
		goto END;
	}

	bRet = m_render->SetAudioVolume(wVolume);
	if (!bRet)
	{
		m_pManager->SetLastError(NET_RENDER_SET_VOLUME_ERROR);
		goto END;
	}
	/*
	if (m_irenderindex != AUDIO_RENDER_INDEX)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		bRet = FALSE;
		goto END;
	}
	*/
	/*
	DWORD dwVol;
	dwVol = ((0xFFFF*wVolume)/100 << 16 | (0xFFFF*wVolume)/100);
    if (render_set_volume(m_irenderindex, dwVol) >= 0)
	{
		bRet = TRUE;
		goto END;
	}
	*/
	//m_pManager->SetLastError(NET_ERROR);

END:
//	LeaveCriticalSection(&m_cs);
	m_cs.UnLock();
	return bRet;
}

/*	$FXN :	SendData2Dev(LONG lchannel, char *pSendBuf, DWORD dwBufSize)
==	======================================================================
==	$DSC :	把pSendBuf,通过lchannel发送给设备，设计这个函数是为了局部功能的模块化。
==	$ARG :	(in)lchannel是语音对讲的通道句柄，(in)pSendBuf是要发送的数据地址，(in)dwBufSize是要发送的数据长度
==		 :	
==	$RET :	成功返回发送的数据长度，失败返回-1 or 0
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
LONG CTalk::SendData2Dev(afk_channel_s *pChannel, char *pSendBuf, DWORD dwBufSize)
{
	LONG lRet = 0;
	if (NULL == pChannel || NULL == pSendBuf || 0 == dwBufSize)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return -1;
	}
	struct afk_talk_info_s tempst;
	tempst.data = pSendBuf;
	tempst.datalen = dwBufSize;
	if (1 == pChannel->set_info(pChannel, 0, &tempst))
	{
		lRet = dwBufSize;
	}

	return lRet;
}

/*	$FXN :	TalkSendData(LONG lTalkHandle, char *pSendBuf, DWORD dwBufSize)
==	======================================================================
==	$DSC :	接受用户的请求，要发送数据到设备。
==	$ARG :	(in)lTalkHandle是上面注册的语音对讲的句柄，(in)pSendBuf是要发送的数据地址，(in)dwBufSize是要发送的数据长度
==		 :	
==	$RET :	成功返回发送的数据长度，失败返回-1 or 0
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
LONG CTalk::TalkSendData(LONG lTalkHandle, char *pSendBuf, DWORD dwBufSize)
{
	m_cs.Lock();

	LONG lRet = 0;
	list<LPTALKHANDLEINFO>::iterator it = m_talkhandlelist.begin();
	for (; it != m_talkhandlelist.end(); it++)
	{
		if ((LONG)(*it) == lTalkHandle)
		{
			break;
		}
	}
	if (it == m_talkhandlelist.end() || NULL == (*it))
	{
		m_pManager->SetLastError(NET_INVALID_HANDLE);
		lRet = -1;
		goto END;
	}
	lRet = SendData2Dev((*it)->pTalkChannel, pSendBuf, dwBufSize);

END:
	m_cs.UnLock();
	
	return lRet;
}

/*	$FXN :	RecordStart()
==	======================================================================
==	$DSC :	启动本地录音功能
==	$ARG :	
==		 :	
==	$RET :	成功返回TRUE,失败返回FALSE
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
BOOL CTalk::RecordStart()
{
//	EnterCriticalSection(&m_cs);
	m_cs.Lock();
	//Begin: Modify by li_deming(11517) 2008-1-14
	
	/*
	if (FALSE == m_ARStartSuc) 
		{
			if (InitAudioRecordEx(m_AudioFormat) == 1)
			{
				m_ARStartSuc = TRUE;
			}
		}
	
		BOOL bRet = FALSE;
		if (FALSE == m_ARStartSuc)
		{
			m_pManager->SetLastError(NET_TALK_INIT_ERROR);
			bRet = FALSE;
			goto END;
		}
		if (NULL == m_RecordHandle)
		{
			m_RecordHandle = OpenAudioRecordEx(CTalk::RecordFunc, (int)this);
			if (m_RecordHandle != NULL)
			{
				bRet = TRUE;
				goto END;
			}
		}
		else
		{
			bRet = TRUE;
			goto END;
		}
		
		m_pManager->SetLastError(NET_ERROR);*/
	
	BOOL bRet = FALSE;
	m_ARStartSuc=FALSE;
	bRet=m_render->OpenAudioRecord(CTalk::RecordFunc,m_nAudioBit,m_dwSampleRate,m_FrameLength,0,(long)this);	
	if(TRUE==bRet)
	{
		m_ARStartSuc=TRUE;
		goto END;
	}	
	m_pManager->SetLastError(NET_ERROR);
	//End:li_deming(11517)	
END:

//	LeaveCriticalSection(&m_cs);
	m_cs.UnLock();
	return bRet;
}

/*	$FXN :	RecordStop()
==	======================================================================
==	$DSC :	停止本地录音功能
==	$ARG :	
==		 :	
==	$RET :	成功返回TRUE,失败返回FALSE
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
BOOL CTalk::RecordStop()
{
//	EnterCriticalSection(&m_cs);
	m_cs.Lock();

	BOOL bRet = FALSE;
	//Begin: Modify by li_deming(11517) 2008-1-14
	/*	
		if (FALSE == m_ARStartSuc)
		{
			m_pManager->SetLastError(NET_TALK_INIT_ERROR);
			bRet = FALSE;
			goto END;
		}
		if (NULL == m_RecordHandle)
		{
			m_pManager->SetLastError(NET_ERROR);
			bRet = FALSE;
		}
		else
		{
			if (CloseAudioRecordEx(m_RecordHandle) != 1)
			{
				m_pManager->SetLastError(NET_ERROR);
				bRet = FALSE;
				goto END;
			}
			m_RecordHandle = NULL;
			bRet = TRUE;
		}
		
	END:
		if (TRUE == m_ARStartSuc)
		{
			UnInitAudioRecordEx();
			m_ARStartSuc = FALSE;
		}*/
	
	//End:li_deming(11517)	
//	LeaveCriticalSection(&m_cs);
	if(TRUE==m_ARStartSuc)
	{
		bRet=m_render->CloseAudioRecord();
	}
	m_cs.UnLock();
	return bRet;
}

/*	$FXN :	AudioDec(char *pAudioDataBuf, DWORD dwBufSize)
==	======================================================================
==	$DSC :	实现音频数据解码
==	$ARG :	(in)pAudioDataBuf是要解码的音频数据，(in)dwBufSize是音频数据的长度
==		 :	
==	$RET :	
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
void CTalk::AudioDec(char *pAudioDataBuf, DWORD dwBufSize)
{
	if (NULL == m_render)
	{
		return;
	}

	/*
	static FILE* f = NULL;
	if (f == NULL)
	{
		f = fopen("e:\\savetalk.dav", "wb");
		if (f == NULL)
		{
			DWORD dw = GetLastError();
			OutputDebugString("damen!\n");
		}
	}
	if (f != NULL)
	{
		fwrite(pAudioDataBuf, 1, dwBufSize, f);
	}
*/
	if (INTERNEL_ENCODE_TYPE_8K8BPCM == m_dwAudioEncodeType)
	{
		for (int i = 0; i < dwBufSize; i++)
		{
			pAudioDataBuf[i] += 128;
		}
	}
/*
	//test
	static FILE* f = NULL;
	if (f == NULL)
	{
		f = fopen("e:\\savetalk.pcm", "wb");
		if (f == NULL)
		{
			DWORD dw = GetLastError();
			OutputDebugString("damen!\n");
		}
	}
	if (f != NULL)
	{
		fwrite(pAudioDataBuf, 1, dwBufSize, f);
	}
*/

	m_render->Play((BYTE*)pAudioDataBuf, dwBufSize);
	/*
	if (AUDIO_RENDER_INDEX == m_irenderindex)
	{
		render_audio(m_irenderindex, (unsigned char *)pAudioDataBuf, dwBufSize, 8, 8000);
	}
	*/
}

/*	$FXN :	StopTalk(LONG lTalkHandle)
==	======================================================================
==	$DSC :	停止和设备之间的对讲功能
==	$ARG :	(in)lTalkHandle是启动对讲时得到的对讲句柄
==		 :	
==	$RET :	成功返回TRUE，否则返回FALSE
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
BOOL CTalk::StopTalk(LONG lTalkHandle)
{
	m_cs.Lock();
	
	BOOL bRet = TRUE;
	list<LPTALKHANDLEINFO>::iterator it = m_talkhandlelist.begin();
	for (; it != m_talkhandlelist.end(); it++)
	{
		if ((LONG)(*it) == lTalkHandle)
		{
			break;
		}
	}
	if (it == m_talkhandlelist.end())
	{
		bRet = FALSE;
		m_pManager->SetLastError(NET_ERROR);
		goto END;
	}
	if (*it)
	{
		if ((*it)->pTalkChannel)
		{
			(*it)->pTalkChannel->close((*it)->pTalkChannel);
		}
		delete (*it);
	}
	m_talkhandlelist.erase(it);

	if (m_render != NULL)
	{
		if (m_render->StopDec() < 0)
		{
#ifdef DEBUG
			OutputDebugString("stop dec while stop talk failed!\n");
#endif
		}

		if (!m_render->CloseAudio())
		{
#ifdef DEBUG
			OutputDebugString("stop dec while stop talk failed!\n");
#endif
		}
	}
	
END:
	m_cs.UnLock();
	return bRet;
}

/*	$FXN :	AddDev(LONG lLoginID)
==	======================================================================
==	$DSC :	把设备增加到广播组，在本地采集到的数据，模块会自动地发送给广播组中的设备
==	$ARG :	(in)lLoginID是设备的句柄
==		 :	
==	$RET :	成功返回TRUE，否则返回FALSE
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
BOOL CTalk::BroadcastAddDev(LONG lLoginID)
{
	if (m_pManager->IsDeviceValid((afk_device_s *)lLoginID) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return FALSE;
    }
	
//	EnterCriticalSection(&m_cs);
	m_cs.Lock();

	list<LPTALKBROADCASTINFO>::iterator it = m_broadcastlist.begin();
	for (; it != m_broadcastlist.end(); it++)
	{
		if ((*it)->lDevHandle == lLoginID)
		{
//			LeaveCriticalSection(&m_cs);
			m_cs.UnLock();
			m_pManager->SetLastError(NET_ERROR);
			return FALSE;
		}
	}

	BOOL bRet = FALSE;
	afk_device_s *device = (afk_device_s *)lLoginID;
	afk_talk_channel_param_s talkchannelpar;
	LPTALKBROADCASTINFO ptmpinfo = NULL;
	ptmpinfo = new TALKBROADCASTINFO;
	if (NULL == ptmpinfo)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto FAILED;
	}
	memset((void *)ptmpinfo, 0, sizeof(TALKBROADCASTINFO));
	memset((void *)&talkchannelpar, 0, sizeof(afk_talk_channel_param_s));
	talkchannelpar.base.udata = (void *)ptmpinfo;
	talkchannelpar.base.func = NULL;
	talkchannelpar.no = ptmpinfo->iChannelId;
	ptmpinfo->pTalkChannel = (afk_channel_s *)device->open_channel(device, AFK_CHANNEL_TYPE_TALK, &talkchannelpar);
	if (NULL == ptmpinfo->pTalkChannel)
	{
		m_pManager->SetLastError(NET_OPEN_CHANNEL_ERROR);
		goto FAILED;
	}
	ptmpinfo->lDevHandle = lLoginID;
	m_broadcastlist.push_back(ptmpinfo);
	bRet = TRUE;
	goto END;

FAILED:
	bRet = FALSE;
	if (ptmpinfo != NULL)
	{
		if (ptmpinfo->pTalkChannel)
		{
			ptmpinfo->pTalkChannel->close(ptmpinfo->pTalkChannel);
		}
		delete ptmpinfo;
	}
END:
//	LeaveCriticalSection(&m_cs);
	m_cs.UnLock();
	return bRet;
}

/*	$FXN :	DelDev(LONG lLoginID)
==	======================================================================
==	$DSC :	和AddDev(LONG lLoginID)是相对的功能
==	$ARG :	(in)lLoginID是设备的句柄
==		 :	
==	$RET :	成功返回TRUE，否则返回FALSE
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
BOOL CTalk::BroadcastDelDev(LONG lLoginID)
{
//	EnterCriticalSection(&m_cs);
	m_cs.Lock();
	
	BOOL bRet = FALSE;
	list<LPTALKBROADCASTINFO>::iterator it = m_broadcastlist.begin();
	for (; it != m_broadcastlist.end(); it++)
	{
		if ((*it) && (*it)->lDevHandle == lLoginID)
		{
			if ((*it)->pTalkChannel)
			{
				(*it)->pTalkChannel->close((*it)->pTalkChannel);
			}
			delete (*it);
			m_broadcastlist.erase(it);
			break;
		}
	}
	if (m_broadcastlist.end() == it)
	{
		m_pManager->SetLastError(NET_ERROR);
		bRet = FALSE;
	}
//	LeaveCriticalSection(&m_cs);
	m_cs.UnLock();
	return bRet;
}

/*	$FXN :	InitAudioEncode(AUDIO_FORMAT aft)
==	======================================================================
==	$DSC :	音频编码初始化(标准格式->亿蛙格式)
==	$ARG :	[in]aft，表示的是要编码的原始数据的音频格式；
==		 :	
==	$RET :	成功返回0,失败返回错误码
==	$MOD :	2007-03-20		yehao(10857)		Create the Function
==	======================================================================
*/
int CTalk::InitAudioEncode(AUDIO_FORMAT aft)
{
	if (aft.byFormatTag != 0 || aft.nChannels != 1 || aft.wBitsPerSample != 16 || aft.nSamplesPerSec != 8000)
	{
		m_iEncodeFlag = 0;
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return NET_ILLEGAL_PARAM;
	}

	m_iEncodeFlag = 1;
	return 0;
}

/*	$FXN :	AudioEncode(LONG lHandle, BYTE *lpInBuf, DWORD *lpInLen, BYTE *lpOutBuf, DWORD *lpOutLen)
==	======================================================================
==	$DSC :	进行音频数据编码
==	$ARG :	[in]lHandle，开启对讲的句柄；//目前没有用
==     	 :	[in]lpInBuf，要进行编码的源数据；
==     	 :	[in,out]lpInBuf，调用时输入的是lpInBuf的数据长度，调用返回时是成功编码的数据长度；
==     	 :	[out]lpOutBuf，用来存储编码后的音频数据；
==     	 :	[in,out]lpOutLen，调用时表述lpOutBuf的最大长度，如果音频缓冲长度不足而失败时lpOutLen表示要求的数据长度，成功返回时表示lpOutBuf有效数据长度；
==		 :	
==	$RET :	成功返回0,失败返回错误码
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
int	CTalk::AudioEncode(LONG lHandle, BYTE *lpInBuf, DWORD *lpInLen, BYTE *lpOutBuf, DWORD *lpOutLen)
{
	if (0 == m_iEncodeFlag)
	{
		m_pManager->SetLastError(NET_AUDIOENCODE_NOTINIT);
		return NET_AUDIOENCODE_NOTINIT;
	}
	if (NULL == lpInBuf || NULL == lpInLen || NULL == lpOutBuf || NULL == lpOutLen || 0 == *lpInLen)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return NET_ILLEGAL_PARAM;
	}
	if (*lpInLen >= MAX_ENCODE_LEN*2)
	{
		*lpInLen = MAX_ENCODE_LEN*2;
		m_pManager->SetLastError(NET_DATA_TOOLONGH);
		return NET_DATA_TOOLONGH;
	}
	if (*lpOutLen < (*lpInLen/2 + 8))
	{
		*lpOutLen = *lpInLen/2 + 8;
		m_pManager->SetLastError(NET_INSUFFICIENT_BUFFER);
		return NET_INSUFFICIENT_BUFFER;
	}
	
	int iRet = 0;
	int iCount = 0;
	int iDataLen = (*lpInLen>>1);
	unsigned short *p16 = (unsigned short *)lpInBuf;
	lpOutBuf[iCount++] = 0x00;
	lpOutBuf[iCount++] = 0x00;
	lpOutBuf[iCount++] = 0x01;
	lpOutBuf[iCount++] = 0xf0;
	lpOutBuf[iCount++] = 0x07;
	lpOutBuf[iCount++] = 0x02;
	*(unsigned short *)(lpOutBuf+iCount) = (unsigned short)iDataLen;
	iCount += 2;
	for (int i = 0; i < iDataLen; i++)
	{
		lpOutBuf[iCount++] = BYTE(*p16>>8);
		p16++;
	}
	*lpOutLen = iCount;	
	*lpInLen = 0;
	
	return iRet;
}

/*	$FXN :	ReleaseAudioEncode(LONG lHandle)
==	======================================================================
==	$DSC :	音频解码完成退出
==	$ARG :	
==		 :	
==	$RET :	成功返回0,失败返回非零的错误码
==	$MOD :	2007-03-20		yehao(10857)		Create the Function
==	======================================================================
*/
int CTalk::ReleaseAudioEncode()
{
//	EnterCriticalSection(&m_cs);
	m_cs.Lock();
	m_iEncodeFlag = 0;
//	LeaveCriticalSection(&m_cs);
	m_cs.UnLock();
	return 0;
}

/*	$FXN :	InitAudioDecode(AUDIO_FORMAT aft)
==	======================================================================
==	$DSC :	音频解码的初始化(亿蛙格式->标准格式)
==	$ARG :	[in]aft，要转换的目标音频格式；
==		 :	
==	$RET :	成功返回有效句柄,失败返回无效句柄
==	$MOD :	2007-03-20		yehao(10857)		Create the Function
==	======================================================================
*/
LONG CTalk::InitAudioDecode(AUDIO_FORMAT aft)
{
	LONG lRet = (LONG)INVALID_HANDLE_VALUE;

	return lRet;
}

/*	$FXN :	AudioDecode(LONG lHandle, BYTE *lpInBuf, DWORD *lpInLen, BYTE *lpOutBuf, DWORD *lpOutLen)
==	======================================================================
==	$DSC :	进行数据解码
==	$ARG :	[in]lHandle，其值就是上面的初始化的返回值；
==     	 :	[in]lpInBuf，要求解码的原音频数据；
==		 :	[in,out]lpInLen，调用时表示lpInBuf的长度，返回时其值是没有完成解码的数据长度；
==		 :	[in]lpOutBuf，用来存放解码后的音频数据；
==		 :	[in,out]lpOutLen，调用时表示lpOutBuf的缓冲最大长度，如果音频缓冲长度不足而失败时lpOutLen表示要求的数据长度，成功返回时表示lpOutBuf有效数据长度；
==		 :	
==	$RET :	成功返回0,失败返回非零的错误码
==	$MOD :	2007-03-20		yehao(10857)		Create the Function
==	======================================================================
*/
int CTalk::AudioDecode(LONG lHandle, BYTE *lpInBuf, DWORD *lpInLen, BYTE *lpOutBuf, DWORD *lpOutLen)
{
	int iRet = 0;

	return iRet;
}

/*	$FXN :	ReleaseAudioDecode(LONG lHandle)
==	======================================================================
==	$DSC :	完成解码，退出
==	$ARG :	[in]lHandle，上面的初始化接口的返回值；
==		 :	
==	$RET :	成功返回0,失败返回非零的错误码
==	$MOD :	2007-03-20		yehao(10857)		Create the Function
==	======================================================================
*/
int CTalk::ReleaseAudioDecode(LONG lHandle)
{
	int iRet = 0;

	return iRet;
}

/*	$FXN :	CloseChannelOfDevice(afk_device_s * pdev)
==	======================================================================
==	$DSC :	关闭和pdev设备相关的所以通道
==	$ARG :	[in]pdev,关联的设备
==		 :	
==	$RET :	成功返回0,失败返回非零的错误码
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
int CTalk::CloseChannelOfDevice(afk_device_s * pdev)
{
//	EnterCriticalSection(&m_cs);
	m_cs.Lock();
	
	int iRet = 0;
	{
		list<LPTALKHANDLEINFO>::iterator it = m_talkhandlelist.begin();
		for (; it != m_talkhandlelist.end(); it++)
		{
			if ((*it)->lDevHandle == LONG(pdev))
			{
				if ((*it)->pTalkChannel != NULL)
				{
					(*it)->pTalkChannel->close((*it)->pTalkChannel);
				}
				delete (*it);
				m_talkhandlelist.erase(it);
				break;
			}
		}
	}
	{
		list<LPTALKBROADCASTINFO>::iterator it = m_broadcastlist.begin();
		for (; it != m_broadcastlist.end(); it++)
		{
			if ((*it)->lDevHandle == LONG(pdev))
			{
				if ((*it)->pTalkChannel != NULL)
				{
					(*it)->pTalkChannel->close((*it)->pTalkChannel);
				}
				delete (*it);
				m_broadcastlist.erase(it);
				break;
			}
		}
	}

//	LeaveCriticalSection(&m_cs);
	m_cs.UnLock();
	return iRet;
}

/*	$FXN :	TalkFunc(...)
==	======================================================================
==	$DSC :	打开和设备的对讲通道时注册的回调函数
==	$ARG :	
==		 :	
==	$RET :	
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
int WINAPI CTalk::TalkFunc(
						   afk_handle_t object,  
						   unsigned char *data, 
						   unsigned int datalen,  
						   void *param,  
						   void *udata)
{
	LPTALKHANDLEINFO pTalkHandle = (LPTALKHANDLEINFO)udata;
	if (pTalkHandle == NULL || 0 == datalen)
	{
		return -1;
	}

	if (pTalkHandle->pfcb)
	{
		pTalkHandle->pfcb((LONG)pTalkHandle, (char *)data, datalen, 1, pTalkHandle->dwUser);
	}
	
	return 0;
}

/*	$FXN :	RecordFunc(LPBYTE pDataBuffer, DWORD DataLength, int user)
==	======================================================================
==	$DSC :	注册录音库时的注册回调函数，在其中除了把音频数据回调给对讲用户外，还发送给广播组中的设备。
==	$ARG :	(in)pDataBuffer是录音音频数据，(in)是音频数据长度,(in)注册录音库时的用户自定义信息
==		 :	
==	$RET :	
==	$MOD :	2006-12-20		yehao(10857)		Create the Function
==	======================================================================
*/
//Begin: Modify by li_deming(11517) 2008-1-14
//void WINAPI CTalk::RecordFunc(LPBYTE pDataBuffer, DWORD DataLength, int user)
void WINAPI CTalk::RecordFunc(LPBYTE pDataBuffer, DWORD DataLength, long user)
//End:li_deming(11517)
{

/*
	//test
	static FILE* ff = NULL;
	int jjj = 0;
	if (ff == NULL)
	{
		ff = fopen("e:\\savetalk1.pcm", "wb");
		if (ff == NULL)
		{
			DWORD dw = GetLastError();
			OutputDebugString("damen!\n");
		}
	}
	
	if (ff != NULL)
	{
		fwrite(pDataBuffer, 1, DataLength, ff);
	}
	
*/	
	if (NULL == pDataBuffer || 0 == DataLength)
	{
		return;
	}
	CTalk *ptalk = (CTalk *)user;
	if (NULL == ptalk)
	{
		return;
	}

	char* pCbData = NULL;
	pCbData = new char[DataLength+100]; //如果说编完码比原来的数据还大，则会挂。。
	if (NULL == pCbData) 
	{
		return;
	}
	int  iCbLen = 0;
	
	if ((ptalk->GetAudioEncodeType() == INTERNEL_ENCODE_TYPE_8K8BPCM) || (ptalk->GetAudioEncodeType() == INTERNEL_ENCODE_TYPE_8K16BNEWPCM)) 
	{
		if (ptalk->GetAudioEncodeBit() == 8)
		{
			for( int j = 0 ; j < DataLength; j++)
			{
				*(pDataBuffer + j) += 128;
			}
		}

		pCbData[0]=0x00;
		pCbData[1]=0x00;
		pCbData[2]=0x01;
		pCbData[3]=0xF0;
		
		pCbData[4]= 8 == ptalk->GetAudioEncodeBit() ? 0x07:0x0C;	//pcm8:0x07; pcm16:0x0C
		pCbData[5]=0x02;//8k
		*(DWORD*)(pCbData+6)=DataLength;
		memcpy(pCbData+8, pDataBuffer, DataLength);
		
		iCbLen = 8+DataLength;
		/*
		//从16bit转至8bit
		iCbLen = DataLength+8;

		DWORD dwRecordDataSize = DataLength;
		unsigned short *p16 = (unsigned short *)pDataBuffer;
		BYTE *pu8 = (BYTE *)pDataBuffer;
		int i = 0;

		dwRecordDataSize = DataLength>>1;
		
		while(dwRecordDataSize>0)
		{
			pCbData[i++]=0x00;
			pCbData[i++]=0x00;
			pCbData[i++]=0x01;
			pCbData[i++]=0xF0;
			
			pCbData[i++]=0x07;//pcm8
			pCbData[i++]=0x02;//8k
			
			if(dwRecordDataSize>=1024)
			{
				pCbData[i++]=0x00;
				pCbData[i++]=0x04;
				
				for(int j=0;j<1024;j++)
				{
					pCbData[i++]=BYTE(*p16>>8);
					p16++;
				}
				dwRecordDataSize-=1024;
			}
			else
			{
				pCbData[i++]=BYTE(dwRecordDataSize&0xff);
				pCbData[i++]=BYTE(dwRecordDataSize>>8);
				for(int j=0;j<(int)dwRecordDataSize;j++)
				{		
					pCbData[i++]=BYTE(*p16>>8);
					p16++;
				}
				dwRecordDataSize=0;
			}
			iCbLen = i;
		}
		*/
	}
	else if (ptalk->GetAudioEncodeType() == INTERNEL_ENCODE_TYPE_G711A8K16B)
	{		
		if (g711a_Encode((char*)pDataBuffer, pCbData+8, DataLength, &iCbLen) != 1)
		{
			delete pCbData;
			return;
		}
		
		//亿蛙码流格式帧头
		pCbData[0]=0x00;
		pCbData[1]=0x00;
		pCbData[2]=0x01;
		pCbData[3]=0xF0;

		pCbData[4]=0x0E; //G711A
		pCbData[5]=0x02;
		pCbData[6]=BYTE(iCbLen&0xff);
		pCbData[7]=BYTE(iCbLen>>8);
		
		iCbLen += 8;
	
	}
	else if (ptalk->GetAudioEncodeType() == INTERNEL_ENCODE_TYPE_AMR8K16B)
	{
		//AMR encode
		iCbLen = AMR_Encode_Frame(ptalk->GetHisEncodeHandle(),(HI_S16*)pDataBuffer,
			(HI_U8*)(pCbData+8),MR102,MIME);
		if (iCbLen <= 0)
		{
			delete pCbData;
			return;
		}
		
		pCbData[0]=0x00;
		pCbData[1]=0x00;
		pCbData[2]=0x01;
		pCbData[3]=0xF0;
		
		pCbData[4]=0x14; //AMR
		pCbData[5]=0x02;
		pCbData[6]=BYTE(iCbLen&0xff);
		pCbData[7]=BYTE(iCbLen>>8);

		iCbLen += 8;
	}
	else if (ptalk->GetAudioEncodeType() == INTERNEL_ENCODE_TYPE_G711U8K16B)
	{
		if (g711u_Encode((char*)pDataBuffer, pCbData+8, DataLength, &iCbLen) != 1)
		{
			delete pCbData;
			return;
		}

		//亿蛙码流格式帧头
		pCbData[0]=0x00;
		pCbData[1]=0x00;
		pCbData[2]=0x01;
		pCbData[3]=0xF0;

		pCbData[4]=0x0A; //G711u
		pCbData[5]=0x02;
		pCbData[6]=BYTE(iCbLen&0xff);
		pCbData[7]=BYTE(iCbLen>>8);
		
		iCbLen += 8;
	}
	else
	{
		return;
	}
/*
		//test
	
		static FILE* ff2 = NULL;
		if (ff2 == NULL)
		{
		ff2 = fopen("e:\\savetalk-enc.pcm", "wb");
		if (ff2 == NULL)
		{
		DWORD dw = GetLastError();
		OutputDebugString("damen!\n");
		}
		}
		
		if (ff2 != NULL)
		{
		 fwrite(pCbData, 1, iCbLen, ff2);
		}
	
*/
	
//	EnterCriticalSection(&ptalk->m_cs);
	ptalk->m_cs.Lock();

	{
		list<LPTALKHANDLEINFO>::iterator it = ptalk->m_talkhandlelist.begin();
		for (; it != ptalk->m_talkhandlelist.end(); it++)
		{
			if (*it && (*it)->pfcb)
			{
				(*it)->pfcb(LONG(*it), pCbData, iCbLen, 0, (*it)->dwUser);
				//(*it)->pfcb(LONG(*it), newdata, DataLength+8, 0, (*it)->dwUser);
			}
		}
	}
	
	{
		list<LPTALKBROADCASTINFO>::iterator it = ptalk->m_broadcastlist.begin();
		for (; it != ptalk->m_broadcastlist.end(); it++)
		{
			if ((*it) && (*it)->pTalkChannel)
			{
				ptalk->SendData2Dev(((*it)->pTalkChannel), pCbData, iCbLen);
			}
		}
	}
//	LeaveCriticalSection(&ptalk->m_cs);
	ptalk->m_cs.UnLock();

	delete[] pCbData;
}
#else	//linux

CTalk::CTalk(CManager &man):m_man(man)
{
	m_ARStartSuc = FALSE;
	
	m_iEncodeFlag = 0;
	m_render = NULL;
	
	m_hisEncodeHandle = 0;
	m_hisDecodeHandle = 0;
}
CTalk::~CTalk()
{
}
int CTalk::Init()
{
	{
		list<LPTALKHANDLEINFO>::iterator it = m_talkhandlelist.begin();
		for (; it != m_talkhandlelist.end(); it++)
		{
			if (*it)
			{
				if ((*it)->pTalkChannel)
				{
					(*it)->pTalkChannel->close((*it)->pTalkChannel);
				}
				delete (*it);
			}
		}
		m_talkhandlelist.clear();
	}
	
	{
		list<LPTALKBROADCASTINFO>::iterator it = m_broadcastlist.begin();
		for (; it != m_broadcastlist.end(); it++)
		{
			if (*it)
			{
				if ((*it)->pTalkChannel)
				{
					(*it)->pTalkChannel->close((*it)->pTalkChannel);
				}
				delete (*it);
			}
		}
		m_broadcastlist.clear();
	}
	
	m_ARStartSuc = FALSE;
	m_dwAudioEncodeType = INTERNEL_ENCODE_TYPE_8K8BPCM;
	m_render = m_pManager->GetRenderManager().GetRender((HWND)0xFFFFFFFF);
	if (0 == (int)m_render || -1 == (int)m_render)
	{
		m_render = NULL;
	}
	m_iEncodeFlag = 0;

	return 0;
}

int CTalk::UnInit()
{
//	EnterCriticalSection(&m_cs);
	m_cs.Lock();
	{
		list<LPTALKHANDLEINFO>::iterator it = m_talkhandlelist.begin();
		for (; it != m_talkhandlelist.end(); it++)
		{
			if (*it)
			{
				if ((*it)->pTalkChannel)
				{
					(*it)->pTalkChannel->close((*it)->pTalkChannel);
				}
				delete (*it);
			}
		}
		m_talkhandlelist.clear();
	}
	
	{
		list<LPTALKBROADCASTINFO>::iterator it = m_broadcastlist.begin();
		for (; it != m_broadcastlist.end(); it++)
		{
			if (*it)
			{
				if ((*it)->pTalkChannel)
				{
					(*it)->pTalkChannel->close((*it)->pTalkChannel);
				}
				delete (*it);
			}
		}
		m_broadcastlist.clear();
	}
	
//	LeaveCriticalSection(&m_cs);
	m_cs.UnLock();
	
	
	if (m_render != NULL)
	{
		m_pManager->GetRenderManager().ReleaseRender(m_render);
		m_render = NULL;
	}
	
	return 0;
}

LONG CTalk::StartTalk(LONG lLoginID, pfAudioDataCallBack pfcb, DWORD dwUser)
{	
	if (m_pManager->IsDeviceValid((afk_device_s *)lLoginID) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return 0;
    }
	
	afk_device_s *device = (afk_device_s *)lLoginID;

	int nTalkMode = 0;
	BOOL bServerMode = FALSE;

	device->get_info(device, dit_talk_use_mode, &nTalkMode);
	bServerMode = nTalkMode==1? TRUE:FALSE;

	//先查询设备支持的音频编码格式
	int	bufLen = sizeof(DIALOG_CAPS)+16*sizeof(AUDIO_ATTR_T);
	char* capBuf = new char[bufLen];
	if (NULL == capBuf)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		return FALSE;
	}
	
	int retlen = 0;
	int iRet = m_pManager->GetDevConfig().QuerySystemInfo(lLoginID, SYSTEM_INFO_TALK_ATTR, capBuf, bufLen, &retlen, QUERY_WAIT);
	if (iRet >= 0 && retlen >= sizeof(DIALOG_CAPS) && ((retlen-sizeof(DIALOG_CAPS))%sizeof(AUDIO_ATTR_T))==0) 
	{
		DIALOG_CAPS* pDialogCap = (DIALOG_CAPS*)capBuf;
		if (0 == pDialogCap->iAudioTypeNum)
		{
			int tecType = 0;
			device->set_info(device, dit_talk_encode_type, &tecType);
			
			delete capBuf;
			m_pManager->SetLastError(NET_UNSUPPORTED);
			return FALSE;
		}
		AUDIO_ATTR_T* pAAT = NULL;
		for (int i = 0; i < pDialogCap->iAudioTypeNum; i++)
		{
			pAAT = (AUDIO_ATTR_T*)(capBuf+sizeof(DIALOG_CAPS)+i*sizeof(AUDIO_ATTR_T));
			if (2 == pAAT->iAudioType && 16 == pAAT->iAudioBit && 8000 == pAAT->dwSampleRate
				&& m_dwAudioEncodeType < INTERNEL_ENCODE_TYPE_G711A8K16B) 
			{
				//设备支持G711 8K 16 Bit
				m_dwAudioEncodeType = INTERNEL_ENCODE_TYPE_G711A8K16B;
			//	m_AudioFormat.dwFrameLen = 320;
				int tecType = 2;
				device->set_info(device, dit_talk_encode_type, &tecType);
			}

			if (3 == pAAT->iAudioType && 16 == pAAT->iAudioBit && 8000 == pAAT->dwSampleRate
				&& m_dwAudioEncodeType < INTERNEL_ENCODE_TYPE_AMR8K16B)
			{
				//设备支持AMR 8K 16 Bit
				m_dwAudioEncodeType = INTERNEL_ENCODE_TYPE_AMR8K16B;		
		//		m_AudioFormat.dwFrameLen = 320;		
				int tecType = 3;
				device->set_info(device, dit_talk_encode_type, &tecType);
			}
		}
	}

	delete capBuf;

	if (NULL == m_render) 
	{
		m_pManager->SetLastError(NET_NO_TALK_CHANNEL);
		return 0;
	}

	if (!bServerMode)
	{
		if (m_render->StartDec(FALSE, 
			(INTERNEL_ENCODE_TYPE_G711A8K16B==m_dwAudioEncodeType || INTERNEL_ENCODE_TYPE_AMR8K16B==m_dwAudioEncodeType) ? 2 : 1) < 0)
		{
			m_pManager->SetLastError(NET_RENDER_SOUND_ON_ERROR);
			return 0;
		}
	}
	
	
//	EnterCriticalSection(&m_cs);
	m_cs.Lock();

	LONG lRet = 0;
	list<LPTALKHANDLEINFO>::iterator it = m_talkhandlelist.begin();
	for (;it != m_talkhandlelist.end(); it++)
	{
		if ((*it)->lDevHandle == lLoginID)
		{
//			LeaveCriticalSection(&m_cs);
			m_cs.UnLock();
			return 0;
		}
	}

	afk_talk_channel_param_s talkchannelpar;
	LPTALKHANDLEINFO ptmpinfo = NULL;
	ptmpinfo = new TALKHANDLEINFO;
	if (NULL == ptmpinfo)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		lRet = 0;
		goto FAILED;
	}
	memset((void *)ptmpinfo, 0, sizeof(TALKHANDLEINFO));
	memset((void *)&talkchannelpar, 0, sizeof(afk_channel_param_s));
	talkchannelpar.base.udata = (void *)ptmpinfo;
	talkchannelpar.base.func = CTalk::TalkFunc;
	talkchannelpar.no = ptmpinfo->iChannelId;
	
	ptmpinfo->pTalkChannel = (afk_channel_s *)device->open_channel(device, AFK_CHANNEL_TYPE_TALK, &talkchannelpar);
	if (NULL == ptmpinfo->pTalkChannel)
	{
		m_pManager->SetLastError(NET_OPEN_CHANNEL_ERROR);
		lRet = 0;
		goto FAILED;
	}
	ptmpinfo->lDevHandle = lLoginID;
	ptmpinfo->pfcb = pfcb;
	ptmpinfo->dwUser = dwUser;
	m_talkhandlelist.push_back(ptmpinfo);
	lRet = (LONG)ptmpinfo;
	
	goto END;

FAILED:
	if (ptmpinfo != NULL)
	{
		if (ptmpinfo->pTalkChannel)
		{
			ptmpinfo->pTalkChannel->close(ptmpinfo->pTalkChannel);
		}
		delete ptmpinfo;
	}

	if (!bServerMode)
	{
		if (m_render != NULL)
		{
			m_render->CloseAudio();
			m_render->StopDec();
		}
	}
	

END:
//	LeaveCriticalSection(&m_cs);
	m_cs.UnLock();
	return lRet;
}

BOOL CTalk::SetVolume(LONG lTalkHandle, WORD wVolume)
{
	return FALSE;
}

LONG CTalk::SendData2Dev(afk_channel_s *pChannel, char *pSendBuf, DWORD dwBufSize)
{	
	LONG lRet = 0;
	if (NULL == pChannel || NULL == pSendBuf || 0 == dwBufSize)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return -1;
	}
	struct afk_talk_info_s tempst;
	tempst.data = pSendBuf;
	tempst.datalen = dwBufSize;
	if (1 == pChannel->set_info(pChannel, 0, &tempst))
	{
		lRet = dwBufSize;
	}
	
	return lRet;
}

LONG CTalk::TalkSendData(LONG lTalkHandle, char *pSendBuf, DWORD dwBufSize)
{
	//	EnterCriticalSection(&m_cs);
	m_cs.Lock();
	
	LONG lRet = 0;
	list<LPTALKHANDLEINFO>::iterator it = m_talkhandlelist.begin();
	for (; it != m_talkhandlelist.end(); it++)
	{
		if ((LONG)(*it) == lTalkHandle)
		{
			break;
		}
	}
	if (it == m_talkhandlelist.end() || NULL == (*it))
	{
		m_pManager->SetLastError(NET_INVALID_HANDLE);
		lRet = -1;
		goto END;
	}
	lRet = SendData2Dev((*it)->pTalkChannel, pSendBuf, dwBufSize);
	
END:
	//	LeaveCriticalSection(&m_cs);
	m_cs.UnLock();
	
	return lRet;
}
BOOL CTalk::RecordStart()
{
	return FALSE;
}
BOOL CTalk::RecordStop()
{
	return FALSE;
}
void CTalk::AudioDec(char *pAudioDataBuf, DWORD dwBufSize)
{
}
BOOL CTalk::StopTalk(LONG lTalkHandle)
{
	return FALSE;
}

BOOL CTalk::BroadcastAddDev(LONG lLoginID)
{
	if (m_pManager->IsDeviceValid((afk_device_s *)lLoginID) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return FALSE;
    }
	
	//	EnterCriticalSection(&m_cs);
	m_cs.Lock();
	
	list<LPTALKBROADCASTINFO>::iterator it = m_broadcastlist.begin();
	for (; it != m_broadcastlist.end(); it++)
	{
		if ((*it)->lDevHandle == lLoginID)
		{
			//			LeaveCriticalSection(&m_cs);
			m_cs.UnLock();
			m_pManager->SetLastError(NET_ERROR);
			return FALSE;
		}
	}
	
	BOOL bRet = FALSE;
	afk_device_s *device = (afk_device_s *)lLoginID;
	afk_talk_channel_param_s talkchannelpar;
	LPTALKBROADCASTINFO ptmpinfo = NULL;
	ptmpinfo = new TALKBROADCASTINFO;
	if (NULL == ptmpinfo)
	{
		m_pManager->SetLastError(NET_SYSTEM_ERROR);
		goto FAILED;
	}
	memset((void *)ptmpinfo, 0, sizeof(TALKBROADCASTINFO));
	memset((void *)&talkchannelpar, 0, sizeof(afk_talk_channel_param_s));
	talkchannelpar.base.udata = (void *)ptmpinfo;
	talkchannelpar.base.func = NULL;
	talkchannelpar.no = ptmpinfo->iChannelId;
	ptmpinfo->pTalkChannel = (afk_channel_s *)device->open_channel(device, AFK_CHANNEL_TYPE_TALK, &talkchannelpar);
	if (NULL == ptmpinfo->pTalkChannel)
	{
		m_pManager->SetLastError(NET_OPEN_CHANNEL_ERROR);
		goto FAILED;
	}
	ptmpinfo->lDevHandle = lLoginID;
	m_broadcastlist.push_back(ptmpinfo);
	bRet = TRUE;
	goto END;
	
FAILED:
	bRet = FALSE;
	if (ptmpinfo != NULL)
	{
		if (ptmpinfo->pTalkChannel)
		{
			ptmpinfo->pTalkChannel->close(ptmpinfo->pTalkChannel);
		}
		delete ptmpinfo;
	}
END:
	//	LeaveCriticalSection(&m_cs);
	m_cs.UnLock();
	return bRet;
}

BOOL CTalk::BroadcastDelDev(LONG lLoginID)
{
	//	EnterCriticalSection(&m_cs);
	m_cs.Lock();
	
	BOOL bRet = FALSE;
	list<LPTALKBROADCASTINFO>::iterator it = m_broadcastlist.begin();
	for (; it != m_broadcastlist.end(); it++)
	{
		if ((*it) && (*it)->lDevHandle == lLoginID)
		{
			if ((*it)->pTalkChannel)
			{
				(*it)->pTalkChannel->close((*it)->pTalkChannel);
			}
			delete (*it);
			m_broadcastlist.erase(it);
			break;
		}
	}
	if (m_broadcastlist.end() == it)
	{
		m_pManager->SetLastError(NET_ERROR);
		bRet = FALSE;
	}
	//	LeaveCriticalSection(&m_cs);
	m_cs.UnLock();
	return bRet;
}

int CTalk::InitAudioEncode(AUDIO_FORMAT aft)
{
	if (aft.byFormatTag != 0 || aft.nChannels != 1 || aft.wBitsPerSample != 16 || aft.nSamplesPerSec != 8000)
	{
		m_iEncodeFlag = 0;
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return NET_ILLEGAL_PARAM;
	}
	
	m_iEncodeFlag = 1;
	return 0;
}

int	CTalk::AudioEncode(LONG lHandle, BYTE *lpInBuf, DWORD *lpInLen, BYTE *lpOutBuf, DWORD *lpOutLen)
{
	if (0 == m_iEncodeFlag)
	{
		m_pManager->SetLastError(NET_AUDIOENCODE_NOTINIT);
		return NET_AUDIOENCODE_NOTINIT;
	}
	if (NULL == lpInBuf || NULL == lpInLen || NULL == lpOutBuf || NULL == lpOutLen || 0 == *lpInLen)
	{
		m_pManager->SetLastError(NET_ILLEGAL_PARAM);
		return NET_ILLEGAL_PARAM;
	}
	if (*lpInLen >= MAX_ENCODE_LEN*2)
	{
		*lpInLen = MAX_ENCODE_LEN*2;
		m_pManager->SetLastError(NET_DATA_TOOLONGH);
		return NET_DATA_TOOLONGH;
	}
	if (*lpOutLen < (*lpInLen/2 + 8))
	{
		*lpOutLen = *lpInLen/2 + 8;
		m_pManager->SetLastError(NET_INSUFFICIENT_BUFFER);
		return NET_INSUFFICIENT_BUFFER;
	}
	
	int iRet = 0;
	int iCount = 0;
	int iDataLen = (*lpInLen>>1);
	unsigned short *p16 = (unsigned short *)lpInBuf;
	lpOutBuf[iCount++] = 0x00;
	lpOutBuf[iCount++] = 0x00;
	lpOutBuf[iCount++] = 0x01;
	lpOutBuf[iCount++] = 0xf0;
	lpOutBuf[iCount++] = 0x07;
	lpOutBuf[iCount++] = 0x02;
	*(unsigned short *)(lpOutBuf+iCount) = (unsigned short)iDataLen;
	iCount += 2;
	for (int i = 0; i < iDataLen; i++)
	{
		lpOutBuf[iCount++] = (BYTE)(*p16>>8);
		p16++;
	}
	*lpOutLen = iCount;	
	*lpInLen = 0;
	
	return iRet;
}
int CTalk::ReleaseAudioEncode()
{
	//	EnterCriticalSection(&m_cs);
	m_cs.Lock();
	m_iEncodeFlag = 0;
	//	LeaveCriticalSection(&m_cs);
	m_cs.UnLock();
	return 0;
}
LONG CTalk::InitAudioDecode(AUDIO_FORMAT aft)
{
	return 0;
}
int CTalk::AudioDecode(LONG lHandle, BYTE *lpInBuf, DWORD *lpInLen, BYTE *lpOutBuf, DWORD *lpOutLen)
{
	return 0;
}
int CTalk::ReleaseAudioDecode(LONG lHandle)
{
	return 0;
}
int CTalk::CloseChannelOfDevice(afk_device_s * pdev)
{
	//	EnterCriticalSection(&m_cs);
	m_cs.Lock();
	
	int iRet = 0;
	{
		list<LPTALKHANDLEINFO>::iterator it = m_talkhandlelist.begin();
		for (; it != m_talkhandlelist.end(); it++)
		{
			if ((*it)->lDevHandle == LONG(pdev))
			{
				if ((*it)->pTalkChannel != NULL)
				{
					(*it)->pTalkChannel->close((*it)->pTalkChannel);
				}
				delete (*it);
				m_talkhandlelist.erase(it);
				break;
			}
		}
	}
	{
		list<LPTALKBROADCASTINFO>::iterator it = m_broadcastlist.begin();
		for (; it != m_broadcastlist.end(); it++)
		{
			if ((*it)->lDevHandle == LONG(pdev))
			{
				if ((*it)->pTalkChannel != NULL)
				{
					(*it)->pTalkChannel->close((*it)->pTalkChannel);
				}
				delete (*it);
				m_broadcastlist.erase(it);
				break;
			}
		}
	}
	
	//	LeaveCriticalSection(&m_cs);
	m_cs.UnLock();
	return iRet;
}

int CTalk::WINAPI TalkFunc(afk_handle_t object,  
				   unsigned char *data, 
				   unsigned int datalen,  
				   void *param,  
				   void *udata)
{
	if (NULL == data || 0 == datalen)
	{
		return -1;
	}
	
	int iRet = 0;
	LPTALKHANDLEINFO pTalkHandle = (LPTALKHANDLEINFO)udata;
	if (pTalkHandle && pTalkHandle->pfcb)
	{
		pTalkHandle->pfcb((LONG)pTalkHandle, (char *)data, datalen, 1, pTalkHandle->dwUser);
	}
	
	return iRet;
}

void CTalk::WINAPI RecordFunc(LPBYTE pDataBuffer, DWORD DataLength, /*int*/long user)
{
}

#endif
