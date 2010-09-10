/*
**  ************************************************************************
**                                    DHPlay  通用播放SDK
**
**         (c) Copyright 1992-2004, ZheJiang Dahua Technology Stock Co.Ltd.
**                            All Rights Reserved
**
**	File  Name	: display.cpp
**	Description	: 播放sdk显示控制程序
**	Modification	: 2005/11/28		chenmy		Create the file
**  ************************************************************************
*/
#include "StdAfx.h"
#include "display.h"
#include "stdio.h"
#include "playmanage.h"

typedef void (__stdcall *pDrawCBFunc)(int index, HDC hDc);


/*	$Function		:	CDisplay
==  ===============================================================
==	Description		:	构造函数
==	Argument		:	输入参数：
							nPort 端口号
==				:			hWnd 窗口句柄
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
CDisplay::CDisplay(int nPort)
{
	m_nIndex = nPort;   //端口号
	m_DisType = ByDDOffscreen ;
	m_colorkey = 0 ;
	m_nWidth = 352 ;         //每帧图片的宽度
	m_nHeight = 288 ;        //每帧图片的高度
	m_CurFrameNum = -1 ;         //当前显示的桢序号
	m_PlayedFrames = 0 ;       //播放桢数
	m_nDisplayType = DISPLAY_NORMAL;
	m_decType = DEC_COMPLEX;	
	m_nCoefficient = 0 ;
	m_volume       = m_renderManager.getvolume();//0x7fff7fff ;
	m_nRotateType = 0;

	for (int i = 0; i < MULTI_DISPLAY_RECT_NUM; i++)
	{
		m_MultiDisInfo[i].bEnable = FALSE;
		m_MultiDisInfo[i].nRegionNum = -1;
		memset(&(m_MultiDisInfo[i].pSrcRect),0,sizeof(RECT));
	}

	memset(&m_mainRect, 0, sizeof(RECT));

	m_IfRenderOpen = false ;
	m_i = 0 ;
	m_pQuarterBuf = NULL;

	m_hWnd = NULL;
}

/*	$Function		:	~CDisplay
==  ===============================================================
==	Description		:	析构函数
==	Argument		:				
==				:			
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
CDisplay::~CDisplay()
{
	if (m_pQuarterBuf)
	{
		delete m_pQuarterBuf;
		m_pQuarterBuf = NULL;
	}
}

/*	$Function		:	Init
==  ===============================================================
==	Description		:	初始化函数
==	Argument		:							
==				:			
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/

//draw_callback draw_cb ;
void __stdcall draw_cb(int index, HDC hDc)
{
	if (g_cDHPlayManage.pCallback[index]&&g_cDHPlayManage.pCallback[index]->GetCBStatus(CBT_DrawCBFun))
		g_cDHPlayManage.pCallback[index]->ProcessDraw(hDc) ;
}

void __stdcall drawEx_cb(int index, HDC hDc)
{
	int port = index / FUNC_MAX_PORT;
	int reginNum = index % FUNC_MAX_PORT;
	
	if (g_cDHPlayManage.pCallback[port]&&g_cDHPlayManage.pCallback[port]->GetCBStatus(CBT_DrawCBFunEx))
		g_cDHPlayManage.pCallback[port]->ProcessDrawEx(reginNum, hDc) ;
}

BOOL CDisplay::OpenSound()
{
	int iRet = 0 ;	
   	iRet = m_renderManager.openSound(1, 8000, 8) ;
	//SetVolume(m_volume&0xffff) ;
	return (iRet == 0) ? TRUE : FALSE ; 
}

BOOL CDisplay::CloseSound()
{
// 	m_renderManager.closeSound() ;
	return TRUE  ; 
}

int CDisplay::Init(HWND mWnd)
{
	m_DDRAWCB = NULL;
	if (g_cDHPlayManage.pCallback[m_nIndex]&&
		g_cDHPlayManage.pCallback[m_nIndex]->GetCBStatus(CBT_DrawCBFun))
	{
		m_DDRAWCB = draw_cb;
	}

	if (g_cDHPlayManage.pCallback[m_nIndex]&&
		g_cDHPlayManage.pCallback[m_nIndex]->GetCBStatus(CBT_DrawCBFunEx))
	{
		m_DDRAWCB = drawEx_cb;
	}
	
	if (ByGDI == m_DisType)
	{
		m_DisType = ByDDOffscreen;
	}

	if (m_renderManager.openVideo(m_nIndex, mWnd, 352, 288,m_DDRAWCB,(VideoRenderMethod)m_DisType,m_colorkey) != 0)    //打开回放窗口
	{
		//如果用该显示模式打开不了,那么用offscreen 再显示一次
		m_DisType = ByDDOffscreen;	
		if (m_renderManager.openVideo(m_nIndex, mWnd, 352, 288,m_DDRAWCB,(VideoRenderMethod)m_DisType,m_colorkey) != 0)
		{						
			//如果用offscreen还显示不了,那么只能用GDI显示
 			m_DisType = ByGDI;	
			if(m_renderManager.openVideo(m_nIndex, mWnd, 352, 288,m_DDRAWCB,(VideoRenderMethod)m_DisType,m_colorkey) != 0)
			{
				//如果还是不行,那么返回失败
				return 9; //DH_PLAY_CREATE_DDRAW_ERROR
			}
			else;
		}
		else;
	}
	
	m_IfRenderOpen = true ;

	m_hWnd = mWnd;

	return 0 ;
}

int CDisplay::ResetVideoDisplay(void)
{
	int ret = m_renderManager.ResetVideoRender(m_nIndex, m_hWnd, m_nWidth, m_nHeight, m_DDRAWCB,
		(VideoRenderMethod)m_DisType,m_colorkey);

	return ret;
}
/*	$Function		:	SetJumpNum
==  ===============================================================
==	Description		:	关闭回放显示，当播放关闭时也应该关闭显示，否则下次render_open会出错
==	Argument		:							
==				:				
== 	Return		:	
==	Modification	:	2006/01/26		chenmy   Create
==  ===============================================================
*/
int CDisplay::Close() 
{
	m_renderManager.close() ;

	m_IfRenderOpen = false ;
	m_CurFrameNum = -1 ;         //当前显示的桢序号
	m_PlayedFrames = 0 ;       //播放桢数
	
	for (int i = 0 ;i < MULTI_DISPLAY_RECT_NUM ; i++)
	{
		if (m_MultiDisInfo[i].nRegionNum >= 0)
		{
			m_MultiDisInfo[i].nVideoRender.close() ;
		}
	}

	return 0;
}

/*	$Function		:	ReciveYUV
==  ===============================================================
==	Description		:	视频数据处理函数，处理解码回调而得到的视频数据
==	Argument		:	输入参数：						
==				:			index 通道号
==              :           py    解码回调而得到的YUV数据的Y部分数据指针
==              :           pu                             U部分数据指针
==              :           pv                             V部分数据指针
==              :           width 由该解码回调数据所决定的图像的宽度
==              :           height                        图像的高度	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
int CDisplay::ReciveYUV(int index, unsigned char *py, unsigned char *pu, unsigned char *pv,
        int width, int height,int AddFrameNum)  //接收视频数据
{
	if (m_nIndex != index)
	{
		return 1;
	}

	if (py == NULL)
	{
		m_CurFrameNum += AddFrameNum ;
		return 0 ;
	}

	int ret = 0;
	
	m_CurFrameNum += AddFrameNum ;
	m_PlayedFrames++ ;

	if (g_cDHPlayManage.pCallback[index] && g_cDHPlayManage.pCallback[index]->GetCBStatus(CBT_DisplayCBFun))   //是否抓图回调
	{
		g_cDHPlayManage.pCallback[index]->ProcessDisplay(py,m_nWidth,m_nHeight);//将抓图回调函数的状态置为"PROCESS",在用户回调线程将马上执行该回调函数
	}
	
	if (g_cDHPlayManage.pCallback[index] && g_cDHPlayManage.pCallback[index]->GetCBStatus(CBT_DecCBFun)
		&& (m_decType == DEC_VIDEO || m_decType == DEC_COMPLEX) )   //是否解码显示回调
	{
		g_cDHPlayManage.pCallback[index]->ProcessDec(DEC_VIDEO,py,m_nWidth*m_nHeight*3/2,m_nWidth,m_nHeight);
		return 0 ;
	}
	else
	{
		if (m_IfRenderOpen == false)
		{
			return 1;
		}

		for (int i = 0 ; i < MULTI_DISPLAY_RECT_NUM ; i++)
		{
			if (m_MultiDisInfo[i].bEnable)
			{
				ShowRegion(m_MultiDisInfo[i].nRegionNum, py);
			}
 		}

		if (m_nDisplayType == DISPLAY_QUARTER)
		{
			if (m_pQuarterBuf == NULL)
			{
				m_pQuarterBuf = new unsigned char[m_nWidth*m_nHeight*3/2];
			}

			memcpy(m_pQuarterBuf, py, m_nWidth*m_nHeight*3/2);
			BYTE *mYp,*mUp, *mVp;
			mYp = m_pQuarterBuf;
			mUp = m_pQuarterBuf + m_nWidth*m_nHeight;
			mVp = m_pQuarterBuf + m_nWidth*m_nHeight*5/4;
		 	ToQuarter(mYp,mUp,mVp,width,height);

			ret = m_renderManager.renderVideo(mYp, mUp, mVp, m_nWidth/2, m_nHeight/2 ,m_nRotateType) ;//显示一桢图像
		}
		else
		{
			int rate = g_cDHPlayManage.pDHPlay[index]->GetFrameRate();
			if (rate > 25)
			{
				if (m_i % (rate/25) == 0)
				{
					if (m_mainRect.right == 0 || m_mainRect.bottom == 0)
					{
						ret = m_renderManager.renderVideo(py, pu, pv, m_nWidth, m_nHeight, m_nRotateType) ;//显示一桢图像
					}
					else
					{
						ret = m_renderManager.renderVideo(py, pu, pv, m_nWidth, m_nHeight, m_nRotateType ,&m_mainRect) ;//显示一桢图像
					}					
				}			

				m_i++ ;

				if (m_i > 10000)
				{
					m_i = 0 ;//防止I过大
				}
			}
			else
			{
				if (m_mainRect.right == 0 || m_mainRect.bottom == 0)
				{
					ret = m_renderManager.renderVideo(py, pu, pv, m_nWidth, m_nHeight , m_nRotateType) ;//显示一桢图像
				}
				else
				{
					ret = m_renderManager.renderVideo(py, pu, pv, m_nWidth, m_nHeight, m_nRotateType , &m_mainRect) ;//显示一桢图像
				}
			}
		}
	}

	return ret;
}

/*	$Function		:	RecivePCM
==  ===============================================================
==	Description		:	音频数据处理函数，处理解码回调得到的音频数据
==	Argument		:	输入参数：						
==				:			index 通道号
==              :           pcm   音频数据
==              :           len   音频数据长度	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
int CDisplay::RecivePCM(int index, unsigned char *pcm, int len,int bitsPerSample,int samplesPerSecond)
{	
	if (m_nIndex != index)
	{
		return 1;
	}

	if (g_cDHPlayManage.pCallback[index] && g_cDHPlayManage.pCallback[index]->GetCBStatus(CBT_AudioCBFun))   //是否音频回调
	{
		g_cDHPlayManage.pCallback[index]->ProcessAudio(pcm ,len ,(bitsPerSample == 8)?T_AUDIO8:T_AUDIO16 , bitsPerSample);//将音频回调函数的状态置为"PROCESS",在用户回调线程将马上执行该回调函数
	}
	
	if (g_cDHPlayManage.pCallback[index] && g_cDHPlayManage.pCallback[index]->GetCBStatus(CBT_DecCBFun)
		&& (m_decType == DEC_AUDIO || m_decType == DEC_COMPLEX))   //是否音频回调
	{
		g_cDHPlayManage.pCallback[index]->ProcessDec(DEC_AUDIO,pcm,len,bitsPerSample,samplesPerSecond);//将音频回调函数的状态置为"PROCESS",在用户回调线程将马上执行该回调函数
	}
	else
	{
		if (m_nCoefficient != 0)
		{
			AdjustWaveAudio(pcm,len,bitsPerSample,m_nCoefficient) ;
		}

	 	int ret = m_renderManager.renderAudio(pcm, len,bitsPerSample,samplesPerSecond);   //播放音频	
		if (ret < 0)
		{
			Sleep(100) ;
			m_renderManager.renderAudio(pcm, len,bitsPerSample,samplesPerSecond);  
		}
	}
	
	return 0;
}

/*	$Function		:	Show
==  ===============================================================
==	Description		:	图片显示函数，显示最新一桢图片
==	Argument		:						
==              :           
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
int CDisplay::Show()    
{
	BYTE *mYp,*mUp, *mVp;
	
	if(g_cDHPlayManage.pCallback[m_nIndex] && g_cDHPlayManage.pCallback[m_nIndex]->GetCBStatus(CBT_DecCBFun)
		&& (m_decType == DEC_VIDEO || m_decType == DEC_COMPLEX) )   //是否解码显示回调
	{
		return 1 ;
	}

	if (!m_nWidth || !m_nHeight || m_IfRenderOpen == false)
	{
		return 1;
	}

	unsigned char* pDataBuf =  g_cDHPlayManage.pDHPlay[m_nIndex]->GetLastFrame();

	if (m_nDisplayType == DISPLAY_QUARTER)		
	{
		if (m_pQuarterBuf == NULL)
		{
			m_pQuarterBuf = new unsigned char[m_nWidth*m_nHeight*3/2];
		}

		mYp = m_pQuarterBuf;
		mUp = m_pQuarterBuf + m_nWidth * m_nHeight;
		mVp = m_pQuarterBuf + m_nWidth * m_nHeight * 5 / 4;

		ToQuarter(mYp,mUp, mVp,m_nWidth,m_nHeight) ;
		__try
		{
			m_renderManager.renderVideo(mYp, mUp, mVp, m_nWidth/2, m_nHeight/2 , m_nRotateType) ;//显示一桢图像
		}
		__except(0,1)
		{
			int wlj = 0;
		}

	}
	else
	{
		mYp = pDataBuf;
		mUp = pDataBuf + m_nWidth * m_nHeight;
		mVp = pDataBuf + m_nWidth * m_nHeight * 5 / 4;
		__try
		{
			m_renderManager.renderVideo(mYp, mUp, mVp, m_nWidth, m_nHeight , m_nRotateType);//显示最新的一桢数据
		}
		__except(0,1)
		{
			int wlj = 0;
		}
	}

	return 0;
}

/*	$Function		:	ShowRegion
==  ===============================================================
==	Description		:	图片显示函数，显示最新一桢图片
==	Argument		:	输入参数：						
==				:		   nRegionNum   区域号					
==              :           
== 	Return		:	
==	Modification	:	2006/4/28		zhougf   Create
==  ===============================================================
*/
int CDisplay::ShowRegion(int nRegionNum, unsigned char* pBuf)
{
	if (nRegionNum < 0 || nRegionNum >= MULTI_DISPLAY_RECT_NUM)
	{
		return 1 ;
	}

	__try
	{
		if (m_MultiDisInfo[nRegionNum].pSrcRect.left  == 0 &&
			m_MultiDisInfo[nRegionNum].pSrcRect.right == 0 &&
			m_MultiDisInfo[nRegionNum].pSrcRect.top   == 0 &&
			m_MultiDisInfo[nRegionNum].pSrcRect.bottom== 0 )
		{
			m_MultiDisInfo[nRegionNum].nVideoRender.renderVideo(pBuf, pBuf+m_nWidth*m_nHeight, 
				pBuf+m_nWidth*m_nHeight*5/4, m_nWidth, m_nHeight, m_nRotateType) ;
		}
		else
		{
			m_MultiDisInfo[nRegionNum].nVideoRender.renderVideo(pBuf, pBuf+m_nWidth*m_nHeight, 
				pBuf+m_nWidth*m_nHeight*5/4, m_nWidth, m_nHeight, m_nRotateType,
				&(m_MultiDisInfo[nRegionNum].pSrcRect)) ;
		}
// 		m_MultiDisInfo[nRegionNum].nVideoRender.renderVideo(pBuf, pBuf+m_nWidth*m_nHeight, 
// 			pBuf+m_nWidth*m_nHeight*5/4, m_nWidth, m_nHeight, m_nRotateType ,
// 			&(m_MultiDisInfo[nRegionNum].pSrcRect)) ;
	}
	__except(0,1)
	{
		int wlj = 0;
	}

	return 0 ;
}

BOOL CDisplay::SetRotateAngle(int rotateType)
{
	m_nRotateType = rotateType;
	return TRUE;
}

/*	$Function		:	GetPicHeight
==  ===============================================================
==	Description		:	获得图片属性函数，得到图片高度
==	Argument		:						
==              :           
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
int CDisplay::GetPicHeight() 
{
	return m_nHeight ;
}


/*	$Function		:	GetPicWidth
==  ===============================================================
==	Description		:	获得图片属性函数，得到图片宽度
==	Argument		:						
==              :           
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
int CDisplay::GetPicWidth()
{
	return m_nWidth ;
}

/*	$Function		:	GetCurFrameNum
==  ===============================================================
==	Description		:	得到当前显示画面的桢序号
==	Argument		:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
int CDisplay::GetCurFrameNum()
{
	return (m_CurFrameNum < 0)? 0 : m_CurFrameNum;
}

/*	$Function		:	GetPlayedFrames
==  ===============================================================
==	Description		:	获得播放桢数
==	Argument		:	
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
int CDisplay::GetPlayedFrames()
{
	return m_PlayedFrames ;
}

/*	$Function		:	SetCurFrameNum
==  ===============================================================
==	Description		:	更新当前显示画面的桢序号
==	Argument		:	输入参数：
							FrameNum 桢序号 			
== 	Return		:	
==	Modification	:	2005/11/28		chenmy   Create
==  ===============================================================
*/
void CDisplay::SetCurFrameNum(int FrameNum)
{
	if (FrameNum < 0 || FrameNum >= g_cDHPlayManage.pDHFile[m_nIndex]->GetTotalFrames())
	{
		return ;
	}
	m_CurFrameNum = FrameNum ;
}


/*	$Function		:	SetOverlayMode
==  ===============================================================
==	Description		:	设置是否以overlay显示，及设置关键色
==	Argument		:	输入参数：						
==				:			boverlay   是否以overlay显示
==              :           nColorKey  关键色	
== 	Return		:	
==	Modification	:	2006/4/28		zhougf   Create
==  ===============================================================
*/
BOOL CDisplay::SetOverlayMode(BOOL boverlay, DWORD nColorKey)
{
    if (boverlay)
	{
		m_DisType = 1 ;
		m_colorkey = nColorKey ;
	}
	else
	{
		m_DisType = 0 ;
	}

	return true ;
}
/*	$Function		:	GetOverlayMode
==  ===============================================================
==	Description		:	获得overlay模式
==	Argument		:		
== 	Return		:	
==	Modification	:	2006/4/28		zhougf   Create
==  ===============================================================
*/
DWORD CDisplay::GetOverlayMode()
{
	return (m_DisType == ByDDOverlay)? 1 : 0;
}
/*	$Function		:	GetColorKey
==  ===============================================================
==	Description		:	得到关键色
==	Argument		:		
== 	Return		:	
==	Modification	:	2006/4/28		zhougf   Create
==  ===============================================================
*/
DWORD CDisplay::GetColorKey()
{
	return m_colorkey ;
}

/*	$Function		:	MulitiDisplay
==  ===============================================================
==	Description		:	设置多区域显示
==	Argument		:	输入参数
						  nRegionNum   区域号
						  hDestWnd     句柄
						  pSrcRect     所指向的区域大小
						  bEnable      是否显示
== 	Return		:	
==	Modification	:	2006/4/28		zhougf   Create
==  ===============================================================
*/
BOOL CDisplay::MulitiDisplay(int nRegionNum,HWND hDestWnd,RECT* pSrcRect,BOOL bEnable)
{	
	if (nRegionNum < 0 || nRegionNum >= MULTI_DISPLAY_RECT_NUM)
	{
		return FALSE ;
	}

	if (nRegionNum == 0)
	{
		if (pSrcRect == NULL || bEnable == FALSE)
		{
			m_mainRect.bottom = m_mainRect.right = m_mainRect.left = m_mainRect.top = 0;
		}
		else
		{
			m_mainRect.left = pSrcRect->left;
			m_mainRect.right = pSrcRect->right;
			m_mainRect.top = pSrcRect->top;
			m_mainRect.bottom = pSrcRect->bottom;
		}

		return TRUE;
	}

	if (bEnable == TRUE)
	{
// 		if (pSrcRect == NULL)
// 		{
// 			return FALSE ;
// 		}

		if (pSrcRect != NULL)
		{
			if (pSrcRect->right <= pSrcRect->left || pSrcRect->bottom <= pSrcRect->top 
				|| pSrcRect->bottom > m_nHeight 
				|| pSrcRect->right  > m_nWidth 
				|| pSrcRect->left   < 0
				|| pSrcRect->top    < 0)
			{
				return FALSE ;
			}
		}


		//如果该区域已经存在，则只改变所指向的区域大小及位置（pSrcRect）
		if (m_MultiDisInfo[nRegionNum].nRegionNum >= 0)//
		{
			m_MultiDisInfo[nRegionNum].pSrcRect.left   = (pSrcRect == NULL)?0:pSrcRect->left ;
			m_MultiDisInfo[nRegionNum].pSrcRect.right  = (pSrcRect == NULL)?m_nWidth:pSrcRect->right ;
			m_MultiDisInfo[nRegionNum].pSrcRect.bottom = (pSrcRect == NULL)?m_nHeight:pSrcRect->bottom ;
			m_MultiDisInfo[nRegionNum].pSrcRect.top    = (pSrcRect == NULL)?0:pSrcRect->top ;
			m_MultiDisInfo[nRegionNum].bEnable  = TRUE ;
			return TRUE ;
		}
		else//为该区域打开一路回放
		{	
			void (__stdcall *DDRAWCB)(int index, HDC hDc) = NULL;
			if ((g_cDHPlayManage.pCallback[m_nIndex]&&
				g_cDHPlayManage.pCallback[m_nIndex]->GetCBStatus(CBT_DrawCBFunEx)))
			{
				DDRAWCB = drawEx_cb;
			}

			int iRet = m_MultiDisInfo[nRegionNum].nVideoRender.openVideo(m_nIndex*FUNC_MAX_PORT+nRegionNum, hDestWnd, 
				352, 288, DDRAWCB, ByDDOffscreen, 0);
			if (iRet != 0)
			{
				int iRet = m_MultiDisInfo[nRegionNum].nVideoRender.openVideo(m_nIndex*FUNC_MAX_PORT+nRegionNum, hDestWnd, 
				352, 288, DDRAWCB, ByGDI, 0);

				if (iRet != 0) return FALSE;
			}
			
// 			m_MultiDisInfo[nRegionNum].nVideoRender.openVideo(m_nIndex*FUNC_MAX_PORT+nRegionNum, hDestWnd,352,288,
// 				drawEx_cb,ByDDOffscreen,0) ;

// 			m_MultiDisInfo[nRegionNum].nVideoRender.openVideo(-1, hDestWnd,352,288,
// 				NULL,ByDDOffscreen,0) ;
		
			m_MultiDisInfo[nRegionNum].pSrcRect.left   = (pSrcRect == NULL)?0:pSrcRect->left ;
			m_MultiDisInfo[nRegionNum].pSrcRect.right  =  (pSrcRect == NULL)?0:pSrcRect->right ;
			m_MultiDisInfo[nRegionNum].pSrcRect.bottom =  (pSrcRect == NULL)?0:pSrcRect->bottom ;
			m_MultiDisInfo[nRegionNum].pSrcRect.top    =  (pSrcRect == NULL)?0:pSrcRect->top ;
			m_MultiDisInfo[nRegionNum].bEnable         = TRUE ;
			m_MultiDisInfo[nRegionNum].nRegionNum      = nRegionNum ;

		}//end of else

	}//end of if (bEnable == TRUE)
	
	else
	{	
		m_MultiDisInfo[nRegionNum].nVideoRender.close() ;
		m_MultiDisInfo[nRegionNum].bEnable = FALSE;
		m_MultiDisInfo[nRegionNum].nRegionNum = -1;
		memset(&(m_MultiDisInfo[nRegionNum].pSrcRect),0,sizeof(RECT));
	}

	return TRUE ;
}

/*	$Function		:	SetDecCBType
==  ===============================================================
==	Description		:	设置解码回调类型
==	Argument		:	输入参数
						  dectype  解码回调类型
== 	Return		:	
==	Modification	:	2006/4/28		zhougf   Create
==  ===============================================================
*/
BOOL CDisplay::SetDecCBType(DWORD dectype)
{
	m_decType = (DEC_TYPE)dectype ;
	return TRUE ;
}

int CDisplay::GetDecType()
{
	return (int)m_decType;
}

void CDisplay::SetWidthHeight(int width, int height)
{
	m_nWidth = width;
	m_nHeight = height;
}

BOOL CDisplay::SetDisplayType(int displaytype)
{
	if (displaytype != DISPLAY_NORMAL && displaytype != DISPLAY_QUARTER)
	{
		return FALSE;
	}
	else
	{
		m_nDisplayType = displaytype ;
	}
	
	return TRUE ;
}

int CDisplay::GetDisplayType()
{
	return m_nDisplayType ;
}

void CDisplay::ToQuarter(BYTE* py,BYTE* pu,BYTE* pv, int width,int height) 
{
	int i, j, m, n ;
	
	m = n = 0 ;
	
	for (i = 0 ; i < (height>>1) ; ++i)
	{
		for (j = 0 ; j < (width>>1) ; ++j)
		{
			*(py + m) = *(py + m + n) ;
			++m ;
			++n ;
		}
		n += width ;
	}

	m = n = 0;
	
	for (i = 0 ; i < (height>>2) ; ++i) 
	{
		for (j = 0; j < (width>>2) ; ++j) 
		{
			*(pu + m) = *(pu + m + n) ;
			++m ;
			++n ;
		}
						
    	n += (width>>1);
	}

	m = n = 0;
	
	for (i = 0 ; i < (height>>2) ; ++i) 
	{
		for (j = 0; j < (width>>2) ; ++j) 
		{
			*(pv + m) = *(pv + m + n) ;
			++m ;
			++n ;
		}
						
    	n += (width>>1);
	}

}

BOOL CDisplay::AdjustWaveAudio(BYTE* pcm, unsigned int datalen, int bitsPerSample,long nCoefficient)
{
	if (bitsPerSample == 8)		//位率8
	{				
		for(int i = 0 ; i < datalen; i++)
		{
			int x = (BYTE)pcm[i];	
			int y = (int)((float)x*(nCoefficient+100)/100+0.5);
													
			if (y > 255)
			{
				y = 255;
			}
			else;
			pcm[i] = y;
					
		} 	
	}

	else if (bitsPerSample == 16)	//位率16
	{
		for (int i = 0; i < datalen/2; i++)
		{
			unsigned short *x = (unsigned short*)(pcm+2*i);
			unsigned long y = (unsigned long)((float)*x *(nCoefficient+100)/100+0.5);
			if (y >= (1<<16))
			{
				y = 65535;
			}
			else;

			pcm[2*i] = y%256;
			pcm[2*i+1] = y/256;							
		}				
	}

	else
	{
		return FALSE ;
	}

	return TRUE ;
}

BOOL CDisplay::SetCoefficient(int nCoefficient)
{
	m_nCoefficient = nCoefficient ;
	return TRUE ;
}

BOOL CDisplay::SetVolume(WORD volume)
{
	if (m_IfRenderOpen == false)
	{
		return FALSE ;
	}

	m_volume = (volume << 16) | volume;

	m_renderManager.setvolume(volume) ;
	
	return TRUE ;
}

WORD  CDisplay::GetVolume() 
{
	return m_volume & 0xffff ;
}

BOOL CDisplay::VerticalSyncEnable(BOOL bEnable)
{
	return m_renderManager.VideoVerticalSyncEnable(bEnable);
}