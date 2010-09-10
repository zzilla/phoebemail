/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：设备类
* 摘　要：SDK与DVR之间采用二进制协议，每次交互采用“请求－应答”的方式，因此可以把
*         每个交互过程看成抽象通道，相关数据在抽象通道中传输，减少各个操作相互关联。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRDEVICE_H_
#define _DVRDEVICE_H_

#include "../StdAfx.h"
#include "dvrchannel.h"

class CTcpSocket;
class CUdpSocket;
class CMulticastSocket;


class CDvrDevice : public afk_device_s
{
public:
	CDvrDevice(afk_dvrdevice_info stuDeviceInfo, CTcpSocket *pTcpSocket,
				const char *szIp, int nPort, const char *szUser, const char *szPassword, 
				fDisconnectCallBack cbDisconnect, fDeviceEventCallBack cbDevEvent, void *userdata);
	virtual ~CDvrDevice();

public:
    /* 版本信息 */
    virtual char*	device_version();

    /* 制造商信息 */
    virtual char*	device_manufactory();

    /* 产品信息 */
    virtual char*	device_product();

    /* 关闭设备 */
    virtual int		device_close();
	
    /* 打开通道 */
    virtual void*	device_open_channel(int type, void *parm);

    /* 通道是否打开 */
    virtual BOOL	device_channel_is_opened(int type, void *parm);

	/* 得到通道 */
	CDvrChannel*	device_get_channel(int type, unsigned int param, unsigned int subparam = 0);

	/* 关闭通道 */
	void			device_remove_channel(CDvrChannel *pDvrChannel);
	
    /* 获取信息 */
    virtual int		device_get_info(int type, void *parm);

    /* 设置信息 */
    virtual int		device_set_info(int type, void *parm);


public:
	/* 流量统计 */
    void			startstatisc();
    void			statiscing(unsigned int nLen);
    unsigned int	statisc();

	/* 动态子连接和断线重连 */
	void*			CreateSubConn(afk_connect_param_t *pConnParam);
	void			DestroySubConn(int type, void *pSubSocket, int nConnectID);
	void			CleanDisConn();
	void			ExecuteAllTask();	// 处理断线重连成功后的一些操作

	
public:
	int				device_AddRef();
	int				device_DecRef();

	
public:
    char*			GetIp() { return m_szIp; }
    int				GetPort() { return m_nPort; }
    int				GetType() { return m_Deviceinfo.type; }
    char*			GetUser() { return m_szUser; }
    char*			GetPassword() { return m_szPassword; }
    CTcpSocket*		GetTcpSocket() { return m_pTcpSocket; } 
    int				GetVideoChannelCount() { return m_Deviceinfo.channelcount; }
    int				GetAlarminputCount() { return m_Deviceinfo.alarminputcount; }
    int				GetAlarmoutputCount() { return m_Deviceinfo.alarmoutputcount; }
    afk_dvrdevice_info GetDvrInfo() { return m_Deviceinfo; }
	void			SetDeviceInfo(int type, DWORD dwParm);
	fDisconnectCallBack		GetDisconnectFunc() { return m_pfDisConnect; }
	fDeviceEventCallBack	GetDevEventFunc() { return m_pfDevEvent; }
    void*			GetDisconnectUserdata() { return m_userdata; }


protected:
	/***************************连接列表**************************/
	// 主连接
	CTcpSocket*					m_pTcpSocket;

	// 子连接
	std::map<unsigned int, CTcpSocket*>	m_lstSubTcpSockets;
	std::map<unsigned int, CUdpSocket*>	m_lstSubUdpSockets;
	std::map<unsigned int, CMulticastSocket*> m_lstMulticastSockets;
	CReadWriteMutex				m_csSubTcp;

	// 删除连接列表，用单独线程删除连接，防止用户在回调里删除连接造成死锁
	std::queue<CTcpSocket*>		m_lstDisTcpSockets;
	std::queue<CUdpSocket*>		m_lstDisUdpSockets;
	std::queue<CMulticastSocket*> m_lsDistMultiSockets;
	CReadWriteMutex				m_csDisTcp;

    /**************************多通道列表*************************/
    // 视频通道列表
	std::map<unsigned int, CDvrChannel*> m_media_channel_list;
	CReadWriteMutex				m_csmedia;

	// 预览通道列表(唯一)
	std::map<unsigned int, CDvrChannel*> m_preview_channel_list;
	CReadWriteMutex				m_cspreview;

	// 下载通道列表
	std::map<unsigned int, CDvrChannel*> m_download_channel_list;
	CReadWriteMutex				m_csdownload;
	
    // 透明通道列表(共有两个，232与485)
	std::vector<CDvrChannel*>	m_trans_channel_list;
	CReadWriteMutex				m_cstrans;

    // 查询通道列表
	std::list<CDvrChannel*>		m_search_channel_list;
	CReadWriteMutex				m_cssearch;

    // 配置通道列表
	std::list<CDvrChannel*>		m_config_channel_list;
	CReadWriteMutex				m_csconfig;

	// 抓图通道列表
	std::vector<CDvrChannel*>	m_snap_channel_list;
	CReadWriteMutex				m_cssnap;

	// 请求通道列表
	std::list<CDvrChannel*>		m_request_channel_list;
	CReadWriteMutex				m_csRequest;

	// 新配置通道列表
	std::list<CDvrChannel*>		m_newconfig_channel_list;
	CReadWriteMutex				m_csnewconfig;

    /**************************单通道列表*************************/
	// 报警通道列表(唯一)
	CDvrChannel*				m_alarm_channel;
	CReadWriteMutex				m_csalarm;

	// 控制通道列表(唯一)
	CDvrChannel*				m_control_channel;
	CReadWriteMutex				m_cscontrol;

    // 升级通道列表(唯一)
	CDvrChannel*				m_upgrade_channel;
	CReadWriteMutex				m_csupgrade;

	// 上传配置文件
	CDvrChannel*				m_importcfg_channel;
	CReadWriteMutex				m_csimportcfg;

    // 统计通道列表(唯一)
	CDvrChannel*				m_statisc_channel;
	CReadWriteMutex				m_csstatisc;

    // 语音对讲通道列表(唯一)
	CDvrChannel*				m_audio_channel;
	CReadWriteMutex				m_csaudio;

    // 用户操作通道列表(唯一)
	CDvrChannel*				m_user_channel;
	CReadWriteMutex				m_csuser;

	// GPS通道列表(唯一)
    CDvrChannel*				m_gps_channel;
	CReadWriteMutex				m_csgps;


private:
	afk_dvrdevice_info			m_Deviceinfo;
    fDisconnectCallBack			m_pfDisConnect;
	fDeviceEventCallBack		m_pfDevEvent;
    void*						m_userdata;
    unsigned int				m_statisc;
    BOOL						m_enablestatisc;
    char						m_szIp[120];
    int							m_nPort;
    char						m_szUser[20];
    char						m_szPassword[20];
	LN_LIFECOUNT				m_lLifeCount;


private:
	DWORD			m_dwSpecial;		// 储存设备的一些实时特殊信息
	CReadWriteMutex m_csSpecial;
	int				m_nRecordProto;		// 录象查询特殊信息
	int				m_nLogProto;		// 日志查询特殊信息
	int				m_nProtocolVer;		// 协议版本号
	int				m_nIsMutiEther;		// 网卡支持情况，0：单网卡 1：多网卡(单网卡用之前旧的协议，后者用新的字符串协议)
	unsigned int	m_nSessionId;
	int				m_nTalkMode;		// 0：客户端方式；1：服务器方式
	int				m_nCardQuery;		// 0:不支持卡号查询，1：支持卡号查询
	int             m_nQuickConfig;     // 0:不支持快速读取配置， 1：支持快速读取配置
	int				m_nAlarmInCaps;		// 报警输入接口数
	DWORD			m_dwTalkFormat;		// 设备支持的语音对讲格式，按位表示；0：无头PCM，1：带DH头PCM，2：G711a，3：AMR，4：G711u，5：G726
	AUDIO_ATTR_T	m_stCurTalkFmt;		// 当前设置的语音对讲格式；-1：默认最高优先级，0：无头PCM，1：PCM，2：G711a，3：AMR，4：G711u，5：G726
	int				m_nAudioAlarmInCaps;// 音频报警输入个数
	DWORD			m_dwAlarmListenMode[32];// 报警订阅方式,按位表示：0：全部通道订阅，1－1024：具体通道的订阅
	DWORD			m_dwCurMotionAlarm[32];	// 当前动态检测报警状态
	DWORD			m_dwCurVideoLost[32];	// 当前视频丢失报警状态
	DWORD			m_dwCurShelter[32];		// 当前视频遮挡报警状态
	DWORD			m_dwCurInputAlarm[32];	// 当前外部输入报警状态
	int				m_nMultiPlayFlag;	// 多画面预览标志
	int				m_nIsHaveTask;		// 表示是否有任务
	DWORD			m_dwLastLoginPacket;
	DWORD			m_dwLastSubKeep;
	AUTO_CONN_STATUS m_emTaskStatus;
	std::map<unsigned int, CTcpSocket*>::iterator m_pCurSubTcpSocket;
	int				m_nImportantRecord; // (0：普通录像或者重要录像的第一次查询；1：重要录像的非第一次查询)
	CONFIG_ETHERNET m_stEthernet[5];    // 网卡信息
	
	// 设备配置能力
	DVR_AUTHORITY_INFO m_nAuthorityInfo;// 设备权限信息
	DEVICE_MESS_CALLBACK m_messCallFunc;// 异步回调函数
	DEVICE_MESS_CALLBACK m_nvdPlaybackCall;// 异步回调函数
	afk_login_device_type m_loginType;
};


#endif	// _DVRDEVICE_H_


