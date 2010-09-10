/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：控制功能交互类
* 摘　要：控制功能数据交互。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRCONTROLCHANNEL_H_
#define _DVRCONTROLCHANNEL_H_

#include "dvrchannel.h"
class CDvrDevice;


class CDvrControlChannel : public CDvrChannel
{
public:
	CDvrControlChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam);
	virtual ~CDvrControlChannel();
	
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
    unsigned int	GetControlType1() { return m_control_channel_parm.type1; }


private:
    afk_control_channel_param_s m_control_channel_parm;
};


#endif // _DVRCONTROLCHANNEL_H_

