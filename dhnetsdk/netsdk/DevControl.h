/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：控制类
* 摘　要：控制功能模块。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DEVCONTROL_H_
#define _DEVCONTROL_H_

#include "StdAfx.h"
#include "netsdk.h"
class CManager;

#define WAIT_TIME 200


class CDevControl  
{
public:

	/* 透明串口信息结构 */
	typedef struct
	{
		afk_device_s*		device;
		afk_channel_s*		channel;
		unsigned int		transComtype;
		fTransComCallBack	cbFunc;
		DWORD				dwUser;

		//用于回调与关闭操作的同步
		DEVMutex*			pcsLock;			

		LN_LIFECOUNT		life;
	} st_TransCom_Info;

	/* 远程升级信息结构 */
	typedef struct
	{
		afk_device_s*		device;
		afk_channel_s*		channel;
		fUpgradeCallBack	cbUpgrade;
		DWORD				dwUser;
		OS_EVENT			hRecEvent;

		//用于回调与关闭操作的同步
		DEVMutex*			pcsLock;			

		LN_LIFECOUNT		life;
	} st_Upgrade_Info;

public:
	class SearchTCI;
	friend class SearchTCI;

	class SearchUI;
	friend class SearchUI;

public:
	CDevControl(CManager *pManager);
	virtual ~CDevControl();

public:
	
	/* 初始化 */
	int							Init(void);

	/* 清理 */
	int							Uninit(void);

	/* 关闭设备所有通道 */
	int							CloseChannelOfDevice(afk_device_s* device);

	/* 打开一个透明串口 */
	LONG						CreateTransComChannel(LONG lLoginID, int TransComType, 
                                      unsigned int baudrate, unsigned int databits,
                                      unsigned int stopbits, unsigned int parity,
                                      fTransComCallBack cbTransCom, DWORD dwUser);

	/* 发送透明串口数据 */
	int							SendTransComData(LONG lTransComChannel, char *pBuffer, DWORD dwBufSize);

	/* 关闭一个透明串口 */
	int							DestroyTransComChannel(LONG lTransComChannel);

	/* 开始升级设备 */
	LONG						StartUpgrade(LONG lLoginID, char *pchFileName, fUpgradeCallBack cbUpgrade, DWORD dwUser);

	LONG						StartUpgradeEx(LONG lLoginID, EM_UPGRADE_TYPE emType, char *pchFileName, fUpgradeCallBack cbUpgrade, DWORD dwUser);

	/* 发送升级数据 */
	int							SendUpgrade(LONG lUpgradeID);

	/* 结束升级设备 */
	int							StopUpgrade(LONG lUpgradeID);

	/*现在已经废掉了Reset接口*/
	int							Reset(LONG lLoginID, BOOL bReset);

	/* 强制I帧 */
	int							MakeKeyFrame(LONG lLoginID, int nChannelID, int nSubChannel);

	/* 设置最大码流 */
	int							SetMaxFlux(LONG lLoginID, WORD wFlux);

	/*重启设备*/
	int							RebootDev(LONG lLoginID);
	
	/*关闭设备，仅特定机器支持*/
	int							ShutDownDev(LONG lLoginID);

	/* 控制设备，包括重启、关闭、硬盘控制等功能 */
	int ControlDevice(LONG lLoginID, CtrlType type, void *param, int waittime);


private:
	/* 查找硬盘标识 */
	int							QueryDiskIndex(LONG lLoginID, int nIndex);
	/*用于查找硬盘是否具备分区的能力*/
	int							QueryPartitionAbility(LONG lLoginID);


private:

	/* 透明串口列表 */
	list<st_TransCom_Info*>		m_lstTCI;
	DEVMutex						m_csTCI;	/* 锁透明串口列表 */
		
	/* 升级列表 */
	list<st_Upgrade_Info*>		m_lstUI;
	DEVMutex						m_csUI;		/* 锁升级列表 */


private:
	CManager*		m_pManager;
};


class CDevControl::SearchTCI
{
	LONG m_lchn;
public:
	SearchTCI(LONG lchn):m_lchn(lchn){}

	bool operator()(st_TransCom_Info*& tci)
	{
		return m_lchn == (LONG)(tci?tci->channel:0);
	}
};

class CDevControl::SearchUI
{
	LONG m_lchn;
public:
	SearchUI(LONG lchn):m_lchn(lchn){}

	bool operator()(st_Upgrade_Info*& ui)
	{
		return m_lchn == (LONG)(ui?ui->channel:0);
	}
};


#endif // _DEVCONTROL_H_


