 
/********************************************************************
	created:	2005/11/28
	created:	28:11:2005   17:03
	filename: 	play.h
	file base:	play
	file ext:	h
	author:		
	
	purpose:	
*********************************************************************/
#ifndef PLAY_H
#define PLAY_H

#include "data.h"
#include "display.h"

#include "decode\graphfactory.h"
//播放状态
typedef enum
{
	PS_INVALID = -1,    //无效
	PS_STOP = 0,        //停止
	PS_PLAY,            //正常播放
	PS_BACK,            //反向回放
	PS_ONEFRAME,
	PS_TOTAL,
}PLAY_STATUS;

//反向回放的子状态
typedef enum 
{
	BF_START = 0,   //反向回放开始，移动文件指针到I桢头
	BF_DECODE,      //解码，取视频数据送解码
	BF_DISPLAY,     //显示，等待显示
}BACK_FLAG;

//声音标志
typedef enum{
	SF_STOP = 0,
	SF_PLAY,
	SF_SHAREPLAY,
	SF_SHARESTOP,
}SOUND_FLAG;

//视频参数信息
typedef struct _COLOR{
	int nBrightness ;
	int nContrast ;
	int nSaturation ;
	int nHue ;
}COLOR_INFO ;

//速度值
//const int SPEED[9] = {1,3,6,12,25,40,60,80,100} ;
const int SPEED[10] = {1,3,6,12,25,50,100,150,200,400} ;

//流类型
typedef enum
{
	STREAM_VIDEO = 1,
	STREAM_AUDIO,
	STREAM_COMPLEX
}STREAM_TYPE ;


//速度状态
typedef enum _SPEED
{
	PS_SLOW4=0,
	PS_SLOW3,
	PS_SLOW2,
	PS_SLOW1,
	PS_NORMAL,
	PS_FAST1,
	PS_FAST2,
	PS_FAST3,
	PS_FAST4,
}PS_SPEED;

class CDHPlay
{		
	PLAY_STATUS		m_nStatus;     //播放状态（-1 无效 、0 停止、 1 暂停 、2正常播放、 3 快1 、4 快2、 。。。。）	
	int				m_ImgBufCount ;          //缓冲区大小（图片数）	
	DecodeOption m_sDecSet;    //当前解码设置，随解码库更改 
	int m_bFlag ;              //缓冲数据有效性，取得文件数据后置成取得数据长度，送解码成功后置0
	BYTE *m_mBuffer;    //送解码数据缓冲
	CDHAVData* m_pDHFile ;	//数据对象指针	
	DhPlayGraph* m_PlayGragh ;
	COLOR_INFO m_ColorInfo ;   //视频参数		
	//用户定位操作
	long m_FrameNumSeted ;     //用户设置的帧数
	long m_tepFrameSeted ;
	float m_fRelativePos ;     //用户设置的播放位置
	__int64 m_userfilepos ;
	HANDLE	m_hThread ;   //送数据线程
	
	BOOL m_IsSoundPlay ;//是否播放声音
	BOOL m_IsPause ;           //是否暂停
	BOOL m_IsOneFrame ;     //是否单帧倒放
	BOOL m_IsFileEnd ;
	int m_ResetStreamFlag ;//流模式重置
//	HANDLE m_RecordFile ;//录像文件 test
	DWORD m_CurPlayedTime;
public:	
	HANDLE m_RecordFile ;//录像文件
	char   m_cDiskNum[4]; //录像文件所在盘符
	BOOL   m_ifRecord;//test
	BOOL   m_bPosAdjust4Index;
	
	//这些变量由于要在解码回调函数dec_cb里调用，所以设置为public类型，以方便调用
	PS_SPEED m_nSpeed ;        //速度状态，从慢四到快四
	int    m_iDesiredRate;
	CDisplay *m_pDisplay ;      //显示对象指针
	DWORD m_nTimerType;
	STREAM_TYPE m_StreamType ; //流输入方式	

	LONG m_nIndex;             //播放索引（播放端口号）
	CRITICAL_SECTION  m_PlayCritSec ;//用于同步送数据线程和停止操作

	CRITICAL_SECTION  m_SpeedCritSec;

	CDHPlay(LONG nPort);
	~CDHPlay();

//主要成员函数有：
	int PlayProc();             //播放线程调用处理函数	
	//供外部调用
	BOOL Start(HWND hWnd);          //开始播放
	BOOL Stop();                    //停止播放
	BOOL Pause(DWORD nPause);       //暂停/恢复
	BOOL PlayFast();                //快放
	BOOL PlaySlow();                //慢放
	BOOL PlayBack();                //反放,暂时没有实现
	BOOL PlayBackOne();             //反向单桢
	BOOL PlayOnebyone();            //单帧播放
	BOOL SetPlayPos(float fRelativePos);  //设置播放位置
	float GetPlayPos();                   //获得当前播放位置
	BOOL InputData(PBYTE pBuf,DWORD nSize);  //流接口输入数据
	DWORD GetFileTime();                     //得到文件的时间长度
	DWORD GetPlayedTime();                   //得到已经播放的时间
	DWORD GetPlayedTimeEX();
	DWORD GetPlayedFrames();                 //得到已经播放的桢数
	BOOL SetImgBufCount(int Num) ;           //设置缓冲区大小（图片数）
	BOOL GetImgBufCount() ;                  //得到缓冲区大小

	BOOL SetDecCBType(DWORD dectype) ;         //设置解码回调类型
	BOOL SetDecAudio(BOOL IfDecAudio) ;//设置是否解码音频
	BOOL ReFreshEx(int nRegionNum) ;         //刷新多区域显示中的某个区域
	//扩展流接口，暂时处理成一样
	BOOL InputVideoData(PBYTE pBuf,DWORD nSize);
	BOOL InputAudioData(PBYTE pBuf,DWORD nSize);
	DWORD GetSourceBufferRemain();    //获取流模式下剩余缓冲大小
	BOOL  ResetBufferRemain();        //复位流模式剩余数据
	BOOL RefreshPlay() ;            //刷新显示
	DWORD GetFileTotalFrames() ;    //得到文件总桢数
	DWORD GetCurFrameNum();         //得到当前播放桢序号
	DWORD GetFrameRate();           //得到当前播放桢率
	BOOL  GetPictureSize(LONG* pWidth, LONG* pHeight);

	BOOL SetCurrentFrameNum(DWORD nFrameNum);   //按桢序号设置播放位置
	BOOL SetPlayTime(DWORD nTime);              //按时间设置播放位置
	
	BOOL GetRefValue(BYTE *pBuffer, DWORD *pSize);         //获取索引信息
	BOOL SetRefValue(BYTE *pBuffer, DWORD nSize);          //设置索引信息

	BOOL ResetBuffer(DWORD nBufType);                 //复位缓冲
	DWORD GetBufferValue(DWORD nBufType);             //获取缓冲的大小
	BOOL SetColor(DWORD nRegionNum, int nBrightness, int nContrast, int nSaturation, int nHue);      //设置视频参数
	BOOL GetColor(DWORD nRegionNum, int *pBrightness, int *pContrast, int *pSaturation, int *pHue);  //获取视频参数

	////////////////////////////////////////////
	BOOL  SetDisplayType(int displaytype) ;
	int   GetDisplayType() ;
	BOOL  AdjustWaveAudio(BYTE* pcm, unsigned int datalen, int bitsPerSample,long nCoefficient) ;//调整波形
	BOOL  SetCoefficient(int nCoefficient) ;	
	BOOL  precess(unsigned char *yuv, int width, int height) ;
	BOOL  StartDataRecord(char *sFileName, int idataType) ;
	BOOL  StartAVIResizeConvert(char *sFileName, long lDesWidth, long lDesHeight);
	BOOL StopDataRecord() ;

	BOOL AdjustFluency(int bufnum, int adjustrange);
	BYTE* GetLastFrame();
	BOOL FixRate(int rate);
	void GetTimeStr(char* pBuf,int len);

	BOOL GetMediaInfo(char* pBuf, int len);

	int GetFileRate();
	BOOL  SetWaterMarkCallbackFunc(void *pvFunc, long useData);
	BOOL  SetWaterPandoraMarkCallbackFunc(void *pvFunc, long nUser);
	BOOL  SetRotateAngle(int rotateType);

	BOOL SetIVSCallbackFunc(void *pIVSFunc, long nUser);
	BOOL GetRealFrameBitRate(double *pBitRate);
protected:
private:
	BOOL Write();       //送数据解码,通过解码回调将数据送出来显示
	int GetAVData();   //获取视频数据
	static DWORD  __stdcall ChangeSpeedProc(LPVOID pParam);
};

#endif
