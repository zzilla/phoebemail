/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：远程回放类
* 摘　要：远程回放功能模块。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _SEARCHRECORDANDPLAYBACK_H_
#define _SEARCHRECORDANDPLAYBACK_H_

#include "StdAfx.h"
#include "netsdk.h"

class CManager;
class CVideoRender;
class CNetPlayBack;

#define INVALID_OFFSET_TIME -1
#define INVAlID_OFFSET_BYTE	-1

#define MAX_RECORD_NUM 16 //每次查询录象返回的最大录象条数 

/* 查找录像文件的用户信息 */
typedef struct
{
	afk_device_s*		device;
	list<NET_RECORDFILE_INFO*> lstrf;	/* 录像文件信息列表 */
} st_SearchRecord_Info;

/* 网络回放信息结构 */
typedef struct
{
	afk_channel_s*		channel;			// 通道对象句柄
	int					channelid;			// 图像通道号
	int					nPlayBackType;		// 0：按文件回放；1：按时间回放
	CVideoRender*		Render;				// 解码、播放对象
	CNetPlayBack*		PlayBack;			// 回放缓冲对象
	unsigned int		nFrameRate;			// 回放帧率
	BOOL				bAudioPlay;			// 是否播放音频
	BOOL				bDownLoadEnd;		// 标志是否已下载结束
	NET_TIME			timeStart;			// 用户回放录象(按文件或按时间)的开始时间
	NET_TIME			timeEnd;			// 同上，结束时间
	unsigned int		nTotalSize;			// 录象总大小
	unsigned int		nReceiveSize;		// 已接收的录象数据长度
	unsigned int		nPlayBackSize;		// 已播放大小
	unsigned int		nOffsetSize;		// 录象定位后的偏移位置
	
	fDownLoadPosCallBack pNetPlayBackPosCallBack;// 回放进度回调函数指针
	DWORD				dwPosUser;			// 进度回调的用户数据
	fDataCallBack		fNetDataCallBack;	// 回放数据回调函数指针
	DWORD				dwDataUser;			// 数据回调的用户数据
	NET_RECORDFILE_INFO* pFileInfo;			// 按文件回放时存放文件信息
	NET_RECORDFILE_INFO* prf;				// 按时间回放时存放文件信息列表
	int					nrflen;				// 按时间回放时，文件信息列表的长度
	int					ncurrf;				// 按时间回放时，当前正在回放的文件下标
	DWORD				dwTotalTime;		// 按时间回放时，总时间秒数
	
	OS_THREAD			hThread;			// 回放线程
	OS_EVENT			hPBExit;			// 通知结束回放线程的事件句柄
	DWORD				dwThreadID;			// 回放线程的线程ID
	int					nConnectID;
} st_NetPlayBack_Info;

/* 录像下载信息结构 */
typedef struct
{
	afk_channel_s*		channel;			// 通道对象句柄
	int					channelid;			// 下载的图像通道号
	FILE*				file;				// 本地文件
	int					fileflushflag;		// 文件缓冲刷新标志
	unsigned int		nTotalSize;			// 下载的数据总长度
	unsigned int		nDownLoadSize;		// 已下载的数据长度
	NET_TIME			timeStart;			// 下载开始时间
	NET_TIME			timeEnd;			// 下载结束时间
	fDownLoadPosCallBack pDownLoadPosCallBack;			// 按文件下载时，下载进度回调函数指针
	fTimeDownLoadPosCallBack pTimeDownLoadPosCallBack;	// 按时间下载时，下载进度回调函数指针
	DWORD				userdata;			// 回调的用户数据
	NET_RECORDFILE_INFO* prf;				// 按时间回放时，存放文件信息的列表
	int					nrflen;				// 同上，列表长度
	int					ncurrf;				// 同上，当前正在下载的文件下标
	int					nConnectID;
} st_DownLoad_Info;

//////////////////////////////////////////////////////////////////////////


class CSearchRecordAndPlayBack  
{
public:
	CSearchRecordAndPlayBack(CManager *pManager);
	virtual ~CSearchRecordAndPlayBack();

public:
	/* 初始化 */
	int							Init(void);

	/* 清理 */
	int							Uninit(void);

	/* 查询某段时间内是否有录象 */
	BOOL						QueryRecordTime(LONG lLoginID, int nChannelId, int nRecordFileType, LPNET_TIME tmStart, 
												LPNET_TIME tmEnd, char* pchCardid, BOOL *bResult, int waittime=1000);

	/* 老的录像查询接口 */
	BOOL						QueryRecordFile(LONG lLoginID, int nChannelId, int nRecordFileType, LPNET_TIME time_start,
												LPNET_TIME time_end, char* cardid, LPNET_RECORDFILE_INFO fileinfo, int maxlen,
												int *filecount, int waittime, BOOL bTime);


	BOOL						QueryFurthestRecordTime(LONG lLoginID, int nRecordFileType, char *pchCardid, NET_FURTHEST_RECORD_TIME* pFurthrestTime, int nWaitTime);

	/* 开始查找录像文件 */
	LONG						FindFile(LONG lLoginID, int nChannelId, int nRecordFileType, char* cardid, 
												LPNET_TIME time_start,LPNET_TIME time_end, BOOL bTime,int waittime);

	/* 查找文件信息 */
	int							FindNextFile(LONG lFindHandle, LPNET_RECORDFILE_INFO lpFindData);

	/* 关闭查找句柄 */
	int							FindClose(LONG lFindHandle);

	/* 网络回放 */
	LONG						PlayBackByRecordFile(LONG lLoginID, LPNET_RECORDFILE_INFO lpRecordFile, 
													HWND hWnd, fDownLoadPosCallBack cbDownLoadPos, DWORD dwUserData);

	/* 扩展，增加了数据回调 */
	LONG						PlayBackByRecordFileEx(LONG lLoginID, LPNET_RECORDFILE_INFO lpRecordFile, 
													HWND hWnd, fDownLoadPosCallBack cbDownLoadPos, DWORD dwPosData, 
													fDataCallBack fDownLoadDataCallBack, DWORD dwDataUser);
	
	/* 暂停回放 */
	int							PausePlayBack(LONG lPlayHandle, BOOL bPause);

	/* 定位录像回放 */
	int							SeekPlayBack(LONG lPlayHandle, unsigned int offsettime, unsigned int offsetbyte);

	/* 停止录像回放 */
	int							StopPlayBack(LONG lPlayHandle);

	/* 单步回放 */
	int							StepPlayBack(LONG lPlayHandle, BOOL bStop);

	/* 快放 */
	int							FastPlayBack(LONG lPlayHandle);

	/* 慢放 */
	int							SlowPlayBack(LONG lPlayHandle);
	
	/* 恢复正常播放速度 */
	int							NormalPlayBack(LONG lPlayHandle);

	/* 设置帧率 */
	int							SetFramePlayBack(LONG lPlayHandle, int framerate);

	/* 获取帧率 */
	int							GetFramePlayBack(LONG lPlayHandle, int *fileframerate, int *playframerate);

	/* 获取OSD时间 */
	int							GetPlayBackOsdTime(LONG lPlayHandle, LPNET_TIME lpOsdTime, LPNET_TIME lpStartTime, LPNET_TIME lpEndTime);

	/* 开始下载录像 */
	LONG						DownloadByRecordFile(LONG lLoginID,LPNET_RECORDFILE_INFO lpRecordFile, char *sSavedFileName, 
													fDownLoadPosCallBack cbDownLoadPos, DWORD dwUserData);

	/* 停止下载录像 */
	int							StopDownload(LONG lFileHandle);

	/* 获取录像下载进度 */
	int							GetDownloadPos(LONG lFileHandle, int *nTotalSize, int *nDownLoadSize);

	/* 设置解码端图像 */
	int							SetDecoderVideoEffect(LONG lPlayHandle, unsigned char brightness, unsigned char contrast, 
														unsigned char hue, unsigned char saturation);

	/* 获取解码端图像 */
	int							GetDecoderVideoEffect(LONG lPlayHandle, unsigned char *brightness, unsigned char *contrast, 
														unsigned char *hue, unsigned char *saturation);

	/* 打开解码库声音 */
	int							Decoder_OpenSound(LONG lPlayHandle);

	/* 关闭解码库声音 */
	int							Decoder_CloseSound();

	/* 控制音量 */
	int							SetVolume(LONG lPlayHandle, int nVolume);

	/* 解码端拍照 */
	int							CapturePicture(LONG lPlayHandle, const char *pchPicFileName);

	/* 获取码流统计 */
	LONG						GetStatiscFlux(LONG lLoginID,LONG lPlayHandle);

	/* 关闭一个设备所有通道 */
	int							CloseChannelOfDevice(afk_device_s* device);

	/* 按时间回放 */
	LONG						PlayBackByTime(LONG lLoginID, int nChannelID, LPNET_TIME lpStartTime, LPNET_TIME lpStopTIme, 
												fDownLoadPosCallBack cbDownLoadPos, DWORD dwPosUser, HWND hWnd);

	/* 按时间回放扩展，增加了数据回调 */
	LONG						PlayBackByTimeEx(LONG lLoginID, int nChannelID, LPNET_TIME lpStartTime, LPNET_TIME lpStopTIme, 
												fDownLoadPosCallBack cbDownLoadPos, DWORD dwPosUser, HWND hWnd, 
												fDataCallBack fDownLoadDataCallBack, DWORD dwDataUser);
	/* 按时间下载 */
	LONG						DownloadByTime(LONG lLoginID, int nChannelId, int nRecordFileType, LPNET_TIME tmStart, LPNET_TIME tmEnd, char *sSavedFileName, fTimeDownLoadPosCallBack cbTimeDownLoadPos, DWORD dwUserData);


public:
	class SearchNPIbyChannel;
	friend class SearchNPIbyChannel;

	class SearchDLIbyChannel;
	friend class SearchDLIbyChannel;


public:
	
	static	int	__stdcall		QueryRecordFileInfoFunc(
												afk_handle_t object,	/* 数据提供者 */
												unsigned char *data,	/* 数据体 */
												unsigned int datalen,	/* 数据长度 */
												void *param,			/* 回调参数 */
												void *udata);

	static	int	__stdcall		NetPlayBackCallBackFunc(
												afk_handle_t object,	/* 数据提供者 */
												unsigned char *data,	/* 数据体 */
												unsigned int datalen,	/* 数据长度 */
												void *param,			/* 回调参数 */
												void *udata);
			

private:
	
	/* 释放录像文件的句柄列表 */
	void						ReleaseAllSearchRecordInfo(void);
	/* 释放单个的录像文件句柄 */
	void						ReleaseRecordFileInfo(st_SearchRecord_Info& sr);
	/* 执行关闭回放操作 */
	int							Process_stopplayback(st_NetPlayBack_Info& npi);
	/* 根据回放句柄获取回放信息 */
	st_NetPlayBack_Info*		GetNetPlayBackInfo(LONG lPlayHandle);
	/* 根据下载句柄获取下载信息 */
	st_DownLoad_Info*			GetDownLoadInfo(LONG lFileHandle);
	/* 执行查询录象文件 */
	int							Process_QueryRecordfile(afk_device_s* device,
												int nChannelId, 
												int nRecordFileType, 
												LPNET_TIME time_start,
												LPNET_TIME time_end,
												char* cardid, 
												int waittime,
												BOOL bTime,
												list<NET_RECORDFILE_INFO*>& lstRecordFile);

	/* 对录象查询的能力级判断 */
	BOOL						SearchRecordProtocol(LONG lLoginID);

private:

	list<st_SearchRecord_Info*> m_lstSRI;	/* 查找录像文件的句柄列表，服务于新的查询录象接口 */
	DEVMutex						m_csSRI;	/* 锁查找录像文件句柄的列表 */
	list<st_NetPlayBack_Info*>	m_lstNPI;	/* 网络回放信息列表 */
	DEVMutex						m_csNPI;	/* 锁网络回放信息列表 */
	list<st_DownLoad_Info*>		m_lstDLI;	/* 录像下载信息列表 */
	DEVMutex						m_csDLI;	/* 锁录像下载信息列表 */

private:
	CManager*		m_pManager;
};


class CSearchRecordAndPlayBack::SearchNPIbyChannel
{
	LONG m_lChannel;
public:
	SearchNPIbyChannel(LONG lChannel):m_lChannel(lChannel){}
	
	bool operator()(st_NetPlayBack_Info*& pi)
	{
		return m_lChannel == (LONG)(pi?pi->channel:0);
	}
};

class CSearchRecordAndPlayBack::SearchDLIbyChannel
{
	LONG m_lChannel;
public:
	SearchDLIbyChannel(LONG lChannel):m_lChannel(lChannel){}

	bool operator()(st_DownLoad_Info*& di)
	{
		return m_lChannel == (LONG)(di?di->channel:0);
	}
};


#endif // _SEARCHRECORDANDPLAYBACK_H_

