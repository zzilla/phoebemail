  /********************************************************************
	created:	2006/01/10
	created:	10:1:2006   8:53
	filename: 	playmanage.cpp
	file base:	playmanage
	file ext:	cpp
	author:		chenmy
	
	purpose:	统一管理当前播放和记录播放的状态
*********************************************************************/
#include "StdAfx.h"
#include "playmanage.h"
#include <ddraw.h>
#include "data.h"

#define XVID_CPU_MMX		0x00000001
#define XVID_CPU_MMXEXT		0x00000002
#define XVID_CPU_SSE		0x00000004
#define XVID_CPU_SSE2		0x00000008
#define XVID_CPU_3DNOW		0x00000010
#define XVID_CPU_3DNOWEXT	0x00000020
#define XVID_CPU_TSC		0x00000040

extern "C" int check_cpu_features(void);

CDHPLAY_MANAGE	g_cDHPlayManage ;

//初始化系统，并取得系统信息
void CDHPLAY_MANAGE::init_sysinfo()
{
	//取得系统支持的信息 按位取
	int status = 0;         //0--是否支持DDRAW 1--是否支持BLT操作 2--是否支持颜色转换 3--是否支持图像缩放
// 	LPDIRECTDRAW7 lpDD ;
// 	DDCAPS caps ;

	status |= SUPPORT_DDRAW;
	status |= SUPPORT_BLT;
	status |= SUPPORT_BLTFOURCC;
	status |= SUPPORT_BLTSHRINKX ;
	status |= SUPPORT_BLTSTRETCHX ;
	status |= SUPPORT_BLTSHRINKY ;
	status |= SUPPORT_BLTSTRETCHY ;

/*	
	if (DirectDrawCreateEx( NULL, (void**)&lpDD, IID_IDirectDraw7,NULL ) == DD_OK)
	{
		ZeroMemory( &caps,sizeof( caps ) );
		caps.dwSize = sizeof(caps);
		
		status |= SUPPORT_DDRAW;

		if( FAILED( lpDD->GetCaps( &caps,NULL ) ) )
		{
		}
		else;
		if (caps.dwCaps  & DDCAPS_BLT )
		{
			status |= SUPPORT_BLT;
		}
		else;

		if (caps.dwCaps  & DDCAPS_BLTFOURCC )
		{			
			status |= SUPPORT_BLTFOURCC;
		}
		else;

		if (caps.dwFXCaps  & DDFXCAPS_BLTSHRINKX  )
		{
			 status |= SUPPORT_BLTSHRINKX ;
			 //status |= SUPPORT_BLTSHRINKY ;
		}
		else;
		
		if (caps.dwFXCaps  & DDFXCAPS_BLTSTRETCHX )
		{
			 status |= SUPPORT_BLTSTRETCHX ;
			 //status |= SUPPORT_BLTSTRETCHY ;
		}
		else;

		if (caps.dwFXCaps  & DDFXCAPS_BLTSHRINKY )
		{		
			status |= SUPPORT_BLTSHRINKY ;
		}
		else;

		if (caps.dwFXCaps  & DDFXCAPS_BLTSTRETCHY )
		{
			status |= SUPPORT_BLTSTRETCHY ;
		}
		else;
	}
*/
	int cpuid = check_cpu_features();
	
	if (cpuid & XVID_CPU_MMX)
	{
		status |= SUPPORT_MMX;
	}
	else;
		
	if (cpuid & XVID_CPU_SSE)
	{
		status |= SUPPORT_SSE;
	}
	else;

	m_pSysInfo.nstatus = status ;
	//高16位表示当前的build号。9~16位表示主版本号，1~8位表示次版本号。
#ifdef DHVECPLAY
	m_pSysInfo.dwVersion = (BUILD_VER << 16) | VERSION ;
#else
	m_pSysInfo.dwVersion = (BUILD_VER << 16) | (VERSION << 8) | MEND_VER;    
#endif
}

static int GetModulePathLen(char* cFileName)
{
	int pos = 0;

	for (;pos < 1024; pos++)
	{
		if (memcmp(cFileName+pos, "playsdk.dll", 10) == 0)
		{
			break;
		}
	}

	if (pos == 1024)
	{
		pos = -1;
	}

	return pos;
}

static char cFileName[1024+256];
//创建播放管理类
CDHPLAY_MANAGE::CDHPLAY_MANAGE()
{
	int i;
	
	m_nSoundPort = -1;                    //当前音频端口	
	m_nTimer1Cnt = 16;

	int picQuality = 0;
	int fluency = 4;
	
	char val[100];

	HMODULE  hModule = GetModuleHandle("playsdk.dll");

	memset(m_DLLPath, 0, 256+1024);
	GetModuleFileName(hModule, m_DLLPath, 256+1024);

	int pos = GetModulePathLen(m_DLLPath);
	if (pos != -1)
	{
		memset(m_DLLPath+pos, 0, 256+1024-pos);
		memcpy(cFileName, m_DLLPath, pos);
		memcpy(cFileName+pos, "playConfig.ini", 14);

		memset(val, 0, 100);
		GetPrivateProfileString("String", "PicQuality","0",
			val, sizeof(val), cFileName);
		picQuality = atoi(val);

		memset(val, 0, 100);
		GetPrivateProfileString("String", "Fluency","4",
			val, sizeof(val), cFileName);
		fluency = atoi(val);
	}
	
	for(i = 0; i < FUNC_MAX_PORT; i++)
	{
		pDHPlay[i]				= NULL;    //播放对象指针表
		pDHFile[i]				= NULL;
		pDisplay[i]             = NULL;
		pCallback[i]			= NULL;
		m_error[i]				= DH_PLAY_NOERROR;
		pMsgFileEnd[i]			= NULL;
		pMsgEncChang[i]			= NULL;
		m_nQuality[i]			= picQuality;	
		m_nStreamMode[i]		= STREAME_REALTIME;
		m_nFluency[i]			= fluency;
		m_ePortState[i]			= PORT_STATE_FREE;
		m_dwTimerType[i]		= (DWORD)ACTimerType_Auto;
		InitializeCriticalSection(&m_interfaceCritSec[i]);
	}

	InitializeCriticalSection(&m_PortStateCrit);
	
	pAudioRecored = NULL;

	m_supportmultidevice = 0;

	init_sysinfo();                //系统信息，包括显卡、声卡等一些系统信息	
}
//释放播放管理类
CDHPLAY_MANAGE::~CDHPLAY_MANAGE()
{
	for (int i = 0 ; i < FUNC_MAX_PORT ; i++)
	{
		DeleteCriticalSection(&m_interfaceCritSec[i]);

		if (pDHPlay[i])
		{
// 			pDHPlay[i]->Stop() ;
			delete pDHPlay[i];
			pDHPlay[i] = NULL ;
		}
		if (pDHFile[i])
		{
			delete pDHFile[i] ;
			pDHFile[i] = NULL ;
		}
		if (pDisplay[i])
		{
			delete pDisplay[i] ;
			pDisplay[i] = NULL ;
		}
		if (pMsgEncChang[i])
		{
			delete pMsgEncChang[i] ;
			pMsgEncChang[i] = NULL ;
		}
		if (pMsgFileEnd[i])
		{
			delete pMsgFileEnd[i] ;
			pMsgFileEnd[i] = NULL ;
		}
		if (pMsgEncChang[i])
		{
			delete pMsgEncChang[i]; 
			pMsgEncChang[i] = NULL ;
		}
		if (pCallback[i])
		{
			delete pCallback[i];
			pCallback[i] = NULL;
		}

		m_ePortState[i] = PORT_STATE_FREE;
	}

	DeleteCriticalSection(&m_PortStateCrit);

	m_nShareSoundPortList.clear() ;

	if (pAudioRecored)
	{
		pAudioRecored->Stop();
		pAudioRecored = NULL;
	}
}
//检测端口是否可进行正常控制操作，0正常，1不正常
int  CDHPLAY_MANAGE::CheckPort(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return 1 ;
	}

	if (!pDHFile[nPort])
	{
		pDHFile[nPort] = new CDHAVData() ;
	}
	if (!pDisplay[nPort])
	{
		pDisplay[nPort] = new CDisplay(nPort) ;
	}
	if (!pMsgEncChang[nPort])
	{
		pMsgEncChang[nPort] = new MSG_INFO ;
	}
	if (!pMsgFileEnd[nPort])
	{
		pMsgFileEnd[nPort] = new MSG_INFO ;
	}
	if (!pDHPlay[nPort])  //还没创建播放时创建播放指针
	{		
		pDHPlay[nPort] = new CDHPlay(nPort);
		pDHPlay[nPort]->m_nTimerType = m_dwTimerType[nPort];
	}

	if (!pDHFile[nPort] || !pDisplay[nPort] || !pMsgEncChang[nPort] 
		|| !pMsgFileEnd[nPort] || !pDHPlay[nPort])
	{
		m_error[nPort] = DH_PLAY_ALLOC_MEMORY_ERROR;
		
		if (pDHFile[nPort])
		{
			delete pDHFile[nPort];
			pDHFile[nPort] = NULL;
		}

		if (pMsgEncChang[nPort])
		{
			delete pMsgEncChang[nPort];
			pMsgEncChang[nPort] = NULL;
		}

		if (pMsgFileEnd[nPort])
		{
			delete pMsgFileEnd[nPort];
			pMsgFileEnd[nPort] = NULL;
		}
		
		if (pDHPlay[nPort]) 
		{
			delete pDHPlay[nPort];
			pDHPlay[nPort] = NULL;
		}	

		return 1;
	}

// 	if (PORT_STATE_FREE == m_ePortState[nPort])
// 	{
// 		m_ePortState[nPort] = PORT_STATE_BUSY;
// 	}

	return 0;
}
//获取当前系统信息，按位取；
int  CDHPLAY_MANAGE::GetCaps()           
{
	return m_pSysInfo.nstatus;
}

//获取错误码
DWORD CDHPLAY_MANAGE::GetError(LONG nPort)
{
	return m_error[nPort];
}
 //获取版本号
DWORD CDHPLAY_MANAGE::GetSdkVersion()
{
	return m_pSysInfo.dwVersion;
}  
 //获取文件头的长度
DWORD CDHPLAY_MANAGE::GetFileHeadLenth()
{
	return sizeof(DH_VIDEOFILE_HDR);
}   

//
// 设置端口状态
//
BOOL CDHPLAY_MANAGE::SetPortState(LONG lPort, PORT_STATE eState)
{
	//端口号超出范围
	if (lPort < 0 || lPort >= FUNC_MAX_PORT)
	{
		return FALSE;
	}

	EnterCriticalSection(&m_PortStateCrit);
	m_ePortState[lPort] = eState;
	LeaveCriticalSection(&m_PortStateCrit);

	return TRUE;
}

//
// 获取端口状态
//
BOOL CDHPLAY_MANAGE::GetPortState(LONG lPort, PORT_STATE *peState)
{
	//端口号超出范围
	if (lPort < 0 || lPort >= FUNC_MAX_PORT)
	{
		return FALSE;
	}

	EnterCriticalSection(&m_PortStateCrit);
	*peState = m_ePortState[lPort];
	LeaveCriticalSection(&m_PortStateCrit);

	return TRUE;
}

int CDHPLAY_MANAGE::OpenYuvRender(int port, HWND hwnd, void (CALLBACK* DrawFun)(int nPort,HDC hDc))
{
	int nIndex = FUNC_MAX_PORT*MULTI_DISPLAY_RECT_NUM + port;

	VideoRenderMethod eDisType = ByDDOffscreen; 
	if (m_YUVRenderMng.openVideo(nIndex, hwnd, 352, 288, DrawFun, eDisType, 0) != 0)    //打开回放窗口
	{
		//如果用该显示模式打开不了,那么用offscreen 再显示一次
		eDisType = ByDDOffscreen;	
		if (m_YUVRenderMng.openVideo(nIndex, hwnd, 352, 288, DrawFun, eDisType, 0) != 0)
		{						
			//如果用offscreen还显示不了,那么只能用GDI显示
			eDisType = ByGDI;	
			if(m_YUVRenderMng.openVideo(nIndex, hwnd, 352, 288, DrawFun, eDisType, 0) != 0)
			{
				//如果还是不行,那么返回失败
				return 9; //DH_PLAY_CREATE_DDRAW_ERROR
			}
			else;
		}
		else;
	}
	
	return 0;
}

int CDHPLAY_MANAGE::RenderYuv(int port, unsigned char* py, unsigned char* pu, unsigned char* pv, int width, int height)
{
	int nRet = m_YUVRenderMng.renderVideo(py, pu, pv, width, height, 0) ;//显示一桢图像
	
	return nRet;
}

int CDHPLAY_MANAGE::CloseYuvRender(int port)
{
	m_YUVRenderMng.close() ;
	
	return 0;
}