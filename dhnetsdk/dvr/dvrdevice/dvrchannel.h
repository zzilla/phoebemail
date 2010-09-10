/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：抽象通道基类
* 摘　要：每个协议交互过程可以看成一个抽象通道。
*         采用抽象工厂的设计模式，为netsdk层提供了统一的接口，屏蔽了因功能不同协议不同造成差异性。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRCHANNEL_H_
#define _DVRCHANNEL_H_

#include "../osIndependent.h"
#include "../kernel/afkdef.h"
#include "../kernel/afkinc.h"

#include "../ReadWriteMutex.h"
using namespace SPTools;

class CDvrDevice;


class CDvrChannel : public afk_channel_s
{
public:
	CDvrChannel(CDvrDevice *pDvrDevice, int nMainCommand);
	virtual ~CDvrChannel();

public:
    /* 关闭通道 */
    virtual BOOL	channel_close() = 0;

    /* 暂停通道 */
    virtual BOOL	channel_pause(BOOL pause) = 0;

    /* 获取信息 */
    virtual int		channel_get_info(int type, void *parm) = 0;

    /* 设置信息 */
    virtual int		channel_set_info(int type, void *parm) = 0;

	/* 处理命令 */
	virtual int		OnRespond(unsigned char *pBuf, int nLen);


public:
    int				GetMainCommand() { return m_nMainCommand; }
	int				GetChannelType() { return m_nChannelType; }
    CDvrDevice*		channel_getdevice() { return m_pDvrDevice; }


public:
	int				channel_addRef();
	int				channel_decRef();

	void			startstatisc();
    void			statiscing(unsigned int nLen);
    unsigned int	statisc();


protected:
    CDvrDevice*		m_pDvrDevice;
    int             m_nMainCommand;
	int				m_nChannelType;

    unsigned int    m_nStatistic;
    BOOL            m_bStatiscEn;

	CReadWriteMutex	m_csDataCallBack;	// 此锁用于跟停止回调同步
	LN_LIFECOUNT	m_llifecount;
};

#endif // _DVRCHANNEL_H_


