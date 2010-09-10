/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：实时监视类
* 摘　要：实时监视功能模块。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _REALPLAY_H_
#define _REALPLAY_H_

#include "StdAfx.h"
#include "netsdk.h"
#include "Timer.h"

class CManager;
class CVideoRender;


/* 实时预览信息结构 */
typedef struct
{
	afk_channel_s		*channel;
	int					channelid;
	CVideoRender		*Render;
	FILE				*file;
	int					fileflushflag;
	fRealDataCallBack	pRealDataCallBack;
	fRealDataCallBackEx	pRealDataCallBackEx;
	DWORD				dwDataTypeFlag;
	DWORD				userdata;
	BOOL				bAudioPlay;
	int					nConnectID;
} st_Monitor_Info;


/* 云台控制信息结构 */
typedef struct
{
	afk_device_s		*device;
	afk_control_channel_param_s controlparam;
} st_PtzControl_Info;


class CRealPlay  
{
public:
	CRealPlay(CManager *pManager);
	virtual ~CRealPlay();

public:
	
	/* 初始化 */
	int							Init(void);

	/* 清理 */
	int							Uninit(void);

	/* 关闭一个设备所有通道 */
	int							CloseChannelOfDevice(afk_device_s* device);

	/* 开始实时监视 */
	LONG						StartRealPlay(LONG lLoginID, int nChannelID, HWND hWnd, UINT subType = 0);

	/* 停止实时监视 */
	int							StopRealPlay(LONG lRealHandle);

	/* 调整图象播放的流畅性 */
	int							AdjustFluency(LONG lRealHandle, int nLevel);

	/* 开始多画面预览 */
	LONG						StartMultiPlay(LONG lLoginID, int nChannelID, HWND hWnd, UINT subType = RType_Multiplay);

	/* 停止多画面预览 */
	int							StopMultiPlay(LONG lMultiPlay);

	/* 设置解码端图像 */
	int							SetDecoderVideoEffect(LONG lRealHandle, unsigned char brightness, unsigned char contrast, unsigned char hue, unsigned char saturation);

	/* 获取解码端图像 */
	int							GetDecoderVideoEffect(LONG lRealHandle, unsigned char *brightness, unsigned char *contrast, unsigned char *hue, unsigned char *saturation);

	/* 打开解码库声音 */
	int							Decoder_OpenSound(LONG lRealHandle);

	/* 关闭解码库声音 */
	int							Decoder_CloseSound();

	/* 控制音量 */
	int							SetVolume(LONG lRealPlay, int nVolume);

	/* 开始保存实时监视数据 */
	int							StartSaveRealData(LONG lRealPlay, const char *pchFileName);

	/* 停止保存实时监视数据 */
	int							StopSaveRealData(LONG lRealPlay);

	/* 设置保存实时监视数据的回调函数 */
	int							SetRealDataCallBack(LONG lRealPlay, fRealDataCallBack cbRealData, DWORD dwUser);

	/* 设置保存实时监视数据的回调函数-扩展*/
	int							SetRealDataCallBackEx(LONG lRealPlay, fRealDataCallBackEx cbRealData, DWORD dwUser, DWORD dwFlag);
	
	/* 解码端拍照 */
	int							CapturePicture(LONG lRealPlay, const char *pchPicFileName);

	/* 开始云台控制处理函数 */
	int							StartPTZControlCommand(afk_device_s *device, afk_control_channel_param_s controlparam, BOOL bOnce, BOOL bInTimer);	

	/* 停止云台控制处理函数 */
	int							StopPTZControlCommand(afk_device_s *device, int nChannelID);

	/* 普通云台控制 */
	int							PTZControl(LONG lLoginID, int nChannelID, DWORD dwPTZCommand, DWORD dwStep, BOOL dwStop);

	/* 亿蛙云台控制 */
	int							YWPTZControl(LONG lLoginID, int nChannelID, DWORD dwPTZCommand, unsigned char param1, unsigned char param2, unsigned char param3, BOOL dwStop);
	
	/*亿蛙云台控制扩展*/
	int							YWPTZControlEx(LONG lLoginID, int nChannelID, DWORD dwPTZCommand, LONG lParam1, LONG lParam2, LONG lParam3, BOOL dwStop);
	
	/* 获取流量统计 */
	LONG						GetStatiscFlux(LONG lLoginID, LONG lPlayHandle);

	/* 获取帧率 */
	int							GetFrameRealPlay(LONG lPlayHandle, int *fileframerate, int *playframerate);


public:
	class SearchMIbyHandle;
	friend class SearchMIbyHandle;

	class SearchPIbyDevice;
	friend class SearchPIbyDevice;


public:
	static void	__stdcall		ptzcontrol_proc(LPVOID lpPara);

	void						Process_ptzcontrol_proc();


private:
	/* 处理关闭实时预览图像 */
	int							ProcessStopRealPlay(st_Monitor_Info& moninfo);


private:
	CManager*					m_pManager;

	std::list<st_Monitor_Info*>	m_lstMonitors;	/* 监视信息列表 */
	DEVMutex						m_csMonitors;
	
	std::list<st_PtzControl_Info*> m_lstPTZ;	/* 云台控制信息列表 */
	DEVMutex						m_csPTZ;

	CTimer					m_timer;		/* 云台定时器 */
	BOOL						m_bIsOpen;		/* 定时器是否开启 */
};


class CRealPlay::SearchMIbyHandle
{
	LONG m_lHandle;
public:
	SearchMIbyHandle(LONG lHandle):m_lHandle(lHandle){}

	bool operator()(st_Monitor_Info*& mi)
	{
		return m_lHandle == (LONG)(mi?mi->channel:0);
	}
};

class CRealPlay::SearchPIbyDevice
{
	LONG m_lDevice;
	int  m_nChannelId;
public:
	SearchPIbyDevice(LONG lDevice,int nChannelId):m_lDevice(lDevice),m_nChannelId(nChannelId){}

	bool operator()(st_PtzControl_Info*& pi)
	{
		return (m_lDevice == (LONG)(pi?pi->device:0)) && (m_nChannelId == pi->controlparam.no);
	}
};


#endif // _REALPLAY_H_



