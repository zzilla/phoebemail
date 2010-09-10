
#ifndef _AFKPLUGIN_H_
#define _AFKPLUGIN_H_

#include "afkinc.h"


/**************************************************************************** 
 * 插件类型定义
 * SDK采用插件设计, 很多实现尽量在插件中完成
 ****************************************************************************/
#define AFK_PLUGIN_DEVICEPROBE		1		/* 设备探测器 */
#define AFK_PLUGIN_STREAM_PARSER	2		/* 流解析器 */
#define AFK_PLUGIN_VIDEO_DECODER	4		/* 视频解码 */
#define AFK_PLUGIN_AUDIO_DECODER	5		/* 音频解码 */

/****************************************************************************
 * 该结构实例由plugin提供.
 * type == AFK_PLUGIN_DEVICEPROBE: init()函数返回afk_device_prob_t*
 ****************************************************************************/
typedef struct afk_plugin_info_s afk_plugin_info_t;

struct afk_plugin_info_s{
	unsigned short	type;						/* 插件类型 */
	unsigned short	API;						/* 支持的核心版本 */
	int				priority;					/* 优先级 */
	char			*id;						/* ID */
	unsigned long	version;					/* 自动增加的Plugin版本号 */
	void			*special_info;				/* plugin 自定义数据 */
	void			*(*init)(void *);			/* 初始化函数 */
};

typedef struct afk_device_prob_s afk_device_prob_t;

/****************************************************************************
 * 设备探测器插件
 * 如果测试连接成功,返回为一个afk_device_s类型的变量
 ****************************************************************************/
typedef void (__stdcall *fDisconnectCallBack)(void *device, void *channel, int nOnline, char *szIp, int nPort, int nInterfaceType, void *userdata);
typedef void (__stdcall *fDeviceEventCallBack)(void *device, int nEventType, void *pEventInfo, void *userdata);

typedef void (__stdcall *fListenCallback)(void* pServerHandle, char *szClientIp, int nClientPort, int nCommand, void *pParam, void *userdata);
typedef int  (__stdcall *fEventCallBack)(LONG lHandle, int connId, char *szIp, WORD wPort, LONG lCommand, void *pParam, DWORD dwParamLen, DWORD dwUserData);


struct afk_device_prob_s
{
	void* (*try_connect)(const char *szIp, int nPort, const char *szUser, const char *szPassword, int nSpecCap, void* pCapParam, 
						fDisconnectCallBack cbDisconnect, fDeviceEventCallBack cbDevEvent, void *userdata, 
						int *pErrorCode, int nTimeout, int nTryTime, BOOL bIsReConn, int nConnHostTime, int nSubConnSpaceTime, int nConnBufLen);

	void* (*start_listen)(char *szIp, int nPort, fListenCallback cbListen, void *userdata, int nConnBufLen);
	BOOL  (*stop_listen)(void *pListenHandle);
	BOOL  (*close_client)(void *pClientHandle);
	BOOL  (*response_reg)(void *pClientHandle, BOOL bAccept);

	BOOL  (*net_startup)(void);
	BOOL  (*net_cleanup)(void);

	LONG  (*start_alarm_server)(char *szIp, int nPort, fEventCallBack RecAlarmCallBack, unsigned long user_data);
	BOOL  (*stop_alarm_server)(LONG lHandler);
	int   (*server_heart_beat)(LONG lHandler);

	LONG  (*start_search_device)(afk_callback cbReceived, void *userdata);
	int   (*stop_search_device)(LONG lHandle);

	LONG  (*new_tcp_socket)(char *szIp, int nPort, int nWaitTime);
	int   (*delete_tcp_socket)(LONG lHandle);
	int   (*write_data)(LONG lHandle, char *pBuf, int nBufLen);
};

#endif // _AFKPLUGIN_H_

