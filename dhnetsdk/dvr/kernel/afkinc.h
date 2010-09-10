
#ifndef AFKINC_H
#define AFKINC_H


/****************************************************************************
 * 通道类型 
 ****************************************************************************/
#define AFK_CHANNEL_TYPE_MEDIA		1		// 普通的实时媒体通道
#define AFK_CHANNEL_TYPE_SEARCH		2		// 查询通道
#define AFK_CHANNEL_TYPE_DOWNLOAD	3		// 下载通道
#define AFK_CHANNEL_TYPE_UPGRADE	4		// 在线更新通道
#define AFK_CHANNEL_TYPE_PREVIEW	5		// 预览通道
#define AFK_CHANNEL_TYPE_ALARM		6		// 报警数据通道
#define AFK_CHANNEL_TYPE_CONTROL    7       // 控制通道
#define AFK_CHANNEL_TYPE_CONFIG     8       // 配置通道
#define AFK_CHANNEL_TYPE_STATISC	9		// 统计信息通道
#define AFK_CHANNEL_TYPE_TRANS      10      // 透明通道
#define AFK_CHANNEL_TYPE_TALK       11      // 语音通道
#define AFK_CHANNEL_TYPE_USER       12      // 用户通道
#define AFK_CHANNEL_TYPE_DECODE		13		// 解码通道
#define AFK_CHANNEL_TYPE_RENDER		14		// 渲染通道
#define AFK_CHANNEL_TYPE_USERMSG	15		// 用户数据
#define AFK_CHANNEL_TYPE_DDNS		16		// DDNS查询通道
#define AFK_CHANNEL_TYPE_CAPTURE	17		// 远程抓图通道
#define AFK_CHANNEL_TYPE_GPS        18      // GPS通道，add by jinyx 2007/04/11
#define AFX_CHANNEL_TYPE_REQUEST	19		// 请求通道
#define AFX_CHANNEL_TYPE_NEWCONFIG  20		// 设备json字符串配置通道


/****************************************************************************
 * 查询通道的子类型
 ****************************************************************************/
#define AFK_CHANNEL_SEARCH_RECORD			0		// 查询录象
#define AFK_CHANNEL_SEARCH_LOG				1		// 查询设备日志
#define AFK_CHANNEL_SEARCH_CHANNELNAME		2		// 查询通道名
#define AFK_CHANNEL_SEARCH_ALARMCTRL_IN		3		// 查询报警输入布撤防
#define AFK_CHANNEL_SEARCH_ALARMCTRL_OUT	4		// 查询报警输出布撤防
#define AFK_CHANNEL_SEARCH_CONFIG			5		// 查询设备配置
#define AFK_CHANNEL_SEARCH_RECORDSTATE		6		// 查询录象状态
#define AFK_CHANNEL_SEARCH_DEVTIME			7		// 获取设备时间
#define AFK_CHANNEL_SEARCH_COMMPROTOCOL		8		// 获取串口协议
#define AFK_CHANNEL_SEARCH_DCDPROTOCOL		9		// 获取解码器协议
#define AFK_CHANNEL_SEARCH_SYSTEM_INFO		10		// 查询系统信息
#define AFK_CHENNEL_SEARCH_DEVWORKSTATE		11		// 查询设备工作状态
#define AFK_CHENNEL_SEARCH_DEVSTATE			12		// 查询设备状态，跟DEVWORKSTATE协议不一样
#define AFK_CHANNEL_SEARCH_PTZ				13		// 查询云台设备属性
#define AFK_CHANNEL_SEARCH_WIRELESS_ALARMOUT 14     // 查询无线报警输出
#define AFK_CHANNEL_SEARCH_ALARMTRRIGER_MODE 15		// 查询报警触发方式
#define AFK_CHANNEL_SEARCH_TRANSCOM			 16		// 查询串口参数

#define AFK_CHANNEL_SEARCH_DDNSIP_BYNAME	100		// 向ddns服务器查询ip


/****************************************************************************
 * 配置通道的子类型
 ****************************************************************************/
#define AFK_CHANNEL_CONFIG_RECORDSTATE	AFK_CHANNEL_SEARCH_RECORDSTATE
#define AFK_CHANNEL_CONFIG_ALARMCTRL	AFK_CHANNEL_SEARCH_ALARMCTRL_IN //IN&OUT
#define AFK_CHANNEL_CONFIG_CONFIG		AFK_CHANNEL_SEARCH_CONFIG
#define AFK_CHANNEL_CONFIG_DEVTIME		AFK_CHANNEL_SEARCH_DEVTIME
#define AFK_CHANNEL_CONFIG_ALARMTRRIGER_MODE	AFK_CHANNEL_SEARCH_ALARMTRRIGER_MODE
#define AFK_CHANNEL_CONFIG_LIMIT_FLUX		99		// 设置限码率
#define AFK_CHANNEL_CONFIG_ALARMDCD_OUT		100		// 报警解码器输出
#define AFK_CHANNEL_CONFIG_CHANNEL_OSD		101		// 通道字符叠加
#define AFK_CHANNEL_CONFIG_FORCE_IFRAME		102		// 强制I帧


/****************************************************************************
 * 下载通道的子类型
 ****************************************************************************/
#define AFK_CHANNEL_DOWNLOAD_RECORD			0		// 下载录象文件
#define AFK_CHANNEL_DOWNLOAD_CONFIGFILE		1		// 下载配置文件


/****************************************************************************
 * 上载通道的子类型
 ****************************************************************************/
#define AFK_CHANNEL_UPLOAD_UPGRADE			0		// 上载升级文件
#define AFK_CHANNEL_UPLOAD_CONFIGFILE		1		// 上载配置文件


/************************************************************************
 * 查询类型定义
 ************************************************************************/
#define AFK_SEARCH_TYPE_MEDIA		1		// 录像查询
#define AFK_SEARCH_TYPE_ALARM		2		// 报警查询
#define AFK_SEARCH_TYPE_LOG			3		// 日志查询

/************************************************************************
 * 控制类型定义
 ***********************************************************************/
#define AFK_CONTROL_TYPE_PTZ		1		// 云台控制
#define AFK_CONTROL_TYPE_YWPTZ		2		// 亿蛙球机控制
#define AFK_CONTROL_TYPE_RESET		3		// 重启控制
#define AFK_CONTROL_TYPE_DEVCONTROL	4		// 设备控制


/****************************************************************************
 * 请求通道类型
 ****************************************************************************/
#define AFK_REQUEST_DECODER_CTRLTV			0x00000001		// 解码器：控制TV画面
#define AFK_REQUEST_DECODER_SWITCH			0x00000002		// 解码器：切换TV图象
#define AFK_REQUEST_DEC_QUERY_TV			0x00000003		// 解码器：查询TV信息
#define AFK_REQUEST_DEC_QUERY_CHANNEL		0x00000004		// 解码器：查询解码通道信息
#define AFK_REQUEST_DEC_QUERY_SYS			0x00000005		// 解码器：查询解码器系统信息
#define AFK_REQUEST_DEC_QUERY_CHNNUM		0x00000006		// 解码器：查询解码器解码通道个数
#define AFK_REQUEST_DEC_ADD_COMBIN			0x00000007		// 解码器：增加TV画面组合
#define AFK_REQUEST_DEC_DEL_COMBIN			0x00000008		// 解码器：删除TV画面组合
#define AFK_REQUEST_DEC_SET_COMBIN			0x00000009		// 解码器：修改TV画面组合
#define AFK_REQUEST_DEC_QUERY_COMBIN		0x0000000A		// 解码器：查询TV画面组合
#define AFK_REQUEST_DEC_SET_TOUR			0x0000000B		// 解码器：设置解码器轮巡
#define AFK_REQUEST_DEC_QUERY_TOUR			0x0000000C		// 解码器：查询解码器轮巡
#define AFK_REQUEST_DEC_SET_TV_ENABLE		0x0000000D		// 解码器：设置解码器TV使能
#define AFK_REQUEST_DEC_PLAYBACKBYDEC		0x0000000E		// 解码器：回放前端设备
#define AFK_REQUEST_DEC_CTRLPLAYBACK		0x0000000F		// 解码器：回放控制
#define AFK_REQUEST_DEC_CHANNEL_STATE		0x00000010		// 解码器：查询解码通道流信息
#define AFK_REQUEST_SET_VIDEOOUT			0x00000011      // 解码器：设置视频输出参数
#define AFK_REQUEST_DEC_CONTROL				0x00000012      // 解码器：控制

#define AFK_REQUEST_GET_IPFILTER			0x00000101		// 查询IP地址过滤
#define AFK_REQUEST_SET_IPFILTER			0x00000102		// 修改IP地址过滤
#define AFK_REQUEST_ADD_IPFILTER			0x00000103		// 增加IP地址过滤
#define AFK_REQUEST_DEL_IPFILTER			0x00000104		// 删除IP地址过滤
#define AFK_REQUEST_GET_AMRENCODE_CFG		0x00000105      // 查询AMR编码配置
#define AFK_REQUEST_SET_AMRENCODE_CFG		0x00000106      // 设置AMR编码配置
#define AFK_REQUEST_GET_RECORDLEN_CFG		0x00000107      // 查询录像打包长度配置
#define AFK_REQUEST_SET_RECORDLEN_CFG		0x00000108      // 设置录像打包长度配置
#define AFK_REQUEST_AUTOREGISTER_CONNECT	0x00000109		// 主动注册重定向：连接
#define AFK_REQUEST_AUTOREGISTER_GETCONID	0x0000010A		// 主动注册重定向：获取主动注册服务器的连接ID
#define AFK_REQUEST_AUTOREGISTER_GETSERVER	0x0000010B		// 主动注册重定向：获取主动注册服务器的状态信息
#define AFK_REQUEST_AUTOREGISTER_REGSERVER	0x0000010C		// 主动注册重定向：主动注册
#define AFK_REQUEST_AUTOREGISTER_DISCONNECT	0x0000010D		// 主动注册重定向：断开连接
#define AFK_REQUEST_GET_VIDEOFORMAT_INFO	0x0000010E      // 查询设备支持的视频制式
#define AFK_REQUEST_GET_MMS_INFO			0x0000010F		// 查询MMS配置
#define AFK_REQUEST_SET_MMS_INFO			0x00000110		// 修改MMS配置
#define AFK_REQUEST_GET_SMSACTIVE_INFO		0x00000111	    // 查询SMSACTIVATION配置
#define AFK_REQUEST_SET_SMSACTIVE_INFO		0x00000112		// 修改SMSACTIVATION配置
#define AFK_REQUEST_GET_DIALIN_INFO			0x00000113		// 查询DIALINACTIVATION配置
#define AFK_REQUEST_SET_DIALIN_INFO			0x00000114		// 修改DIALINACTIVATION配置
#define AFK_REQUEST_CONNECT					0x00000115		// 建立连接
#define AFK_REQUEST_DISCONNECT				0x00000116		// 断开连接
#define AFK_REQUEST_GET_BURNFILE_TRANS		0x00000117		// 查询刻录附件文件传送
#define AFK_REQUEST_SET_BURNFILE_TRANS		0x00000118		// 设置刻录附件文件传送
#define AFK_REQUEST_GET_BURN_ATTACH			0x00000119		// 查询附件刻录使能
#define AFK_REQUEST_SET_BURN_ATTACH			0x0000011A		// 设置附件刻录使能
#define AFK_REQUEST_SET_SNIFFER				0x0000011B		// 修改信息
#define AFK_REQUEST_GET_SNIFFER				0x0000011C		// 查询信息
#define AFK_REQUEST_SET_DOWNLOAD_RATE		0x0000011D		// 设置下载速度限制
#define AFK_REQUEST_GET_DOWNLOAD_RATE		0x0000011E		// 查询下载速度限制
#define AFK_REQUEST_GET_POINT_CFG			0x0000011F		// 获取预制点配置
#define AFK_REQUEST_SET_POINT_CFG			0x00000120		// 设置预制点配置
#define AFK_REQUEST_GET_OSDENABLE_CFG		0x00000121		// 设置OSD叠加使能
#define AFK_REQUEST_SET_OSDENABLE_CFG		0x00000122		// 获取OSD叠加使能
#define AFK_REQUEST_LOGOUT					0x00000123		// 登出通知设备
#define AFK_REQUEST_GET_TVADJUST_CFG        0x00000124      // 获取TV调节参数
#define AFK_REQUEST_SET_TVADJUST_CFG        0x00000125      // 设置TV调节参数
#define AFK_REQUEST_FUNCTION_MASK			0x00000126		// 功能列表
#define AFK_REQUEST_GET_VEHICLE_CFG			0x00000127		// 获取车载配置
#define AFK_REQUEST_SET_VEHICLE_CFG			0x00000128		// 设置车载配置
#define AFK_REQUEST_GET_ATM_OVERLAY_ABILITY 0x00000129		// 获取atm叠加能力
#define AFK_REQUEST_GET_ATM_OVERLAY_CFG		0x0000012a		// 获取atm叠加配置
#define AFK_REQUEST_SET_ATM_OVERLAY_CFG		0x0000012b		// 设置atm叠加配置
#define AFK_REQUEST_QUERY_BACKUP_DEV		0x0000012c		// 查询备份设备列表
#define AFK_REQUEST_QUERY_BACKUP_DEV_INFO	0x0000012d		// 查询备份设备详细信息
#define AFK_REQUEST_BACKUP_START			0x0000012e		// 开始备份
#define AFK_REQUEST_BACKUP_STOP				0x0000012f		// 停止备份
#define AFK_REQUEST_BACKUP_FEEDBACK			0x00000130		// 备份进度反馈
#define AFK_REQUEST_ATM_TRADE_TYPE			0x00000131		// ATM设备交易类型
#define AFK_REQUEST_GET_DECODER_TOUR_CFG	0x00000132		// 获取解码轮巡配置
#define AFK_REQUEST_SET_DECODER_TOUR_CFG	0x00000133		// 设置解码轮巡配置	
#define AFK_REQUEST_GET_SIP_CFG				0x0000013c		// 获取SIP配置
#define AFK_REQUEST_SET_SIP_CFG				0x0000013d		// 设置SIP配置
#define AFK_REQUEST_GET_SIP_ABILITY			0x0000013e		// 获取SIP能力
#define AFK_REQUEST_GET_SIP_STATE			0x0000013f		// 获取SIP状态

#define AFK_REQUEST_GET_NEWCONFIG_CFG		0x00000134		// 获取设备配置（新的字符串协议）
#define AFK_REQUEST_SET_NEWCONFIG_CFG		0x00000135		// 设置设备配置（新的字符串协议）

#define AFK_REQUEST_GET_POS_CFG             0x00000140      // 获取POS参数
#define AFK_REQUEST_SET_POS_CFG             0x00000141      // 设置POS参数

/****************************************************************************
 * 请求通道子类型
 ****************************************************************************/
#define AFK_REQUEST_DEC_BYDECBYFILE			0x00000001		// 解码器：按文件回放前端设备
#define AFK_REQUEST_DEC_BYDECBYTIME			0x00000002		// 解码器：按时间回放前端设备


/************************************************************************
 * 句柄定义
 ***********************************************************************/
typedef void * afk_handle_t;
typedef struct afk_time_s afk_time_t;
typedef struct afk_channel_param_s afk_channel_param_t;
typedef struct afk_media_channel_param_s afk_media_channel_param_t;
typedef struct afk_search_channel_param_s afk_search_channel_param_t;
typedef struct afk_download_channel_param_s afk_download_channel_param_t;
typedef struct afk_upgrade_channel_param_s afk_upgrade_channel_param_t;
typedef struct afk_preview_channel_param_s afk_preview_channel_param_t;
typedef struct afk_alarm_channel_param_s afk_alarm_channel_param_t;
typedef struct afk_control_channel_param_s afk_control_channel_param_t;
typedef struct afk_config_channel_param_s afk_config_channel_param_t;
typedef struct afk_user_channel_param_s afk_user_channel_param_t;
typedef struct afk_alarm_trriger_mode_s afk_alarm_trriger_mode_t;


/************************************************************************
 * 库内部使用相关结构体定义
 ***********************************************************************/
struct afk_time_s
{
	int				year;			// 年
	int				month;			// 月
	int				day;			// 日
	int				hour;			// 小时
	int				minute;			// 分钟
	int				second;			// 秒
};

/* 设备时间信息 */
struct afk_devicetime_info_s
{
    int				year;
    int				month;
    int				day;
    int				hour;
    int				minute;
    int				second;
};

/* 查询文件条件信息 */
struct afk_query_record_s
{
    unsigned int	ch;				// 通道号
    unsigned int	type;			// 0:所有录像  1:外部报警 2:动态检测 3:所有报警 4:卡号
    afk_time_s		starttime;		// 开始时间
    afk_time_s		endtime;		// 结束时间
    char			cardid[255];	// 卡号
    bool			bytime;			// 按时间查询
};

/* 录像文件信息 */
struct afk_record_file_info_s
{
    unsigned int	ch;				// 通道号
    char			filename[128];	// 文件名
    unsigned int	size;			// 文件长度
    afk_time_s		starttime;		// 开始时间
    afk_time_s		endtime;		// 结束时间
    unsigned int	driveno;		// 磁盘号
    unsigned int	startcluster;	// 起始簇号
	unsigned char	nRecordFileType;// 录象文件类型
	unsigned char	ImportantRecID;	// 0:普通录像 1:重要录像
	unsigned char	Hint;			// 文件索引
	unsigned char   Reserved;		// 保留
};

/* 报警状态信息 */
struct afk_state_info_s
{   
	int				channelcount;
	int				alarminputcount;
	unsigned char	alarm[16];
	unsigned char	motiondection[16];
	unsigned char	videolost[16];
	unsigned char   PosTick[16];
};
    
/* 报警设置相关信息 */
struct afk_alarm_info_s
{
    bool			isinput;		// 是否报警输入
    unsigned int	count;			// 个数
    unsigned char*	state;			// size:count
	bool			bSupportTriggerMode;// 是否支持报警触发方式
};

/* 报警输出触发方式*/
struct afk_alarm_trriger_mode_info
{
	unsigned char	mode;			// 触发方式
};

struct afk_alarm_trriger_mode_s
{
	unsigned char	channel_count;	// 要设置的通道数
	afk_alarm_trriger_mode_info trriger_mode[16];// 触发方式
};

/* 报警控制信息 */
struct afk_alarm_control_info_s
{
//	bool			isinput;		// 是否报警输入
	unsigned char	type;			// 控制类型
    unsigned int	alarmid;		// 报警ID
    bool			controlflag;	// 报警控制标志
};

struct afk_decoder_alarm_info_s
{
	int				decoderNo;		// 报警解码器号
	unsigned short	alarmChn;		// 报警输出口
	unsigned short	alarmState;		// 报警输出状态
};

/* 配置相关信息 */
struct afk_config_info_s
{
    unsigned char*	configbuf;
    int				buflen;
};

/* 强制IA祯 */
struct afk_channel_getIframe_info_s
{
	unsigned char	subchn;			// 0x00 主码流1, 0x01 出辅码流2,0x02	出辅码流3,0x03 出辅码流4,0x04 出JPEG抓图
};

/* 透明串口发送信息 */
struct afk_trans_info_s
{
    char*			data;			// 数据指针
    int				datalen;		// 数据长度
};

/* 对讲发送信息 */ 
struct afk_talk_info_s
{
    char*			data;			// 数据指针
    int				datalen;		// 数据长度
};


/* set_info, get_info types. */
#define SP_USER_REUSABLE			0x01	// 是否支持用户复用
#define SP_NEW_CHANNAME				0x02	// 是否使用新的通道名协议
#define SP_NEW_RECORDPROTOCOL		0x03	// 是否支持新的录像及图片列表查询方式
#define SP_NEW_LOGPROTOCOL			0x04	// 是否支持新的日志格式查询
#define SP_NEW_MULTIDDNSPRO			0x08	// 是否支持带保活信息的多DDNS协议

typedef enum 
{
	dit_special_cap_flag = 0,	// 特殊能力标志,param = DWORD*　，按位表示
	dit_protocol_version,		// 协议版本号
	dit_talk_encode_type,		// 对讲编码类型
	dit_recordprotocol_type,	// 录象查询能力标志
	dit_logprotocol_type,		// 日志查询能力标志
	dit_session_id,				// 本次登录的session id
	// 与设备交互的信息
	dit_ether_flag,				// 网卡支持情况
	dit_talk_use_mode,			// 对讲使用模式(客户端和服务器)
	dit_card_query,				// 卡号查询
	dit_alarmin_num,			// 报警输入个数
	dit_talk_encode_list,		// 对讲编码列表
	dit_audio_alarmin_num,      // 音频检测报警输入个数
	dit_dev_type,				// 设备类型
	dit_alarm_listen_use_mode,	// 报警侦听的模式0-16
	dit_multiplay_flag,			// 多画面预览能力
	dit_heart_beat,				// 向设备发送心跳包
	dit_device_task,			// 是否有任务标志
	dit_execute_task,			// 处理任务
	dit_online_flag,			// 是否在线
	dit_config_authority_mode,  // 配置权限
	dit_user_name,              // 用户名
	dit_set_callback,			// 设置异步回调函数
	dit_asyn_timeout,			// 异步检测超时
//	dit_partition_ver,			// 分区协议版本号
//	dit_support_partition,		// 支持分区个数
	dit_firstqueryrecord_flag,	// 重要录象查询标识非第一次查询，param(0：普通录像或者重要录像的第一次查询；1：重要录像的非第一次查询)
	dit_nvd_playback_flag,		// 解码器回放进度回调
	dit_svr_alarm_flag,			// SVR报警状态
	dit_ether_info,             // 网卡信息，包括网卡名称的记录
	dit_login_device_flag,		// 设置登入方式
	dit_quick_config,           // 是否支持快速读取配置
	dit_cleanres_flag,			// 清理连接资源
} afk_device_info_type;

typedef struct
{
    int			type;
    int			channelcount;
    int			alarminputcount;
    int			alarmoutputcount;
    int			version;
    BOOL		ispal;
    BOOL		istrans;
    int			enctype;		// 8:mpeg4, 9:H.264
    BOOL		istalk;			// 对讲
    BOOL		ispreview;		// 多画面预览
	char		szDevType[256]; // 设备类型
	BOOL		bAutoRegister;	// 是否主动注册登入
} afk_dvrdevice_info;

typedef enum
{
	EM_SEND_LOGINPACKET,
	EM_WAIT_LOGINACK,
	EM_PARSE_LOGINACK,
	EM_RECONN_SUBCONN,
	EM_WAIT_SUBCONN,
	EM_SEND_REALPACKET,
	EM_DYNAMIC_SUBCONN,
} AUTO_CONN_STATUS;

typedef enum 
{
	channel_request_param = 0,
	channel_detect_timeout,
	channel_protocoltype,
	channel_packetsequence,
} afk_channel_info_type;

typedef enum
{
	channel_connect_tcp = 0,
	channel_connect_udp,
	channel_connect_multicast,
} afk_channel_connect;

typedef struct  
{
	char		szRemoteIp[32];
	int			nRemotePort;
	int			nLoginMode;
} afk_login_device_type;



/************************************************************************
 * 打开通道的参数定义, 跟通道类型一一对应
 ************************************************************************/
typedef int (__stdcall *afk_callback)(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata
);

struct afk_channel_param_s
{
	afk_callback		func;			// 回调函数
	void*				udata;			// 用户数据
};

struct afk_connect_param_t 
{
	char				szSevIp[32];
	char				szLocalIp[32];
	int					nSevPort;
	int					nLocalPort;
	unsigned int		nConnectID;
	int					nConnBufSize;
	int					nConnTime;
	int					nTryTime;
	int					nConnType;
	int					nInterfaceType;
	int					nParam;
	void*				userdata;
};

struct afk_media_channel_param_s
{
	afk_channel_param_t base;			// 基本参数
	afk_connect_param_t conn;			// 连接信息
	int					no;				// 通道号
	int					subtype;		// 主辅码流类型
};

struct afk_preview_channel_param_s
{
	afk_channel_param_t	base;			// 基本参数
	afk_connect_param_t conn;			// 连接信息
	int					parm1;			// 预览参数1, 不同公司产品, 参数不一样
	int					parm2;			// 预览参数2, 不同公司产品, 参数不一样
};

struct afk_search_channel_param_s
{
	afk_channel_param_t base;	        // 基本参数
	int					no;	            // 通道号
    int					type;	        // 查询类型
    /* 0:查询录像 1:查询日志 2:查询通道名 3:查询报警布撤防输入 4:查询报警布撤防输出
		5:普通设置 6:COM口设置 7:网络设置 8:定时设置 9:图像设置 10:PTZ设置 11:动态检测设置 12:报警设置 */
	/* for DDNS only: 0：按名字查询IP，1：按序列号查询IP */
	int					subtype;		// 辅类型

	int					statetype;		// 查询报警、状态类型
	afk_query_record_s	queryrecord;	// 查询录像条件
	int					querydevstate;	// 查询设备状态 0 查询设备状态， 1 查询通道状态
	char*				webName;
	int					webNamelen;
	int					param;
};

struct afk_download_channel_param_s
{
	afk_channel_param_t	base;			// 基本参数
	afk_connect_param_t conn;			// 连接信息
	afk_record_file_info_s info;		// 录像文件信息
	int					nByTime;		// 按时间回放
	int					nParam;			// 扩展参数，0－回放，1－下载
	int					type;
	int					subtype;
};

struct afk_upgrade_channel_param_s
{
	afk_channel_param_t base;			// 基本参数
	char				filename[256];	// 上传的文件路径
    int					filetype;		// 上传的文件类型
    int					size;			// 上传的文件大小
	int					type;
	int					subtype;
};

struct afk_alarm_channel_param_s
{
    afk_channel_param_t	base;			// 基本参数
	int					parm;			// 区分不同协议的报警监听
};

struct afk_raw_channel_param_s
{
    afk_channel_param_t base;			// 基本参数
};

/* 通道名称信息 */
struct afk_channelname_info_s
{
    char				channelname[16][16];
};

struct afk_config_channel_param_s 
{
    afk_channel_param_t	base;			// 基本参数
    int					no;				// 通道号
    int					type;			// 查询类型
    /* 0:通道名 1:报警布撤防 2:普通设置 3:COM口设置 4:网络设置 5:定时设置
		6:图像设置 7:PTZ设置 8:动态检测设置 9:报警设置 */
	int					subtype;		// 辅类型
	
	afk_channelname_info_s	chnameinfo;	// 通道名参数
    afk_alarm_control_info_s alarminfo;	// 布撤防参数
	afk_decoder_alarm_info_s decoderalarm; // 报警解码器输出
    afk_config_info_s	configinfo;		// 配置参数
    afk_devicetime_info_s	devicetimeinfo; // 设备时间参数
	afk_channel_getIframe_info_s chngetIframe; // 强制I祯
	unsigned short		limitflux;		// 限制码流
	int					param[10];
	afk_alarm_trriger_mode_t alarmtrrigermode; // 报警输出触发方式
};

struct afk_control_channel_param_s
{
	afk_channel_param_t	base;			// 基本参数
    int					no;				// 通道号
    int					type1;			// 控制类型1
    int					type2;			// 控制类型2
    int					parm1;			// 控制参数1
    int					parm2;			// 控制参数2
    int					parm3;			// 控制参数3
	char*				pParm4;			// 控制参数4
};

struct afk_trans_channel_param_s
{
    afk_channel_param_t base;			// 基本参数
    unsigned char		trans_device_type; // 透明通道类型 0:串口 1:485
    unsigned char		baudrate;		// 波特率
    unsigned char		databits;		// 数据位
    unsigned char		stopbits;		// 停止位
    unsigned char		parity;			// 检验位
};

struct afk_download_control_param_s
{
    unsigned int		offsettime;
    unsigned int		offsetdata;
};

struct afk_talk_channel_param_s
{
    afk_channel_param_t base;			// 基本参数
    int					no;				// 通道号
	int					nEncodeType;	// 编码格式
	int					nAudioBit;		// 位数
	unsigned int		dwSampleRate;	// 采样率
};

struct afk_user_channel_param_s
{
    afk_channel_param_t base;			// 基本参数
    int					type;			// 用户操作类型
    char*				userbuf;		// 数据指针
    int					buflen;			// 数据长度
};

struct afk_usermsg_channel_s
{
	afk_channel_param_t	base;			// 基本参数
	char*				msg;			// 消息
	int					msglen;			// 消息长度
};

/* 抓图参数 */
struct afk_capture_channel_param_s
{
	afk_channel_param_t	base;			// 基本参数
	int					channo;			// 通道号
	int					pipeno;			// 流水号
	int					parm;			// 自定义参数
};

struct afk_device_base_s
{
	unsigned long		type;
	afk_callback		func;			// 回调函数
	void*				udata;			// 用户数据
};

struct afk_remote_control_s
{
	afk_device_base_s	base;
};

struct afk_cb_media_parm_s
{
	afk_handle_t		dev;			// 设备句柄
};


typedef void(__stdcall *SnapPictureRevCallback)(long lLoginID, unsigned char *pBuf, 
												unsigned int RevLen, unsigned int EncodeType,
												unsigned long CmdSerial, unsigned long dwUser);

struct afk_snap_channel_param_s
{
	afk_channel_param_t base;	        // 基本参数
	int					no;		        // 通道号
	int					subtype;		// 主从码流类型
	
	SnapPictureRevCallback snapcallback;// 抓图回调
	long				dwLoginID;		// 设备句柄,设置snapcallback的dwILoginID
	unsigned long		dwuser;			// 设置snapcallback的dwUser
};

typedef void (__stdcall *GPSRevCallback)(long ILongID,char strgps[],
										 long lenth,unsigned long dwuser);

struct afk_gps_channel_param_s
{
	afk_channel_param_t	base;			// 基本参数
	GPSRevCallback		callback;		// GPS回调
	long				LoginId;		// 设备句柄
	unsigned long		dwuser;			// 用户数据
    unsigned long		keeptime;		// 持续时间
	unsigned long		InterlTime;		// 发送间隔
	long				active;			// 是否开启
};

/* 请求通道参数 */
struct afk_request_channel_param 
{
	// 基本信息
	afk_channel_param_t	base;			// 内部回调，用于将数据填充到用户缓冲中
	int					nSequence;		// 请求序列号
	int					nType;			// 请求类型
	int					nSubType;		// 请求子类型
	void*				pCondition;		// 请求条件
	// 接收数据
	char*				pRecvBuf;		// 接收缓冲
	int					nBufLen;		// 缓冲长度
	// 异步信息
	void*				userparam;		// 用户参数
	void*				callbackFunc;	// 外部回调函数指针
	void*				userdata;		// 外部回调用户参数
	// 同步信息
	void*				pRecvEvent;		// 接收到数据通知事件
    int*				pResultCode;	// 返回码
	int*				pRetLen;		// 接收长度
};

/* 新配置通道参数 */
struct afk_newconfig_channel_param 
{
	// 基本信息
	afk_channel_param_t	base;			// 内部回调，用于将数据填充到用户缓冲中
	int					nSequence;		// 请求序列号
	int					nType;			// 请求类型
	int					nSubType;		// 请求子类型
	void*				pCondition;		// 请求条件
	// 接收数据
	char*				pRecvBuf;		// 接收缓冲
	int					nBufLen;		// 缓冲长度
	// 异步信息
	void*				userparam;		// 用户参数
	void*				callbackFunc;	// 外部回调函数指针
	void*				userdata;		// 外部回调用户参数
	// 同步信息
	void*				pRecvEvent;		// 接收到数据通知事件
    int*				pResultCode;	// 返回码
	int*				pDvrRestart;	// 设备重启标志
	int*				pRetLen;		// 接收长度
};



#endif /* AFKINC_H */




