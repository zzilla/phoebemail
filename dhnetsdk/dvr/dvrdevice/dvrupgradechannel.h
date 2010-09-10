/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：升级功能交互类
* 摘　要：升级功能数据交互。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRUPGRADECHANNEL_H_
#define _DVRUPGRADECHANNEL_H_

#include "dvrchannel.h"
class CDvrDevice;


class CDvrUpgradeChannel : public CDvrChannel
{
public:
	CDvrUpgradeChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam);
	virtual ~CDvrUpgradeChannel();

public:
	/* 关闭通道 */
    virtual BOOL	channel_close();

    /* 暂停通道 */
    virtual BOOL	channel_pause(BOOL pause);

    /* 获取信息 */
    virtual int		channel_get_info(int type, void *parm);

    /* 设置信息 */
    virtual int		channel_set_info(int type, void *parm);

	/* 处理命令 */
	virtual int		OnRespond(unsigned char *pBuf, int nLen);


public:
    afk_upgrade_channel_param_s	GetUpgradeChannelParam() { return m_upgrade_channel_param; }
	BOOL			GetUpgradeState() { return m_bSendDone; }
	void			SetUpgradeState(BOOL bSendDone) { m_bSendDone = bSendDone; }
	unsigned int	GetMainType() { return m_upgrade_channel_param.type; }


public:
    unsigned int	m_packetindex;
	OS_THREAD		m_hThread;
	OS_EVENT		m_hExitThread;
	DWORD			m_dwThreadID;

protected:
    afk_upgrade_channel_param_s  m_upgrade_channel_param;
    BOOL			m_bSendDone;
};


#endif // _DVRUPGRADECHANNEL_H_


