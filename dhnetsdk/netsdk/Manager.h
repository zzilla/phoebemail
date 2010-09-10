/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：管理类
* 摘　要：SDK是DVR功能对外开放的窗口，DVR拥有很多功能，采取外观设计模式是较好的选择。
*         对DVR功能进行归类，主要功能有：实时监视、远程回放、报警、语音对讲、查询、控制、配置等。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _MANAGER_H_
#define _MANAGER_H_

#include "netsdk.h"
#include "netsdktypes.h"

#ifdef WIN32
#include <Mmsystem.h>
#pragma comment( lib, "Winmm.lib")
#endif

struct afk_device_prob_s;
class CRealPlay;
class CSearchRecordAndPlayBack;
class CAlarmDeal;
class CTalk; 
class CDevConfig;
class CDevConfigEx;
class CDevControl;
class CRenderManager;
class CServerSet;
class CSnapPicture;
class CGPSSubcrible;
class CDecoderDevice;
class CAutoRegister;
class CDynamicThread;
class CDevNewConfig;


class CManager  
{
public:
	CManager();
	virtual ~CManager();

public:
	/* 初始化环境 */
	int							Init(fDisConnect cbDisConnect, DWORD dwUser);

	/* 清理环境 */
	int							Uninit();

	/* 登入设备 */
	LONG						Login_Dev(char *szDevIp, int nPort, char *szUser, char *szPassword, LPNET_DEVICEINFO lpDeviceInfo, int *pErrorCode);
	
	/* 登录设备的扩展接口 */
	LONG						Login_DevEx(char *szDevIp, int nPort, char *szUser, char *szPassword, int nSpecCap, void* pCapParam, LPNET_DEVICEINFO lpDeviceInfo, int *pErrorCode = 0);
	
	/* 登出设备 */
	int							Logout_Dev(LONG lLoginID);

	/* 获取设备码流统计，lPlayHandle为0时目标为整个设备 */
	LONG						GetStatiscFlux(LONG lLoginID, LONG lPlayHandle);

	/* 通过设备名字或序列号向DDNS服务器查询IP */
	BOOL						GetDVRIPByResolveSvr(char *szDevIp, int nPort, unsigned char *szDVRName, int nDVRNameLen, unsigned char *szDVRSerialNumber, int nDVRSerialLen, char *szGetIp);

	/* 开始侦听设备 */
	LONG						ListenServer(char *szIp, int nPort, fServiceCallBack cbListen, DWORD dwUserData);

	/* 停止侦听设备 */
	BOOL						StopListenServer(LONG lServerHandle);
	
	/* 响应设备的注册请求 */
	BOOL						ResponseDevReg(char *szDevSerial, char *szIp, int nPort, BOOL bAccept);

	/* 设置参数 */
	BOOL						SetDeviceMode(LONG lLoginID, EM_USEDEV_MODE emType, void* pValue);


public:
	/* 检查设备是否有效 */	
	int							IsDeviceValid(afk_device_s* device, int specFlag = 0);

	/* device资源生命计数减1 */
	void						EndDeviceUse(afk_device_s* device);

	/* 删除设备 */
	int							DeleteDevice(afk_device_s* device);

	/* 发送心跳包和断线检测 */
	void						AllSendHeartBeat();

	/* 执行断线重连任务 */
	void						AllDevExecuteTask();

	/* 异步接口超时检测 */
	void						AllAsynTimeoutDetect();
	
	/* 删除报警数据 */
	void						DelAlarmData(AFK_ALARM_DATA *pstAlarmData);

	/* 处理断线信息回调 */
	void						DealDisconnCallback();

	/* 处理报警数据回调 */
	void						DealAlarmDataCallback();

	/* 处理主动注册连接回调 */
	void						DealAutoRegiCallback();

	/* 处理主动注册断线回调 */
	void						DealAutoRegiDisCallback();

	/* 处理异步信息回调 */
	void						DealResponseDataCallback();

	/* 处理NVD进度回调 */
	void						DealNVDPosCallback();

	/* 清理断线连接资源 */
	void						DealCleanResource();

	/* 主动注册侦听处理 */
	void						DealListenServer(void* pServerHandle, char *szClientIp, int nClientPort, int nCommand, void *pParam);
	
	/* 清空报警队列 */
	int							ClearAlarm(afk_device_s* device);

	/* 生成唯一操作号 */
	DWORD						GetPacketSequence(void);

	/* 设备主动断开内部回调函数 */
	void						DeviceDisConnect(afk_device_s* device, afk_channel_s* channel, int nOnline, char *szIp, int nPort, int nInterfaceType);

	/* 设备事件内部回调函数 */
	void						DeviceEvent(afk_device_s* device, int nEventType, void *param);

	/* 用户权限 */
	int							AnalyzeAuthorityInfo(afk_device_s* device);


public:
	class SearchLEbyThreadID;
	friend class SearchLEbyThreadID;

	class SearchLSIbyHandle;
	friend class SearchLSIbyHandle;

	class SearchLCIbyIpPort;
	friend class SearchLCIbyIpPort;
	

public:
	/* 获取函数执行失败的原因 */
	DWORD						GetLastError(void);
	int							SetLastError(DWORD dwError);

	/* 获取实时监视模块对象 */
	CRealPlay&					GetRealPlay(void) { return *m_pRealPlayMdl; }					

	/* 获取远程回放模块对象 */
	CSearchRecordAndPlayBack&	GetPlayBack(void) { return *m_pPlayBackMdl; }

	/* 获取render管理器对象 */
	CRenderManager&				GetRenderManager(void) { return *m_pRenderManager; }

	/* 获取设备配置模块对象 */
	CDevConfig&					GetDevConfig(void) { return *m_pConfigMdl; }

	/* 获取设备配置模块模块 */
	CDevConfigEx&				GetDevConfigEx(void) { return *m_pConfigExMdl; }

	/* 获取设备字符串配置模块对象 */
	CDevNewConfig&				GetNewDevConfig(void) { return *m_pNewConfigMdl; }

	/* 获取设备控制模块对象 */
	CDevControl&				GetDevControl(void) { return *m_pControlMdl; }

	/* 获取报警处理模块对象 */
	CAlarmDeal&					GetAlarmDeal(void) { return *m_pAlarmMdl; }

	/* 获取语音对讲模块对象 */
	CTalk&						GetTalk(void) { return *m_pTalkMdl; }

	/* 获取报警服务器模块对象 */
	CServerSet&					GetAlarmServer(void) { return *m_pAlarmServerMdl; }

	/* 获取抓图功能模块对象 */
    CSnapPicture&               GetSnapPicture(void) { return *m_pSnapPicMdl; }

	/* 获取GPS功能模块对象 */
	CGPSSubcrible&              GetGPSSubcrible(void) { return *m_pGPSMdl; }

	/* 获取解码器模块对象 */
	CDecoderDevice&				GetDecoderDevice(void) { return *m_pNVDMdle; }

	/* 获取主动注册模块对象 */
	CAutoRegister&				GetAutoRegister(void) { return *m_pAutoRegiMdl; }

	CDynamicThread&				GetDynamicThread(void) { return *m_pDynamicThread; }

	/* 获取SDK接口互斥锁对象 */
	DEVMutex&					GetCS(void)	{ return m_csDevices; }

	/* 设置连接次数 */
	void						SetConnectTryNum(const int& ConnectTryNum) 
	{ 
		if(ConnectTryNum > 0)
		{
			m_nConnTryNum = ConnectTryNum; 
		}
	}

	/* 设置连接超时 */
	void						SetConnectTime(const int& ConnectTime) 
	{
		if(m_nLoginTimeout > 0)
		{
			m_nLoginTimeout = ConnectTime; 
		}
	}

	/* 设置网络参数 */
	void						SetNetParameter(NET_PARAM *pNetParam);

	/* 获取网络参数 */
	void						GetNetParameter(NET_PARAM *pNetParam);

	/* 设置断线重连成功回调函数 */
	void						SetAutoReconnCallBack(fHaveReConnect cbHaveReconn, DWORD dwUser)
	{
		m_pfReConnect = cbHaveReconn;
		m_dwHaveReconnUser = dwUser;
	}

	/* 设置子连接断线回调函数 */
	void						SetSubDisconnCallBack(fSubDisConnect cbSubDisconn, DWORD dwUser)
	{
		m_pfSubDisConnect = cbSubDisconn;
		m_dwSubDisconnUser = dwUser;
	}

	/* 设置消息回调函数 */
	void						SetMessCallBack(fMessCallBack cbMessage,DWORD dwUser)
	{
		m_pfMessCallBack = cbMessage;
		m_dwMessUser = dwUser;
	}

	/* 设置画面叠加回调函数 */
	void						SetDrawCallBack(fDrawCallBack cbDraw,const DWORD& dwUser)
	{
		m_pfDrawCallBack = cbDraw;
		m_dwDrawUser	 = dwUser;	
	}

	/* 获取重连回调函数 */
	fHaveReConnect&				GetHaveReconnFunc(void) { return m_pfReConnect; }

	/* 获取图像叠加回调函数 */
	fDrawCallBack&				GetDrawFunc(void) {	return m_pfDrawCallBack; }

	/* 获取画面叠加回调自定义参数 */
	DWORD						GetDrawCallBackUserData(void) { return m_dwDrawUser; }

	/* 获取报警队列 */
	std::list<AFK_ALARM_DATA*>&	GetAlarmDataList(void) { return	m_lstAlarmData; }

	/* 获取异步数据回调函数 */
	std::list<AFK_RESPONSE_DATA*>& GetResponseDataList(void) { return m_lstResponseData; }

	/* 获取报警队列线锁 */
	DEVMutex&					GetAlarmDataCS(void) { return m_csAlarmData; }

	DEVMutex&					GetASCS(void) { return m_csAlarmServer; }

	/* 获取异步数据回调线锁 */
	DEVMutex&					GetResponseDataCS(void) { return m_csResponse; }


private:
	/* 查询设备基本信息 */
	int							GetDeviceInfo(afk_device_s* device, NET_DEVICEINFO *pstDevInfo, int nWaitTime);

	/* 转换错误代码 内部 --> 外部 */
	int							GetLoginError(int nErrorCode);


public:
	afk_device_prob_s*			m_pDeviceProb;		/* 设备模块装载器 */
	
	OS_EVENT					m_hDisEvent;
	OS_EVENT					m_hExitThread;
	OS_EVENT					m_hAlarmDataEvent;
	OS_EVENT					m_hCleanResEvent;
	OS_EVENT					m_hAutoRegiEvent;
	OS_EVENT					m_hAutoRegiDisEvent;
	OS_EVENT					m_hResponseEvent;
	OS_EVENT					m_hNVDPlaybackEvent;
	OS_THREAD					m_hThread;
	OS_THREAD					m_hHeatBeatThread;
	OS_THREAD					m_hCleanResThread;


private:	/* 一下是类的私有变量 */
	LN_LIFECOUNT				m_nLifeCount;		/*生命周期计数变量*/

	fMessCallBack				m_pfMessCallBack;	/* SDK用户消息回调函数指针 */

	fDrawCallBack				m_pfDrawCallBack;	/* 图像叠加回调函数指针 */

	fDisConnect					m_pfDisConnect;		/* 设备断线回调函数指针 */

	fHaveReConnect				m_pfReConnect;		/* 设备断线重连成功回调函数指针 */

	fSubDisConnect				m_pfSubDisConnect;	/* 动态子连接断线回调函数指针 */

	CRealPlay*					m_pRealPlayMdl;		/* 实时监视功能模块 */
	CSearchRecordAndPlayBack*	m_pPlayBackMdl;		/* 远程回放功能模块 */
	CRenderManager*				m_pRenderManager;	/* Render资源管理器 */
	CAlarmDeal*					m_pAlarmMdl;		/* 报警功能模块 */
	CTalk*						m_pTalkMdl;			/* 语音对讲功能模块 */
	CDevConfig*					m_pConfigMdl;		/* 配置功能模块 */
	CDevConfigEx*				m_pConfigExMdl;		/* 扩展配置功能模块 */
	CDevControl*				m_pControlMdl;		/* 控制功能模块 */
	CServerSet*					m_pAlarmServerMdl;	/* 报警服务器功能模块 */
	CSnapPicture*               m_pSnapPicMdl;		/* 抓图功能模块 */ 
	CGPSSubcrible*              m_pGPSMdl;			/* GPS功能模块 */
	CDecoderDevice*				m_pNVDMdle;			/* NVD功能模块 */
	CAutoRegister*				m_pAutoRegiMdl;		/* 主动注册功能模块 */
	CDynamicThread*				m_pDynamicThread;
	CDevNewConfig*				m_pNewConfigMdl;	/* 字符串配置功能模块 */

	std::list<AFK_LAST_ERROR *>	m_lstLastError;		/* Last Error 列表 */
	DEVMutex						m_csError;

	std::list<afk_device_s*>	m_lstDevices;		/* 当前在线的设备列表 */
	DEVMutex						m_csDevices;

	std::list<AFK_DISCONN_INFO*> m_lstDisconnDev;	/* 断开连接的设备列表 */
	DEVMutex						m_csDisconn;

	std::list<AFK_ALARM_DATA*>	m_lstAlarmData;		/* 报警消息队列 */
	DEVMutex						m_csAlarmData;

	std::list<afk_device_s*>	m_lstCleanRes;		/* 清理连接的设备列表 */
	DEVMutex						m_csCleanRes;

	std::list<AFK_LISTER_DATA*>	m_lstListenHandle;	/* 侦听句柄列表 */
	DEVMutex						m_csListen;

	std::list<AFK_CLIENT_REGISTER_INFO*> m_lstCltRegInfo;/* 客户端注册信息队列 */
	DEVMutex						m_csCltRegInfo;
	
	std::list<AFK_CLIENT_DISCONN_INFO*>	m_lstCltDisconn;/* 客户端断线连接列表 */
	DEVMutex						m_csCltDisconn;

	std::list<AFK_RESPONSE_DATA*> m_lstResponseData;/* 异步数据回调列表 */
	DEVMutex						m_csResponse;

	std::list<AFK_NVD_PLAYBACK*> m_lstNVDPlayback;	/* NVD回放进度回调列表 */
	DEVMutex						m_csNVDPlayback;

	DEVMutex						m_csAlarmServer;

	DWORD						m_dwErrorCode;		/* 运行时错误代码 */

	DWORD						m_dwDrawUser;		/* 图像叠加回调用户参数 */
	
	DWORD						m_dwDisConnectUser;	/* 设备断线回调用户参数 */

	DWORD						m_dwHaveReconnUser;	/* 断线重连成功回调用户参数 */

	DWORD						m_dwSubDisconnUser;	/* 动态子连接断线回调用户参数 */

	DWORD						m_dwMessUser;		/* 用户消息回调自定义数据 */

	int							m_nLoginTimeout;	/* 等待登入返回包超时时间 */

	int							m_nConnTryNum;		/* 连接尝试次数 */

	int							m_nSubConnSpaceTime;/*子连接之间的等待时间*/

	int							m_nConnHostTime;	/* 连接主机时间 */

	int							m_nConnBufLen;		/* 每个连接接收缓冲大小 */

	int							m_nGetDevInfoTime;	/* 获取设备信息的超时时间*/

	int							m_nGetSubConnTime;	/* 获取子连接信息的超时时间 */
	
	LONG						m_dwPacketSequence;	/* 唯一操作流水号 */
};

class CManager::SearchLEbyThreadID
{
#ifdef WIN32

	DWORD m_dwID;
public:
	SearchLEbyThreadID(DWORD dwThreadID):m_dwID(dwThreadID){}

#else	//linux

	pthread_t m_dwID;
public:
	SearchLEbyThreadID(pthread_t dwThreadID):m_dwID(dwThreadID){}

#endif
	bool operator()(AFK_LAST_ERROR*& le)
	{
		return (m_dwID == (le?le->dwThreadID:0));
	}
};

class CManager::SearchLSIbyHandle
{
	LONG m_lHandle;
public:
	SearchLSIbyHandle(LONG lHandle):m_lHandle(lHandle){}
	
	bool operator()(AFK_LISTER_DATA*& ld)
	{
		return (m_lHandle == (LONG)(ld?ld->lsnHandle:0));
	}
};

class CManager::SearchLCIbyIpPort
{
	char *m_ip;
	WORD m_port;
public:
	SearchLCIbyIpPort(char *ip, WORD port):m_ip(ip), m_port(port){}
	
	bool operator()(AFK_CLIENT_NODE*& cn)
	{
		if (NULL == m_ip || NULL == cn->ip)
		{
			return false;
		}
		return (_stricmp(m_ip, cn->ip)==0)&&(m_port==cn->port);
	}
};


#endif // _MANAGER_H_

