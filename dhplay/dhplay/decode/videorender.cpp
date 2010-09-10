#include <windows.h>
#include <math.h>
#include <stdio.h>
#include "videorender.h"
#include "decode.h"


#ifdef _DEBUG
#define __Debug_String1(_t, _v)  {char str[120]; sprintf(str, (_t), (_v)); OutputDebugString(str);}
#else
#define __Debug_String1(_t, _v)
#endif


#define PAL_RATE					25 // PAL制标准帧率
#define NTSC_RATE					30 // NTSC标准帧率
#define RENDERTIME					3
#define TIMER_RESOLUTION			3  // 定期期精度，毫秒为单位。经过测试，3ms级可以保证精度，又可以降低CPU开销

#define IVSINFOTYPE_PRESETPOS		1
#define IVSINFOTYPE_MOTINTRKS		2


bool TimeKillSynchronousFlagAvailable( void )
{
    OSVERSIONINFO osverinfo;
	
    osverinfo.dwOSVersionInfoSize = sizeof(osverinfo);
	
    if( GetVersionEx( &osverinfo ) ) {
        
        // Windows XP's major version is 5 and its' minor version is 1.
        // timeSetEvent() started supporting the TIME_KILL_SYNCHRONOUS flag
        // in Windows XP.
        if( (osverinfo.dwMajorVersion > 5) || 
            ( (osverinfo.dwMajorVersion == 5) && (osverinfo.dwMinorVersion >= 1) ) ) {
            return true;
        }
    }
	
    return false;
}


DhVideoRender::DhVideoRender()
{
	m_interval	= 1000/PAL_RATE;
	m_tmpinterval = m_interval ;
	m_rate    = PAL_RATE ;
	m_callback = NULL ;
	m_timerId = 0 ;
	m_ifPause = 1 ;
	m_item = new DhAVFrame ;
	memset(m_item, 0, sizeof(DhAVFrame));
	m_item->context = 0;
	m_item->width = 0 ;
	m_item->height= 0 ;

	m_item->ivsobjnum  = 0;
	m_item->ivsObjList = new DH_IVS_OBJ[MAX_IVSOBJ_NUM];

	m_pACTimer = NULL;
	InitializeCriticalSection(&m_CritSec);
	InitializeCriticalSection(&m_ResetTimerCritSec);
	m_bTimerDisable = FALSE;

	m_hMMTimerEvent  = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_iMMTimerSigRef = 0;

	m_timertype = _TIMER_1;
	m_hTimerThread = NULL;
	m_time2_state = 1;
	m_flagvalue = 0;
	m_flagnum = 0;

	m_audio_state = 1;
	m_audioThread = NULL;

	m_IVSCallback = NULL;
	m_IVS_user = 0;

	m_iIntervalSum = 0;
	m_iFrameLenSum = 0;
	m_dRealBitRate = 0;

#ifdef THREAD_RENDER
	m_hVideoRender = CreateEvent(NULL, FALSE, FALSE, NULL);
#endif
}

void DhVideoRender::SetPause(int value)
{
	m_ifPause = value > 0 ;
}

int   DhVideoRender::GetPause() 
{
	return m_ifPause ;
}

DhVideoRender::~DhVideoRender()
{
    clear();

	if (NULL != m_item)
	{
		SAFE_DELETEBUFF(m_item->ivsObjList);
		SAFE_DELETEBUFF(m_item->tIvsPrePos.pIVSPreposInfo);
		SAFE_DELETE(m_item);
	}

	m_IVSCallback = NULL;
	m_IVS_user = 0;
	
	DeleteCriticalSection(&m_CritSec);
	DeleteCriticalSection(&m_ResetTimerCritSec);

	CloseHandle(m_hMMTimerEvent);
	m_hMMTimerEvent  = NULL;
	m_iMMTimerSigRef = 0;

#ifdef THREAD_RENDER
	CloseHandle(m_hVideoRender);
#endif
}

int DhVideoRender::init(int port, int imgcount,int chunksize, dec_callback cb)
{
	m_YuvDataList.clear() ;
	int ret = m_YuvDataList.init(imgcount,chunksize);
	if (ret > 0)
	{
#ifdef _DEBUG
		OutputDebugString("m_YuvDataList.init fail!!!!!\n");
#endif
		return ret;
	}

	m_PcmDataList.clear();
	ret = m_PcmDataList.init(10, AUDIOBUFLEN);
	if (ret > 0)
	{
#ifdef _DEBUG
		OutputDebugString("m_PcmDataList.init fail!!!!!\n");
#endif
		return ret;
	}

	m_callback = cb ;
	m_port = port ;

	m_interval	= 1000/PAL_RATE;
	m_tmpinterval = m_interval ;
	m_rate    = PAL_RATE ;
	m_timerId = 0;

	m_ifPause = 1 ;
	m_timertype = _TIMER_1;
	m_hTimerThread = NULL;
	m_time2_state = 1;
	m_flagvalue = 0;
	m_flagnum = 0;

  	m_audio_state = 1;
	m_audioThread = NULL;
	m_flagcount = 0;

 	m_preFrameTime = timeGetTime();

	m_bTimerDisable = FALSE;

	m_iIntervalSum = 0;
	m_iFrameLenSum = 0;
	m_dRealBitRate = 0;

   return 0;
}

int DhVideoRender::clear()
{
	m_YuvDataList.clear() ;
	m_PcmDataList.clear();

	m_ifPause = 1 ;

	if (m_audioThread)
	{
		m_audio_state = 0;
		WaitForSingleObject(m_audioThread, INFINITE);
		CloseHandle(m_audioThread);
		m_audioThread = NULL;
	}

	if (m_hTimerThread)
	{
		m_time2_state = 0;
		WaitForSingleObject(m_hTimerThread, INFINITE);
		CloseHandle(m_hTimerThread);
		m_hTimerThread = NULL;
	}

	return 0;
}

int DhVideoRender::call_render() 
{
	if (m_ifPause)
	{
		return 0 ;
	}

	render() ;	

	return 0 ;
}
#include "../dhplay.h"

int DhVideoRender::render()
{	
	BOOL iRet = m_YuvDataList.read(m_item) ;
	DWORD curTime = timeGetTime();
	
	if (iRet == FALSE)
	{
// #ifdef _DEBUG
// //		OutputDebugString("read null 解码后缓冲区数据为空\n") ;
// 		char str[100];
// 		sprintf(str, "解码前缓冲为Port = %d, length = %d\n", m_port, PLAY_GetSourceBufferRemain(m_port));
// 		OutputDebugString(str) ;
// #endif
 		m_preFrameTime = curTime;
		return 1 ;
	}

#ifdef _DEBUG
	char str[100];
	sprintf(str, "Port = %d, Frame present interval = %d\n", m_port, curTime - m_preFrameTime);
//	OutputDebugString(str) ;

#endif

	// If I frame
	if ((m_item->frameType == 0) && (m_iFrameLenSum != 0))
	{
		m_dRealBitRate = ((double)m_iFrameLenSum*8000.0/(double)m_iIntervalSum)/1024.0;
		m_iFrameLenSum = m_item->frameLen;
		m_iIntervalSum = m_interval;
	}
	else
	{
		m_iFrameLenSum += m_item->frameLen;
		m_iIntervalSum += m_interval;
	}

	//回调智能分析轨迹信息
	if (m_IVSCallback != NULL)
	{
		// 预置点信息
		if (m_item->tIvsPrePos.lInfoSize > 0)
		{
			m_IVSCallback((char*)(m_item->tIvsPrePos.pIVSPreposInfo),
				IVSINFOTYPE_PRESETPOS, 
				4, m_item->tIvsPrePos.lInfoSize,
				m_port, m_IVS_user);
		}

		// 物体轨迹信息
		if (m_item->ivsobjnum > 0)
		{
			m_IVSCallback((char*)(m_item->ivsObjList), 
				          IVSINFOTYPE_MOTINTRKS, 
						  sizeof(DH_IVS_OBJ), sizeof(DH_IVS_OBJ)*m_item->ivsobjnum,
						  1, m_IVS_user);
		}
		else if (m_item->ivsobjnum < 0)
		{
			m_IVSCallback((char*)(m_item->ivsObjList), 
				          IVSINFOTYPE_MOTINTRKS, 
						  sizeof(DH_IVS_OBJ), sizeof(DH_IVS_OBJ)*m_item->ivsobjnum,
						  2, m_IVS_user);
		}
	}

	m_callback(m_port,m_item->context,m_item->width*m_item->height*3/2,DEC_YUVDATA,m_item->width,m_item->height) ;

	m_preFrameTime = curTime;

	return 0;
}

#ifdef THREAD_RENDER
DWORD WINAPI video_render_proc(LPVOID pParam)  
{
	DhVideoRender *video_render  = (DhVideoRender*)pParam ;
	
	while (video_render->m_timerId)
	{
		WaitForSingleObject(video_render->m_hVideoRender, INFINITE);
		video_render->call_render() ;		
	}
	
	return 0 ;
}
#endif

void CALLBACK timer_proc(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	DhVideoRender *video_render = (DhVideoRender*)dwUser;

	if (!video_render)
	{
		return;
	}

 	__try 
	{		
		video_render->call_render() ;		
	}
	
	__except (0, 1)
	{
		int zgf = 0;
	}
}

DWORD  DhVideoRender::timer(LPVOID pParam)  
{
	DhVideoRender* dhtimer = (DhVideoRender*)pParam;

	if (0 == dhtimer)
	{
		return 1;
	}
	
	while (dhtimer->m_time2_state)
	{
		__try 
		{		
			dhtimer->call_render();	
		}

		__except (0, 1)
		{
			int zgf = 0;
		}

		dhtimer->m_flagvalue = (dhtimer->m_flagvalue) % 15 + 1;//从1-15
		if (dhtimer->m_flagvalue == 1)
		{
			dhtimer->m_flagnum = 0;
		}
		
		int interval_down = (dhtimer->m_interval / 15) * 15;
		int interval_up = interval_down + 15;
		int down_num = 15 - (dhtimer->m_interval - interval_down);
		int up_num = 15 - down_num;

		if (up_num == 0)
		{
			Sleep(interval_down);
			continue;
		}

		if (down_num == 0)
		{
			Sleep(interval_up);
			continue;
		}

		int RealInternal = 40;

		if (down_num > up_num)
		{
			int a = down_num/up_num;
			if ((dhtimer->m_flagvalue % (a+1) == 0) && dhtimer->m_flagnum < up_num)
			{
				RealInternal = interval_up;
				dhtimer->m_flagnum++;
			}
			else
			{
				RealInternal = interval_down;
			}
		}
		else 
		{
			int a = up_num/down_num;
			if ((dhtimer->m_flagvalue % (a+1) == 0) && dhtimer->m_flagnum < down_num)
			{
				RealInternal = interval_down;
				dhtimer->m_flagnum++;
			}
			else
			{
				RealInternal = interval_up;
			}
		}

		Sleep(RealInternal);
	}

	return 0 ;
}

int DhVideoRender::startTimer()
{
	if (m_timerId != 0)
	{
		return -1;
	}

	m_timerId = m_pACTimer->SetACTimer(m_interval, timer_proc, DWORD(this));

#ifdef THREAD_RENDER
	DWORD dwThreadId;
	m_hTimerThread = CreateThread(NULL, 0, video_render_proc, this, 0, &dwThreadId);
#endif

	return 0;
}

void DhVideoRender::stopTimer()
{

	BOOL bRet = m_pACTimer->KillACTimer(m_timerId);
	if (!bRet)
	{
#ifdef _DEBUG
		char str[120];
		SYSTEMTIME time;
		GetLocalTime(&time);
		sprintf(str,"Port %d killTimer %d fail!! ref= %d at %d:%d:%d:%d:%d\n", m_port, m_timerId, m_iMMTimerSigRef, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds) ; 
		OutputDebugString(str) ;
#endif
	}

	m_timerId = 0;

#ifdef THREAD_RENDER
	SetEvent(m_hVideoRender);
	WaitForSingleObject(m_hTimerThread,INFINITE);
	if (m_hTimerThread)
	{
		CloseHandle(m_hTimerThread);
		m_hTimerThread = NULL;
	}
#endif

}

void DhVideoRender::StopTimerSafely(void)
{
	stopTimer();
}

int DhVideoRender::changeRate(int rate)
{
	if (rate <= 0 || rate > 250)
	{
		return 1 ;
	}

	m_tmpinterval = 1000 / rate ;

	if (_TIMER_2 == m_timertype)
	{
		m_interval = m_tmpinterval - RENDERTIME;
	}

	m_rate = rate ;

	m_interval = m_tmpinterval ;
	m_pACTimer->ResetACTimer(m_timerId, m_interval);

	return 0 ;
}

int DhVideoRender::changeInterval(int interval)
{
	if (interval < 4 || interval > 160000)
	{
		return -1;
	}

	m_tmpinterval = interval;

	if (_TIMER_2 == m_timertype)
	{
		m_interval = m_tmpinterval - RENDERTIME;
	}

	m_interval = m_tmpinterval ;
	m_pACTimer->ResetACTimer(m_timerId, m_interval);

	m_rate = 1000/m_tmpinterval;

	return 0;
}

int DhVideoRender::getRate()
{
	// 返回真正的帧率
	return m_rate;
}

int  DhVideoRender::StopSoundThread()
{
	if (m_audioThread)
	{
		m_audio_state = 0;
		WaitForSingleObject(m_audioThread, INFINITE);
		CloseHandle(m_audioThread);
		m_audioThread = NULL;
	}

	return 0;
}

DWORD  DhVideoRender::audiothread(LPVOID pParam) 
{
	DhVideoRender *video_render = (DhVideoRender*)pParam;

	DhAVFrame *tmpframe = new DhAVFrame;
	memset(tmpframe, 0, sizeof(DhAVFrame));

	tmpframe->context = new unsigned char[AUDIOBUFLEN];

	while (video_render && video_render->m_audio_state)
	{
		if (video_render->m_ifPause)
		{
			Sleep(15);
			continue ;
		}
		
		BOOL iRet = video_render->m_PcmDataList.read(tmpframe) ;

		if (iRet == FALSE)
		{
			Sleep(15);
			continue ;
		}

		__try
		{
			video_render->m_callback(video_render->m_port,tmpframe->context,tmpframe->size,DEC_PCMDATA,tmpframe->width,tmpframe->height);
		}
		__except(0,1)
		{
			int wlj = 0;
		}
	}
	
	return 0;
}

int  DhVideoRender::StartSoundThread()
{
	if (m_audioThread == NULL)
	{
		m_audio_state = 1;
		
		DWORD dwThreadId;
		m_audioThread = CreateThread(NULL, 0, audiothread, this, 0, &dwThreadId);
		SetThreadPriority(m_audioThread, THREAD_PRIORITY_HIGHEST) ;

	}

	return 0;
}
