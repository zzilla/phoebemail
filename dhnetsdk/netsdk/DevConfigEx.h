/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：配置类
* 摘　要：配置功能模块。
*
*/
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEVCONFIGEX_H__651C4A7B_0169_4E84_BB50_CCC9AE4CF779__INCLUDED_)
#define AFX_DEVCONFIGEX_H__651C4A7B_0169_4E84_BB50_CCC9AE4CF779__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "netsdk.h"

class CManager;

class CDevConfigEx  
{
public:
	/* 配置导出相关参数 */
	typedef struct
	{
		afk_channel_s		*channel;			// 通道对象句柄
		FILE				*file;				// 本地文件
		int					fileflushflag;		// 文件缓冲刷新标志
		char				szFilePath[256];	// 文件路径
		unsigned int		nTotalSize;			// 数据总长度
		unsigned int		nDownLoadSize;		// 已下载的数据长度
		unsigned int		nTotalFileCount;	// 总的配置文件数量
		unsigned int		nCurFileNum;		// 当前配置文件的编号
		fDownLoadPosCallBack pDownLoadPosCallBack;	// 下载进度回调函数指针
		DWORD				userdata;			// 用户数据
		int					nConnectID;
	} st_ConfigExport_Info;

	/* 配置导入相关参数 */
	typedef struct
	{
		afk_device_s*		device;
		afk_channel_s*		channel;
		fDownLoadPosCallBack pImportPosCallBack;
		DWORD				dwUser;
		OS_EVENT			hRecEvent;
		BOOL				bAccept;
	} st_ConfigImport_Info;


public:
	CDevConfigEx(CManager *pManager);
	virtual ~CDevConfigEx();


public:
	class SearchDLIbyChannel;
	friend class SearchDLIbyChannel;

	class SearchUploadbyChannel;
	friend class SearchUploadbyChannel;


public:
	/* 初始化 */
	int							Init(void);

	/* 清理 */
	int							Uninit(void);

	/* 关闭一个设备的所有通道 */
	int							CloseChannelOfDevice(afk_device_s* device);

	/* 开始导出配置文件 */
	LONG						ExportConfigFile(LONG lLoginID, CONFIG_FILE_TYPE emConfigFileType, char *szSavedFilePath, 
                                      fDownLoadPosCallBack cbDownLoadPos, DWORD dwUserData);

	/* 停止导出配置文件 */
	int							StopExportCfgFile(LONG lExportHandle);

	/* 开始导入配置文件 */
	LONG						ImportConfigFile(LONG lLoginID, char *szFileName, fDownLoadPosCallBack cbUploadPos, DWORD dwUserData);

	/* 停止导出配置文件 */
	int							StopImportCfgFile(LONG lImportHandle);

	/*
	 * 以分页方式查询日志。
	 * 兼容	QueryLog的日志查询，并将其日志信息转化成新的日志结构体（DEVICE_LOG_ITEM）。
	 */
	int QueryDeviceLog(LONG lLoginID, QUERY_DEVICE_LOG_PARAM *pQueryParam, char *pLogBuffer, int nLogBufferLen, int *pRecLogNum, int waittime);

	/* 搜索网络上的DVR设备 */
	int SearchDevice(char* szBuf, int nBufLen, int* pRetLen, DWORD dwSearchTime);

	/* 申请建立会话 */
	int SetupSession(LONG lLoginID, int nChannelID, afk_connect_param_t* pConnParam);

	/* 销毁会话 */
	int DestroySession(LONG lLoginID, int nConnectID);

	/* 查询组播地址 */
	int	GetMulticastAddr(afk_device_s* device, afk_login_device_type *pstMulticastInfo);

	/* 查询UDP端口 */
	int	GetUdpPort(afk_device_s* device, afk_login_device_type *pstUdpInfo);

	/*按新协议获取配置*/
	int GetDevNewConfig(LONG lLoginID, DWORD dwCommand, LONG lChannel, LPVOID lpOutBuffer,DWORD dwOutBufferSize, LPDWORD lpBytesReturned,int waittime);
	/*  各功能子函数 */
	int	GetDevNewConfig_IPFilter(LONG lLoginID, DEVICE_IPIFILTER_CFG &stIPFilter, int waittime);
	int GetDevNewConfig_TalkEncode(LONG lLoginID, DEVICE_TALK_ENCODE_CFG &stTalkEncode, int waittime);
	int GetDevNewConfig_RecordPacketLen(LONG lLoginID, DEVICE_RECORD_PACKET_CFG &stRecordLen, int waittime);
	int	GetDevNewConfig_AlmDecCfg(LONG lLoginID, int nChannel, ALARMDECODER_CFG *stAlmDec, int nWaitTime);
	int GetDevNewConfig_Sniffer(LONG lLoginID, YW_ATM_SNIFFER_CFG_EX* pSnifferCfgEx, int nGroupID, int waittime);
	int GetDevNewConfig_MMS(LONG lLoginID, DEVICE_MMS_CFG &stMMS, int waittime);
	int GetDevNewConfig_DownloadRate(LONG lLoginID, int& nDownLoadRate, int waittime);
	int GetDevNewConfig_PanoramaSwitch(LONG lLoginID, ALARM_PANORAMA_SWITCH_CFG *lpAlarmCfg, int waittime);
	int GetDevNewConfig_LostFocus(LONG lLoginID, ALARM_LOST_FOCUS_CFG *lpAlarmCfg, int waittime);
	int GetDevNewConfig_VideoOut(LONG lLoginID, DEV_VIDEOOUT_INFO &stVideoOutCfg, int waittime);
	int GetDevNewConfig_Point(LONG lLoginID, DEVICE_POINT_CFG& stuPointCfg, int waittime);
	int	GetDevNewConfig_AllLocalALMCfg(LONG lLoginID, ALARM_IN_CFG_EX *pstLocalAlm, LONG lChnNum, int waittime);
	int	GetDevNewConfig_LocalALMCfg(LONG lLoginID, ALARM_IN_CFG_EX &stLocalAlm, LONG lChannel, int waittime);
	int	GetDevNewConfig_AllNetALMCfg(LONG lLoginID, ALARM_IN_CFG_EX *pstNetlAlm, LONG lChnNum, int waittime);
	int	GetDevNewConfig_NetALMCfg(LONG lLoginID, ALARM_IN_CFG_EX &stNetAlm, LONG lChannel, int waittime);
	int	GetDevNewConfig_AllMotionALMCfg(LONG lLoginID, MOTION_DETECT_CFG_EX *pstMotionAlm, LONG lChnNum, int waittime);
	int	GetDevNewConfig_MotionALMCfg(LONG lLoginID, MOTION_DETECT_CFG_EX &stMotionAlm, LONG lChannel, int waittime);
	int	GetDevNewConfig_AllVideoLostALMCfg(LONG lLoginID, VIDEO_LOST_CFG_EX *pstVideoLostAlm, LONG lChnNum, int waittime);
	int	GetDevNewConfig_VideoLostALMCfg(LONG lLoginID, VIDEO_LOST_CFG_EX &stVideoLostAlm, LONG lChannel, int waittime);
	int	GetDevNewConfig_AllBlindALMCfg(LONG lLoginID, BLIND_CFG_EX *pstBlindAlm, LONG lChnNum, int waittime);
	int	GetDevNewConfig_BlindALMCfg(LONG lLoginID, BLIND_CFG_EX &stBlindAlm, LONG lChannel, int waittime);
	int	GetDevNewConfig_DiskALMCfg(LONG lLoginID, DISK_ALARMIN_CFG_EX *stDiskAlm, int waittime);
	int	GetDevNewConfig_NetBrokenALMCfg(LONG lLoginID, NET_BROKEN_ALARM_CFG_EX *pNetBrkCfg, int waittime);
	int	GetDevNewConfig_EncoderCfg(LONG lLoginID, DEV_ENCODER_CFG *pEncoderCfg, int waittime);
	int GetDevNewConfig_IPCOllisionDetect(LONG lLoginID, ALARM_IP_COLLISION_CFG *lpAlarmCfg, int waittime);
	int GetDevNewConfig_OSDEnable(LONG lLoginID, DEV_OSD_ENABLE_CONFIG* stuOSDEnableCfg, int waittime);
	int GetDevNewConfig_TVAdjustCfg(LONG lLoginID, LONG lTVID, DEVICE_TVADJUST_CFG* pTVAdjustCfg, int waittime);
	int GetDevNewConfig_VehicleCfg(LONG lLoginID, DEV_ABOUT_VEHICLE_CONFIG* pVehicleCfg, int waittime);
	int GetDevNewConfig_AtmOverlayGeneral(LONG lLoginID, DEV_ATM_OVERLAY_GENERAL* pAtmGeneral, int waittime);
	int GetDevNewConfig_AtmOverlayConfig(LONG lLoginID, DEV_ATM_OVERLAY_CONFIG* pAtmConfig, int waittime);
	int GetDevNewConfig_DecoderTourConfig(LONG lLoginID, int nChannel, DEVICE_DECODER_TOUR_CFG* pTourCfg, int waittime);
	int GetDevNewConfig_SIPConfig(LONG lLoginID, DEVICE_SIP_CFG* pSIPCfg, int waittime);
	int GetDevNewConfig_SIPAbility(LONG lLoginID, STREAM_MEDIA_EN* pStreamMediaEn, int waittime);

	//获取POS相关参数 2010-07-13 add by jfchen 
	int GetDevNewConfig_POSConfig( LONG lLoginID , DEVICE_POS_INFO* pPosCfg , int waittime ) ;
	/* 按新协议设置配置 */
	int	SetDevNewConfig(LONG lLoginID, DWORD dwCommand,LONG lChannel, LPVOID lpInBuffer,DWORD dwInBufferSize,int waittime);
	/*  各功能子函数 */
	int	SetDevNewConfig_IPFilter(LONG lLoginID, DEVICE_IPIFILTER_CFG *pstIPFilter, int waittime);
	int	SetDevNewConfig_TalkEncode(LONG lLoginID, DEVICE_TALK_ENCODE_CFG *pstTalkEncode, int waittime);
	int SetDevNewConfig_RecordPacketLen(LONG lLoginID, DEVICE_RECORD_PACKET_CFG *pstRecordLen, int waittime);
	int	SetDevNewConfig_AlmDecCfg(LONG lLoginID, int nChannel, ALARMDECODER_CFG *stAlmDec, int nWaitTime);
	int	SetDevNewConfig_MMS(LONG lLoginID, DEVICE_MMS_CFG *pstMMS, int waittime);
    LONG SetDevNewConfig_FileTrans(LONG lLoginID, int nTransType, char* szInBuf, int nInBufLen, fTransFileCallBack cbTransFile, DWORD dwUserData, int waittime);
	int SetDevNewConfig_Sniffer(LONG lLoginID, YW_ATM_SNIFFER_CFG_EX* pSnifferCfgEx, int nGroupID, int waittime);
	int SetDevNewConfig_DownloadRate(LONG lLoginID, int* pDownLoadRate, int waittime);
	int SetDevNewConfig_PanoramaSwitch(LONG lLoginID, ALARM_PANORAMA_SWITCH_CFG *lpAlarmCfg, int waittime);
	int SetDevNewConfig_LostFocus(LONG lLoginID, ALARM_LOST_FOCUS_CFG *lpAlarmCfg, int waittime);
	int SetDevNewConfig_VideoOut(LONG lLoginID, DEV_VIDEOOUT_INFO *lpVideoOutCfg, int waittime);
	int SetDevNewConfig_Point(LONG lLoginID, DEVICE_POINT_CFG* stuPointCfg, int waittime);
	int	SetDevNewConfig_LocalALMCfg(LONG lLoginID, ALARM_IN_CFG_EX* pLocalAlm, LONG lChannel, int waittime);
	int	SetDevNewConfig_NetALMCfg(LONG lLoginID, ALARM_IN_CFG_EX* pNetAlm, LONG lChannel, int waittime);
	int	SetDevNewConfig_MotionALMCfg(LONG lLoginID, MOTION_DETECT_CFG_EX* pMotionAlm, LONG lChannel, int waittime);
	int	SetDevNewConfig_VideoLostALMCfg(LONG lLoginID, VIDEO_LOST_CFG_EX* pVideoLostAlm, LONG lChannel, int waittime);
	int	SetDevNewConfig_BlindALMCfg(LONG lLoginID, BLIND_CFG_EX* pBlindAlm, LONG lChannel, int waittime);
	int	SetDevNewConfig_DiskALMCfg(LONG lLoginID, DISK_ALARMIN_CFG_EX* pDiskAlmCfg, int waittime);
	int	SetDevNewConfig_NetBrokenALMCfg(LONG lLoginID, NET_BROKEN_ALARM_CFG_EX* pNetBrkCfg, int waittime);
	int	SetDevNewConfig_EncoderCfg(LONG lLoginID, DEV_ENCODER_CFG *pEncoderCfg, int waittime);
	int SetDevNewConfig_IPCOllisionDetect(LONG lLoginID, ALARM_IP_COLLISION_CFG *lpAlarmCfg, int waittime);
	int SetDevNewConfig_OSDEnable(LONG lLoginID, DEV_OSD_ENABLE_CONFIG* stuOSDEnableCfg, int waittime);
	int SetDevNewConfig_TVAdjustCfg(LONG lLoginID, LONG lTVID, DEVICE_TVADJUST_CFG* pTVAdjustCfg, int waittime);
	int SetDevNewConfig_VehicleCfg(LONG lLoginID, DEV_ABOUT_VEHICLE_CONFIG* pVehicleCfg, int waittime);
	int SetDevNewConfig_AtmOverlayConfig(LONG lLoginID, DEV_ATM_OVERLAY_CONFIG* pAtmConfig, int waittime);
	int SetDevNewConfig_DecoderTourConfig(LONG lLoginID, int nChannel, DEVICE_DECODER_TOUR_CFG* pTourCfg, int waittime);
	int SetDevNewConfig_SIPConfig(LONG lLoginID, DEVICE_SIP_CFG* pSIPCfg, int waittime);

	//设置POS参数，2010-07-14 add by jfchen
	int SetDevNetConfig_POSConfig( LONG lLoginID , DEVICE_POS_INFO *pPosCfg , int waittime ) ;
private:
	int GetDevNewConfig_SnifferGroup(char* szSour, YW_ATM_SNIFFER_CFG_EX* pSnifferCfgEx);
	int GetDevNewConfig_SnifferFrame(char* szSour, YW_SNIFFER_FRAMEID* pFrame);
	int GetDevNewConfig_SnifferContent(char* szSour, YW_SNIFFER_CONTENT* pContent);
	
	int SetDevNewConfig_SnifferGroup(YW_ATM_SNIFFER_CFG_EX* pSnifferCfgEx, int nGroupID, char* szBuf, int nBufLen);
	int SetDevNewConfig_SnifferFrame(YW_SNIFFER_FRAMEID* pFrame, int nGroupID, int nFrameID, char* szBuf, int nBufLen);
	int SetDevNewConfig_SnifferContent(YW_SNIFFER_CONTENT* pContent,int nGroupID, int nFrameID, int nContentID, char* szBuf, int nBufLen);
	
	int	GetDevNewConfig_SMSACTIVE(LONG lLoginID, DEVICE_SMSACTIVATION_CFG &stSMSActive, int waittime);
	int	SetDevNewConfig_SMSACTIVE(LONG lLoginID, DEVICE_SMSACTIVATION_CFG *pstSMSActive, int waittime);
	int	GetDevNewConfig_DIALIN(LONG lLoginID, DEVICE_DIALINACTIVATION_CFG &stDIALIN, int waittime);
	int	SetDevNewConfig_DIALIN(LONG lLoginID, DEVICE_DIALINACTIVATION_CFG *pstDIALIN, int waittime);

public:

	static	int	__stdcall		ConfigExportCallBackFunc(
												afk_handle_t object,	/* 数据提供者 */
												unsigned char *data,	/* 数据体 */
												unsigned int datalen,	/* 数据长度 */
												void *param,			/* 回调参数 */
												void *udata);

	static int __stdcall		ConfigImportCallBackFunc(
												afk_handle_t object,	/* 数据提供者 */
												unsigned char *data,	/* 数据体 */
												unsigned int datalen,	/* 数据长度 */
												void *param,			/* 回调参数 */
												void *udata);


private:
	list<st_ConfigExport_Info*>		m_lstCfgExport;
	DEVMutex						m_csCfgExport;

	list<st_ConfigImport_Info*>		m_lstCfgImport;
	DEVMutex						m_csCfgImport;


private:
	CManager		*m_pManager;
};

class CDevConfigEx::SearchDLIbyChannel
{
	LONG m_lChannel;
public:
	SearchDLIbyChannel(LONG lChannel):m_lChannel(lChannel){}

	bool operator()(st_ConfigExport_Info*& di)
	{
		return m_lChannel == (LONG)(di?di->channel:0);
	}
};

class CDevConfigEx::SearchUploadbyChannel
{
	LONG m_lChannel;
public:
	SearchUploadbyChannel(LONG lChannel):m_lChannel(lChannel){}

	bool operator()(st_ConfigImport_Info*& di)
	{
		return m_lChannel == (LONG)(di?di->channel:0);
	}
};


/*
 *	从字符串协议中解析相应值
 */
int GetValueFormNewPacket(const char *szBuf, const char *szKeyBuf, char *szOutBuf, int nOutBufLen);


#endif // !defined(AFX_DEVCONFIGEX_H__651C4A7B_0169_4E84_BB50_CCC9AE4CF779__INCLUDED_)


