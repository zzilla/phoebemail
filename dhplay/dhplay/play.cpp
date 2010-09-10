/*
**  ************************************************************************
**                                    DHPlay  通用播放SDK
**
**         (c) Copyright 1992-2004, ZheJiang Dahua Technology Stock Co.Ltd.
**                            All Rights Reserved
**
**	File  Name	: play.cpp
**	Description	: 播放sdk播放控制程序
**	Modification	: 2005/11/28		chenmy		Create the file
**  ************************************************************************
*/

#include "StdAfx.h"
#include "stdio.h"
#include "play.h"
#include "playmanage.h"
#include "time.h"

#ifdef DHVECPLAY
	#include "dhvecplay.h"
#else
	#include "dhplayEx.h"
#endif

#include <math.h>

#ifdef _DEBUG
#define __Debug_String1(_t, _v)  {char str[120]; sprintf(str, (_t), (_v)); OutputDebugString(str);}
#else
#define __Debug_String1(_t, _v)
#endif


#define DEFAULT_BRIGHTNESS	0
#define DEFAULT_CONTRAST	128
#define DEFAULT_HUE			0
#define DEFAULT_SATURATION	64

/*	$Function		:	CDHPlay
==  ===============================================================
==	Description		:	构造函数
==	Argument		:	nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
CDHPlay::CDHPlay(LONG nPort)
{
	m_mBuffer = new BYTE [MAX_CHUNKSIZE];    //送解码数据缓冲
	m_nIndex = nPort;
	m_nStatus = PS_INVALID;  //播放状态（-1 无效 、0 停止、 1 暂停 、2正常播放、 3 快1 、4 快2、 。。。。）
	memset((BYTE *)&m_sDecSet, 0, sizeof(struct DecodeOption));
	m_bFlag = 0;            //缓冲数据有效性，取得文件数据后置1，送解码成功后置0
	memset(m_mBuffer, 0, MAX_CHUNKSIZE);    //送解码数据缓冲	
	m_ImgBufCount = 15;       //缓冲区大小  15
	m_pDHFile = g_cDHPlayManage.pDHFile[m_nIndex] ;
	m_pDisplay = g_cDHPlayManage.pDisplay[m_nIndex] ;     //显示对象指针
/*	m_PlayGragh = new DhPlayGraph ;*/
	m_PlayGragh = g_GraphFactory.getObject();
	m_IsSoundPlay = FALSE ;
	m_IsOneFrame = FALSE ;
	m_IsFileEnd      = FALSE ;
	m_IsPause = false ;
	m_nSpeed = PS_NORMAL ;
	m_iDesiredRate = SPEED[m_nSpeed];

	m_nTimerType = (DWORD)ACTimerType_Auto;
	m_StreamType = STREAM_COMPLEX ;
	
	m_ColorInfo.nBrightness = DEFAULT_BRIGHTNESS ;
	m_ColorInfo.nContrast   = DEFAULT_CONTRAST ;
	m_ColorInfo.nSaturation = DEFAULT_SATURATION ;
	m_ColorInfo.nHue        = DEFAULT_HUE ;

	m_FrameNumSeted = -1 ;
	m_tepFrameSeted = -1 ;
	m_fRelativePos  = -1 ;
	m_userfilepos = -1 ;
	InitializeCriticalSection(&m_PlayCritSec);

	m_hThread = NULL ;
	m_RecordFile = NULL ;
	m_ResetStreamFlag = -1 ;
	m_CurPlayedTime = 0;

	m_bPosAdjust4Index = FALSE;

	memset(m_cDiskNum, 0, 4);
}

/*	$Function		:	~CDHPlay
==  ===============================================================
==	Description		:	析构函数
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
CDHPlay::~CDHPlay()
{
	if (m_hThread != NULL)
	{
		CloseHandle(m_hThread) ;
		m_hThread = NULL ;
	}

	DeleteCriticalSection(&m_PlayCritSec);

	if (m_mBuffer != NULL)
	{
		delete[] m_mBuffer;    //送解码数据缓冲
		m_mBuffer = NULL ;
	}

	if (m_PlayGragh)
	{
// 		delete m_PlayGragh ;
// 		m_PlayGragh = NULL ;
		g_GraphFactory.returnObject(m_PlayGragh);
	}

	if (m_RecordFile)
	{
		CloseHandle(m_RecordFile) ;
		m_RecordFile = NULL ;
	}
}

/*	$Function		:	dec_cb
==  ===============================================================
==	Description		:	解码回调函数，最为关键的函数，对解码得到的音视频数据进行处理
==	Argument		:	输入参数：
							index 通道号
							data  音视频数据
							datalen 数据长度
							cbtype  回调类型，在decode.h中定义
							param	状态信息
							udata   用户数据
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
/*void swapimg(unsigned char *src, unsigned char *dst, int width, int height) ;*/
void adjust_lum(unsigned char *src, unsigned char *dst,int area,int LumGain,int LumOffset) ;
void adjust_hueSat(
	unsigned char *srcU,unsigned char *srcV,
	unsigned char *dstU,unsigned char *dstV,
	int stride, int dx,int dy,
	int huue, int saturation) ;

int __stdcall dec_cb(int index,unsigned char *data,unsigned int datalen,int cbtype,int param1,int param2)
{
	//数据源小于阀值回调处理
	if ((index < 0)||(index > FUNC_MAX_PORT))
	{
		return FALSE;
	}

	//视频数据
	if (cbtype == DEC_YUVDATA)
	{
		if (datalen == 0)
		{
			g_cDHPlayManage.pDHPlay[index]->m_pDisplay->ReciveYUV(index,
				NULL,NULL,NULL,0,0,1);
			return TRUE ;
		}

 		g_cDHPlayManage.pDHPlay[index]->precess(data,param1,param2) ;
			//将解码所得数据送给回放库显示或回调，由显示类进行处理
	
		int ret = g_cDHPlayManage.pDHPlay[index]->m_pDisplay->ReciveYUV(index,
			data,data + param1 * param2,data + param1 * param2 * 5 / 4,param1,param2,1);
		if (ret >0)
		{
			ret = g_cDHPlayManage.pDHPlay[index]->m_pDisplay->ResetVideoDisplay();
			if (ret > 0)
			{
				g_cDHPlayManage.m_error[index] = ret;
				return FALSE;
			}
		}

		return TRUE;
	}//end of if (cbtype == DEC_RENDER_YUVDATA)

	else if (cbtype == DEC_PCMDATA && g_cDHPlayManage.pDHPlay[index]->m_nSpeed == PS_NORMAL)
	{
		if (datalen == 0)
		{
			return TRUE ;
		}
	
 		g_cDHPlayManage.pDHPlay[index]->m_pDisplay->RecivePCM(index,data,datalen,param1,param2);

		return TRUE;
			
	}//end of if (cbtype == DEC_DECODE_PCMDATA)
			
	return FALSE ;
}

/////////////////////////////////////////////////与文件相关的接口///////////////////////////////////////////////////////////



/*	$Function		:	SetPlayPos
==  ===============================================================
==	Description		:	改变文件播放位置
==	Argument		:	输入：
							fRelativePos 所设置的文件位置与文件总长度的百分比
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::SetPlayPos(float fRelativePos)
{
	// 只有文件模式下，才可按百分比定位
	if ((m_pDHFile->GetDataOpenMode() != FM_FILE) || m_pDHFile == NULL || m_PlayGragh == NULL)
	{
		return FALSE  ;
	}
	
	if (!m_pDHFile->IsIndexCreated() || GetFileTotalFrames() <= 0)//如果没有建立文件索引
	{
	   	m_PlayGragh->reset(Reset_level_FASTRETURN);
		m_fRelativePos = fRelativePos ;
		return TRUE ;
	}

	//如果建立了文件索引，就快速定位的相应的帧
	int nFrame = GetFileTotalFrames() * fRelativePos ;

	if (nFrame < 0 || nFrame > GetFileTotalFrames()) 
	{
		return FALSE ;
	}

	if (nFrame == GetFileTotalFrames())
	{
		nFrame-- ;
	}
	
	return SetCurrentFrameNum(nFrame) ;
}

//获得当前位置，返回值为百分比的值
/*	$Function		:	GetPlayPos
==  ===============================================================
==	Description		:	获得当前位置，返回值为当前文件位置与文件总长度的百分比值
==	Argument		:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
float CDHPlay::GetPlayPos()
{
	if (!m_pDHFile->IsIndexCreated() || GetFileTime() == 0
		|| m_pDHFile->GetStreamDataType() == Stream_ShStream)
	{
		m_bPosAdjust4Index = TRUE;

		if (m_fRelativePos >= 0)
		{
			return m_fRelativePos;
		}

		//如果文件索引没有建立，就根据已处理的数据，返回文件中的位置的比值
		float fRelativePos = 0;
		m_pDHFile->GetRelativePos(m_PlayGragh->GetHandledDataLen(), &fRelativePos);

		return fRelativePos;
	}
	//返回当前帧与总帧数的比值
	else 
	{
		// 索引建立完成后，需要根据已处理的数据长度，调整播放位置相关信息
		if (m_bPosAdjust4Index)
		{
			m_bPosAdjust4Index = FALSE;

			float fRelativePos = 0;
			m_pDHFile->GetRelativePos(m_PlayGragh->GetHandledDataLen(), &fRelativePos);

			SetPlayPos(fRelativePos);
		}

		return GetCurFrameNum() / (GetFileTotalFrames() + 0.0) ;	
	}

	return 0 ;
}

/*	$Function		:	GetFileTime
==  ===============================================================
==	Description		:	获得文件的总的时间长度，由data分析取得
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
DWORD CDHPlay::GetFileTime()
{
	if (!m_pDHFile)
	{
		return -1 ;
	}
	
	return m_pDHFile->GetFileTotalTime(); 
}


/*	$Function		:	GetPlayedTime
==  ===============================================================
==	Description		:	获得文件当前播放的时间时间，由data分析取得
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
DWORD CDHPlay::GetPlayedTime()
{
	if (!m_pDHFile)
	{
		return -1 ;
	}

	if (m_pDHFile->IsIndexCreated())
	{
		if (m_pDHFile->GetStreamDataType() == Stream_ShStream)
		{
			return m_pDHFile->GetPlayedTime(0, false);
		}

		return m_pDHFile->GetPlayedTime(GetCurFrameNum());
	}
	else
	{
		return GetPlayedTimeEX();
	}

	return 0 ;
}

DWORD CDHPlay::GetPlayedTimeEX()
{
	DWORD dwRet = 0;

	if (m_pDHFile->IsIndexCreated())
	{
		if (m_CurPlayedTime != 0)
		{
			dwRet = m_CurPlayedTime ;
		}
		else
		{
			dwRet = GetPlayedTime();
		}

		return dwRet;
	}

	if (/*m_nStatus == PS_ONEFRAME &&*/ m_CurPlayedTime != 0)
	{
		dwRet = m_CurPlayedTime ;
	}
	else if (m_PlayGragh)
	{
		dwRet = m_PlayGragh->GetCurTimeEx();
	}
	
	return dwRet;
}
/*	$Function		:	SetPlayTime
==  ===============================================================
==	Description		:	按时间设置播放位置
==	Argument		:	输入参数：
==				:			nTime 文件正常播放到用户想设定的文件位置所需的时间
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::SetPlayTime(DWORD nTime)
{
	if (m_nStatus == PS_INVALID)
	{
		return FALSE ;
	}

	if (m_pDHFile == NULL || nTime > GetFileTime()*1000)
	{
		return FALSE ;
	}

	if (!m_pDHFile->IsIndexCreated())
	{
		m_PlayGragh->SetBeginTime(nTime/1000);
		m_CurPlayedTime = nTime;
		return TRUE;
// 		return m_pDHFile->SetPos(nTime / (GetFileTime() + 0.0)) ;
	}

	if (nTime == GetFileTime()*1000)
	{
		return SetCurrentFrameNum(GetFileTotalFrames()-1) ;
	}

	FRAME_POS temp_pos ;

	int rate = 25 ;

 	m_pDHFile->GetKeyFramePos(nTime, BY_FRAMETIME, &temp_pos,&rate) ;

	if (m_pDHFile->GetStreamDataType() == Stream_ShStream)
	{
		if (m_PlayGragh)
		{
			m_PlayGragh->reset(Reset_level_FASTRETURN) ;
		}

		m_userfilepos = (DWORD)temp_pos.nFilePos ;
		
		return TRUE ;
	}
	
	if (rate <= 0 || rate > 100)
	{
		rate = 25 ;
	}

	DWORD nFrame = temp_pos.nFrameNum + (nTime - temp_pos.nFrameTime)/(1000/rate) ;

	if (nFrame == GetFileTotalFrames())
	{
		nFrame = nFrame - 1 ;
	}
	
	m_CurPlayedTime = nTime;

	if (SetCurrentFrameNum(nFrame))
	{
		return TRUE;
	}
	else
	{
		m_CurPlayedTime = 0;
		return FALSE;
	}
}


/*	$Function		:	GetPlayedFrames
==  ===============================================================
==	Description		:	获得已经解码的桢数
==	Argument		:
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
DWORD CDHPlay::GetPlayedFrames()
{
	if (!m_pDisplay)
	{
		return -1 ;
	}

	return m_pDisplay->GetPlayedFrames();   
}


/*	$Function		:	GetFileTotalFrames
==  ===============================================================
==	Description		:	得到文件总桢数
==	Argument		:	
==				:	
== 	Return		:	
==	Modifcation	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
DWORD CDHPlay::GetFileTotalFrames()     
{
	if (!m_pDHFile)
	{
		return -1 ;
	}

	return m_pDHFile->GetTotalFrames();
}


/*	$Function		:	GetFrameRate
==  ===============================================================
==	Description		:	得到当前播放文件桢率
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
DWORD CDHPlay::GetFrameRate()
{
	int rate = 0;

	if (m_PlayGragh != NULL)
	{
		rate = m_PlayGragh->getRate() ;
	}

	return (DWORD)(rate);
}
 
        
int CDHPlay::GetFileRate()
{
	int rate = 0;

	if (m_PlayGragh != NULL)
	{
		rate = m_PlayGragh->getFileRate() ;
	}

	return rate;
}
/*	$Function		:	GetCurFrameNum
==  ===============================================================
==	Description		:	得到当前播放桢序号  
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
DWORD CDHPlay::GetCurFrameNum()         
{
	if (!m_pDisplay)
	{
		return -1 ;
	}

	if (m_pDHFile && m_pDHFile->IsIndexCreated())
	{
		return m_pDisplay->GetCurFrameNum() ;
	}
	else
	{
		return m_pDisplay->GetPlayedFrames() ;
	}

	return 0 ;
}

/*	$Function		:	SetCurrentFrameNum
==  ===============================================================
==	Description		:	使文件从指定桢数开始播放,关键函数
==	Argument		:	输入参数：
==				:			nFrameNum 指定桢数
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::SetCurrentFrameNum(DWORD nFrameNum)
{
	if (!m_pDHFile || m_nStatus == PS_INVALID || m_nStatus == PS_STOP)
	{
		return FALSE ;
	}

	if (m_pDHFile->IsIndexCreated() == FALSE)
	{
		return FALSE ;
	}

	if (nFrameNum >= GetFileTotalFrames())
	{
		return FALSE ;
	}

	m_pDisplay->SetCurFrameNum(nFrameNum);
	m_PlayGragh->reset(Reset_level_FASTRETURN);
	m_FrameNumSeted = nFrameNum ;

	return TRUE;
}

/*	$Function		:	GetRefValue
==  ===============================================================
==	Description		:	获取文件索引信息
==	Argument		:	输出函数：
==				:			pBuffer 用以保存文件索引信息的数据
							pSize   pBuff的长度
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/

BOOL CDHPlay::GetRefValue(BYTE *pBuffer, DWORD *pSize)
{
	if (!m_pDHFile)
	{
		return FALSE ;
	}

	return m_pDHFile->GetIndexInfo(pBuffer, pSize) ;
}

/*	$Function		:	SetRefValue
==  ===============================================================
==	Description		:	设置文件索引作息
==	Argument		:	输入函数：
==				:			pBuffer 用以保存文件索引信息的数据
							pSize   pBuff的长度
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::SetRefValue(BYTE *pBuffer, DWORD nSize)
{
	if (!m_pDHFile)
	{
		return FALSE ;
	}

	return m_pDHFile->SetIndexInfo(pBuffer, nSize) ;
}

/*	$Function		:	GetPictureSize
==  ===============================================================
==	Description		:	获取数据源图片的大小
==	Argument		:	输入函数：
==				:			pBuffer 用以保存文件索引信息的数据
							pSize   pBuff的长度
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL  CDHPlay::GetPictureSize(LONG* pWidth, LONG* pHeight)
{
	if (!m_pDisplay)
	{
		return FALSE ;
	}
	
	*pWidth = m_pDisplay->GetPicWidth();
	*pHeight= m_pDisplay->GetPicHeight() ;

	return TRUE ;
}
/////////////////////////////////////////////////播放控制接口///////////////////////////////////////////////////////////////////
DWORD WINAPI ThreadProc(LPVOID pParam)  
{
	CDHPlay* dhplay = (CDHPlay*)pParam ;

    __try  //zgfzgf
	{	
		if (dhplay == NULL)
		{
			return 1 ;
		}

		dhplay->PlayProc() ;
	}
    __except(0,1)
	{
		return 1;
 	}
	
	return 0 ;
}
/*	$Function		:	开始播放文件或流
==  ===============================================================
==	Description		:	Start
==	Argument		:	输入函数：
==				:			hwnd 窗口句柄
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::Start(HWND hwnd)    
{
	if (!m_pDHFile)      //当没有视频数据资源时不能开始播放
	{
		g_cDHPlayManage.m_error[m_nIndex] = DH_PLAY_ORDER_ERROR ;/////////////////////////////////			
		return false;
	}

	if (m_nStatus == PS_PLAY || m_nStatus == PS_ONEFRAME ||  m_IsPause || m_IsOneFrame)
	{
		m_IsPause = FALSE;
		m_nStatus = PS_PLAY ;
		m_nSpeed = PS_NORMAL ;
		m_iDesiredRate = 25;

		// 20091026 fix bug: 由慢放到正常播放，会卡一下
		DWORD dwThreadId;
		CreateThread(NULL, 0, ChangeSpeedProc, this, 0, &dwThreadId);
		//m_PlayGragh->changeRate(m_PlayGragh->getFileRate()) ;
	
		if (m_nStatus == PS_ONEFRAME)
		{
			SetCurrentFrameNum(GetCurFrameNum()) ;
		}

		if (m_IsOneFrame)
		{
			m_IsOneFrame = FALSE ;
			m_PlayGragh->start() ;
		}

		//防止单帧播放后音视频不同步
		if (m_pDHFile->IsIndexCreated() && (m_pDHFile->GetStreamDataType() != Stream_ShStream))
		{
			SetCurrentFrameNum(GetCurFrameNum()) ;
		}
		else
		{
			float fPos = GetPlayPos();
			SetPlayPos(fPos);
		}

		return true ;
	}

	if (m_pDisplay == NULL)
	{
		m_pDisplay = new CDisplay(m_nIndex);
		if (m_pDisplay = NULL)
		{
			g_cDHPlayManage.m_error[m_nIndex] = 6; //DH_PLAY_ALLOC_MEMORY_ERROR

			__Debug_String1("Port %d  new CDisplay fail!!!!!!!!!!\n", m_nIndex);
			return false;
		}
	}
	
 	int ret = m_pDisplay->Init(hwnd);
	if (ret > 0)
	{
		g_cDHPlayManage.m_error[m_nIndex] = ret;

		__Debug_String1("Port %d  m_pDisplay->Init fail!!!!!!!!!!\n", m_nIndex);
		return false;
	}

	m_sDecSet.cb = dec_cb;
	m_sDecSet.decId = m_nIndex;
	m_sDecSet.imgBufCount = m_ImgBufCount;

	if (m_pDHFile->GetDataOpenMode() == FM_FILE)
	{
		m_sDecSet.transMode = SM_File;
	}
	else
	{
		if (g_cDHPlayManage.m_nStreamMode[m_nIndex] == STREAME_REALTIME)
		{
			m_sDecSet.transMode = SM_Stream_Stream;
		}
		else
		{
			m_sDecSet.transMode = SM_Stream_File;
		}
	}

	if (g_cDHPlayManage.pCallback[m_nIndex] && g_cDHPlayManage.pCallback[m_nIndex]->GetCBStatus(CBT_DecCBFun)
		&& g_cDHPlayManage.pDisplay[m_nIndex]->GetDecType() == DEC_COMPLEX )
	{
		m_sDecSet.ifDecOnly = TRUE ;
	}
	else
	{
		m_sDecSet.ifDecOnly = FALSE ;
	}
		
	m_sDecSet.hwnd = hwnd;

	CAccurateTimer *pACTimer = NULL;

	if (!m_sDecSet.ifDecOnly)
	{
		g_cDHPlayManage.m_ACTimerManager.GetACTimer(&pACTimer, (tACTimerType)m_nTimerType);
		m_nTimerType = pACTimer->getACTimerType();
	}
	
	m_sDecSet.pACTimerObj = pACTimer;

	ret = m_PlayGragh->init(&m_sDecSet);
 	if (ret > 0)
	{
		g_cDHPlayManage.m_error[m_nIndex] = ret;

		__Debug_String1("Port %d  m_PlayGragh->Init fail!!!!!!!!!!\n", m_nIndex);
		return false;
	}

	ret = m_PlayGragh->start();
 	if (ret > 0)
	{
		g_cDHPlayManage.m_error[m_nIndex] = ret;

		__Debug_String1("Port %d m_PlayGragh->start() fail!!!!!!!!!!\n", m_nIndex);
		return false;
	}
	
	if (m_IsSoundPlay)
	{
		m_PlayGragh->SetDecAudio(TRUE) ;
	}

	m_pDHFile->ReOpen() ;       //置文件指针位置为文件开始位置

	m_nStatus = PS_PLAY;

	DWORD dwThreadId ;

	m_hThread = CreateThread( 
        NULL,                        // no security attributes 
        0,                           // use default stack size  
        ThreadProc,                  // thread function 
        this,						// argument to thread function 
        0,                           // use default creation flags 
        &dwThreadId); 
	
	if (m_hThread == NULL)
	{
		g_cDHPlayManage.m_error[m_nIndex] = 8; //DH_PLAY_CREATE_OBJ_ERROR

		__Debug_String1("Port %d CDHPlay::Start creat ThreadProc fail!!!\n", m_nIndex);
		return false ;
	}
//
//	char str[100];
//	sprintf(str,"d://zgf0805_%d.avi", m_nIndex);
//     StartDataRecord(str, 1);

	return true;
}


/*	$Function		:	Write
==  ===============================================================
==	Description		:	往解码库写数据
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::Write()
{
	if (m_nStatus == PS_STOP|| m_nStatus == PS_INVALID )   //没打开解码
	{		
		return FALSE;
	}

	m_PlayGragh->parseData(m_mBuffer, m_bFlag) ;//内部调用，不判断

	return TRUE ;
}


/*	$Function		:	Stop
==  ===============================================================
==	Description		:	停止播放
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::Stop()    
{
	if (m_PlayGragh == NULL)
	{
		return FALSE ;
	}

	if (PS_STOP == m_nStatus)
	{
		return TRUE;
	}

	m_nStatus = PS_STOP ;	

	m_PlayGragh->stop() ;

	if (m_hThread)
	{
		DWORD dwRes = 0;
		dwRes = WaitForSingleObject(m_hThread, INFINITE) ;
		if(WAIT_OBJECT_0 == dwRes)			
		{
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
	}

	m_PlayGragh->RealStop() ;
	
	if (m_pDisplay)
	{
		m_pDisplay->Close();	
	}

	return TRUE;
}


/*	$Function		:	CDHPlay
==  ===============================================================
==	Description		:	暂停播放/继续播放
==	Argument		:	输入参数：
==				:			nPause 1 文件暂停 0 文件继续播放
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::Pause(DWORD nPause)  
{
	if (!m_PlayGragh)   //没打开解码
	{
		return FALSE;
	}

	if (nPause > 0)
	{
		m_PlayGragh->pause() ;
		
		// 20090929 bug: PlayBackOne 之后，再继续、暂停后引起音视频不同步
		m_PlayGragh->SetStepGoStatus(FALSE); 
		m_IsPause = TRUE ;
	}
	else
	{		
		if (!m_IsOneFrame)
		{
			m_PlayGragh->start() ;
		}

		m_IsPause = false ;
	}

	return TRUE;
}

DWORD  CDHPlay::ChangeSpeedProc(LPVOID pParam)  
{
	CDHPlay* pDHPlay = (CDHPlay*)pParam;
	
	pDHPlay->m_PlayGragh->changeRate(pDHPlay->m_iDesiredRate);
	
	return 0;
}

/*	$Function		:	PlayFast
==  ===============================================================
==	Description		:	快速播放
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::PlayFast() 
{
	if (!m_PlayGragh)   //没打开解码
	{
		return FALSE;
	}
	
	if (m_nSpeed >= PS_FAST4)
	{
		return FALSE ;
	}

	m_nSpeed = (PS_SPEED)((int)(m_nSpeed+1)) ;
	m_iDesiredRate = SPEED[m_nSpeed];

	if (SPEED[m_nSpeed] <= 250)
	{
//		m_PlayGragh->changeRate(SPEED[m_nSpeed]) ;

		DWORD dwThreadId;
		CreateThread(NULL, 0, ChangeSpeedProc, this, 0, &dwThreadId);
	}

	//防止快放慢放后音视频不同步
	if (!m_IsOneFrame && (PS_NORMAL == m_nSpeed))
	{
		if (m_pDHFile->IsIndexCreated() && (m_pDHFile->GetStreamDataType() != Stream_ShStream))
		{
			SetCurrentFrameNum(GetCurFrameNum()) ;
		}
		else
		{
			float fPos = GetPlayPos();
			SetPlayPos(fPos);
		}
	}

	return TRUE;
}


/*	$Function		:	PlaySlow
==  ===============================================================
==	Description		:	慢速播放
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::PlaySlow()  
{
	if (!m_PlayGragh)   //没打开解码
	{
		return FALSE;
	}
	
	if (m_nSpeed <= PS_SLOW4)
	{
		return FALSE ;
	}

//	if (GetFrameRate() <= SPEED[m_nSpeed-1])
//	{
//		return false ;
//	}

	m_nSpeed = (PS_SPEED)((int) m_nSpeed - 1) ;
	m_iDesiredRate = SPEED[m_nSpeed];

//	m_PlayGragh->changeRate(SPEED[m_nSpeed]) ;

	DWORD dwThreadId;
	CreateThread(NULL, 0, ChangeSpeedProc, this, 0, &dwThreadId);

	//防止快放慢放后音视频不同步
	if (!m_IsOneFrame && (PS_NORMAL == m_nSpeed))
	{
		if (m_pDHFile->IsIndexCreated() && (m_pDHFile->GetStreamDataType() != Stream_ShStream))
		{
			SetCurrentFrameNum(GetCurFrameNum()) ;
		}
		else
		{
			float fPos = GetPlayPos();
			SetPlayPos(fPos);
		}
	}

	return TRUE;
}


/*	$Function		:	PlayBack
==  ===============================================================
==	Description		:	反向播放
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::PlayBack()  
{
	return TRUE;
}


/*	$Function		:	PlayOnebyone
==  ===============================================================
==	Description		:	单帧正向播放
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::PlayOnebyone()  
{
	if (m_PlayGragh == NULL)
	{
		return FALSE ;
	}

	m_IsOneFrame = TRUE ;

	if (m_nSpeed != PS_NORMAL)
	{
		m_nSpeed = PS_NORMAL;
		m_PlayGragh->changeRate(SPEED[m_nSpeed]);
	}

	m_PlayGragh->stepGo() ;
	
	return TRUE ;
}


/*	$Function		:	PlayBackOne
==  ===============================================================
==	Description		:	单帧反向播放
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::PlayBackOne()
{
	//20060429 为解决单帧到放到底再播放，再单帧倒放不了而修改。任务号6259 winton 修改
	DWORD nNum = 0;

	if (m_pDHFile && m_pDHFile->IsIndexCreated() == FALSE)
	{
		return FALSE ;
	}

	if (GetCurFrameNum() < 1)
	{
		return FALSE;
	}
	else
	{
		nNum = GetCurFrameNum()-1 ;
	}

	//单帧倒放时调整回帧率
	//20090929 bug: 快放时，点击单帧倒放，需要点多下画面才变化
	if (m_nSpeed != PS_NORMAL)
	{
		m_nSpeed = PS_NORMAL;
		m_PlayGragh->changeRate(SPEED[m_nSpeed]);
	}
	    
	m_PlayGragh->pause() ;
	m_PlayGragh->SetStepGoStatus(TRUE);
	m_IsOneFrame = TRUE ;

	SetCurrentFrameNum(nNum) ;

	return TRUE ;
}

/*	$Function		:	SetImgBufCount
==  ===============================================================
==	Description		:	设置缓冲区图像帧数
==	Argument		:	输入参数
							Num 帧数
==				:	
== 	Return		:	
==	Modification	:	2006/4/28		zhougf   Create
==  ===============================================================
*/
BOOL CDHPlay::SetImgBufCount(int Num)
{
	if (Num < 0)
	{
		return FALSE ;
	}

	m_ImgBufCount = Num ;
	
	return TRUE ;
}

/*	$Function		:	GetImgBufCount
==  ===============================================================
==	Description		:	得到缓冲区图像帧数
==	Argument		:	输入参数
							Num 帧数
==				:	
== 	Return		:	
==	Modification	:	2006/4/28		zhougf   Create
==  ===============================================================
*/
int CDHPlay::GetImgBufCount()
{
	return m_ImgBufCount ;
}
       

/*	$Function		:	SetDecCBType
==  ===============================================================
==	Description		:	设置解码回调类型
==	Argument		:	输入参数：
==				:			dectype 解码回调类型
							
== 	Return		:		
==	Modification	:	2006/4/28		zhougf   Create
==  ===============================================================
*/
BOOL CDHPlay::SetDecCBType(DWORD dectype)
{
	if (m_pDisplay == NULL)
	{
		m_pDisplay = new CDisplay(m_nIndex) ;
	}
	
	return m_pDisplay->SetDecCBType(dectype) ;
}
///////////////////////////////////////////////////////流接口相关//////////////////////////////////////////////////
/*	$Function		:	InputData
==  ===============================================================
==	Description		:	输入音视频流数据
==	Argument		:	输入参数：
==				:			pBuf 音视频流数据
                            nSize 音视频流数据长度
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::InputData(PBYTE pBuf,DWORD nSize)
{
#ifdef _DEBUG
//	char str[100] ;
//	sprintf(str,"Port = %d,InputData,TickCount=%d, nSize=%d!!!!!!!!!!!!!!!!!!!!!!\n",m_nIndex,GetTickCount(), nSize) ;
//   	OutputDebugString(str) ;
#endif
//
//	char str_zgf[100];
//	sprintf(str_zgf, "c://0912_download_%d.dav", m_nIndex);

	
	if (m_nStatus != PS_PLAY || m_pDHFile == NULL)
	{
		return FALSE ;
	}

	BOOL iRet = TRUE ;

	if (m_nIndex == 100)
	{
		static unsigned char PCMHeader[8] = {0x00,0x00,0x01,0xF0,30,0x02,0x00,0x00};
		int len = nSize&0xffff;
		PCMHeader[6] = len & 0xff;
		PCMHeader[7] = len >> 8;
		iRet = m_pDHFile->InputData(PCMHeader, 8);
	}

	if (iRet)
	{
		iRet = m_pDHFile->InputData(pBuf,nSize) ;
	}

// 	// 处理写堵塞丢帧
// 	if (!iRet)
// 	{
// 		m_PlayGragh->reset(Reset_level_RAWYUV);
// 		m_ResetStreamFlag = Reset_level_RAWYUV;
// 	}

	return iRet ;
}

BOOL CDHPlay::SetDecAudio(BOOL IfDecAudio) 
{
	m_IsSoundPlay = IfDecAudio ;

	if (m_PlayGragh == NULL)
	{
		return FALSE ;	
	}	

	m_PlayGragh->SetDecAudio(IfDecAudio) ;

	if (m_IsSoundPlay == TRUE)
	{
		m_pDisplay->OpenSound() ;
	}
	else
	{
		m_pDisplay->CloseSound() ;
	}

	return TRUE ;
}
/*	$Function		:	InputVideoData
==  ===============================================================
==	Description		:	输入视频流
==	Argument		:	输入参数：
==				:			pBuf 视频流数据
                            nSize 视频流数据长度
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::InputVideoData(PBYTE pBuf,DWORD nSize)
{
	if (m_PlayGragh == NULL)
	{
		return FALSE ;
	}

	m_StreamType = STREAM_VIDEO ;
	
	BOOL IfDecAudio = FALSE ;
	m_PlayGragh->SetDecAudio(IfDecAudio) ;//不传音频流
	
	BOOL IfDecVideo = TRUE ;
	m_PlayGragh->SetDecVideo(IfDecVideo) ;//传视频流

	return InputData(pBuf,nSize);
}
/*	$Function		:	InputAudioData
==  ===============================================================
==	Description		:	输入音频流
==	Argument		:	输入参数：
==				:			pBuf 音频流数据
                            nSize 音频流数据长度
== 	Return		:		
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::InputAudioData(PBYTE pBuf,DWORD nSize)
{
	if (m_PlayGragh == NULL)
	{
		return FALSE  ;
	}
	
	m_StreamType = STREAM_AUDIO ;

	BOOL IfDecVideo = FALSE ;
	m_PlayGragh->SetDecVideo(IfDecVideo) ;//不传视频流
	
	if (m_IsSoundPlay)
	{
		BOOL IfDecAudio = TRUE ;
		m_PlayGragh->SetDecAudio(IfDecAudio) ;
	}

	return InputData(pBuf,nSize);
}

/*	$Function		:	GetSourceBufferRemain
==  ===============================================================
==	Description		:	获取流模式下剩余缓冲大小
==	Argument		:	输入参数：
==				:			
== 	Return		:	
==	Modification	:	2006/01/16		chenmy   Create
==  ===============================================================
*/
DWORD CDHPlay::GetSourceBufferRemain()
{
	int dwRet = 0;

	if(m_pDHFile)
	{
		dwRet = m_pDHFile->GetBufferRemain();
	}

	return dwRet;
}

/*	$Function		:	ResetBufferRemain
==  ===============================================================
==	Description		:	复位流模式下剩余数据
==	Argument		:	输入参数：
==				:			
== 	Return		:	
==	Modification	:	2006/01/16		chenmy   Create
==  ===============================================================
*/
BOOL  CDHPlay::ResetBufferRemain()
{
	if (m_PlayGragh)
	{
		m_PlayGragh->reset(Reset_level_FASTRETURN) ;//快速返回
		m_ResetStreamFlag = Reset_level_STREAMPASER ;
	}
	else
	{
		return FALSE ;
	}

	m_bFlag = 0 ;

	return TRUE;
}
/*	$Function		:	ResetBuffer
==  ===============================================================
==	Description		:	复位缓冲
==	Argument		:	输入参数：
==				:			nBufType 缓冲类型
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::ResetBuffer(DWORD nBufType)
{
	if (!m_PlayGragh)
	{
		return FALSE ;
	}

	int iRet =0;	

	switch(nBufType) 
	{
	case BUF_AUDIO_SRC :    
		return 0;
	case BUF_VIDEO_SRC :                 //字节数（只对流操作有效）
		if (m_pDHFile && m_pDHFile->GetDataOpenMode() == FM_STREAM)
		{
			m_PlayGragh->reset(Reset_level_FASTRETURN) ;//快速返回
			m_ResetStreamFlag = Reset_level_RAWYUV;
			iRet = 1 ;
		}
		break;
		
	case BUF_VIDEO_RENDER :	
		if (m_pDHFile && m_pDHFile->GetDataOpenMode() == FM_FILE)
		{
			iRet = GetBufferValue(BUF_VIDEO_RENDER);
			m_PlayGragh->reset(Reset_level_YUV);
			m_pDisplay->SetCurFrameNum(GetCurFrameNum()+iRet);			
		}
		else if (m_pDHFile && m_pDHFile->GetDataOpenMode() == FM_STREAM)
		{
			iRet = GetBufferValue(BUF_VIDEO_RENDER) ;
			m_PlayGragh->reset(Reset_level_YUV);
		}
		
		break;
	case BUF_AUDIO_RENDER: 
		break;
	default:
		break;
	}
	
	if(iRet >= 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
//获取播放器中的缓冲区大小（帧数或者byte）
/*	$Function		:	GetBufferValue
==  ===============================================================
==	Description		:	获取播放器中的缓冲区大小（帧数或者byte）
==	Argument		:	输入参数：
==				:			nBufType 缓冲类型	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
DWORD CDHPlay::GetBufferValue(DWORD nBufType)
{
	DWORD dwRet = 0;

	if (m_pDHFile == NULL || m_PlayGragh == NULL)
	{
		return 0 ;
	}

	switch(nBufType) {
	case BUF_AUDIO_SRC:       
	case BUF_VIDEO_SRC :                 //字节数（只对流操作有效）
		if (m_pDHFile)
		{
			dwRet = m_pDHFile->GetBuffSize();
		}
		break;
	case BUF_VIDEO_RENDER :
		m_PlayGragh->GetImgCount((int*)&dwRet) ;
		break ;
	case BUF_AUDIO_RENDER: 
		break;
	default:
		break;
	}
	return dwRet;
}
////////////////////////////////////////////////////显示处理////////////////////////////////////////////////////////

/*	$Function		:	SetColor
==  ===============================================================
==	Description		:	设置视频参数
==	Argument		:	输入参数：
==				:			nRegionNum 显示区域
							nBrightness 亮度
							nContrast   对比度
							nSaturation 饱和度
							nHue        色调
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::SetColor(DWORD nRegionNum, int nBrightness, int nContrast, int nSaturation, int nHue)
{
	m_ColorInfo.nBrightness = 4 * nBrightness - 256 ;
	m_ColorInfo.nContrast   = 2 * nContrast ;
	m_ColorInfo.nSaturation = nSaturation ;
	m_ColorInfo.nHue        = 45 * nHue / 16 - 180 ;

	return TRUE;
}


/*	$Function		:	GetColor
==  ===============================================================
==	Description		:	获取视频参数
==	Argument		:	输出参数：
==				:			nRegionNum 显示区域
							nBrightness 亮度
							nContrast   对比度
							nSaturation 饱和度
							nHue        色调
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::GetColor(DWORD nRegionNum, int *pBrightness, int *pContrast, int *pSaturation, int *pHue)
{
	if (pBrightness == NULL || pContrast == NULL || 
		pSaturation == NULL || pHue == NULL)
	{
		return FALSE ;
	}

	*pBrightness = (m_ColorInfo.nBrightness+256) / 4 ;
	*pContrast   = m_ColorInfo.nContrast / 2 ;
	*pSaturation = m_ColorInfo.nSaturation ;
	*pHue        = ((m_ColorInfo.nHue+180)*16) / 45  ;

	return TRUE;
}

BOOL CDHPlay::ReFreshEx(int nRegionNum)
{
	if (NULL == m_PlayGragh)
	{
		return FALSE;
	}

	unsigned char* pDataBuf = m_PlayGragh->GetLastFrame();

	if (NULL == pDataBuf)
	{
		return FALSE;
	}

	if (m_pDisplay)
	{
		m_pDisplay->ShowRegion(nRegionNum, pDataBuf) ;
	}
	
	return TRUE ;
}
/*	$Function		:	RefreshPlay
==  ===============================================================
==	Description		:	刷新显示
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
BOOL CDHPlay::RefreshPlay()             
{
	if (m_nStatus != PS_PLAY)
	{
		return FALSE ;
	}

	if (m_pDisplay)
	{
		m_pDisplay->Show();
	}
			
	return TRUE ;
}

BOOL CDHPlay::SetDisplayType(int displaytype)
{
	if (m_pDisplay)
	{
		return m_pDisplay->SetDisplayType(displaytype) ;
	}
	
	return FALSE ;
}

int CDHPlay::GetDisplayType()
{
	if (m_pDisplay)
	{
		return m_pDisplay->GetDisplayType() ;
	}

	return -1 ;
}

BOOL CDHPlay::AdjustWaveAudio(BYTE* pcm, unsigned int datalen, int bitsPerSample,long nCoefficient)//调整波形
{
	if (m_pDisplay)
	{
		return m_pDisplay->AdjustWaveAudio(pcm,datalen,bitsPerSample,nCoefficient) ;
	}
	else ;

	return FALSE ;
}

BOOL CDHPlay::SetCoefficient(int nCoefficient)
{
	if (m_pDisplay)
	{
		return m_pDisplay->SetCoefficient(nCoefficient) ;
	}
	else ;

	return FALSE ;
}

BOOL  CDHPlay::StartDataRecord(char *sFileName, int idataType)
{
	if (m_pDHFile == NULL)
	{
		return FALSE ;
	}
	
	std::string filefullname;

	filefullname = sFileName ;

	std::string::size_type pos = 0 ;
	std::string::size_type idx =  0 ;

	// 获取录像文件所在的盘符
	memset(m_cDiskNum, 0, 4);
	idx = filefullname.find(":", pos);
	memcpy(m_cDiskNum, sFileName, idx+1);

	unsigned __int64 iFreeBytes;
	GetDiskFreeSpaceEx(m_cDiskNum, (PULARGE_INTEGER)&iFreeBytes, NULL, NULL);
	
	if (iFreeBytes <= 0)
	{
		return FALSE;
	}
	
	while ((idx = filefullname.find("\\", pos)) !=  std::string::npos)
	{
		CreateDirectory((filefullname.substr(0, idx)).c_str(), NULL) ;
		pos = idx + 1 ;
	}

	if (idataType == 1)
	{
		m_PlayGragh->ConvToAVI((unsigned char*)sFileName);	
		return TRUE;
	}
	
	m_RecordFile = CreateFile(
		sFileName,
		GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (m_RecordFile == NULL )
	{
		return FALSE ;
	}

	return TRUE ;
}

BOOL CDHPlay::StartAVIResizeConvert(char *sFileName, long lDesWidth, long lDesHeight)
{
	if (m_pDHFile == NULL)
	{
		return FALSE ;
	}
	
	std::string filefullname;
	
	filefullname = sFileName ;
	
	std::string::size_type pos = 0 ;
	std::string::size_type idx =  0 ;
	
	// 获取录像文件所在的盘符
	memset(m_cDiskNum, 0, 4);
	idx = filefullname.find(":", pos);
	memcpy(m_cDiskNum, sFileName, idx+1);
	
	unsigned __int64 iFreeBytes;
	GetDiskFreeSpaceEx(m_cDiskNum, (PULARGE_INTEGER)&iFreeBytes, NULL, NULL);
	
	if (iFreeBytes <= 0)
	{
		return FALSE;
	}
	
	while ((idx = filefullname.find("\\", pos)) !=  std::string::npos)
	{
		CreateDirectory((filefullname.substr(0, idx)).c_str(), NULL) ;
		pos = idx + 1 ;
	}
	
	m_PlayGragh->SetAVIConvSize(lDesWidth, lDesHeight);
	m_PlayGragh->ConvToAVI((unsigned char*)sFileName);	

	return TRUE;
}

BOOL CDHPlay::StopDataRecord()
{
	if (m_PlayGragh)
	{
		m_PlayGragh->StopAVIConv();
	}

	__try
	{
		if (m_RecordFile)
		{
			CloseHandle(m_RecordFile) ;
			m_RecordFile = NULL ;
		}
	}
	__except(0,1)
	{
		m_RecordFile = NULL ;
	}

	return TRUE ;
}

BOOL CDHPlay::AdjustFluency(int bufnum, int adjustrange)
{
	BOOL bRet = TRUE;

	if (m_PlayGragh)
	{
		m_PlayGragh->AdjustFluency(bufnum, adjustrange);
	}
	else
	{
		bRet = FALSE;
	}

	return bRet;
}

unsigned char* CDHPlay::GetLastFrame()
{
	if (m_PlayGragh == NULL)
	{
		return NULL;
	}

	return	m_PlayGragh->GetLastFrame();
}

BOOL CDHPlay::GetMediaInfo(char* pBuf, int len)
{
	BOOL bRet =TRUE;

	if (m_pDHFile->GetDataOpenMode() == FM_FILE)
	{
		bRet = m_PlayGragh->GetMediaInfo(pBuf, len);
		if (!bRet)
		{
			bRet = m_pDHFile->GetMediaInfo(pBuf, len);
		}
	}
	else
	{
		bRet = m_PlayGragh->GetMediaInfo(pBuf, len);
	}

	return bRet;
}

void CDHPlay::GetTimeStr(char* pBuf,int len)
{
	if (m_PlayGragh == NULL)
	{
		memset(pBuf, 0, len);
	}
	else
	{
		m_PlayGragh->GetTimeStr(pBuf);
	}
}

BOOL CDHPlay::FixRate(int rate)
{
	BOOL bRet = TRUE;

	if (m_PlayGragh)
	{
		bRet = m_PlayGragh->FixRate(rate);
	}

	return bRet;
}

DWORD WINAPI ThreadProcFileEnd(LPVOID pParam)
{
	int port = (int)pParam ;
	g_cDHPlayManage.pCallback[port]->ProcessFileEnd() ;

	return 0 ;
}
/////////////////////////////////////////////////////主线程控制///////////////////////////////////////////////////////////
/*	$Function		:	GetAVData
==  ===============================================================
==	Description		:	获得数据，并作相应处理
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
int	 CDHPlay::GetAVData()
{
	int iRet = 0;
	DWORD dwThreadId ;
	DWORD len ;

	switch(m_nStatus)
	{
	case PS_STOP:
		return 0 ;					

	case PS_PLAY:    //正向读取数据		

		iRet = m_pDHFile->Read(m_mBuffer, MAX_CHUNKSIZE);

		if (iRet == 0)
		{
			Sleep(1) ;
			if (!m_IsPause && !m_IsOneFrame && m_pDHFile->GetDataOpenMode() == FM_FILE 
				&& GetBufferValue(BUF_VIDEO_RENDER) == 0 && m_IsFileEnd == FALSE)
			{
				if (g_cDHPlayManage.pMsgFileEnd[m_nIndex] && 
					g_cDHPlayManage.pMsgFileEnd[m_nIndex]->nMsgFlag)
				{

#ifdef _DEBUG
					char str[100];
					sprintf(str, "PostMessage %d", m_nIndex);
					OutputDebugString(str);
#endif
					PostMessage(g_cDHPlayManage.pMsgFileEnd[m_nIndex]->hWnd,
						g_cDHPlayManage.pMsgFileEnd[m_nIndex]->nMsg,m_nIndex,m_nIndex) ;	
				}

				if (g_cDHPlayManage.pCallback[m_nIndex] && 
					g_cDHPlayManage.pCallback[m_nIndex]->GetCBStatus(CBT_FileEnd))
				{
					CreateThread( 
					NULL,                        // no security attributes 
					0,                           // use default stack size  
					ThreadProcFileEnd,                  // thread function 
					(void*)m_nIndex,                // argument to thread function 
					0,                           // use default creation flags 
					&dwThreadId); 				
				}

				m_IsFileEnd = TRUE ;
			}//end of if (!m_IsPause && !m_IsOneFrame && m_pDHFile->GetDataOpenMode()
			
			return 0 ;
		}
		else if (iRet < 0)
		{
			Sleep(10) ;
			return 0 ;
		}
		else 
		{
			__try
			{
				if (m_RecordFile != NULL)
				{
					unsigned __int64 iFreeBytes;
					GetDiskFreeSpaceEx(m_cDiskNum, (PULARGE_INTEGER)&iFreeBytes, NULL, NULL);

					if (iRet <= iFreeBytes)
					{
						BOOL ret = WriteFile(m_RecordFile, m_mBuffer, iRet, &len,NULL) ;
						if (ret) FlushFileBuffers(m_RecordFile);
					}
					else
					{
						CloseHandle(m_RecordFile) ;
						m_RecordFile = NULL ;
					}
				} 
			}
			__except(0,1)
			{
				if (m_RecordFile)
				{
					CloseHandle(m_RecordFile) ;
					m_RecordFile = NULL ;
				}
			}
			
		}

		break;

	case PS_ONEFRAME:
				
		iRet = m_pDHFile->ReadOneFrame(m_mBuffer) ;

		if (iRet <= 0 )
		{
			int width ;
			int height ;

			BYTE* pBuf = m_PlayGragh->GetLocateFrameData(&width, &height) ;

			if (width != 0 && height != 0)
			{
				precess(pBuf,width,height) ;
				
				__try
				{
					m_pDisplay->ReciveYUV(m_nIndex,pBuf,pBuf+width*height,pBuf+width*height*5/4,width,height,0) ;			
				}
				__except(0,1)
				{
					int wlj = 0 ;
				}
		
				m_pDisplay->SetCurFrameNum(m_tepFrameSeted) ;
			}

			if (m_nStatus != PS_STOP)
			{
				m_nStatus = PS_PLAY ;		
			}

			m_PlayGragh->SetIfFrameUsed(TRUE) ;

			m_CurPlayedTime = 0;
		
			if (!m_IsPause && !m_IsOneFrame)
			{
				m_PlayGragh->start() ;
			}
		}		
		break ;
	default :
		return 0;
	}
	
	return iRet;
}

BOOL CDHPlay::SetWaterMarkCallbackFunc(void *pvFunc, long nUser)
{
	if (pvFunc != NULL)
	{
		m_PlayGragh->SetWaterMarkCallback(pvFunc, nUser);
		return TRUE;
	}

	return FALSE;
}

BOOL CDHPlay::SetWaterPandoraMarkCallbackFunc(void *pvFunc, long nUser)
{
	if (pvFunc != NULL)
	{
		m_PlayGragh->SetPandoraWaterMarkCallback(pvFunc, nUser);
		return TRUE;
	}
	
	return FALSE;
}

BOOL CDHPlay::SetIVSCallbackFunc(void *pIVSFunc, long nUser)
{
	if (pIVSFunc != NULL)
	{
		m_PlayGragh->SetIVSCallback(pIVSFunc, nUser);
		return TRUE;
	}
	
	return FALSE;
}

BOOL CDHPlay::GetRealFrameBitRate(double *pBitRate)
{
	if (m_PlayGragh == NULL) return FALSE;

	*pBitRate = m_PlayGragh->GetFrameBitRate();

	return TRUE;
}

/*	$Function		:	PlayProc
==  ===============================================================
==	Description		:	PlayProc
==	Argument		:	主线程控制函数
==				:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
int CDHPlay::PlayProc()
{	
	while (m_nStatus != PS_STOP)
	{		
		//某些用户操作放到主线程中进行，以与数据解码同步
		if (m_ResetStreamFlag >= 0)//流方式，用户清空缓冲
		{
			m_pDHFile->ResetRemainData() ;
			m_PlayGragh->reset(m_ResetStreamFlag) ;
			m_ResetStreamFlag = -1 ;
			m_bFlag = 0;
		}
		
		if (m_userfilepos >= 0)//视毫文件，定位
		{
			DWORD tmp_userfilepos = m_userfilepos ;
			m_PlayGragh->reset(Reset_level_RAWYUV) ;
			m_pDHFile->ChangePos(tmp_userfilepos) ;
			m_userfilepos = -1 ;
			m_IsFileEnd = FALSE ;
		}

		if (m_fRelativePos >= 0)//不能建立索引的文件，定位
		{
			float tmp_RelativePos = m_fRelativePos ;
			DWORD dwCurDataPos = 0;

		 	m_PlayGragh->reset(Reset_level_RAWYUV) ;
			m_pDHFile->SetPos(tmp_RelativePos) ;

			//根据播放百分位，重新设置已处理数据的长度
			m_pDHFile->GetDataPos(tmp_RelativePos, &dwCurDataPos);
			m_PlayGragh->SetHandledDataLen(dwCurDataPos);

			m_fRelativePos = -1 ;
			m_IsFileEnd = FALSE ;
		}	

		if (m_FrameNumSeted >= 0)//能建立索引的文件，精确定位
		{
			DWORD nCurFrameNum = 0 ;
			DWORD EndPos = 0 ;

			m_tepFrameSeted = m_FrameNumSeted ;

			nCurFrameNum = m_pDHFile->SetCurrentFrameNum(m_tepFrameSeted);//得到指定帧前一I帧的帧序号，并将文件位置设为这一I帧的位置		
			EndPos = m_pDHFile->GetFrameEndPos(m_tepFrameSeted) ;//得到指定帧在文件中的结束位置
			m_pDHFile->SetEndPos(EndPos) ;//设置读指针结束位置	
			m_FrameNumSeted = -1 ;	

			m_PlayGragh->reset(Reset_level_RAWYUV) ;

			if (m_nStatus == PS_STOP)
			{
				break;
			}
			m_nStatus = PS_ONEFRAME ;	
			
			m_PlayGragh->SetIfFrameUsed(FALSE) ;
			m_IsFileEnd = FALSE ;
		}

		//流模式下，当缓冲小于某一阀值回调
		if (g_cDHPlayManage.pCallback[m_nIndex] && 
			g_cDHPlayManage.pCallback[m_nIndex]->GetCBStatus(CBT_SourceBuf) )
		{
			g_cDHPlayManage.pCallback[m_nIndex]->ProcessSourceBuf();
		}

		 //取数据放到回放缓冲
		m_bFlag = GetAVData();	
	
		if (m_bFlag > 0)   //缓冲数据有效送数据
		{
			__try
			{
				int ret = m_PlayGragh->parseData(m_mBuffer, m_bFlag);
				if (ret > 0) g_cDHPlayManage.m_error[m_nIndex] = ret;
			}
			__except(0,1)
			{
				int wlj = 0;
			}
		}
	}
	return 0;
}

BOOL CDHPlay::SetRotateAngle(int rotateType)
{
	return m_pDisplay->SetRotateAngle(rotateType);	
}

BOOL CDHPlay::precess(unsigned char *data, int width, int height)
{
// 	if ((width == 176)&&( height == 128))	//N制QCIF，去掉最后8行
// 	{
// 		height = 120;	
// 	}

	if (m_ColorInfo.nBrightness != DEFAULT_BRIGHTNESS || m_ColorInfo.nContrast != DEFAULT_CONTRAST)
	{
		adjust_lum(data, data, width * height, m_ColorInfo.nContrast, m_ColorInfo.nBrightness);			
	}

	if (m_ColorInfo.nSaturation != DEFAULT_SATURATION || m_ColorInfo.nHue != DEFAULT_HUE)
	{
		adjust_hueSat(data+width*height, data+width*height*5/4, data+width*height,
				data+width*height*5/4, width/2, width/2, height/2, m_ColorInfo.nHue, m_ColorInfo.nSaturation);
	}

	
	//图像初始高度和宽度
	int m_width  = m_pDisplay->GetPicWidth();
	int m_height = m_pDisplay->GetPicHeight();
				
	if ((m_height != height || m_width != width))  //当图像格式发生改变
	{	
		g_cDHPlayManage.pDHPlay[m_nIndex]->m_pDisplay->SetWidthHeight(width,height);

		//如果设置了编码格式改变发送消息
		if (g_cDHPlayManage.pMsgEncChang[m_nIndex] &&
			g_cDHPlayManage.pMsgEncChang[m_nIndex]->nMsgFlag)
		{
			PostMessage(g_cDHPlayManage.pMsgEncChang[m_nIndex]->hWnd,g_cDHPlayManage.pMsgEncChang[m_nIndex]->nMsg,0,0) ;	
		}					
		//如果设置了编码格式改变回调，就调用相应回调函数
		if (g_cDHPlayManage.pCallback[m_nIndex]&&g_cDHPlayManage.pCallback[m_nIndex]->GetCBStatus(CBT_EncChangeCBFun))
		{
			g_cDHPlayManage.pCallback[m_nIndex]->ProcessEncChange() ;
		}
	}//end of if ((m_height != height || m_width != width)) 

	return 0 ;
}


static bool hueInited = false;
static int hueSin[360+1];
static int hueCos[360+1];

//__declspec(align(8)) 
static const __int64 m128=0x0080008000800080;

void adjust_hueSat(
	unsigned char *srcU,unsigned char *srcV,
	unsigned char *dstU,unsigned char *dstV,
	int stride, int dx,int dy,
	int huue, int saturation)
{
	int hue = huue;          //-180 ... 0 ... 180
	int sat = saturation ;//0 (BW) - 64 (normal) - 128 (too much color);
	if (!hueInited) {
		__asm {emms};
		for (int i=-180,ii=0;i<=180;ii++,i++)
		{
			double Hue=(i * 3.1415926) / 180.0;
			hueSin[ii]=int(sin(Hue) * 128);
			hueCos[ii]=int(cos(Hue) * 128);
		}
		hueInited=true; 
	} 
	// #define CHUESAT
#ifdef CHUESAT
	int Sin = hueSin[hue+180];// (int) (sin(Hue) * 4096);
	int Cos = hueCos[hue+180];// (int) (cos(Hue) * 4096);
	int diffx=stride-dx;
#else
	__int64 Sin = hueSin[hue+180];
	__int64 Cos = hueCos[hue+180];
	__int64 Sat = sat;
//	__declspec(align(8)) 
		static __int64 Sin64,Cos64,Sat64;
	Sin64=(Sin&0x000000000000ffff)+((Sin<<16)&0x00000000ffff0000)+((Sin<<32)&0x0000ffff00000000)+((Sin<<48)&0xffff000000000000);
	Cos64=(Cos&0x000000000000ffff)+((Cos<<16)&0x00000000ffff0000)+((Cos<<32)&0x0000ffff00000000)+((Cos<<48)&0xffff000000000000);
	Sat64=(Sat&0x000000000000ffff)+((Sat<<16)&0x00000000ffff0000)+((Sat<<32)&0x0000ffff00000000)+((Sat<<48)&0xffff000000000000);
#endif/
	for (unsigned char *srcUend=srcU+stride*dy;srcU<srcUend;srcU+=stride,srcV+=stride,dstU+=stride,dstV+=stride)
	{
		unsigned char *srcUlnEnd=srcU+dx;
#ifdef CHUESAT
		stride=diffx;
		for (;srcU<srcUlnEnd;srcU++,srcV++,dstU++,dstV++)
		{
			int u = *srcU - 128;
			int v = *srcV - 128;
			int u2 = ((u * Cos)>>7) + ((v * Sin)>>7);
			int v2 = ((v * Cos)>>7) - ((u * Sin)>>7);
			u = ((u2 * sat) >> 6) + 128;
			v = ((v2 * sat) >> 6) + 128;
			if (u < 0) u = 0;
			if (u > 255) u = 255;
			if (v < 0) v = 0;
			if (v > 255) v = 255;
			*dstU = u;
			*dstV = v;
		} 
#else
		//srcU[0]=1;srcU[1]=2;srcU[2]=3;srcU[3]=4;
		//srcV[0]=5;srcV[1]=6;srcV[2]=7;srcV[3]=8;
		__asm 
		{
			mov eax,[srcUlnEnd]
			mov ecx,[srcU]
			mov esi,[srcV]
			mov edx,[dstU]
			mov edi,[dstV]
			movq mm0,[Sat64] // mm0 = Sat64
			movq mm1,[Sin64] // mm1 = Sin64
			movq mm2,[Cos64] // mm2 = Cos64
			lineLoop:
			pxor mm3,mm3
			punpcklbw mm3,[ecx]  
			psrlw mm3,8     //mm3 = *srcU        

			pxor mm4,mm4
			punpcklbw mm4,[esi]  
			psrlw mm4,8     //mm4 = *srcV        

			movq mm7,[m128]
			PSUBSW mm3,mm7  //mm3 = u = *srcU - 128
			PSUBSW mm4,mm7  //mm4 = v = *srcV - 128

			movq mm5,mm3
			PMULLW mm5,mm2     //mm5 = u * Cos
			PSRAW mm5,7        //mm5 = u2 = (u * Cos + v * Sin)>>12
			movq mm6,mm4
			PMULLW mm6,mm1     //mm6 = v * Sin
			PSRAW mm6,7
			paddw mm5,mm6      //mm5 = u * Cos + v * Sin
			PMULLW mm5,mm0     //mm5 = u2 * Sat
			PSRAW mm5,6        //mm5 =(u2 * Sat)>>8
			PADDW mm5,[m128] //mm5 =(u2 * Sat)>>8 +128

			movq mm7,mm4
			PMULLW mm7,mm2     //mm7 = v * Cos
			PSRAW mm7,7
			movq mm6,mm3
			PMULLW mm6,mm1     //mm6 = u * Sin
			PSRAW mm6,7        //mm7 = v2 = (v * Cos + u * Sin)>>12
			psubw mm7,mm6      //mm7 = v * Cos - u * Sin
			PMULLW mm7,mm0     //mm7 = v2 * Sat
			PSRAW mm7,6        //mm7 =(v2 * Sat)>>8
			PADDW mm7,[m128] //mm7 =(v2 * Sat)>>8 +128

			pxor mm6,mm6
			packuswb mm5,mm6
			movd [edx],mm5
			pxor mm6,mm6
			packuswb mm7,mm6
			movd [edi],mm7
			add ecx,4
			add esi,4
			add edx,4
			add edi,4
			cmp ecx,eax
			jl  lineLoop
		}
#endif 
	}; 
	__asm {emms}; 
}

void adjust_lum(unsigned char *src, unsigned char *dst,int area,int LumGain,int LumOffset)
{
	//	int area = Coded_Picture_Width * Coded_Picture_Height;
	static const __int64 mmmask_0064 = 0x0040004000400040;	
	
	__int64 LumGainMask = ((__int64)LumGain<<48) + ((__int64)LumGain<<32) + ((__int64)LumGain<<16) + (__int64)LumGain;
	__int64 LumOffsetMask = ((__int64)LumOffset<<48) + ((__int64)LumOffset<<32) + ((__int64)LumOffset<<16) + (__int64)LumOffset;
	
	__asm
	{
		mov			eax, [src]
			mov			ebx, [dst]
			mov			esi, 0x00
			mov			edi, [area]
			pxor		mm0, mm0
			movq		mm5, [LumOffsetMask]
			movq		mm6, [LumGainMask]
			movq		mm7, [mmmask_0064]
			
lumconv:
		movq		mm1, [eax+esi]

			movq		mm2, mm1
			
			punpcklbw	mm1, mm0

			punpckhbw	mm2, mm0
			
			pmullw		mm1, mm6

			pmullw		mm2, mm6
			
			paddw		mm1, mm7

			paddw		mm2, mm7
			
			psrlw		mm1, 7
			psrlw		mm2, 7

			paddw		mm1, mm5
			paddw		mm2, mm5

			packuswb	mm1, mm0
			packuswb	mm2, mm0

			add			esi, 0x08
			cmp			esi, edi
			
			//		movq		[ebx+esi-8], mm1
			//		movq		[ebx+esi-4], mm2
			PUNPCKLDQ	mm1, mm2//zyd
			
			movq        [ebx+esi-8], mm1//zyd
			
			jl			lumconv
			emms
	}
}

