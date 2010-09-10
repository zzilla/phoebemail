#ifndef _VIDEORENDER_H
#define _VIDEORENDER_H

#include "StdAfx.h"
#include "RealPlay.h"

#ifdef WIN32	//目前解码只支持windows操作系统
#include "./depend/playsdk.h"
#include "./depend/playdef.h"
#endif 
/*
class CVideoRender
{
public:
	CVideoRender(long hWnd);
	virtual ~CVideoRender();
protected:
	//窗口句柄
    long  m_hWnd;

	//标志是否正在播放
    bool  m_bPlay;

	//帧率
    int   m_nframerate;

	//标志是否正在单帧播放
    bool  m_bStep;

	//标志是否已暂停
    bool  m_bPause;

	//图像颜色参数
    unsigned char m_brightness;
    unsigned char m_contrast;
    unsigned char m_hue;
    unsigned char m_saturation;

	//帧时间信息
    unsigned int  m_year;
    unsigned int  m_month;
    unsigned int  m_day;
    unsigned int  m_hour;
    unsigned int  m_minute;
    unsigned int  m_second;

public:
	//画面显示回调函数指针
    void  *m_pDrawFunc;
    void  *m_userdata1;
    void  *m_userdata2;
    void  *m_userdata3;

    st_Monitor_Info *m_monitor;

public:
	//保护m_monitor
	CRITICAL_SECTION m_csMT;

	//保护drawCallback
	CRITICAL_SECTION m_csDC;

	//标志是否打开了音频
	bool  m_bAudioPlay;
private:
	//解码标志位，0－MAX_RENDER_COUNT
    int   m_nScreenID;

	//解码器对象
    void *m_h264Decoder;
public:
	//获取/占据空闲解码标志位，0－MAX_RENDER_COUNT
    static int GetFreeScreenID();
    static void SetScreenFlag(int index, char bFlag);

	//设置OSD时间，对象内部用
    void  SetOSDTime(unsigned long year, unsigned long month, unsigned long day,
        unsigned long hour, unsigned long minute, unsigned long second);

	//获取OSD时间，接口函数
    void  GetOSDTime(unsigned long *year, unsigned long *month, unsigned long *day,
        unsigned long *hour, unsigned long *minute, unsigned long *second);

	//设置画面显示回调函数
    void  SetDrawCallBack(void *pDrawCBFunc, void *userdata1, void *userdata2, void *userdata3);

	//开始/结束播放，对象内部函数
    bool  StartPlay(int type, bool framectrl);
    bool  EndPlay();
	
	//填充多媒体数据
    bool  Play(unsigned char *pBuf, int nLen);
	//清空解码缓存
    int  Reset();
	//判断解码缓存是否为空
    bool  IsEmpty();

	//开始/结束解码，对象接口函数，是对StartPlay/EndPlay的封装
	int  StartDec(BOOL framectrl);
	int  StopDec();

	//闲置render资源再次被利用时，刷新一些成员变量
	int  ChangeHwnd(HWND hwnd);

	//获取帧率
    int   GetFrameRate() { return m_nframerate; }
	//播放控制
    bool  SetFrameRate(int framerate);
    bool  Slow();
    bool  Fast();
    bool  Pause(bool bPause);
    bool  Step(bool bStop);
	//抓图
    bool  SnapPicture(const char *pchFileName);

	//音频函数
    bool  OpenAudio();
    bool  CloseAudio();
    bool  PlayAudio(unsigned char *pBuf, int nLen);
    bool  IsAudio() { return m_bAudioPlay; }
    bool  SetAudioVolume(int nVolume);

	//图像颜色参数
    bool  AdjustColor(unsigned char brightness, unsigned char contrast, 
        unsigned char hue, unsigned char saturation);
    void  GetColorParam(unsigned char *brightness, unsigned char *contrast, 
        unsigned char *hue, unsigned char *saturation);

	void SetMonitorInfo(st_Monitor_Info* pMI);

	int  CaptureToFile(char* inbuf, int buflen, const char *filename, void *parm);
};
*/

#define RENDER_BUFFER_LEN (1024*900)

class CVideoRender
{
public:
	CVideoRender(HWND hWnd);
	virtual ~CVideoRender();
	
public:
	//开始/停止解码
	int  StartDec(BOOL framectrl, DWORD dwTalkType = 0/*是否用于语音对讲，对讲音频编码类型信息*/);
	int  StopDec();

	//音频函数
    BOOL  OpenAudio();
    BOOL  CloseAudio();
//    BOOL  PlayAudio(unsigned char *pBuf, int nLen);
    BOOL  IsAudio();
    BOOL  SetAudioVolume(int nVolume);

	//获取帧率
    int   GetFrameRate();
	//播放控制
    BOOL  SetFrameRate(int framerate);
    BOOL  Slow();
    BOOL  Fast();
    BOOL  Pause(BOOL bPause);
    BOOL  Step(BOOL bStop);
	BOOL  PlayNormal();

	//抓图
    BOOL  SnapPicture(const char *pchFileName);

	
	//闲置render资源再次被利用时，刷新一些成员变量
	int  ChangeHwnd(HWND hWnd);

	
	//填充多媒体数据
    BOOL  Play(unsigned char *pBuf, int nLen);
	//清空解码缓存
    int  Reset();
	//判断解码缓存是否为空
    BOOL  IsEmpty();
	
	//获取OSD时间，接口函数
    BOOL  GetOSDTime(unsigned long *year, unsigned long *month, unsigned long *day,
        unsigned long *hour, unsigned long *minute, unsigned long *second);
	
	//设置画面显示回调函数
    BOOL  SetDrawCallBack(void* pDrawCBFunc, void* ud1, void* ud2, void* ud3);
	
	
	//图像颜色参数
    BOOL  AdjustColor(unsigned char brightness, unsigned char contrast, 
        unsigned char hue, unsigned char saturation);
    BOOL  GetColorParam(unsigned char *brightness, unsigned char *contrast, 
        unsigned char *hue, unsigned char *saturation);
	

	BOOL SetMonitorInfo(st_Monitor_Info* pMI);

	//调整图象播放的流畅性
	BOOL  AdjustFluency(int nLevel);

	//获取解码缓存中未解码的数据大小
	DWORD GetSourceBufferRemain();

	
#ifdef WIN32	//目前解码只支持windows操作系统
	BOOL CloseAudioRecord();
	BOOL OpenAudioRecord(pCallFunction pProc,long nBitsPerSample,long nSamplesPerSec,long nLength,long nReserved,long nUser);
#endif

	//内部接口
public:
	void MyDrawFunc_Imp(long nPort,HDC hDc);
	void MyDisplayFunc_Imp(long nPort, char* pBuf, long nSize, long nWidth, long nHeight, long nStamp, long nType);
	void MyDemuxFunc_Imp(long nPort, char* pBuf, long nSize, void* pParam, long nReserved);
	void MyAudioFunc_Imp(long nPort, char * pAudioBuf, long nSize, long nStamp, long nType);

protected:
private:
	HWND m_hWnd;
	int m_iPlayPort;

	BOOL m_bPlaying;
	BOOL m_bStep;
	BOOL m_bAudio;

	int m_tmpbpsam; //临时存放采样深度
	int m_tmpsamps; //临时存放采样率

	void*	m_pfDrawFunc;
	void*	m_drawUd1;
	void*	m_drawUd2;
	void*	m_drawUd3;

	st_Monitor_Info *m_monitor;
	//保护m_monitor
	DEVMutex m_csMT;
	
	//test file
//	FILE* m_file;
};

#endif

