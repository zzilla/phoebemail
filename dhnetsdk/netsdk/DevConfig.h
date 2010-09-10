/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：配置类
* 摘　要：配置功能模块。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DEVCONFIG_H_
#define _DEVCONFIG_H_

#include "netsdk.h"
#include "assistant.h"

#define MAX_COMM_PROTO_NUM	32
#define CONFIG_ALARM_NUM	16
#define CONFIG_DETECT_NUM	16
#define CONFIG_RECORD_NUM	16
#define CONFIG_CAPTURE_NUM	16
#define CONFIG_PTZ_NUM		16
#define SETUP_SLEEP			0
#define QUERY_WAIT			500
#define CONFIG_ALARM_NUM_EX	32
#define CONFIG_CAPTURE_NUM_EX	32

#include "../dvr/dvrdevice/dvr2cfg.h"

#define Min(a,b) ((a)<(b)?(a):(b))
#define BITRHT(x,y) ((x)>>(y))

typedef struct
{
    fLogDataCallBack cbLogData;
    DWORD dwUser;
}QueryLogInfo; 

class CManager;


class CDevConfig  
{
public:
	CDevConfig(CManager *pManager);
	virtual ~CDevConfig();

public:
	
	/* 初始化 */
	int		Init(void);

	/* 清理 */
	int		Uninit(void);

	int		CloseChannelOfDevice(afk_device_s* device);


	/* 获取设备配置信息（老的） */
	int		QueryConfig(LONG lLoginID, int nConfigType, int nParam,
                                   char *pConfigbuf, int maxlen, int *nConfigbuflen, 
								   int waittime);

	/* 设置设备配置信息（老的） */
	int		SetupConfig(LONG lLoginID, int nConfigType, int nParam, 
                                   char *pConfigbuf, int nConfigbuflen, int waittime);

	/* 获取通道名字 */
	int		QueryChannelName(LONG lLoginID, 
                                        char *pChannelName, int maxlen, int *nChannelCount, int waittime);

	/* 回调方式查询日志 */
	int		QueryLogCallback(LONG lLoginID, fLogDataCallBack cbLogData, DWORD dwUser);

	/* 同步方式查询日志 */
	int		QueryLog(LONG lLoginID, char *pLogBuffer, int maxlen, int *nLogBufferlen, void* reserved, int waittime, int type = 0);
	
	/* 查询录象控制状态 */
	int		QueryRecordState(LONG lLoginID, char *pRSBuffer, int maxlen, 
                                        int *nRSBufferlen, int waittime);
	/* 设置录象控制状态 */
	int		SetupRecordState(LONG lLoginID, char *pRSBuffer, int nRSBufferlen);
	
	/* 查询设备时间 */
	int		QueryDeviceTime(LONG lLoginID, LPNET_TIME pDeviceTime, int waittime);

	/* 设置设备时间 */
	int		SetupDeviceTime(LONG lLoginID, LPNET_TIME pDeviceTime);

	/* 查询串口协议列表、掩码 */
	int		QueryComProtocol(LONG lLoginID, int nProtocolType, char *pProtocolBuffer, 
                               int maxlen, int *nProtocollen, int waittime,DWORD dwProtocolIndex =0);

	/* 查询设备的系统信息 */
	int		QuerySystemInfo(LONG lLoginID, int nSystemType, 
                              char *pSysInfoBuffer, int maxlen, int *nSysInfolen, int waittime);

	/* 查询设备用户信息 */
	int		QueryUserInfo(LONG lLoginID, USER_MANAGE_INFO *info, int waittime);
	int		QueryUserInfoEx(LONG lLoginID, USER_MANAGE_INFO_EX *info, int waittime);
	/* 操作（添加、修改、删除）设备用户信息 */
	int		OperateUserInfo(LONG lLoginID, int nOperateType, void *opParam, void *subParam, int waittime);
	int		OperateUserInfoEx(LONG lLoginID, int nOperateType, void *opParam, void *subParam, int waittime);
	/* 查询IO端口（报警输入、输出等）使能状态 */
	int		QueryIOControlState(LONG lLoginID, IO_CTRL_TYPE emType, 
                                  void *pState, int maxlen, int *nIOCount, int waittime);

	/* 设置IO端口（报警输入、输出等）使能状态 */
	int		IOControl(LONG lLoginID, IO_CTRL_TYPE emType, void *pState, int maxlen, int waittime = 1000);

	/* 获取设备的工作状态 */
	int		GetDEVWorkState(LONG lLoginID, LPNET_DEV_WORKSTATE lpWorkState, int waittime);

	/* 新的获取设备系统配置数据 */
	int		GetDevConfig(LONG lLoginID, DWORD dwCommand,LONG lChannel, LPVOID lpOutBuffer,DWORD dwOutBufferSize,LPDWORD lpBytesReturned,int waittime);
	/*  各功能子函数 */
	int		GetDevConfig_DevCfg(LONG lLoginID, DEV_SYSTEM_ATTR_CFG &stSAC, int waittime);
	int		GetDevConfig_NetCfg(LONG lLoginID, DEVICE_NET_CFG &stNC, int waittime);
	int		GetDevConfig_ChnCfg(LONG lLoginID, DEV_CHANNEL_CFG &stCHC, LONG lChannel, int waittime);
	int		GetDevConfig_AllChnCfg(const LONG lLoginID, DEV_CHANNEL_CFG *pstCHC, LONG lChnNum, int waittime);
	int		GetDevConfig_RecCfg(LONG lLoginID, DEV_RECORD_CFG &stREC, LONG lChannel, int waittime);
	int		GetDevConfig_RecCfgNew(LONG lLoginID, DEV_RECORD_CFG &stREC, LONG lChannel, int waittime);
	int		GetDevConfig_AllRecCfg(LONG lLoginID, DEV_RECORD_CFG *pstREC, LONG lChnNum, int waittime);
	int		GetDevConfig_AllRecCfgNew(LONG lLoginID, DEV_RECORD_CFG *pstREC, LONG lChnNum, int waittime);
	int		GetDevConfig_ComCfg(LONG lLoginID, DEV_COMM_CFG &stCMC, int waittime);
	int		GetDevConfig_PtzCfg(LONG lLoginID, PTZ_OPT_ATTR *stuPtzAttr,DWORD dwProtocolIndex,int waittime);
	int		GetDevConfig_AlmCfg(LONG lLoginID, DEV_ALARM_SCHEDULE &stAS, int waittime);
	int		GetDevConfig_Devtime(LONG lLoginID, NET_TIME &stNT, int waittime);
	int		GetDevConfig_PreviewCfg(LONG lLoginID, DEV_PREVIEW_CFG &stPVC, int waittime);
	int		GetDevConfig_AutoMtCfg(LONG lLoginID, DEV_AUTOMT_CFG &stAMT, int waittime);
	int		GetDevConfig_VdMtrxCfg(LONG lLoginID, DEV_VIDEO_MATRIX_CFG *pstMatrix, int waittime);
	int		GetDevConfig_MultiDdnsCfg(LONG lLoginID, DEV_MULTI_DDNS_CFG &stMD, int waittime);
	int		GetDevConfig_SnapCfg(LONG lLoginID, DEVICE_SNAP_CFG *pstSC, LONG lChnNum, int waittime);
	int		GetDevConfig_UrlCfg(LONG lLoginID, DEVWEB_URL_CFG &stUC, int waittime);
	int		GetDevConfig_FtpCfg(LONG lLoginID, DEVICE_FTP_PROTO_CFG &stFC, int waittime);
	int		GetDevConfig_InterVedioCfg(LONG lLoginID, char* pCfg, int cfgLen, LONG lPlatform, int waittime);
	int		GetDevConfig_AllChnCoverCfg(const LONG lLoginID, DEV_VIDEOCOVER_CFG *lpChnCoverCfg, LONG lChnNum, int waittime);
	int		GetDevConfig_ChnCoverCfg(LONG lLoginID, DEV_VIDEOCOVER_CFG &ChnCoverCfg, LONG lChannel, int waittime);
	int		GetDevConfig_TransStrategyCfg(LONG lLoginID, DEV_TRANSFER_STRATEGY_CFG &stTSC, int waittime);
	int		GetDevConfig_DownloadStrategyCfg(LONG lLoginID, DEV_DOWNLOAD_STRATEGY_CFG &stDSC, int waittime);
	int		GetDevConfig_AllWaterMakeCfg(LONG lLoginID, DEVICE_WATERMAKE_CFG *pstCHC, LONG lChnNum, int waittime);
	int		GetDevConfig_WaterMakeCfg(LONG lLoginID, DEVICE_WATERMAKE_CFG *pstCHC, LONG lChannel, int waittime);
	int		GetDevConfig_EthernetCfg(LONG lLoginID, CONFIG_ETHERNET *pEthernetInfo, int &nEthernetNum, int waittime);
	int		GetDevConfig_WLANCfg(LONG lLoginID, DEV_WLAN_INFO &stuWlanInfo, int waittime);
	int		GetDevConfig_WLANDevCfg(LONG lLoginID, DEV_WLAN_DEVICE_LIST *pWlanLst, int waittime);
	int		GetDevConfig_AutoRegisterCfg(LONG lLoginID, DEV_REGISTER_SERVER *pRegisterSev, int waittime);
	int		GetDevConfig_AllChnCmrCfg(LONG lLoginID, DEVICE_CAMERA_CFG *pstCHC, LONG lChnNum, int waittime);
	int		GetDevConfig_ChnCmrCfg(LONG lLoginID, DEVICE_CAMERA_CFG &stCHC, LONG lChannel, int waittime);
	int		GetDevConfig_InfraredCfg(LONG lLoginID, INFRARED_CFG* pInfraredAlarm, int waittime);
	int		GetDevConfig_SnifferCfg(LONG lLoginID, DEVICE_SNIFFER_CFG *pstSniffer, int waittime);
	int		GetDevConfig_MailCfg(LONG lLoginID, DEVICE_MAIL_CFG *pstMail, int waittime);
	int		GetDevConfig_EtherDHCPCfg(LONG lLoginID, CONFIG_ETHERNET *pEthernetInfo, int waittime);
	int		GetDevConfig_DNSCfg(LONG lLoginID, DEVICE_DNS_CFG *pstDNS, int waittime);
	int		GetDevConfig_NTPCfg(LONG lLoginID, DEVICE_NTP_CFG *pstNTP, int waittime);
	int		GetDevConfig_AudioDetectCfg(LONG lLoginID, AUDIO_DETECT_CFG* pAudioDetectAlarm, int waittime);
	int		GetDevConfig_StorageStateCfg(LONG lLoginID, STORAGE_STATION_CFG &stuStorageCHA, LONG lChannel, int waittime);
	int		GetDevConfig_AllStorageStateCfg(LONG lLoginID, STORAGE_STATION_CFG *pstStorageCHC, LONG lChnNum, int waittime);
	int		GetDevConfig_DSTCfg(LONG lLoginID, DEVICE_DST_CFG *pstDSTInfo, int waittime);
	int		GetDevConfig_AlarmCenterCfg(LONG lLoginID, ALARMCENTER_UP_CFG *pAlarmCenterCfg, int waittime);
	int		GetDevConfig_VideoOSDCfg(LONG lLoginID, DVR_VIDEO_OSD_CFG &pstVideoOSD, LONG lChannel, int waittime);
	int     GetDevConfig_AllVideoOSDCfg(LONG lLoginID, DVR_VIDEO_OSD_CFG *pstVideoOSD, LONG lChnNum, int waittime);
	int     GetDevConfig_GPRSCDMACfg(LONG lLoginID, DEVICE_CDMAGPRS_CFG* pstGPRSCDMAInfo, int waittime);

	int     GetDevConfig_PosCfg( LONG lLoginID , DEVICE_POS_INFO* pPosCfg , int waittime ) ;
	int     SetDevConfig_PosCfg( LONG lLoginID , DEVICE_POS_INFO* pPosCfg , int waittime ) ;


	/* 新的设置设备系统配置数据 */
	int		SetDevConfig(LONG lLoginID, DWORD dwCommand,LONG lChannel, LPVOID lpInBuffer,DWORD dwInBufferSize,int waittime);
	/*  各功能子函数 */
	int		SetDevConfig_DevCfg(LONG lLoginID, DEV_SYSTEM_ATTR_CFG  *pstSAC, int waittime);
	int		SetDevConfig_NetCfg(LONG lLoginID, DEVICE_NET_CFG *pstNC, int waittime);
	int		SetDevConfig_ChnCfg(LONG lLoginID, DEV_CHANNEL_CFG *pstCHC, LONG lChannel, int waittime);
	int		SetDevConfig_AllChnCfg(LONG lLoginID, DEV_CHANNEL_CFG *pstCHC, LONG lChnNum, int waittime);
	int		SetDevConfig_RecCfg(LONG lLoginID, DEV_RECORD_CFG *pstREC, LONG lChannel, int waittime);
	int		SetDevConfig_RecCfgNew(LONG lLoginID, DEV_RECORD_CFG *stREC, LONG lChannel, int waittime);
	int		SetDevConfig_AllRecCfg(LONG lLoginID, DEV_RECORD_CFG *pstREC, int waittime);
	int		SetDevConfig_ComCfg(LONG lLoginID, DEV_COMM_CFG *pstCMC, int waittime);
	int		SetDevConfig_AlmCfg(LONG lLoginID, DEV_ALARM_SCHEDULE *pstAS, int waittime);
	int		SetDevConfig_Devtime(LONG lLoginID, NET_TIME *pstNT, int waittime);
	int		SetDevConfig_PreviewCfg(LONG lLoginID, DEV_PREVIEW_CFG *pstPVC, int waittime);
	int		SetDevConfig_AutoMtCfg(LONG lLoginID, DEV_AUTOMT_CFG *pstAMT, int waittime);
	int		SetDevConfig_VdMtrxCfg(LONG lLoginID, DEV_VIDEO_MATRIX_CFG *pstVM, int waittime);
	int		SetDevConfig_MultiDdnsCfg(LONG lLoginID, DEV_MULTI_DDNS_CFG *pstMD, int waittime);
	int		SetDevConfig_SnapCfg(LONG lLoginID, DEVICE_SNAP_CFG *pstSC, LONG lChnNum, int waittime);
	int		SetDevConfig_UrlCfg(LONG lLoginID, DEVWEB_URL_CFG *pstUC, int waittime);
	int		SetDevConfig_FtpCfg(LONG lLoginID, DEVICE_FTP_PROTO_CFG *pstFC, int waittime);
	int		SetDevConfig_InterVedioCfg(LONG lLoginID, char* pCfg, int cfgLen, LONG lPlatform, int waittime);
	int		SetDevConfig_ChnCoverCfg(LONG lLoginID, DEV_VIDEOCOVER_CFG *pChnCoverCfg, LONG lChannel, int waittime);
	int		SetDevConfig_TransStrategyCfg(LONG lLoginID, DEV_TRANSFER_STRATEGY_CFG *pTransStrategyCfg, int waittime);
	int		SetDevConfig_DownloadStrategyCfg(LONG lLoginID, DEV_DOWNLOAD_STRATEGY_CFG *pDownloadStrategyCfg, int waittime);
	int		SetDevConfig_AllWaterMakeCfg(LONG lLoginID, DEVICE_WATERMAKE_CFG *pstCHC, LONG lChnNum, int waittime);
	int		SetDevConfig_WaterMakeCfg(LONG lLoginID, DEVICE_WATERMAKE_CFG *pstCHC, LONG lChannel, int waittime);
	int		SetDevConfig_EthernetCfg(LONG lLoginID, CONFIG_ETHERNET *pEthernetInfo, int nEthernetNum, int waittime);
	int		SetDevConfig_WLANCfg(LONG lLoginID, DEV_WLAN_INFO *pWlanInfo, int waittime);
	int		SetDevConfig_WLANDevCfg(LONG lLoginID, DEV_WLAN_DEVICE *pWlanDevInfo, int waittime);
	int		SetDevConfig_AutoRegisterCfg(LONG lLoginID, DEV_REGISTER_SERVER *pRegiSevInfo, int waittime);
	int		SetDevConfig_ChnCmrCfg(LONG lLoginID, DEVICE_CAMERA_CFG *pstCHC, LONG lChannel, int waittime);
	int		SetDevConfig_InfraredCfg(LONG lLoginID, INFRARED_CFG *pInfrared, int waittime);
	int		SetDevConfig_SnifferCfg(LONG lLoginID, DEVICE_SNIFFER_CFG *pstSniffer, int waittime);
	int		SetDevConfig_EtherDHCPCfg(LONG lLoginID, CONFIG_ETHERNET *pEthernetInfo, int nEthernetNum, int waittime);
	int		SetDevConfig_DNSCfg(LONG lLoginID, DEVICE_DNS_CFG *pstDNS, int waittime);
	int		SetDevConfig_NTPCfg(LONG lLoginID, DEVICE_NTP_CFG *pstNTP, int waittime);
	int		SetDevConfig_AudioDetectCfg(LONG lLoginID, AUDIO_DETECT_CFG* pAudioDetectAlarm, int waittime);
	int		SetDevConfig_StorageStateCfg(LONG lLoginID, STORAGE_STATION_CFG *pstStorageCHC, LONG lChannel, int waittime);
	int		SetDevConfig_DSTCfg(LONG lLoginID, DEVICE_DST_CFG *pstDSTInfo, int waittime);
	int		SetDevConfig_AlarmCenterCfg(LONG lLoginID, ALARMCENTER_UP_CFG *pAlarmCenterCfg, int waittime);
	int		SetDevConfig_VideoOSDCfg(LONG lLoginID, DVR_VIDEO_OSD_CFG *pstVideoOSD, LONG lChannel, int waittime); 
    int     SetDevConfig_GPRSCDMACfg(LONG lLoginID, DEVICE_CDMAGPRS_CFG* pstGPRSCDMAInfo, int waittime);

	//以下是新的报警配置协议(对应dvr版本LB2.42基线以后)
	//linjianyan 2007-7-17
	int		GetDevConfig_AlmCfgNew(LONG lLoginID, DEV_ALARM_SCHEDULE &stAs, int waittime);

	void	GetAlmActionMsk(CONFIG_TYPES almType, DWORD *dwMsk);
	void	GetAlmActionFlag(EVENT_HANDLER hEvent, DWORD *dwFlag);
	int		GetDevConfig_AlmCfgLocalAlarm(LONG lLoginID, ALARM_IN_CFG *pAlmCfg, int waittime);
	int		GetDevConfig_AlmCfgNetAlarm(LONG lLoginID, ALARM_IN_CFG *pAlmCfg, int waittime);
	int		GetDevConfig_AlmCfgMotion(LONG lLoginID, MOTION_DETECT_CFG *pMtnCfg, int waittime);
	int		GetDevConfig_AlmCfgLoss(LONG lLoginID, VIDEO_LOST_ALARM_CFG *pLosCfg, int waittime);
	int		GetDevConfig_AlmCfgBlind(LONG lLoginID, BLIND_ALARM_CFG *pBldCfg, int waittime);
	int		GetDevConfig_AlmCfgDisk(LONG lLoginID, DISK_ALARM_CFG *pDiskAlmCfg, int waittime);
	int		GetDevConfig_AlmCfgNetBroken(LONG lLoginID, NETBROKEN_ALARM_CFG *pNetBrkCfg, int waittime);
	
	int		SetDevConfig_AlmCfgNew(LONG lLoginID, DEV_ALARM_SCHEDULE &stAs, int waittime);
	
	void	SetAlmActionFlag(EVENT_HANDLER *pEvent, DWORD dwFlag);
	int		SetDevConfig_AlmCfgLocalAlarm(LONG lLoginID, ALARM_IN_CFG *pAlmCfg, int waittime);
	int		SetDevConfig_AlmCfgNetAlarm(LONG lLoginID, ALARM_IN_CFG *pAlmCfg, int waittime);
	int		SetDevConfig_AlmCfgMotion(LONG lLoginID, MOTION_DETECT_CFG *pMtnCfg, int waittime);
	int		SetDevConfig_AlmCfgLoss(LONG lLoginID, VIDEO_LOST_ALARM_CFG *pLosCfg, int waittime);
	int		SetDevConfig_AlmCfgBlind(LONG lLoginID, BLIND_ALARM_CFG *pBldCfg, int waittime);
	int		SetDevConfig_AlmCfgDisk(LONG lLoginID, DISK_ALARM_CFG *pDiskAlmCfg, int waittime);
	int		SetDevConfig_AlmCfgNetBroken(LONG lLoginID, NETBROKEN_ALARM_CFG *pNetBrkCfg, int waittime);
	
	int		GetDevConfig_WorkSheet(LONG lLoginID, CONFIG_WSHEET_TYPE sheetType, CONFIG_WORKSHEET *pWSheetCfg, int waittime, int nSheetNum=16, int nChannel = 0);
	int		SetDevConfig_WorkSheet(LONG lLoginID, CONFIG_WSHEET_TYPE sheetType, CONFIG_WORKSHEET *pWSheetCfg, int waittime, int nSheetNum=16, int nChannel = 0);
	//new  set mail cfg func
	int		SetDevConfig_MailCfg(LONG lLoginID, DEVICE_MAIL_CFG *pstMail ,int waittime);
	/* 设置设备图象通道名称（MB格式串） */
	int		SetupChannelName(LONG lLoginID,char *pbuf, int nbuflen);

	/* 主动查询设备状态，包括报警、硬盘、录象状态等。 */
	int		QueryDevState(LONG lLoginID, int nType, char *pBuf, int nBufLen, int *pRetLen, int waittime=1000);

	/* 设置通道字符叠加 */
	int		SetupChannelOsdString(LONG lLoginID, int nChannelNo, CHANNEL_OSDSTRING* struOsdString, int nbuflen);

	/* 获取各接入平台信息 */
	int		GetPlatFormInfo(LONG lLoginID, DWORD dwCommand, int nSubCommand, int nParam, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned,int waittime);

	/* 设置各接入平台信息 */
	int		SetPlatFormInfo(LONG lLoginID, DWORD dwCommand, int nSubCommand, int nParam, LPVOID lpInBuffer, DWORD dwInBufferSize, int waittime);

	/*  查询设备能力集 */
	int		GetDevFunctionInfo(LONG lLoginID, int nType, char *pBuf, int nBufLen, int *pRetLen, int waittime=1000);

	/* 获取设备支持的语音对讲格式列表 */
	int		GetDevTalkFormatList(LONG lLoginID, DEV_TALK_FORMAT_LIST *pstTalkFormat, int waittime=1000);

	int		QueryNetUserState(LONG lLoginID, DEV_TOTAL_NET_STATE *pstNetUserState, int waittime=1000);

	//解析设备功能能力级的子功能项
	void	GetFunctionMsk(int EnableType, const char *strSource ,DWORD *dwMsk, LONG lLogin, int waittime);
	

	/* 查询串口参数 */
	int		QueryTransComParams(LONG lLoginID, int TransComType, COMM_STATE* pCommState, int nWaitTime);

private:

	/* 日志查询能力集判断 */
	BOOL	SearchLogProtocol(LONG lLoginID, int waittime);

#ifdef WIN32
	/* 解析日志XML数据 */
	int		ParseLogXML(char *pXMLFile, char *pOutLogBuffer, int nMaxLen, int *nResultlen);

#endif
private:
	CManager*		m_pManager;
	//Begin: Add by li_deming(11517) 2008-2-20
//	DWORD	  m_dwSign;	//用来记载一些特殊信息：
						//0x0:初始值(无效值)
						//0x1:没有保活时间的多DDNS配置信息
						//0x2:有保活时间的多DDNS配置信息
						//0x3:
	//End:li_deming(11517)
	OS_EVENT					m_hRecEvent;	/* 接受数据同步事件 */
};

/* 根据设备协议打包主机信息（PPPOE、DDNS） */
int DecodeHostString(int iHostType, void *vHostStru, char *szSubParam, char *szOutBuf, int iMaxOutBufLen, LONG lLoginID);

/* 根据设备协议打包邮件配置 */
int DecodeMailCfg(void *lpMailCfg, char *pOutBuf, int iMaxOutBufLen);

/* 数值转换 波特率 <----> 下标 */
DWORD ChangeBaudRate(DWORD dwBaud);

/* 以下三个用于串口协议、掩码、用户接口结构的转换 */
void GetMaskedFuncName(char *buf, int buflen, DWORD dwMask, char *namelist, int maxnum, DWORD *getnum);
void GetMaskedFuncIndex(BYTE *des, BYTE src, DWORD dwMask);
void GetMaskedFuncIndex2Dev(BYTE *des, BYTE src, DWORD dwMask);



#endif // _DEVCONFIG_H_


