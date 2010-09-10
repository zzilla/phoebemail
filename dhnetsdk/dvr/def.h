
#ifndef DVR_DEF_H
#define DVR_DEF_H

#define HEADER_SIZE  32

// 此枚举如果变动，则netsdk.h中也应做相应变动 linjy-2006-12-16
enum ServerType
{
	PRODUCT_NONE = 0,
    PRODUCT_DVR_NONREALTIME_MACE,	// 非实时MACE
	PRODUCT_DVR_NONREALTIME,			// 非实时
	PRODUCT_NVS_MPEG1,				// 网络视频服务器
	PRODUCT_DVR_MPEG1_2,				// MPEG1二路录像机
	PRODUCT_DVR_MPEG1_8,				// MPEG1八路录像机
	PRODUCT_DVR_MPEG4_8,				// MPEG4八路录像机
	PRODUCT_DVR_MPEG4_16,			// MPEG4十六路录像机
	PRODUCT_DVR_MPEG4_SX2,			// MPEG4视新十六路录像机
	PRODUCT_DVR_MEPG4_ST2,			// MPEG4视通录像机
	PRODUCT_DVR_MEPG4_SH2,			// MPEG4视豪录像机
    PRODUCT_DVR_MPEG4_GBE,			// MPEG4视通二代增强型录像机
    PRODUCT_DVR_MPEG4_NVSII,			// MPEG4网络视频服务器II代
	PRODUCT_DVR_STD_NEW,				// 新标准配置协议
	PRODUCT_DVR_DDNS,				// DDNS服务器
	PRODUCT_DVR_ATM,					// ATM机
	PRODUCT_NB_SERIAL,				// 二代非实时NB系列机器
	PRODUCT_LN_SERIAL,				// LN系列产品
	PRODUCT_BAV_SERIAL,				// BAV系列产品
	PRODUCT_SDIP_SERIAL,				// SDIP系列产品
	PRODUCT_IPC_SERIAL,				// IPC系列产品
	PRODUCT_NVS_B,					// NVS B系列
	PRODUCT_NVS_C,					// NVS H系列
	PRODUCT_NVS_S,					// NVS S系列
	PRODUCT_NVS_E,					// NVS E系列
	PRODUCT_DVR_NEW_PROTOCOL,		// 新协议
	PRODUCT_NVD_SERIAL,				// 解码器
	PRODUCT_DVR_N5,					// N5
	PRODUCT_DVR_MIX_DVR,				// 混合DVR
	PRODUCT_SVR_SERIAL,				// SVR
	PRODUCT_SVR_BS,					// SVR-BS
	PRODUCT_NVR_SERIAL,				// NVR
};

#endif // DVR_DEF_H

