/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：对外接口实现类
* 摘　要：实现接口。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRINTERFACE_H_
#define _DVRINTERFACE_H_

#include "StdAfx.h"

/*****************************设备接口*******************************/
/* 版本信息 */
char*	interface_version(afk_device_s *devive);

/* 制造商信息 */							
char*	interface_manufactory(afk_device_s *devive);

/* 产品信息 */
char*	interface_product(afk_device_s *devive);

/* IP地址 */
char*	interface_ip(afk_device_s *devive);

/* 端口 */
int		interface_port(afk_device_s *devive);

/* 类型 */
int		interface_type(afk_device_s *devive);

/* 通道个数 */
int		interface_channelcount(afk_device_s *devive);

/* 报警输入个数 */                        
int		interface_alarminputcount(afk_device_s *devive);

/* 报警输出个数 */                     
int		interface_alarmoutputcount(afk_device_s *devive);

/* 关闭设备 */
int		interface_close(afk_device_s *devive);

/* 得到通道 */
void*	interface_get_channel(afk_device_s *devive, int type, unsigned int parm);
					
/* 打开通道 */
void*	interface_open_channel(afk_device_s *devive, int type, void *parm);

/* 是否通道打开 */
BOOL	interface_channel_is_opened(afk_device_s *devive, int type, void *parm);

/* 获取信息 */
int		interface_get_info(afk_device_s *devive, int type, void *parm);

/* 设置信息 */
int		interface_set_info(afk_device_s *devive, int type, void *parm);

/* 设备资源生命计数 */
int		interface_dev_addref(afk_device_s *devive);
int		interface_dev_decref(afk_device_s *devive);


/*****************************通道接口*******************************/
/* 得到设备 */
void*	interface_get_device(afk_channel_s *channel);

/* 关闭通道 */
BOOL	interface_close_ch(afk_channel_s *channel);

/* 暂停通道 */
BOOL	interface_pause_ch(afk_channel_s *channel, BOOL pause);
						
/* 获取信息 */
int		interface_get_info_ch(afk_channel_s *channel, int type, void *parm);

/* 设置信息 */
int		interface_set_info_ch(afk_channel_s *channel, int type, void *parm);


/*****************************创建连接*******************************/
#ifdef NETSDK_VERSION_SSL
template <class T> T* device_create_connect(afk_connect_param_t* pConnParam, BOOL bSSL=FALSE) 
#else
template <class T> T* device_create_connect(afk_connect_param_t* pConnParam) 
#endif
{
	T* pSubSocket = new T();
	if (NULL == pSubSocket)
	{
		return NULL;
	}
	
	if (pSubSocket->CreateRecvBuf(pConnParam->nConnBufSize) < 0)
	{
		delete pSubSocket;
		return NULL;
	}

#ifdef NETSDK_VERSION_SSL
	// SSL
	if (bSSL)
	{
		pSubSocket->SetSSL(1);
	}
#endif
	
	BOOL bIsConnect = FALSE;
	int nTryTime = pConnParam->nTryTime;
	while (nTryTime > 0)
	{
		if (pSubSocket->ConnectHost(pConnParam->szSevIp, pConnParam->nSevPort, pConnParam->nConnTime) >= 0)
		{
			bIsConnect = TRUE;
			break;
		}
		
		nTryTime--;
	}
	
	if (!bIsConnect)
	{
		delete pSubSocket;
		return NULL;
	}
	
	return pSubSocket;
}

		
#endif // _DVRINTERFACE_H_

