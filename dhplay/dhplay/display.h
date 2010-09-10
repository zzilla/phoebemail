 //DISPLAY_H

/********************************************************************
	created:	2005/11/28
	created:	28:11:2005   17:00
	filename: 	display.h
	file base:	display
	file ext:	h
	author:		
	
	purpose:	
*********************************************************************/
//对于反向回放的处理，增加一个缓冲做成双缓冲处理显示数据，与解码同步，缓解解码和显示的等待；
//另外缓冲采取动态分配方式，防止同一应用程序多路回放其他正常播放通道的资源浪费 205-12-23
//将所有回放显示相关的处理都移到此类中处理 2006-1-25 chenmy
#ifndef DISPLAY_H
#define DISPLAY_H
#include <vector>
#include "render/manager.h"

#define  MULTI_DISPLAY_RECT_NUM 16

//解码类型
typedef enum
{
	DEC_NO = 0 ,
	DEC_VIDEO ,
	DEC_AUDIO ,
	DEC_COMPLEX,
}DEC_TYPE ;

//多区域显示相关信息
typedef struct _MULTI_DIS_INFO
{
	int nRegionNum ;
	RECT pSrcRect ;
	BOOL bEnable ;
 	DhRenderManager nVideoRender  ;
}MULTI_DIS_INFO ;

class CDisplay
{	
	MULTI_DIS_INFO m_MultiDisInfo[MULTI_DISPLAY_RECT_NUM];
	unsigned char* m_pQuarterBuf;
	DWORD m_volume ;
	int  m_nIndex;    //端口号
	HWND m_hWnd;
	int  m_nWidth;      //每帧图片的宽度
	int  m_nHeight;     //每帧图片的高度
	int  m_CurFrameNum ;          //当前显示画面的桢序号
	int  m_PlayedFrames ;         //播放桢数
	DWORD m_DisType ;
	DWORD m_colorkey ;
	DEC_TYPE m_decType ;
	int   m_nDisplayType ;//显示类型，正常分辨率与四分之一分辨率,20060818 zgf
	long  m_nCoefficient ;//波形调整参数
	int m_i ;
	bool m_IfRenderOpen ;
	DhRenderManager m_renderManager ;
	RECT m_mainRect;
	int  m_nRotateType;

	void (__stdcall *m_DDRAWCB)(int index, HDC hDc);
//主要成员函数：
public:
	
	CDisplay(int nPort = 0);
	~CDisplay();
	
	int   Init(HWND mWnd);         //初始化函数      
	int   Close();         //关闭回放显示     
	int   ResetVideoDisplay(void);
	int   ReciveYUV(int index, unsigned char *py, unsigned char *pu, unsigned char *pv,
            int width, int height,int AddFrameNum);  //接收视频数据
	int   RecivePCM(int index, unsigned char *pcm, int len,int bitsPerSample,int samplesPerSecond); //接受音频数据
	int   Show();      //再次显示数据
	int   ShowRegion(int RegionNum, unsigned char* pBuf) ;
	int   GetPicWidth()  ;//获得图片宽度
	int   GetPicHeight() ;//获得图片高度
	void  SetWidthHeight(int width,int height);
		
	int   GetCurFrameNum();                  //获得当前显示画面的桢序号
	int   GetPlayedFrames() ;                //获得播放桢数
	void  SetCurFrameNum(int FrameNum) ;    //设置当前显示画面的桢序号

	BOOL  SetOverlayMode(BOOL boverlay,DWORD nColorKey) ;//设置是否以overlay形式播放及关键色
	DWORD GetColorKey() ;                 //得到关键色
	DWORD GetOverlayMode() ;              //返回是否以overlay形式播放
	BOOL  MulitiDisplay(int nRegionNum,HWND hDestWnd,RECT* pSrcRect,BOOL bEnable) ;//多区域显示

	BOOL  SetDecCBType(DWORD dectype) ;//设置解码回调类型
	int   GetDecType();
	//////////////////////////////////////////add by zgf 2006_0819
	BOOL  SetDisplayType(int displaytype) ; //设置是正常分辨率还是四分之一分辨率
	int   GetDisplayType() ;
	void  ToQuarter(BYTE* py,BYTE* pu,BYTE* pv,int width,int height) ;//把图像数据转成四分之一大小
	BOOL  AdjustWaveAudio(BYTE* pcm, unsigned int datalen, int bitsPerSample,long nCoefficient) ;//调整波形
	BOOL  SetCoefficient(int nCoefficient) ;
    WORD  GetVolume() ;
	BOOL  SetVolume(WORD volume) ;
	BOOL  OpenSound() ;
	BOOL  CloseSound() ;
	BOOL  SetRotateAngle(int rotateType); 
	BOOL  VerticalSyncEnable(BOOL bEnable);
};

#endif 
