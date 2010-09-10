///////////////////////////////////////////////////////////////////////////////////////////
// 文件名:		dvr2cfg.h
// 创建者:		赵均树
// 创建时间:	2006/09/18
// 内容描述:	亿蛙dvr设备配置信息结构
///////////////////////////////////////////////////////////////////////////////////////////    

#ifndef _DVR2CFG_H_
#define _DVR2CFG_H_

#include "../osIndependent.h"

#define N_SYS_CH 16
#define MAX_AUTHORITY_NUM 80


typedef enum tagProductType
{
	DVR_PRODUCT_H = 0,		//视豪
	DVR_PRODUCT_G,			//视通
	DVR_PRODUCT_L,			//视新
	DVR_PRODUCT_N,			//非实时
	DVR_PRODUCT_ATM,			//ATM
	NVS_PRODUCT_S,			//网络视频服务器
	NVS_PRODUCT_E,			//NVS 0404E
	DVR_PRODUCT_NB,			//二代非实时NB
	DVR_PRODUCT_LN,			//LN 系列
	DVR_PRODUCT_BAV,			//BAV 系列
	NVS_PRODUCT_B = 10,		//NVS_B
	NVS_PRODUCT_C,			//NVS_H
	NVD_PRODUCT_SERIAL = 12,	//解码器
	
	SDIP_PRODUCT = 50,		//球机
	IPC_PRODUCT,				//ipc
	IPC_PRODUCT_400 = 55,	//ipc4000
	IPC_PRODUCT_A6=60,		//A6
	N5_PRODUCT,				//N5
	MIX_DVR_PRODUCT,			//混合dvr
	SVR_PRODUCT,				//SVR
	NVR_PRODUCT = 65,		//NVR
	DEV_NORMAL_TYPE_PRODUCT=255		//设备类型
} PRODUCTTYPE;

typedef enum _config_type_t
{
	CONFIG_TYPE_GENERAL = 0,	/*普通配置*/
	CONFIG_TYPE_SERIAL_COMM,	/*串口配置*/
	CONFIG_TYPE_NET,			/*网络配置*/
	CONFIG_TYPE_RECORD,			/*定时录象配置*/
	CONFIG_TYPE_CAPTURE,		/*图像配置*/
	CONFIG_TYPE_PTZ,			/*云台步长*/
	CONFIG_TYPE_DETECT,			/*图像检测配置*/
	CONFIG_TYPE_ALARM,			/*报警配置*/
	CONFIG_TYPE_DISPLAY,		/*图像显示配置*/
								/*9 - /NULL*/
	CONFIG_TYPE_TITLE = 10,		/*通道名称，设备2.4版本后不再支持*/
	CONFIG_TYPE_MAIL,			/*邮件配置*/
	CONFIG_TYPE_PREVIEW,		/*预览配置*/
	CONFIG_TYPE_PPPOE,			/*PPPoE配置*/
	CONFIG_TYPE_DDNS,			/*DDNS配置*/
	CONFIG_TYPE_SNIFFER,		/*网络抓包配置*/
	CONFIG_TYPE_DSPCAP,			/*编码能力信息*/
//	CONFIG_TYPE_FTP,			/*FTP配置*/
	CONFIG_TYPE_AUTO_MT = 18,	/*自动维护配置*/
	CONFIG_TYPE_NTP,			/*NTP配置*/
	CONFIG_TYPE_LIMIT_BR,		/*限码流配置*/
	
	CONFIG_TYPE_WATERMAKE = 29,		/*图象水印配置*/
	CONFIG_TYPE_VIDEO_MATRIX = 30,	/*本地矩阵控制配置*/
	CONFIG_TYPE_COVER = 34,			/*视频区域遮挡*/
	CONFIG_TYPE_DHCP = 35,			/*DHCP配置*/
	CONFIG_TYPE_WEB_URL = 36,		/*抓图保存web路径配置*/
	CONFIG_TYPE_FTP_PRO = 37,		/*FTP配置*/
	CONFIG_TYPE_CAMERA = 38,		/*摄像头属性配置*/
	CONFIG_TYPE_ETHERNET = 39,		/*网卡配置*/
	CONFIG_TYPE_DNS = 40,			/*DNS服务器配置*/
	CONFIG_TYPE_STORSTATION = 41,   //存储位置
	CONFIG_TYPE_DOWNLOAD_STRATEGY = 42,	//音频配置
	CONFIG_TYPE_VIDEO_OSD = 44,			//视频OSD叠加配置

	CONFIG_TYPE_ALARM_CENTER_UP	= 120,  //报警中心上传配置。
	CONFIG_TYPE_RECORD_NEW	= 123,		//新录象配置
	CONFIG_TYPE_TIMESHEET = 125,		//获取工作表
	CONFIG_TYPE_COLOR = 126,			// 颜色配置
	CONFIG_TYPE_CAPTURE_127	= 127,		//双码流图像配置
	CONFIG_TYPE_CAPTURE_128 = 128,		//新的编码配置
	CONFIG_TYPE_AUDIO_CAPTURE = 129,	//音频配置
	CONFIG_TYPE_WLAN = 131,				//查询无线配置
	CONFIG_TYPE_TRANSFER_STRATEGY = 133,//网络传输策略配置
	CONFIG_TYPE_WIRELESS_ADDR = 134,    //无线报警配置,主要是输出地址和遥控器地址
	CONFIG_TYPE_WLAN_DEVICE = 135,		//搜索无线设备
	CONFIG_TYPE_MULTI_DDNS = 140,		//多DDNS服务器配置
	CONFIG_TYPE_INTERVIDEO = 190,		//平台接入配置
	CONFIG_TYPE_OEM_INFO = 200,			//第三方接入信息
	CONFIG_TYPE_DSP_MASK = 210,
	CONFIG_TYPE_AUTO_REGISTER = 241,	// 主动注册参数配置
	CONFIG_TYPE_POS ,					//下载POS参数 jfchen

	CONFIG_TYPE_ALARM_BEGIN,
	CONFIG_TYPE_ALARM_LOCALALM,
	CONFIG_TYPE_ALARM_NETALM,
	CONFIG_TYPE_ALARM_DECODER,
	CONFIG_TYPE_ALARM_MOTION,
	CONFIG_TYPE_ALARM_BLIND,
	CONFIG_TYPE_ALARM_LOSS,
	CONFIG_TYPE_ALARM_NODISK,
	CONFIG_TYPE_ALARM_DISKERR,
	CONFIG_TYPE_ALARM_DISKFULL,
	CONFIG_TYPE_ALARM_NETBROKEN,
	CONFIG_TYPE_ALARM_ENCODER,
	CONFIG_TYPE_ALARM_WIRELESS,
	CONFIG_TYPE_ALARM_AUDIODETECT,
	CONFIG_TYPE_ALARM_DISKNUM,
	CONFIG_TYPE_ALARM_PANORAMA,
	CONFIG_TYPE_ALARM_LOSTFOCUS,
	CONFIG_TYPE_ALARM_IPCOLLISION,
	CONFIG_TYPE_ALARM_END,

}CONFIG_TYPES;

/*
typedef enum _config_alarm_type_t
{
	CONFIG_ALARM_BEGIN		= 50,
	CONFIG_ALARM_LOCALALM	,
	CONFIG_ALARM_NETALM		,
	CONFIG_ALARM_DECODER		,
	CONFIG_ALARM_MOTION		,
	CONFIG_ALARM_BLIND		,
	CONFIG_ALARM_LOSS		,
	CONFIG_ALARM_NODISK		,
	CONFIG_ALARM_DISKERR		,
	CONFIG_ALARM_DISKFULL	,
	CONFIG_ALARM_NETBROKEN	,
	CONFIG_ALARM_END			,
} CONFIG_ALARM_TYPE;
*/
typedef enum _config_wsheet_typt_t
{
	WSHEET_RECORD = 1,
	WSHEET_LOCAL_ALARM = 2,
	WSHEET_NET_ALARM = 3,
	WSHEET_MOTION = 4,
	WSHEET_BLIND = 5,
	WSHEET_LOSS = 6,
	WSHEET_NO_DISK = 7,
	WSHEET_DISK_ERROR = 8,
	WSHEET_DISK_FULL = 9,
	WSHEET_NET_BROKEN = 10,
	WSHEET_DECODER_ALARM = 11,
    WSHEET_WIRELESS_ALARM = 13,
	WSHEET_PANORAMA_SWITCH = 14,
	WSHEET_LOST_FOCUS = 15,
} CONFIG_WSHEET_TYPE;

typedef enum _system_info_type_t
{
	SYSTEM_INFO_GENERAL = 0,		/* 普通信息 */
	SYSTEM_INFO_DEV_ATTR,			/* 设备属性信息 */
	SYSTEM_INFO_DISK_DRIVER,		/* 硬盘信息 */
	SYSTEM_INFO_FILE_SYSTEM,		/* 文件系统信息 */
	SYSTEM_INFO_VIDEO_ATTR,			/* 视频属性 */
	SYSTEM_INFO_CHARACTER_SET,		/* 字符集信息 */
	SYSTEM_INFO_OPTICS_STORAGE,		/* 光存储设备信息 */
	SYSTEM_INFO_DEV_ID,				/* 设备序列号(ID) */
	SYSTEM_INFO_DEV_VER,			/* 设备版本，字符串形式 */
	SYSTEM_INFO_LOGIN_ATTR,         /* 登陆特性*/
	SYSTEM_INFO_TALK_ATTR = 10,		/* 语音对讲属性 */
	SYSTEM_INFO_DEV_TYPE = 11,		/* 设备类型 */
	SYSTEM_INFO_PLATFORM = 12,		/* 查询设备特殊协议(平台接入)支持信息 */
	SYSTEM_INFO_SD_CARD = 13,		/* SD卡信息 */   
	SYSTEM_INFO_MOTIONDETECT = 14,	/* 设备视频动态检测属性信息 */
	SYSTEM_INFO_VIDEOBLIND = 15,	/* 视频区域遮挡属性信息 */
	SYSTEM_INFO_CAMERA = 16,		/* 摄像头属性信息 */
	SYSTEM_INFO_WATERMARK = 17,		/* 查询图象水印能力 */
	SYSTEM_INFO_WIRELESS = 18,		/* 查询Wireless能力 */
    SYSTEM_INFO_Language = 20,		/* 查询支持的语言列表 */
	SYSTEM_INFO_PICTURE = 25,		/* 是否支持新的录像及图片列表查询方式 */
	SYSTEM_INFO_DEV_ALL = 26,		/* 设备功能列表 */
	SYSTEM_INFO_INFRARED = 27,		/* 查询无线报警能力 */
	SYSTEM_INFO_NEWLOGTYPE = 28,	/* 是否支持新的日志格式 */
	SYSTEM_INFO_OEM_INFO = 29,		/* OEM信息 */
	SYSTEM_INFO_NET_STATE=30,		/* 网络状态 */
	SYSTEM_INFO_DEV_SNAP =31,		/* 设备抓图功能能力查询 */
	SYSTEM_INFO_VIDEO_CAPTURE = 32,	/* 视频前端采集能力查询 */
	SYSTEM_INFO_DISK_SUBAREA = 33,  /* 硬盘分区能力查询*/
} SYSTEM_INFO_TYPES;

typedef enum __device_event_type
{
	EVENT_CONFIG_REBOOT = 0,
	EVENT_CONFIG_ACK,
	EVENT_TALK_START,
	EVENT_TALK_STOP,
	EVENT_CONFIG_CHANGE,
	EVENT_NVD_PLAYBACK,
	EVENT_DEVICE_DEAL,
} DEVICE_EVENT_TYPE;

typedef struct
{
    long    left;
    long    top;
    long    right;
    long    bottom;
} COVER_RECT;

typedef struct ywtime					        /*时间				*/
{
	unsigned int second		:6;				    /*	秒	1-60		*/
	unsigned int minute		:6;				    /*	分	1-60		*/
	unsigned int hour		:5;				    /*	时	1-24		*/
	unsigned int day		:5;				    /*	日	1-31		*/
	unsigned int month		:4;				    /*	月	1-12		*/
	unsigned int year		:6;				    /*	年	2001-2064	*/
}FRAME_TIME;

typedef struct
{
	unsigned char	CapVersion[8];  //版本号
	unsigned char	CifMode;		//分辨率
	unsigned char	VideoType;		//编码模式：
	unsigned char	EncodeMode;		//码流控制
	unsigned char	ImgQlty;		//码流的画质
	unsigned char	Frames;			//帧率
	unsigned char	Brightness;		//亮度
	unsigned char	Contrast;		//对比度
	unsigned char	Saturation;		//饱和度
	unsigned char	Hue;			//色度
	unsigned char	AudioEn;		//音频使能
	unsigned char	Gain;			//增益
	unsigned char    CoverEnable;	//区域遮盖使能
	COVER_RECT	Cover;			//区域遮盖
}CONFIG_CAPTURE;

#define N_COLOR_SECTION 2
#define N_ENCODE_AUX_INTER 3

char * const CAPTURE_VERSION = "0603";
char * const COLOR_VERSION = "0101";

enum rec_type_inter
{
	REC_TYP_TIM_INTER = 0,		/*定时录像*/
	REC_TYP_MTD_INTER,					
	REC_TYP_ALM_INTER,
	REC_TYP_NUM_INTER,
};

enum enc_cover_type
{
	ENC_COVER_NONE = 0,
	ENC_COVER_PREVIEW = 0x1,
	ENC_COVER_MONITOR = 0x2,
	ENC_COVER_NUM = 0x3
};

enum enc_snap_type
{
	SNAP_TYP_TIM_INTER = 0,
	SNAP_TYP_ALARM_INTER,
	SNAP_TYP_NUM_INTER,
};

//! 编码选项
typedef struct  
{
	BYTE	ImageSize;			/*!< 分辨率 参照枚举capture_size_t(DVRAPI.H) */
	BYTE	BitRateControl;		/*!< 码流控制 参照枚举capture_brc_t(DVRAPI.H) */
	BYTE	ImgQlty;			/*!< 码流的画质 档次1-6		*/
	BYTE	Frames;				/*!< 帧率　档次N制1-6,P制1-5 */	
	BYTE	AVEnable;			/*!< 音视频使能 1位为视频，2位为音频。ON为打开，OFF为关闭 */
	BYTE	IFrameInterval;		/*!< I帧间隔帧数量，描述两个I帧之间的P帧个数，有效范围0-149，不支持为255 */
	WORD	usBitRate;			/*!< DSP限定编码码流 */
}ENCODE_OPTION;

//! 标题结构
typedef struct  
{
	DWORD	TitlefgRGBA;			/*!< 标题的前景RGB，和透明度 */
	DWORD	TitlebgRGBA;		/*!< 标题的后景RGB，和透明度*/
	WORD	TitleLeft;			/*!< 标题距左边的距离与整长的比例*8192 */
	WORD	TitleTop;			/*!< 标题的上边的距离与整长的比例*8192 */
	WORD	TitleRight;			/*!< 标题的右边的距离与整长的比例*8192 */
	WORD	TitleBottom;			/*!< 标题的下边的距离与整长的比例*8192 */
	BYTE	TitleEnable;			/*!< 标题使能 */
	BYTE	Reserved[3];
}ENCODE_TITLE;

//! 编码信息结构
typedef struct {
	BYTE				CapVersion[8];				/*!< 版本号			*/		
	ENCODE_OPTION		MainOption[REC_TYP_NUM_INTER];	/*!< 主码流，REC_TYP_NUM不同录像类型*/
	ENCODE_OPTION		AssiOption[N_ENCODE_AUX_INTER];	/*!< 支持3 路辅码流 */
	BYTE				Compression;				/*!< 压缩模式 */;	
	BYTE    			CoverEnable;				/*!< 区域遮盖开关　0x00不使能遮盖，0x01仅遮盖预览，0x10仅遮盖录像，0x11都遮盖	*/
	BYTE 				alignres[2];			/*!< 保留对齐用 */
	RECT				Cover;						/*!< 区域遮盖范围	*/	
	
	ENCODE_TITLE 		TimeTitle;					/*!< 时间标题*/
	ENCODE_TITLE 		ChannelTitle;				/*!< 通道标题*/
	
	ENCODE_OPTION		SnapOption[SNAP_TYP_NUM_INTER]; /*!< 2种 抓图配置 */   
	short				PicTimeInterval;			/*!<自定义抓图时间间隔，单位为s*/ 

	//delete by cqs 之前临时要求做用于测试，后来屏蔽掉转移到摄像头属性那边
//	BYTE				MainScanMode[REC_TYP_NUM_INTER];	//扫描模式 与主码流对应	0-逐行扫描，1-隔行扫描
//	BYTE				AssiScanMode[N_ENCODE_AUX_INTER] ;	//扫描模式 与辅码流对应 0-逐行扫描，1-隔行扫描

	BYTE	Volume;								/* 保存音量的阀值 */
	BYTE	VolumeEnable;						/* 音量阀值使能 */									   
	BYTE	Noise;								/* 燥音的阀值 */
	BYTE	Reserved[43];  /*!< 保存用 */
} NEW_CONFIG_CAPTURE;

//! 区域遮挡配置结构体
typedef struct __video_cover_t
{
	BYTE				iChannel;		// 通道号，[1~n]
	BYTE				iCoverNum;		// 覆盖的区域个数
	char				reserved[30];	// 保留
}CONFIG_VIDEO_COVER_T;

typedef struct __video_cover_attr_t
{
	COVER_RECT				rcBlock;		// 覆盖的区域坐标
	int					Color;			// 覆盖的颜色
	BYTE				iBlockType;		// 覆盖方式：0-黑块，1-马赛克
	BYTE				Encode;			// 编码级遮挡, 1—生效, 0—不生郊
	BYTE				Priview;		// 预览遮挡, 1—生效, 0—不生效
	BYTE				reserved[28];	// 保留
}CONFIG_VIDEO_COVER_ATTR;

//! 查询设备视频动态检测属性信息
typedef struct __MOTION_DETECT_CAPS
{
	DWORD				Enabled;		// 置1表示支持动态检测，置0表示不支持动态检测
	DWORD				GridLines;		// 动态检测的区域需要划分成的列数
	DWORD				GridRows;		// 动态检测的区域需要划分成的行数
	BYTE				Result;			// 是否能得到每块区域的检测结果
	BYTE				Hint;			// 是否能在检测成功时给予提示
}MOTION_DETECT_CAPS;

//! 查询视频区域遮挡属性信息
typedef struct __video_cover_caps_t
{
	unsigned char		iEnabled;		// 置1表示支持 区域遮挡，0 不支持
	unsigned char		iBlockNum;		// 支持的遮挡块数
	unsigned char		iRev[62];		// 动态检测的区域需要划分成的行数。
}VIDEO_COVER_CAPS_T;

//!无线网络信息
typedef struct 
{
	int					nEnable;		// 无线使能
	char				szSSID[36];		// SSID
	int					nLinkMode;		// 连接模式 0:auto 1:adhoc 2:Infrastructure
	int					nEncryption;	// 加密 0: off 2:WEP64bit 3: WEP128bit 4:WPA-PSK-TKIP, 5: WPA-PSK-CCMP
	int					nKeyType;		// 0:Hex 1:ASCII
    int					nKeyID;			// 序号
	char				szKeys[128];	// 密码
	int					nKeyFlag;
	int					nReserved[3];
} CONFIG_WLAN_INFO;

//!搜索无线设备信息
typedef struct  
{
	char				szSSID[36];
	int					nLinkMode;			// 连接模式 0:adhoc 1:Infrastructure
	int 				nEncryption;		// 加密 0: off  2:WEP64bit 3: WEP128bit
} CONFIG_WLAN_DEVICE;

//!摄像头属性
typedef struct 
{
	BYTE				bExposure;		// 曝光模式 1-9:手动曝光等级; 0:自动曝光
	BYTE				bBacklight;		// 背光补偿 3:强；2：中；1：弱； 0:关
	BYTE				bAutoColor2BW;	// 日/夜模式 2:开；1：自动; 0:关
	BYTE				bMirror;		// 镜像  1 开， 0关
	BYTE				bFlip;			// 翻转  1 开， 0关
	BYTE				bLensFunction;	// 高4位表示功能能力，低4位表示功能开关
										// bit4 = 1:支持自动光圈功能；
										// bit0 = 0:关闭自动光圈；bit0 = 1:开启自动光圈；
	BYTE				bWhiteBalance;  //白平衡 0:Disabled,1:Auto 2:sunny 3:cloudy 4:home 5:office 6:night 
	BYTE                bSignalFormat;  //信号格式0-Inside(内部输入) 1- BT656 2-720p 3-1080i  4-1080p  5-1080sF
	float				ExposureValue1; //自动曝光时间下限或者手动曝光自定义时间,毫秒为单位，取值0.1ms~80ms
	float				ExposureValue2;	//自动曝光时间上限,毫秒为单位，取值0.1ms~80ms
	BYTE				bRotate90;		//0-不旋转，1-顺时针90°，2-逆时针90°
	char				bRev[111];		// 保留
} CONFIG_CAMERA_T;

//!多网卡信息
typedef struct  
{
	char				szEthernetName[64];
	char				szIp[32];
	char				szMask[32];
	char				szGateway[32];
} CONFIG_MULTI_ETHERNET;

//!默认网卡
typedef struct  
{
	char				szEthernetName[64];
	int					nTranMedia;	//0：有线 1：无线
	int					nValid;	//0：无效 1：有效
} CONFIG_DEFAULT_ETHERNET;

//!音频格式相关的参数掩码
typedef enum EnAudioBit
{
    EN_AUDIO_BIT_8  = 0,      //8bit
    EN_AUDIO_BIT_16  ,    //16bit
};
typedef enum EnAudioSamples
{
    EN_AUDIO_SAMPLE_8  = 0,    //8K
    EN_AUDIO_SAMPLE_16  ,  //16K
};
typedef enum EnAudioEncodeFmt
{
    EN_AUDIO_PCM = 0,
    EN_AUDIO_ADPCM  ,   
};

//!音频的编码结构(是与视频通道绑定的)
typedef struct _audio_capture_cfg
{
	WORD 	nChannels;				//声道数  
    WORD 	wFormatTag;				//编码类型
	DWORD	dwBitsPerSample;			//采样深度
	DWORD	nSamplesPerSec;			//采样率  
	DWORD   Reserved[8];
}AUDIO_CAPTURE_CFG;

//使用协议中的0xa3的128号子命令的结构体
typedef struct {
	BYTE	CapVersion[8];		/*!< 版本号			*/		
	BYTE	CifMode;			/*!< 分辨率 参照枚举capture_size_t(DVRAPI.H) */
	BYTE	VideoType;			/*!< 编码模式 参照枚举capture_comp_t(DVRAPI.H) */
	BYTE	EncodeMode;			/*!< 码流控制 参照枚举capture_brc_t(DVRAPI.H) */
	BYTE	ImgQlty;			/*!< 码流的画质 档次1-6		*/
	BYTE	Frames;				/*!< 帧率　档次N制1-6,P制1-5 */	
	BYTE	Brightness;			/*!< 亮度	0-100		*/
	BYTE	Contrast;			/*!< 对比度	0-100		*/
	BYTE	Saturation;			/*!< 饱和度	0-100		*/
	BYTE	Hue;				/*!< 色度	0-100		*/
	BYTE	AudioEn;			/*!< 音频使能 ON为打开，OFF为关闭	*/
	BYTE	Gain;				/*!< 增益	0-100		*/
	BYTE    CoverEnable;		/*!< 区域遮盖开关　ON为打开，OFF为关闭	*/
	COVER_RECT	Cover;				/*!< 区域遮盖范围	*/	

	BYTE	TimeTilteEn;		/*!< 时间标题使能  ON为打开，OFF为关闭 */
	BYTE	ChTitleEn;			/*!< 通道标题使能  ON为打开，OFF为关闭 */
	
	BYTE	ExtFunction;		/*!< 扩展功能支持，主要用于双码流 */
	BYTE	VideoEn;			/*!< 视频参数　*/
	WORD	TimeTitleL;			/*!< 时间标题距左边的距离与整长的比例*8192 */
	WORD	TimeTitleT;			/*!< 时间标题的上边的距离与整长的比例*8192 */
	WORD	TimeTitleR;			/*!< 时间标题的右边的距离与整长的比例*8192 */
	WORD	TimeTitleB;			/*!< 时间标题的下边的距离与整长的比例*8192 */
	WORD	ChannelTitleL;		/*!< 通道标题距左边的距离与整长的比例*8192 */
	WORD	ChannelTitleT;		/*!< 通道标题距上边的距离与整长的比例*8192 */
	WORD	ChannelTitleR;		/*!< 通道标题距右边的距离与整长的比例*8192 */
	WORD	ChannelTitleB;		/*!< 通道标题距下边的距离与整长的比例*8192 */
	BYTE	Reserved[64];		/*!< 保存用，多留点，省得以后不停地改结构通知客户端 */
} CONFIG_CAPTURE_V2201;

typedef struct {    // 定时时段 
    unsigned char    StartHour; // 开始时间 
    unsigned char    StartMin; 
    unsigned char    StartSec; 
    unsigned char    HourEnd;   // 结束时间 
    unsigned char    EndMin; 
    unsigned char    EndSec; 
    unsigned char    State; // 第二位是定时，第三位是动态检测，第四位是报警 
    unsigned char    Reserve; // Reserve已经被使用，更改的话请通知录像模块
} TSECT;

//! 颜色设置内容
typedef struct 
{
	TSECT 	Sector;				/*!< 对应的时间段*/
	BYTE	Brightness;			/*!< 亮度	0-100		*/
	BYTE	Contrast;			/*!< 对比度	0-100		*/
	BYTE	Saturation;			/*!< 饱和度	0-100		*/
	BYTE	Hue;				/*!< 色度	0-100		*/
	BYTE	Gain;				/*!< 增益	0-100		*/
	BYTE	Reserve[3];
} COLOR_PARAM;

//! 颜色结构
typedef struct  
{
	BYTE ColorVersion[8];	
	COLOR_PARAM Color[N_COLOR_SECTION];
} CONFIG_COLOR;

#define N_WEEKS         7 
#define N_TSECT         6 

typedef struct { 
    unsigned char    RecVersion[8];               /*!< 版本号 */ 
    TSECT   Sector[N_WEEKS][N_TSECT];    /*!< 定时时段，一个星期七天，每天 N_TSECT时间段 */ 
    unsigned char    PreRecord;                   /*!< 预录开关, 0-关闭, 1-打开 */ 
    unsigned char    Redundancy;                  /*!< 冗余开关，0-无效, 1-有效 */ 
    unsigned char    Reserved[2];                 /*!< 保留 */ 
} CONFIG_RECORD; 


typedef struct tagPTZ_ATTR
{
    unsigned long baudrate; // 具体值参考串口配置
    unsigned char databits;
    unsigned char parity;
    unsigned char stopbits;
}PTZ_ATTR;

typedef struct
{
    unsigned char Ptz_Version[8];    /*!< 版本号 */
    PTZ_ATTR PTZAttr;       /*!< 串口属性 */
    unsigned short DestAddr;          /*!< 目的地址 0-255 */
    unsigned short Protocol;          /*!< 协议类型 保存协议的下标，动态变化 */
    unsigned short MonitorAddr;       /*!< 监视器地址 0-64 */
	BYTE    CamID;					/*!摄像头ID  */
	BYTE 	byMartixID;				/* 矩阵号*/
	BYTE	bPTZType;               /*!<0-兼容，本地云台 1-远程网络云台>*/
    BYTE    Reserved[7];			/*! <  保留  */
} CONFIG_PTZ;

typedef struct { 
	unsigned char   Com_Version[8];       /*!< 版本号 */ 
	unsigned char    Function;           /*!< 串口功能 0-普通, 1-控制键盘, 2-透明串口, 3-ATM机*/ 
	unsigned char    DataBits;           /*!< 数据位 取值为5,6,7,8 */ 
	unsigned char    StopBits;           /*!< 停止位 0-1位, 1-1.5位, 2-2位 */ 
	unsigned char    Parity;             /*!< 校验位 0-no, 1-odd, 2-even */ 
	unsigned long   BaudBase;           /*!< {300,600,1200,2400,4800,9600,19200,38400,57600,115200}*/ 
} CONFIG_COMM;


typedef struct
{
	char    ProtocolName[12];			//协议名
	unsigned long	BaudBase;	        //波特率
	unsigned char	DataBits;	        //数据位
	unsigned char	StopBits;	        //停止位
	unsigned char	Parity;		        //校验位
	unsigned char    Resove;
} DEC_PROTOCOL;

typedef struct
{
    unsigned char Version[8]; // 8字节的版本信息
    char HostName[16]; // 主机名
    unsigned long HostIP; // IP 地址
    unsigned long Submask; // 子网掩码
    unsigned long GateWayIP; // 网关 IP
    unsigned long DNSIP; // DNS IP

    // 外部接口
    unsigned long	AlarmServerIP; // 报警中心IP
    unsigned short  AlarmServerPort; // 报警中心端口
    unsigned long	SMTPServerIP; // SMTP server IP
    unsigned short  SMTPServerPort; // SMTP server port
    unsigned long	LogServerIP; // Log server IP
    unsigned short  LogServerPort; // Log server port

    // 本机服务端口
    unsigned short  HttpPort; // HTTP服务端口号
    unsigned short  HttpsPort; // HTTPS服务端口号
    unsigned short  TCPPort; // TCP 侦听端口
    unsigned short  TCPMaxConn; // TCP 最大连接数
    unsigned short  SSLPort; // SSL 侦听端口
    unsigned short  UDPPort; // UDP 侦听端口
    unsigned long	McastIP; // 组播IP
    unsigned short  McastPort; // 组播端口
    
    // 其他
    unsigned char  MonMode; // 监视协议 0-TCP, 1-UDP, 2-MCAST //待确定1-TCP
    unsigned char  PlayMode; // 回放协议 0-TCP, 1-UDP, 2-MCAST//待确定1-TCP
    unsigned char  AlmSvrStat; // 报警中心状态 0-关闭, 1-打开
}CONFIG_NET;

typedef struct
{
	int iOnTimeUpDay;   /*设置上传日期	"Never=0", "Everyday=1", "Sunday=2", "Monday", 
						Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"*/	
	int iOnTimeUpHour; //设置上传时间 [0, 23]	
}ALARM_CENTER_CFG;

/*以太网配置*/
typedef struct 
{
	char	szEthernetName[64];		// 网卡名称
	char	sDevIPAddr[16];			// DVR IP 地址
	char	sDevIPMask[16];			// DVR IP 地址掩码
	char	sGatewayIP[16];			// 网关地址
	char	byMACAddr[40];			// MAC地址，只读
	char	szMode[16];				// Mode:网卡所处于模式, bond:绑定模式，　balance:负载均衡模式
	char	szDuplex[16];			// Full :全双工 Half :半双工 Auto: 自适应
	char	szSpeed[16];			// 10Mbps: 100Mbps: Auto: 自适
	BYTE	bTranMedia;				// 0：有线 1：无线
	BYTE	bValid;					// 0：无效 1：有效
	BYTE	bDefaultEth;			// 0：非默认 1：默认
	BYTE	bDHCPEnable;			// DHCP使能
	BYTE	bDHCPFunc;				// DHCP能力
} CONFIG_ETHERNET;

/************************************************************************
 * 云台控制类型定义
 ***********************************************************************/
typedef enum PTZControlType
{
	PTZ_UP =0,				//上
	PTZ_DOWN,			//下
	PTZ_LEFT,			//左
	PTZ_RIGHT,			//右
	PTZ_ZOOM_ADD,		//变倍
	PTZ_ZOOM_DEC,		
	PTZ_FOCUS_ADD = 0x07,		//调焦
	PTZ_FOCUS_DEC = 0x08,		
	PTZ_APERTURE_ADD,	//光圈
	PTZ_APERTURE_DEC,
    PTZ_POINT_MOVE = 0x10,     //转至
    PTZ_POINT_SET ,      //设置
    PTZ_POINT_DEL,      //删除
	PTZ_LAMP = 0x0e,           //灯
    PTZ_POINT_LOOP = 0x0f,     //点间轮循
};

/*
typedef struct
{
    String  m_strServerAdd;  // 邮件服务器地址
    String  m_strDestEmail;  //邮箱目的地址
    String  m_strCCEmail;    //邮箱抄送地址
    String  m_strBccEmail;   //邮箱暗抄地址
    String  m_strUEmail;     // 发件人邮箱
    String  m_strUName;  	 // 发件人名称
    String  m_strUPwd; 	     // 发件人密码
    String  m_strAttatch;  	 //add attatchment
    String  m_strSubject;  	 // 标题
    String  m_strBody;       //内容
    int m_iMaxAttatchSize;   // 附件大小上限, -1 不限制
} MAILER_CFG;
*/

enum log_t
{
	//system logs
	LOG_REBOOT = 0x0000,
	LOG_SHUT,

	//config logs
	LOG_CONFSAVE = 0x0100,
	LOG_CONFLOAD,

	//fs logs
	LOG_FSERROR = 0x0200,   //文件系统错误
	LOG_HDD_WERR,           //硬盘写错误
	LOG_HDD_RERR,           //硬盘读错误
	LOG_HDD_TYPE,           //设置硬盘类型
	LOG_HDD_FORMAT,         //格式化硬盘

	//alarm logs
	LOG_ALM_IN = 0x0300,

	//record
	LOG_MANUAL_RECORD_START = 0x0400,
	LOG_MANUAL_RECORD_STOP,

	//user manager
	LOG_LOGIN = 0x0500,
	LOG_LOGOUT,
	LOG_ADD_USER,
	LOG_DELETE_USER,
	LOG_MODIFY_USER,
	LOG_ADD_GROUP,
	LOG_DELETE_GROUP,
	LOG_MODIFY_GROUP,

	//clear
	LOG_CLEAR = 0x0600,

	//record operation
	LOG_SEARCH = 0x0700,    //录像查询
	LOG_DOWNLOAD,           //录像下载
	LOG_PLAYBACK,           //录像回放
	LOG_TYPE_NR = 8,
};

typedef struct _LOG_ITEM
{
    FRAME_TIME time;        //日期
    unsigned short type;          //类型
    unsigned char flag;          //标记, 0x00 正常，0x01 扩展，0x02 数据
    unsigned char data;          //数据 0退出 1登陆
    unsigned char context[8];    //内容
}LOG_ITEM;

typedef struct
{
	unsigned char	Gen_Version[8]; //版本号
	unsigned short	LocalNo;		// 本机编号
	unsigned char	VideoFmt;		// 制式 :NTSC,PAL等
	unsigned char	Language;		// 语言选择
	unsigned char	OverWrite;		// 盘满时　1　覆盖,　0　停止
	unsigned char	RecLen;			// 录象段长度
	unsigned char	ShutPswEn;		// 关机密码使能
	unsigned char	DateFmt;		//日期格式
	unsigned char	DateSprtr;		//日期分割符
	unsigned char	TimeFmt;		//时间格式
	unsigned char	DST;			// 是否实行夏令时 1-实行 0-不实行
	unsigned char	Reserved;		//未知
	unsigned char	ManualStart;	//手动录像启动
	unsigned char	ManualStop;		//手动录像停止
} CONFIG_GENERAL;

typedef struct 
{ 
    unsigned long dwVersion; 
    unsigned char dbReserve[4]; 
}VER_CONFIG; 

typedef struct
{
    VER_CONFIG sVerConfig;  /*! 配置文件版本信息 */
    unsigned long MotionRecordMask; /*! 录象掩码 */
    unsigned long MotionAlarmMask;  /*! 报警输出: 按位表示报警通道 */
    unsigned long MotionTimeDelay;  /*! 延时：10～300 sec */
    unsigned long MotionRegion[18]; /*! 区域 */
    unsigned long BlindAlarmMask;   /*! 黑屏检测: 按位表示报警通道 */
    unsigned long LossAlarmMask;    /*! 丢失检测: 按位表示报警通道 */
    unsigned char MotionLever;       /*! 灵敏度：1～6 */
    unsigned char dbReserved[2];     /*! 保留字段 */
    unsigned char BlindTipEn;        /*! 黑屏检测提示使能 */
    unsigned char LossTipEn;         /*! 丢失检测提示使能 */
    unsigned char Mailer;            /*! 发送邮件掩码（1-8位表示最多8通道的图像） */
    unsigned char SendAlarmMsg;      /*! 发送报警信息（给报警服务器等） */
    unsigned char adbReserved[2];    /*! 保留字段 */
} CONFIG_DETECT;

typedef struct __sys_string_support
{
	unsigned char  Version;		// 本配置的版本信息
    unsigned long Type;       // 支持的类型， 掩码表示， 从低到高依表示：自定义|UTF-8
	unsigned char  iReserved_00;	// 预留
	unsigned char  iReserved_01;	// 预留
	unsigned char  iReserved_02;	// 预留
	unsigned char  iReserved_03;	// 预留
	unsigned char  iReserved_04;	// 预留
	unsigned char  iReserved_05;	// 预留
	unsigned char  iReserved_06;	// 预留
	unsigned char  iReserved_07;	// 预留
}SYS_STRING_SUPPORT;

#define ALARM_SECTS 2

typedef struct 
{
    VER_CONFIG sVerConfig;          /*! 配置文件版本信息 */
    unsigned long AlarmMask;                /*! 报警输出掩码 */
    unsigned long RecordMask;               /*! 录象通道掩码 */
    unsigned long TimeDelay;                /*! 录象延时：10～300 sec */
    unsigned char SensorType;                /*! 传感器类型：常开 or 常闭 */
    unsigned char TimePreRec;                /*! 预录时间 或者 预录开关 */
    unsigned char AlarmTipEn;                /*! 报警提示 */
    TSECT sAlarmSects[ALARM_SECTS]; /*! 报警输出和提示的时间段 */
    unsigned long dwSectMask[ALARM_SECTS];  /*! 报警输出和提示的时间段的掩码 */
    unsigned long dwInputTypeMask;          /*! 报警输入类型掩码 */
    unsigned char dbTourEn;                  /*! 是否允许报警轮训 */
    unsigned char Mailer;                    /*! 发送邮件掩码（1-8位表示最多8通道的图像） */
    unsigned char SendAlarmMsg;              /*! 发送报警信息（给报警服务器等） */
    unsigned char adbReserved;               /*! 保留字段 */
}CONFIG_ALARM, *pCONFIG_ALARM;

#define NAME_LEN	32

typedef struct {
	//VER_CONFIG sVerConfig;
	unsigned char	   ChnTitle[NAME_LEN];
}CONFIG_CHNTITLE;

typedef struct __sysattr_t
{
	unsigned char iVerMaj;          // BOIS主版本号(软件)
	unsigned char iVerSub;          // BOIS次版本号(软件)
	unsigned char iVideoInCaps;     // 视频输入接口数量
	unsigned char iAudioInCaps;     // 音频输入接口数量
	unsigned char iAlarmInCaps;     // 报警输入接口数
	unsigned char iAlarmOutCaps;    // 报警输出接口数
	unsigned char iIdePortNum;      // IDE接口数
	unsigned char iAetherNetPortNum;// 网络接口数
	unsigned char iUsbPortNum;      // USB接口数
	unsigned char iComPortNum;      // 串口数
	unsigned char iParallelPortNum; // 并口口数
	unsigned char iMatrixOutNum;	// 视频矩阵输出端口数量 0表示不支持矩阵功能
	unsigned short year;            // 编译日期: 年
	unsigned char  month;           // 编译日期: 月
	unsigned char  day;             // 编译日期: 日
	unsigned char iVerUser;         // BOIS用户版本号(软件)
	unsigned char iVerModify;       // BOIS修改版本号(软件)
	unsigned char iWebVersion[4];   // WEB版本
	unsigned char WebYear;          // 预留
	unsigned char WebMonth;         // 预留
	unsigned char WebDay;           // 预留
	unsigned char iReserved_01;     // 预留
	unsigned char iReserved_02;     // 预留
	unsigned char iReserved_03;     // 预留
	unsigned char iReserved_04;     // 预留
	unsigned char iReserved_05;     // 预留
	unsigned char iIsMutiEthIf;     // 网卡支持情况，0：单网卡 1：多网卡（单网卡用之前旧的协议，后者用新的字符串协议）
	unsigned char iReserved_07;     // 预留
}SYSATTR_T;

//版本0
typedef struct { 
	int		ide_num;   //硬盘数量 
	int		ide_port;   //IDE口数量 
	unsigned long	ide_msk;    //硬盘掩码 
	unsigned long	ide_bad;    //坏盘掩码 
	unsigned long	ide_cap[32];  //硬盘容量 
} IDE_INFO, *pIDE_INFO; 

//版本1
/// IDE设备信息结构
typedef struct IDE_INFO64{
	/// 已挂载的硬盘的数量,，不包括坏盘。硬盘个数也等于ide_msk中置1的位个数减去
	/// ide_bad中置1的位个数. 应用程序访问硬盘时, 硬盘序号规则如下: 第一个被置1
	/// 的位对应的IDE通道上挂载的正常硬盘的硬盘序号为0，第二个被置1的位对应正常
	/// 硬盘的硬盘序号为1，依次类推。
	int		ide_num;//分区个数
	
	///< IDE通道的数目，包括扩展的IDE通道。
	int		ide_port;//ignore
	
	/// 指示各个IDE通道上是否挂载硬盘，包括坏盘。每一位对应IDE通道的主盘或从盘，
	/// 置位表示以挂载硬盘，否则没有。对应关系为bit0对应IDE0的主盘，bit1对应IDE0
	/// 的从盘，bit2对应IDE1的主盘，bit3对应IDE1的从盘，依次类推。
	unsigned long	ide_msk;//ignore
	
	/// 指示各个IDE通道上是否挂载了坏的硬盘。每一位对应IDE通道的主盘或从盘，置位
	/// 表示以有坏硬盘，否则没有。对应关系同ide_msk。
	unsigned long	ide_bad;//ignore
	
	/// 已挂载的正常硬盘的容量，以扇区为单位，扇区大小为IDE_SECTOR_SIZE。扇区数用
	/// 32位来表示，数组下标是正常硬盘的序号。
#ifdef WIN32
	unsigned __int64			ide_cap[32];
#else	//linux
	unsigned long long			ide_cap[32];
#endif
	FRAME_TIME  start_time1;		//总的录像开始时间
	FRAME_TIME	end_time1;			//总的录像结束时间
	DWORD	total_space;		//总容量
	DWORD	remain_space;		//剩余容量
} IDE_INFO64, *pIDE_INFO64;

 /*
 //驱动器信息结构 
 typedef struct _DRIVER_INFO{ 
 	unsigned long	driver_type;    //驱动器类型 
 	long	is_current;   //是否为当前工作盘 
 	FRAME_TIME  start_time1;    //录像时间段1开始时间 
 	FRAME_TIME  end_time1;    //录像时间段1结束时间 
 	long	two_part;    //是否有第二段
 	FRAME_TIME  start_time2;    //录像时间段2开始时间 
 	FRAME_TIME  end_time2;    //录像时间段2结束时间 
 	unsigned long	total_space;    //总容量 
 	unsigned long	remain_space;    //剩余容量 
 	unsigned long	error_flag;   //错误标志，文件系统初始化时被设置 
 }DRIVER_INFO,*pDRIVER_INFO; 
 
 typedef  struct  {
 	int ide_num; //硬盘数量
 	int ide_port; //IDE口数量
 	DWORD ide_msk; //硬盘掩码
 	DWORD ide_bad; //坏盘掩码
 	DWORD ide_cap[32]; //硬盘容量
 }  IDE_INFO,  *pIDE_INFO;
 */
 
//驱动器信息结构
typedef  struct  _DRIVER_INFO{
	unsigned long driver_type; //驱动器类型
	long is_current; //是否为当前工作盘
	unsigned long section_count; //时间段数
	FRAME_TIME    start_time1; //录像时间段1开始时间
	FRAME_TIME end_time1; //录像时间段1结束时间
	long two_part; //是否有第二段
	FRAME_TIME start_time2; //录像时间段2开始时间
	FRAME_TIME end_time2; //录像时间段2结束时间
	unsigned long total_space; //总容量
	unsigned long remain_space; //剩余容量
	unsigned long error_flag; //硬盘状态 0：normal 1：error
	unsigned long index; //唯一分区标识，0~3：磁盘号；4~6：分区号；7：标识0为本地 1为远程
}DRIVER_INFO,*pDRIVER_INFO;

//是否支持新的录像及图片列表查询方式
typedef struct __RecordEnable_T
{
	BYTE isSupportNewA5Query;
	BYTE supportNewA5QueryType;
	BYTE reserved[2];
}RecordEnable_T;

//是否支持新的日志查询(XML格式返回数据)
typedef struct tagNewLogType
{
	char isSupportNewLogType;
	char reserved[3];
}NEW_LOG_TYPE_SUPPORT_T;

//图象水印能力
typedef struct __WaterMake_Enable_T
{
	char isSupportWM;	//1 支持；0 不支持
	char supportWhat;	//0：文字水印；1：图片水印；2：同时支持文字水印和图片水印
	char reserved[2];
}WaterMake_Enable_T;

//是否支持无线功能
typedef struct __Wireless_Enable_T
{
	char isWirelessEnable;
	char reserved[3];
}Wireless_Enable_T;

//! 预览图像设置
typedef struct __config_ex_capture_t
{
	BYTE		 iQuality;		// 画质
	BYTE		 iReserved[31];	// 保留
}CONFIG_EX_CAPTURE_T;

//音频输入检测配置
typedef struct _CONFIG_AUDIO_DETECT
{
	VER_CONFIG sVerConfig;		/*! 配置文件版本信息 */
	DWORD	AlarmMask;			/*! 报警联动输出掩码 */
	DWORD	RecordMask;			/*! 录象通道掩码 */
	DWORD	TimeDelay;			/*! 录象延时：10～300 sec */
	BYTE	iEnable; 			/*! [用来表示语音激励使能] */
	BYTE	TimePreRec;			/*! 预录时间 或者 预录开关 */
	BYTE	iValve;				/*! [用来表示阀值0---100] */
	BYTE	almReserved;		/*! 保留字段 */
	TSECT	sAlarmSects[ALARM_SECTS];	/*! 报警输出和提示的时间段 */
	DWORD	dwSectMask[ALARM_SECTS];	/*! 报警输出和提示的时间段的掩码 */
	DWORD	dwInputTypeMask;	/*! 报警输入类型掩码 */
	BYTE	dbTourEn;			/*! 是否允许报警轮循 */
	BYTE	Mailer;				/*! 发送邮件使能 1- 发送 0 - 不发 */
	BYTE	SendAlarmMsg;		/*! 发送报警信息（给报警服务器等） */	
	BYTE	adbReserved;		/*! 保留字段 */
} CONFIG_AUDIO_DETECT, *pCONFIG_AUDIO_DETECT;

typedef struct __sys_status_t
{
	BYTE	iVer;
	BYTE	iCPUOcc;
	BYTE	iDispStatus_VGA;
	BYTE	iDispStatus_TV;
	BYTE	iRev[60];
} SYS_STATUS_T;

typedef struct __channel_info_t
{
	BYTE	iVer;
	BYTE	iChannel;
	BYTE	iSubChannel;
	BYTE	iRev;
} CHANNEL_INFO_T;

//自动维护
typedef struct __CONFIG_AUTO_NET
{
	BYTE verConfig[8]; /*! 配置文件版本信息 */
	BYTE AutoRebootDay; /*! 0-从不, 1-每天, 2-每星期日, 3-每星期一,..... */
	BYTE AutoRebootTime; /*! 0-0:00 1-1:00,........23-:23:00 */
	BYTE AutoDeleteFilesTime; /*! 0-从不, 1-24H, 2-48H, 3-72H, 4-96H, 5-ONE WEEK, 6-ONE MONTH */
	BYTE reserved[13]; // 保留位
} CONFIG_AUTO_NET;

//Begin: new alarm, Add by yehao(10857) 2007-04-24
typedef struct tagTIMESECTION
{
	BOOL	bEnable;
	int		H1;
	int		M1;
	int		S1;
	int		H2;
	int		M2;
	int		S2;
} TIMESECTION, *LPTIMESECTION;

#define WORKSHEET_NUM 16

typedef struct tagWORKSHEET
{
	int		iName;
	TIMESECTION tsSchedule[N_WEEKS][N_TSECT];
} CONFIG_WORKSHEET;

typedef struct tagPTZ_LINK
{
	int iType;					/*! 联动类型 */
	int iValue;					/*! 联动序号 */
} PTZ_LINK, *LPPTZ_LINK;

typedef struct tagEVENT_HANDLER
{
	DWORD	dwRecord;
	int		iRecordLatch;
	DWORD	dwTour;
	DWORD	dwSnapShot;
	DWORD	dwAlarmOut;
	int		iAOLatch;
	PTZ_LINK	PtzLink[N_SYS_CH];

	BOOL	bRecordEn;
	BOOL	bTourEn;
	BOOL	bSnapEn;
	BOOL	bAlarmOutEn;
	BOOL	bPtzEn;
	BOOL	bTip;
	BOOL	bMail;
	BOOL	bMessage;
	BOOL	bBeep;
	BOOL	bVoice;
	BOOL	bFtp;

	DWORD	dwWorkSheet;
	
	DWORD	dwMatrix;				/*!< 矩阵掩码 */
	BOOL	bMatrixEn;				/*!< 矩阵使能 */
	BOOL	bLog;					/*!< 日志使能，目前只有在WTN动态检测中使用 */
	int		iEventLatch;			/*!< 联动开始延时时间，s为单位 范围是0－－15 默认值是0*/
	BOOL	bMessagetoNet;			/*!< 消息上传给网络使能 */
	DWORD	wiAlarmOut; 			/*!< 无线报警输出 */
	BYTE	bMMSEn;					/*!< 短信报警使能  */
	BYTE	SnapshotTimes;          /*!< 抓图张数 */
	char	dReserved[22];			/*!< 保留字节 */
} EVENT_HANDLER, *LPEVENT_HANDLER;

//本地报警和网络报警事件：
typedef struct tagCONFIG_ALARM_X
{
	BOOL	bEnable;
	int		iSensorType;
	EVENT_HANDLER	hEvent;
} CONFIG_ALARM_X, *LPCONFIG_ALARM_X;

//动态监测事件
#define MD_REGION_ROW	32
typedef struct tagCONFIG_MOTIONDETECT
{
	BOOL	bEnable;
	int		iLevel;
	int		iRegion[MD_REGION_ROW];	//每行使用二进制串
	EVENT_HANDLER	hEvent;
} CONFIG_MOTIONDETECT, *LPCONFIG_MOTIONDETECT;

//视频丢失事件
typedef struct tagCONFIG_LOSSDETECT
{
	BOOL	bEnable;
	EVENT_HANDLER	hEvent;
} CONFIG_LOSSDETECT, *LPCONFIG_LOSSDETECT;

//视频遮挡事件
typedef struct tagCONFIG_BLINDDETECT
{
	BOOL	bEnable;
	int		iLevel;			//灵敏度，1 ~ 6
	EVENT_HANDLER	hEvent;
} CONFIG_BLINDDETECT, *LPCONFIG_BLINDDETECT;

#define ALARM_MAX_NAME 64
//!(无线)红外报警
//无线遥控器配置：
typedef struct tagCONFIG_WI_REMOTE_CONTROL
{
	BYTE  address[ALARM_MAX_NAME];// 遥控器地址
	BYTE  name[ALARM_MAX_NAME];// 遥控器名称
	BYTE  reserved[32]; //保留字段
}CONFIG_ROBOT;

//无线报警输出配置：
typedef struct tagCONFIG_WI_ALARM_OUT
{
	BYTE  address[ALARM_MAX_NAME];//报警输出地址
	BYTE  name[ALARM_MAX_NAME];//报警输出名称
	BYTE  reserved[32]; //保留字段
}WI_ALARM_OUT;

typedef struct Infrared_Alarm_Info
{
	char				alarmInAddress[ALARM_MAX_NAME];		//报警器输入的地址
	int					alarmInPattern;						//报警器输入波形
//	char				remoteAddress[ALARM_MAX_NAME];		//遥控器的有效地址
//	char				alarmOutAddress[ALARM_MAX_NAME];	//报警输出的地址
	char				res[ALARM_MAX_NAME*2];
	int					alarmOutPattern;					//报警输出波形
	int					iSensorType;						//外部设备传感器类型常开 or 常闭
	char				alarmName[ALARM_MAX_NAME];			//报警输入名称
	BOOL				enable;								//报警输入使能
	int					defendEfectTime;					//布撤防延时时间，在此时间后该报警输入有效
	int					defendAreaType;						//防区类型
	int					alarmSmoothTime;					//报警平滑时间，即在此时间内如果只有一个报警输入连续输入两次则忽略掉后面一次
	EVENT_HANDLER		hEvent;								//报警联动
} CONFIG_ALARM_INFRARED;

//!音频事件结构
typedef struct tagCONFIG_AUDIODETECT_EVENT
{
	//!处理开启
	BOOL enable;
	int  Volume_min;
	int  Volume_max;
	//!处理参数
	EVENT_HANDLER hEvent;
} CONFIG_ALARM_AUDIO;

//解码器报警事件
#define ALARMDEC_OUT_SLOTS_NUM	8
#define ALARMDEC_IN_SLOTS_NUM	8

typedef struct tagCONFIG_ALARMDECODER
{
	int		iAddress;
	BOOL	bEnable;
	int		iOutSlots[ALARMDEC_OUT_SLOTS_NUM];//映射到本地报警输出的通道号，可以重叠，但不能断开
	CONFIG_ALARM_X	InSlots[ALARMDEC_IN_SLOTS_NUM];
} CONFIG_ALARMDECODER, *LPCONFIG_ALARMDECODER;

//硬盘容量不足事件
typedef struct tagCONFIG_DISKNOSPACE
{
	BOOL	bEnable;
	int		iLowerLimit;	//0 ~ 99
	EVENT_HANDLER	hEvent;
} CONFIG_DISKNOSPACE, *LPCONFIG_DISKNOSPACE;

typedef struct tagCONFIG_RECORD_NEW
{
	int iPreRecord;			/*!< 预录时间，为零时表示关闭 */	
	BOOL bRedundancy;		/*!< 冗余开关 */
	BOOL bSnapShot;			/*!< 快照开关 */	
	BYTE iWsName;			/*!< 选中的工作表名 */
	BYTE byRecordType;      /*录像类型0-主码流 1-辅码流1 2-辅码流2 3-辅码流3*/
	BYTE byReserved[2];     //保留
} CONFIG_RECORD_NEW;

//硬盘出错事件
//无硬盘事件
//断网事件
typedef struct tagCONFIG_GENERIC_EVENT
{
	BOOL	bEnable;
	EVENT_HANDLER	hEvent;
} CONFIG_GENERIC_EVENT, *LPCONFIG_GENERIC_EVENT;

typedef struct tagCONFIG_STORAGE_NUMBER_EVENT
{
	//!处理开启
	BOOL enable;
	//!客户配置的硬盘个数
	int  iHddNumber;			
	EVENT_HANDLER handler;
} CONFIG_STORAGE_NUMBER_EVENT;

// 全景切换报警配置
typedef struct tagCONFIG_PANORAMA_SWITCH
{
	BOOL			bEnable;
	EVENT_HANDLER	hEvent;
} CONFIG_PANORAMA_SWITCH, *LPCONFIG_PANORAMA_SWITCH;

// 失去焦点报警配置
typedef struct tagCONFIG_LOST_FOCUS
{
	BOOL			bEnable;
	EVENT_HANDLER	hEvent;
} CONFIG_LOST_FOCUS, *LPCONFIG_LOST_FOCUS;

//End: yehao(10857) 2007-04-24

//语音对讲
typedef struct __dialog_caps_t
{
	// 支持的音频数据个数的数量
	BYTE iAudioTypeNum;		/*!< 是否支持语音对讲 */
	// 保留位
	BYTE iRev[31];
} DIALOG_CAPS;

typedef struct __audio_attr_t
{
	//支持的编码类型
	WORD iAudioType;			// 1:PCM, 2:G711a, 3:AMR ,4:G711u 5:G726
	//位
	WORD iAudioBit;			// 用实际的值表示， 如8位 则填值为8
	//支持的采样率
	DWORD dwSampleRate;			// 用实际的值表示， 如8k 则填值为8000
	//保留位
	BYTE	iRev[64];
}AUDIO_ATTR_T;

//本地视频矩阵控制配置
#define MATRIX_MAXOUT 4
#define TOUR_GROUP_NUM 6

typedef struct
{
	BYTE VideoOut[TOUR_GROUP_NUM]; /*!< 视频输出 0代表--，g_nCapture+1代表
	all*/
	int Interval; /*!< 轮巡间隔 */
	BYTE EnableTour; /*!< 是否轮巡 */
	BYTE AlarmChannel; /*!< 联动报警通道 0代表--，g_nCapture+1代
	表1-4, g_nCapture+2代表5-8*/
	BYTE Reserve[2];
}VIDEOGROUP;

typedef struct
{
	BYTE VideoMatrixVersion[8]; /*!< 配置文件版本信息 */
	VIDEOGROUP VideoGroup[MATRIX_MAXOUT]; /*!< 六组数据视频矩阵输出 */
}CONFIG_VIDEO_MATRIX;

//卡存储信息结构
typedef struct __card_storage_info
{
	BYTE Version; //结构版本信息
	BYTE Reserved[3];
	DWORD DevNum; //设备数量
	DWORD DevPort; //接口数量
	DWORD DevBad; //故障设备掩码
	DWORD DevCap[32]; //总容量
}CARD_STORAGE_INFO, *pCARD_STORAGE_INFO;

typedef struct __card_storage_device_info
{
	BYTE Version; //结构版本信息
	BYTE Reserved[3];
	DWORD DriverType; //驱动器类型
	DWORD TotalSpace; //总容量(字节)
	DWORD RemainSpace; //剩余容量(字节)
	BYTE DriverName[64]; //驱动器名称
}CARD_STORAGE_DEVICE_INFO,*pCARD_STORAGE_DEVICE_INFO;

//刻录机信息
typedef struct __BACKUP_DEVICE  
{
	struct __BACKUP_DEVICE		*prev;			/* pointer to previous device */
	struct __BACKUP_DEVICE		*next;			/* pointer to next device */
	
	char		name[32];		/* device name */
	
	int			type;				/* device type, usb disk or cdrw */
	int			bus;				/* bus type, usb or ide */
	
	unsigned int	capability;	/* device capacity(KB) */
	unsigned int	remain;		/* device remain space(KB) */
	char		directory[128];	/* attached dir */
	int			flag;				/* reserved */
	
	void		*priv;			/* pointer to private data */
}BACKUP_DEVICE;

//FTP structure
#define	MAX_USERNAME_LEN	64   //10
#define	MAX_PASSWORD_LEN	64    //10
#define	MAX_CHANNEL_NUM		16
#define	DAYS_WEEK			7
#define	TIME_SECTION		2
#define FTP_MAX_PATH		240

typedef struct __MD_ALARM_FTP_Set
{
	struct
	{
		TSECT	m_tSect;
		int		m_MdEn;// 动态检测
		int		m_AlarmEn;//外部报警
		int		m_TimerEn;//普通录像
		int		m_Rev[4];
	} m_Period[TIME_SECTION];
} CONFIG_MD_ALRAM_FTP_SET;

//FTP
typedef struct __General_Protocol_FTP_set
{
	int				m_isEnable;		//是否启用
	unsigned long	m_unHostIP;		//主机IP
	unsigned short	m_nHostPort;		//主机端口
	char 			m_protocol;			//0-FTP 1-SMB
	char			m_NASVer;				//网络存储服务器版本0=老的FTP(界面上显示时间段),1=NAS存储（界面上屏蔽时间段）
	char			m_cDirName[FTP_MAX_PATH];			//FTP目录路径
	char			m_cUserName[MAX_USERNAME_LEN];	//用户名
	char			m_cPassword[MAX_PASSWORD_LEN];	//密码
	int				m_iFileLen;					//文件长度
	int				m_iInterval;					//相邻文件时间间隔
	CONFIG_MD_ALRAM_FTP_SET	m_stMdAlarmSet[MAX_CHANNEL_NUM][DAYS_WEEK];
	char 			m_Rev[128];
} CONFIG_FTP_PROTO_SET;

//多媒体能力
typedef struct __main_frame_caps
{
	DWORD Compression;
	DWORD ImageSize;
}MAIN_FRAME_CAPS;

enum capture_size_t {
	CAPTURE_D1,
	CAPTURE_HD1,	
	CAPTURE_BCIF,
	CAPTURE_CIF,
	CAPTURE__QCIF,	
	CAPTURE_VGA,	
	CAPTURE_QVGA,
	CAPTURE_SVCD,
	CAPTURE_QQVGA,
	CAPTURE_UXGA,
	CAPTURE_NR,
};

typedef struct tagCAPTURE_EXT_STREAM
{
	DWORD ExtraStream;	//按位表示设备支持的功能，
						//第一位表示支持主码流
						//第二位表示支持辅码流1
						//第三位表示支持辅码流2
						//第五位表示支持jpg抓图
	DWORD CaptureSizeMask[64];	//表示主码流为各分辨率时，支持的辅码流分辨率掩码。
}CAPTURE_EXT_STREAM, *pCAPTURE_EXT_STREAM;	

typedef struct tagCAPTURE_DSPINFO
{
	unsigned int nMaxEncodePower; /*- DSP 支持的最高编码能力 -*/
	unsigned short nMaxSupportChannel; /*- DSP 支持最多输入视频通道数 -*/
	unsigned short bChannelMaxSetSync; /*- DSP 每通道的最大编码设置是否同步 0-
									   不同步, 1 -同步 -*/
}CAPTURE_DSPINFO, *PCAPTURE_DSPINFO;

//版本1：2008.3.13增加了分辨率类型CAPTURE_SIZE_QQVGA
/// 捕获分辨率类型
/*
enum capture_size_t {
	CAPTURE_SIZE_D1,		///< 720*576(PAL)	720*480(NTSC)
		CAPTURE_SIZE_HD1,		///< 352*576(PAL)	352*480(NTSC)
		CAPTURE_SIZE_BCIF,		///< 720*288(PAL)	720*240(NTSC)
		CAPTURE_SIZE_CIF,		///< 352*288(PAL)	352*240(NTSC)
		CAPTURE_SIZE_QCIF,		///< 176*144(PAL)	176*120(NTSC)
		CAPTURE_SIZE_VGA,		///< 640*480(PAL)	640*480(NTSC)
		CAPTURE_SIZE_QVGA,		///< 320*240(PAL)	320*240(NTSC)
		CAPTURE_SIZE_SVCD,		///< 480*480(PAL)	480*480(NTSC)
		CAPTURE_SIZE_QQVGA,		///< 160*128(PAL)	160*128(NTSC)
		CAPTURE_SIZE_NR			///< 枚举的图形大小种类的数目。
};
*/
/// 捕获辅助码流支持特性结构
typedef struct _CAPTURE_EXT_STREAM_NEW
{
	DWORD ExtraStream;						///< 用channel_t的位来表示支持的功能。
	DWORD CaptureSizeMask[64];	///< 每一个值表示对应分辨率支持的辅助码流。
}CAPTURE_EXT_STREAM_NEW, *pCAPTURE_EXT_STREAM_NEW;	

//U网通平台配置
typedef struct _intervideo_ucom_chn
{
    unsigned char iChnEn;
    unsigned char iRes[3];
    unsigned char strChnId[32];
}INTERVIDEO_UCOM_CHN;

typedef struct __config_intervideo_ucom
{
	unsigned char iFuncEnable;		// 接入功能使能与否 0 - 使能
	unsigned char iAliveEnable;	// 心跳使能与否
	unsigned short iAlivePeriod;	// 心跳周期
	unsigned long ServerIp;		// CMS的IP
	unsigned short ServerPort;// CMS的Port
	unsigned short __Res;
    unsigned char strRegPwd[16]; //注册密码
	unsigned char strDeviceId[32];
    INTERVIDEO_UCOM_CHN  sChnInfo[N_SYS_CH];//通道id,en
	unsigned char strUserName[32];
	unsigned char strPassWord[32];
	unsigned short __Resv[256];
}CONFIG_INTERVIDEO_UCOM;

//上海贝尔阿尔卡特平台配置
typedef struct asbWEB_VSP_CONFIG
{
    unsigned short usCompanyID[2];  /* 公司ID,数值,不同的第三方服务公司,考虑4字节对齐,目前只用数组第一个 */
    char szDeviceNO[32];  /* 前端设备序列号，字符串, 包括'\0'结束符共32byte */
    char szVSName[32];  /* 前端设备名称，字符串, 包括'\0'结束符共16byte */
    char szVapPath[32];  /* VAP路径 */
    unsigned short usTcpPort;  /* TCP 端口,数值: 数值 1~65535 */    
    unsigned short usUdpPort;  /* UDP 端口,数值: 数值 1~65535 */    
    bool bCsEnable[4];  /* 中心服务器使能标志, 数值: true使能, false不使能,考虑4字节对齐,目前只用数组第一个 */
    char szCsIP[16];  /* 中心服务器IP地址, 字符串, 包括'\0'结束符共16byte */
    unsigned short usCsPort[2];  /* 中心服务器端口,数值: 数值 1~65535,考虑4字节对齐,目前只用数组第一个 */    
    bool bHsEnable[4];  /* 心跳服务器使能标志, 数值: true使能, false不使能,考虑4字节对齐,目前只用数组第一个 */
    char szHsIP[16];  /* 心跳服务器IP地址, 字符串, 包括'\0'结束符共16byte */
    unsigned short usHsPort[2];  /* 心跳服务器端口,数值: 数值 1~65535,考虑4字节对齐,目前只用数组第一个 */ 
    int iHsIntervalTime;  /* 心跳服务器间隔周期,数值(单位:秒) */ 
    bool bRsEnable[4];  /* 注册服务器使能标志, 数值: true使能, false不使能,考虑4字节对齐,目前只用数组第一个 */
    char szRsIP[16];  /* 注册服务器IP地址, 字符串, 包括'\0'结束符共16byte */
    unsigned short usRsPort[2];  /* 注册服务器端口,数值: 数值 1~65535,考虑4字节对齐,目前只用数组第一个 */
    int iRsAgedTime;  /* 注册服务器有效时间,数值(单位:小时) */
    char szAuthorizeServerIp[16];  /* 鉴权服务器IP */
    unsigned short usAuthorizePort[2];  /* 鉴权服务器端口,考虑4字节对齐,目前只用数组第一个 */
    char szAuthorizeUsername[32];  /* 鉴权服务器帐号 */
    char szAuthorizePassword[36];  /* 鉴权服务器密码 */
    
    char szIpACS[16];  /* ACS(自动注册服务器) IP */
    unsigned short usPortACS[2];  /* ACS Port,考虑4字节对齐,目前只用数组第一个 */
    char szUsernameACS[32];  /* ACS用户名 */
    char szPasswordACS[36];  /* ACS密码 */
    bool bVideoMonitorEnabled[4];  /* DVS是否定期上报前端视频信号监控信息, 数值: true使能, false不使能 */
    int iVideoMonitorInterval;  /* 上报周期（分钟） */
    
    char szCoordinateGPS[64];  /* GPS坐标 */
    char szPosition[32];  /* 设备位置 */
    char szConnPass[36];  /* 设备接入码 */
}WEB_VSP_CONFIG;

typedef struct tagWEB_NSS_CONFIG
{
    unsigned short server_port;               //服务器端口, 数值, 数值1~65535
    char server_ip[32];      //服务器IP地址, 字符串, 包括'\0'结束符共32byte
    char device_serial_no[32]; //前端设备序列号，字符串,包括'\0'结束符共32byte
    char username[32];
    char passwd[50];
}WEB_NSS_CONFIG;

//add by cqs 2008-05-09 Watchnet抓包新结构
#define SNIFFER_FRAMEID_NUM 6     ////6个FRAME ID 选项
#define SNIFFER_CONTENT_NUM 4     /////每个FRAME对应的4个抓包内容
#define MAX_PROTOCOL_NAME_LENGTH 20
#define SNIFFER_GROUP_NUM 4       ///4组抓包设置

typedef enum __DISPLAY_POSITION
{
	DISPLAY_TOP = 0,
		DISPLAY_BOTTOM
}DISPLAY_POSITION;    //叠加位置，分为左上和左下

typedef struct __SNIFFRT_CONFIG_IP
{	
	unsigned long SnifferSrcIP; //抓包源地址
	int SnifferSrcPort;  //抓包源端口
	unsigned long SnifferDestIP; //抓包目标地址
	int SnifferDestPort; //抓包目标端口
} SNIFFER_IP;

typedef struct __sniffer_frameid
{
	int		Offset; //标志位的位偏移
	int		Length; //标志位的长度
	char	Key[16]; //标志位的值
} SNIFFER_FRAMEID;

typedef struct __SNIFFER_CONTENT
{
	int  Offset;    ///标志位的位偏移
	int  Offset2;    //目前没有应用
	int  Length;  //标志位的长度
	int  Length2;   //目前没有应用
	char  KeyTitle[24];  //标题的值
} SNIFFER_CONTENT;

typedef  struct _CONFIG_ATM_NET    ////每组抓包对应的配置结构
{
	SNIFFER_IP  snifferIp;   ////抓包IP设置
	BOOL generalProtocolEnable;  /////协议使能
	BOOL dataScopeEnable;     /////目前没有应用
	char Protocol[MAX_PROTOCOL_NAME_LENGTH];
	SNIFFER_FRAMEID  snifferFrameId[SNIFFER_FRAMEID_NUM];   ////6个FRAME ID 选项
	SNIFFER_CONTENT snifferContent[SNIFFER_FRAMEID_NUM][SNIFFER_CONTENT_NUM];  /////每个FRAME对应的4个抓包内容
	int displayPosition;       /////显示位置
	int recdChannelMask;   //通道掩码
} CONFIG_ATM_SNIFFER;

typedef struct __General_Protocol_NTP_set
{
	char				Enable;					//是否启用
	char				Reserved;
	unsigned short		unHostPort; 			//NTP服务器默认端口为123
	unsigned long		unHostIP;				//主机IP
	int					nUpdateInterval;		//更新时间
	int					iTimeZone;				//时区在-12~13,保存的都为0-25
}CONFIG_NTP_PROTO_SET;
#define MAX_PATH_STOR 240
//add by cqs 2008-07-07 存储位置结构
typedef struct tagCONFIG_STORAGE 
{
	DWORD dwLocalMask;      //本地存储掩码							|注:掩码高16位表示抓图，低16位表示录象
	DWORD dwMobileMask;     //可移动存储							|	0x0001		/*!< 系统预录事件	*/
	int  RemoteType;        //远程存储类型 0: Ftp  1: Smb			|	0x0002		/*!< 定时录像事件	*/
	DWORD dwRemoteMask;     //远程存储								|	0x0004		/*!< 动检录像事件	*/
	DWORD dwRemoteSecondSelLocal; //远程异常时本地存储				|	0x0008		/*!< 报警录像事件	*/	
	DWORD dwRemoteSecondSelMobile; //远程异常时可移动存储			|	0x0010		/*!< 卡号录像事件	*/
	char SubRemotePath[MAX_PATH_STOR];  //远程目录, 其中长度为260	|	0x0020		/*!< 手动录像事件	*/
	DWORD dwFunctionMask;        //功能屏蔽位，bit0 = 1:屏蔽抓图事件触发存储位置功能
    DWORD Res[19];
}CONFIG_STORAGE_STATION;

//视频前端采集能力
typedef struct __prevideo_capture
{
	unsigned char iCifCaps; 	// 大小
	unsigned char iFrameCaps;	// 帧率
	unsigned char reserved[30];	// 保留字节
}Prevideo_Capture;

//视频属性信息
typedef struct __sysattr_video_caps
{
	unsigned int iCifCaps; 		// 大小
	unsigned int iTypeCaps;		// 编码
	unsigned int iFrameCaps;	// 帧率
	unsigned int iQualityCaps;	// 画质
	unsigned char iCifFrames;	// 每个芯片编码总帧数 单位：cif/s
	unsigned char iWaterMask;	// 支持水印情况，按位表示，１位：字符，２位：图片
	unsigned char iEncodeCap;	// 编码能力算法标示
	unsigned char _iRev_02;		// 保留
	unsigned int iReserved_01;	// 保留
	unsigned int iReserved_02;	// 保留
	unsigned int iReserved_03;	// 保留
	unsigned char iStandard;	// 制式
	unsigned char iRev_00;		// 保留
	unsigned char iRev_01;		// 保留
	unsigned char iRev_02;		// 保留
}SYSATTR_VIDEO_CAPS_T;


//! 导出配置文件
typedef struct 
{
	char			FileName[32];			// 配置文件名称
	unsigned long	FileTotalLen;			// 文件总长度，单位字节
	unsigned long	ThisFrameNo;			// 本数据包的序号
	unsigned long	ThisFrameLen;			// 本数据包有效信息的长度
	unsigned char	DataBuffer[1024];		// 数据缓冲长度1k
} CFG_FILE_INFO;

typedef struct 
{
	unsigned long	AllConfigLen;			// 配置信息的总长度
	unsigned short	TotalConfigFileNum;		// 总的配置文件数量
	unsigned short	CurrentFileNo;			// 当前配置文件的编号
	CFG_FILE_INFO	ConfigFile;
} CFG_INFO;									// 总长度1076字节


typedef struct  
{
	int				type;					// 修改参数类别
	int				nResultCode;			// 返回码
	BOOL			bReboot;				// 重启标志
} CONFIG_ACK_INFO;


typedef struct  
{
	int				nEncoderNum;
	unsigned int	nTotalSize;
	unsigned int	nCurSize;
} NVD_PLAYBACK_EVENT;


//用户能力信息

enum EnAuthority
{
	DVRG_AUTHORITY_SYSSET			=  0,		// 控制面板		_01
	DVRG_AUTHORITY_SHUTDOWN			=  1,		// 关闭系统		_02
	DVRG_AUTHORITY_MONITOR			=  2,		// 监视			_03
	DVRG_AUTHORITY_MONITOR_01		=  3,		// 监视通道_01	_04
	DVRG_AUTHORITY_MONITOR_02		=  4,		// 监视通道_02	_05
	DVRG_AUTHORITY_MONITOR_03		=  5,		// 监视通道_03	_06
	DVRG_AUTHORITY_MONITOR_04		=  6,		// 监视通道_04	_07
	DVRG_AUTHORITY_MONITOR_05		=  7,		// 监视通道_05	_08
	DVRG_AUTHORITY_MONITOR_06		=  8,		// 监视通道_06	_09
	DVRG_AUTHORITY_MONITOR_07		=  9,		// 监视通道_07	_10
	DVRG_AUTHORITY_MONITOR_08		= 10,		// 监视通道_08	_11
	DVRG_AUTHORITY_MONITOR_09		= 11,		// 监视通道_09	_12
	DVRG_AUTHORITY_MONITOR_10		= 12,		// 监视通道_10	_13
	DVRG_AUTHORITY_MONITOR_11		= 13,		// 监视通道_11	_14
	DVRG_AUTHORITY_MONITOR_12		= 14,		// 监视通道_12	_15
	DVRG_AUTHORITY_MONITOR_13		= 15,		// 监视通道_13	_16
	DVRG_AUTHORITY_MONITOR_14		= 16,		// 监视通道_14	_17
	DVRG_AUTHORITY_MONITOR_15		= 17,		// 监视通道_15	_18
	DVRG_AUTHORITY_MONITOR_16		= 18,		// 监视通道_16	_19
	DVRG_AUTHORITY_REPLAY			= 19,		// 回放         _20
	DVRG_AUTHORITY_REPLAY_01		= 20,		// 回放通道_01	_21
	DVRG_AUTHORITY_REPLAY_02		= 21,		// 回放通道_02  _22
	DVRG_AUTHORITY_REPLAY_03		= 22,		// 回放通道_03  _23
	DVRG_AUTHORITY_REPLAY_04		= 23,		// 回放通道_04  _24
	DVRG_AUTHORITY_REPLAY_05		= 24,		// 回放通道_05  _25
	DVRG_AUTHORITY_REPLAY_06		= 25,		// 回放通道_06  _26
	DVRG_AUTHORITY_REPLAY_07		= 26,		// 回放通道_07  _27
	DVRG_AUTHORITY_REPLAY_08		= 27,		// 回放通道_08  _28
	DVRG_AUTHORITY_REPLAY_09		= 28,		// 回放通道_09  _29
	DVRG_AUTHORITY_REPLAY_10		= 29,		// 回放通道_10  _30
	DVRG_AUTHORITY_REPLAY_11		= 30,		// 回放通道_11  _31
	DVRG_AUTHORITY_REPLAY_12		= 31,		// 回放通道_12  _32
	DVRG_AUTHORITY_REPLAY_13		= 32,		// 回放通道_13  _33
	DVRG_AUTHORITY_REPLAY_14		= 33,		// 回放通道_14  _34
	DVRG_AUTHORITY_REPLAY_15		= 34,		// 回放通道_15  _35
	DVRG_AUTHORITY_REPLAY_16		= 35,		// 回放通道_16	_36
	DVRG_AUTHORITY_NETPREVIEW       = 36,       // NetPreview
	DVRG_AUTHORITY_NETPREVIEW_1     = 37,		// NetPreview_CH01
	DVRG_AUTHORITY_NETPREVIEW_2     = 38,		// NetPreview_CH02
	DVRG_AUTHORITY_NETPREVIEW_3     = 39,		// NetPreview_CH03
	DVRG_AUTHORITY_NETPREVIEW_4     = 40,		// NetPreview_CH04
	DVRG_AUTHORITY_NETPREVIEW_5     = 41,		// NetPreview_CH05
	DVRG_AUTHORITY_NETPREVIEW_6     = 42,		// NetPreview_CH06
	DVRG_AUTHORITY_NETPREVIEW_7     = 43,		// NetPreview_CH07
	DVRG_AUTHORITY_NETPREVIEW_8     = 44,		// NetPreview_CH08
	DVRG_AUTHORITY_NETPREVIEW_9     = 45,		// NetPreview_CH09
	DVRG_AUTHORITY_NETPREVIEW_10    = 46,		// NetPreview_CH10
	DVRG_AUTHORITY_NETPREVIEW_11    = 47,		// NetPreview_CH11
	DVRG_AUTHORITY_NETPREVIEW_12    = 48,		// NetPreview_CH12
	DVRG_AUTHORITY_NETPREVIEW_13    = 49,		// NetPreview_CH13
	DVRG_AUTHORITY_NETPREVIEW_14    = 50,		// NetPreview_CH14
	DVRG_AUTHORITY_NETPREVIEW_15    = 51,		// NetPreview_CH15
	DVRG_AUTHORITY_NETPREVIEW_16    = 52,		// NetPreview_CH16
	DVRG_AUTHORITY_RECORD			= 53,		// 录像			
	DVRG_AUTHORITY_BACKUP			= 54,		// 备份			
	DVRG_AUTHORITY_HARDISK			= 55,		// 硬盘管理     
	DVRG_AUTHORITY_PTZ				= 56,		// 云台控制		
	DVRG_AUTHORITY_ACCOUNT			= 57,		// 用户帐号		
	DVRG_AUTHORITY_SYSINFO			= 58,		// 系统信息     
	DVRG_AUTHORITY_ALARM			= 59,		// 报警输IO/配置
	DVRG_AUTHORITY_CONFIG			= 60,		// 系统设置     	
	DVRG_AUTHORITY_LOG_QUERY		= 61,		// 查询日志     
	DVRG_AUTHORITY_LOG_DEL			= 62,		// 删除日志		
	DVRG_AUTHORITY_UPDATE			= 63,		// 系统升级		
	DVRG_AUTHORITY_CONTROL			= 64,		// 控制权		
	DVRG_AUTHORITY_AUTOMAINTAIN		= 65,		// 自动维护		
	DVRG_AUTHORITY_GENERALCONF		= 66,		// 普通设置
    DVRG_AUTHORITY_ENCODECONF		= 67,		// 编码设置
    DVRG_AUTHORITY_RECORDCONF		= 68,		// 录像设置
    DVRG_AUTHORITY_COMCONF			= 69,		// 串口设置
    DVRG_AUTHORITY_NETCONF			= 70,		// 网络设置
    DVRG_AUTHORITY_ALARMCONF		= 71,		// 报警设置
    DVRG_AUTHORITY_VIDEOCONF		= 72,		// 视频检测
    DVRG_AUTHORITY_PTZCONF			= 73,		// 云台配置
    DVRG_AUTHORITY_OUTPUTCONF		= 74,		// 输出模式
    DVRG_AUTHORITY_DEFAULTCONF		= 75,		// 恢复默认
	DVRG_AUTHORITY_FORMAT           = 76,       // 格式化

	DVRG_AUTHORITY_ITEMS			,			// 仅供计数用
};

typedef struct  
{
	BYTE bAuthorityInfo[MAX_AUTHORITY_NUM];
//	DWORD dwReserved[256];
}DVR_AUTHORITY_INFO;

//视频OSD叠加配置
#define VIDEO_OSD_NAME_NUM 64		// 叠加的名称长度，目前支持32个英文，16个中文
#define VIDEO_CUSTOM_OSD_NUM 8		// 支持的自定义叠加的数目，不包含时间和通道
//视频物件结构
typedef struct { 

	DWORD	rgbaFrontground;		/*!< 物件的前景RGB，和透明度 */	
	DWORD	rgbaBackground;			/*!< 物件的后景RGB，和透明度*/	
	RECT	rcRelativePos;			/*!< 物件边距与整长的比例*8191 */	
	BOOL	bPreviewBlend;			/*!< 预览叠加 */	
	BOOL	bEncodeBlend;			/*!< 编码叠加 */
} DVR_VIDEO_WIDGET;

typedef struct tagCONFIG_VIDEOITEM
{
	DVR_VIDEO_WIDGET   OSD_POS; ///< OSD叠加的位置和背景色
	char OSD_NAME[VIDEO_OSD_NAME_NUM]; ///< OSD叠加的名称
}DVR_CONFIG_VIDEOITEM;
 
//! 每个通道的OSD信息
typedef struct tagCONFIG_VIDEOOSD
{
	DVR_CONFIG_VIDEOITEM TimeTitle;      ///< 时间标题
	DVR_CONFIG_VIDEOITEM ChannelTitle; ///< 通道标题
	DVR_CONFIG_VIDEOITEM OSD_TITLE[VIDEO_CUSTOM_OSD_NUM]; ///< 自定义叠加的OSD
}DVR_CONFIG_VIDEOOSD;

//设置重要录像信息
typedef struct _REC_FB						        /*1*/
{
	unsigned char usCardNo	:4;					/* 卡号 */
	unsigned char usAorV	:2;				    /* 01：音频；10：视频；00：图片 */
	unsigned char usAlarm	:2;					/* 00：普通文件；01：报警文件,10：动态检测报警；11：媒体文件中断 */
}REC_FS;

typedef struct _REC_DIRINFO
{
	REC_FS		  DirInfo;					//卡信息
	FRAME_TIME	      DirStartTime;				//开始时间
	FRAME_TIME	      DirEndTime;				//结束时间
	unsigned int  FileSize;
	unsigned int  StartCluster;
	unsigned char DriveNo;
    unsigned char HInt;
	unsigned char ImportantRecID;			//0:普通录像 1:重要录像				
	unsigned char Reserved;					//保留
}IMPORTANT_REC_DIRINFO;


typedef struct __DEVICE_MESS_CALLBACK 
{
	void*		fun;
	void*		data;
}DEVICE_MESS_CALLBACK;

typedef struct __MESS_CALLBACK_DATA 
{
	int			nResultCode;
	char		*pBuf;
	int			nBufLen;
	LONG		lOperateHandle;
	void*		userparam;
	void*		cbMessFunc;
	void*		userdata;
	void*		pRecvEvent;
	int			*pRetLen;
	int			*pResult;
	int         nDvrRestart;
	int			*pDvrRestart;	
} MESS_CALLBACK_DATA;

// 请求包：控制TV
typedef struct __REQUEST_DEC_CTRLTV 
{
	int			nMonitorID;
	int			nSplitType;
	BYTE		*pEncoderChannel;
	int			nBufLen;
	BOOL		bEnable;
} REQUEST_DEC_CTRLTV;

// 请求包：切换TV图像
typedef struct __REQUEST_DEC_SWITCHTV 
{
	int			nEncoderID;
	char		szDevIp[64];
	WORD		wDevPort;
	WORD		wEnable;
	char		szDevUser[16];
	char		szDevPwd[16];
	int			nDevChannel;
	int			nStreamType;
	BYTE		byConnType;
	BYTE		byWorkMode;
	WORD		wListenPort;
	DWORD		dwProtoType;
	char		szDevName[128];
	BYTE		byVideoInType;
	BYTE        bySnapMode;
	BYTE        byManuFactory;
	BYTE        byDeviceType;
} REQUEST_DEC_SWITCHTV;

// 请求包：增加画面组合
typedef struct __REQUEST_DEC_ADD_COMBIN 
{
	int			nMonitorID;
	int			nSplitType;
	BYTE		*pEncoderChannnel;
	int			nBufLen;
} REQUEST_DEC_ADD_COMBIN;

// 请求包：删除画面组合
typedef struct __REQUEST_DEC_DEL_COMBIN 
{
	int			nMonitorID;
	int			nCombinID;
} REQUEST_DEC_DEL_COMBIN;

// 请求包：修改画面组合
typedef struct __REQUEST_DEC_SET_COMBIN 
{
	int			nCombinID;
	int			nMonitorID;
	int			nSplitType;
	BYTE		*pEncoderChannnel;
	int			nBufLen;
} REQUEST_DEC_SET_COMBIN;

// 请求包：设置解码器轮巡
typedef struct __REQUEST_DEC_SET_TOUR
{
	int			nMonitorID;
	int			nTourTime;
	BYTE		*pTourCombin;
	BYTE		*pCombinState;
	int			nCombinCount;
} REQUEST_DEC_SET_TOUR;

// 请求包：设置TV输出使能
typedef struct __REQUEST_DEC_SET_TVENABLE
{
	BYTE		*pSetTVEnable;
	int			nBuflen;
} REQUEST_DEC_SET_TVENABLE;

// 请求包：按文件回放前端设备
typedef struct __REQUEST_DEC_DEVBYFILE 
{
	int			nEncoderID;
	char		szDevIp[32];
	WORD		wDevPort;
	WORD		wEnable;
	char		szDevUser[16];
	char		szDevPwd[16];
	unsigned int nChannelID;
	unsigned int nFileSize;
    unsigned int driveno;
    unsigned int startcluster;
	BYTE		nRecordFileType;
	BYTE		bImportantRecID;
	BYTE		bHint;
	BYTE		bReserved;
	FRAME_TIME		startTime;
    FRAME_TIME		endTime;
} REQUEST_DEC_DEVBYFILE;

// 请求包：按时间回放前端设备
typedef struct __REQUEST_DEC_DEVBYTIME 
{
	int			nEncoderID;
	char		szDevIp[32];
	WORD		wDevPort;
	WORD		wEnable;
	char		szDevUser[16];
	char		szDevPwd[16];
	unsigned int nChannelID;
    FRAME_TIME		startTime;
    FRAME_TIME		endTime;
} REQUEST_DEC_DEVBYTIME;

// 请求包：解码器回放控制
typedef struct __REQUEST_DEC_SET_PLAYBACK
{
	int			nType;
	int			nEncoderNum;
	int			nValue;
} REQUEST_DEC_SET_PLAYBACK;

// 请求包：修改IP地址过滤
typedef struct __REQUEST_SET_IPFILTER 
{
	int			nEnable;
	int			nType;
	int			nBlackIpNum;
	char		szBlackIp[200][16];
	int			nTrustIpNum;
	char		szTrustIp[200][16];
} REQUEST_SET_IPFILTER;

// 请求包：增加IP地址过滤
typedef struct __REQUEST_ADD_IPFILTER 
{
	int			nEnable;
	int			nType;
	char		szIp[16];
	char		szListStart[16];
	char		szListEnd[16];
} REQUEST_ADD_IPFILTER;

// 请求包：修改语音对讲编码配置
typedef struct __REQUEST_SET_TALKENCODE 
{
	int			nCompression;//压缩格式
	int			nFrequency;  //采样频率
	int			nDepth;		//采样深度
	int			nPacketPeriod;//打包周期
	int			nMode;		//编码模式
} REQUEST_SET_TALKENCODE;

// 请求包：修改录像打包长度配置
typedef struct __REQUEST_SET_RECORDLEN 
{
	int			nType;		//按时间还是按大小0:按时间，1：按大小
	int			nValue;		//录像文件大小KB
} REQUEST_SET_RECORDLEN;

// 请求包：修改MMS
typedef struct __REQUEST_SET_MMS
{
	int			nEnable;
	int			nReceiverNum;
	char		szReceiver[100][32];
	unsigned char bType;
	char		SZTitle[32*2+1];
} REQUEST_SET_MMS;

// 请求包：修改SMSACTIVATION
typedef struct __REQUEST_SET_SMSACTIVATION
{

	int			nEnable;
	int			nSenderNum;
	char		szSender[100][32];
} REQUEST_SET_SMSACTIVATION;

// 请求包：修改DIALINACTIVATION
typedef struct __REQUEST_SET_DIALINACTIVATION
{
	int			nEnable;
	int			nCallerNum;
	char		szCaller[100][32];
} REQUEST_SET_DIALINACTIVATION;

typedef struct __REQUEST_SET_BURNFILE
{
	DWORD		dwConID;
	char		szFileName[MAX_PATH_STOR];
	int			nFileSize;
}REQUEST_SET_BURNFILE;

// 请求包：修改VideoOut参数
typedef struct __REQUEST_SET_VIDEOOUT 
{
	int        nVideoStander; //视频制式
	int        nImageSize;//分辨率
}REQUEST_SET_VIDEOOUT;

/* 登陆属性 */
typedef struct __login_attribute
{
	/* 是否必须登陆设备方可使用， 0－要登陆，1－不必登陆 */
	BYTE	iShouldLogin;
	/* 支持的登陆用户名最大长度 */
	BYTE	iUNameMaxLen;
	/* 支持的密码最大长度 */
	BYTE	iUPwdMaxLen;
	/* 支持的加密方式 */
	BYTE	iEncryptType;/* 从低到高，按位表示加密方式：0-3des, */
	BYTE	iReserved[28];/* 保留 */
}LOGIN_ATTRIBUTE;

// 请求包：修改TV调节参数
typedef struct __REQUEST_SET_TVADJUST 
{
	int                 nTVID;
	int					nTop;							// 上测边距（0－100）
	int					nBotton;						// 下测边距（0－100）
	int					nLeft;							// 左测边距（0－100）
	int                 nRight;							// 右测边距（0－100）
}REQUEST_SET_TVADJUST;

//备份记录属性
typedef struct __BACKUP_RECORDFILE_INFO
{
	unsigned int	nChannel;							//通道号		
	BYTE			byType;								//录象文件类型  0：普通录象；1：报警录象；2：移动检测；3：卡号录象；4：图片
	FRAME_TIME			DirStartTime;						//开始时间
	FRAME_TIME			DirEndTime;							//结束时间
	unsigned int	nFileSize;	
	unsigned int	nDriveNo;
	unsigned int	nStartCluster;
    unsigned int	nHInt;
	unsigned int	nImportantRecID;					//0:普通录像 1:重要录像				
} BACKUP_RECORDFILE_INFO;

//请求包: 控制开始备份
typedef struct __REQUEST_BACKUP_RECORD
{
	char					szDeviceName[32];			//备份设备名称
	int						nRecordNum;					//备份记录数量
	BACKUP_RECORDFILE_INFO	stuRecordInfo[1024];		//备份记录信息
} REQUEST_BACKUP_RECORD;

// 请求包：解码器控制
typedef struct __REQUEST_DEC_CONTROL
{
	int			nType; //控制类型:0-tour
	int         nAction;
	int			nMonitorID;
} REQUEST_DEC_CONTROL;


//请求包：POS相关参数设置结构
typedef struct __REQUEST_POS_INFO
{
	int fontColor ; //0:default(white) 1:black 2:blue 3:red 4:white 5:green 6:brown 7:yellow 8:grey
	int fontSize ;  //0:8 1:16 2:24 3:32
	int fontX ;     //-1:aligh left -2:aligh right -3:align center >0:custom
	int fontY ;     //
	BYTE show ;     //0:hide 1:show
	int  holdTime ; //小票OSD的停留显示时间 , (5-300) 单位秒
	char posAlarmHostAddr[16] ; //小票的告警服务器地址
	int  posAlarmHostPort ;
	BYTE reserved[64] ;
} REQUEST_POS_INFO ;



#endif  //_DVR2CFG_H_

