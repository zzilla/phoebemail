/********************************************************************
	created:	2006/01/10
	created:	10:1:2006   9:15
	filename: 	callback.h
	file base:	callback
	file ext:	h
	author:		chenmy	
	purpose:	
*********************************************************************/
#ifndef CALLBACK_H
#define CALLBACK_H

#ifdef DHVECPLAY
	#include "dhvecplay.h"
#else
	#include "dhplay.h"
#endif

#include "display.h"

//注册的回调类型
typedef enum
{
	CBT_DecCBFun = 0,              //解码显示回调
	CBT_DisplayCBFun ,             //显示抓图回调
	CBT_DrawCBFun ,                //注册画图板回调
	CBT_FILEREF,                   //建立文件索引成功，则回调
	CBT_FILEREFEX,
	CBT_SourceBuf ,                //数据源缓冲小于阀值回调
	CBT_AudioCBFun ,               //音频回调
	CBT_VerifyCBFun,               //水印验证回调
	CBT_EncChangeCBFun,            //编码格式改变回调
	CBT_FileEnd,                        //文件播放结束回调
	CBT_DemuxCBFun,
	CBT_DrawCBFunEx,
	CBT_VisibleDec,
	CBT_TOTAL ,
}CALLBACK_TYPE;

//解码回调的参数回调函数,added by wlj 2009.1.19
typedef struct 
{
	long nUser ;
}PARAM_DEC;

typedef void (__stdcall *Callback_Dec)(long nPort,char * pBuf,long nSize,
        FRAME_INFO * pFrameInfo, long nReserved1,long nReserved2) ; //获取解码数据，由用户自行处理

typedef struct 
{
	long nUser ;
}PARAM_VISIBLEDEC;

//视频回调的参数和回调函数
typedef struct 
{
	long nUser ;
}PARAM_DISPLAY;
typedef void (__stdcall *Callback_Display)(long nPort,char * pBuf,long nSize,
		long nWidth,long nHeight,long nStamp,long nType,long nReserved);//抓图回调函数

//GDI回调的参数和回调函数
typedef struct 
{
	LONG nUser;
}PARAM_DRAW;
typedef void (__stdcall *Callback_Draw)(long nPort,HDC hDc,LONG nUser);

//多区域GDI回调的参数和回调函数
typedef struct 
{
	LONG nUser;
}PARAM_DRAWEX;
typedef void (__stdcall *Callback_DrawEx)(long nPort,long nReginNum, HDC hDc,LONG nUser);



typedef struct 
{
	LONG nUser ;
}PARAM_FILEREF;
typedef void (__stdcall *Callback_FileRef)(DWORD nPort,DWORD nUser) ;

typedef void (__stdcall *Callback_FileRefEx)(DWORD nPort, BOOL bIndexCreated, DWORD nUser) ;

//数据源小于阀值回调的参数和回调函数
typedef struct 
{
	DWORD nThreShold;
	LONG nUser;
}PARAM_SOURCEBUF;
typedef void (__stdcall *Callback_SourceBuf)(long nPort,DWORD nBufSize,DWORD dwUser,void*pResvered);


//音频回调的参数和回调函数
typedef struct 
{
	long nUser ;
}PARAM_AUDIO;
typedef void (__stdcall * Callback_Audio)(long nPort, char * pAudioBuf,
								 long nSize, long nStamp, long nType, long nUser);

//编码改变时回调参数和回调函数
typedef struct {
	long nUser;
}PARAM_ENCCHANGE;
typedef void (__stdcall *Callback_EncChange)(long nPort,long nUser);  //编码改变时回调函数

//水印验证回调参数和回调函数
typedef struct {
	DWORD nBeginTime;
	DWORD nEndTime;
	FRAME_POS nFilePos;
	DWORD bIsVideo;
	long nUser;
}PARAM_VERIFY;
typedef void (__stdcall *Callback_Verify)(long nPort, FRAME_POS * pFilePos, DWORD bIsVideo, DWORD nUser); 

typedef struct 
{
	LONG nUser ;
}PARAM_FILEEND;
typedef void (__stdcall *Callback_FileEnd)(DWORD nPort,DWORD nUser) ;

typedef struct 
{
	LONG nUser ;
}PARAM_DEMUX;
//数据回调的回调函数
typedef void (__stdcall *Callback_Demux)(long nPort,char * pBuf,long nSize,
        void * pParam, long nReserved,long nUser) ; //获取解码数据，由用户自行处理

//用户回调信息，包括各回调函数的主要数据参数
class CCallback
{
	LONG m_nIndex;						//播放索引（播放端口号）
	BOOL m_cbState[CBT_TOTAL];          //当前各种回调函数的状态
	BOOL m_cbStreamState;	//流方式下，回调过后置为0，如果源缓冲区剩余数据小于等于阀值时回调

	PARAM_DEC mParam_decCB;  //added by wlj 2009.01.19
	Callback_Dec cb_Dec;                  //函数指针
	Callback_Dec temp_Dec ;

	PARAM_VISIBLEDEC mParam_visibleDecCB; 
	Callback_Dec cb_VisibleDec; 
	Callback_Dec temp_VisibleDec;

	PARAM_DISPLAY mParam_display;
	Callback_Display cb_Display;
	Callback_Display temp_Display ;

	PARAM_DRAW mParam_draw;
//	Callback_Draw cb_Draw;

	PARAM_FILEREF mParam_fileref ;
	Callback_FileRef cb_FileRef ;
	Callback_FileRef temp_FileRef ;

	Callback_FileRefEx cb_FileRefEx;
	Callback_FileRefEx temp_FileRefEx;
	
	PARAM_SOURCEBUF mParam_sourcebuf;
	Callback_SourceBuf cb_Sourcebuf;
	Callback_SourceBuf temp_SourceBuf ;

	PARAM_AUDIO mParam_audio;
	Callback_Audio cb_Audio;
	Callback_Audio temp_Audio ;

	PARAM_VERIFY mParam_verify;
	Callback_Verify cb_Verify;
	Callback_Verify temp_Verify ;

	PARAM_ENCCHANGE mParam_encchange;        
	Callback_EncChange cb_EncChange;
	Callback_EncChange temp_EncChange ;

	PARAM_FILEEND  mParam_fileend;    
	Callback_FileEnd    cb_FileEnd ;
	Callback_FileEnd    temp_FileEnd ;

	PARAM_DEMUX  mParam_demux;
	Callback_Demux    cb_demux;
	Callback_Demux    temp_demux;
public:

	CCallback(LONG nPort);
	~CCallback();
	Callback_Draw cb_Draw;
	Callback_Draw temp_Draw ;

	PARAM_DRAWEX mParam_drawEx;
	Callback_DrawEx cb_DrawEx;
	Callback_DrawEx temp_DrawEx ;

		//设置回调
	BOOL SetDecCallBack(Callback_Dec pcbDec,BOOL IfSet = TRUE);
	BOOL SetDecCallBackEx(Callback_Dec pcbDec,long nUser,BOOL IfSet = TRUE);
	BOOL SetDisplayCallBack(Callback_Display pcbDisplay, long nUser);
	BOOL SetDrawCallback(Callback_Draw pcbDraw, long nUser);
	BOOL SetDrawExCallback(Callback_DrawEx pcbDrawEx, long nReginNum, long nUser);
	BOOL SetSourceBufCallBack(Callback_SourceBuf pcbSourceBuf, DWORD nThreShold, long dwUser, void *pReserved);
	BOOL ResetSourceBufFlag();
	BOOL SetAudioCallBack(Callback_Audio pcbAudio, long nUser);
	BOOL SetVerifyCallBack(Callback_Verify pcbfunVerify, DWORD nBeginTime, DWORD nEndTime, long nUser);
	BOOL SetEncChangeCallBack(Callback_EncChange pcbEncChange, long nUser);
	BOOL SetFileRefCallBack(Callback_FileRef pcbFileRef,DWORD nUser) ;
	BOOL SetFileEndCallBack(Callback_FileEnd pcbFileEnd, DWORD nUser) ;
	BOOL SetDemuxCallBack(Callback_Demux pcbDemux, long nUser) ;
	BOOL SetVisibleDecCallBack(Callback_Dec pcbDec,long nUser,BOOL IfSet = TRUE);
	BOOL SetFileRefCallBackEx(Callback_FileRefEx pcbFileRef, DWORD nUser);
public:
		//根据参数调用回调函数
	BOOL ProcessDec(DEC_TYPE type,BYTE* pBuf,int nSize,int width,int height); // 1视频 2 音频
	BOOL ProcessVisibleDec(DEC_TYPE type,BYTE* pBuf,int nSize,int width,int height);
	BOOL ProcessDisplay(BYTE* pBuf,int width,int height);	
	BOOL ProcessAudio(BYTE * pAudioBuf, long nSize, long nStamp, long nType);
	BOOL ProcessDemux(char * pBuf,long nSize, void* pParam);
	BOOL ProcessSourceBuf();
	BOOL ProcessVerify();
	BOOL ProcessEncChange();
	BOOL ProcessDraw(HDC hDC) ;
	BOOL ProcessDrawEx(long nReginNum, HDC hDC) ;
	BOOL ProcessFileRef() ;
	BOOL ProcessFileRefEx(BOOL bIndexCreated) ;
	BOOL ProcessFileEnd() ;

public:
	BOOL GetCBStatus(CALLBACK_TYPE nStatus);        //获取某类型回调的状态
	
};

#endif
