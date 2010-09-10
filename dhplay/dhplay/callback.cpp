/*
**  ************************************************************************
**                                     DHPlay  通用播放SDK
**         (c) Copyright 1992-2004, ZheJiang Dahua Technology Stock Co.Ltd.
**                            All Rights Reserved
**
**	File  Name	: callback.cpp
**	Description	: 播放sdk回调控制程序
**	Modification	: 2005/01/10		chenmy		Create the file
**  ************************************************************************
*/
#include "StdAfx.h"
#include "callback.h"
#include "playmanage.h"

/*	$Function		:	CCallback
==  ===============================================================
==	Description		:	构造函数
==	Argument		:	输入参数：
							nPort 端口号
==				:	
== 	Return		:	
==	Modification	:	2005/01/10		chenmy   Create
==  ===============================================================
*/
CCallback::CCallback(LONG nPort)
{
	m_nIndex = nPort;               //播放索引	

	for(int i = 0; i < CBT_TOTAL; i++)
	{
		m_cbState[i] = false;           
	}

	m_cbStreamState = TRUE ;
	
	//初始化各回调函数指针为空指针 
	cb_Dec         = NULL ;                  
	temp_Dec       = NULL ;

	cb_Display     = NULL ;
	temp_Display   = NULL ;	

	cb_Draw        = NULL ;
	temp_Draw      = NULL ;

	cb_Audio       = NULL ;
	temp_Audio     = NULL ;

	cb_Verify      = NULL ;
	temp_Verify    = NULL ;

	cb_EncChange   = NULL ;
	temp_Verify    = NULL ;

	cb_FileRef     = NULL ;
	temp_FileRef   = NULL ;

	cb_Sourcebuf   = NULL ;
	temp_SourceBuf = NULL ;

	cb_DrawEx      = NULL ;
	temp_DrawEx    = NULL ;
	
	cb_VisibleDec = NULL;
	temp_VisibleDec = NULL;

	cb_demux = NULL;
	temp_demux = NULL;
}

/*	$Function		:	~CCallback
==  ===============================================================
==	Description		:	析构函数
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/01/10		chenmy   Create
==  ===============================================================
*/
CCallback::~CCallback()
{

}


/*	$Function		:	GetCBStatus
==  ===============================================================
==	Description		:	获取回调状态函数，获得某回调函数是否设置有效 1有效，0 无效
==	Argument		:	输入参数：
							nStatus 回调函数类型
==				:	
== 	Return		:	
==	Modification	:	2005/01/10		chenmy   Create
==  ===============================================================
*/
BOOL CCallback::GetCBStatus(CALLBACK_TYPE nStatus)
{
	if (nStatus >= CBT_TOTAL)
	{
		return FALSE ;
	}
	return m_cbState[nStatus]  ;
}

/*	$Function		:	SetDecCallBack
==  ===============================================================
==	Description		:	解码回调控制函数，设置/取消解码回调
==	Argument		:	输入参数：
							pcbDec 解码回调函数指针
==				:	
== 	Return		:	
==	Modification	:	2005/01/10		chenmy   Create
==  ===============================================================
*/
BOOL CCallback::SetDecCallBack(Callback_Dec pcbDec,BOOL IfSet)
{
	//将函数指针指向由参数所传递进来的函数名
	m_cbState[CBT_DecCBFun] = IfSet ;
	mParam_decCB.nUser = 0;
	temp_Dec = pcbDec ;
	
	if (temp_Dec == NULL)
		m_cbState[CBT_DecCBFun] = false;

	return TRUE;	
}

/*	$Function		:	SetDecCallBackEx
==  ===============================================================
==	Description		:	解码回调扩展控制函数，设置/取消解码回调
==	Argument		:	输入参数：
							pcbDec 解码回调函数指针
==				:			nUser  用户数据
== 	Return		:	
==	Modification	:	2009/02/16		wanglanjun   Create
==  ===============================================================
*/
BOOL CCallback::SetDecCallBackEx(Callback_Dec pcbDec,long nUser,BOOL IfSet)
{
	//将函数指针指向由参数所传递进来的函数名
	m_cbState[CBT_DecCBFun] = IfSet ;
	mParam_decCB.nUser = nUser;
	temp_Dec = pcbDec ;
	
	if (temp_Dec == NULL)
		m_cbState[CBT_DecCBFun] = false;

	return TRUE;	
}

/*	$Function		:	SetVisibleDecCallBack
==  ===============================================================
==	Description		:	解码回调同时显示视频，设置/取消解码回调
==	Argument		:	输入参数：
							pcbDec 解码回调函数指针
==				:			nUser  用户数据
== 	Return		:	
==	Modification	:
==  ===============================================================
*/
BOOL CCallback::SetVisibleDecCallBack(Callback_Dec pcbDec,long nUser,BOOL IfSet)
{
	//将函数指针指向由参数所传递进来的函数名
	m_cbState[CBT_VisibleDec] = IfSet ;
	mParam_decCB.nUser = nUser;
	temp_VisibleDec = pcbDec ;
	
	if (temp_VisibleDec == NULL)
		m_cbState[CBT_VisibleDec] = false;

	return TRUE;	
}

/*	$Function		:	ProcessDec
==  ===============================================================
==	Description		:	解码回调函数，处理解码回调
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/01/10		chenmy   Create
==  ===============================================================
*/
BOOL CCallback::ProcessDec(DEC_TYPE type,BYTE* pBuf,int nSize,int width,int height)
{	
	FRAME_INFO nFrameInfo ;
	
	if (type == DEC_VIDEO) 
	{
		//参数附值
		nFrameInfo.nWidth = width; //画面宽，单位像素。如果是音频数据则为0； 
		nFrameInfo.nHeight = height; //画面高。如果是音频数据则为0； 
		nFrameInfo.nStamp = g_cDHPlayManage.pDHPlay[m_nIndex]->GetPlayedTimeEX(); //时标信息，单位毫秒。 
		nFrameInfo.nType = T_YV12; //数据类型，T_AUDIO16，T_RGB32， T_YV12，详见宏定义说明。 
		nFrameInfo.nFrameRate = g_cDHPlayManage.pDHPlay[m_nIndex]->GetFrameRate();//编码时产生的图像帧率。
	}
	
	if (type == DEC_AUDIO)
	{
		nFrameInfo.nWidth = nFrameInfo.nHeight = 0 ;
		nFrameInfo.nStamp = g_cDHPlayManage.pDHPlay[m_nIndex]->GetPlayedTimeEX(); //时标信息，单位毫秒。

		if (width == 8)
		{
			nFrameInfo.nType = T_AUDIO8; //数据类型，T_AUDIO16，T_RGB32， T_YV12，详见宏定义说明。 
		}
		else if (width == 16)
		{
			nFrameInfo.nType = T_AUDIO16;
		}

		nFrameInfo.nFrameRate = height;		
	}

	cb_Dec = temp_Dec;
	//调用解码回调函数
	if (cb_Dec == NULL)
	{
		return FALSE ;
	}
	
	cb_Dec(m_nIndex,(char*)pBuf,nSize,&nFrameInfo, mParam_decCB.nUser/*g_cDHPlayManage.pDHPlay[m_nIndex]->GetPlayedFrames()*/, NULL);

	return TRUE;

}

BOOL CCallback::ProcessVisibleDec(DEC_TYPE type,BYTE* pBuf,int nSize,int width,int height)
{	
	FRAME_INFO nFrameInfo;
	
	if (type == DEC_VIDEO) 
	{
		//参数附值
		nFrameInfo.nWidth = width; //画面宽，单位像素。如果是音频数据则为0； 
		nFrameInfo.nHeight = height; //画面高。如果是音频数据则为0； 
		nFrameInfo.nStamp = g_cDHPlayManage.pDHPlay[m_nIndex]->GetPlayedTimeEX(); //时标信息，单位毫秒。 
		nFrameInfo.nType = T_YV12; //数据类型，T_AUDIO16，T_RGB32， T_YV12，详见宏定义说明。 
		nFrameInfo.nFrameRate = g_cDHPlayManage.pDHPlay[m_nIndex]->GetFrameRate();//编码时产生的图像帧率。
	}
	
	if (type == DEC_AUDIO)
	{
		nFrameInfo.nWidth = nFrameInfo.nHeight = 0 ;
		nFrameInfo.nStamp = g_cDHPlayManage.pDHPlay[m_nIndex]->GetPlayedTimeEX(); //时标信息，单位毫秒。
		
		if (width == 8)
		{
			nFrameInfo.nType = T_AUDIO8; //数据类型，T_AUDIO16，T_RGB32， T_YV12，详见宏定义说明。 
		}
		else if (width == 16)
		{
			nFrameInfo.nType = T_AUDIO16;
		}
		
		nFrameInfo.nFrameRate = height;		
	}
	
	cb_VisibleDec = temp_VisibleDec;
	//调用解码回调函数
	if (cb_VisibleDec == NULL)
	{
		return FALSE ;
	}
	
	cb_VisibleDec(m_nIndex,(char*)pBuf,nSize,&nFrameInfo, mParam_decCB.nUser/*g_cDHPlayManage.pDHPlay[m_nIndex]->GetPlayedFrames()*/, NULL);
	
	return TRUE;
}

/*	$Function		:	SetDisplayCallBack
==  ===============================================================
==	Description		:	抓图回调控制函数，设置抓图回调
==	Argument		:	输入参数：
							pcbDisplay 抓图回调函数指针
==				:	
== 	Return		:	
==	Modification	:	2005/01/10		chenmy   Create
==  ===============================================================
*/
BOOL CCallback::SetDisplayCallBack(Callback_Display pcbDisplay, long nUser)
{
	//将函数指针指向由参数所传递进来的函数名
	temp_Display = pcbDisplay;
	mParam_display.nUser = nUser;

	if (temp_Display != NULL) 
	{
		m_cbState[CBT_DisplayCBFun] = true;
	}
	else 
	{
		m_cbState[CBT_DisplayCBFun] = false;
	}
	
	return TRUE;
}

/*	$Function		:	ProcessDisplay
==  ===============================================================
==	Description		:	抓图回调函数，处理抓图回调
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/01/10		chenmy   Create
==  ===============================================================
*/
BOOL CCallback::ProcessDisplay(BYTE* pBuf,int width,int height)
{
	//参数附值
	cb_Display = temp_Display ;

	if (cb_Display == NULL)
	{
		return FALSE ;
	}
	//调用抓图回调函数
	cb_Display(m_nIndex,(char*)pBuf,width*height*3/2,width,height,
		g_cDHPlayManage.pDHPlay[m_nIndex]->GetPlayedTimeEX(),T_YV12,mParam_display.nUser) ;

	return TRUE;
}
/*	$Function		:	SetDrawCallback
==  ===============================================================
==	Description		:	设置画图板回调
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/01/10		chenmy   Create
==  ===============================================================
*/
BOOL CCallback::SetDrawCallback(Callback_Draw pcbDraw, long nUser)
{
	mParam_draw.nUser = nUser;
	temp_Draw = pcbDraw;

	if (temp_Draw != NULL)
	{
		m_cbState[CBT_DrawCBFun] = true;
	}
	else 
	{
		m_cbState[CBT_DrawCBFun] = false;
	}
	
	return TRUE;
}
BOOL CCallback::ProcessDraw(HDC hDC)
{
	cb_Draw = temp_Draw ;

	if (cb_Draw == NULL)
	{
		return FALSE ;
	}
	
	cb_Draw(m_nIndex,hDC,mParam_draw.nUser) ;
	
	return TRUE ;
}

BOOL CCallback::SetDrawExCallback(Callback_DrawEx pcbDrawEx, long nReginNum, long nUser)
{
	if (nReginNum > MAX_DISPLAY_WND || nReginNum < 0)
	{
		return FALSE;
	}
	
	mParam_drawEx.nUser = nUser;
	temp_DrawEx = pcbDrawEx;
	
	if (temp_DrawEx != NULL)
	{
		m_cbState[CBT_DrawCBFunEx] = true;
	}
	else 
	{
		m_cbState[CBT_DrawCBFunEx] = false;
	}
	
	return TRUE;
}

BOOL CCallback::ProcessDrawEx(long nReginNum, HDC hDC)
{
	cb_DrawEx = temp_DrawEx ;
	
	if (cb_DrawEx == NULL)
	{
		return FALSE ;
	}
	
	cb_DrawEx(m_nIndex,nReginNum, hDC,mParam_drawEx.nUser) ;
	
	return TRUE ;
}

BOOL CCallback::SetFileRefCallBack(Callback_FileRef pcbFileRef, DWORD nUser)
{
	mParam_fileref.nUser = nUser ;
	temp_FileRef = pcbFileRef ;

	if (temp_FileRef != NULL)
	{
		m_cbState[CBT_FILEREF] = true;
	}
	else
	{
		m_cbState[CBT_FILEREF] = false;
	}
	
	return TRUE;
}
BOOL CCallback::ProcessFileRef()
{
	cb_FileRef = temp_FileRef ;

	if (cb_FileRef == NULL)
	{
		return FALSE ;
	}

	cb_FileRef(m_nIndex, mParam_fileref.nUser) ;

	return TRUE ;
}

BOOL CCallback::SetFileRefCallBackEx(Callback_FileRefEx pcbFileRefEx, DWORD nUser)
{
	mParam_fileref.nUser = nUser ;
	temp_FileRefEx = pcbFileRefEx ;
	
	if (temp_FileRefEx != NULL)
	{
		m_cbState[CBT_FILEREFEX] = true;
	}
	else
	{
		m_cbState[CBT_FILEREFEX] = false;
	}
	
	return TRUE;
}

BOOL CCallback::ProcessFileRefEx(BOOL bIndexCreated)
{
	cb_FileRefEx = temp_FileRefEx ;
	
	if (cb_FileRefEx == NULL)
	{
		return FALSE ;
	}
	
	cb_FileRefEx(m_nIndex, bIndexCreated, mParam_fileref.nUser) ;
	
	return TRUE ;
}
/*	$Function		:	SetSourceBufCallBack
==  ===============================================================
==	Description		:	设置数据源小于阀值回调，此回调在回调线程中检测，一旦检测到调用执行，标志置位无效，需要用户复位标志
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/01/17		chenmy   Create
==  ===============================================================
*/
BOOL CCallback::SetSourceBufCallBack(Callback_SourceBuf pcbSourceBuf, DWORD nThreShold, long dwUser, void *pReserved)
{
	mParam_sourcebuf.nThreShold = nThreShold;
	mParam_sourcebuf.nUser = dwUser;
	
	temp_SourceBuf = pcbSourceBuf;

	if (temp_SourceBuf != NULL)
	{
		m_cbState[CBT_SourceBuf] = true;
	}
	else 
	{
		m_cbState[CBT_SourceBuf] = false;
	}
	
	return TRUE;
}

/*	$Function		:	ResetSourceBufFlag
==  ===============================================================
==	Description		:	复位回调标志
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/01/17		chenmy   Create
==  ===============================================================
*/
BOOL CCallback::ResetSourceBufFlag()
{
	if (cb_Sourcebuf == NULL)
	{
		return FALSE;
	}

	m_cbStreamState = TRUE ;
	
	return TRUE;
}
/*	$Function		:	ProcessSourceBuf
==  ===============================================================
==	Description		:	数据源小于阀值回调处理
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/01/17		chenmy   Create
==  ===============================================================
*/
BOOL CCallback::ProcessSourceBuf()
{
	DWORD nRemain;
	
	if (g_cDHPlayManage.pDHPlay[m_nIndex]==NULL)
	{
		return FALSE;
	}

	//取得当前数据源的剩余数据大小
	nRemain = g_cDHPlayManage.pDHPlay[m_nIndex]->GetSourceBufferRemain();

	if (nRemain <= mParam_sourcebuf.nThreShold && m_cbStreamState) 
	{
		cb_Sourcebuf = temp_SourceBuf ;
		
		if (cb_Sourcebuf == NULL)
		{
			return FALSE ;
		}
		else
		{
			m_cbStreamState = FALSE ;
			cb_Sourcebuf(m_nIndex,nRemain,mParam_sourcebuf.nUser,0);			
		}
				
	}
	
	return TRUE;
}
/*	$Function		:	SetAudioCallBack
==  ===============================================================
==	Description		:	音频回调控制函数，设置音频回调
==	Argument		:	输入参数：
							pcbAudio 音频回调函数指针
							nUser 用户
==				:	
== 	Return		:	
==	Modification	:	2005/01/10		chenmy   Create
==  ===============================================================
*/
BOOL CCallback::SetAudioCallBack(Callback_Audio pcbAudio, long nUser)
{
	temp_Audio = pcbAudio ;
	mParam_audio.nUser = nUser;

	if (temp_Audio != NULL)
	{
		m_cbState[CBT_AudioCBFun] = true;
	}
	else
	{
		m_cbState[CBT_AudioCBFun] = false ;
	}
	
	return TRUE;
}

/*	$Function		:	ProcessAudio
==  ===============================================================
==	Description		:	音频回调函数，处理音频回调
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/01/10		chenmy   Create
==  ===============================================================
*/
BOOL CCallback::ProcessAudio(BYTE * pAudioBuf, long nSize, long nStamp, long nType)
{
	cb_Audio = temp_Audio;
	
	if (cb_Audio == NULL)
	{
		return FALSE ;
	}

	//调用音频回调函数
	cb_Audio(m_nIndex, (char*)pAudioBuf, nSize, 
		g_cDHPlayManage.pDHPlay[m_nIndex]->GetPlayedTimeEX(),nType,mParam_audio.nUser);

	return TRUE;
}


/*	$Function		:	SetVerifyCallBack
==  ===============================================================
==	Description		:	水印控制函数，设置水印回调
==	Argument		:	输入参数：
							pcbfunVerify 水印回调函数指针
==				:			nBeginTime 开始时间
							nUser 用户
== 	Return		:	
==	Modification	:	2005/01/10		chenmy   Create
==  ===============================================================
*/
BOOL CCallback::SetVerifyCallBack(Callback_Verify pcbfunVerify, DWORD nBeginTime, DWORD nEndTime, long nUser)
{
	//将函数指针指向由参数传递进来的函数名
	temp_Verify = pcbfunVerify;
	//参数设置
	mParam_verify.nBeginTime = nBeginTime;
	mParam_verify.nEndTime = nEndTime;
	mParam_verify.nUser = nUser;

	if (temp_Verify != NULL)
	{
		m_cbState[CBT_VerifyCBFun] = true;
	}
	else
	{
		m_cbState[CBT_VerifyCBFun] = false;
	}

	return TRUE;
}

/*	$Function		:	ProcessVerify
==  ===============================================================
==	Description		:	水印回调函数，处理水印回调
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/01/10		chenmy   Create
==  ===============================================================
*/
BOOL CCallback::ProcessVerify()
{
	cb_Verify = temp_Verify ;

	if (cb_Verify == NULL)
	{
		return FALSE ;
	}
	//调用水印回调函数
	cb_Verify(m_nIndex, &mParam_verify.nFilePos, mParam_verify.bIsVideo, 
		mParam_verify.nUser);
	
	return TRUE;
}


/*	$Function		:	SetEncChangeCallBack
==  ===============================================================
==	Description		:	编码格式控制函数，改变编码格式回调
==	Argument		:	输入参数：
							pcbEncChange 改变编码格式的回调函数指针
==				:			nUser 用户
== 	Return		:	
==	Modification	:	2005/01/10		chenmy   Create
==  ===============================================================
*/
BOOL CCallback::SetEncChangeCallBack(Callback_EncChange pcbEncChange, long nUser)
{
	//函数指针指向由参数传递过来的函数名
	temp_EncChange = pcbEncChange;

	if (temp_EncChange != NULL)
	{
		m_cbState[CBT_EncChangeCBFun] = TRUE;
	}
	else
	{
		m_cbState[CBT_EncChangeCBFun] = FALSE;
	}
	
	mParam_encchange.nUser = nUser;
	
	return TRUE;
}

/*	$Function		:	ProcessEncChange
==  ===============================================================
==	Description		:	编码格式改变回调函数，处理编码格式改变回调
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/01/10		chenmy   Create
==  ===============================================================
*/
BOOL CCallback::ProcessEncChange()
{
	cb_EncChange = temp_EncChange ;
	
	if (cb_EncChange == NULL)
	{
		return FALSE ;
	}

	cb_EncChange(m_nIndex, mParam_encchange.nUser);
	
	return TRUE;
}

BOOL CCallback::SetFileEndCallBack(Callback_FileEnd pcbFileEnd, DWORD nUser) 
{
	mParam_fileend.nUser = nUser ;
	temp_FileEnd = pcbFileEnd ;

	if (temp_FileEnd != NULL)
	{
		m_cbState[CBT_FileEnd] = TRUE;
	}
	else
	{
		m_cbState[CBT_FileEnd] = FALSE;
	}
	
	return TRUE;
}

BOOL CCallback::ProcessFileEnd()
{
	cb_FileEnd = temp_FileEnd ;

	if (cb_FileEnd == NULL)
	{
		return FALSE ;
	}

	cb_FileEnd(m_nIndex, mParam_fileend.nUser) ;

	return TRUE ;
}

BOOL CCallback::SetDemuxCallBack(Callback_Demux pcbDemux, long nUser)
{
	temp_demux = pcbDemux;
	mParam_demux.nUser = nUser;

	if (temp_demux != NULL) 
	{
		m_cbState[CBT_DemuxCBFun] = true;
	}
	else 
	{
		m_cbState[CBT_DemuxCBFun] = false;
	}
	
	return TRUE;
}

BOOL CCallback::ProcessDemux(char * pBuf,long nSize, void* pParam)
{
	cb_demux = temp_demux;

	if (cb_demux == NULL)
	{
		return FALSE;
	}

	cb_demux(m_nIndex, pBuf, nSize, pParam, 0, mParam_demux.nUser);

	return TRUE;
}