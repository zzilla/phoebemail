/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：对外接口实现类
* 摘　要：实现接口。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DEVPROB_H_
#define _DEVPROB_H_

#include "./kernel/afkplugin.h"
typedef struct yw_prob_s yw_prob_t;

struct yw_prob_s {
	afk_device_prob_t base;
};


void build_login_packet(unsigned char *pBuf, const char *szUser, const char *szPassWord, int nSpecCap, void *pCapParam, unsigned int *pExtLen);


void* try_connect_dvr(const char *szIp, int nPort, const char *szUser, const char *szPassWord, 
                  int nSpecCap, void *pCapParam, fDisconnectCallBack cbDisconnect, fDeviceEventCallBack cbDevEvent, void *userdata, 
				  int *pErrorCode, int nTimeout=3000, int nTryTime=1, BOOL bIsReConn=FALSE, int nConnHostTime=1500, int nSubConnSpaceTime=10, int nConnBufLen=200*1024);


void* start_listen_dvr(char *szIp, int nPort, fListenCallback cbListen, void *userdata, int nConnBufLen);

BOOL stop_listen_dvr(void *pListenHandle);

BOOL response_reg_dvr(void *pClientHandle, BOOL bAccept);

BOOL close_client_dvr(void *pClientHandle);


BOOL network_startup(void);

BOOL network_cleanup(void);


LONG _start_alarm_server(char *szIp, int nPort, fEventCallBack RecAlarmCallBack, unsigned long user_data);

BOOL _stop_alarm_server(LONG lHandler);

int _heart_beat(LONG lHandler);


LONG _start_search_device(afk_callback cbReceived, void* pUserData);

int _stop_search_device(LONG lHandle);


int _write_data(LONG lHandle, char *pBuf, int nBufLen);

int _delete_tcp_socket(LONG lHandle);

LONG _new_tcp_socket(char *szIp, int nPort, int nWaitTime);


#endif // _DEVPROB_H_

