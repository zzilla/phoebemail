 /*
**  ************************************************************************
**                                     DHPlay  通用播放SDK

**         (c) Copyright 1992-2004, ZheJiang Dahua Technology Stock Co.Ltd.
**                            All Rights Reserved
**
**	File  Name	: dhpaly.cpp
**	Description	: 播放sdk外部接口实现
**	Modification	: 2005/12/15		chenmy		Create the file
**  2005/12/29   	chenmy	改变用户回调函数的处理，将回调函数统一在各端口的play中处理 
**  2006/01/10   	chenmy	将回调函数的处理独立出来，统一在callback中处理  
**  2006/01/11   	chenmy	增加音量控制处理类接口，实现控制音量的相关功能   
**  ************************************************************************
*/

#include "StdAfx.h"
#include "utils.h"

#define DHPLAY_EX
#if defined DHVECPLAY
	#include "dhvecplay.h"
#elif defined DHPLAY_EX
	#include "dhplayEx.h"
#else
	#include "dhplay.h"
#endif

#include "playmanage.h"
#include <ddraw.h>
#include "YUV2PICS/JpegEncoder.h"


static int g_play[FUNC_MAX_PORT];
CritSec g_PlayCritsec ;


#if defined( _WINDLL)
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	int i = 0;
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			for (i = 0; i < FUNC_MAX_PORT; ++i) 
			{
				g_play[i] = 0;
			}
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
#endif


int img_conv(unsigned char* psrc, unsigned int srcwidth, unsigned int srcheight, 
			 unsigned char* pdst, unsigned int dstwidth, unsigned int dstheight);


//以下为对外接口，供用户作二次开发调用
PLAYSDK_API BOOL CALLMETHOD PLAY_InitDDraw(HWND hWnd)
{
	return TRUE;
}

PLAYSDK_API BOOL CALLMETHOD PLAY_RealeseDDraw()
{
	return TRUE;
}

/*	$Function		:	CALLMETHOD PLAY_OpenFile
==  ===============================================================
==	Description		:	打开播放文件，用于在播放文件之前调用
==	Argument		:	nPort 端口号   sFileName 文件名
==				:	
== 	Return		:	    成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_OpenFile(LONG nPort,LPSTR sFileName)
{
#ifdef _DEBUG
	char str[120] ;
	sprintf(str,"PLAY_OpenFile Enter %d\n",nPort) ; 
	OutputDebugString(str) ;
#endif

	if (g_cDHPlayManage.CheckPort(nPort))
	{
		return FALSE;
	}

	EnterCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

	FILE_OPEN_ERROR nReturn = g_cDHPlayManage.pDHFile[nPort]->Open(nPort, sFileName);
	
	if (nReturn == FILE_CANNOT_OPEN)
	{
		LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);
		return FALSE ;
	}

	LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

#ifdef _DEBUG
// 	char str[120] ;
	sprintf(str,"PLAY_OpenFile Leave %d\n",nPort) ; 
	OutputDebugString(str) ;
#endif

	return TRUE ;
}

/*	$Function		:	CALLMETHOD DH_Play_CloseFile
==  ===============================================================
==	Description		:	关闭文件
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	    成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_CloseFile(LONG nPort)
{
#ifdef _DEBUG
	char str[120] ;
	sprintf(str,"PLAY_CloseFile Enter %d\n",nPort) ; 
	OutputDebugString(str) ;
#endif

	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	EnterCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

	if (g_cDHPlayManage.pDHFile[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);
		return FALSE ;
	}

	BOOL nReturn = g_cDHPlayManage.pDHFile[nPort]->Close();

	delete g_cDHPlayManage.pDHFile[nPort] ;
	g_cDHPlayManage.pDHFile[nPort] = NULL ;

	g_cDHPlayManage.m_dwTimerType[nPort] = 0;

	PLAY_ReleasePort(nPort);

	LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

#ifdef _DEBUG
// 	char str[120] ;
	sprintf(str,"PLAY_CloseFile Leave %d\n",nPort) ; 
	OutputDebugString(str) ;
#endif

 	return nReturn ;
}

void WINAPI CallFunction(LPBYTE pDataBuffer, DWORD DataLength, long nUser)
{
	static unsigned char PCMHeader[8] = {0x00,0x00,0x01,0xF0,7,0x02,0x00,0x00};
	int len = (DataLength/*>>1*/)&0xffff;
	PCMHeader[6] = len & 0xff;
	PCMHeader[7] = len >> 8;

	unsigned short *p16 = (unsigned short *)pDataBuffer;

	for (int i = 0; i < len; i++)
	{
		pDataBuffer[i] = pDataBuffer[i] + 128;/*BYTE((*p16)>>8);*//*pInBuf[j*2] + 128*/;
//		p16++;
	}
	FILE* f = fopen("c://zgf0718_audio_dhplaydemo_1.dav", "ab");
	fwrite(PCMHeader, 1, 8, f);
	fwrite(pDataBuffer, 1, len, f);
	fclose(f);
}

/*	$Function	 :	PLAY_GetFreePort
==  ===============================================================
==	Description	 :	获取空闲的端口号
==	Parameter	 :	plPort	端口号指针
==				 :	
== 	Return		 :  成功 TRUE, 失败 FALSE
==	Modification :	2008/8/13 
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_GetFreePort(LONG *plPort)
{	
	if (NULL == plPort)
	{
		return FALSE;
	}

	for (int i = 101; i < FUNC_MAX_PORT; i++)
	{
		PORT_STATE ePortState;
		g_cDHPlayManage.GetPortState(i, &ePortState);

		if (PORT_STATE_FREE == ePortState)
		{
			g_cDHPlayManage.SetPortState(i, PORT_STATE_BUSY);
			break;
		}
	}

	if (i >= FUNC_MAX_PORT)
	{
		return FALSE;
	}

	*plPort = i;
	return TRUE;
}

/*	$Function	 :	PLAY_ReleasePort
==  ===============================================================
==	Description	 :	释放端口号
==	Parameter	 :	lPort	端口号
==				 :	
== 	Return		 :  成功 TRUE, 失败 FALSE
==	Modification :	2008/8/13 
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_ReleasePort(LONG lPort)
{
	if (lPort < 0 || lPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	g_cDHPlayManage.SetPortState(lPort, PORT_STATE_FREE);

	return TRUE;
}

/*	$Function		:	CALLMETHOD PLAY_Play
==  ===============================================================
==	Description		:	开始播放文件
==	Argument		:	nPort 端口号 hWnd 播放显示窗口句柄
==				:	
== 	Return		:	    成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_Play(LONG nPort, HWND hWnd)
{
#ifdef _DEBUG
	char str[120] ;
	sprintf(str,"PLAY_Play Enter %d , hwnd = 0x%08X\n",nPort , hWnd) ; 
	OutputDebugString(str) ;
#endif

	if (nPort < 0 || nPort >= FUNC_MAX_PORT)  //端口号超出范围
	{
		return FALSE ;
	}

 	EnterCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

	if (g_cDHPlayManage.pDHFile[nPort] == NULL)//在调用Play之前会称调用openfile、openstream
	{
	 	LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE;
	}

	if (g_cDHPlayManage.CheckPort(nPort))
	{
	 	LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);
		return FALSE;
	}

	g_cDHPlayManage.m_error[nPort] = DH_PLAY_NOERROR;

	BOOL iRet = g_cDHPlayManage.pDHPlay[nPort]->Start(hWnd) ;

	if (iRet)
	{
		g_cDHPlayManage.SetPortState(nPort, PORT_STATE_BUSY);
	}

 	LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

#ifdef _DEBUG
	sprintf(str,"PLAY_Play Leave %d\n",nPort) ; 
	OutputDebugString(str) ;
#endif

 	PLAY_ResetSourceBufFlag(nPort);

  //  	PLAY_OpenAudioRecord(CallFunction, 8, 8000, 1024, 0, NULL);

	return iRet ;
}

/*	$Function		:	CALLMETHOD PLAY_Stop
==  ===============================================================
==	Description		:	停止播放
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	    成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_Stop(LONG nPort)
{
	int iRet;	
	
#ifdef _DEBUG
	char str[120] ;
	sprintf(str,"PLAY_Stop Enter %d , TickCount=%d\n",nPort,GetTickCount()) ; 
	OutputDebugString(str) ;
// 	FILE * fp = fopen("c://dhplay.log", "ab");
// 	if (fp) 
// 	{
// 		fprintf(fp, str);
// 		fclose(fp);
// 	}

#endif

	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	PORT_STATE ePortState;
	g_cDHPlayManage.GetPortState(nPort, &ePortState);
	
	if (PORT_STATE_FREE == ePortState)
	{
		return TRUE;
	}

 	EnterCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
	 	g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
 		LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);
		return FALSE ;
	}

	iRet = g_cDHPlayManage.pDHPlay[nPort]->Stop();

	if (g_cDHPlayManage.pCallback[nPort])
	{
		g_cDHPlayManage.pDHPlay[nPort]->SetDecCBType(DEC_COMPLEX) ;
		g_cDHPlayManage.pCallback[nPort]->SetDecCallBack(NULL,FALSE);
	}
	
	PLAY_StopSoundShare(nPort) ;

	if (g_cDHPlayManage.m_nSoundPort == nPort)
	{
		g_cDHPlayManage.m_nSoundPort = -1 ;
	}

	delete g_cDHPlayManage.pDHPlay[nPort]  ;
	g_cDHPlayManage.pDHPlay[nPort]  = NULL ;
	
 	LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

	return iRet;
}

/*	$Function		:	CALLMETHOD PLAY_Pause
==  ===============================================================
==	Description		:	暂停/恢复播放
==	Argument		:	nPort 端口号   nPause 是否暂停 1 暂停 0 恢复
==				:	
== 	Return		:	    成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_Pause(LONG nPort,DWORD nPause)
{	
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->Pause(nPause);
}

//
/*	$Function		:	CALLMETHOD PLAY_Fast
==  ===============================================================
==	Description		:	快速播放控制，每操作一次播放速度快一倍，最大操作4次，再操作循环
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	    成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_Fast(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}
	
	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->PlayFast() ;
}

/*	$Function		:	CALLMETHOD PLAY_Slow
==  ===============================================================
==	Description		:	慢速播放控制，每操作一次播放速度降低一倍，最大操作4次，再操作循环
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	    成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_Slow(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->PlaySlow() ;
}

/*	$Function		:	CALLMETHOD PLAY_OneByOne
==  ===============================================================
==	Description		:	单桢播放
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	    成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_OneByOne(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->PlayOnebyone(); 
}

/*	$Function		:	CALLMETHOD PLAY_Back
==  ===============================================================
==	Description		:	反向回放，（可以不用此函数，sdk没做要求）
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	    成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_Back(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->PlayBack(); 
}

/*	$Function		:	CALLMETHOD PLAY_BackOne
==  ===============================================================
==	Description		:	单桢反向播放 （此接口重复 真正用CALLMETHOD PLAY264_OneByOneBack）
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	    成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_BackOne(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->PlayBackOne(); 
}

/*	$Function		:	CALLMETHOD PLAY_SetPlayPos
==  ===============================================================
==	Description		:	设置播放位置
==	Argument		:	nPort 端口号  fRelativePos 文件长度的百分比
==				:	
== 	Return		:	    成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetPlayPos(LONG nPort,float fRelativePos)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->SetPlayPos(fRelativePos); 
}

/*	$Function		:	CALLMETHOD PLAY_GetPlayPos
==  ===============================================================
==	Description		:	获取当前播放位置
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	    成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API float CALLMETHOD PLAY_GetPlayPos(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->GetPlayPos(); 
}

/*	$Function		:	CALLMETHOD PLAY_SetFileEndMsg
==  ===============================================================
==	Description		:	设置文件结束时需要发送的消息
==	Argument		:	nPort 端口号   hWnd 消息发送窗口 nMsg 消息
==				:	
== 	Return		:	    成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/

PLAYSDK_API BOOL CALLMETHOD PLAY_SetFileEndMsg(LONG nPort,HWND hWnd,UINT nMsg)
{
	if (g_cDHPlayManage.CheckPort(nPort))
	{
		return FALSE;
	}

	g_cDHPlayManage.pMsgFileEnd[nPort]->hWnd = hWnd ;
	g_cDHPlayManage.pMsgFileEnd[nPort]->nMsg = nMsg ;
	g_cDHPlayManage.pMsgFileEnd[nPort]->nMsgFlag = TRUE ;

	return TRUE ;
}

/*	$Function		:	
==  ===============================================================
==	Description		:	设置音量
==	Argument		:	nPort 端口号  nVolume 音量值
==				:	
== 	Return		:		成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetVolume(LONG nPort,WORD nVolume)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDisplay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	g_cDHPlayManage.pDisplay[nPort]->SetVolume(nVolume) ;

	return TRUE ;
}

/*	$Function		:	CALLMETHOD PLAY_StopSound
==  ===============================================================
==	Description		:	停止播放声音,设置静音
==	Argument		:	
==				:	
== 	Return		:		成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_StopSound()
{
	if (g_cDHPlayManage.m_nShareSoundPortList.size() > 0)
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.m_nSoundPort != -1)
	{
		if (g_cDHPlayManage.pDHPlay[g_cDHPlayManage.m_nSoundPort] == NULL)
		{
			return FALSE ;
		}

		g_cDHPlayManage.pDHPlay[g_cDHPlayManage.m_nSoundPort]->SetDecAudio(FALSE) ;
		g_cDHPlayManage.m_nSoundPort = -1 ;
	}

	return TRUE ;
}

/*	$Function		:	CALLMETHOD PLAY_PlaySound
==  ===============================================================
==	Description		:	恢复声音
==	Argument		:	nPort 端口号
==				:	
== 	Return		:		成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/

PLAYSDK_API BOOL CALLMETHOD PLAY_PlaySound(LONG nPort)
{
	
	EnterCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

	if (g_cDHPlayManage.CheckPort(nPort))
	{
		LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);
		return FALSE;
	}

	if (g_cDHPlayManage.m_nShareSoundPortList.size() > 0)//共享方式和独占方式不能混用
	{
		LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);
		return FALSE ;
	}

	int presoundport = g_cDHPlayManage.m_nSoundPort ;

	if (presoundport != -1)
	{
		if (g_cDHPlayManage.pDHPlay[presoundport])
		{
			g_cDHPlayManage.pDHPlay[presoundport]->SetDecAudio(FALSE) ;
		}
	}
	
	g_cDHPlayManage.m_nSoundPort = nPort ;

	g_cDHPlayManage.pDHPlay[nPort]->SetDecAudio(TRUE) ;

	LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

	return TRUE ;
}

/*	$Function		:	CALLMETHOD PLAY_OpenStream
==  ===============================================================
==	Description		:	打开流播放接口
==	Argument		:	nPt 端口号  pFileHeadBuf 文件头数据指针  nSize 数据长度
==				:		nBufPoolSize 设置接收数据的缓冲的大小
== 	Return		:		成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_OpenStream(LONG nPort,PBYTE pFileHeadBuf,DWORD nSize,DWORD nBufPoolSize)
{
#ifdef _DEBUG
	char str[120] ;
	sprintf(str,"PLAY_OpenStream Enter %d\n",nPort) ; 
	OutputDebugString(str) ;
#endif
	
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

 	EnterCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

	if (g_cDHPlayManage.CheckPort(nPort))
	{
	 	LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);
		return FALSE;
	}

#ifdef _DEBUG
	sprintf(str,"PLAY_OpenStream Leave %d\n",nPort) ; 
	OutputDebugString(str) ;
#endif

	if (nBufPoolSize <= SOURCE_BUF_MIN) 
	{
		nBufPoolSize = 900*1024;
	}

	BOOL iRet = g_cDHPlayManage.pDHFile[nPort]->OpenStream(pFileHeadBuf,nSize,nBufPoolSize); 

 	LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

	return iRet; 
}

/*	$Function		:	CALLMETHOD PLAY_InputData
==  ===============================================================
==	Description		:	输入流数据
==	Argument		:	nPort 端口号   pBuf 数据指针 nSize 数据长度
==				:	
== 	Return		:		成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_InputData(LONG nPort,PBYTE pBuf,DWORD nSize)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}
	
	EnterCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
#ifdef _DEBUG
		char str[100] ;
		sprintf(str,"通道%d 播放类没打开\n",nPort) ;
		OutputDebugString(str) ;
#endif
		LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);
		return FALSE ;
	}

	int iRet;
	__try
	{
		iRet = g_cDHPlayManage.pDHPlay[nPort]->InputData(pBuf,nSize); 
		if (iRet <0) 
		{

		}
	}
	__except(0,1)
	{
		int wlj = 0;
	}

	LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

	return iRet ;
}

/*	$Function		:	CALLMETHOD PLAY_CloseStream
==  ===============================================================
==	Description		:	关闭流接口
==	Argument		:	nPort 端口号
==				:	
== 	Return		:		成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_CloseStream(LONG nPort)
{
#ifdef _DEBUG
	char str[120] ;
	sprintf(str,"PLAY_CloseStream Enter %d\n",nPort) ; 
	OutputDebugString(str) ;
#endif

	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

 	EnterCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

	if (g_cDHPlayManage.pDHFile[nPort] == NULL)
	{
 		LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	g_cDHPlayManage.pDHFile[nPort]->CloseStream(); 

	delete g_cDHPlayManage.pDHFile[nPort] ;
	g_cDHPlayManage.pDHFile[nPort] = NULL ;

	g_cDHPlayManage.m_dwTimerType[nPort] = 0;

	PLAY_ReleasePort(nPort);

#ifdef _DEBUG
	sprintf(str,"PLAY_CloseStream Leave %d\n",nPort) ; 
	OutputDebugString(str) ;
#endif
	
 	LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

	return TRUE ;
}

/*	$Function		:	CALLMETHOD PLAY_GetCaps
==  ===============================================================
==	Description		:	获取系统属性,当前系统能支持的功能，按位取
==	Argument		:	
==				:	
== 	Return		:	    属性值，按位取
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API int  CALLMETHOD PLAY_GetCaps()
{
	return g_cDHPlayManage.GetCaps();
}

/*	$Function		:	CALLMETHOD PLAY_GetFileTime
==  ===============================================================
==	Description		:	获取文件总的时间长度
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	    时间长度值
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API DWORD CALLMETHOD PLAY_GetFileTime(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHFile[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHFile[nPort]->GetFileTotalTime(); 
}

/*	$Function		:	CALLMETHOD PLAY_GetPlayedTime
==  ===============================================================
==	Description		:	获取当前已播放文件的时间值
==	Argument		:	nPort 端口号
==				:	
== 	Return		:		时间值
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API DWORD CALLMETHOD PLAY_GetPlayedTime(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	DWORD dwRet = 0;
	__try
	{
		dwRet = g_cDHPlayManage.pDHPlay[nPort]->GetPlayedTime()/1000; 
	}
	__except(0,1)
	{
		dwRet = 0;
	}

	return dwRet;
}

/*	$Function		:	CALLMETHOD PLAY_GetPlayedFrames
==  ===============================================================
==	Description		:	获取已播放的桢数
==	Argument		:	nPort 端口号
==				:	
== 	Return		:		桢数
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API DWORD CALLMETHOD PLAY_GetPlayedFrames(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return 0 ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return 0 ;
	}

	DWORD iRet = g_cDHPlayManage.pDHPlay[nPort]->GetPlayedFrames() ;
	
	return iRet; 
}

PLAYSDK_API BOOL CALLMETHOD PLAY_GetRealFrameBitRate(LONG nPort, double* pBitRate)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )
	{
		return FALSE;
	}
	
	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR;
		return FALSE;
	}
	
	DWORD iRet = g_cDHPlayManage.pDHPlay[nPort]->GetRealFrameBitRate(pBitRate);
	
	return iRet; 
}

/*	$Function		:	CALLMETHOD PLAY_SetDecCallBack
==  ===============================================================
==	Description		:	设置解码数据显示回调函数
==	Argument		:	nPort 端口号
==				:	    DecCBFun 回调函数指针
== 	Return		:	    成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDecCallBack(LONG nPort,void (CALLBACK* DecCBFun)(long nPort,char * pBuf,long nSize,FRAME_INFO * pFrameInfo, long nReserved1,long nReserved2))
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (!g_cDHPlayManage.pCallback[nPort])
	{
		g_cDHPlayManage.pCallback[nPort] = new CCallback(nPort);
	}

	g_cDHPlayManage.pCallback[nPort]->SetDecCallBack(DecCBFun,TRUE);

	return TRUE;
}

/*	$Function		:	CALLMETHOD PLAY_SetDecCallBackEx
==  ===============================================================
==	Description		:	设置解码数据显示回调扩展函数(增加用户参数)
==	Argument		:	nPort 端口号
==				:	    DecCBFun 回调函数指针
== 	Return		:	    成功 TRUE, 失败 FALSE
==	Modification	:	2009/2/16	wanglanjun   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDecCallBackEx(LONG nPort,void (CALLBACK* DecCBFun)(long nPort,char * pBuf,long nSize,FRAME_INFO * pFrameInfo, long nReserved1,long nReserved2),long nUser)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (!g_cDHPlayManage.pCallback[nPort])
	{
		g_cDHPlayManage.pCallback[nPort] = new CCallback(nPort);
	}

	g_cDHPlayManage.pCallback[nPort]->SetDecCallBackEx(DecCBFun,nUser,TRUE);

	return TRUE;
}

PLAYSDK_API BOOL CALLMETHOD PLAY_SetVisibleDecCallBack(LONG nPort,void (CALLBACK* DecCBFun)(long nPort,char * pBuf,long nSize,FRAME_INFO * pFrameInfo, long nReserved1,long nReserved2),long nUser)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}
	
	if (!g_cDHPlayManage.pCallback[nPort])
	{
		g_cDHPlayManage.pCallback[nPort] = new CCallback(nPort);
	}
	
	g_cDHPlayManage.pCallback[nPort]->SetVisibleDecCallBack(DecCBFun,nUser,TRUE);
	
	return TRUE;
}

/*	$Function		:	CALLMETHOD PLAY_SetDisplayCallBack
==  ===============================================================
==	Description		:	设置抓图回调
==	Argument		:	nPort 端口号
==				:		DisplayCBFun 回调函数指针
== 	Return		:	    成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDisplayCallBack(LONG nPort,void (CALLBACK* DisplayCBFun)(long nPort,char * pBuf,long nSize,long nWidth,long nHeight,long nStamp,long nType,long nReserved), long nUser)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (!g_cDHPlayManage.pCallback[nPort])
	{
		g_cDHPlayManage.pCallback[nPort] = new CCallback(nPort);
	}
	
	return g_cDHPlayManage.pCallback[nPort]->SetDisplayCallBack(DisplayCBFun,nUser);
}

/*	$Function		:	CALLMETHOD PLAY_CatchPicEx
==  ===============================================================
==	Description		:	抓图函数,将当前图像数据保存为制定的图片格式
==	Argument		:	nPort 端口号
==					:	sFileName 文件名称
==					:   图片格式
== 	Return			:	成功 TRUE, 失败 FALSE
==	Modification	:	
==  ===============================================================
*/
PLAYSDK_API BOOL _stdcall PLAY_CatchPicEx(LONG nPort,char* sFileName,tPicFormats ePicfomat)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	if (sFileName == NULL)
	{
		return FALSE ;
	}

	unsigned char* pDataBuf = g_cDHPlayManage.pDHPlay[nPort]->GetLastFrame();

	long width , height ;
	g_cDHPlayManage.pDHPlay[nPort]->GetPictureSize(&width,&height);

	BOOL ret = FALSE;
	switch (ePicfomat)
	{
	case PicFormat_BMP:
		ret = PLAY_ConvertToBmpFile((char*)pDataBuf,width*height*3/2,width,height,0,sFileName);
		break;
	case PicFormat_JPEG:
		ret = PLAY_ConvertToJpegFile((char*)pDataBuf, width, height, YV12, 100, sFileName);
		break;
	default:
		break;
	}

	return ret;
}

//
// 抓取指定格式的图片，并保存为指定的宽高
// param：nPort				端口号
//        sFileName			保存路径
//        lTargetWidth		指定宽度
//        lTargetHeight		指定高度
//        ePicfomat			图片类型
//
PLAYSDK_API BOOL CALLMETHOD PLAY_CatchResizePic(LONG nPort, char* sFileName, LONG lTargetWidth, LONG lTargetHeight, tPicFormats ePicfomat)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}
	
	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	if (sFileName == NULL)
	{
		return FALSE ;
	}
	
	unsigned char* pDataBuf = g_cDHPlayManage.pDHPlay[nPort]->GetLastFrame();
	
	long width , height ;
	g_cDHPlayManage.pDHPlay[nPort]->GetPictureSize(&width,&height);

	BYTE* pYUVBuf = pDataBuf;
	if ((lTargetWidth != width) || (lTargetHeight != height))
	{
		if ((lTargetWidth > 0) && (lTargetHeight > 0))
		{
			pYUVBuf = NULL;
			pYUVBuf = new BYTE[lTargetWidth*lTargetHeight*3/2];
			if (pYUVBuf == NULL) return FALSE;

			img_conv(pDataBuf, width, height, pYUVBuf, lTargetWidth, lTargetHeight);
			
			width	= lTargetWidth;
			height	= lTargetHeight;
		}
	}
	
	BOOL ret = FALSE;
	switch (ePicfomat)
	{
	case PicFormat_BMP:
		ret = PLAY_ConvertToBmpFile((char*)pYUVBuf,width*height*3/2,width,height,0,sFileName);
		break;
	case PicFormat_JPEG:
		ret = PLAY_ConvertToJpegFile((char*)pYUVBuf, width, height, YV12, 100, sFileName);
		break;
	default:
		break;
	}

	if (pYUVBuf != pDataBuf)
	{
		delete[] pYUVBuf;
	}
	
	return ret;
}

/*	$Function		:	CALLMETHOD PLAY_ConvertToJpegFile
==  ===============================================================
==	Description		:	将YUV数据压缩为jpeg格式并保存文件
==	Argument		:	[IN] pYUVBuf 需要转换的数据指针 nWidth 图片宽度  nHeight 图片高度 YUVtype YUV格式类型 quality压缩质量(0, 100]
==					:	[OUT] sFileName 文件名
						
==					:	
== 	Return			:	成功 TRUE, 失败 FALSE
==	Modification	:
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_ConvertToJpegFile(char *pYUVBuf, long nWidth, long nHeight, int YUVtype, int quality, char *sFileName)
{
	if (pYUVBuf == NULL || sFileName == NULL || !nWidth || !nHeight)
	{
		return FALSE;
	} 

	std::string filefullname;

	filefullname = sFileName ;

	std::string::size_type pos = 0 ;
	std::string::size_type idx = 0 ;

	// 获取录像文件所在的盘符
	char cDiskNum[4];
	memset(cDiskNum, 0, 4);
	idx = filefullname.find(":", pos);
	memcpy(cDiskNum, sFileName, idx+1);
	
	unsigned __int64 iFreeBytes;
	GetDiskFreeSpaceEx(cDiskNum, (PULARGE_INTEGER)&iFreeBytes, NULL, NULL);
	// 存储空间不足，则返回
	if (iFreeBytes <= nWidth*nHeight*5)
	{
		return FALSE;
	}
	
	while ((idx = filefullname.find("\\", pos)) !=  std::string::npos)
	{
		CreateDirectory((filefullname.substr(0, idx)).c_str(), NULL) ;
		pos = idx + 1 ;
	}
	
	FILE* fp ;
	if ((fp = fopen(sFileName,"wb")) == NULL)
	{
		return FALSE ;
	}

	BYTE* pJpegBuf = new BYTE[nWidth*nHeight*2];
	if (pJpegBuf == NULL)
	{
		return FALSE;
	}

	int jpegsize = 0;
	memset(pJpegBuf, 0, nWidth*nHeight*2);
 	JpegEncode(pJpegBuf, (BYTE*)pYUVBuf, &jpegsize, nWidth, nHeight, YUVtype, quality);

	fwrite(pJpegBuf, 1, jpegsize, fp);
		
    //关闭文件
	fclose(fp);
	
	if (pJpegBuf != NULL)
	{
		delete[] pJpegBuf;
		pJpegBuf = NULL;
	}

	return TRUE ;
}


/*	$Function		:	CALLMETHOD PLAY_CatchPic
==  ===============================================================
==	Description		:	抓图函数,将当前图像数据保存为BMP文件
==	Argument		:	nPort 端口号
==				:		sFileName 文件名称
== 	Return		:	    成功 TRUE, 失败 FALSE
==	Modification	:	2006/4/28		zhougf   Create
==  ===============================================================
*/
PLAYSDK_API BOOL _stdcall PLAY_CatchPic(LONG nPort,char* sFileName)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	if (sFileName == NULL)
	{
		return FALSE ;
	}

	unsigned char* pDataBuf = g_cDHPlayManage.pDHPlay[nPort]->GetLastFrame();

	long width , height ;
	g_cDHPlayManage.pDHPlay[nPort]->GetPictureSize(&width,&height) ;

	return PLAY_ConvertToBmpFile((char*)pDataBuf,width*height*3/2,width,height,0,sFileName) ;
}


int params_bic[32][6] = {{0, -3, 256, 4,   -1,  0},  {1, -5,  255, 6,   -1,  0},
{1,  -9, 254, 12,  -2,  0},  {2, -13, 251, 19,  -3,  0},
{2, -17, 247, 27,  -4,  1},  {2, -19, 243, 36,  -6,  0},
{3, -22, 237, 45,  -8,  1},  {3, -24, 231, 54,  -9,  1},
{3, -25, 224, 64,  -11, 1},  {3, -25, 216, 74,  -13, 1},
{3, -27, 208, 86,  -15, 1},  {3, -27, 199, 95,  -16, 2},
{3, -27, 190, 106, -18, 2},  {3, -27, 181, 117, -20, 2},
{3, -26, 170, 128, -21, 2},  {3, -25, 160, 139, -23, 2},
{3, -24, 149, 149, -24, 3},  {2, -23, 139, 160, -25, 3},
{2, -21, 128, 170, -26, 3},  {2, -20, 117, 180, -26, 3},
{2, -18, 106, 190, -27, 3},  {2, -16, 95,  199, -27, 3},
{1, -15, 85,  208, -26, 3},  {1, -13, 75,  216, -26, 3},
{1, -11, 64,  224, -25, 3},  {1, -9,  54,  231, -24, 3},
{1, -8,  45,  237, -22, 3},  {0, -6,  36,  243, -19, 2},
{1, -4,  27,  247, -17, 2},  {0, -3,  19,  251, -13, 2},
{0, -2,  12,  254, -9,  1},  {0, -1,  6,   255, -5,  1}
};


int params_uv[32][2] = {{0,   256}, {8,   248}, {16,  240}, {24,  232}, 
{32,  224},	{40,  216}, {48,  208}, {56,  200}, 
{64,  192}, {72,  184}, {80,  176}, {88,  168}, 
{96,  160}, {104, 152}, {112, 144},	{120, 136}, 
{128, 128}, {136, 120}, {144, 112}, {152, 104}, 
{160,  96}, {168,  88}, {176,  80}, {184,  72}, 
{192,  64}, {200,  56}, {208,  48}, {216,  40},
{224,  32}, {232,  24}, {240,  16}, {248,   8}
};

int params_bil[32][4] ={{40, 176, 40, 0}, {36, 176, 43, 1},
{32, 175, 48, 1}, {30, 174, 51, 1},
{27, 172, 56, 1}, {24, 170, 61, 1},
{22, 167, 66, 1}, {19, 164, 71, 2},
{17, 161, 76, 2}, {15, 157, 82, 2},
{14, 152, 87, 3}, {12, 148, 93, 3},
{11, 143, 99, 3}, {9, 138, 105, 4},
{8, 133, 110, 5}, {7, 128, 116, 5},
{6, 122, 122, 6}, {5, 117, 127, 7},
{5, 110, 133, 8}, {4, 105, 138, 9},
{3, 99, 143, 11}, {3, 93, 148, 12},
{3, 87, 152, 14}, {2, 82, 157, 15},
{2, 76, 161, 17}, {2, 71, 164, 19},
{1, 66, 167, 22}, {1, 61, 170, 24},
{1, 56, 172, 27}, {1, 52, 173, 30},
{1, 47, 175, 33}, {0, 44, 176, 36}
};


void YResizeCubic(unsigned char* ptr_in, unsigned char* ptr_rz, 
				  int old_rows, int old_cols, int rsz_rows, int rsz_cols)
{
	unsigned char* ptr_temp;
	unsigned char* ptr_line;
	ptr_temp = (unsigned char*)malloc((old_rows + 6) * rsz_cols);
	ptr_line = (unsigned char*)malloc(old_cols + 6);

	int i, j, m, idx, tmp_data;
	int* ptr_flt;
	unsigned long ratio;
	
	ratio = old_cols * 1024 / rsz_cols;

	for(i = 0; i < old_rows; i++)
	{
		memcpy(ptr_line + 3, ptr_in + i * old_cols, old_cols);
		memset(ptr_line, ptr_in[i * old_cols], 3);
		memset(ptr_line + old_cols + 3, ptr_in[(i + 1) * old_cols - 1], 3);
		
		for(j = 0; j < rsz_cols; j++)
		{
			idx     = ((j * ratio) % 1024 * 32) / 1024;
			ptr_flt = params_bic[idx];

			idx      = j * ratio / 1024 + 3;
			tmp_data = 0;
			for(m = 0; m < 6; m++)
				tmp_data += ptr_line[idx + m - 2] * ptr_flt[m];

			tmp_data /= 256;
			if(tmp_data < 0) tmp_data = 0;
			if(tmp_data > 255) tmp_data = 255;
			ptr_temp[(i + 3) * rsz_cols + j] = (unsigned char)tmp_data;
		}
	}

	memcpy(ptr_temp, ptr_temp + 3 * rsz_cols, rsz_cols);
	memcpy(ptr_temp + rsz_cols, ptr_temp + 3 * rsz_cols, rsz_cols);
	memcpy(ptr_temp + rsz_cols * 2, ptr_temp + 3 * rsz_cols, rsz_cols);
	memcpy(ptr_temp + rsz_cols * (old_rows + 3), ptr_temp + (old_rows + 2) * rsz_cols, rsz_cols);
	memcpy(ptr_temp + rsz_cols * (old_rows + 4), ptr_temp + (old_rows + 2) * rsz_cols, rsz_cols);
	memcpy(ptr_temp + rsz_cols * (old_rows + 5), ptr_temp + (old_rows + 2) * rsz_cols, rsz_cols);

	ratio = old_rows * 1024 / rsz_rows;
	for(j = 0; j < rsz_cols; j++)
		for(i = 0; i < rsz_rows; i++)
		{
			idx     = ((i * ratio) % 1024 * 32) / 1024;
			ptr_flt = params_bic[idx];

			idx      = i * ratio / 1024 + 3;
			tmp_data = 0;
			for(m = 0; m < 6; m++)
				tmp_data += ptr_temp[(idx + m - 2) * rsz_cols + j] * ptr_flt[m];
			tmp_data /= 256;
			if(tmp_data < 0) tmp_data = 0;
			if(tmp_data > 255) tmp_data = 255;
			ptr_rz[i * rsz_cols + j] = (unsigned char)tmp_data;
		}

	free(ptr_temp);
	free(ptr_line);
}

int img_conv(unsigned char* psrc, unsigned int srcwidth, unsigned int srcheight, 
			 unsigned char* pdst, unsigned int dstwidth, unsigned int dstheight)
{
	
	unsigned char* YBuf, *UBuf, *VBuf;
	YBuf = pdst;
	UBuf = YBuf + dstwidth*dstheight;
	VBuf = UBuf + dstwidth*dstheight/4;
	
	YResizeCubic((unsigned char*)psrc, (unsigned char*)YBuf, srcheight, srcwidth, dstheight, dstwidth);
	YResizeCubic((unsigned char*)(psrc+srcwidth*srcheight), (unsigned char*)UBuf, srcheight/2, srcwidth/2, dstheight/2, dstwidth/2);
	YResizeCubic((unsigned char*)(psrc+srcwidth*srcheight*5/4), (unsigned char*)VBuf, srcheight/2, srcwidth/2, dstheight/2, dstwidth/2);
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////
// U and V component resize using bilinear interpolation
void UVResize(unsigned char* ptr_in, unsigned char* ptr_rz, 
			  int old_rows, int old_cols, int rsz_rows, int rsz_cols)
{
	unsigned char* ptr_temp;
	unsigned char* ptr_line;
	ptr_temp = (unsigned char*)malloc((old_rows + 2) * rsz_cols);
	ptr_line = (unsigned char*)malloc(old_cols + 2);
	
	int i, j, idx, tmp_data;
	int* ptr_flt;
	unsigned long ratio;
		
	ratio = 1024 * old_cols / rsz_cols;
	for(i = 0; i < old_rows; i++)
	{
		memcpy(ptr_line + 1, ptr_in + i * old_cols, old_cols);
		ptr_line[0] = ptr_line[1];
		ptr_line[old_cols + 1] = ptr_line[old_cols];

		for(j = 0; j < rsz_cols; j++)
		{
			idx     = ((j * ratio) % 1024 * 32) / 1024;
			ptr_flt = params_uv[idx];

			idx      = j * ratio / 1024 + 1;
			tmp_data = (ptr_line[idx] * ptr_flt[0] + ptr_line[idx + 1] * ptr_flt[1]) / 256;
			if(tmp_data > 255) tmp_data = 255;
			ptr_temp[(i + 1) * rsz_cols + j] = (unsigned char)tmp_data;
		}
	}
	memcpy(ptr_temp, ptr_temp + rsz_cols, rsz_cols);
	memcpy(ptr_temp + rsz_cols * (old_rows + 1), ptr_temp + old_rows * rsz_cols, rsz_cols);	

	ratio = 1024 * old_rows / rsz_rows;
	for(j = 0; j < rsz_cols; j++)
		for(i = 0; i < rsz_rows; i++)
		{
			idx     = ((i * ratio) % 1024 * 32) / 1024;	
			ptr_flt = params_uv[idx];

			idx      = i * ratio / 1024 + 1;
			tmp_data = (ptr_temp[idx * rsz_cols + j] * ptr_flt[0] + 
				ptr_temp[(idx + 1) * rsz_cols + j] * ptr_flt[1]) / 256;
			if(tmp_data > 255) tmp_data = 255;
			ptr_rz[i * rsz_cols + j] = (unsigned char)tmp_data;
		}
		
		free(ptr_temp);
		free(ptr_line);
}


#define RGB_Y_OUT		1.164
#define B_U_OUT			2.018
#define Y_ADD_OUT		16

#define G_U_OUT			0.391
#define G_V_OUT			0.813
#define U_ADD_OUT		128

#define R_V_OUT			1.596
#define V_ADD_OUT		128


#define SCALEBITS_OUT	13
#define FIX_OUT(x)		((uint16_t) ((x) * (1L<<SCALEBITS_OUT) + 0.5))

int32_t RGB_Y_tab[256];
int32_t B_U_tab[256];
int32_t G_U_tab[256];
int32_t G_V_tab[256];
int32_t R_V_tab[256];

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) > (b) ? (b) : (a))
/* yuv 4:2:0 planar -> rgb24 */


void colorspace_init(void) {
	int32_t i;

    for(i = 0; i < 256; i++) {
		RGB_Y_tab[i] = FIX_OUT(RGB_Y_OUT) * (i - Y_ADD_OUT);
		B_U_tab[i] = FIX_OUT(B_U_OUT) * (i - U_ADD_OUT);
		G_U_tab[i] = FIX_OUT(G_U_OUT) * (i - U_ADD_OUT);
		G_V_tab[i] = FIX_OUT(G_V_OUT) * (i - V_ADD_OUT);
		R_V_tab[i] = FIX_OUT(R_V_OUT) * (i - V_ADD_OUT);
	}
}

BYTE str_zgf[2000*3];
//BYTE g_rgb24[720*576*3];

void yv12_to_rgb24_c(uint8_t *dst, int dst_stride,
				 uint8_t *y_src, uint8_t *u_src, uint8_t * v_src, 
				 int y_stride, int uv_stride,
				 int width, int height)
{
	static int zgf_i = 0;
	if (zgf_i == 0)
	{
		colorspace_init();
		zgf_i = 1;
	}

	unsigned char* zgf_dst = dst;

	const uint32_t dst_dif = 6 * dst_stride - 3 * width;
	int32_t y_dif = 2 * y_stride - width;
	
	uint8_t *dst2 = dst + 3 * dst_stride;
	const uint8_t *y_src2 = y_src + y_stride;
	uint32_t x, y;
	
	if (height < 0) { // flip image?
		height = -height;
		y_src += (height - 1) * y_stride;
		y_src2 = y_src - y_stride;
		u_src += (height / 2 - 1) * uv_stride;
		v_src += (height / 2 - 1) * uv_stride;
		y_dif = -width - 2 * y_stride;
		uv_stride = -uv_stride;
	}

	for (y = height / 2; y; y--) 
	{
		// process one 2x2 block per iteration
		for (x = 0; x < (uint32_t)width / 2; x++)
		{
			int u, v;
			int b_u, g_uv, r_v, rgb_y;
			int r, g, b;

			u = u_src[x];
			v = v_src[x];

			b_u = B_U_tab[u];
			g_uv = G_U_tab[u] + G_V_tab[v];
			r_v = R_V_tab[v];

			rgb_y = RGB_Y_tab[*y_src];
			b = (rgb_y + b_u) >> SCALEBITS_OUT;
			g = (rgb_y - g_uv) >> SCALEBITS_OUT;
			r = (rgb_y + r_v) >> SCALEBITS_OUT;
			dst[0] = MAX(0,MIN(255, b));
			dst[1] = MAX(0,MIN(255, g));
			dst[2] = MAX(0,MIN(255, r));

			y_src++;
			rgb_y = RGB_Y_tab[*y_src];
			b = (rgb_y + b_u) >> SCALEBITS_OUT;
			g = (rgb_y - g_uv) >> SCALEBITS_OUT;
			r = (rgb_y + r_v) >> SCALEBITS_OUT;
			dst[3] = MAX(0,MIN(255, b));
			dst[4] = MAX(0,MIN(255, g));
			dst[5] = MAX(0,MIN(255, r));
			y_src++;

			rgb_y = RGB_Y_tab[*y_src2];
			b = (rgb_y + b_u) >> SCALEBITS_OUT;
			g = (rgb_y - g_uv) >> SCALEBITS_OUT;
			r = (rgb_y + r_v) >> SCALEBITS_OUT;
			dst2[0] = MAX(0,MIN(255, b));
			dst2[1] = MAX(0,MIN(255, g));
			dst2[2] = MAX(0,MIN(255, r));
			y_src2++;

			rgb_y = RGB_Y_tab[*y_src2];
			b = (rgb_y + b_u) >> SCALEBITS_OUT;
			g = (rgb_y - g_uv) >> SCALEBITS_OUT;
			r = (rgb_y + r_v) >> SCALEBITS_OUT;
			dst2[3] = MAX(0,MIN(255, b));
			dst2[4] = MAX(0,MIN(255, g));
			dst2[5] = MAX(0,MIN(255, r));
			y_src2++;

			dst += 6;
			dst2 += 6;
		}

		dst += dst_dif;
		dst2 += dst_dif;

		y_src += y_dif;
		y_src2 += y_dif;

		u_src += uv_stride;
		v_src += uv_stride;
	}

	for (int i = 0; i < height/2; i++)
	{
		memcpy(str_zgf, zgf_dst + i*width*3, width*3);
		memcpy(zgf_dst+i*width*3, zgf_dst+(height-i-1)*width*3, width*3);
		memcpy(zgf_dst+(height-1-i)*width*3, str_zgf, width*3);
	}

}


void YuvToRgb(const unsigned char &Y, const unsigned char &U, 
						   const unsigned char &V, RGBTRIPLE &rgb)
{
///*-----------------------------------------------------------------------------------------
	int rgbR = (int)(Y + 1.140 * (V - 128))  ;
	int	rgbG = (int)(Y - 0.581 * (V - 128)  - 0.395 * (U - 128)) ;
	int	rgbB = (int)(Y + 2.032 * (U - 128)) ;

	//确保RGB各值在0－255范围内，超过255,则设为255,小于0,一设为0
	if (rgbB > 255)
		rgb.rgbtBlue = 255 ;
	else if (rgbB < 0)
		rgb.rgbtBlue = 0 ;
	else
		rgb.rgbtBlue = rgbB ;	
	
	if (rgbG > 255)
		rgb.rgbtGreen = 255 ;
	else if (rgbG < 0)
		rgb.rgbtGreen = 0 ;
	else
		rgb.rgbtGreen = rgbG ;

	if (rgbR > 255)
		rgb.rgbtRed = 255 ;
	else if (rgbR < 0)
		rgb.rgbtRed = 0 ;
	else 
		rgb.rgbtRed = rgbR ;
}

void Convert2BMP(BYTE* pBmpBuf, DWORD* pBmpSize, BYTE* pDataBuf, long nWidth, long nHeight)
{
	BYTE* tmp_zgf = NULL ;
	BYTE *YBuf, *UBuf, *VBuf ;  //存放Y、U、V值的缓冲区

	YBuf = pDataBuf ;
	UBuf = pDataBuf + nWidth * nHeight ;
	VBuf = pDataBuf + nWidth * nHeight * 5 / 4 ;

// 	if ((nWidth>=640 && nWidth<=720) && (nHeight<=288 && nHeight>=240))
// 	{
// 		tmp_zgf = new BYTE[nWidth*nHeight*3] ;
// 		memset(tmp_zgf, 0, nWidth*nHeight*3);
// 
// 		int i ;
// 		for (i = 0 ; i < nHeight ; i++)
// 		{
// 			memcpy(tmp_zgf + 2*i * nWidth, pDataBuf + i*nWidth, nWidth);
// 			memcpy(tmp_zgf + (2*i + 1) * nWidth, pDataBuf + i*nWidth, nWidth) ;
// 		}
// 
// 		for (i = 0 ; i < nHeight/2 ;i++)
// 		{
// 			memcpy(tmp_zgf+nWidth*2*nHeight + 2*i * nWidth/2, pDataBuf + nWidth*nHeight + i*nWidth/2, nWidth/2);
// 			memcpy(tmp_zgf+nWidth*2*nHeight + (2*i + 1) * nWidth/2, pDataBuf + nWidth*nHeight + i*nWidth/2, nWidth/2) ;
// 		}
// 
// 		for (i = 0 ; i < nHeight/2 ;i++)
// 		{
// 			memcpy(tmp_zgf+nWidth*5*nHeight/2 + 2*i * nWidth/2, pDataBuf + nWidth*nHeight*5/4 + i*nWidth/2, nWidth/2);
// 			memcpy(tmp_zgf+nWidth*5*nHeight/2 + (2*i + 1) * nWidth/2, pDataBuf + nWidth*nHeight*5/4 + i*nWidth/2, nWidth/2) ;
// 		}
// 
// 		nHeight *= 2 ;
// 
// 		YBuf = tmp_zgf ;
// 		UBuf = tmp_zgf + nWidth * nHeight ;
// 		VBuf = tmp_zgf + nWidth * nHeight * 5 / 4 ;
// 	}
// 	else if ((nWidth<=352 && nWidth>=320) && (nHeight<=576 && nHeight>=480)) 
// 	{
// 		tmp_zgf = new BYTE[nWidth*nHeight*3] ;
// 		memset(tmp_zgf, 0, nWidth*nHeight*3);
// 
// 		for (int i = 0 ; i < nWidth*nHeight*3/2 ; i++)
// 		{
// 			tmp_zgf[i*2] = tmp_zgf[i*2 + 1] = pDataBuf[i] ;
// 		}
// 
// 		nWidth *= 2 ;
// 
// 		YBuf = tmp_zgf ;
// 		UBuf = tmp_zgf + nWidth * nHeight ;
// 		VBuf = tmp_zgf + nWidth * nHeight * 5 / 4 ;
// 	}
// 	else
// 	{
// 		YBuf = pDataBuf ;
// 		UBuf = pDataBuf + nWidth * nHeight ;
// 		VBuf = pDataBuf + nWidth * nHeight * 5 / 4 ;
// 	}

	BITMAPFILEHEADER bmpFileHeader ;   //BMP文件头
	BITMAPINFOHEADER bmpInfoHeader ;   //BMP信息头

	//设置BMP文件头
	bmpFileHeader.bfType = 0x4D42 ; // 文件头类型 'BM'(42 4D)
	bmpFileHeader.bfSize = sizeof(bmpFileHeader) + sizeof(bmpInfoHeader) + nWidth * nHeight * 3 ; //文件大小
	bmpFileHeader.bfReserved1 = 0 ; //保留字
	bmpFileHeader.bfReserved2 = 0 ; //保留字
	bmpFileHeader.bfOffBits = 54 ;  //位图像素数据的起始位置

	//设置BMP信息头
	bmpInfoHeader.biSize = 40 ;     //信息头所占字节数
	bmpInfoHeader.biWidth = nWidth ;   //位图宽度
	bmpInfoHeader.biHeight = nHeight ;  //位图高度
	bmpInfoHeader.biPlanes = 1 ;    //位图平面数
	bmpInfoHeader.biBitCount = 24 ;  //像素位数
	bmpInfoHeader.biCompression = 0 ;  //压缩类型，0 即不压缩
	bmpInfoHeader.biSizeImage = 0 ;    //
	bmpInfoHeader.biXPelsPerMeter = 0 ; //
	bmpInfoHeader.biYPelsPerMeter = 0 ; //
	bmpInfoHeader.biClrUsed = 0 ;      //
	bmpInfoHeader.biClrImportant = 0 ;  //

	//写入BMP文件头
	memcpy(pBmpBuf, &bmpFileHeader, sizeof(BITMAPFILEHEADER));

	//写入BMP信息头
	memcpy(pBmpBuf+sizeof(BITMAPFILEHEADER), &bmpInfoHeader, sizeof(BITMAPINFOHEADER));

	BYTE* prgb24 = pBmpBuf + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	yv12_to_rgb24_c(prgb24 , nWidth , YBuf , UBuf , VBuf, nWidth , nWidth/2, nWidth , nHeight);

	*pBmpSize = nWidth*nHeight*3 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
}

/*	$Function		:	CALLMETHOD PLAY_ConvertToBmpFile
==  ===============================================================
==	Description		:	将数据转换为bmp文件
==	Argument		:	[IN] pBuf 需要转换的数据指针 nSize  数据长度 nWidth 图片宽度  nHeight 图片高度 nType 类型
						[OUT] sFileName bmp文件名
==				:	
== 	Return		:		成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_ConvertToBmpFile(char * pBuf,long nSize,long nWidth,long nHeight,long nType,char *sFileName)
{
	if (pBuf == NULL || sFileName == NULL || !nSize || !nWidth || !nHeight)
	{
		return FALSE;
	}

	std::string filefullname;

	filefullname = sFileName ;

	std::string::size_type pos = 0 ;
	std::string::size_type idx = 0 ;

	// 获取录像文件所在的盘符
	char cDiskNum[4];
	memset(cDiskNum, 0, 4);
	idx = filefullname.find(":", pos);
	memcpy(cDiskNum, sFileName, idx+1);
	
	unsigned __int64 iFreeBytes;
	GetDiskFreeSpaceEx(cDiskNum, (PULARGE_INTEGER)&iFreeBytes, NULL, NULL);
	// 存储空间不足，则返回
	if (iFreeBytes <= nWidth*nHeight*5)
	{
		return FALSE;
	}
	
	while ((idx = filefullname.find("\\", pos)) !=  std::string::npos)
	{
		CreateDirectory((filefullname.substr(0, idx)).c_str(), NULL) ;
		pos = idx + 1 ;
	}
	
	FILE* fp ;
	if ((fp = fopen(sFileName,"wb")) == NULL)
	{
		return FALSE ;
	}

	DWORD dwBmpSize = 0;
	int len = nWidth*nHeight*4 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	BYTE* pBmpBuf = new BYTE[len];
	memset(pBmpBuf, 0, len);

	Convert2BMP(pBmpBuf, &dwBmpSize, (BYTE*)pBuf, nWidth, nHeight);

	fwrite(pBmpBuf, 1, dwBmpSize, fp);
		
    //关闭文件	
	fclose(fp) ;

	if (pBmpBuf)
	{
		delete pBmpBuf ;
		pBmpBuf = NULL ;
	}

	return TRUE ;
}

/*	$Function		:	CALLMETHOD PLAY_GetFileTotalFrames
==  ===============================================================
==	Description		:	获取文件的总桢数
==	Argument		:	nPort 端口号
==				:	
== 	Return		:		总桢数
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API DWORD	CALLMETHOD PLAY_GetFileTotalFrames(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHFile[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHFile[nPort]->GetTotalFrames() ;
}

/*	$Function		:	CALLMETHOD PLAY_GetCurrentFrameRate
==  ===============================================================
==	Description		:	获取当前播放数据桢率
==	Argument		:	nPort 端口号
==				:	
== 	Return		:		桢率值
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API DWORD CALLMETHOD PLAY_GetCurrentFrameRate(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	return g_cDHPlayManage.pDHPlay[nPort]->GetFrameRate() ;
}

/*	$Function		:	CALLMETHOD PLAY_GetPlayedTimeEx
==  ===============================================================
==	Description		:	获取播放时间（ms）暂时没做ms精确度 直接处理成s*1000
==	Argument		:	nPort 端口号
==				:	
== 	Return		:		播放时间
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API DWORD	CALLMETHOD PLAY_GetPlayedTimeEx(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	DWORD dwRet = 0;
	__try
	{
		dwRet = g_cDHPlayManage.pDHPlay[nPort]->GetPlayedTimeEX(); 
	}
	__except(0,1)
	{
		dwRet = 0;
	}

#ifdef _DEBUG
	char str[120] ;
	sprintf(str,"PLAY_GetPlayedTimeEx %d     %d\n",nPort, dwRet) ; 
	//OutputDebugString(str) ;
#endif

	return dwRet;
}

/*	$Function		:	CALLMETHOD PLAY_SetPlayedTimeEx
==  ===============================================================
==	Description		:	设置播放时间（ms）暂时没做ms精确度 直接处理成s*1000
==	Argument		:	nPort 端口号
==				:	
== 	Return		:		成功 TRUE, 失败 FALSE
==	Modification	:	2006/5/11		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetPlayedTimeEx(LONG nPort,DWORD nTime)
{
#ifdef _DEBUG
	char str[120] ;
	sprintf(str,"PLAY_SetPlayedTimeEx  %d   %d\n",nPort, nTime) ; 
	//OutputDebugString(str) ;
#endif

	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}
	
	if (g_cDHPlayManage.pDHPlay[nPort] == NULL || g_cDHPlayManage.pDHFile[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	return g_cDHPlayManage.pDHPlay[nPort]->SetPlayTime(nTime);
}

/*	$Function		:	CALLMETHOD PLAY_GetCurrentFrameNum
==  ===============================================================
==	Description		:	获取当前播放的桢序号
==	Argument		:	nPort 端口号
==				:	
== 	Return		:		桢序号
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API DWORD CALLMETHOD PLAY_GetCurrentFrameNum(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->GetCurFrameNum() ;
}

/*	$Function		:	CALLMETHOD PLAY_SetStreamOpenMode
==  ===============================================================
==	Description		:	设置流打开模式 必须在播放之前设置
==	Argument		:	nPort 端口号  nMode 模式类型 STREAME_REALTIME实时模式，适合播放网络实时数据，解码器会立刻解码。 
==	STREAME_FILE 文件模式，适合用户把文件数据用流方式输入，注意：当PLAY264_InputData()返回FALSE时，用户要等一下重新输入。
==	注意：可以做暂停，快放，慢放，单帧播放操作
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create  目前暂时没对此分开处理，都是堵塞时返回FALSE
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetStreamOpenMode(LONG nPort,DWORD nMode)
{
	if (g_cDHPlayManage.CheckPort(nPort))
	{
		return FALSE;
	}

	g_cDHPlayManage.m_nStreamMode[nPort] = nMode;

	return TRUE;
}

/*	$Function		:	CALLMETHOD PLAY_GetFileHeadLength
==  ===============================================================
==	Description		:	获取文件头的长度
==	Argument		:	
==				:	
== 	Return		:		文件头的长度
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API DWORD	CALLMETHOD PLAY_GetFileHeadLength()
{
	return g_cDHPlayManage.GetFileHeadLenth();
}

/*	$Function		:	CALLMETHOD PLAY_GetSdkVersion
==  ===============================================================
==	Description		:	获取sdk的版本
==	Argument		:	
==				:	
== 	Return		:		sdk的版本值
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API DWORD	CALLMETHOD PLAY_GetSdkVersion()
{
	return g_cDHPlayManage.GetSdkVersion();
}

/*	$Function		:	CALLMETHOD PLAY_GetLastError
==  ===============================================================
==	Description		:	获取错误类型
==	Argument		:	nPort 端口号
==				:	
== 	Return		:		错误类型号
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API DWORD  CALLMETHOD PLAY_GetLastError(LONG nPort)
{
	return g_cDHPlayManage.GetError(nPort) ;
}

/*	$Function		:	CALLMETHOD PLAY_RefreshPlay
==  ===============================================================
==	Description		:	刷新显示
==	Argument		:	nPort 端口号
==				:	
== 	Return		:		成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_RefreshPlay(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->RefreshPlay() ;
}

/*	$Function		:	CALLMETHOD PLAY_SetOverlayMode
==  ===============================================================
==	Description		:	设置overlay模式的关键色
==	Argument		:	nPort 端口号  bOverlay 是否overlay显示  colorKey 关键色
==				:	
== 	Return		:		成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetOverlayMode(LONG nPort,BOOL bOverlay,COLORREF colorKey)
{
#ifdef _DEBUG
	OutputDebugString("PLAY_SetOverlayMode Enter\n") ;
#endif

	if (g_cDHPlayManage.CheckPort(nPort))
	{
		return FALSE;
	}

	g_cDHPlayManage.pDHPlay[nPort]->m_pDisplay->SetOverlayMode(bOverlay,colorKey);
	
#ifdef _DEBUG
	OutputDebugString("PLAY_SetOverlayMode Leave\n") ;
#endif

	return TRUE ;
}

PLAYSDK_API	BOOL CALLMETHOD PLAY_VerticalSyncEnable(LONG nPort, BOOL bEnable)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.CheckPort(nPort))
	{
		return FALSE;
	}

	BOOL ret = FALSE;

	if (g_cDHPlayManage.pDHPlay[nPort]->m_pDisplay != NULL)
	{
		ret = g_cDHPlayManage.pDHPlay[nPort]->m_pDisplay->VerticalSyncEnable(bEnable);
	}
	
	return ret ;
}

/*	$Function		:	CALLMETHOD PLAY_GetPictureSize
==  ===============================================================
==	Description		:	获取图像大小
==	Argument		:	nPort 端口号
==				:		[out]  pWidth pHeight 宽度和高度
== 	Return		:		成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_GetPictureSize(LONG nPort,LONG *pWidth,LONG *pHeight)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	if (pWidth == NULL || pHeight == NULL)
	{
		return FALSE ;
	}
	
	return g_cDHPlayManage.pDHPlay[nPort]->GetPictureSize(pWidth, pHeight) ;
}

/*	$Function		:	CALLMETHOD PLAY_SetPicQuality
==  ===============================================================
==	Description		:	设置画质
==	Argument		:	nPort 端口号  bHighQuality 是否高清
==				:	
== 	Return		:		成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetPicQuality(LONG nPort,BOOL bHighQuality)
{
	g_cDHPlayManage.m_nQuality[nPort] = (int)bHighQuality;

	return TRUE;
}

/*	$Function		:	CALLMETHOD PLAY_PlaySoundShare
==  ===============================================================
==	Description		:	设置共享声音播放
==	Argument		:	nPort 端口号
==				:	
== 	Return		:		成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_PlaySoundShare(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	if (g_cDHPlayManage.m_nSoundPort != -1) 
	{
		return FALSE ;
	}

	std::vector<int>::iterator pos = g_cDHPlayManage.m_nShareSoundPortList.begin() ;

	for (; pos != g_cDHPlayManage.m_nShareSoundPortList.end() ; ++pos)
	{
		if (*pos == nPort)
		{
			break ;
		}
	}
	
	if (pos == g_cDHPlayManage.m_nShareSoundPortList.end())
	{
		g_cDHPlayManage.m_nShareSoundPortList.push_back(nPort) ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->SetDecAudio(TRUE) ;
}

/*	$Function		:	CALLMETHOD PLAY_StopSoundShare
==  ===============================================================
==	Description		:	以共享方式关闭声音
==	Argument		:	nPort 端口号
==				:	
== 	Return		:		成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_StopSoundShare(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	g_cDHPlayManage.pDHPlay[nPort]->SetDecAudio(FALSE) ;

	std::vector<int>::iterator pos = g_cDHPlayManage.m_nShareSoundPortList.begin() ;

	for (; pos != g_cDHPlayManage.m_nShareSoundPortList.end() ; ++pos)
	{
		if (*pos == nPort)
		{
			g_cDHPlayManage.m_nShareSoundPortList.erase(pos) ;
			break;
		}
	}

	return TRUE ;
}

/*	$Function		:	CALLMETHOD PLAY_GetStreamOpenMode
==  ===============================================================
==	Description		:	获得流模式类型。
==	Argument		:	nPort 端口号
==				:	
== 	Return		:		模式类型
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API LONG CALLMETHOD PLAY_GetStreamOpenMode(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	return g_cDHPlayManage.m_nStreamMode[nPort];
}

/*	$Function		:	CALLMETHOD PLAY_GetOverlayMode
==  ===============================================================
==	Description		:	检查当前播放器是否使用了OVERLAY模式；
==	Argument		:	nPort 端口号
==				:	
== 	Return		:		是否使用了OVERLAY模式
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API LONG CALLMETHOD PLAY_GetOverlayMode(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return 0 ;
	}

	if (g_cDHPlayManage.pDisplay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return 0 ;
	}
	
	return g_cDHPlayManage.pDisplay[nPort]->GetOverlayMode() ;
}

/*	$Function		:	CALLMETHOD PLAY_GetColorKey
==  ===============================================================
==	Description		:	获得OVERLAY表面使用的透明色；
==	Argument		:	nPort 端口号
==				:	
== 	Return		:		透明色
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API COLORREF CALLMETHOD PLAY_GetColorKey(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDisplay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDisplay[nPort]->GetColorKey() ;
}

/*	$Function		:	CALLMETHOD PLAY_GetVolume
==  ===============================================================
==	Description		:	获得当前设置的音量
==	Argument		:	nPort 端口号
==				:	
== 	Return		:		音量
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API WORD CALLMETHOD PLAY_GetVolume(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDisplay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDisplay[nPort]->GetVolume() ;
}

/*	$Function		:	CALLMETHOD PLAY_GetPictureQuality
==  ===============================================================
==	Description		:	/获得当前图像质量
==	Argument		:	nPort 端口号
==				:	    bHighQuality[OUT] 是否高清
== 	Return		:		成功 TRUE, 失败 FALSE
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_GetPictureQuality(LONG nPort,BOOL *bHighQuality)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (bHighQuality == NULL)
	{
		return FALSE ;
	}

	*bHighQuality = (BOOL) g_cDHPlayManage.m_nQuality[nPort];
	
	return TRUE;
}

/*	$Function		:	CALLMETHOD PLAY_GetSourceBufferRemain
==  ===============================================================
==	Description		:	获得流播放模式下源缓冲剩余数据的大小
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2006/01/16		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API DWORD CALLMETHOD PLAY_GetSourceBufferRemain(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->GetSourceBufferRemain();
}

/*	$Function		:	CALLMETHOD PLAY_ResetSourceBuffer
==  ===============================================================
==	Description		:	清除流播放模式下源缓冲区剩余数据
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2006/01/16		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_ResetSourceBuffer(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	EnterCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);
		return FALSE ;
	}

#ifdef _DEBUG
	char str[120] ;
	sprintf(str,"PLAY_ResetSourceBuffer %d\n",nPort) ; 
	OutputDebugString(str) ;
#endif

#ifdef _DEBUG
	OutputDebugString("before g_cDHPlayManage.pDHPlay[nPort]->ResetBufferRemain()");
#endif

	BOOL bRet = g_cDHPlayManage.pDHPlay[nPort]->ResetBufferRemain();

#ifdef _DEBUG
	OutputDebugString("after g_cDHPlayManage.pDHPlay[nPort]->ResetBufferRemain()");
#endif

	LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);

	return bRet;
}

/*	$Function		:	CALLMETHOD PLAY_SetSourceBufCallBack
==  ===============================================================
==	Description		:	设置源缓冲区阀值和剩余数据小于等于阀值时的回调函数指针
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2006/01/16		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetSourceBufCallBack(LONG nPort,DWORD nThreShold,
		void (CALLBACK * SourceBufCallBack)(long nPort,DWORD nBufSize,DWORD dwUser,void*pResvered),
		DWORD dwUser,void *pReserved)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (!g_cDHPlayManage.pCallback[nPort])
	{
		g_cDHPlayManage.pCallback[nPort] = new CCallback(nPort);
	}

	return g_cDHPlayManage.pCallback[nPort]->SetSourceBufCallBack(SourceBufCallBack, nThreShold, dwUser, pReserved);
}
/*	$Function		:	CALLMETHOD PLAY_ResetSourceBufFlag
==  ===============================================================
==	Description		:	重置回调标志位为有效状态
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2006/01/16		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_ResetSourceBufFlag(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pCallback[nPort] && g_cDHPlayManage.pCallback[nPort]->GetCBStatus(CBT_SourceBuf))
	{
		return g_cDHPlayManage.pCallback[nPort]->ResetSourceBufFlag();
	}
	
	return FALSE;
}
/*	$Function		:	CALLMETHOD PLAY_SetDisplayBuf
==  ===============================================================
==	Description		:	设置播放缓冲区（即解码后的图像缓冲区）大小
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2006/01/16		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDisplayBuf(LONG nPort,DWORD nNum)
{
#ifdef _DEBUG
	OutputDebugString("PLAY_SetDisplayBuf Enter\n") ;
#endif

	if ((nNum < MIN_DIS_FRAMES)||(nNum > MAX_DIS_FRAMES))
	{
		return FALSE;
	}
	
	if (g_cDHPlayManage.CheckPort(nPort))
	{
		return FALSE;
	}
	
	BOOL iRet ;
	iRet = g_cDHPlayManage.pDHPlay[nPort]->SetImgBufCount(nNum);

#ifdef _DEBUG
	OutputDebugString("PLAY_SetDisplayBuf Leave\n") ;
#endif

	return iRet ;
}

/*	$Function		:	CALLMETHOD PLAY_GetDisplayBuf
==  ===============================================================
==	Description		:	获得播放缓冲区最大缓冲的帧数；
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2006/01/16		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API DWORD CALLMETHOD PLAY_GetDisplayBuf(LONG nPort)
{
	if (g_cDHPlayManage.CheckPort(nPort))
	{
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->GetImgBufCount();
}

/*	$Function		:	CALLMETHOD PLAY_OneByOneBack
==  ===============================================================
==	Description		:	单帧回放。每调用一次倒退一帧
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_OneByOneBack(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->PlayBackOne(); 
}

/*	$Function		:	CALLMETHOD PLAY_SetFileRefCallBack
==  ===============================================================
==	Description		:	设置回调函数指针，文件索引建立后回调
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetFileRefCallBack(LONG nPort, void (CALLBACK *pFileRefDone)(DWORD nPort,DWORD nUser),DWORD nUser)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (!g_cDHPlayManage.pCallback[nPort])
	{
		g_cDHPlayManage.pCallback[nPort] = new CCallback(nPort);
	}
	
	return g_cDHPlayManage.pCallback[nPort]->SetFileRefCallBack(pFileRefDone, nUser);
}

PLAYSDK_API BOOL CALLMETHOD PLAY_SetFileRefCallBackEx(LONG nPort, void (CALLBACK *pFileRefDoneEx)(DWORD nPort, BOOL bIndexCreated, DWORD nUser),DWORD nUser)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}
	
	if (!g_cDHPlayManage.pCallback[nPort])
	{
		g_cDHPlayManage.pCallback[nPort] = new CCallback(nPort);
	}
	
	return g_cDHPlayManage.pCallback[nPort]->SetFileRefCallBackEx(pFileRefDoneEx, nUser);
}
/*	$Function		:	CALLMETHOD PLAY_SetCurrentFrameNum
==  ===============================================================
==	Description		:	设置当前播放播放位置到指定帧号；根据帧号来定位播放位置
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetCurrentFrameNum(LONG nPort,DWORD nFrameNum)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	return g_cDHPlayManage.pDHPlay[nPort]->SetCurrentFrameNum(nFrameNum); 
}

/*	$Function		:	CALLMETHOD PLAY_GetKeyFramePos
==  ===============================================================
==	Description		:	查找指定位置之前的关键帧位置
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_GetKeyFramePos(LONG nPort,DWORD nValue, DWORD nType, PFRAME_POS pFramePos)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHFile[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHFile[nPort]->GetKeyFramePos(nValue, nType, pFramePos); 
}

/*	$Function		:	CALLMETHOD PLAY_GetNextKeyFramePos
==  ===============================================================
==	Description		:	查找指定位置之后的关键帧位置
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_GetNextKeyFramePos(LONG nPort,DWORD nValue, DWORD nType, PFRAME_POS pFramePos)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}
	
	if (g_cDHPlayManage.pDHFile[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	return g_cDHPlayManage.pDHFile[nPort]->GetNextKeyFramePos(nValue, nType, pFramePos);
}
//Note: These funtion must be builded under win2000 or above with Microsoft Platform sdk.
//	    You can download the sdk from "http://www.microsoft.com/msdownload/platformsdk/sdkupdate/";

/*	$Function		:	CALLMETHOD
==  ===============================================================
==	Description		:	枚举系统中的显示设备
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
BOOL WINAPI Callback(GUID FAR *lpGUID,LPSTR pName,LPSTR pDesc,LPVOID pContext,HMONITOR  hm )
{
	DeviceInfo* devInfo = new DeviceInfo ;

	devInfo->lpDeviceDescription = new char(strlen(pDesc)) ;
	strcpy(devInfo->lpDeviceDescription,pDesc) ;
	
	devInfo->lpDeviceName = new char(strlen(pName)) ;
	strcpy(devInfo->lpDeviceName,pName) ;

// 	devInfo->hMonitor = new HMONITOR(hm) ;

	g_cDHPlayManage.DeviceInfoList.push_back(devInfo) ;
	return 0 ;
}

/*	$Function		:	CALLBACK PLAY_InitDDrawDevice
==  ===============================================================
==	Description		:	初始化DDraw设备
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_InitDDrawDevice()
{
// 	HINSTANCE handle = LoadLibrary("ddraw.dll") ;
// 	if (!handle)
// 		return false;
// 
// 	LPDIRECTDRAWENUMERATEEX lpDDEnumEx;
// 	lpDDEnumEx = (LPDIRECTDRAWENUMERATEEX) GetProcAddress(handle,
// 		"DirectDrawEnumerateExA");
// 
// 	//列举出所有连接到桌面的显示设备，
// 	if (lpDDEnumEx)
// 		lpDDEnumEx(Callback, NULL, DDENUM_ATTACHEDSECONDARYDEVICES |
// 	    	DDENUM_NONDISPLAYDEVICES );
// 	else return false ;
// 	
// 	FreeLibrary(handle);

	g_cDHPlayManage.m_supportmultidevice = 1;

	return true ;
}

/*	$Function		:	CALLMETHOD PLAY_ReleaseDDrawDevice
==  ===============================================================
==	Description		:	释放枚举显示设备的过程中分配的资源
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API void CALLMETHOD PLAY_ReleaseDDrawDevice()
{
// 	//释放向量中的数据
// 	for (int i = 0 ; i < g_cDHPlayManage.DeviceInfoList.size(); i++)
// 	{
// 		DeviceInfo* temp = g_cDHPlayManage.DeviceInfoList[i] ;
// 		delete temp->lpDeviceDescription ;
// 		delete temp->lpDeviceName ;
// // 		delete temp->hMonitor ;
// 		delete temp ;
// 		temp->lpDeviceDescription = NULL ;
// 		temp->lpDeviceName = NULL ;
// // 		temp->hMonitor = NULL ;
// 		temp = NULL ;
// 	} 
// 	
// 	g_cDHPlayManage.DeviceInfoList.clear() ;
	g_cDHPlayManage.m_supportmultidevice = 0;
}

/*	$Function		:	CALLMETHOD PLAY_GetDDrawDeviceTotalNums
==  ===============================================================
==	Description		:	获得系统中与windows桌面绑定的总的显示设备数目
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API DWORD CALLMETHOD PLAY_GetDDrawDeviceTotalNums()
{
	return g_cDHPlayManage.DeviceInfoList.size() ;//些函数必须在CALLMETHODPLAY264_InitDDrawDevice之后调用
}

/*	$Function		:	CALLMETHOD PLAY_SetDDrawDevice
==  ===============================================================
==	Description		:	设置播放窗口使用的显卡
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDDrawDevice(LONG nPort,DWORD nDeviceNum)
{
	return 0;
}

// #if (WINVER > 0x0400)
/*	$Function		:	CALLMETHOD PLAY_GetDDrawDeviceInfo
==  ===============================================================
==	Description		:	得到指定显卡和监视器信息；
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_GetDDrawDeviceInfo(DWORD nDeviceNum,LPSTR  lpDriverDescription,DWORD nDespLen,
								   LPSTR lpDriverName ,DWORD nNameLen,long *hhMonitor)
{
// 	if (nDeviceNum < 0 ||nDeviceNum > g_cDHPlayManage.DeviceInfoList.size())
// 		return false ;
// 
// 	DeviceInfo* devInfo = g_cDHPlayManage.DeviceInfoList[nDeviceNum] ;
// 	memcpy(lpDriverDescription,devInfo->lpDeviceDescription,nDespLen) ;
// 	memcpy(lpDriverName,devInfo->lpDeviceName,nNameLen) ;
// // 	hhMonitor = devInfo->hMonitor ;
	return true;
}

// #endif
/*	$Function		:	CALLMETHOD PLAY_GetCapsEx
==  ===============================================================
==	Description		:	获得指定显示设备的系统信息
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API int   CALLMETHOD PLAY_GetCapsEx(DWORD nDDrawDeviceNum)
{
	return 0;
}

/*	$Function		:	CALLMETHOD PLAY_ThrowBFrameNum
==  ===============================================================
==	Description		:	设置不解码B帧帧数（不支持，因为没有B桢）
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_ThrowBFrameNum(LONG nPort,DWORD nNum)
{
	return TRUE;
}

/*	$Function		:	CALLMETHOD PLAY_SetDisplayType
==  ===============================================================
==	Description		:	设置显示的模式
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDisplayType(LONG nPort,LONG nType)
{
	if (g_cDHPlayManage.CheckPort(nPort))
	{
		return FALSE;
	}
	else;
	
	return g_cDHPlayManage.pDHPlay[nPort]->SetDisplayType(nType) ;
}

/*	$Function		:	CALLMETHOD PLAY_GetDisplayType
==  ===============================================================
==	Description		:	获得目前设置的显示模式
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API long CALLMETHOD PLAY_GetDisplayType(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->GetDisplayType() ;
}

/*	$Function		:	
==  ===============================================================
==	Description		:	设置解码回调的流类型。
==	Argument		:	nPort 端口号
==				:	    nStream 1视频流，2音频流，3复合流
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDecCBStream(LONG nPort,DWORD nStream)
{
	if (g_cDHPlayManage.CheckPort(nPort))
	{
		return FALSE;
	}
	else;

	return  g_cDHPlayManage.pDHPlay[nPort]->SetDecCBType(nStream) ;

}

/*	$Function		:	CALLMETHOD PLAY_SetDisplayRegion
==  ===============================================================
==	Description		:	设置或增加显示区域。可以做局部放大显示。
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDisplayRegion(LONG nPort,DWORD nRegionNum, RECT *pSrcRect, HWND hDestWnd, BOOL bEnable)
{
	if (g_cDHPlayManage.CheckPort(nPort))
	{
		return FALSE;
	}

	if (nRegionNum >= 16)	//只能开16个子区域显示
	{
		return FALSE;
	}

	long width,height ;
	g_cDHPlayManage.pDHPlay[nPort]->GetPictureSize(&width,&height) ;
	g_cDHPlayManage.pDHPlay[nPort]->m_pDisplay->MulitiDisplay(nRegionNum,hDestWnd,pSrcRect,bEnable) ;

	return TRUE;
}

/*	$Function		:	CALLMETHOD PLAY_RefreshPlayEx
==  ===============================================================
==	Description		:	刷新显示，为支持PLAY264_SetDisplayRegion而增加一个参数。
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_RefreshPlayEx(LONG nPort,DWORD nRegionNum)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	g_cDHPlayManage.pDHPlay[nPort]->ReFreshEx(nRegionNum) ;

	return TRUE ;

}
/*#if (WINVER > 0x0400)*/
//Note: The funtion must be builded under win2000 or above with Microsoft Platform sdk.
//	    You can download the sdk from http://www.microsoft.com/msdownload/platformsdk/sdkupdate/;

/*	$Function		:	CALLMETHOD PLAY_SetDDrawDeviceEx
==  ===============================================================
==	Description		:	设置播放窗口使用的显卡
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDDrawDeviceEx(LONG nPort,DWORD nRegionNum,DWORD nDeviceNum)
{
	return 0;
}
/*#endif*/

/*	$Function		:	CALLMETHOD PLAY_GetRefValue
==  ===============================================================
==	Description		:	获取文件索引信息，以便下次打开同一个文件时直接使用这个信息
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_GetRefValue(LONG nPort,BYTE *pBuffer, DWORD *pSize)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHFile[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	return g_cDHPlayManage.pDHFile[nPort]->GetIndexInfo(pBuffer, pSize);
}

/*	$Function		:	CALLMETHOD PLAY_SetRefValue
==  ===============================================================
==	Description		:	设置文件索引
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetRefValue(LONG nPort,BYTE *pBuffer, DWORD nSize)
{
	if	(g_cDHPlayManage.CheckPort(nPort))
	{
		return FALSE;
	}

	return g_cDHPlayManage.pDHFile[nPort]->SetIndexInfo(pBuffer, nSize);
}

/*	$Function		:	CALLMETHOD PLAY_OpenStreamEx
==  ===============================================================
==	Description		:	以音视频分开输入得方式打开流。
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_OpenStreamEx(LONG nPort,PBYTE pFileHeadBuf,DWORD nSize,DWORD nBufPoolSize)
{
	return PLAY_OpenStream(nPort,pFileHeadBuf,nSize,nBufPoolSize) ;
}

/*	$Function		:	CALLMETHOD PLAY_CloseStreamEx
==  ===============================================================
==	Description		:	关闭数据流
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_CloseStreamEx(LONG nPort)
{
	return PLAY_CloseStream(nPort) ;
}

/*	$Function		:	CALLMETHOD PLAY_InputVideoData
==  ===============================================================
==	Description		:	输入从卡上得到的视频流 (可以是复合流，但音频数据会被忽略)
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_InputVideoData(LONG nPort,PBYTE pBuf,DWORD nSize)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->InputVideoData(pBuf,nSize);
}

/*	$Function		:	CALLMETHOD PLAY_InputAudioData
==  ===============================================================
==	Description		:	输入从卡上得到的音频流；打开声音之后才能输入数据
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_InputAudioData(LONG nPort,PBYTE pBuf,DWORD nSize)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->InputAudioData(pBuf,nSize);
}

/*	$Function		:	CALLMETHOD PLAY_RigisterDrawFun
==  ===============================================================
==	Description		:	注册一个回调函数，获得当前表面的device context,如果是使用overlay表面，这个接口无效
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_RigisterDrawFun(LONG nPort,void (CALLBACK* DrawFun)(long nPort,HDC hDc,LONG nUser),LONG nUser)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (!g_cDHPlayManage.pCallback[nPort])
	{
		g_cDHPlayManage.pCallback[nPort] = new CCallback(nPort);
	}

	return g_cDHPlayManage.pCallback[nPort]->SetDrawCallback(DrawFun, nUser);
}

PLAYSDK_API BOOL CALLMETHOD PLAY_RigisterDrawFunEx(LONG nPort, LONG nReginNum, void (CALLBACK* DrawFunEx)(long nPort,long nReginNum,HDC hDc,LONG nUser),LONG nUser)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}
	
	if (!g_cDHPlayManage.pCallback[nPort])
	{
		g_cDHPlayManage.pCallback[nPort] = new CCallback(nPort);
	}
	
	return g_cDHPlayManage.pCallback[nPort]->SetDrawExCallback(DrawFunEx, nReginNum, nUser);
}

/*	$Function		:	CALLMETHOD PLAY_SetTimerType
==  ===============================================================
==	Description		:	设置当前通道使用的定时器，注意：必须在Open之前调用
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2006/5/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetTimerType(LONG nPort,DWORD nTimerType,DWORD nReserved)
{
	if (g_cDHPlayManage.CheckPort(nPort))
	{
		return FALSE;
	}
	else;

	if ((nTimerType != TIMER_1) && (nTimerType != TIMER_2)) return FALSE;
	
	if (nTimerType == TIMER_1)
	{
		 MMRESULT timerTemp = timeSetEvent(1, 500, NULL, NULL, TIME_PERIODIC|TIME_CALLBACK_FUNCTION);
		 if (timerTemp == 0)
		 {
			 return FALSE;
		 }

		 timeKillEvent(timerTemp);
	}

	g_cDHPlayManage.m_dwTimerType[nPort] = nTimerType;
	g_cDHPlayManage.pDHPlay[nPort]->m_nTimerType = nTimerType;

	return TRUE;
}

/*	$Function		:	CALLMETHOD PLAY_GetTimerType
==  ===============================================================
==	Description		:	获得当前通道使用的定时器
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2006/5/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_GetTimerType(LONG nPort,DWORD *pTimerType,DWORD *pReserved)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	if (pTimerType == NULL)
	{
		return FALSE ;
	}

	*pTimerType = g_cDHPlayManage.pDHPlay[nPort]->m_nTimerType;

	return TRUE;
}

/*	$Function		:	CALLMETHOD PLAY_ResetBuffer
==  ===============================================================
==	Description		:	清空播放器中的缓冲区
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_ResetBuffer(LONG nPort,DWORD nBufType)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->ResetBuffer(nBufType);
}

//
/*	$Function		:	CALLMETHOD PLAY_GetBufferValue
==  ===============================================================
==	Description		:	获取播放器中的缓冲区大小（帧数或者byte）。这个接口可以帮助用户了解缓冲区中的数据，
						从而在网络延时方面有所估计
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API DWORD CALLMETHOD PLAY_GetBufferValue(LONG nPort,DWORD nBufType)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	return g_cDHPlayManage.pDHPlay[nPort]->GetBufferValue(nBufType);
}

/*	$Function		:	CALLMETHOD PLAY_AdjustWaveAudio
==  ===============================================================
==	Description		:	调整WAVE波形，可以改变声音的大小。它和PLAY264_SetVolume的不同在于，它是调整声音数据，只对该路其作用，
						而CALLMETHODPLAY_SetVolume是调整声卡音量，对整个系统起作用
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_AdjustWaveAudio(LONG nPort,LONG nCoefficient)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	if ((nCoefficient > MAX_WAVE_COEF)||(nCoefficient < MIN_WAVE_COEF))
	{
		return FALSE;
	}
	else;

	return g_cDHPlayManage.pDHPlay[nPort]->SetCoefficient(nCoefficient);
}

/*	$Function		:	CALLMETHOD PLAY_SetVerifyCallBack
==  ===============================================================
==	Description		:	注册一个回调函数，校验数据是否被修改，实现水印功能
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetVerifyCallBack(LONG nPort, DWORD nBeginTime, DWORD nEndTime, 
	void (CALLBACK * funVerify)(long nPort, FRAME_POS * pFilePos, DWORD bIsVideo, DWORD nUser),  DWORD  nUser)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (!g_cDHPlayManage.pCallback[nPort])
	{
		g_cDHPlayManage.pCallback[nPort] = new CCallback(nPort);
	}
	
	return g_cDHPlayManage.pCallback[nPort]->SetVerifyCallBack(funVerify, nBeginTime, nEndTime, nUser);
}

/*	$Function		:	CALLMETHOD PLAY_SetAudioCallBack
==  ===============================================================
==	Description		:	音频回调
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetAudioCallBack(LONG nPort, void (CALLBACK * funAudio)(long nPort, char * pAudioBuf,
								 long nSize, long nStamp, long nType, long nUser), long nUser)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}
	
	if (!g_cDHPlayManage.pCallback[nPort])
	{
		g_cDHPlayManage.pCallback[nPort] = new CCallback(nPort);
	}

	return g_cDHPlayManage.pCallback[nPort]->SetAudioCallBack(funAudio,nUser);
}

/*	$Function		:	CALLMETHOD PLAY_SetEncTypeChangeCallBack
==  ===============================================================
==	Description		:	解码时图象格式发生改变通知用户的回调函数；在打开文件前使用
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetEncTypeChangeCallBack(LONG nPort,void(CALLBACK *funEncChange)(long nPort,long nUser),long nUser)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}
	
	if (!g_cDHPlayManage.pCallback[nPort])
	{
		g_cDHPlayManage.pCallback[nPort] = new CCallback(nPort);
	}

	return g_cDHPlayManage.pCallback[nPort]->SetEncChangeCallBack(funEncChange, nUser);
}

/*	$Function		:	CALLMETHOD PLAY_SetColor
==  ===============================================================
==	Description		:	设置视频参数(显示设置)
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetColor(LONG nPort, DWORD nRegionNum, int nBrightness, int nContrast, int nSaturation, int nHue)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	g_cDHPlayManage.pDHPlay[nPort]->SetColor(nRegionNum, nBrightness, nContrast, nSaturation, nHue);

	return TRUE;
}

/*	$Function		:	CALLMETHOD PLAY_GetColor
==  ===============================================================
==	Description		:	获得视频参数
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_GetColor(LONG nPort, DWORD nRegionNum, int *pBrightness, int *pContrast, int *pSaturation, int *pHue)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	g_cDHPlayManage.pDHPlay[nPort]->GetColor(nRegionNum, pBrightness, pContrast, pSaturation, pHue);

	return TRUE;
}

/*	$Function		:	CALLMETHOD PLAY_SetEncChangeMsg
==  ===============================================================
==	Description		:	设置当解码时编码格式变化时发送的消息
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/12/15		chenmy   Create
==  ===============================================================
*/
PLAYSDK_API BOOL CALLMETHOD PLAY_SetEncChangeMsg(LONG nPort,HWND hWnd,UINT nMsg)
{
	if (g_cDHPlayManage.CheckPort(nPort))
	{
		return FALSE;
	}
  
	g_cDHPlayManage.pMsgEncChang[nPort]->hWnd = hWnd ;
	g_cDHPlayManage.pMsgEncChang[nPort]->nMsg = nMsg ;
	g_cDHPlayManage.pMsgEncChang[nPort]->nMsgFlag = TRUE ;
	
	return TRUE ;
}

PLAYSDK_API BOOL CALLMETHOD PLAY_SetMDRange(LONG nPort,RECT* MDRect,DWORD nVauleBegin,DWORD nValueEnd,DWORD nType)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHFile[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHFile[nPort]->SetMDRange(nVauleBegin, nValueEnd, nType, MDRect) ;
}

PLAYSDK_API BOOL CALLMETHOD PLAY_SetMDThreShold(LONG nPort, DWORD ThreShold)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHFile[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHFile[nPort]->SetMDThreShold(ThreShold) ;
}

PLAYSDK_API DWORD CALLMETHOD PLAY_GetMDPosition(LONG nPort, DWORD Direction, DWORD nFrame, DWORD* MDValue)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}
	
	if (g_cDHPlayManage.pDHFile[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHFile[nPort]->GetMDPosition(Direction, nFrame, MDValue) ;
}

PLAYSDK_API BOOL CALLMETHOD PLAY_SetFileEndCallBack(LONG nPort, void (CALLBACK *pFileEnd)(DWORD nPort,DWORD nUser),DWORD nUser)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (!g_cDHPlayManage.pCallback[nPort])
	{
		g_cDHPlayManage.pCallback[nPort] = new CCallback(nPort);
	}
	
	return g_cDHPlayManage.pCallback[nPort]->SetFileEndCallBack(pFileEnd, nUser);
}

PLAYSDK_API BOOL CALLMETHOD PLAY_StartDataRecord(LONG nPort, char *sFileName, int idataType)
{
	if (g_cDHPlayManage.CheckPort(nPort))
	{
		return FALSE;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->StartDataRecord(sFileName, idataType) ;	
}

PLAYSDK_API BOOL CALLMETHOD PLAY_StopDataRecord(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->StopDataRecord() ;
}

PLAYSDK_API BOOL CALLMETHOD PLAY_StartAVIResizeConvert(LONG nPort, char *sFileName, long lWidth, long lHeight)
{
	if (g_cDHPlayManage.CheckPort(nPort))
	{
		return FALSE;
	}
	
	return g_cDHPlayManage.pDHPlay[nPort]->StartAVIResizeConvert(sFileName, lWidth, lHeight) ;	
}

PLAYSDK_API BOOL CALLMETHOD PLAY_StopAVIResizeConvert(LONG nPort)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}
	
	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	return g_cDHPlayManage.pDHPlay[nPort]->StopDataRecord() ;
}

PLAYSDK_API BOOL CALLMETHOD PLAY_AdjustFluency(LONG nPort, int level)
{
	if (g_cDHPlayManage.CheckPort(nPort))
	{
		return FALSE;
	}
/*
	int adjust_bufnum, adjust_range;

	switch (level)
	{
	case 0:
		adjust_bufnum = 15;
		adjust_range = 0;
		break;
	case 1:
		adjust_bufnum = 10;
		adjust_range = 2;
		break;
	case 2:
		adjust_bufnum = 7;
		adjust_range = 4;
		break;
	case 3://默认
		adjust_bufnum = 4;
		adjust_range = 5;
		break;
	case 4:
		adjust_bufnum = 3;
		adjust_range = 10;
		break;
	case 5:
		adjust_bufnum = 2;
		adjust_range = 15;
		break;
	case 6:
		adjust_bufnum = 0;
		adjust_range = 20;
		break;
	default:
		return FALSE;

	}

	return g_cDHPlayManage.pDHPlay[nPort]->AdjustFluency(adjust_bufnum, adjust_range);
*/
	//1最流畅 7最实时，默认为4
	if (level > 7 || level < 1)
	{
		return FALSE;
	}

	g_cDHPlayManage.m_nFluency[nPort] = 8 - level;

	return TRUE;
}

PLAYSDK_API BOOL CALLMETHOD PLAY_ChangeRate(LONG nPort, int rate)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->FixRate(rate) ;
}


PLAYSDK_API BOOL CALLMETHOD PLAY_SetDemuxCallBack(LONG nPort, void (CALLBACK* DemuxCBFun)(long nPort,char * pBuf,	long nSize,void * pParam, long nReserved,long nUser), long nUser)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (!g_cDHPlayManage.pCallback[nPort])
	{
		g_cDHPlayManage.pCallback[nPort] = new CCallback(nPort);
	}
	
	return g_cDHPlayManage.pCallback[nPort]->SetDemuxCallBack(DemuxCBFun, nUser);
}

PLAYSDK_API BOOL CALLMETHOD PLAY_QueryInfo(LONG nPort , int cmdType
										   , char* buf, int buflen, int* returnlen)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	if (buf == 0 || returnlen == 0)
	{
		return FALSE;
	}

	BOOL bRet = TRUE;
	int  rate;

	switch (cmdType)
	{
	case PLAY_CMD_GetTime:

		*returnlen = sizeof(int)*6;

		if (buflen < 24)
		{
			break;
		}

		g_cDHPlayManage.pDHPlay[nPort]->GetTimeStr(buf,buflen);

		break;
	case PLAY_CMD_GetFileRate:	
		*returnlen = 4;
		if (buflen < 4)
		{
			break;
		}

		rate = g_cDHPlayManage.pDHPlay[nPort]->GetFileRate();
		buf[0] = rate&0xff;
		buf[1] = rate&0xff00;
		buf[2] = rate&0xff0000;
		buf[3] = rate&0xff000000;
		break;
	case PLAY_CMD_GetMediaInfo:
		bRet = g_cDHPlayManage.pDHPlay[nPort]->GetMediaInfo(buf,buflen);
		*returnlen = sizeof(MEDIA_INFO);
		break;
	}
	
	return bRet;
}

PLAYSDK_API BOOL CALLMETHOD PLAY_OpenAudioRecord(pCallFunction pProc, LONG nBitsPerSample, LONG nSamplesPerSec, long nLength, long nReserved, LONG nUser)
{
	BOOL bRet = TRUE;

	if (nBitsPerSample == 0 || nSamplesPerSec == 0)
	{
		return FALSE;
	}

 	EnterCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[0]);

	if (g_cDHPlayManage.pAudioRecored == NULL)
	{
		g_cDHPlayManage.pAudioRecored = new CHI_PLAY_AudioIn;
	}

	bRet = g_cDHPlayManage.pAudioRecored->Start(pProc, nBitsPerSample, nSamplesPerSec, nLength, nReserved, nUser);

 	LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[0]);

	return bRet;
}

PLAYSDK_API BOOL CALLMETHOD PLAY_CloseAudioRecord()
{
	BOOL bRet = TRUE;

 	EnterCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[0]);

	if (g_cDHPlayManage.pAudioRecored == NULL)
	{
	 	LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[0]);
		return FALSE;
	}

	g_cDHPlayManage.pAudioRecored->Stop();

 	LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[0]);

	return bRet;
}


PLAYSDK_API BOOL CALLMETHOD PLAY_SetWaterMarkCallBack(LONG nPort, GetWaterMarkInfoCallbackFunc pFunc, long nUser)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}

	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}

	return g_cDHPlayManage.pDHPlay[nPort]->SetWaterMarkCallbackFunc(pFunc, nUser);
}

PLAYSDK_API BOOL CALLMETHOD PLAY_SetPandoraWaterMarkCallBack(LONG nPort, GetWaterMarkInfoCallbackFunc pFunc, long nUser)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}
	
	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	return g_cDHPlayManage.pDHPlay[nPort]->SetWaterPandoraMarkCallbackFunc(pFunc, nUser);
}

PLAYSDK_API BOOL CALLMETHOD PLAY_SetIVSCallBack(LONG nPort, GetIVSInfoCallbackFunc pFunc, long nUser)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}
	
	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	return g_cDHPlayManage.pDHPlay[nPort]->SetIVSCallbackFunc(pFunc, nUser);
}

//
DWORD CALLMETHOD PLAY_CreateFile(LONG nPort,LPSTR sFileName)
{
	DWORD dRet = 1;

	{
		AutoLock lock(&g_PlayCritsec);

		int index;
		for (index = 1; index < FUNC_MAX_PORT; index++)
		{
			if(g_play[index] == 0)
			{
				g_play[index] = 1;
				break;
			}
		}

		if (index >= FUNC_MAX_PORT)
		{
			return 0;	
		}

		dRet = index;
	}
 
	BOOL bRet = PLAY_OpenFile(dRet, sFileName);

	if (bRet == FALSE)
	{
		return 0;
	}

	return dRet;
}

PLAYSDK_API BOOL CALLMETHOD PLAY_DestroyFile(LONG nPort)
{
	if(!PLAY_CloseFile(nPort)) 
	{
		return FALSE ;
	}

	g_play[nPort] = 0;

	return TRUE;
}

PLAYSDK_API DWORD CALLMETHOD PLAY_CreateStream(DWORD nBufPoolSize)
{
	DWORD dRet = 1;

	{
		AutoLock lock(&g_PlayCritsec);

		int index;
		for (index = 1; index < FUNC_MAX_PORT; index++)
		{
			if(g_play[index] == 0)
			{
				g_play[index] = 1;
				break;
			}
		}

		if (index >= FUNC_MAX_PORT)
		{
			return 0;	
		}

		dRet = index;
	}

	BOOL bRet = PLAY_OpenStream(dRet, 0, 0, nBufPoolSize);

	if (bRet == FALSE)
	{
		return 0;
	}

	return dRet;
}

PLAYSDK_API BOOL CALLMETHOD PLAY_DestroyStream(LONG nPort)
{
	if(!PLAY_CloseStream(nPort)) 
	{
		return FALSE ;
	}

	g_play[nPort] = 0;

	return TRUE;
}

PLAYSDK_API BOOL CALLMETHOD PLAY_SetRotateAngle(LONG nPort , int nrotateType)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}
	BOOL bRet = FALSE;
	EnterCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);
	
	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);
		return FALSE ;
	}
	
	bRet = g_cDHPlayManage.pDHPlay[nPort]->SetRotateAngle(nrotateType);
	LeaveCriticalSection(&g_cDHPlayManage.m_interfaceCritSec[nPort]);
	return bRet;
}

PLAYSDK_API BOOL CALLMETHOD PLAY_GetPicBMP(LONG nPort, PBYTE pBmpBuf, DWORD dwBufSize, DWORD* pBmpSize)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}
	
	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	if (pBmpBuf == NULL)
	{
		return FALSE ;
	}
	
	BYTE* pDataBuf = g_cDHPlayManage.pDHPlay[nPort]->GetLastFrame();
	
	long width , height ;
	g_cDHPlayManage.pDHPlay[nPort]->GetPictureSize(&width,&height);

	DWORD dwReqLen = width*height*3 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	if (dwBufSize < dwReqLen)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_MEMORY_TOOSMALL;
		return FALSE;
	}
	
	memset(pBmpBuf, 0, dwBufSize);
	Convert2BMP(pBmpBuf, pBmpSize, (BYTE*)pDataBuf, width, height);

	return TRUE;
}

PLAYSDK_API BOOL CALLMETHOD PLAY_GetPicJPEG(LONG nPort, PBYTE pJpegBuf, DWORD dwBufSize, DWORD* pJpegSize, int quality)
{
	if (nPort < 0 || nPort >= FUNC_MAX_PORT )  //端口号超出范围
	{
		return FALSE ;
	}
	
	if (g_cDHPlayManage.pDHPlay[nPort] == NULL)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_ORDER_ERROR ;
		return FALSE ;
	}
	
	if (pJpegBuf == NULL)
	{
		return FALSE ;
	}
	
	BYTE* pDataBuf = g_cDHPlayManage.pDHPlay[nPort]->GetLastFrame();
	
	long width , height ;
	g_cDHPlayManage.pDHPlay[nPort]->GetPictureSize(&width,&height);
	
	DWORD dwReqLen = width*height*3/2;
	if (dwBufSize < dwReqLen)
	{
		g_cDHPlayManage.m_error[nPort] = DH_PLAY_MEMORY_TOOSMALL;
		return FALSE;
	}

	memset(pJpegBuf, 0, dwBufSize);
 	JpegEncode(pJpegBuf, pDataBuf, (int*)pJpegSize, width, height, YV12, quality);

	return TRUE;
}

PLAYSDK_API int CALLMETHOD PLAY_OpenYuvRender(int port, HWND hwnd, void (CALLBACK* DrawFun)(int nPort,HDC hDc))
{
	return g_cDHPlayManage.OpenYuvRender(port, hwnd, DrawFun);
}

PLAYSDK_API int CALLMETHOD PLAY_RenderYuv(int port, unsigned char* py, unsigned char* pu, unsigned char* pv, int width, int height)
{
	return g_cDHPlayManage.RenderYuv(port, py, pu, pv, width, height);
}

PLAYSDK_API int CALLMETHOD PLAY_CloseYuvRender(int port)
{
	return g_cDHPlayManage.CloseYuvRender(port);
}