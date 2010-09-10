/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：新配置类
* 摘　要：新配置功能模块(Json协议格式)
*
*/
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEVNEWCONFIG_H__02ACB021_5772_48ED_B4A2_3358EAE097C4__INCLUDED_)
#define AFX_DEVNEWCONFIG_H__02ACB021_5772_48ED_B4A2_3358EAE097C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "netsdk.h"

class CManager;

class CDevNewConfig  
{
public:
	CDevNewConfig(CManager *pManager);
	virtual ~CDevNewConfig();
	
public:
	/* 初始化 */
	int					Init(void);

	/* 清理 */
	int					Uninit(void);

	/* 关闭一个设备的所有通道 */
	int					CloseChannelOfDevice(afk_device_s* device);

	/* 同步配置信息 */
	int					SysConfigInfo(LONG lLoginID, int nType, void* pCondition, char *pOutBuffer, int maxlen, int *pRetlen, int *error, int *restart, int waittime=1000);


	/* 新的获取设备配置（字符串协议）*/
	int					GetDevConfig(LONG lLoginID, char* szCommand, int nChannelID, char* szOutBuffer, DWORD dwOutBufferSize, int *error, int waittime/*=500*/);
	
	/* 新的设置设备配置（字符串协议）*/
	int					SetDevConfig(LONG lLoginID, char* szCommand, int nChannelID, char* szInBuffer, DWORD dwInBufferSize, int *error, int *restart, int waittime/*=500*/); 

private:
	CManager*			m_pManager;
};

#endif // !defined(AFX_DEVNEWCONFIG_H__02ACB021_5772_48ED_B4A2_3358EAE097C4__INCLUDED_)
