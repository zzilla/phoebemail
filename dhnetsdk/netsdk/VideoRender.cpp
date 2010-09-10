
#include "VideoRender.h"

#ifdef WIN32	//目前解码只支持windows操作系统

//#include "./depend/play.h"// Delete by li_deming(11517) 2008-1-14
#include "./depend/playdef.h"
/*
#include "depend/decode.h"
#include "depend/render.h"
#include "./depend/interface.h"
*/

#define MAX_RENDER_COUNT  32
static BOOL g_inited = FALSE;
static UINT	g_count = 0;
static BYTE g_decFlag[MAX_RENDER_COUNT] = {0};
DEVMutex g_csFlag;

/////////////////////////////////////////////////////////////////

// 获取一个解码空闲位
int GetFreeFlag()
{
//	EnterCriticalSection(&g_csFlag);
	g_csFlag.Lock();
	
	int ret = -1;
    for (int i=0; i<MAX_RENDER_COUNT; i++)
    {
		if (g_decFlag[i] == 0)
		{
			g_decFlag[i] = 1;
			ret = i;
			break;
		}
    }
	
//	LeaveCriticalSection(&g_csFlag);
	g_csFlag.UnLock();
    return ret;
}

//归还解码空闲位
void ReturnFlag(int index)
{
	if (index > MAX_RENDER_COUNT-1 || index < 0)
	{
		return ;
	}
	
//	EnterCriticalSection(&g_csFlag);
	g_csFlag.Lock();
	
	g_decFlag[index] = 0;
	
//	LeaveCriticalSection(&g_csFlag);
	g_csFlag.UnLock();
}

CVideoRender::CVideoRender(HWND hWnd):m_hWnd(hWnd)
{
	{
//		EnterCriticalSection(&g_csFlag);
		g_csFlag.Lock();
		
		if (!g_inited)
		{
			memset(g_decFlag, 0, MAX_RENDER_COUNT);
			g_inited = true;
		}
		
//		LeaveCriticalSection(&g_csFlag);
		g_csFlag.UnLock();
	}
	g_count++;
	
	m_iPlayPort = -1;
	m_bPlaying = FALSE;
	m_bAudio = FALSE;
	m_pfDrawFunc = NULL;
	m_drawUd1 = NULL;
	m_drawUd2 = NULL;
	m_drawUd3 = NULL;
	m_bStep = FALSE;

	m_tmpbpsam = 0;
	m_tmpsamps = 0;

	m_monitor = NULL;
//	m_file = 0;
//	InitializeCriticalSection(&m_csMT);
}

CVideoRender::~CVideoRender()
{
	g_count--;
//	DeleteCriticalSection(&m_csMT);
}


BYTE GetFrameType(tagDemuxInfo* parm)
{
	BYTE frameType = 0;
	switch(parm->subtype) 
	{
	case DEC_VIDEO_I_FRAME: 
		frameType = 0;
		break;
	case DEC_VIDEO_P_FRAME: 
		frameType = 1;
		break;
	case DEC_VIDEO_B_FRAME: 
		frameType = 3;
		break;
	default:
		break;
	}

	return frameType;
}

BYTE GetFormatType(tagDemuxInfo *parm)
{
	BYTE formatType = 0;
	if (parm->width == 576 || parm->width == 288) 
	{
		formatType = 0;//PAL
	}
	else
	{
		formatType = 1;//NTSC
	}

	return formatType;
}

BYTE GetSizeType(tagDemuxInfo *parm)
{
	BYTE sizeType = 0;
	if (parm->width == 352 && parm->height == 288)
	{
		sizeType = 0; //CIF
	}
	if (parm->width == 352 && parm->height == 576)
	{
		sizeType = 1; //HD1
	}
	if (parm->width == 704 && parm->height == 288)
	{
		sizeType = 2; //2CIF
	}
	if (parm->width == 704 && parm->height == 576)
	{
		sizeType = 3; //D1
	}
	if (parm->width == 640 && parm->height == 480)
	{
		sizeType = 4; //VGA
	}
	if (parm->width == 176 && parm->height == 144)
	{
		sizeType = 5; //QCIF
	}
	if (parm->width == 320 && parm->height == 240)
	{
		sizeType = 6; //QVGA
	}
	if (parm->width == 480 && parm->height == 480) 
	{
		sizeType = 7;//SVCD
	}
	if (parm->width == 160 && parm->height == 128) 
	{
		sizeType = 8;//QQVGA
	}
	if (parm ->width == 800 && parm->height == 592)
	{
		sizeType = 9;//SVGA
	}
	if (parm->width == 1024 && parm->height == 768)
	{
		sizeType = 10;//XVGA
	}
	if (parm->width == 1280 && parm->height == 800)
	{
		sizeType = 11;//WXGA
	}
	if (parm->width == 1280 && parm->height == 1024) 
	{
		sizeType = 12;//SXGA
	}
	if (parm->width == 1600 && parm->height == 1024) 
	{
		sizeType = 13;//WSXGA
	}
	if (parm->width == 1600 && parm->height == 1200)
	{
		sizeType = 14;//UXGA
	}
	if (parm->width == 1920 && parm->height == 1200) 
	{
		sizeType = 15;//WUXGA
	}
	if (parm->width == 240 && parm->height == 192) 
	{
		sizeType = 16;//LFT
	}
	if (parm->width == 1280 && parm->height == 720) 
	{
		sizeType = 17;//720
	}
	if (parm->width == 1920 && parm->height == 1080) 
	{
		sizeType = 18;//1080
	}



	return sizeType;
}

DWORD FOURCCC(char c1, char c2, char c3, char c4)
{
	DWORD fourcc = 0;
	fourcc |= c1;
	fourcc = fourcc << 8;
	fourcc |= c2;
	fourcc = fourcc << 8;
	fourcc |= c3;
	fourcc = fourcc << 8;
	fourcc |= c4;

	return fourcc;
}

BYTE getSamples(long smp)
{
	// 采样 0 - 8000, 1 - 11025, 2 - 16000, 3 - 22050, 4 - 32000, 5 - 44100, 6 - 48000
	BYTE sampleType = 0;
	switch(smp) 
	{
	case 8000:
		sampleType = 0;
		break; 
	case 11025:
		sampleType = 1;
		break;
	case 16000:
		sampleType = 2;
		break; 
	case 22050:
		sampleType = 3;
		break;
	case 32000:
		sampleType = 4;
		break; 
	case 44100:
		sampleType = 5;
		break;
	case 48000:
		sampleType = 6;
		break; 
	default:
		break;
	}

	return sampleType;
}

void CALLBACK MyDrawFunc(long nPort,HDC hDc,LONG nUser)
{
	CVideoRender* pThis = (CVideoRender*)nUser;
	if (NULL == pThis)
	{
		return;
	}

	pThis->MyDrawFunc_Imp(nPort, hDc);
}

void CVideoRender::MyDrawFunc_Imp(long nPort,HDC hDc)
{
	if (m_iPlayPort != nPort)
	{
		return;
	}
	
	fDrawCallBack df = (fDrawCallBack)m_pfDrawFunc;
	if (df != NULL)
	{
		df((LONG)m_drawUd1, (LONG)m_drawUd2, hDc, (DWORD)m_drawUd3);
	}
}

void CALLBACK MyDisplayFunc(long nPort, char* pBuf, long nSize, long nWidth, long nHeight, long nStamp, long nType, long nReserved)
{
	CVideoRender* pThis = (CVideoRender*)nReserved;
	if (NULL == pThis)
	{
		return;
	}
	
	pThis->MyDisplayFunc_Imp(nPort, pBuf, nSize, nWidth, nHeight, nStamp, nType);
}

void CVideoRender::MyDisplayFunc_Imp(long nPort, char* pBuf, long nSize, long nWidth, long nHeight, long nStamp, long nType)
{
	if (m_iPlayPort != nPort)
	{
		return;
	}

//	EnterCriticalSection(&m_csMT);
	m_csMT.Lock();
	
    if (m_monitor && m_monitor->channel)
    {
		if (m_monitor->pRealDataCallBack)
		{
			m_monitor->pRealDataCallBack((LONG)m_monitor->channel, 2, 
				(BYTE*)pBuf, nSize, m_monitor->userdata);
		}

		if (m_monitor->pRealDataCallBackEx)
		{
			DWORD tmpFlag = 0x00000000;
			LONG lParam = 0;
			
			tmpFlag = m_monitor->dwDataTypeFlag & 0x00000004;
			
			if (tmpFlag) 
			{
				m_monitor->pRealDataCallBackEx((LONG)m_monitor->channel, 2, 
					(BYTE*)pBuf, nSize, lParam, m_monitor->userdata);
			}
		}
    }
	
//	LeaveCriticalSection(&m_csMT);
	m_csMT.UnLock();
}

void CALLBACK MyDemuxFunc(long nPort, char* pBuf, long nSize, void* pParam, long nReserved, long nUser)
{
	CVideoRender* pThis = (CVideoRender*)nUser;
	if (NULL == pThis)
	{
		return;
	}

	pThis->MyDemuxFunc_Imp(nPort, pBuf, nSize, pParam, nReserved);
}

void CVideoRender::MyDemuxFunc_Imp(long nPort, char* pBuf, long nSize, void* pParam, long nReserved)
{
	if (m_iPlayPort != nPort)
	{
		return;
	}
	
	tagDemuxInfo* pDemuxInfo = (tagDemuxInfo*)pParam;
	if (NULL == pDemuxInfo) 
	{
		return;
	}

	if (DEC_DEMUX_TYPE_VIDEO_FRAME == pDemuxInfo->type) 
	{
//		EnterCriticalSection(&m_csMT);
		m_csMT.Lock();
		
		if (m_monitor && m_monitor->channel)
		{
			if (m_monitor->pRealDataCallBack)
			{
				m_monitor->pRealDataCallBack((LONG)m_monitor->channel, 1, 
					(BYTE*)pBuf, nSize, m_monitor->userdata);
			}
			
			if (m_monitor->pRealDataCallBackEx)
			{
				DWORD tmpFlag = 0x00000000;
				tagVideoFrameParam tmpVParam = {0};
				LONG lParam = 0;
				
				tmpFlag = m_monitor->dwDataTypeFlag & 0x00000002;
				if (tmpFlag) 
				{
					tmpVParam.encode = pDemuxInfo->encode;
					tmpVParam.frametype = GetFrameType(pDemuxInfo);
					tmpVParam.format = GetFormatType(pDemuxInfo);
					tmpVParam.size = GetSizeType(pDemuxInfo);
					tmpVParam.struTime.dwYear = pDemuxInfo->year;
					tmpVParam.struTime.dwMonth = pDemuxInfo->month;
					tmpVParam.struTime.dwDay = pDemuxInfo->day;
					tmpVParam.struTime.dwHour = pDemuxInfo->hour;
					tmpVParam.struTime.dwMinute = pDemuxInfo->minute;
					tmpVParam.struTime.dwSecond = pDemuxInfo->second;
					
					if (pDemuxInfo->encode == DEC_ENCODE_H264)
					{
						tmpVParam.fourcc = 0;
					}
					else
					{
						tmpVParam.fourcc = FOURCCC('X','V','I','D');
					}
					
					lParam = (LONG)&tmpVParam;
					
					m_monitor->pRealDataCallBackEx((LONG)m_monitor->channel, 1, 
						(BYTE*)pBuf, nSize, lParam, m_monitor->userdata);
				}
			}
		}
		
//		LeaveCriticalSection(&m_csMT);
		m_csMT.UnLock();
	}
	else if (DEC_DEMUX_TYPE_AUDIO_FRAME == pDemuxInfo->type)
	{
		m_tmpbpsam = pDemuxInfo->bitspersample;
		m_tmpsamps = pDemuxInfo->samplespersecond;
	}
}

void CALLBACK MyAudioFunc(long nPort, char * pAudioBuf, long nSize, long nStamp, long nType, long nUser)
{
	CVideoRender* pThis = (CVideoRender*)nUser;
	if (NULL == pThis)
	{
		return;
	}

	pThis->MyAudioFunc_Imp(nPort, pAudioBuf, nSize, nStamp, nType);
}

void CVideoRender::MyAudioFunc_Imp(long nPort, char * pAudioBuf, long nSize, long nStamp, long nType)
{
	if (m_iPlayPort != nPort)
	{
		return;
	}
	
//	EnterCriticalSection(&m_csMT);
	m_csMT.Lock();
	
    if (m_monitor && m_monitor->channel)
    {
		if (m_monitor->pRealDataCallBack)
		{
			m_monitor->pRealDataCallBack((LONG)m_monitor->channel, 3, 
				(BYTE*)pAudioBuf, nSize, m_monitor->userdata);
		}
		
		if (m_monitor->pRealDataCallBackEx)
		{
			DWORD tmpFlag = 0x00000000;
			tagCBPCMDataParam tmpPCMParam = {0};
			LONG lParam = 0;

			tmpFlag = m_monitor->dwDataTypeFlag & 0x00000008;
			if (tmpFlag)
			{
				tmpPCMParam.channels = 1; //单声道
				tmpPCMParam.depth = m_tmpbpsam;
				tmpPCMParam.param1 = 0;
				tmpPCMParam.samples = getSamples(m_tmpsamps);

				lParam = (LONG)&tmpPCMParam;
			}
			
			if (tmpFlag) 
			{
				m_monitor->pRealDataCallBackEx((LONG)m_monitor->channel, 3, 
					(BYTE*)pAudioBuf, nSize, lParam, m_monitor->userdata);
			}
		}
	}
	
//	LeaveCriticalSection(&m_csMT);
	m_csMT.UnLock();
}

//开始/停止解码
int  CVideoRender::StartDec(BOOL framectrl, DWORD dwTalkType)
{
	if (m_bPlaying)
	{
		return -1;
	}
	
	switch(dwTalkType)
	{
	case 0: //非语音对讲
		{
			m_iPlayPort = GetFreeFlag();
			if (-1 == m_iPlayPort)
			{
				//没有标志位了
				return -1;
			}
		}
		break;
	case 1:	//8k8bit pcm对讲
		{
			m_iPlayPort = 100; //100口为语音对讲专用
		}
		break;
	case 2: //8k16bit g711a or amr OR 带头信息的PCM
		{
			m_iPlayPort = 99;
		}
		break;
	default:
		return -1;
		break;
	}

	BOOL b = PLAY_SetStreamOpenMode(m_iPlayPort, framectrl ? STREAME_FILE : STREAME_REALTIME);
	if (!b)
	{
		return -1;
	}

	b = PLAY_OpenStream(m_iPlayPort, NULL, 0, RENDER_BUFFER_LEN);
	if (!b)
	{
		return -1;
	}

	b = PLAY_SetDisplayCallBack(m_iPlayPort, MyDisplayFunc, (long)this);
	if (!b)
	{
		PLAY_CloseStream(m_iPlayPort);
		return -1;
	}

	b = PLAY_SetAudioCallBack(m_iPlayPort, MyAudioFunc, (long)this);
	if (!b)
	{
		PLAY_CloseStream(m_iPlayPort);
		return -1;
	}
	
	b = PLAY_RigisterDrawFun(m_iPlayPort, MyDrawFunc, (LONG)this);
	if (!b)
	{
#ifdef DEBUG
		OutputDebugString("Register draw call back failed!\n");
#endif
	}

	b = PLAY_Play(m_iPlayPort, m_hWnd);
	if (!b)
	{
		PLAY_CloseStream(m_iPlayPort);
		return -1;
	}

	if (dwTalkType != 0)
	{
		b = PLAY_PlaySoundShare(m_iPlayPort);

		if (!b)
		{
			PLAY_CloseStream(m_iPlayPort);
			return -1;
		}
	}
	
	b = PLAY_SetDemuxCallBack(m_iPlayPort, MyDemuxFunc, (long)this);
	if (!b)
	{
		PLAY_Stop(m_iPlayPort);
		PLAY_CloseStream(m_iPlayPort);
		return -1;
	}

	m_bPlaying = TRUE;
	m_bAudio = FALSE;
	
	return 0;
}

int  CVideoRender::StopDec()
{
	if (!m_bPlaying)
	{
		return -1;
	}

	int ret = 0;
	
	SetMonitorInfo(0);

	BOOL b = PLAY_Stop(m_iPlayPort);
	if (!b) 
	{
		ret = -1;
	}

	b = PLAY_CloseStream(m_iPlayPort);
	if (!b)
	{
		ret = -1;
	}

	ReturnFlag(m_iPlayPort);
	m_iPlayPort = -1;
	
	m_bPlaying = FALSE;


	return ret;
}

//音频函数
BOOL  CVideoRender::OpenAudio()
{
	if (!m_bPlaying || m_bAudio)
	{
		return FALSE;
	}
	
	BOOL b = PLAY_PlaySoundShare(m_iPlayPort);
	if (b)
	{
		m_bAudio = TRUE;
	}
	else
	{
#ifdef DEBUG
		OutputDebugString("open sound ERROR!\n");
#endif
	}

	return b;
}

BOOL  CVideoRender::CloseAudio()
{
	if (!m_bPlaying || !m_bAudio)
	{
		return FALSE;
	}
	
	BOOL b = PLAY_StopSoundShare(m_iPlayPort);
	if (b)
	{
		m_bAudio = FALSE;
	}
	else
	{
#ifdef DEBUG
		OutputDebugString("close sound ERROR!\n");
#endif
	}

	return b;
}

/*
BOOL  CVideoRender::PlayAudio(unsigned char *pBuf, int nLen)
{
	//NP yet
	return 0;
}
*/

BOOL  CVideoRender::IsAudio()
{
	return m_bAudio;
}

BOOL  CVideoRender::SetAudioVolume(int nVolume)
{
	if (!m_bPlaying)
	{
		return FALSE;
	}

	return PLAY_SetVolume(m_iPlayPort, (0XFFFF*nVolume)/100);
}


//获取帧率
int   CVideoRender::GetFrameRate()
{
	if (!m_bPlaying)
	{
		return -1;
	}

	return PLAY_GetCurrentFrameRate(m_iPlayPort);
}

//帧率控制
BOOL  CVideoRender::SetFrameRate(int framerate)
{
	if (!m_bPlaying)
	{
		return FALSE;
	}
	
	return PLAY_ChangeRate(m_iPlayPort, framerate);
}

BOOL  CVideoRender::Slow()
{
	if (!m_bPlaying)
	{
		return FALSE;
	}

	return PLAY_Slow(m_iPlayPort);
}

BOOL  CVideoRender::Fast()
{
	if (!m_bPlaying)
	{
		return FALSE;
	}
	
	return PLAY_Fast(m_iPlayPort);
}

BOOL  CVideoRender::Pause(BOOL bPause)
{
	if (!m_bPlaying)
	{
		return FALSE;
	}
	
	return PLAY_Pause(m_iPlayPort, bPause);
}


BOOL  CVideoRender::PlayNormal()
{
	if (!m_bPlaying)
	{
		return FALSE;
	}

	return PLAY_Play(m_iPlayPort, m_hWnd);
}

BOOL  CVideoRender::Step(BOOL bStop)
{
	if (!m_bPlaying)
	{
		return FALSE;
	}

	BOOL bRet = TRUE;
	if (bStop)
	{
		if (m_bStep)
		{
			bRet = PLAY_Play(m_iPlayPort, m_hWnd);
			if (!bRet)
			{
#ifdef DEBUG
				OutputDebugString("recover from step failed!\n");
#endif
			}
			else
			{
				m_bStep = FALSE;
			}
		}
		else
		{
			bRet = FALSE;
		}
	}
	else
	{
		bRet = PLAY_OneByOne(m_iPlayPort);
		if (!bRet)
		{
#ifdef DEBUG
			OutputDebugString("start step play failed!\n");
#endif
		}
		else
		{
			m_bStep = TRUE;
		}
	}

	return bRet;
}

//抓图
BOOL  CVideoRender::SnapPicture(const char *pchFileName)
{
	if (!m_bPlaying)
	{
		return FALSE;
	}

	return PLAY_CatchPic(m_iPlayPort, (char*)pchFileName);
}

//闲置render资源再次被利用时，刷新一些成员变量
int  CVideoRender::ChangeHwnd(HWND hWnd)
{
	if (m_bPlaying)
	{
		//不是闲置的render资源
		return -1;
	}
	
	m_hWnd = hWnd;
	m_bAudio = FALSE;

	return 0;
}


//填充多媒体数据
BOOL  CVideoRender::Play(unsigned char *pBuf, int nLen)
{
	//test file
//	if (NULL == m_file)
//	{
//		static int fileno = 0;
//		fileno++;
//		char fileStr[100] = {0};
//		sprintf(fileStr, "d:\\tempsave_%d.dav", fileno);
//		m_file = fopen(fileStr, "wb");
//		if (m_file == NULL)
//		{
//			DWORD dw = GetLastError();
//			OutputDebugString("damen!\n");
//		}
//	}
//	if (m_file != NULL)
//	{
//		fwrite(pBuf, 1, nLen, m_file);
//	}

	//Sleep(100);
	BOOL bRet = FALSE;

	if (m_iPlayPort != -1)
	{
		bRet = PLAY_InputData(m_iPlayPort, pBuf, nLen);

		/*
		if (bRet)
		{
			
			char str_zgf[100];
			sprintf(str_zgf, "d://save_%d.dav",m_iPlayPort);
			FILE* fp = fopen(str_zgf, "ab");
			fwrite(pBuf, 1, nLen, fp);
			fclose(fp);
		}
		*/
	}

	return bRet;
}

/*
 *	调整图象播放的流畅性
 */
BOOL  CVideoRender::AdjustFluency(int nLevel)
{
	if(nLevel<0 || nLevel>6)
	{
		return FALSE;
	}

	BOOL bRet = FALSE;
	if (m_iPlayPort != -1)
	{
		bRet = PLAY_AdjustFluency(m_iPlayPort, nLevel);
	}
	
	return bRet;
}

//清空解码缓存
int  CVideoRender::Reset()
{
	if (!m_bPlaying)
	{
		return -1;
	}

	int ret = 0;
	if (!PLAY_ResetBuffer(m_iPlayPort, BUF_VIDEO_SRC))
	{
		ret--;
	}
	if (!PLAY_ResetBuffer(m_iPlayPort, BUF_AUDIO_SRC))
	{
		ret--;
	}
	if (!PLAY_ResetBuffer(m_iPlayPort, BUF_VIDEO_RENDER))
	{
		ret--;
	}
	if (!PLAY_ResetBuffer(m_iPlayPort, BUF_AUDIO_RENDER))
	{
		ret--;
	}

	return ret;
}

//判断解码缓存是否为空
BOOL  CVideoRender::IsEmpty()
{
	if (!m_bPlaying)
	{
		return FALSE;
	}
	
	if (PLAY_GetSourceBufferRemain(m_iPlayPort) == 0
		&&
		PLAY_GetBufferValue(m_iPlayPort, BUF_VIDEO_RENDER) == 0)
	{
		return TRUE;
	}
	
	return FALSE;
}

//获取解码缓存中未解码的数据大小
DWORD CVideoRender::GetSourceBufferRemain()
{
	if (!m_bPlaying)
	{
		return 0;
	}

	return PLAY_GetSourceBufferRemain(m_iPlayPort);
}


//获取OSD时间，接口函数
BOOL  CVideoRender::GetOSDTime(unsigned long *year, unsigned long *month, unsigned long *day,
				 unsigned long *hour, unsigned long *minute, unsigned long *second)
{
	if (NULL == year || NULL == month || NULL == day 
		|| NULL == hour || NULL == minute || NULL == second)
	{
		return FALSE;
	}

	TimeInfo ti = {0};
	int retlen = 0;

	BOOL b = PLAY_QueryInfo(m_iPlayPort, PLAY_CMD_GetTime, (char*)&ti, sizeof(TimeInfo), &retlen);
	if (!b || retlen != sizeof(TimeInfo))
	{
		return FALSE;
	}

	*year = ti.year;
	*month = ti.month;
	*day = ti.day;
	*hour = ti.hour;
	*minute = ti.minute;
	*second = ti.second;

	//NP yet
	return TRUE;
}

BOOL  CVideoRender::SetDrawCallBack(void* pDrawCBFunc, void* ud1, void* ud2, void* ud3)
{
	m_pfDrawFunc = pDrawCBFunc;
	m_drawUd1 = ud1;
	m_drawUd2 = ud2;
	m_drawUd3 = ud3;
	
	return TRUE;
}

BOOL CVideoRender::SetMonitorInfo(st_Monitor_Info* pMI)
{
//	EnterCriticalSection(&m_csMT);
	m_csMT.Lock();
	
	m_monitor = pMI;
	
//	LeaveCriticalSection(&m_csMT);
	m_csMT.UnLock();

	return TRUE;
}

BOOL  CVideoRender::AdjustColor(unsigned char brightness, unsigned char contrast, 
				  unsigned char hue, unsigned char saturation)
{
	if (!m_bPlaying)
	{
		return FALSE;
	}
	
	return PLAY_SetColor(m_iPlayPort, 0, 
		(brightness+1)/2, (contrast+1)/2, 
		(saturation+1)/2, (hue+1)/2);
}

BOOL  CVideoRender::GetColorParam(unsigned char *brightness, unsigned char *contrast, 
        unsigned char *hue, unsigned char *saturation)
{
	if (!m_bPlaying)
	{
		return FALSE;
	}

	if (NULL == brightness || NULL == contrast 
		|| NULL == hue || NULL == saturation)
	{
		return FALSE;
	}
	
	int b;
	int c;
	int h;
	int s;

	BOOL bRet = PLAY_GetColor(m_iPlayPort, 0, 
		&b, &c, &s, &h);
	if (!bRet) 
	{
		return FALSE;
	}
	
	*brightness	=	(b==0) ? 0 : (b*2)-1;
	*contrast	=	(c==0) ? 0 : (c*2)-1;
	*hue		=	(h==0) ? 0 : (h*2)-1;
	*saturation	=	(s==0) ? 0 : (s*2)-1;

	return TRUE;
}


//Begin: Add by li_deming(11517) 2008-1-14
BOOL CVideoRender::OpenAudioRecord(pCallFunction pProc, long nBitsPerSample, long nSamplesPerSec, long nLength, long nReserved, long nUser)
{
	return PLAY_OpenAudioRecord(pProc,nBitsPerSample,nSamplesPerSec,nLength,nReserved,nUser);
}

BOOL CVideoRender::CloseAudioRecord()
{
	return PLAY_CloseAudioRecord();
}
//End:li_deming(11517)

#else	//linux ,暂无实现

CVideoRender::CVideoRender(HWND hWnd):m_hWnd(hWnd)
{
}

CVideoRender::~CVideoRender()
{
}

void CALLBACK MyDrawFunc(long nPort,HDC hDc,LONG nUser)
{
}

void CVideoRender::MyDrawFunc_Imp(long nPort,HDC hDc)
{
}

void CALLBACK MyDisplayFunc(long nPort, char* pBuf, long nSize, long nWidth, long nHeight, long nStamp, long nType, long nReserved)
{
}

void CVideoRender::MyDisplayFunc_Imp(long nPort, char* pBuf, long nSize, long nWidth, long nHeight, long nStamp, long nType)
{
}

void CALLBACK MyDemuxFunc(long nPort, char* pBuf, long nSize, void* pParam, long nReserved, long nUser)
{
}

void CVideoRender::MyDemuxFunc_Imp(long nPort, char* pBuf, long nSize, void* pParam, long nReserved)
{
}

void CALLBACK MyAudioFunc(long nPort, char * pAudioBuf, long nSize, long nStamp, long nType, long nUser)
{
}

void CVideoRender::MyAudioFunc_Imp(long nPort, char * pAudioBuf, long nSize, long nStamp, long nType)
{
}

//开始/停止解码
int  CVideoRender::StartDec(BOOL framectrl, DWORD dwTalkType)
{
	return 0;
}

int  CVideoRender::StopDec()
{
	return 0;
}

//音频函数
BOOL  CVideoRender::OpenAudio()
{
	return TRUE;
}

BOOL  CVideoRender::CloseAudio()
{
	return TRUE;
}

BOOL  CVideoRender::IsAudio()
{
	return TRUE;
}

BOOL  CVideoRender::SetAudioVolume(int nVolume)
{
	return TRUE;
}


//获取帧率
int   CVideoRender::GetFrameRate()
{
	return 0;
}

//帧率控制
BOOL  CVideoRender::SetFrameRate(int framerate)
{
	return TRUE;
}

BOOL  CVideoRender::Slow()
{
	return TRUE;
}

BOOL  CVideoRender::Fast()
{
	return TRUE;
}

BOOL  CVideoRender::Pause(BOOL bPause)
{
	return TRUE;
}


BOOL  CVideoRender::PlayNormal()
{
	return TRUE;
}

BOOL  CVideoRender::Step(BOOL bStop)
{
	return TRUE;
}

//抓图
BOOL  CVideoRender::SnapPicture(const char *pchFileName)
{
	return TRUE;
}

//闲置render资源再次被利用时，刷新一些成员变量
int  CVideoRender::ChangeHwnd(HWND hWnd)
{
	return 0;
}


//填充多媒体数据
BOOL  CVideoRender::Play(unsigned char *pBuf, int nLen)
{
	return TRUE;
}

//清空解码缓存
int  CVideoRender::Reset()
{
	return 0;
}

//判断解码缓存是否为空
BOOL  CVideoRender::IsEmpty()
{
	return FALSE;
}

//获取解码缓存中未解码的数据大小
DWORD CVideoRender::GetSourceBufferRemain()
{
	return 0;
}

//获取OSD时间，接口函数
BOOL  CVideoRender::GetOSDTime(unsigned long *year, unsigned long *month, unsigned long *day,
				 unsigned long *hour, unsigned long *minute, unsigned long *second)
{
	return TRUE;
}

BOOL  CVideoRender::SetDrawCallBack(void* pDrawCBFunc, void* ud1, void* ud2, void* ud3)
{
	return TRUE;
}

BOOL CVideoRender::SetMonitorInfo(st_Monitor_Info* pMI)
{
	return TRUE;
}

BOOL  CVideoRender::AdjustColor(unsigned char brightness, unsigned char contrast, 
				  unsigned char hue, unsigned char saturation)
{
	return TRUE;
}

BOOL  CVideoRender::GetColorParam(unsigned char *brightness, unsigned char *contrast, 
        unsigned char *hue, unsigned char *saturation)
{
	return TRUE;
}

BOOL  CVideoRender::AdjustFluency(int nLevel)
{
	return FALSE;
}

#endif

